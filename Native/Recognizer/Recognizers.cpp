/**
 * Recognizer manager
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Recognizers.h"
#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Device/Device.Controller.h"
#include "Gesture.Three.Finger.h"
#include "Touch.Visualizer.h"

#ifdef ENVIRONS_IOS
/// Includes for recognizer
#   include "Recognizer/Gesture.Bezel.Touch.h"
#   include "Recognizer/Gesture.Three.Touch.h"
#endif

using namespace environs::lib;


// The TAG for prepending to log messages
#define CLASS_NAME	"Recognizers. . . . . . ."


namespace environs 
{
	//
	// Externals new Recognizers

	//
	// Static class members
	volatile bool	Recognizers::active 			= false;
	

    
    Recognizers * Recognizers::GetRecognizers ( Instance * obj, int deviceID, DeviceDisplay * screen )
    {
        if ( Kernel::touchRecognizerNamesCount <= 0 || !obj )
            return 0;
        
        Recognizers * recs = new Recognizers ( deviceID, screen );
        if ( recs ) {
            recs->env = obj;
        }
        return recs;
    }
    

	Recognizers::Recognizers ( int deviceID, DeviceDisplay * screen )
	{
		this->deviceID = deviceID;

		CLogID ( "Construct..." );

        allocated                       = false;
		active							= false;
		useThread						= true;
        env                             = 0;

        activeRecognizer                = -1;
		recognizers						= 0;
		recognizersCount				= 0;
		recognizedGesture				= false;

		recognizerContainerIndexer		= 1;
		recognizerTouchesCount [0]		= 0;
		recognizerTouchesCount [1]		= 0;
		recognizerTouchesHandled [0]	= true;
		recognizerTouchesHandled [1]	= true;

		Zero ( recognizerThreadID );
        
		if ( screen )
			recognizerDisplay = *screen;
		else
			ZeroStruct ( recognizerDisplay, DeviceDisplay );
	}


	Recognizers::~Recognizers ( )
	{
		CLogID ( "Destruct..." );

		Dispose ( );

        if ( allocated ) {
            allocated = false;
            
            LockDispose ( &accessMutex );
            
            CondDispose ( &recognizerEvent );
		}
		CLogID ( "Destruct: destroyed." );
	}


	void Recognizers::Dispose ( )
	{
		CLogID ( "Dispose" );


		// Dispose recognizers
		if ( recognizers ) {
			for ( int i = 0; i < recognizersCount; i++ ) {
				if ( recognizers [i] )
					delete recognizers [i];
			}
			free ( recognizers );
			recognizers = 0;
		}

		if ( recognizerTouches [0] ) {
			free ( recognizerTouches [0] );
			recognizerTouches [0] = 0;
		}
		if ( recognizerTouches [1] ) {
			free ( recognizerTouches [1] );
			recognizerTouches [1] = 0;
		}
		recognizersCount = 0;
		recognizedGesture = false;
	}



	bool Recognizers::Init ( bool useThread )
	{
		CLogID ( "Init" );
        
        this->useThread = useThread;
        
        if ( !allocated ) {
            Zero ( recognizerEvent );
            
            if ( pthread_cond_manual_init ( &recognizerEvent, 0 ) ) {
                CErr ( "Init: Failed to init stateSignal!" );
                return false;
            }
            
            if ( !LockInit ( &accessMutex ) )
                return false;
            
            allocated = true;
        }
        
        if ( useThread ) {
			recognizerTouches [0] = (InputPackRec *) calloc ( MAX_TOUCH_VISUALS, sizeof ( InputPackRec ) );
            if ( !recognizerTouches [0] )
                return false;

			recognizerTouches [1] = (InputPackRec *) calloc ( MAX_TOUCH_VISUALS, sizeof ( InputPackRec ) );
            if ( !recognizerTouches [1] )
                return false;
        }
        else {
            recognizerTouches [0]	= 0;
            recognizerTouches [1]	= 0;
        }
        
        if ( !recognizers )
        {
            recognizedGesture = false;
            recognizersCount = 0;
            
            recognizers = (IInputRecognizer **) calloc ( 1, sizeof(IInputRecognizer *)* ENVIRONS_INPUT_RECOGNIZER_MAX );
            if ( !recognizers )
                return false;
            
            for ( unsigned int i = 0; i < Kernel::touchRecognizerNamesCount; i++ )
            {
                if ( Kernel::touchRecognizerNames [i] )
                    AddRecognizer ( Kernel::touchRecognizerNames [i], false );
            }
        }
        
        // Start recognizer thread
        if ( recognizersCount && this->useThread && !pthread_valid ( recognizerThreadID ) )
        {
            int rc = pthread_create ( &recognizerThreadID, NULL, &Recognizers::recognizerThreadStarter, this );
            if ( rc != 0 ) {
                CErrID ( "Init: Failed to create thread for gesture recognizer!" );
                return false;
            }
            pthread_detach_handle ( recognizerThreadID );
            
            recognizerTouchesHandled [0]	= true;
            recognizerTouchesHandled [1]	= true;
        }

		return true;
	}


	bool Recognizers::SetIncomingPortalID ( int portalID )
	{
		CVerbArgID ( "SetIncomingPortalID: [%i]", portalID );

		bool success = true;

		while ( recognizersCount )
		{
			if ( !LockAcquire ( &accessMutex, "SetIncomingPortalID" ) )
				break;

			for ( int index = 0; index < recognizersCount; index++ )
			{
				IInputRecognizer * reco = recognizers [index];
				if ( reco && !reco->SetIncomingPortalID ( portalID ) )
					success = false;
			}

			if ( !LockRelease ( &accessMutex, "SetIncomingPortalID" ) )
				break;
			return success;
		}
		return false;
	}
    
    
    // We include touch recognizers only for ios platforms as those dont allow loading of additional libraries
#ifdef ENVIRONS_IOS
    IInputRecognizer * getRecognizerInstance ( const char * moduleName )
    {
        if ( strstr ( moduleName, "libEnv-RecGestureBezelTouch" ) ) {
            return new GestureBezelTouch ( );
        }
        if ( strstr ( moduleName, "libEnv-RecGestureThreeTouch" ) ) {
            return new GestureThreeTouch ( );
        }
        return 0;
    }
#endif
    
    
    bool Recognizers::AddRecognizer ( const char * modName, bool checkDuplicate )
    {
        CVerbID ( "AddRecognizer" );
        
        if ( !modName )
            return false;
        
        if ( recognizersCount >= ENVIRONS_INPUT_RECOGNIZER_MAX )
            return false;

		if ( !LockAcquire ( &accessMutex, "AddRecognizer" ) )
			return false;
        
        bool success = false;
    
        
        IInputRecognizer * reco = (IInputRecognizer *)
        
#ifdef ENVIRONS_IOS
        environs::getRecognizerInstance ( modName );
#else
        environs::API::CreateInstance ( modName, 0, InterfaceType::InputRecognizer, deviceID, env );
#endif
        if ( !reco )
            goto Finish;
        
        if ( checkDuplicate ) {
            /// Make sure that this recognizer has not already been added
            for ( int i = 0; i < recognizersCount; i++ ) {
                if ( !strcmp ( recognizers [i]->name, reco->name ) ) {
                    CErrArgID ( "AddRecognizer: Recognizer [%s] in module [%s] has already been added.", reco->name, modName );
                    goto Finish;
                }
            }
        }
        
        if ( !reco->Init ( device, &native.display ) ) {
            CErrID ( "AddRecognizer: Failed to initiailze recognizer." );
            goto Finish;
        }
        
        recognizers [recognizersCount] = reco;
        reco = 0;
        
        recognizersCount++;
        success = true;
        
        CVerbArg ( "AddRecognizer: successfully added [%s]", modName );
        
	Finish:
		LockReleaseV ( &accessMutex, "AddRecognizer" );

        if ( success )
            return true;
        
        if ( reco ) {
            HMODULE hLib = (HMODULE)reco->hModLib;
            delete reco;
            if ( hLib )
                dlclose ( hLib );
        }
        CVerbArg ( "AddRecognizer: Add of [%s] failed.", modName );
        return false;
    }
    
    
    
    bool Recognizers::RemoveRecognizer ( const char * modName )
    {
        CVerbID ( "RemoveRecognizer" );
        
        if ( !modName )
            return false;

		if ( !LockAcquire ( &accessMutex, "RemoveRecognizer" ) )
			return false;
        
        HMODULE hLib;
        bool success = false;
        
        for ( int index = 0; index < recognizersCount; index++ ) {
            if ( !strcmp ( recognizers [index]->name, modName ) ) {
                
                CVerbArg ( "RemoveRecognizer: Removing recognizer [%i/%s]", index, modName );
                
                IInputRecognizer * reco = recognizers [index];
                
				hLib = (HMODULE)reco->hModLib;
                
                delete reco;
                if ( hLib )
                    dlclose ( hLib );
                
                for ( int j = index; j < recognizersCount - 1; j++ ) {
                    recognizers [ j ] = recognizers [ j + 1 ];
                }
                recognizersCount--;
                success = true;
                break;
            }
        }

		LockReleaseV ( &accessMutex, "RemoveRecognizer" );

        return success;
    }

    
    void Recognizers::SetDeviceBase ( void * device )
    {
        CVerbID ( "SetDeviceBase" );
        
        if ( recognizersCount )
		{
			if ( !LockAcquire ( &accessMutex, "SetDeviceBase" ) )
				return;
            
            for ( int index = 0; index < recognizersCount; index++ )
            {
                IInputRecognizer * reco = recognizers [index];
                if ( reco )
                    reco->deviceBase = device;
            }

			LockReleaseV ( &accessMutex, "SetDeviceBase" );           
        }
    }
    

	bool Recognizers::Start ( void * device )
	{
		CVerbID ( "Start" );
        
        if ( !device )
            return false;
        
        SetDeviceBase ( device );
        
		return true;
	}


	bool Recognizers::Stop ( )
	{
		CVerbID ( "Stop" );
        
        SetDeviceBase ( 0 );
        
		return true;
	}
    
    
    int Recognizers::Perform ( InputPackRec ** touches, int touchesSize )
    {
        if ( activeRecognizer >= 0 ) {
            CVerbArgID ( "Perform: Active recognizer [%u -> %s]", activeRecognizer, recognizers [activeRecognizer]->name );
            
            int performResult = recognizers [activeRecognizer]->Perform ( touches, touchesSize );
            if ( performResult <= 0 ) {
                CVerbArgID ( "Perform: Active recognizer [%u -> %s] failed. Reset state of recognizer chain.", activeRecognizer, recognizers [activeRecognizer]->name );
                
                activeRecognizer = -1;
                return performResult;
            }
            return 1;
        }
        
        int triggerCmd = 0;
        
        CVerbID ( "Perform: Try looking for a recognizer to trigger..." );
        
        for ( int i = 0; i < recognizersCount; i++ )
        {
            IInputRecognizer * reco = recognizers [i];
            
            if ( reco->triggerTouchCount != touchesSize && reco->triggerTouchCount )
                continue;
            
            triggerCmd = reco->Trigger ( touches, touchesSize );
            if ( triggerCmd != RECOGNIZER_REJECT ) {
                CVerbArgID ( "Perform: Recognizer [%u -> %s] triggered successfully.", i, reco->name );
                activeRecognizer = i;
                
                if ( triggerCmd > 1 ) {
                    /// Untouch the touches
                    CVerbArgID ( "Perform: Recognizer [%u -> %s] Untouch the state.", i, reco->name );
                    
                    for ( int j = 0; j < touchesSize; j++ ) {
                        touches [j]->raw.state = INPUT_STATE_DROP;
                    }
                }
                return triggerCmd;
            }
        }
        return triggerCmd;
    }
    
    
    void Recognizers::Finish ( InputPackRec ** touches, int touchesSize )
    {
        CVerbID ( "Finish" );
        
        for (  int j = 0; j < touchesSize; j++ ) {
            touches [j]->raw.state = INPUT_STATE_CHANGE;
        }
    }
    
    
    void Recognizers::Flush ( )
    {
        CVerbID ( "Flush" );
        
        if ( activeRecognizer >= 0 ) {
            CVerbArgID ( "Finish: Finish active recognizer [%u -> %s]", activeRecognizer, recognizers [activeRecognizer]->name );
            
            recognizers [activeRecognizer]->Flush ();
            
            activeRecognizer = -1;
        }
    }
    

	void * Recognizers::recognizerThreadStarter ( void * arg )
	{
		if ( !arg ) {
			CErr ( "recognizerThreadStarter: called with invalid (NULL) argument." );
			return 0;
		}

		// Execute thread
		return ((Recognizers *)arg)->Thread_Recognizer ( );
	}


	void * Recognizers::Thread_Recognizer ( )
	{
		CLogID ( "Thread_Recognizer started..." );

		pthread_setname_current_envthread ( "Recognizers::Thread_Recognizer" );

		while ( active )
		{
			pthread_cond_mutex_lock ( &accessMutex );

			if ( pthread_cond_wait_time ( &recognizerEvent, &accessMutex, INFINITE ) )
			{
				CLogID ( "Recognizer: error occoured during wait for recognizer event or portal closed!" );

				pthread_cond_mutex_unlock ( &accessMutex );
				break;
			}
			pthread_cond_mutex_unlock ( &accessMutex );

			unsigned int	recoIndex		= 0;

			if ( pthread_mutex_lock ( &accessMutex ) ) {
				CErrID ( "Recognizer: Failed to lock accessMutex." );
				break;
			}

			recoIndex = recognizerContainerIndexer;

			if ( pthread_mutex_unlock ( &accessMutex ) ) {
				CErrID ( "Recognizer: Failed to unlock accessMutex." );
				break;
			}

			unsigned int	recoCount		= recognizerTouchesCount [recoIndex];
			InputPackRec *	recoContainer	= recognizerTouches [recoIndex];

			if ( recognizerTouchesHandled [recoIndex] )
				continue;

			if ( !recoCount || !recoContainer ) {
				recognizerTouchesHandled [recoIndex] = true;
				continue;
			}

			if ( recognizersCount && recognizers ) {
				recognizedGesture = false;

				for ( int i = 0; i < recognizersCount; i++ ) {
					if ( recognizers [i]->Perform ( (InputPackRec **)recoContainer, recoCount ) ) {
						recognizedGesture = true;
						break;
					}
				}
			}

			if ( pthread_mutex_lock ( &accessMutex ) ) {
				CErrID ( "Recognizer: Failed to lock accessMutex." );
				break;
			}

			recognizerTouchesHandled [recoIndex] = true;

			if ( pthread_mutex_unlock ( &accessMutex ) ) {
				CErrID ( "Recognizer: Failed to unlock accessMutex." );
				break;
			}

			pthread_cond_preparev ( &recognizerEvent );
		}

		CLogID ( "Thread_Recognizer terminated..." );
		return 0;
	}


} // -> namespace

