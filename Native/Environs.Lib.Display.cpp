/**
 * Environs Native Layer API exposed by the libraries for large display devices
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
#define ENVIRONS_NATIVE_MODULE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

// Import access to the environs native object
#include "Environs.Obj.h"
using namespace environs;

// Import declarations and exports for the API
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Crypt.h"
#include "Environs.Mobile.h"
#include "Environs.Modules.h"
//#include "Interop/Stat.h"

#include "Core/Async.Worker.h"
#include "Core/Byte.Buffer.h"
#include "Core/Callbacks.h"
#include "Core/Notifications.h"

#include "Device/Device.Controller.h"
#include "Portal/Portal.Device.h"

#ifdef DISPLAYDEVICE
#include "Renderer/Render.OpenCL.h"
#endif


// Preprocs for ASSERT_OBJECT
#define CLASS_NAME	"Native.Display . . . . ."



namespace environs 
{
    
#ifdef DISPLAYDEVICE
    
	namespace API
    {

		ENVIRONSAPI void PreDisposeN ()
		{
#ifdef _WIN32
			Kernel::DisposeWinSock ();
#endif
		}
        
        
        ENVIRONSAPI int SetMainAppWindowN ( jint hInst, WNDHANDLE hWnd )
        {
            CVerb ( "SetMainAppWindowN" );
            
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "SetMainAppWindowN" );

			sp ( Instance ) envSP = native.instancesSP [ hInst ];

			LockReleaseVA ( native.instancesSPLock, "SetMainAppWindowN" );
#endif
			if ( !envSP || !envSP->kernel )
				return 0;

            return envSP->kernel->SetMainAppWindow ( hWnd );
        }
        
        /*
         * Method:    SetPortalOverlayARGB
         * Signature: (I)I;
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SetPortalOverlayARGBN, jint hInst, jint nativeID, jint portalID, jint layerID, jint left, jint top, jint width, jint height, jint stride, void * renderData, jint alpha, bool positionDevice )
        {
            CVerbIDN ( "SetPortalOverlayARGBN" );
            
            if ( instances[hInst]->environsState < environs::Status::Started ) {
                CVerbIDN ( "SetPortalOverlayARGBN: Environs is not started!" );
                return false;
            }

			bool success = false;

			PortalDevice * portal = GetLockedPortalDevice ( portalID );
			if ( portal ) {
				if ( portal->generator )
					success = portal->generator->setPortalOverlayARGB ( layerID, left, top, width, height, stride, renderData, alpha, positionDevice );

				ReleasePortalDevice ( portal );
			}
            /*if ( !renderData ) {
             CErrID ( "setPortalOverlayARGB: Called with NULL renderData argument." );
             return false;
             }*/
			/*
            unsigned int id = PortalID ();
            if ( id >= MAX_PORTAL_STREAMS_A_DEVICE )
                return false;
            
            bool ret = false;
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            
            if ( !device )
                return false;
            
            if ( pthread_mutex_lock ( &device->portalMutex ) ) {
                CErrIDN ( "SetPortalOverlayARGB: Failed to acquire lock on mutex." );
            }
            else {
                if ( device->portalGenerators [id] )
                    ret = device->portalGenerators [id]->setPortalOverlayARGB ( layerID, left, top, width, height, stride, renderData, alpha, positionDevice );
                
                if ( pthread_mutex_unlock ( &device->portalMutex ) ) {
                    CErrIDN ( "SetPortalOverlayARGB: Failed to release lock on portal mutex." );
                }
            }
            
            UnlockDevice ( device );
			*/
			return success;
        }
        
        
        ENVIRONSAPI EBOOL SetPortalSourceWindowN ( WNDHANDLE hWnd, jint nativeID )
        {
            CVerbIDN ( "SetPortalSourceWindowN" );
            if ( !hWnd )
                return false;
            
            DeviceController * device = (DeviceController *)environs::GetDevice ( nativeID );
            if ( !device )
                return false;
            
            device->portalhWnd = hWnd;
            
            UnlockDevice ( device );
            return true;
        }


		ENVIRONSAPI void UpdateAppWindowSizeN ( jint hInst )
		{
			CVerb ( "UpdateAppWindowSizeN" );

			Instance * env = instances [ hInst ];
			if ( env )
				env->kernel->UpdateAppWindowSize ();
		}


		ENVIRONSAPI void SetUseTouchInjectionN ( EBOOL enabled )
		{
			CVerb ( "SetUseTouchInjectionN" );

			DeviceController::inject_touch = (enabled ? true : false);

			//environs.deviceType = DEVICE_TYPE_SURFACE1;
		}

        
        ENVIRONSAPI void DeviceUpdatedN ( jint hInst, jint nativeID, jint portalID, int async, jint x, jint y, float angle )
        {
            CVerb ( "DeviceUpdatedN" );
            
            Instance * env = instances[hInst];
            
            if ( async )
                env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_UPDATE_XYANG, portalID, x, y, angle );
            else
                env->asyncWorker.UpdateDevice ( nativeID, portalID, ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG, x, y, angle, true );
        }
        
        
        ENVIRONSAPI void DevicePositionUpdatedN ( jint hInst, jint nativeID, jint portalID, int async, jint x, jint y )
        {
            CVerb ( "DevicePositionUpdatedN" );
            
            Instance * env = instances[hInst];
            
            if ( async )
                env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_UPDATE_XYANG, portalID, x, y, -1 );
            else
                env->asyncWorker.UpdateDevice ( nativeID, portalID, ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG, x, y, -1, true );
        }
        
        
        ENVIRONSAPI void DeviceAngleUpdatedN ( jint hInst, jint nativeID, jint portalID, int async, float angle )
        {
            CVerb ( "DeviceAngleUpdatedN" );
            
            Instance * env = instances[hInst];
            
            if ( async )
                env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_UPDATE_ANG, portalID, angle );
            else
                env->asyncWorker.UpdateDevice ( nativeID, portalID, ASYNCWORK_TYPE_DEVICE_UPDATE_ANG, 0, 0, angle, true );
        }
        
        
        ENVIRONSAPI void DeviceRemovedN ( jint hInst, jint nativeID, jint portalID, int async, jint x, jint y, float angle )
        {
            CVerb ( "DeviceRemovedN" );
            
            Instance * env = instances[hInst];
            
            if ( async )
                env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_REMOVED, portalID, x, y, angle );
            else
                env->asyncWorker.UpdateDevice ( nativeID, portalID, ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG, x, y, angle, false );
        }
        
        
        ENVIRONSAPI void DeviceRemovedIDN ( jint hInst, jint nativeID, jint portalID, int async )
        {
            CVerb ( "DeviceRemovedIDN" );
            
            Instance * env = instances[hInst];
            
            if ( async )
                env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_REMOVED_ID, -1 );
            else
                env->asyncWorker.UpdateDevice ( nativeID, portalID, ASYNCWORK_PARAM_DEVICE_UPDATE_STATUS, 0, 0, -1, false );
        }


		ENVIRONSAPI void SetStreamOverUdpN ( int hInst )
		{
            CVerb ( "SetStreamOverUdpN" );
            
            Instance * env = instances[hInst];

			env->streamOverCom = false;

            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Now streaming over UDP" );
		}


		ENVIRONSAPI void SetStreamOverTcpN ( int hInst )
		{
            CVerb ( "SetStreamOverTcpN" );
            
            Instance * env = instances[hInst];

			env->streamOverCom = true;

            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Now streaming over TCP" );
		}


		ENVIRONSAPI void SetStreamJpegsN ( int hInst )
		{
            CVerb ( "SetStreamJpegsN" );
            
            Instance * env = instances[hInst];

			env->usePNG = false;
			env->useStream = false;

            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Using JPEG for stream" );
		}



		ENVIRONSAPI void SetStreamPngsN ( int hInst )
		{
            CVerb ( "SetStreamPngsN" );
            
            Instance * env = instances[hInst];

			env->usePNG = true;
			env->useStream = false;

            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Using PNG for stream" );
		}


		ENVIRONSAPI EBOOL StreamToggleQualityN ( int hInst )
		{
            CVerb ( "StreamToggleQualityN" );
            
            Instance * env = instances[hInst];

			bool newValue = !env->useStreamUltrafast;
			env->useStreamUltrafast = newValue;
            
            if ( newValue )
                onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Using Stream ultrafast" );
            else
                onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Using Stream veryfast" );

			return newValue;
		}


		ENVIRONSAPI EBOOL GetUseOpenCLN ( int hInst )
		{
			return instances[hInst]->useOCL ? true : false;
		}


		ENVIRONSAPI void SetUseOpenCLN ( int hInst, EBOOL enable )
		{
            CVerb ( "SetUseOpenCLN" );
            
            Instance * env = instances[hInst];

			bool status = enable ? true : false;

			if ( status ) {
				RenderOpenCL::InitOpenCL ( );
				if ( !RenderOpenCL::ocl_initialized ) {
					status = false;
				}
			}

			env->useOCL = status;
            
            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, status ? "Enabled OpenCL" : "Disabled OpenCL" );
		}


		ENVIRONSAPI void SetUseDeviceMarkerAutomaticN ( EBOOL enable )
		{
			CVerb ( "SetUseDeviceMarkerAutomaticN" );

			opt_useDeviceMarkerHandler = (enable ? true : false);
		}


		ENVIRONSAPI void SetDeviceMarkerReducedPrecisionN ( EBOOL enable )
		{
			CVerb ( "SetDeviceMarkerReducedPrecisionN" );

			opt_useDeviceMarkerReducedPrecision = (enable ? true : false);
		}


		ENVIRONSAPI void SetUseMouseEmulationN ( jint hInst, EBOOL enable )
		{
            CVerb ( "SetUseMouseEmulationN" );

			bool status = enable ? true : false;

			instances[hInst]->simulateMouse = status;

			Kernel::SetUseTouchRecognizer("libEnv-RecMouseSimulatorWin" LIBEXTENSION, status);
		}


		ENVIRONSAPI EBOOL GetUseMouseEmulationN ( jint hInst )
		{
			CVerb ( "GetUseMouseEmulationN" );

			return instances[hInst]->simulateMouse ? 1 : 0;
		}


		ENVIRONSAPI void SetUseTouchVisualizationN ( jint hInst, EBOOL enable )
		{
            CVerb ( "SetUseTouchVisualizationN" );

			bool status = enable ? true : false;

			instances[hInst]->visualizeTouches = status;

			Kernel::SetUseTouchRecognizer("libEnv-RecTouchVisualizer" LIBEXTENSION, status);
		}


		ENVIRONSAPI EBOOL GetUseTouchVisualizationN ( jint hInst )
		{
			CVerb ( "GetUseTouchVisualizationN" );

			return instances[hInst]->visualizeTouches ? 1 : 0;
		}
		

		/**
		* Method:    SetTrackerParams
		* Signature: ()V
		*/
		ENVIRONSAPI EBOOL SetTrackerParamsN ( jint hInst, jint index, jint channels, jint width, jint height, jint stride )
		{
            CVerb ( "SetTrackerParamsN" );
            
            Instance * env = instances[hInst];
			
			if ( env->kernel ) {
				ITracker * tracker = env->kernel->GetTracker ( index );
				if ( tracker )
					if ( tracker->Init ( width, height, channels, stride ) )
						return tracker->Start ();
			}
			return false;
		}


		/**
		* Method:    SetTrackerImage
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL SetTrackerImageN ( jint hInst, jint index, void * rawImage, jint size )
		{
            CVerb ( "SetTrackerImageN" );
            
            Instance * env = instances[hInst];

			if ( rawImage && size && env->kernel ) {
				ITracker * tracker = env->kernel->GetTracker ( index );
				if ( tracker )
					return tracker->Perform ( rawImage, size );
			}

			return false;
		}

    }
    
#endif
}
