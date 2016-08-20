/**
 * Touch source that serves as the sink of touch events dispatched by the application
 * and as a source of touch events send to connected devices.
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

#ifndef DISPLAYDEVICE

#include "Touch.Source.h"
#include "Environs.Obj.h"		/// Access environs native object
#include "Environs.Lib.h"		/// Access environs API
#include "Environs.Native.h"	/// Access to native logging
#include "Environs.Utils.h"
#include "Device/Device.Controller.h"
#include "Interop/jni.h"

/// Includes for recognizer
#include "Recognizer/Gesture.Bezel.Touch.h"
#include "Recognizer/Gesture.Three.Touch.h"

#include <stdio.h>
#include <errno.h>

#ifndef WINDOWS_PHONE
#	include <unistd.h>
#	include <sys/time.h>
#	include <netinet/in.h>
#endif

using namespace environs::lib;


#define	CLASS_NAME 	"Touch.Source . . . . . ."

/* Namespace: environs -> */
namespace environs
{
    int t1_delta = 0;

	namespace API
	{
		ENVIRONSAPI void EnvironsFunc ( TouchDispatchN, jint hInst, jint portalID, jobject buffer, jint count, jboolean init )
		{
			Instance * env = instances [ hInst ];

			if ( env->environsState < environs::Status::Connected )
				return;

			PortalDevice * portal = GetLockedPortalDevice ( portalID );
			if ( !portal ) {
				CErr ( "TouchDispatch: No portal resource found." );
				return;
			}
#ifdef ANDROID
			InputPackRaw * touches;
#endif

			if ( !buffer ) {
				CVerbVerb ( "TouchDispatch: called with NULL buffer argument!" );
				if ( portal->receiver )
					portal->receiver->touchSource->Flush ();
				goto Finish;
			}

#ifdef ANDROID
			touches = ( InputPackRaw * ) jenv->GetDirectBufferAddress ( buffer );
			if ( !touches ) {
				CErr ( "TouchDispatch: Failed to get reference to memory of the shared buffer!" );
				goto Finish;
			}
			portal->receiver->touchSource->Handle ( touches, count, init );
#else
			portal->receiver->touchSource->Handle ( ( environs::lib::InputPackRaw * ) buffer, count, init );
#endif
		Finish:
			ReleasePortalDevice ( portal );
		}


		ENVIRONSAPI void EnvironsFunc ( SetUseTouchRecognizerEnableN, jint hInst, jboolean enable )
		{
			TouchSource::recognizersEnabled = ( bool ) enable;
		}
	}


	bool TouchSource::recognizersEnabled = true;

	/*
	* Each Frame consists of a state: Init, Added, Moved, Dropped, Flush
	* Frame consists of all available touches w
	* A Touch consists of its data; currently x,y,session_id.
	* A Touch also consists of its state: added, moved, drop
	*
	* A TouchSink checks the received touches against its living touches:
	* - if a touch is
	*
	*/
	TouchSource::TouchSource ( int deviceID, unsigned int capacity )
	{
		CVerbArgID ( "Construct: Capacity [ %u ]", capacity );

        allocated           = false;
        
        device              = 0;
        portalID            = -1;
        this->deviceID		= deviceID;
        this->capacity		= capacity;
        
		touches				= nill;
		touchesSize			= 0;

		alive				= false;
        frameNumber         = 1;
		aliveThreadID		= 0;

		last_heartBeat		= 0;

		viewAdapt			= false;
		xScale				= 1;
		yScale				= 1;

		xOffset				= 0;
		yOffset				= 0;

        recognizers			= 0;

        t1_delta = INTEROPTIMEMS ( 1000 );
	}


	TouchSource::~TouchSource ( )
	{
		CVerbID ( "Destructor" );

		Stop ( );

		if ( touches ) {
			delete touches;
			touches = 0;
		}
        
        if ( recognizers ) {
            delete recognizers;
            recognizers = 0;
        }
        
        if ( allocated ) {
            allocated = false;
            
            LockDispose ( &accessMutex );
        }
	}

    
    bool TouchSource::Init ( void * device, int portalID )
    {
        CVerbID ( "Init" );
        
        if ( capacity <= 0 ) {
            CErrID ( "Init: capacity for buffer is <= 0." );
            return false;
        }
        
        if ( !device ) {
            CVerbID ( "Init: Invalid device argument." );
            return false;
        }
        
        if ( this->device ) {
            UnlockDevice ( (DeviceBase *)this->device );
        }
        
        IncLockDevice ( (DeviceBase *)device );
        
        this->device = device;
        this->portalID = portalID;
        
        touches = new lib::InputPackRec * [capacity];
        if ( !touches ) {
            CErrID ( "Init: Failed to allocate touch buffer." );
            return false;
        }
        memset ( touches, 0, capacity * sizeof(lib::InputPackRec*) );
        
        if ( !allocated )
        {
            if ( !LockInit ( &accessMutex ) )
                return false;
            
            allocated = true;
        }
        
        lib::InputFrame * frame	= (lib::InputFrame *) sendBuffer;
        frame->preamble [0] = 't';
        frame->preamble [1] = 'f';
        frame->preamble [2] = ':';
        frame->version		= UDP_MSG_PROTOCOL_VERSION;
        
        frame->deviceID		= deviceID;
        
        if ( !recognizers ) {
            recognizers = Recognizers::GetRecognizers ( ((DeviceBase *)device)->env, deviceID, &native.display );
            if ( recognizers )
                return recognizers->Init ( false );
        }
        
        return true;
    }
    
    
    bool TouchSource::RemoveRecognizer ( const char * modName )
    {
        CVerbID ( "RemoveRecognizer" );
        
        if ( recognizers )
            return recognizers->RemoveRecognizer ( modName) ;
        
        return false;
    }


	bool TouchSource::AddRecognizer ( const char * modName )
	{
		CVerbID ( "AddRecognizer" );
        
        if ( !recognizers )
            recognizers = Recognizers::GetRecognizers ( ((DeviceBase *)device)->env, deviceID, &native.display );
        
        if ( recognizers )
            return recognizers->AddRecognizer ( modName, true );
        
        return false;
	}


	bool TouchSource::Start ()
	{
		CVerbID ( "Start" );

		if ( !LockAcquire ( &accessMutex, "Start" ) )
			return false;

		bool success = false;

		alive = true;

		DeviceBase * dev = ( DeviceBase * ) device;

		if ( dev->portalReceivers [ 0 ] ) {
			lib::InputFrame * frame	= ( lib::InputFrame * ) sendBuffer;
			frame->appOrPortalID = portalID;
		}
        
		//
		// Create alive thread for sending heart-beat and updates
		//
		if ( aliveThreadID == 0 )
		{
			int ret = pthread_create ( &aliveThreadID, NULL, TouchSource::AliveThread, this );
			if ( ret != 0 ) {
				CErrID ( "Start: Failed to create alive thread!" );
				goto Finish;
			}

			CVerbID ( "Start: Alive thread created ..." );
		}
        
        if ( recognizers ) {
            recognizers->SetIncomingPortalID ( portalID );
            recognizers->Start ( device );
        }
        
        success = true;
        
    Finish:
		if ( !LockRelease ( &accessMutex, "Start" ) )
			return false;
        
		return success;
	}

    
	bool TouchSource::Stop ( )
	{
		CVerbID ( "Stop" );

		bool ret = alive;

		alive = false;

		if ( aliveThreadID != 0 ) {
			CVerbID ( "Stop: stopping alive thread!" );

			pthread_join ( aliveThreadID, NULL );
			aliveThreadID = 0;
		}

		Flush ();

		if ( !LockAcquire ( &accessMutex, "Stop" ) )
			return false;
        
        if ( recognizers )
            recognizers->Stop ();
        
        if ( device ) {
            UnlockDevice ( (DeviceBase *)this->device );
            device = 0;
        }

		if ( !LockRelease ( &accessMutex, "Stop" ) )
			return false;
		return ret;
	}


	void TouchSource::Clear ( )
	{
		for ( int i=0; i<touchesSize; i++ ) {
			if ( touches [i] ) {
				delete touches [i];
				touches [i] = 0;
			}
		}
		touchesSize = 0;
	}

    
	void TouchSource::Handle ( InputPackRaw * touchPacks, unsigned int count, bool init )
    {
		CVerbArgID ( "Handle: count [ %i ] [ %s ]", count, init ? "init" : "update" );
		//CLogArgID ( "Init: id [ %i ]   x [ %i ]  y [ %i ]", id, x, y );
        
        if ( init ) {
            Flush ( );
            //frameNumber++;
        }
        
        if ( pthread_mutex_lock ( &accessMutex ) ) {
            CErr ( "Handle: Failed to lock mutex." );
            return;
        }

		if ( ( touchesSize + count ) >= capacity ) {
			// Increase capacity
			CInfoArgID ( "Handle: increasing capacity [ %i ] to [ %i ]", capacity, touchesSize + count );

			int capacity_old = capacity;
			capacity = touchesSize + count;

			InputPackRec ** touches_new = new InputPackRec * [ capacity ];
			if ( !touches_new ) {
				CErrArg ( "Handle: Failed to increase capacity to [ %i ]", capacity );
				return;
			}

			memset ( touches_new, 0, capacity * sizeof ( InputPackRec * ) );
			memcpy ( touches_new, touches, capacity_old * sizeof ( InputPackRec * ) );
			delete touches;
			touches = touches_new;
		}
        
		int dropped = 0;
        
        for ( unsigned int i = 0; i < count; i++ )
        {
            InputPackRaw * touch = touchPacks + i;
            
            switch ( touch->state )
            {
				case INPUT_STATE_ADD:
				{
					/// Look whether we already have this touch
					for ( unsigned int j = 0; j < touchesSize; j++ )
					{
						InputPackRec * ntouch = touches [ j ];

						if ( ntouch->raw.id == touch->id )
						{
							CVerbArgID ( "Handle: Add but found already added.. Alter Add to a Change id [ %i ]   x [ %i ]  y [ %i ]", touch->id, touch->x, touch->y );
							touch->state = INPUT_STATE_CHANGE;

							ntouch->org_x = touch->x;
							ntouch->org_y = touch->y;

							touch->x	+= xOffset;
							touch->y	+= yOffset;
							
							if ( viewAdapt ) {
								touch->x	= ( int ) ( ( ( double ) touch->x ) * xScale );
								touch->y	= ( int ) ( ( ( double ) touch->y ) * yScale );
							}
							memcpy ( &ntouch->raw.x, &touch->x, INPUTPACK_X_TO_END_SIZE );

							CVerbArgID ( "Handle: Added id [ %i ]   x [ %i ]  y [ %i ]", ntouch->raw.id, ntouch->raw.x, ntouch->raw.y );
							break;
						}
					}

					if ( touch->state == INPUT_STATE_ADD )
					{
						InputPackRec * ntouch = AllocTouch ( touch );
						if ( ntouch )
						{
							ntouch->org_x = touch->x;
							ntouch->org_y = touch->y;

							CVerbVerbArgID ( "Handle: Add id [ %i ]   x [ %i ]  y [ %i ] RAW", touch->id, touch->x, touch->y );

							ntouch->raw.x	+= xOffset;
							ntouch->raw.y	+= yOffset;

							if ( viewAdapt ) {
								ntouch->raw.x	= ( int ) ( ( ( double ) ntouch->raw.x ) * xScale );
								ntouch->raw.y	= ( int ) ( ( ( double ) ntouch->raw.y ) * yScale );
							}

							CVerbArgID ( "Handle: Add id [ %i ]   x [ %i ]  y [ %i ]", ntouch->raw.id, ntouch->raw.x, ntouch->raw.y );
							touches [ touchesSize ] = ntouch;
							touchesSize++;
						}
					}
					break;
				}

				case INPUT_STATE_DROP:
				{
					CVerbArgID ( "Handle: Drop id [ %i ]   x [ %i ]  y [ %i ]", touch->id, touch->x, touch->y );
					dropped++;
				}

				case INPUT_STATE_CHANGE:
				{
					for ( unsigned int j = 0; j < touchesSize; j++ )
					{
						InputPackRec * ntouch = touches [ j ];

						if ( ntouch->raw.id == touch->id )
						{
							CVerbVerbArgID ( "Handle: %s id [ %i ]   x [ %i ]  y [ %i ] RAW", touch->state == INPUT_STATE_DROP ? "Drop" : "Change", touch->id, touch->x, touch->y );

							ntouch->org_x = touch->x;
							ntouch->org_y = touch->y;

							touch->x	+= xOffset;
							touch->y	+= yOffset;

							if ( viewAdapt ) {
								touch->x	= ( int ) ( ( ( double ) touch->x ) * xScale );
								touch->y	= ( int ) ( ( ( double ) touch->y ) * yScale );
							}
							CVerbArgID ( "Handle: %s id [ %i ]   x [ %i ]  y [ %i ]", touch->state == INPUT_STATE_DROP ? "Drop" : "Change", touch->id, touch->x, touch->y );

							memcpy ( &ntouch->raw.state, &touch->state, INPUTPACK_STATE_TO_END_SIZE );
							CVerbArgID ( "Handle: Updated to id [ %i ]   x [ %i ]  y [ %i ]", touch->id, touch->x, touch->y );
							break;
						}
					}
					break;
				}

#ifndef DEBUGVERB
				default:
					CErrArgID ( "Handle: Invalid state [ %i ] for id [ %i ]   x [ %i ]  y [ %i ]", touch->state, touch->id, touch->x, touch->y );
					break;
#endif
            }
        }

		bool send = true;

		if ( dropped >= touchesSize ) {
			// All touches have been dropped
			// -> we reset active recognizers and send the dropped frame
			if ( recognizersEnabled && recognizers )
				recognizers->Flush ();
		}
		else {
			// If a drop event happend and no recognizer has taken over the touches, then send a frame
			if ( dropped && ( recognizers && recognizers->activeRecognizer < 0 ) )
				SendFrame ( false );

			// Remove the dropped touches
			for ( unsigned int i = 0; i < touchesSize; i++ ) {
				if ( touches [ i ]->raw.state == INPUT_STATE_DROP ) {
					CVerbArgID ( "Handle: Dropping id [ %i ]   x [ %i ]  y [ %i ]", touches [ i ]->raw.id, touches [ i ]->raw.x, touches [ i ]->raw.y );

					int remaining = touchesSize - i - 1;
					if ( remaining > 0 )
						memcpy ( touches + i, touches + i + 1, remaining * sizeof ( InputPack * ) );
					touchesSize--;
				}
			}

			if ( recognizersEnabled && recognizers ) {
				int res = recognizers->Perform ( touches, touchesSize );

				if ( res >= RECOGNIZER_HANDLED ) {
					if ( res >= RECOGNIZER_TAKEN_OVER_INPUTS ) {
						SendFrame ( false );

						recognizers->Finish ( touches, touchesSize );
					}
					send = false;
				}
				else if ( res == RECOGNIZER_GIVE_BACK_INPUTS ) {
					// Recognizer gave up responsibility for the touches
					for ( int j = 0; j < touchesSize; j++ ) {
						touches [ j ]->raw.state = INPUT_STATE_ADD;
					}
				}
			}
		}
        
        if ( pthread_mutex_unlock ( &accessMutex ) ) {
            CErr ( "Handle: Failed to unlock mutex." );
            return;
        }
        
        if ( send )
            SendFrame ( );
    }


	void TouchSource::Flush ( )
	{
		CVerbID ( "Flush" );

		if ( !LockAcquire ( &accessMutex, "Flush" ) )
			return;

		if ( recognizersEnabled && recognizers )
			recognizers->Flush ();

		Clear ();

		// Send flush event to TouchSink
        SendFrame ( false );
        
		LockReleaseV ( &accessMutex, "Flush" );
	}


	void TouchSource::Cancel ( )
	{
		CVerbID ( "Cancel" );

		if ( !LockAcquire ( &accessMutex, "Cancel" ) )
			return;
        
        if ( recognizersEnabled && recognizers )
            recognizers->activeRecognizer = -1;

		if ( !LockRelease ( &accessMutex, "Cancel" ) )
			return;

		// TODO Need to implement custom handling of cancelation!!!
		Flush ();
	}


	/*
	* Header:	tf:version(1)	id(4)	size(2)	frameState(1) touchCount(1)
	* Touch:	s_id(4)	x(2)	y(2)	angle(4)
	* <EOF>
	*/
	void TouchSource::SendFrame ( bool useLock )
	{
		CVerbVerbID ( "SendFrame" );

		int count = 0;
		InputFrame * frame = ( InputFrame * ) sendBuffer;

		if ( useLock && pthread_mutex_lock ( &accessMutex ) ) {
			CErr ( "SendFrame: Failed to lock mutex." );
			return;
		}

		frame->frameNumber = frameNumber; frameNumber++;

		// Set framestate

		//CLogArg ( "SendFrame: size of touchpack is [%u], count of touches [ %i ]", touchPackSize, touchesSize );

		if ( touchesSize > 0 )
		{
			char * touchSegment = ( sendBuffer + sizeof ( InputFrame ) );
			for ( int i=0; i<touchesSize; i++ )
			{
				InputPackRec * touch = touches [ i ];
				if ( touch ) {
					//CLogArg ( "SendFrame: copy [ %i ]", i );
					memcpy ( touchSegment, &touch->raw, INPUTPACK_V3_SIZE );

					count++;
				}
				touchSegment += INPUTPACK_V3_SIZE;
			}
		}

		//CLogArg ( "SendFrame: done copying [ %i ]", count );
		frame->count = ( char ) count;

		int length = sizeof ( InputFrame ) + ( int ) ( count * INPUTPACK_V3_SIZE );
		frame->size = length;

		// Send to TouchSink
		CVerbVerbArgID ( "SendFrame: frames [ %i ] size [ %i ]", frame->count, length );

		if ( device && ( ( DeviceBase * ) device )->SendDataPacket ( sendBuffer, length ) )
		{
			// Update heartbeat indicator
			last_heartBeat = GetEnvironsTickCount ();
		}

		if ( useLock && pthread_mutex_unlock ( &accessMutex ) ) {
			CErr ( "SendFrame: Failed to unlock mutex." );
			return;
		}
	}


	InputPackRec * TouchSource::AllocTouch ( InputPackRaw * touchPack )
	{
		InputPackRec * touch = new InputPackRec ();
		if ( touch ) {
			memcpy ( &touch->raw, touchPack, sizeof ( InputPackRaw ) );
		}
		return touch;
    }
	

	void * TouchSource::AliveThread ( void * object )
	{
		TouchSource * source = ( TouchSource * ) object;
		if ( !source ) {
			CErr ( "AliveThread: called with (NULL) argument!" );
			return 0;
		}
		return source->AliveThreadInstance ();
	}


	void * TouchSource::AliveThreadInstance ()
	{
		CVerb ( "AliveThread: started ..." );

		pthread_setname_current_envthread ( "TouchSource::AliveThread" );

		while ( alive )
		{
			if ( !recognizers || ( recognizers->activeRecognizer < 0 ) )
			{
				INTEROPTIMEVAL now = GetEnvironsTickCount ();

				if ( ( now - last_heartBeat ) > t1_delta )
				{
					SendFrame ();
				}
			}

			sleep ( 1 );
		}

		return 0;
	}


}

#endif

