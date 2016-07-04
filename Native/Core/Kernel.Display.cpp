/**
* Environs Windows Kernel
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

#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Kernel.Display.h"
#include "Device/Device.Controller.h"
#include "Renderer/Render.OpenCL.h"
#include "DynLib/Dyn.OpenCL.h"

#include <string>
#include <map>
using namespace std;



// The TAG for prepending to log messages
#define CLASS_NAME	"Kernel.Display . . . . ."


namespace environs 
{
	//
	// Static class members	
	//ITracker *	KernelDevice::tracker = 0;
	char			*	KernelDevice::trackerNames [ ENVIRONS_TRACKER_MAX ];
	unsigned int		KernelDevice::trackerNamesCount = 0;
	
	//
	// Externals
	extern const char * pref_dataStoreDefault;
	extern bool			InitStorageUtil ( const char * storagePath );


	namespace API
	{
	}
	
    
    
    int Kernel::SetUseTouchRecognizer ( const char * moduleName, bool enable )
    {
        CVerbVerb ( "SetUseTouchRecognizer" );
        
        if ( enable ) {
            if ( !Kernel::AddTouchRecognizer ( moduleName ) )
                return 0;
        }
        else {
            if ( !Kernel::RemoveTouchRecognizer ( moduleName ) )
                return 0;
        }
        
        return 1;
    }
    
	
	void KernelDevice::InitStatics ()
	{
		CVerb ( "InitStatics" );

		Zero ( trackerNames );

		trackerNamesCount = 0;
	}
	

	void KernelDevice::ReleaseStatics ()
	{
		CVerb ( "ReleaseStatics" );

		for ( unsigned int i = 0; i < trackerNamesCount; i++ )
			if ( trackerNames [ i ] )
				free ( trackerNames [ i ] );
		
		trackerNamesCount = 0;

		Zero ( trackerNames );

		RenderOpenCL::DisposeOpenCL ();

		if ( ocl_LibInitialized )
			ReleaseOpenCLLib ();
	}


	KernelDevice::KernelDevice ()
	{
		CVerb ( "Construct..." );

		/// No tracker at creation
		trackerCount = 0;

		/// Initialize tracker places
		Zero ( tracker );
	}


	KernelDevice::~KernelDevice ()
	{
		CVerb ( "Destruct..." );

		ReleaseTrackers ();

		CVerb ( "Destruct done." );
	}


	int KernelDevice::onPreInit ()
	{
		CVerb ( "onPreInit" );

		return true;
	}

    
    void KernelDevice::ReleaseTrackers ()
    {
        CVerb ( "ReleaseTrackers" );
        
        if ( trackerCount ) {
            for ( unsigned int i = 0; i < trackerCount; i++ ) {
                if ( tracker [ i ] ) {
                    environs::API::DisposeInstance ( tracker [ i ] );
                }
            }
        }
        
        trackerCount = 0;
        Zero ( tracker );
	}


	ITracker * KernelDevice::CreateInstance ( char * name )
	{
        ITracker * t = (ITracker *) environs::API::CreateInstance ( name, 0, InterfaceType::Tracker, 0, env );
		if ( !t ) {
			CWarnArg ( "createInstance: Failed to create instance of [%s].", name );
			return 0;
		}

		t->hEnvirons = hEnvirons;
		t->modName = name;
		t->env = env;
		return t;
	}


	bool KernelDevice::RunInstance ( ITracker * &t, int trackerID )
	{
		if ( !t )
			return false;

		if ( t->IsRuntimeSupported ( native.platform, 0 ) )
		{
			environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_STATE_INIT_SENSOR, trackerID, 0, 0 );

			if ( t->Init ( native.display.width, native.display.height, 4, native.display.width ) )
			{
				environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_STATE_START, trackerID, 0, 0 );

				if ( t->Start () == 1 ) {
					t->trackerID = trackerID;
					 
					CLogArg ( "runInstance: [%s] running.", t->name );
					return true;
				}
				else {
					environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_STATE_START_FAILED, trackerID, 0, 0 );
				}
			}
			else {
				environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_STATE_INIT_SENSOR_FAILED, trackerID, 0, 0 );
			}
		}

		CErrArg ( "runInstance: [%s] failed.", t->name );

		delete t;
		t = 0;
		return false;
	}


	int KernelDevice::onInitialized ()
	{
		CVerb ( "onInitialized" );

		trackerCount = 0;

		for ( unsigned int i = 0; i < trackerNamesCount; i++ ) {
			if ( !trackerNames [i] || !*trackerNames [i] )
				continue;

			ITracker * t = CreateInstance ( trackerNames [i] );
			if ( !t || !RunInstance ( t, trackerCount ) )
			{
				// Remove tracker from queue
				CLogArg ( "onInitialized: Removing tracker [%s] from queue.", trackerNames [i] );

				if ( t )
					delete t;
				environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_ENABLE_FAILED, trackerCount, trackerNames [i], (int)strlen ( trackerNames [i] ) );

				free ( trackerNames [i] );

				trackerNamesCount--;

				for ( unsigned int j = i; j < trackerNamesCount; j++ ) {
					trackerNames [j] = trackerNames [j + 1];
				}

				continue;
			}
			else {
				environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_ENABLED, trackerCount, trackerNames [i], (int) strlen ( trackerNames [i] ) );
			}

			tracker [trackerCount] = t;
			trackerCount++;
		}

		return true;
	}


	ITracker * KernelDevice::GetTracker ( unsigned int index )
	{
		CVerbVerb ( "GetTracker" );

		if ( trackerCount && index < trackerCount )
			return tracker [index];

		return 0;
	}


	int KernelDevice::SetUseTracker ( const char * moduleName )
	{
		CVerbVerb ( "SetUseTracker" );

		ITracker *	t		= 0;
		int			index	= -1;

		if ( !moduleName || !*moduleName )
			return index;

		size_t len = strlen ( moduleName );
		if ( !len )
			return index;

		index = GetUseTracker ( moduleName );
		if ( index >= 0 )
			return index;

		/// Add to tracker names if neccessary
		char * trackerName = 0;

		for ( unsigned int i = 0; i < trackerNamesCount; i++ ) {
			char * name = trackerNames [i];

			if ( name && !strcmp ( name, moduleName ) ) {
				trackerName = name; index = (int) i; break;
			}
		}

		do
		{
			if ( !trackerName )
			{
				if ( trackerNamesCount + 1 >= ENVIRONS_TRACKER_MAX ) {
					CErrArg ( "setUseTracker: Failed to add tracker. Tracker name queue is full [%d].", ENVIRONS_TRACKER_MAX );
					break;
				}

				trackerName = (char *) calloc ( 1, len + 1 );
				if ( !trackerName )
					break;

				strlcpy ( trackerName, moduleName, len + 1 );
				/*
				t = createInstance ( trackerName );
				if ( !t ) {
				free ( trackerName );
				break;
				}*/

				trackerNames [trackerNamesCount] = trackerName;

				//index = trackerNamesCount;
				trackerNamesCount++;
			}
            
            index = -1;
            
            if ( trackerCount + 1 >= ENVIRONS_TRACKER_MAX ) {
                CErrArg ( "setUseTracker: Failed to add tracker. Tracker queue is full [%d].", ENVIRONS_TRACKER_MAX );
                break;
            }
            
            if ( !t )
                t = CreateInstance ( trackerName );
            if ( !t || !RunInstance ( t, trackerCount ) ) {
                
                environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_ENABLE_FAILED, trackerCount, trackerName, (int) strlen ( trackerName ) );
                break;
            }
            
            index = trackerCount;
            
            tracker [index] = t;
            trackerCount++;
            
            environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_ENABLED, index, trackerName, (int) strlen ( trackerName ) );
            t = 0;
		} while ( 0 );

		if ( t )
			delete t;

		return index;
	}


	int KernelDevice::GetUseTracker ( const char * moduleName )
	{
		CVerbVerb ( "GetUseTracker" );

		ITracker * t;

		if ( !moduleName || !*moduleName )
			return -1;
        
        if ( !trackerCount )
            return -1;
        
        for ( unsigned int i = 0; i < trackerCount; i++ ) {
            t = tracker [i];
            
            if ( t && t->modName && !strcmp ( t->modName, moduleName ) )
                return (int) i;
        }
        
        //return -1;

		for ( unsigned int i = 0; i < trackerNamesCount; i++ ) {
			char * name = trackerNames [i];

			if ( name && !strcmp ( name, moduleName ) )
				return (int) i;
		}

		return -1;
	}


	bool KernelDevice::DisposeTracker ( const char * moduleName )
	{
		CVerbVerb ( "DisposeTracker" );

		ITracker * t;

		if ( !moduleName || !*moduleName )
			return false;

        
        if ( trackerCount )
        {
            for ( unsigned int i = 0; i < trackerCount; i++ ) {
                t = tracker [i];
                
                if ( t && t->modName && !strcmp ( t->modName, moduleName ) ) {
                    delete t;
                    
                    environs::API::onEnvironsNotifierContext1 ( env, 0, NOTIFY_TRACKER_DISABLED, i, 0, 0 );
                    
                    trackerCount--;
                    
                    for ( unsigned int j = i; j < trackerCount; j++ ) {
                        tracker [j] = tracker [j + 1];
                    }
                }
            }
        }

		for ( unsigned int i = 0; i < trackerNamesCount; i++ ) {
			char * name = trackerNames [i];

			if ( name && !strcmp ( name, moduleName ) ) {
				free ( name );

				trackerNamesCount--;

				for ( unsigned int j = i; j < trackerNamesCount; j++ ) {
					trackerNames [j] = trackerNames [j + 1];
				}

				return true;
			}
		}

		return false;
	}


	void KernelDevice::ReleaseLibrary ()
	{
		CVerb ( "ReleaseLibrary" );

		native.SetDataStore ( (char *) opt_dataStoreDefault );
	}


	int KernelDevice::onPreStart ()
	{
		CVerb ( "onPreStart" );

		return true;
	}


	int KernelDevice::onStarted ()
	{
		CVerb ( "onStarted" );

		return true;
	}


	int KernelDevice::onPreStop ()
	{
		CVerb ( "onPreStop" );

		return true;
	}


	int KernelDevice::onStopped ()
	{
		CVerb ( "onStopped" );

		ReleaseTrackers ();
		return true;
	}


	int KernelDevice::SetMainAppWindow ( WNDHANDLE appWnd )
	{
		if ( GetActivePortals () > 0 )
			return -1;

		env->appWindowHandle	= appWnd;

		return 1;
	}




} /* namespace environs */
