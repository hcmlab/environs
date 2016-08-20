/**
 *	Portal Generator
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
#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
using namespace environs::API;

#include "Portal.Info.Base.h"
#include "Portal.Generator.h"
#include "Portal/Portal.Device.h"
#include "Environs.Lib.h"
#include "Core/Performance.Count.h"
#include "Device/Device.Controller.h"

#ifdef _WIN32
#	ifndef WINDOWS_PHONE
#		pragma warning( push )
#		pragma warning( disable: 4458 )
#		include <gdiplus.h>
#		pragma warning( pop )
#	endif
#else
#	include <fcntl.h>
#	include <errno.h>
#endif

#include <cmath>

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator . . . ."


namespace environs 
{
	//
	// Externals
	extern pthread_mutex_t	devicesAccessMutex;

	//
	// Static class members
	LONGSYNC		PortalGenerator::referenceCount		= 0;

	pthread_cond_t	PortalGenerator::hPortalUpdateTimer;

	pthread_cond_t	PortalGenerator::portalWorkerEvent;
	pthread_mutex_t PortalGenerator::portalWorkerEventLock;
	

	void CALLBACK PortalUpdateTimerFunc ( void * lpParametar, EBOOL TimerOrWaitFired )
	{
		CVerbVerb ( "PortalUpdateTimerFunc" );
        
        if ( pthread_cond_signal ( &captureClassTimerSignal ) ) {
            CVerb ( "PortalUpdateTimerFunc: Failed to signal captureClassTimerSignal!" );
        }
        pthread_cond_preparev ( &captureClassTimerSignal );
        
		/// Lock the signal mutex
		pthread_cond_mutex_lock ( &PortalGenerator::portalWorkerEventLock );

		/// Signal a worker thread
		if ( pthread_cond_signal ( &PortalGenerator::portalWorkerEvent ) ) {
			CVerb ( "PortalUpdateTimerFunc: Failed to signal PortalGenerator::portalWorkerEvent!" );
		}

		/// UnLock the signal mutex
		pthread_cond_mutex_unlock ( &PortalGenerator::portalWorkerEventLock );
	}


	PortalGenerator::PortalGenerator ()
	{
		CLog ( "Construct..." );

		status						= PortalSourceStatus::Created;

		allocated					= false;
		env							= 0;
        
#ifdef ENABLE_PORTAL_STALL_MECHS
        stalled                     = false;
        stalledCounter              = 0;
#endif
		portalID					= -1;
		accessLocks					= 1;
		streamOptions				= nill;

		filledContexts				= 0;
		Zero ( workerStages );
        
        renderContextNext			= 0;
        
        Zero ( renderDimensionsChanged );
        Zero ( workerThreads );

		for ( int i=0; i < MAX_PORTAL_CONTEXT_WORKERS; ++i )
			workerThreadsID [ i ] = ENVIRONS_THREAD_NO_THREAD;
		
		//ZeroStructArray ( renderContexts, RenderContext );
		// Zero is fine as rendercontexts have no floats and semaphores of any type will be explicitly initialized by the generator
		Zero ( renderContexts );

		Zero ( renderOverlays );
		ZeroStruct ( portalInfosCache, PortalInfoBase );

		centerX						= 0;
		centerY						= 0;
		renderDimensions.left		= 300;
		renderDimensions.top		= 300;
		renderDimensions.width_cap	= 250;
		renderDimensions.height_cap	= 800;
		renderDimensions.orientation= 90.0f;
		//orientation				= 135.0f;
		//orientationLast			= 135.0f;
		renderDimensions.square		= 0;

		orientationLast				= 90.0f;
		deviceAzimut				= 0.0f;
		deviceAzimutLast			= 0.0f;

		recognizers					= 0;

#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		recognizersCount			= 0;
		recognizedGesture			= false;

		pthread_reset ( recognizerThreadID );
        recognizerThreadIDState     = ENVIRONS_THREAD_NO_THREAD;
		
		recognizerContainerIndexer		= 1;
		recognizerTouchesCount [0]		= 0;
		recognizerTouchesCount [1]		= 0;
		recognizerTouchesHandled [0]	= true;
		recognizerTouchesHandled [1]	= true;
#endif
	}


	bool PortalGenerator::InitClass ()
	{
		CVerb ( "InitClass" );
		
		memset ( &hPortalUpdateTimer, 0, sizeof(pthread_cond_t) );

		CVerb ( "InitClass: Creating portal worker event" );
        
        if ( !CondInit ( &portalWorkerEvent ) )
            return false;

        if ( !LockInit ( &portalWorkerEventLock ) )
            return false;

		return true;
	}

	
	void PortalGenerator::InitFrameTrigger ()
	{
		// Create the timer thread
		if ( !pthread_cond_valid ( hPortalUpdateTimer ) ) {
			CVerb ( "InitFrameTrigger: Creating portal trigger timer" );

#if (defined(_WIN32) && !defined(WINDOWS_PHONE))
			BOOL success = CreateTimerQueueTimer ( &hPortalUpdateTimer, NULL,
				(WAITORTIMERCALLBACK)PortalUpdateTimerFunc, NULL, 0,
				33,
				WT_EXECUTEDEFAULT ); // WT_EXECUTEINTIMERTHREAD ); // WT_EXECUTEDEFAULT
			if ( !success || !pthread_cond_valid ( hPortalUpdateTimer ) )
			{
				CErr ( "InitFrameTrigger: Failed to create timer for portal frames trigger!" );
			}
#endif
		}
	}

	
	void PortalGenerator::DisposeFrameTrigger ()
	{		
		//
		// Stop portal timer, if the last intance of DeviceController is about to be disposed
		if ( pthread_cond_valid ( hPortalUpdateTimer ) )
		{
			CVerb ( "DisposeFrameTrigger: Deleting portal trigger timer" );
#if (defined(_WIN32) && !defined(WINDOWS_PHONE))
			if ( !DeleteTimerQueueTimer ( NULL, hPortalUpdateTimer, INVALID_HANDLE_VALUE ) )
			{
				DWORD lastError = GetLastError ( );
				if ( lastError == ERROR_IO_PENDING ) { // <- this shouldn't be the case, since invalid handle value was supplied as completion routine, but we check it for fail case..
					hPortalUpdateTimer = NULL;
				}
				else {
					CErrArg ( "DisposeFrameTrigger: Failed to destroy timer for portal frames trigger (%i)! Try again next time if possible.", lastError );
				}
			}
			hPortalUpdateTimer = 0;
#else
			memset ( &hPortalUpdateTimer, 0, sizeof(hPortalUpdateTimer) );
#endif
		}
	}


	PortalGenerator::~PortalGenerator ()
	{
		CLogID ( "Destruct..." );

		// Check whether we can continue the disposal

		int count = 0;
		while ( ___sync_val_compare_and_swap ( &accessLocks, OBJECTSTATE_DELETEABLE_1, OBJECTSTATE_DELETED ) != OBJECTSTATE_DELETEABLE_1 ) {
			if ( count % 50 ) {
				CErrID ( "Destruct: Cannot destroy object. It is being accessed somewhere. spinning now.. wait for 100ms..." );
			}
			count++;
			Sleep ( 400 );
		}		

		Dispose ();

		int pid = PortalIndex ();

		if ( IsValidPortalIndex ( pid ) )
			parentDevice->portalGeneratorsDevice [pid] = -1;

        if ( allocated ) {
            allocated = false;
            
#ifndef ENABLE_IMPROVED_PORTAL_GENERATOR
            LockDispose ( &renderContextMutex );
#endif
            LockDispose ( &renderDimensionsMutex );
            LockDispose ( &renderOverlayMutex );
            
            CondDispose ( &workerStateEvent );
            
#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
            LockDispose ( &recognizerCritSec );
            
            CondDispose ( &recognizerEvent );
#endif
		}

		if ( __sync_sub_and_fetch ( &referenceCount, 1 ) == 0 )
		{
			DisposeFrameTrigger ();
            
            CVerbID ( "Destruct: Closing portal worker event." );
            
            CondDispose ( &portalWorkerEvent );
            
            LockDispose ( &portalWorkerEventLock );
		}
                
		DisposePortalDevice ( portalID );
        
		CVerbID ( "Destruct: destroyed." );
	}


	void PortalGenerator::Dispose ()
	{
		CVerbID ( "Dispose" );

		DisposePortal ();
        
        onEnvironsNotifier1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_STREAM_STOPPED, portalID );

		for ( unsigned int i = 0; i < MAX_PORTAL_OVERLAYS; i++ ) {
			if ( renderOverlays [i] ) {
				if ( renderOverlays [i]->data )
					free ( renderOverlays [i]->data );
				free ( renderOverlays [i] );
			}
		}
		Zero ( renderOverlays );

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		if ( recognizers ) {
			delete recognizers;
			recognizers = 0;
		}
#else
		if ( recognizerTouches [0] ) {
			delete (recognizerTouches [0]);
			recognizerTouches [0]	= 0;
		}
		if ( recognizerTouches [1] ) {
			delete (recognizerTouches [1]);
			recognizerTouches [1]	= 0;
		}
#endif
	}


	bool PortalGenerator::GetLock ()
	{
		CVerbID ( "GetLock" );

		if ( status != PortalSourceStatus::Deleteable ) {
			CVerbID ( "GetLock: is enabled" );

			__sync_add_and_fetch ( &accessLocks, 1 );
			return true;
		}
		return false;
	}


	void PortalGenerator::GetDimensionsLock ( RenderDimensions * &dims, lib::InputPackRec * &recoContainer, unsigned int &recoIndex )
	{
		//CVerbID ( "GetContextLock" );

		if ( status != PortalSourceStatus::Deleteable ) {
			//CVerbID ( "GetContextLock: is enabled" );

			if ( __sync_add_and_fetch ( &accessLocks, 1 ) == OBJECTSTATE_DELETEABLE_1 ) {
				//CVerbID ( "GetContextLock: is about to be deleted." );

				// Restore old lock
				__sync_sub_and_fetch ( &accessLocks, 1 );
				return;
			}

			dims = &renderDimensions;

			recoIndex = 0;

			if ( recognizers ) {
#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
#else
				if ( LockAcquire ( &recognizerCritSec, "GetDimensionsLock" ) ) {
					if ( recognizerContainerIndexer == 0 )
						recoIndex = 1;

					LockReleaseV ( &recognizerCritSec, "GetDimensionsLock" );

					recoContainer = recognizerTouches [recoIndex];
					recognizerTouchesCount [recoIndex] = 0;
				}
#endif
			}
		}
	}


	void PortalGenerator::ReleaseLock ()
	{
		CVerbID ( "ReleaseLock" );

		__sync_sub_and_fetch ( &accessLocks, 1 );
	}


	void PortalGenerator::ReleaseDimensionsLock ( unsigned int touchCount, unsigned int recoIndex )
	{
		CVerbVerbID ( "ReleaseDimensionsLock" );

		if ( touchCount ) {
#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
			WARNING ("Needs implementation");
#else
			if ( !pthread_mutex_trylock ( &recognizerCritSec ) )
			{
				recognizerTouchesCount [recoIndex] = touchCount;
				recognizerTouchesHandled [recoIndex] = false;
				recognizerContainerIndexer = recoIndex;

				if ( pthread_mutex_unlock ( &recognizerCritSec ) ) {
					CErrID ( "ReleaseDimensionsLock: Failed to unlock recognizerCritSec!" );
				}
				else {
					if ( pthread_cond_signal ( &recognizerEvent ) ) {
						CVerbID ( "ReleaseDimensionsLock: Failed to signal recognizerEvent!" );
					}
				}
			}
#endif
		}

		__sync_sub_and_fetch ( &accessLocks, 1 );
	}
	

	bool PortalGenerator::Init ( PortalStreamOptions * streamOptionsa, DeviceController * parent, int portalIDa )
	{
		CVerbID ( "Init" );

		if ( __sync_add_and_fetch ( &referenceCount, 1 ) == 1 )
			if ( !InitClass () )
				return false;

		if ( status >= PortalSourceStatus::Initialized )
			return true;

#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		recognizersCount			= 0;
		recognizedGesture			= false;

		pthread_reset ( recognizerThreadID );

		recognizerContainerIndexer		= 1;
		recognizerTouchesCount [ 0 ]		= 0;
		recognizerTouchesCount [ 1 ]		= 0;
		recognizerTouchesHandled [ 0 ]	= true;
		recognizerTouchesHandled [ 1 ]	= true;

		if ( env->useRecognizers ) {
			recognizerTouches [ 0 ] = new lib::InputPackRec [ MAX_TOUCH_VISUALS ];
			if ( !recognizerTouches [ 0 ] ) {
				env->useRecognizers = false;
			}
			else {
				memset ( recognizerTouches [ 0 ], 0, sizeof ( lib::InputPackRec ) * MAX_TOUCH_VISUALS );

				recognizerTouches [ 1 ] = new lib::InputPackRec [ MAX_TOUCH_VISUALS ];
				if ( !recognizerTouches [ 1 ] ) {
					delete recognizerTouches [ 0 ];
					recognizerTouches [ 0 ] = 0;
					env->useRecognizers = false;
				}
				else
					memset ( recognizerTouches [ 1 ], 0, sizeof ( lib::InputPackRec ) * MAX_TOUCH_VISUALS );
			}
		}
		else {
			recognizerTouches [ 0 ]	= 0;
			recognizerTouches [ 1 ]	= 0;
		}
#endif

        if ( !allocated ) {
            if ( !CondInit ( &workerStateEvent ) )
                return false;
            
            if ( !LockInit ( &renderOverlayMutex ) )
                return false;
            
            if ( !LockInit ( &renderDimensionsMutex ) )
                return false;

#ifndef ENABLE_IMPROVED_PORTAL_GENERATOR
            if ( !LockInit ( &renderContextMutex ) )
                return false;
#endif
            
#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
			Zero ( recognizerEvent );
			if ( pthread_cond_manual_init ( &recognizerEvent, NULL ) ) {
				CErrID ( "Init: Failed to init recognizerEvent." );
				return false;
			}
            
            if ( !LockInit ( &recognizerCritSec ) )
                return false;
#endif
			allocated = true;
		}

        int portalDeviceID = GetFreePortalSlot ( parent, portalIDa );
        
		if ( portalDeviceID < 0 || portalDeviceID >= MAX_PORTAL_INSTANCES ) {
            return false;
        }
        
		PortalDevice * portal = portalDevices + portalDeviceID;
        
        portalIDa = portal->portalID;
        
        portal->generator = this;

		deviceID					= parent->deviceID;
		portalID					= portalIDa;
        
		streamOptions				= streamOptionsa;
		parentDevice				= parent;

		int pid = PortalIndex ();
		if ( IsInvalidPortalIndex ( pid ) )
			return false;

		parentDevice->portalGeneratorsDevice [pid] = portalDeviceID;
		parentDevice->portalGeneratorsDeviceInput = portalDeviceID;
        
		renderDimensions.width_cap		= parentDevice->portalInfoOff.width;
		renderDimensions.height_cap		= parentDevice->portalInfoOff.height;

		renderDimensions.streamWidth	= streamOptionsa->streamWidth;
		renderDimensions.streamHeight	= streamOptionsa->streamHeight;

		renderDimensions.square = (int) sqrt ( pow ( (double) renderDimensions.width_cap, 2 ) + pow ( (double) renderDimensions.height_cap, 2 ) );
		if ( renderDimensions.square % 2 )
			renderDimensions.square++;
		/*if (square % 32) {
		int mod = square % 32;
		square += 32 - mod;
		}*/
        
		this->centerX = parentDevice->portalInfoOff.centerX;
		this->centerY = parentDevice->portalInfoOff.centerY;
		renderDimensions.orientation = parentDevice->portalInfoOff.orientation;
		
		// Cache x and change a little to enforce update of all parameters
		int x = this->centerX;
		this->centerX++;
		UpdatePosition ( x, centerY, renderDimensions.orientation );

		if ( !InitPortal () )
			return false;

		// Try establishing the prefered worker stages, if native layer is responsible for generation
		// Otherweise, native layer is only responsible for managing ids, messages, etc.
		//
		if ( env->usePlatformPortalGenerator ) {
			streamOptions->useStream = true;
			streamOptions->streamType = DATA_STREAM_H264_NALUS;			
		}
		else {
			if ( !GetWorkerStages ( &workerStages, 1 ) )
			{
				CWarnID ( "Init: Failed to establish the preferred worker stages for streaming." );

				// Try the fallback stages
				if ( !GetWorkerStages ( &workerStages, 2 ) )
				{
					CWarnID ( "Init: Failed to establish the fallback worker stages for streaming." );

					// Fallback to an image stream worker stages
					if ( !GetWorkerStages ( &workerStages, 0 ) )
					{
						CErrID ( "Init: Failed to establish the fallback worker stages for streaming sole images!" );
						return false;
					}
				}
            }
            streamOptions->streamType = workerStages.encoder->encodedType;
		}

		unsigned int streamType = ( ( streamOptions->streamType & DATA_STREAM_VIDEO ) == DATA_STREAM_VIDEO ) ? MSG_PORTAL_PROVIDE_STREAM : MSG_PORTAL_PROVIDE_IMAGES;

		if ( parentDevice->SendPortalMessage ( ( unsigned short ) streamType, portalIDa ) )
		{
			if ( env->usePlatformPortalGenerator || SendStreamInit ( renderDimensions.streamWidth, renderDimensions.streamHeight ) ) {
				return true;
			}
		}

		CErrID ( "Init: Failed to send portal request fallback to image message or stream initilization packet!" );

		return false;
	}


	bool PortalGenerator::SendStreamInit ( int width, int height )
	{
		CVerbID ( "SendStreamInit" );

        if ( width )
            renderDimensions.streamWidth = width;
        
        if ( height )
            renderDimensions.streamHeight = height;
        
		char buffer [24];

        unsigned int * pUI = reinterpret_cast<unsigned int *>(buffer);
        
        *pUI = 0; pUI++;
        *pUI = renderDimensions.streamWidth; pUI++;
		*pUI = renderDimensions.streamHeight;

		unsigned short streamType = (unsigned short) streamOptions->streamType;
		streamType &= 0xFFF0;
		streamType |= DATA_STREAM_INIT;
        
		/*if ( parentDevice->SendBuffer ( false, MSG_TYPE_STREAM, portalID, 0, streamType, buffer, sizeof ( unsigned int ) * 3 ) < 0 ) {
			CErrID ( "SendStreamInit: Failed to send stream init message" );
			return false;
		}*/

		if ( parentDevice->SendBuffer ( true, MSG_TYPE_STREAM, portalID, 0, streamType, buffer, sizeof ( unsigned int ) * 3 ) < 0 ) {
			CErrID ( "SendStreamInit: Failed to send stream init message" );
			return false;
		}
        
        int streamTypeNotifier = streamOptions->useStream ? NOTIFY_PORTAL_PROVIDE_STREAM_ACK : NOTIFY_PORTAL_PROVIDE_IMAGES_ACK;
        
        onEnvironsNotifier1 ( env, parentDevice->deviceNode->info.objID, streamTypeNotifier, portalID );

        status = PortalSourceStatus::Initialized;
        
		return true;
	}


	bool PortalGenerator::Start ()
	{
		CVerbID ( "Start" );

		if ( status < PortalSourceStatus::Initialized ) {
			CVerbID ( "Start: portal source has not been initialized. Try initializing..." );
			if ( !parentDevice )
				return false;

			if ( !Init ( streamOptions, parentDevice, portalID ) )
				return false;
		}
        
        if ( status < PortalSourceStatus::Active )
        {
            if ( !env->usePlatformPortalGenerator )
            {
                if ( workerStages.capture && workerStages.capture->portalWorkerEvent )
                    workerStages.capture->Start ();
                else
                    InitFrameTrigger ();
                
#ifndef ENABLE_WORKER_STAGES_LOCKS
                // Prepare the context events
				for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ ) {
                    pthread_cond_prepare_checked ( &renderContexts [i].eventEncoded );
                    pthread_cond_prepare_checked ( &renderContexts [i].eventRendered );
                }
                
                unsigned int contextStart = renderContextNext;
                if ( !contextStart )
					contextStart = MAX_PORTAL_CONTEXT_WORKERS - 1;
                contextStart--;
                
                pthread_cond_signal_checked ( &renderContexts [contextStart].eventEncoded );
                pthread_cond_signal_checked ( &renderContexts [contextStart].eventRendered );
#endif
            }
            status = PortalSourceStatus::Active;
            
            if ( Kernel::active ) {
                onEnvironsNotifier1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_STREAM_STARTED, portalID );
            }
        }

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		if ( recognizers )
			recognizers->Start ( parentDevice );
#endif

		return (status == PortalSourceStatus::Active);
	}


	bool PortalGenerator::Pause ()
	{
		CVerbID ( "Pause" );

		if ( status == PortalSourceStatus::Active ) {
			status = PortalSourceStatus::Initialized;

            if ( Kernel::active )
                onEnvironsNotifier1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_STREAM_PAUSED, portalID );
			return true;
		}
		return false;
	}


	bool PortalGenerator::Stop ()
	{
		CVerbID ( "Stop" );

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		if ( recognizers )
			recognizers->Stop ( );
#endif

		if ( status > PortalSourceStatus::Deleteable ) {
			status = PortalSourceStatus::Deleteable;

			if ( workerStages.capture && workerStages.capture->portalWorkerEvent )
				workerStages.capture->Stop ( );

            if ( Kernel::active )
                onEnvironsNotifier1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_STREAM_STOPPED, portalID );
			return true;
		}

		return false;
	}


	bool PortalGenerator::InitRenderContexts ()
	{
		CVerbID ( "InitRenderContexts" );

		DisposeRenderContexts ( );

		RenderContext * context;

		for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ )
		{			
			context = renderContexts + i;
			context->id = i;

#ifdef ENABLE_WORKER_STAGES_LOCKS
      
			if ( !env_sem_create ( &context->renderSem, 0, "r", deviceID, portalID, i ) ) {
				CErrArgID ( "InitRenderContexts: Failed to create render semaphore for render context [%i]", i );
				goto Failed;
			}

			if ( !env_sem_create ( &context->encoderSem, 0, "e", deviceID, portalID, i ) ) {
				CErrArgID ( "InitRenderContexts: Failed to create encoder semaphore for render context [%i]", i );
				goto Failed;
			}

			if ( !env_sem_create ( &context->sendSem, 0, "s", deviceID, portalID, i ) ) {
				CErrArgID ( "InitRenderContexts: Failed to create sender semaphore for render context [%i]", i );
				goto Failed;
			}
#else
			if ( !pthread_cond_valid ( context->eventEncoded ) ) {
				pthread_cond_manual_init ( &context->eventEncoded, 0 );
				if ( !pthread_cond_valid ( context->eventEncoded ) ) {
					CErrArgID ( "InitRenderContexts: Failed to create encoded event for portal context [%i]", i );
					goto Failed;
				}
			}

			if ( !pthread_cond_valid ( context->eventRendered ) ) {
				pthread_cond_manual_init ( &context->eventRendered, 0 );
				if ( !pthread_cond_valid ( context->eventRendered ) ) {
					CErrArgID ( "InitRenderContexts: Failed to create rendered event for portal context [%i]", i );
					goto Failed;
				}
			}
#endif
			context->renderSkipped = false;
			context->isInitialized = true;
		}

		context = &renderContexts [USE_WORKER_THREADS_INITIAL_CONTEXT - 1];

#ifdef ENABLE_WORKER_STAGES_LOCKS
		if ( !env_sem_post ( context->renderSem ) ) {
			CErrID ( "InitRenderContexts: Failed to set initial value of render semaphore for start" );
			goto Failed;
		}
		if ( !env_sem_post ( context->encoderSem ) ) {
			CErrID ( "InitRenderContexts: Failed to set initial value of encoder semaphore for start" );
			goto Failed;
		}
		if ( !env_sem_post ( context->sendSem ) ) {
			CErrID ( "InitRenderContexts: Failed to set initial value of sender semaphore for start" );
			goto Failed;
		}
#else
		pthread_cond_signal ( &context->eventEncoded );
		pthread_cond_signal ( &context->eventRendered );
#endif
		return true;

	Failed:
		return false;
	}


	bool PortalGenerator::InitPortal ()
	{
		CVerbID ( "InitPortal" );

		if ( parentDevice->interactSocket == -1 ) {
			CErrID ( "InitPortal: tcp socket invalid" );
			return false;
		}

		// Are we alredy sending the portal?
		if ( status >= PortalSourceStatus::Initialized ) {
			CErrID ( "InitPortal: we're already streaing and initialized" );
			return true;
		}

		if ( !env->usePlatformPortalGenerator )
		{
            if ( pthread_valid ( workerThreads [0] ) ) {
                // The portal is already active
                status = PortalSourceStatus::Initialized;
                return true;
            }
            
            // Create portal sync events
            if ( !InitRenderContexts () ) {
                CErrID ( "InitPortal: Failed to create render contexts." );
                goto EndWithFailure;
            }
            
            // Start portal worker thread
			for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ ) {
                renderDimensionsChanged [i] = 1;
                
                if ( workerThreadsID [i] == ENVIRONS_THREAD_DETACHEABLE && pthread_valid ( workerThreads [i] ) )
                    continue;
                
                renderContextNext = i;
                
                workerThreadsID [i] = ENVIRONS_THREAD_DETACHEABLE;
                
                pthread_create ( &(workerThreads [i]), NULL, &PortalGenerator::Thread_WorkerStarter, this );
                if ( !pthread_valid ( workerThreads [i] ) )
                {
                    workerThreadsID [i] = ENVIRONS_THREAD_NO_THREAD;
                    
                    CErrID ( "InitPortal: Failed to create thread for screen update!" );
                    goto EndWithFailure;
                }
                
                pthread_cond_mutex_lock ( &portalWorkerEventLock );
                if ( pthread_cond_wait_time ( &workerStateEvent, &portalWorkerEventLock, 1000 ) ) {
                    CErrID ( "InitPortal: Failed to wait for worker thread started event!" );
                    goto EndWithFailure;
                }
                pthread_cond_mutex_unlock ( &portalWorkerEventLock );
            }
            
            renderContextNext = USE_WORKER_THREADS_INITIAL_CONTEXT;
		}

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		if ( !recognizers ) {
			recognizers = Recognizers::GetRecognizers ( deviceID );
			if ( recognizers )
				return recognizers->Init ( true );
		}
#else
		if ( env->useRecognizers )
		{
			InitRecognizers ();

			// Start recognizer thread
			if ( recognizers && recognizerThreadIDState == ENVIRONS_THREAD_NO_THREAD )
			{
                recognizerThreadIDState = ENVIRONS_THREAD_DETACHEABLE;
                
				pthread_create ( &recognizerThreadID, NULL, &PortalGenerator::Thread_RecognizerStarter, this );
				if ( !pthread_valid ( recognizerThreadID ) )
                {
                    recognizerThreadIDState = ENVIRONS_THREAD_NO_THREAD;
                    
					CErrID ( "InitPortal: Failed to create thread for gesture recognizer!" );
					goto EndWithFailure;
				}

				recognizerTouchesHandled [0]	= true;
				recognizerTouchesHandled [1]	= true;
			}
		}
#endif
		return true;

	EndWithFailure:
		DisposePortal ( );
		return false;
	}
   
    
	void PortalGenerator::DisposeWorkerStages ( WorkerStages * stages )
	{
		CVerbID ( "DisposeWorkerStages" );

        if ( !stages )
            return;
        
		if ( stages->capture )
			environs::API::DisposeInstance ( stages->capture );
        
        if ( stages->render )
			environs::API::DisposeInstance ( stages->render );
        
        if ( stages->encoder ) {
			if ( stages->encoder->sendID >= 0 ) {
				FAKEJNI ();
				EnvironsCallArg ( ReleasePortalSendIDN, stages->encoder->sendID );
			}
            environs::API::DisposeInstance ( stages->encoder );
        }
        
		ZeroStruct ( *stages, WorkerStages );
    }
    
    
	bool PortalGenerator::GetWorkerStages ( WorkerStages * stages, int index )
	{
        CVerbArgID ( "GetWorkerStages: Requested index [%u]!", index );
        
        if ( !stages ) {
            CErrID ( "GetWorkerStages: Invalid stages argument!" );
            return false;
        }
        
        bool success = false;
        
        DisposeWorkerStages ( stages );

        do {
            if ( !CreateWorkerStages ( stages, index ) ) {
                CErrArgID ( "GetWorkerStages: Failed to create custom worker stages for index [%u]!", index ); break;
            }
            
            if ( !stages->capture || !stages->encoder ) {
                CErrArgID ( "GetWorkerStages: One of the custom worker stages for index [%u] is invalid [%c | %c | %c]!", index,
                           stages->capture ? 'v' : 'i', stages->render ? 'v' : 'i', stages->encoder ? 'v' : 'i' ); break;
            }
            
            // Check whether we have a null renderer.
            if ( stages->render && stages->render->interfaceType == InterfaceType::Unknown ) {
                environs::API::DisposeInstance ( stages->render );
                stages->render = 0;
            }
            stages->capture->stages = stages;
			stages->capture->portalID = portalID;
            stages->encoder->stages = stages;
            
			if ( !stages->capture->PreInit ( deviceID, (stages->capture->captureType == CaptureType::Camera ? (void *) ((size_t) portalID) : parentDevice->portalhWnd), streamOptions ) ) {
				CErrID ( "GetWorkerStages: Failed to pre initialize capture stage." ); break;
			}

			if ( stages->capture->captureType == CaptureType::Camera ) {
				//streamOptions->streamWidth = stages->capture->width;
				//streamOptions->streamHeight = stages->capture->height;

				renderDimensions.streamWidth = stages->capture->width;
				renderDimensions.streamHeight = stages->capture->height;
				renderDimensions.width_cap	= stages->capture->width;
				renderDimensions.height_cap	= stages->capture->height;
				renderDimensions.orientation = 90;

				stages->capture->portalWorkerEvent = &portalWorkerEvent;
				stages->capture->portalWorkerEventLock = &portalWorkerEventLock;
			}

            // Connect and initialize the stages
            if ( stages->render ) {
                stages->render->stages = stages;
                
                if ( !ConnectModules ( stages->capture, stages->render ) ) {
                    CErrID ( "GetWorkerStages: Failed to connect capture stage to render stage." ); break;
                }
                
                if ( !ConnectModules ( stages->render, stages->encoder ) ) {
                    CErrID ( "GetWorkerStages: Failed to connect render stage to encoder stage." ); break;
                }
            }
            else {
                // A Null renderer requires the capture output to be directly connected to the encoder input
                
                if ( !ConnectModules ( stages->capture, stages->encoder ) ) {
                    CErrID ( "GetWorkerStages: Failed to connect capture stage to encoder stage." ); break;
                }
                
                for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ )
                    renderContexts[i].renderSkipped = true;
            }
            
            if ( !stages->capture->initialized && !stages->capture->Init () ) {
                CErrID ( "GetWorkerStages: Failed to initialize capture stage." ); break;
            }
            
            if ( stages->render && !stages->render->initialized && !stages->render->Init ( deviceID, this, parentDevice->portalhWnd ) ) {
                CErrID ( "GetWorkerStages: Failed to initialize render stage." ); break;
            }
            
            int bitRateOrPng = (index == 2 ? streamOptions->usePNG : env->streamBitrateKB * 1000);
            int fps = (index == 0 ? 0 : env->streamFPS);
            
            if ( !stages->encoder->initialized && !stages->encoder->Init ( deviceID, bitRateOrPng, renderDimensions.streamWidth, renderDimensions.streamHeight, fps ) ) {
                CErrID ( "GetWorkerStages: Failed to initialize encoder stage." ); break;
            }
            
            /// Please note: requireSendID must not be used by jni implementations. We fake the environment here, as the (java wrapped) call requires that.
            /// JNI environments must call the acquire/release from within a java native call.
            if ( stages->encoder->requireSendID ) {
                FAKEJNI ();
                stages->encoder->sendID = EnvironsCallArg ( AcquirePortalSendIDN, stages->encoder->encodedType, portalID );
                if ( stages->encoder->sendID < 0 ) {
                    CErrID ( "GetWorkerStages: AcquirePortalSendID returned invalid sendID." ); break;
                }
            }
            
            stages->capture->renderOverlays = renderOverlays;
            stages->capture->renderOverlayMutex = &renderOverlayMutex;
            success = true;
        }
        while ( false );
        
        if ( !success )
            DisposeWorkerStages ( stages );
		return success;
	}


	bool PortalGenerator::CreateWorkerStages ( WorkerStages * stages, int index )
	{
        CVerbArgID ( "CreateWorkerStages: Requested index [%u]!", index );

		return false;
    }

	
	bool PortalGenerator::setPortalOverlayARGB ( int layerID, int left, int top, unsigned int width, unsigned int height, unsigned int stride, void * renderData, int alpha, bool positionDevice )
	{
		if ( layerID >= MAX_PORTAL_OVERLAYS ) {
			CErrArgID ( "setPortalOverlayARGB: layerID [%u] must be between 0 and 2.", layerID );
			return false;
		}
		
		void			* data		= 0;
		RenderOverlay	* overlay	= 0;
		RenderOverlay	* old		= 0;

		if ( alpha < 0 ) {
			/// alpha < 0 means clear the particular layer
			if ( renderOverlays [ layerID ] ) {
				old = renderOverlays [ layerID ];

				if ( !LockAcquire ( &renderOverlayMutex, "setPortalOverlayARGB" ) )
					goto Finish;

				renderOverlays [layerID] = 0;

				LockReleaseV ( &renderOverlayMutex, "setPortalOverlayARGB" );
			}
			goto Finish;
		}

		if ( !renderData ) {
			/// Null renderData means change the alpha value
			if ( renderOverlays [ layerID ] ) {
				renderOverlays [layerID]->alpha = alpha;
			}
			goto Finish;
		}

		overlay = (RenderOverlay *) calloc ( 1, sizeof (RenderOverlay) );
		if ( !overlay )
			return false;

		overlay->left = left;
		overlay->top = top;
		overlay->width = width;
		overlay->height = height;
		overlay->stride = stride;
		overlay->alpha = alpha;
		overlay->positionDevice = positionDevice;

		data = malloc ( height * stride );
		if ( !data ) {
			CErrArgID ( "setPortalOverlayARGB: Failed to allocate [%u bytes] for renderData.", height * stride );
			goto Failed;
		}

		memcpy ( data, renderData, height * stride );
		overlay->data = data;

		if ( !workerStages.capture || !workerStages.capture->AllocateOverlayBuffers ( overlay ) )
			goto Failed;

		if ( renderOverlays [ layerID ] )
			old = renderOverlays [ layerID ];

		if ( !LockAcquire ( &renderOverlayMutex, "setPortalOverlayARGB" ) )
			goto Finish;

		/// Add a sync with the capture/render stage here
		renderOverlays [layerID] = overlay;

		LockReleaseV ( &renderOverlayMutex, "setPortalOverlayARGB" );

	Finish:
		if ( old ) {
			if ( workerStages.capture )
				workerStages.capture->ReleaseOverlayBuffers ( old );

			if ( old->data )
				free ( old->data );
			free ( old );
		}

		return true;

Failed:
		if ( data )
			free ( data );
		if ( overlay )
			free ( overlay );

		return false;
	}
   

	void PortalGenerator::UpdatePortalSize ( int width_new, int height_new, bool updateAll )
	{
		CVerbArgID ( "UpdatePortalSize: width [%i] - height [%i]", width_new, height_new );

		if ( width_new % 2 )
			width_new++;

		if ( height_new % 2 )
			height_new++;

		if ( (width_new != renderDimensions.width_cap || height_new != renderDimensions.height_cap)
			&& (width_new > 0 && width_new < native.display.width && height_new > 0 && height_new < native.display.height) )
		{
			if ( LockAcquire ( &renderDimensionsMutex, "UpdatePortalSize" ) ) {
				// Calculate new left/top
				int diffWidth = (width_new - renderDimensions.width_cap) >> 1;
				int diffHeight = (height_new - renderDimensions.height_cap) >> 1;

				int left_new = renderDimensions.left - diffWidth;
				if ( left_new < 0 )
					left_new = 0;
				//			goto DontUpdate;

				int top_new = renderDimensions.top - diffHeight;
				if ( top_new < 0 )
					top_new = 0;
				//			goto DontUpdate;

				if ( left_new + width_new > native.display.width )
					width_new = native.display.width - left_new;

				if ( top_new + height_new > native.display.height )
					height_new = native.display.height - top_new;

				renderDimensions.left = left_new;
				renderDimensions.top = top_new;

				centerX = left_new + (width_new >> 1);
				centerY = top_new + (height_new >> 1);

				// TODO Check for aspect ratio!!!
				renderDimensions.width_cap = width_new;
				renderDimensions.height_cap = height_new;

				// Calculate capture settings
				renderDimensions.square = (int) sqrt ( (double) (width_new * width_new + height_new * height_new) );
				if ( renderDimensions.square % 2 )
					renderDimensions.square++;
				/*if (square % 32) {
				int mod = square % 32;
				square += 32 - mod;
				}*/

				UpdateRenderDimensions ();

				CVerbArgID ( "UpdatePortalSize: updating left [%i], top [%i], height [%i], width [%i], square [%i]", left_new, top_new, width_new, height_new, renderDimensions.square );

				LockReleaseV ( &renderDimensionsMutex, "UpdatePortalSize" );

				/// Force loop unrolling ?
				for ( int i=0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ )
					___sync_val_compare_and_swap ( &renderDimensionsChanged [i], 0, 1 );
			}

			if ( updateAll && env->desktopDrawRequested )
				UpdateDevicesCoverage ( parentDevice );

			PortalInfoBase info;
			info.portalID = portalID;

            if ( GetPortalInfo ( &info ) ) {
                onEnvironsNotifierContext1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_SIZE_CHANGED, portalID, &info, sizeof ( PortalInfoBase ) );
			}
		}
	}
	

	void PortalGenerator::UpdateDevicesCoverage ( void * deviceArg )
	{
		DeviceBase * deviceCallee = (DeviceBase *)deviceArg;
        
#ifdef DEBUGVERB
		int deviceID = deviceCallee->deviceID;
#endif
		CVerbID ( "UpdateDevicesCoverage" );

		int left	= DEFAULT_NO_PORTAL_POSITION_LEFT;
		int top		= DEFAULT_NO_PORTAL_POSITION_TOP;
		int width	= DEFAULT_NO_PORTAL_POSITION_WIDTH;
		int height	= DEFAULT_NO_PORTAL_POSITION_HEIGHT;

		bool init = true;

		Instance * env = deviceCallee->env;
		
		if ( !LockAcquire ( &devicesAccessMutex, "UpdateDevicesCoverage" ) )
			return;

        DeviceBase ** devices = GetDeviceMap ();
        int last = GetConnectedDevicesManagedLast ();
        
		for ( int index = 1; index <= last; index++ )
		{
			DeviceBase * device = devices [ index ];
            
			bool lock = (device != deviceCallee);

			if ( device && device->deviceStatus >= DeviceStatus::Connected )
			{
				if ( lock && !LockAcquire ( &device->portalMutex, "UpdateDevicesCoverage" ) )
					continue;

				for ( unsigned int id = 0; id < MAX_PORTAL_STREAMS_A_DEVICE; id++ ) 
				{
					if ( device->portalGenerators [id] ) {
						int left_cap = device->portalGenerators [id]->renderDimensions.left_cap;
						int top_cap = device->portalGenerators [id]->renderDimensions.top_cap;
						int square = device->portalGenerators [id]->renderDimensions.square;

						if ( init ) {
							left = left_cap; top = top_cap; width = square; height = square;
							init = false;
						}
						else {
							if ( left_cap < left ) {
								int width_pot = (left - left_cap);
								if ( left_cap + square > width_pot + width ) {
									width = square;
								}
								else {
									width += width_pot;
								}
								left = left_cap;
							}
							else {
								int width_pot = (left_cap - left);
								if ( width_pot + square > width ) {
									width = width_pot + square;
								}
							}

							if ( top_cap < top ) {
								int height_pot = (top - top_cap);
								if ( top_cap + square > height_pot + height ) {
									height = square;
								}
								else {
									height += height_pot;
								}
								top = top_cap;
							}
							else {
								int height_pot = (top_cap - top);
								if ( height_pot + square > height ) {
									height = height_pot + square;
								}
							}
						}
					}
				}

				if ( lock )
					LockReleaseV ( &device->portalMutex, "UpdateDevicesCoverage" );
			}
		}

		LockReleaseV ( &devicesAccessMutex, "UpdateDevicesCoverage" );


		if ( left < 0 )
			left = 0; 
		if ( top < 0 )
			top = 0; 
		if ( (left + width) > native.display.width )
			width = ( native.display.width - left);
		if ( (top + height) > native.display.height )
			height = ( native.display.height - top);

		env->desktopDrawLeft		= left;
		env->desktopDrawTop		= top;
		env->desktopDrawWidth	= width;
		env->desktopDrawHeight	= height;

	/*	desktopDrawLeft		= 0;
		desktopDrawTop		= 0;
		desktopDrawWidth	= environs::device_width;
		desktopDrawHeight	= environs::device_height;*/
	}
	
	
	inline void PortalGenerator::UpdateRenderDimensions ()
	{
		CVerbID ( "UpdateRenderDimensions" );

		unsigned int xOffset = (renderDimensions.square - renderDimensions.width_cap) >> 1;
		renderDimensions.xOffset = xOffset;
		renderDimensions.left_cap = renderDimensions.left - xOffset + native.device_left;

		unsigned int yOffset = (renderDimensions.square - renderDimensions.height_cap) >> 1;
		renderDimensions.yOffset = yOffset;
		renderDimensions.top_cap = renderDimensions.top - yOffset + native.device_top;
	}


	void PortalGenerator::UpdatePosition ( int x, int y, float angle, bool updateAll )
	{
		CVerbArgID ( "UpdatePosition: x [%i] - y [%i] - a [%f]", x, y, angle );

		bool updatePos = (centerX != x || centerY != y);
		bool updateAngle = (renderDimensions.orientation != angle);

		if ( !updatePos && !updateAngle )
			return;

		if ( LockAcquire ( &renderDimensionsMutex, "UpdatePosition" ) ) {
			if ( updatePos )
			{
				int width_half = renderDimensions.width_cap >> 1;
				int height_half = renderDimensions.height_cap >> 1;

				// We check for constraints, but "help" with reasonable adapting in case of invalid values

				// Check for left constraint
				if ( x - width_half < 0 ) {
					CVerbArgID ( "UpdatePosition: |<< forcing x [%i] to be width_half [%i]", x, width_half );
					x = width_half;
				}
				else
					if ( x + width_half > native.display.width ) {
						CVerbArgID ( "UpdatePosition: >>| forcing x [%i] to be max_width_half [%i]", x, native.display.width - width_half );
						x = native.display.width - width_half;
					}

				// Check for top constraint
				if ( y - height_half < 0 ) {
					CVerbArgID ( "UpdatePosition: |<< forcing y [%i] to be height_half [%i]", y, height_half );
					y = height_half;
				}
				else
					if ( y + height_half > native.display.height ) {
						CVerbArgID ( "UpdatePosition: >>| forcing y [%i] to be max_height_half [%i]", y, native.display.height - height_half );
						y = native.display.height - height_half;
					}

				centerX = x;
				centerY = y;

				renderDimensions.left = x - width_half;
				renderDimensions.top = y - height_half;
				CVerbArgID ( "UpdatePosition: left [%i] top [%i]", renderDimensions.left, renderDimensions.top );
			}

			if ( updateAngle )
			{
				renderDimensions.orientation = angle;
				deviceAzimutLast = deviceAzimut;
				CVerbArgID ( "UpdatePosition: angle [%f] ", angle );
			}

			UpdateRenderDimensions ();

			LockReleaseV ( &renderDimensionsMutex, "UpdatePosition" );
		}

		if ( updatePos || updateAngle ) 
		{
			for ( int i=0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ )
				___sync_val_compare_and_swap ( &renderDimensionsChanged [i], 0, 1 );

			if ( updateAll && env->desktopDrawRequested )
				UpdateDevicesCoverage ( parentDevice );
			
			PortalInfoBase info;
            if ( GetPortalInfo ( &info ) )
                onEnvironsNotifierContext1 ( env, parentDevice->nativeID, NOTIFY_PORTAL_LOCATION_CHANGED, portalID, &info, sizeof ( PortalInfoBase ) );
		}
	}


	bool PortalGenerator::GetPortalInfo ( PortalInfoBase * info )
	{
		CVerbID ( "GetPortalInfo" );

		if ( !info )
			return false;

		if ( !LockAcquire ( &renderDimensionsMutex, "GetPortalInfo" ) )
			return false;

		info->portalID	= portalID;
		info->flags 	= 0xF;
		info->centerX	= centerX;
		info->centerY	= centerY;
		info->width		= renderDimensions.width_cap;
		info->height	= renderDimensions.height_cap;
		info->orientation = renderDimensions.orientation;


		if ( !LockRelease ( &renderDimensionsMutex, "GetPortalInfo" ) )
			return false;
		return true;
	}


	int PortalGenerator::GetPortalInfoIfChanged ( PortalInfoBase * info )
	{
		CVerbID ( "GetPortalInfoIfChanged" );

		if ( !info )
			return 1;

		if ( portalInfosCache.centerX == centerX && portalInfosCache.centerY == centerY && portalInfosCache.orientation == renderDimensions.orientation 
			&& portalInfosCache.width == renderDimensions.width_cap && portalInfosCache.height == renderDimensions.height_cap )
			return 1;

		if ( !LockAcquire ( &renderDimensionsMutex, "GetPortalInfoIfChanged" ) )
			return false;

		portalInfosCache.centerX = centerX;
		portalInfosCache.centerY = centerY;
		portalInfosCache.orientation = renderDimensions.orientation;
		portalInfosCache.width = renderDimensions.width_cap;
		portalInfosCache.height = renderDimensions.height_cap;

		if ( !LockRelease ( &renderDimensionsMutex, "GetPortalInfoIfChanged" ) )
			return false;

		info->portalID	= portalID;
		info->flags 	= 0xF;

		memcpy ( &info->centerX, &portalInfosCache.centerX, sizeof(int) * 5 );

		return 2;
	}
    
#ifdef ENABLE_PORTAL_STALL_MECHS
    void  PortalGenerator::Stall ( )
    {
        CVerbID ( "Stall" );
        
        stalled = true;
        stalledCounter = 0;
        if ( workerStages.encoder )
            workerStages.encoder->iFrameFPSMode = true;
    }
    
    
    void  PortalGenerator::UnStall ( )
    {
        CVerbID ( "UnStall" );
        
        stalled = false;
        stalledCounter = 0;
        if ( workerStages.encoder )
            workerStages.encoder->iFrameFPSMode = false;
    }
#endif

	void * PortalGenerator::Thread_WorkerStarter ( void * object )
	{
		PortalGenerator * device = (PortalGenerator*)object;

		// Execute thread
		device->Thread_Worker ( );

		return 0;
	}


#ifdef ENABLE_IMPROVED_PORTAL_GENERATOR
    
    void PortalGenerator::Thread_Worker ()
    {
        unsigned int workerIndex = renderContextNext;
        
        CLogArgID ( "Worker [%d]: created.", workerIndex );
        
        pthread_setname_current_envthread ( "PortalGenerator::Worker" );
        
        
        unsigned int        contIndex = 0;
        
        IPortalRenderer *	renderer;
        RenderContext	*	context, *	contextPrev;
        bool				success;
        RenderDimensions	renderDims = renderDimensions;
        
        IPortalCapture	*	capture = 0;
        IPortalEncoder	*	encoder = 0;
        
#ifdef ENABLE_WORKER_STAGES_COMPARE
        unsigned int		equal;
#endif
        char			*	sendBuffer;
        unsigned int		payloadSize;
        //unsigned short		payloadType = 0; // (unsigned short) workerStages.encoder->type; // (unsigned short) streamOptions->streamType; // streamOptions->useStream ? DATA_STREAM_H264_NALS : DATA_STREAM_IMAGE_ARBITRARY;
        int					retCode;
        
#ifdef ENABLE_WORKER_STAGES_LOCKS
        bool                renderLocked;
        bool                encoderLocked;
        bool                sendLocked;
        bool                renderLockedPrev;
        bool                encoderLockedPrev;
        bool                sendLockedPrev;
#endif
        pc_initPerformanceCounter ( );
        
        if ( pthread_cond_signal ( &workerStateEvent ) ) {
            CErrArgID ( "Worker [%u]: Failed to signal workerStateEvent!", workerIndex );
        }
        
        while ( status > PortalSourceStatus::Deleteable )
        {
            /// Acquire the event lock
            pthread_cond_mutex_lock ( & portalWorkerEventLock );
            
            if ( pthread_cond_wait_time ( & portalWorkerEvent, & portalWorkerEventLock, INFINITE ) ) {
                CWarnArgID ( "Worker [%u]: failed waiting for event!", workerIndex );
                break;
            }
            
            /// Release the event lock
            pthread_cond_mutex_unlock ( & portalWorkerEventLock );
            
            CVerbVerbArgID ( "Worker: update [%i]", workerIndex );
            
            if ( status < PortalSourceStatus::Active )
                continue;
            
            /**************************************************************************************
             * Acquire a render context to work on
             * *************************************************************************************/
            
            CVerbVerbArgID ( "Worker [%u]: Context [%u] is next...", workerIndex, renderContextNext );
            contIndex = renderContextNext;
            
            context = renderContexts + contIndex;

            if ( ___sync_val_compare_and_swap ( &context->hasContent, 0, 1 ) == 1 ) {
                continue;
            }
            
            CVerbVerbArgID ( "Worker [%u]: Context [%u] is now taken over..", workerIndex, renderContextNext );
            
			if ( contIndex + 1 >= MAX_PORTAL_CONTEXT_WORKERS )
                renderContextNext = 0;
            else
                renderContextNext = contIndex + 1;
                        
			contextPrev = !contIndex ? (context + (MAX_PORTAL_CONTEXT_WORKERS - 1)) : (context - 1);
            
            success = false;
            
            /**************************************************************************************
             * Wait until the Renderer is "UnLocked" by the previous Worker thread
             * *************************************************************************************/
#ifdef ENABLE_WORKER_STAGES_LOCKS
            renderLocked        = true;
            encoderLocked       = true;
            sendLocked          = true;
            
            renderLockedPrev	= false;
            encoderLockedPrev	= false;
            sendLockedPrev		= false;
            
            CVerbVerbArgID ( "Worker [%u]: Context [%u] acquiring previous renderLock...", workerIndex, contIndex );
            if ( !env_sem_wait ( contextPrev->renderSem ) ) {
                CErrArgID ( "Worker [%u]: Failed to wait for previous rendered event for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            renderLockedPrev	= true;
#else
            CVerbVerbArgID ( "Worker [%u]: Context [%u] waiting for previous eventRendered...", workerIndex, contIndex );
            if ( pthread_cond_wait_time ( &contextPrev->eventRendered, 0, INFINITE ) ) {
                CErrArgID ( "Worker [%u]: Failed to wait for rendered event for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            /// Prepare the "Lock" of the previous Renderer for the next turn
            CVerbVerbArgID ( "Worker [%u]: Context [%u] reseting previous eventRendered...", workerIndex, contIndex );
            pthread_cond_prepare ( &contextPrev->eventRendered );
            // Prepare the "Lock" of the previous Encoder for the next turn
#endif
            if ( status < PortalSourceStatus::Active )
                goto Next;
            
            if ( ___sync_val_compare_and_swap ( &renderDimensionsChanged [workerIndex], 1, 0 ) )
            {
                //**************************************************************************************
                //* Load the current render context (portal location / state)
                //* ************************************************************************************
                if ( pthread_mutex_lock ( &renderDimensionsMutex ) ) {
                    CErrArgID ( "Worker [%u]: Failed to lock renderDimensionsMutex.", workerIndex );
                    break;
                }
                
                memcpy ( &renderDims, &this->renderDimensions, sizeof ( renderDims ) );
                //			renderDims = this->renderDimensions;	// should be faster than a memcpy system call
                
                if ( pthread_mutex_unlock ( &renderDimensionsMutex ) ) {
                    CErrArgID ( "Worker [%u]: Failed to unlock renderDimensionsMutex.", workerIndex );
                    break;
                }
            }
            
            
            /**************************************************************************************
             * Stage: Capturing / Grabbing
             * *************************************************************************************/
            capture = workerStages.capture;
            
            if ( capture->squareLength != renderDims.square )
                capture->ReleaseResources ();
            
            if ( !capture->buffersInitialized ) {
                if ( capture->AllocateResources ( &renderDims ) <= 0 ) {
                    CVerbArgID ( "Worker [%u]: Context [%u]. Capture plugin [%s] failed to allocate buffers.", workerIndex, contIndex, capture->name );
                    goto Next;
                }
                capture->buffersInitialized = true;
                capture->squareLength = renderDims.square;
            }
            
            if ( capture->Perform ( &renderDims ) <= 0 ) {
                CErrArgID ( "Worker [%u]: Context [%u]. Capture plugin [%s] failed, returning an recoverable status.", workerIndex, contIndex, capture->name );
                goto Next;
            }
            context->hasChanged = true;
            
            
            /**************************************************************************************
             * Stage: Rendering
             * *************************************************************************************/
            pc_MeasureInit ( context );
            
            renderer = workerStages.render;
            if ( renderer )
            {
                if ( renderer->squareLength != renderDims.square || !context->renderedData ) {
                    if ( renderer->UpdateBuffers ( &renderDims, context ) <= 0 ) {
                        CVerbArgID ( "Worker [%u]: Context [%u]. Render plugin failed to update buffers.", workerIndex, contIndex );
                        goto Next;
                    }
                    
                    renderer->buffersInitialized	= true;
                    renderer->squareLength			= renderDims.square;
                    
                    context->renderedDataType		= renderer->outputType;
                }
                
#ifdef ENABLE_WORKER_STAGES_COMPARE
                /// Compare the context with the previous one
                if ( renderer->Compare ( equal ) )
                {
                    context->hasChanged = true;
                    if ( equal ) {
                        if ( opt_useRenderCache ) {
                            filledContexts++;
							if ( filledContexts > MAX_PORTAL_CONTEXT_WORKERS ) {
                                context->hasChanged = false;
                            }
                        }
                    }
                    else
                        filledContexts = 0;
                }
                else {
                    success = false;
                    goto Next;
                }
                
                /// Render the context
                if ( context->hasChanged )
#endif
                    success = renderer->Perform ( &renderDims, context );
                
                pc_MeasureRender ( context );
                
                if ( !success ) {
                    CErrArgID ( "Worker [%u]: Context [%u]. Failed to render [%s] portal!", workerIndex, contIndex, renderer->name );
                    context->hasContent = false;
                    goto Next;
                }
                
                /// "Unlock" the Renderer
#ifdef ENABLE_WORKER_STAGES_LOCKS
                CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking renderLock...", workerIndex, contIndex );
                if ( !env_sem_post ( context->renderSem ) ) {
                    CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
                    goto Next;
                }
                renderLocked	= false;
#else
                CVerbVerbArgID ( "Worker [%u]: Context [%u] signaling for eventRendered...", workerIndex, contIndex );
                pthread_cond_signal ( &context->eventRendered );
#endif
            }
            if ( status < PortalSourceStatus::Active )
                goto Next;
            
            /**************************************************************************************
             * Stage: Encoding
             * *************************************************************************************/
#ifdef ENABLE_WORKER_STAGES_LOCKS
            CVerbVerbArgID ( "Worker [%u]: Context [%u] locking previous encoderLock...", workerIndex, contIndex );
            if ( !env_sem_wait ( contextPrev->encoderSem ) ) {
                CErrArgID ( "Worker [%u]: Failed to wait for previous encoder event for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            encoderLockedPrev	= true;
#else
            CVerbVerbArgID ( "Worker [%u]: Context [%u] waiting for previous eventEncoded...", workerIndex, contIndex );
            if ( pthread_cond_wait_time ( &contextPrev->eventEncoded, 0, INFINITE ) ) {
                CErrArgID ( "Worker [%u]: Failed to wait for encoded event at pack [%d]!", workerIndex, contIndex );
                break;
            }
            // Prepare the "Lock" of the previous Encoder for the next turn
            CVerbVerbArgID ( "Worker [%u]: Context [%u] reseting previous eventEncoded...", workerIndex, contIndex );
            pthread_cond_prepare ( &contextPrev->eventEncoded );
#endif
            if ( status < PortalSourceStatus::Active )
                goto Next;
            
            encoder = workerStages.encoder;
            
            retCode = encoder->Perform ( context );
            
            pc_MeasureEncode ( context );
            
            /// "Unlock" the Capturer (Renderer) if we have no renderer
            if ( !renderer ) {
#ifdef ENABLE_WORKER_STAGES_LOCKS
                CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking renderLock...", workerIndex, contIndex );
                if ( !env_sem_post ( context->renderSem ) ) {
                    CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
                    goto Next;
                }
                renderLocked	= false;
#else
                CVerbArgID ( "Worker [%u]: Context [%u] signaling for eventRendered...", workerIndex, contIndex );
                pthread_cond_signal ( &context->eventRendered );
#endif
            }
            
            /// Encoder is free for use within the next worker thread
#ifdef ENABLE_WORKER_STAGES_LOCKS
            CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking encoderLock...", workerIndex, contIndex );
            if ( !env_sem_post ( context->encoderSem ) ) {
                CErrArgID ( "Worker [%u]: Failed to release encoder lock for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            encoderLocked	= false;
#else
            CVerbVerbArgID ( "Worker [%u]: Context [%u] signaling eventEncoded...", workerIndex, contIndex );
            pthread_cond_signal ( &context->eventEncoded );
#endif
            if ( retCode < 0 ) {
                CErrArgID ( "Worker [%u]: Failed to encode render context [%i]", workerIndex, contIndex );
                goto Next;
            }
            
            if ( retCode == 0 ) {
                CVerbVerbArgID ( "Worker [%u]: Encoder plugin returned 0.", workerIndex );
                goto Next;
            }
            
            /**************************************************************************************
             * Stage: Sending
             * *************************************************************************************/
            sendBuffer = context->outputBuffer;
            if ( !sendBuffer ) {
                CVerbArgID ( "Worker [%u]: Invalid output buffer from encoder", workerIndex );
                goto Next;
            }
            
            payloadSize = *(((unsigned int *)sendBuffer) + 1);
            if ( !payloadSize ) {
                CVerbArgID ( "Worker [%u]: Invalid output buffer size from encoder", workerIndex );
                goto Next;
            }
            
            //CLogArgID ( "Worker [%d]: sending [%i]", workerIndex, contIndex );
            
#ifdef ENABLE_WORKER_STAGES_LOCKS
            CVerbVerbArgID ( "Worker [%u]: Context [%u] locking previous sendLock...", workerIndex, contIndex );
            if ( !env_sem_wait ( contextPrev->sendSem ) ) {
                CErrArgID ( "Worker [%u]: Failed to wait for previous send event for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            sendLockedPrev		= true;
#endif
            if ( streamOptions->streamOverCom )
                parentDevice->SendTcpPortal ( MSG_TYPE_STREAM, (unsigned short) streamOptions->streamType, portalID, payloadSize, sendBuffer + 8 );
            else
                parentDevice->SendUdpPortal ( (unsigned short) streamOptions->streamType, portalID, payloadSize, sendBuffer + 8 );
            
#ifdef ENABLE_WORKER_STAGES_LOCKS
            CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking sendLock...", workerIndex, contIndex );
            if ( !env_sem_post ( context->sendSem ) ) {
                CErrArgID ( "Worker [%u]: Failed to release send lock for context [%d]!", workerIndex, contIndex );
                goto Next;
            }
            sendLocked		= false;
#endif
            pc_MeasureDone ( context );
            
            
        Next:
#ifdef ENABLE_WORKER_STAGES_LOCKS
            if ( status > PortalSourceStatus::Deleteable )
            {
                if ( !renderLockedPrev ) {
                    CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous renderLock...", workerIndex, contIndex );
                    if ( !env_sem_wait ( contextPrev->renderSem ) ) {
                        CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous render event for context [%d]!", workerIndex, contIndex );
                    }
                }
                
                if ( !encoderLockedPrev ) {
                    CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous encoderLock...", workerIndex, contIndex );
                    if ( !env_sem_wait ( contextPrev->encoderSem ) ) {
                        CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous encoder event for context [%d]!", workerIndex, contIndex );
                    }
                }
                
                if ( !sendLockedPrev ) {
                    CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous sendLock...", workerIndex, contIndex );
                    if ( !env_sem_wait ( contextPrev->sendSem ) ) {
                        CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous send event for context [%d]!", workerIndex, contIndex );
                    }
                }
            }
            
            if ( renderLocked ) {
                CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking renderLock...", workerIndex, contIndex );
                if ( !env_sem_post ( context->renderSem ) ) {
                    CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
                }
            }
            
            if ( encoderLocked ) {
                CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking encoderLock...", workerIndex, contIndex );
                if ( !env_sem_post ( context->encoderSem ) ) {
                    CErrArgID ( "Worker [%u]: Failed to release encoder lock for context [%d]!", workerIndex, contIndex );
                }
            }
            
            if ( sendLocked ) {
                CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking sendLock...", workerIndex, contIndex );
                if ( !env_sem_post ( context->sendSem ) ) {
                    CErrArgID ( "Worker [%u]: Failed to release send lock for context [%d]!", workerIndex, contIndex );
                }
            }
#else
            if ( !success ) {
                /// Release our "Locks" (even twice is safe)
                CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL signaling eventRendered...", workerIndex, contIndex );
                pthread_cond_signal ( &context->eventRendered );
                CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL signaling eventEncoded...", workerIndex, contIndex );
                pthread_cond_signal ( &context->eventEncoded );
            }
#endif
            
            // Mark portal pack as free
            if ( ___sync_val_compare_and_swap ( &context->hasContent, 1, 0 ) != 1 ) {
                CErrArgID ( "Worker [%u]: Invalid render context state! Could not release the context.", workerIndex );
                break;
            }
        }
        
#ifdef ENABLE_WORKER_STAGES_LOCKS
        /// Signal next worker thread to terminate
        if ( pthread_cond_signal ( &portalWorkerEvent ) ) {
            CErrArgID ( "Worker [%u]: Failed to signal workerStateEvent!", workerIndex );
        }
#endif
        CLogArgID ( "Worker [%u]: bye bye... Last context was [%u]", workerIndex, contIndex );
        
        return;
    }
    
#else
	void PortalGenerator::Thread_Worker ()
	{
		unsigned int workerIndex = renderContextNext;

		CLogArgID ( "Worker [%d]: created.", workerIndex );

		pthread_setname_current_envthread ( "PortalGenerator::Worker" );


		unsigned int        contIndex = 0;

		IPortalRenderer *	renderer;
		RenderContext	*	context, *	contextPrev;
		bool				success;
		RenderDimensions	renderDims = renderDimensions;

		IPortalCapture	*	capture = 0;
		IPortalEncoder	*	encoder = 0;

#ifdef ENABLE_WORKER_STAGES_COMPARE
		unsigned int		equal;
#endif
		char			*	sendBuffer;
		int					payloadSize;
		//unsigned short		payloadType = 0; // (unsigned short) workerStages.encoder->type; // (unsigned short) streamOptions->streamType; // streamOptions->useStream ? DATA_STREAM_H264_NALS : DATA_STREAM_IMAGE_ARBITRARY;
		int					retCode;

#ifdef ENABLE_WORKER_STAGES_LOCKS
		bool                renderLocked;
		bool                encoderLocked;
		bool                sendLocked;
		bool                renderLockedPrev;
		bool                encoderLockedPrev;
		bool                sendLockedPrev;
#endif
		pc_initPerformanceCounter ( );

		if ( pthread_cond_signal ( &workerStateEvent ) ) {
			CErrArgID ( "Worker [%u]: Failed to signal workerStateEvent!", workerIndex );
		}

		while ( status > PortalSourceStatus::Deleteable )
        {
            bool skip = false;
            
			/// Acquire the event lock
			pthread_cond_mutex_lock ( & portalWorkerEventLock );

			if ( pthread_cond_wait_time ( & portalWorkerEvent, & portalWorkerEventLock, INFINITE ) ) {
				CWarnArgID ( "Worker [%u]: failed waiting for event!", workerIndex );
				break;
			}
#ifdef ENABLE_PORTAL_STALL_MECHS
            if ( stalled ) {
                if ( pthread_mutex_lock ( &renderContextMutex ) ) {
                    CErrArgID ( "Worker [%u]: Failed to lock renderContextMutex.", workerIndex );
                    break;
                }
                
                stalledCounter += 10;
                if ( stalledCounter >= env->streamFPS ) {
                    stalledCounter = 0;
                }
                else skip = true;
                
                if ( pthread_mutex_lock ( &renderContextMutex ) ) {
                    CErrArgID ( "Worker [%u]: Failed to lock renderContextMutex.", workerIndex );
                    break;
                }
            }
#endif

			/// Release the event lock
			pthread_cond_mutex_unlock ( & portalWorkerEventLock );

			CVerbVerbArgID ( "Worker: update [%i]", workerIndex );

			if ( skip || status < PortalSourceStatus::Active )
				continue;
            
			/**************************************************************************************
             * Acquire a render context for this round
             * *************************************************************************************/
			// Acquire access to portal packs
			if ( pthread_mutex_lock ( &renderContextMutex ) ) {
				CErrArgID ( "Worker [%u]: Failed to lock renderContextMutex.", workerIndex );
				break;
			}

			CVerbVerbArgID ( "Worker [%u]: Context next [%u] ...", workerIndex, renderContextNext );
			contIndex = renderContextNext;

			context = renderContexts + contIndex;
			if ( context->hasContent ) {
				pthread_mutex_unlock ( &renderContextMutex );
				continue;
			}
			CVerbVerbArgID ( "Worker [%u]: Context [%u] is now taken over..", workerIndex, renderContextNext );
			context->hasContent = true;

#ifdef ENABLE_PORTAL_STALL_MECHS
			if ( stalled && workerStages.encoder )
				workerStages.encoder->iFrameRequest = true;
#endif
			renderContextNext++;
			if ( renderContextNext >= MAX_PORTAL_CONTEXT_WORKERS )
				renderContextNext = 0;

			// Release access to portal packs
			if ( pthread_mutex_unlock ( &renderContextMutex ) ) {
				CErrArgID ( "Worker [%u]: Failed to unlock renderContextMutex.", workerIndex );
				break;
			}

			contextPrev = !contIndex ? (context + (MAX_PORTAL_CONTEXT_WORKERS - 1)) : (context - 1);

			success = false;

			/**************************************************************************************
			* Wait until the Renderer is "UnLocked" by the previous Worker thread
			* *************************************************************************************/
#ifdef ENABLE_WORKER_STAGES_LOCKS
			renderLocked        = true;
			encoderLocked       = true;
			sendLocked          = true;

			renderLockedPrev	= false;
			encoderLockedPrev	= false;
			sendLockedPrev		= false;

			CVerbVerbArgID ( "Worker [%u]: Context [%u] acquiring previous renderLock...", workerIndex, contIndex );
			if ( !env_sem_wait ( contextPrev->renderSem ) ) {
				CErrArgID ( "Worker [%u]: Failed to wait for previous rendered event for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			renderLockedPrev	= true;
#else
			CVerbVerbArgID ( "Worker [%u]: Context [%u] waiting for previous eventRendered...", workerIndex, contIndex );
			if ( pthread_cond_wait_time ( &contextPrev->eventRendered, 0, INFINITE ) ) {
				CErrArgID ( "Worker [%u]: Failed to wait for rendered event for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			/// Prepare the "Lock" of the previous Renderer for the next turn
			CVerbVerbArgID ( "Worker [%u]: Context [%u] reseting previous eventRendered...", workerIndex, contIndex );
			pthread_cond_prepare ( &contextPrev->eventRendered );
			// Prepare the "Lock" of the previous Encoder for the next turn
#endif
			if ( status < PortalSourceStatus::Active )
				goto Next;

			if ( ___sync_val_compare_and_swap ( &renderDimensionsChanged [workerIndex], 1, 0 ) )
			{
				//**************************************************************************************
				//* Load the current render context (portal location / state)
				//* ************************************************************************************
				if ( pthread_mutex_lock ( &renderDimensionsMutex ) ) {
					CErrArgID ( "Worker [%u]: Failed to lock renderDimensionsMutex.", workerIndex );
					break;
				}

				memcpy ( &renderDims, &this->renderDimensions, sizeof ( renderDims ) );
				//			renderDims = this->renderDimensions;	// should be faster than a memcpy system call

				if ( pthread_mutex_unlock ( &renderDimensionsMutex ) ) {
					CErrArgID ( "Worker [%u]: Failed to unlock renderDimensionsMutex.", workerIndex );
					break;
				}
			}


			/**************************************************************************************
			* Stage: Capturing / Grabbing
			* *************************************************************************************/
			capture = workerStages.capture;

			if ( capture->squareLength != renderDims.square )
				capture->ReleaseResources ();

			if ( !capture->buffersInitialized ) {
				if ( capture->AllocateResources ( &renderDims ) <= 0 ) {
                    CVerbArgID ( "Worker [%u]: Context [%u]. Capture plugin [%s] failed to allocate buffers.", workerIndex, contIndex, capture->name );
					goto Next;
                }
				capture->buffersInitialized = true;
				capture->squareLength = renderDims.square;
			}

			retCode = capture->Perform ( &renderDims, context );
			if ( retCode <= 0 ) {
				if ( retCode < 0 )
					CErrArgID ( "Worker [%u]: Context [%u]. Capture plugin [%s] failed, returning an recoverable status.", workerIndex, contIndex, capture->name );
				goto Next;
			}
			context->hasChanged = true;


			/**************************************************************************************
			* Stage: Rendering
			* *************************************************************************************/
			pc_MeasureInit ( context );

			renderer = workerStages.render;
			if ( renderer )
			{
				if ( renderer->squareLength != renderDims.square || !context->renderedData ) {
					if ( renderer->UpdateBuffers ( &renderDims, context ) <= 0 ) {
                        CVerbArgID ( "Worker [%u]: Context [%u]. Render plugin failed to update buffers.", workerIndex, contIndex );
						goto Next;
                    }

					renderer->buffersInitialized	= true;
					renderer->squareLength			= renderDims.square;

					context->renderedDataType		= renderer->outputType;
				}

#ifdef ENABLE_WORKER_STAGES_COMPARE
				/// Compare the context with the previous one
				if ( renderer->Compare ( equal ) ) 
				{
					context->hasChanged = true;
					if ( equal ) {
						if ( opt_useRenderCache ) {
							filledContexts++;
							if ( filledContexts > MAX_PORTAL_CONTEXT_WORKERS ) {
								context->hasChanged = false;
							}
						}
					}
					else
						filledContexts = 0;
				}
				else {
					success = false;
					goto Next;
				}

				/// Render the context
				if ( context->hasChanged )
#endif
					success = renderer->Perform ( &renderDims, context );

				pc_MeasureRender ( context );

				if ( !success ) {
					CErrArgID ( "Worker [%u]: Context [%u]. Failed to render [%s] portal!", workerIndex, contIndex, renderer->name );
					context->hasContent = false;
					goto Next;
				}

				/// "Unlock" the Renderer
#ifdef ENABLE_WORKER_STAGES_LOCKS
				CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking renderLock...", workerIndex, contIndex );
				if ( !env_sem_post ( context->renderSem ) ) {
					CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
					goto Next;
				}
				renderLocked	= false;
#else
				CVerbVerbArgID ( "Worker [%u]: Context [%u] signaling for eventRendered...", workerIndex, contIndex );
				pthread_cond_signal ( &context->eventRendered );
#endif
			}
			if ( status < PortalSourceStatus::Active )
				goto Next;

			/**************************************************************************************
			* Stage: Encoding
			* *************************************************************************************/
#ifdef ENABLE_WORKER_STAGES_LOCKS
			CVerbVerbArgID ( "Worker [%u]: Context [%u] locking previous encoderLock...", workerIndex, contIndex );
			if ( !env_sem_wait ( contextPrev->encoderSem ) ) {
				CErrArgID ( "Worker [%u]: Failed to wait for previous encoder event for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			encoderLockedPrev	= true;
#else
			CVerbVerbArgID ( "Worker [%u]: Context [%u] waiting for previous eventEncoded...", workerIndex, contIndex );
			if ( pthread_cond_wait_time ( &contextPrev->eventEncoded, 0, INFINITE ) ) {
				CErrArgID ( "Worker [%u]: Failed to wait for encoded event at pack [%d]!", workerIndex, contIndex );
				break;
			}
			// Prepare the "Lock" of the previous Encoder for the next turn
			CVerbVerbArgID ( "Worker [%u]: Context [%u] reseting previous eventEncoded...", workerIndex, contIndex );
			pthread_cond_prepare ( &contextPrev->eventEncoded );
#endif
			if ( status < PortalSourceStatus::Active )
				goto Next;

			encoder = workerStages.encoder;

			retCode = encoder->Perform ( context );

			pc_MeasureEncode ( context );

			/// "Unlock" the Capturer (Renderer) if we have no renderer
			if ( !renderer ) {
				___sync_val_compare_and_swap ( &capture->dataAccessed, 1, 2 );

#ifdef ENABLE_WORKER_STAGES_LOCKS
				CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking renderLock...", workerIndex, contIndex );
				if ( !env_sem_post ( context->renderSem ) ) {
					CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
					goto Next;
				}
				renderLocked	= false;
#else
				CVerbArgID ( "Worker [%u]: Context [%u] signaling for eventRendered...", workerIndex, contIndex );
				pthread_cond_signal ( &context->eventRendered );
#endif
			}

			/// Encoder is free for use within the next worker thread
#ifdef ENABLE_WORKER_STAGES_LOCKS
			CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking encoderLock...", workerIndex, contIndex );
			if ( !env_sem_post ( context->encoderSem ) ) {
				CErrArgID ( "Worker [%u]: Failed to release encoder lock for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			encoderLocked	= false;
#else
			CVerbVerbArgID ( "Worker [%u]: Context [%u] signaling eventEncoded...", workerIndex, contIndex );
			pthread_cond_signal ( &context->eventEncoded );
#endif
			if ( retCode < 0 ) {
				CErrArgID ( "Worker [%u]: Failed to encode render context [%i]", workerIndex, contIndex );
				goto Next;
			}

			if ( retCode == 0 ) {
				CVerbVerbArgID ( "Worker [%u]: Encoder plugin returned 0.", workerIndex );
				goto Next;
            }

			/**************************************************************************************
			* Stage: Sending
			* *************************************************************************************/
			if ( !context->outputBuffer ) {
                CVerbArgID ( "Worker [%u]: Invalid output buffer from encoder", workerIndex );
				goto Next;
			}
			sendBuffer = BYTEBUFFER_DATA_POINTER_START ( context->outputBuffer );

			payloadSize = context->outputBuffer->payloadSize;
			if ( !payloadSize ) {
                CVerbArgID ( "Worker [%u]: Invalid output buffer size from encoder", workerIndex );
				goto Next;
            }

			//CLogArgID ( "Worker [%d]: sending [%i]", workerIndex, contIndex );

#ifdef ENABLE_WORKER_STAGES_LOCKS
			CVerbVerbArgID ( "Worker [%u]: Context [%u] locking previous sendLock...", workerIndex, contIndex );
			if ( !env_sem_wait ( contextPrev->sendSem ) ) {
				CErrArgID ( "Worker [%u]: Failed to wait for previous send event for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			sendLockedPrev		= true;
#endif
			if ( payloadSize > 0 ) {
                unsigned short flags = context->isIFrame ? DATA_STREAM_IFRAME : 0;
                
				if ( streamOptions->streamOverCom )
					parentDevice->SendTcpPortal ( (unsigned short) streamOptions->streamType | flags, portalID, context->frameCounter, 0, 0, sendBuffer, payloadSize );
				else
					parentDevice->SendUdpPortal ( (unsigned short) streamOptions->streamType | flags, portalID, context->frameCounter, 0, 0, sendBuffer, payloadSize );
			}
            
#ifdef ENABLE_WORKER_STAGES_LOCKS
			CVerbVerbArgID ( "Worker [%u]: Context [%u] unlocking sendLock...", workerIndex, contIndex );
			if ( !env_sem_post ( context->sendSem ) ) {
				CErrArgID ( "Worker [%u]: Failed to release send lock for context [%d]!", workerIndex, contIndex );
				goto Next;
			}
			sendLocked		= false;
#endif
			pc_MeasureDone ( context );

			
		Next:
#ifdef ENABLE_WORKER_STAGES_LOCKS
			if ( status > PortalSourceStatus::Deleteable )
			{
				if ( !renderLockedPrev ) {
					CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous renderLock...", workerIndex, contIndex );
					if ( !env_sem_wait ( contextPrev->renderSem ) ) {
						CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous render event for context [%d]!", workerIndex, contIndex );
					}
				}

				if ( !encoderLockedPrev ) {
					CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous encoderLock...", workerIndex, contIndex );
					if ( !env_sem_wait ( contextPrev->encoderSem ) ) {
						CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous encoder event for context [%d]!", workerIndex, contIndex );
					}
				}

				if ( !sendLockedPrev ) {
					CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL locking previous sendLock...", workerIndex, contIndex );
					if ( !env_sem_wait ( contextPrev->sendSem ) ) {
						CErrArgID ( "Worker [%u]: Failed to FINAL wait for previous send event for context [%d]!", workerIndex, contIndex );
					}
				}
			}

			if ( renderLocked ) {
				CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking renderLock...", workerIndex, contIndex );
				if ( !env_sem_post ( context->renderSem ) ) {
					CErrArgID ( "Worker [%u]: Failed to release render lock for context [%d]!", workerIndex, contIndex );
				}
			}
            
			if ( encoderLocked ) {
				CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking encoderLock...", workerIndex, contIndex );
				if ( !env_sem_post ( context->encoderSem ) ) {
					CErrArgID ( "Worker [%u]: Failed to release encoder lock for context [%d]!", workerIndex, contIndex );
				}
			}
            
			if ( sendLocked ) {
				CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL unlocking sendLock...", workerIndex, contIndex );
				if ( !env_sem_post ( context->sendSem ) ) {
					CErrArgID ( "Worker [%u]: Failed to release send lock for context [%d]!", workerIndex, contIndex );
				}
			}
#else
			if ( !success ) {
				/// Release our "Locks" (even twice is safe)
				CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL signaling eventRendered...", workerIndex, contIndex );
				pthread_cond_signal ( &context->eventRendered );
				CVerbVerbArgID ( "Worker [%u]: Context [%u] FINAL signaling eventEncoded...", workerIndex, contIndex );
				pthread_cond_signal ( &context->eventEncoded );
			}
#endif
			// Acquire access to portal packs
			if ( pthread_mutex_lock ( &renderContextMutex ) ) {
				CErrArgID ( "Worker [%u]: Failed to lock renderContextMutex.", workerIndex );
				break;
			}

			// Mark portal pack as free
			context->hasContent = false;

			// Release access to portal packs
			if ( pthread_mutex_unlock ( &renderContextMutex ) ) {
				CErrArgID ( "Worker [%u]: Failed to unlock renderContextMutex.", workerIndex );
				break;
			}
		}

#ifdef ENABLE_WORKER_STAGES_LOCKS
		/// Signal next worker thread to terminate
		if ( pthread_cond_signal ( &portalWorkerEvent ) ) {
			CErrArgID ( "Worker [%u]: Failed to signal workerStateEvent!", workerIndex );
		}
#endif
		CLogArgID ( "Worker [%u]: bye bye... Last context was [%u]", workerIndex, contIndex );

		return;
	}
#endif



	void PortalGenerator::DisposePortal ()
	{
		CVerbID ( "DisposePortal" );

		// We need this for the case that the network has shut down while we're still about to connect to the device
		status = PortalSourceStatus::Deleteable;

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		if ( recognizers ) {
			delete recognizers;
			recognizers = 0;
		}
#else
		//
		// Stop recognizer thread
		DisposeThread ( &recognizerThreadIDState, recognizerThreadID, 0, "recognizer", recognizerEvent );
#endif


		for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ ) {
#ifdef ENABLE_WORKER_STAGES_LOCKS
			env_sem_posts ( renderContexts [i].renderSem );
			env_sem_posts ( renderContexts [i].encoderSem );
			env_sem_posts ( renderContexts [i].sendSem );
#else
			// Unlock all worker threads
			pthread_cond_mutex_lock ( & portalWorkerEventLock );
			pthread_cond_signal_checked ( &portalWorkerEvent );
			pthread_cond_mutex_unlock ( & portalWorkerEventLock );

			pthread_cond_signal_checked ( &renderContexts [i].eventRendered );
			pthread_cond_signal_checked ( &renderContexts [i].eventEncoded );
#endif
		}

		char threadName [] = "portal worker [0]";

		for ( unsigned int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ )
		{
			///
			/// Stop worker threads

#ifdef ENABLE_WORKER_STAGES_LOCKS
			for ( unsigned int k = 0; k < (MAX_PORTAL_CONTEXT_WORKERS - i); k++ ) {
				/// Lock the signal mutex
				pthread_cond_mutex_lock ( &portalWorkerEventLock );

				/// Signal a worker thread
				if ( pthread_cond_signal ( &portalWorkerEvent ) ) {
					CErrID ( "DisposePortal: Failed to signal workerStateEvent!" );
				}

				/// UnLock the signal mutex
				pthread_cond_mutex_unlock ( &portalWorkerEventLock );
			}
#else
			unsigned int j = !i ? MAX_PORTAL_CONTEXT_WORKERS - 1 : i - 1;

			pthread_cond_signal_checked ( &renderContexts [j].eventEncoded );
			pthread_cond_signal_checked ( &renderContexts [j].eventRendered );
#endif
			DisposeThread ( &workerThreadsID [i], workerThreads [i], 0, threadName, portalWorkerEvent );
			*(threadName + 15) += 1;

#ifndef ENABLE_WORKER_STAGES_LOCKS
			pthread_cond_signal_checked ( &renderContexts [i].eventEncoded );
			pthread_cond_signal_checked ( &renderContexts [i].eventRendered );
#endif
        }

		DisposeWorkerStages ( &workerStages );

		DisposeRenderContexts ( );

#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		// Dispose recognizers
		if ( recognizers ) {
			for ( unsigned int i = 0; i < recognizersCount; i++ ) {
				if ( recognizers [i] )
					delete recognizers [i];
			}
			free ( recognizers );
			recognizers = 0;
		}

		recognizersCount = 0;
		recognizedGesture = false;
#endif

		// Dispose recognizer touch events here?

	}


	void PortalGenerator::DisposeRenderContexts ()
	{
		CVerbID ( "DisposeRenderContexts" );

		for ( int i = 0; i < MAX_PORTAL_CONTEXT_WORKERS; i++ ) {
			RenderContext * context = renderContexts + i;

			CVerbArgID ( "DisposeRenderContext [%i]", renderContexts [i].id );

			if ( !context->renderSkipped ) {

#if defined(_WIN32) && !defined(WINDOWS_PHONE)
				if ( context->renderedDataType == PortalBufferType::GDIBitmap ) {
					if ( context->renderedDataHandle ) {
						delete ((Gdiplus::Graphics *) context->renderedDataHandle);
						context->renderedDataHandle = 0;
					}

					if ( context->renderedData ) {
						delete ((Gdiplus::Bitmap *)context->renderedData);
						context->renderedData = 0;
					}
				}
				else
#endif
					if ( context->renderedData )
						free ( context->renderedData );
			}

			/// the outputbuffer should be released from the mvcrt of the encoder module (heap corruption may occur)
			if ( context->outputBuffer )
				disposeBuffer ( context->outputBuffer );

#ifdef ENABLE_WORKER_STAGES_LOCKS
			if ( !context->isInitialized )
				continue;

			if ( !env_sem_dispose ( context->renderSem ) ) {
				CErrArgID ( "DisposeRenderContext: Failed to destroy render semaphore of context [%i].", i );
			}

			if ( !env_sem_dispose ( context->encoderSem ) ) {
				CErrArgID ( "DisposeRenderContext: Failed to destroy encoder semaphore of context [%i].", i );
			}

			if ( !env_sem_dispose ( context->sendSem ) ) {
				CErrArgID ( "DisposeRenderContext: Failed to destroy send semaphore of context [%i].", i );
			}
#else
            if ( pthread_cond_valid ( context->eventRendered ) )
                CondDispose ( &context->eventRendered );
            if ( pthread_cond_valid ( context->eventEncoded ) )
                CondDispose ( &context->eventEncoded );
#endif
		}
		Zero ( renderContexts );
	}



#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
	void * PortalGenerator::Thread_RecognizerStarter ( void * arg )
	{
		if ( !arg ) {
			CErr ( "recognizerThreadStarter: called with invalid (NULL) argument." );
			return 0;
		}

		// Execute thread
		return ((PortalGenerator *)arg)->Thread_Recognizer ();
	}


	void * PortalGenerator::Thread_Recognizer ()
	{
		CLogID ( "Thread_Recognizer started..." );

		pthread_setname_current_envthread ( "PortalGenerator::Thread_Recognizer" );

		while ( status > PortalSourceStatus::Deleteable )
		{
			pthread_cond_mutex_lock ( &recognizerCritSec );

			if ( pthread_cond_wait_time ( &recognizerEvent, &recognizerCritSec, INFINITE ) || status < PortalSourceStatus::Initialized )
			{
				if ( status >= PortalSourceStatus::Initialized ) {
					CLogID ( "Recognizer: error occoured during wait for recognizer event or portal closed!" );
				}

				pthread_cond_mutex_unlock ( &recognizerCritSec );
				break;
			}
			pthread_cond_mutex_unlock ( &recognizerCritSec );

			unsigned int	recoIndex		= 0;

			if ( pthread_mutex_lock ( &recognizerCritSec ) ) {
				CErrID ( "Recognizer: Failed to lock recognizerCritSec." );
				break;
			}

			recoIndex = recognizerContainerIndexer;

			if ( pthread_mutex_unlock ( &recognizerCritSec ) ) {
				CErrID ( "Recognizer: Failed to unlock recognizerCritSec." );
				break;
			}

			unsigned int	recoCount		= recognizerTouchesCount [recoIndex];
			lib::InputPackRec *		recoContainer	= recognizerTouches [recoIndex];

			if ( recognizerTouchesHandled [recoIndex] )
				continue;

			if ( !recoCount || !recoContainer ) {
				recognizerTouchesHandled [recoIndex] = true;
				continue;
			}

			if ( recognizersCount && recognizers ) {
				recognizedGesture = false;

				for ( unsigned int i = 0; i < recognizersCount; i++ ) {
					if ( recognizers [i]->Perform ( (lib::InputPackRec **)recoContainer, recoCount ) ) {
						recognizedGesture = true;
						break;
					}
				}
			}

			if ( pthread_mutex_lock ( &recognizerCritSec ) ) {
				CErrID ( "Recognizer: Failed to lock recognizerCritSec." );
				break;
			}

			recognizerTouchesHandled [recoIndex] = true;

			if ( pthread_mutex_unlock ( &recognizerCritSec ) ) {
				CErrID ( "Recognizer: Failed to unlock recognizerCritSec." );
				break;
			}

			pthread_cond_preparev ( &recognizerEvent );
		}

		CLogID ( "Thread_Recognizer terminated..." );
		return 0;
	}
#endif


} /// -> namespace environs

