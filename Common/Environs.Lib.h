/**
 * Environs Native Layer API exposed by the libraries
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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_API_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_API_H

#include "Environs.Lib.Inc.h"

#include "Environs.Lib.Display.h"
#include "Environs.Lib.Mobile.h"

namespace environs 
{
	namespace API 
	{
		extern void UpdateNetworkStatus ();
        extern void SetDeviceID ( int deviceID );

		/**
		 *	API exports that are used for every platform
		 *
         * */
        ENVIRONSAPI LIBEXPORT void CallConv SetCallbacks ( long TouchCallback, long MessageCallback, long MessageExtCallback, long NotifyCallback, long NotifyExtCallback, long DataCallback, long StatusMessageCallback );

		ENVIRONSAPI LIBEXPORT EBOOL CallConv GetPortalInfo1 ( void * portalInfo );
		ENVIRONSAPI LIBEXPORT EBOOL CallConv SetPortalInfo1 ( void * portalInfo );
		ENVIRONSAPI LIBEXPORT void CallConv SetDeviceID2 ( int deviceID );

		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		ENVIRONSAPI LIBEXPORT EBOOL CallConv GetIsReleaseBuild ();
                
/**
*	API exports for all platforms but Android
*
* */
#if !defined(ANDROID)

#ifdef __cplusplus
		extern "C"
		{
#endif
			LIBEXPORT const char * CallConv GetVersionString ();
			LIBEXPORT int CallConv			GetVersionMajor ();
			LIBEXPORT int CallConv			GetVersionMinor ();
			LIBEXPORT int CallConv			GetVersionRevision ();
			LIBEXPORT int CallConv			GetRuntimePlatform ();

			LIBEXPORT void CallConv			RegisterMainThread ();
			LIBEXPORT void CallConv			SetDebug ( int mode );
			LIBEXPORT void CallConv			SetOSLevel ( int level );
			LIBEXPORT void CallConv			ResetIdentKeys ();
			LIBEXPORT void CallConv			SetDeviceID1 ( int myDeviceID );
			LIBEXPORT int CallConv			GetDeviceID ();
			LIBEXPORT int CallConv			GetDeviceIDFromMediator ();
            
            
			LIBEXPORT int CallConv			GetMediatorFilterLevel ( );
			LIBEXPORT void CallConv			SetMediatorFilterLevel ( int level );
			LIBEXPORT int CallConv			GetIPAddress ( );
			LIBEXPORT int CallConv			GetSubnetMask ( );
			LIBEXPORT void CallConv			SetDeviceDims ( int width, int height, int width_mm, int height_mm, int leftpos, int toppos );

			LIBEXPORT EBOOL CallConv		SetPorts1 ( int tcpPort, int udpPort );
			LIBEXPORT void CallConv			SetDeviceType ( char type );
			LIBEXPORT char CallConv			GetDeviceType ( );
            LIBEXPORT EBOOL CallConv		HasDeviceUID ( );
            LIBEXPORT EBOOL CallConv		SetDeviceUID ( const char * name );
			LIBEXPORT EBOOL CallConv		SetDeviceName ( const char * name );
            
			LIBEXPORT EBOOL CallConv		SetUseTouchRecognizer1 ( const char * name, bool enable );
            
			LIBEXPORT EBOOL CallConv		SetProjectName1 ( const char * name );
			LIBEXPORT const char * CallConv	GetProjectName ( );
			LIBEXPORT EBOOL CallConv		SetApplicationName1 ( const char * name );
			LIBEXPORT const char * CallConv	GetApplicationName ( );
			LIBEXPORT EBOOL CallConv		SetMediatorUserName1 ( const char * name );
			LIBEXPORT EBOOL CallConv		SetMediatorPassword1 ( const char * name );
			LIBEXPORT void CallConv			SetUseAuthentication ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		SetGCMAPIKey ( const char * key );

			LIBEXPORT int CallConv			Init1 ();
			LIBEXPORT void CallConv			SetAppStatus ( int status );
			LIBEXPORT int CallConv			SetMainAppWindow ( WNDHANDLE hWnd );
            
            LIBEXPORT EBOOL CallConv		SetPortalSourceWindow ( WNDHANDLE hWnd, int nativeID );
			LIBEXPORT void CallConv			UpdateAppWindowSize ( );
			/**
			* Instruct Environs native layer to prepare required portal resources to base on generation within the platform layer.
			*
			* @param enable      true = enable, false = disable
			*/
			LIBEXPORT void CallConv			SetUsePlatformPortalGenerator ( EBOOL enabled );

			LIBEXPORT EBOOL CallConv		GetUseOpenCL ( );
			LIBEXPORT void CallConv			SetUseOpenCL ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseStream ( );
			LIBEXPORT void CallConv			SetUseStream ( EBOOL enable );

			LIBEXPORT void CallConv			SetUseMouseEmulation ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		GetUseMouseEmulation ( );

			LIBEXPORT void CallConv			SetUseTouchVisualization ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		GetUseTouchVisualization ( );
            
            LIBEXPORT int CallConv			GetDeviceConnectStatus ( int nativeID );
            LIBEXPORT EBOOL CallConv		GetDirectContactStatus ( int nativeID );
            
            /**
             * Get the status, whether the device (id) has established an active portal
             *
             * @param 	deviceID    The device id of the target device.
             * @param 	projectName	Project name of the application environment
             * @param 	appName		Application name of the application environment
             * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
             * @return	success 	true = yes, false = no
             */
            LIBEXPORT EBOOL CallConv		GetPortalEnabled ( int nativeID, int portalType );

            /**
             * Get the portalID of the first active portal
             *
             * @param 	deviceID    The device id of the target device.
             * @param 	projectName	Project name of the application environment
             * @param 	appName		Application name of the application environment
             * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
             * @return	portalID 	The portal ID.
             */
            LIBEXPORT int CallConv          GetPortalId ( int nativeID, int portalType );
            
            LIBEXPORT int CallConv			DeviceDetected ( int deviceID, const char * projName, const char * appName, int async, int x, int y, float angle );
            
            LIBEXPORT int CallConv			DeviceConnect ( int deviceID, const char * projName, const char * appName, int async );
            LIBEXPORT EBOOL CallConv		DeviceDisconnect1 ( int nativeID, int async );
            
            LIBEXPORT void CallConv			DeviceUpdated ( int nativeID, int async, int x, int y, float angle );
            LIBEXPORT void CallConv			DevicePositionUpdated ( int nativeID, int async, int x, int y );
            LIBEXPORT void CallConv			DeviceAngleUpdated ( int nativeID, int async, float angle );
            LIBEXPORT void CallConv			DeviceRemoved ( int nativeID, int async, int x, int y, float angle );
            LIBEXPORT void CallConv			DeviceRemovedID ( int nativeID, int async );
				
			LIBEXPORT int CallConv			GetStatus1 ();
			LIBEXPORT int CallConv			Start1 ();
			LIBEXPORT int CallConv			Stop1 ();
			LIBEXPORT void CallConv			Release1 ();

			LIBEXPORT void CallConv			SetStreamOverUdp ( );
			LIBEXPORT void CallConv			SetStreamOverTcp ( );
			LIBEXPORT EBOOL CallConv		StreamToggleQuality ( );
			LIBEXPORT void CallConv			SetStreamJpegs ( );
			LIBEXPORT void CallConv			SetStreamPngs ( );
            
            LIBEXPORT EBOOL CallConv		SendBuffer ( int nativeID, int async, int fileID, const char * fileDescriptor, char * buffer,  int size );
            LIBEXPORT EBOOL CallConv		SendFile ( int nativeID, int async, int fileID, const TCHAR * fileDescriptor, const TCHAR * file );

            LIBEXPORT EBOOL CallConv		SendMsg ( int deviceID, const char * projName, const char * appName, int async, const char * message, int length );
            
			LIBEXPORT EBOOL CallConv		SendPushNotification ( int deviceID, const char * projName, const char * appName, const char * message );

			LIBEXPORT EBOOL CallConv		RegisterAtMediators ();

			LIBEXPORT EBOOL CallConv		SetMediator ( const char * ip, int port );
			LIBEXPORT const char * CallConv GetMediatorIP ();
			LIBEXPORT const char * CallConv GetMediatorUserName1 ();
			LIBEXPORT int CallConv			GetMediatorPort ();

			LIBEXPORT void CallConv			SetUseDefaultMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseDefaultMediator ( );
            
			LIBEXPORT void CallConv			SetUseCustomMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCustomMediator ( );
			
            LIBEXPORT void CallConv			SetUseDefaultMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseDefaultMediator ( );

			LIBEXPORT void CallConv			SetUseCLSForMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForMediator ( );

			LIBEXPORT void CallConv			SetUseCLSForDevices ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForDevices ( );

			LIBEXPORT void CallConv			SetUseCLSForDevicesEnforce ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForDevicesEnforce ( );

			LIBEXPORT void CallConv			SetUseCLSForAllTraffic ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseCLSForAllTraffic ( );
            
            LIBEXPORT void CallConv			SetUseNotifyDebugMessage ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseNotifyDebugMessage ( );
            
            LIBEXPORT void CallConv			SetUseLogFile ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseLogFile ( );
            
			LIBEXPORT void CallConv			SetNetworkStatus ( int netStat );
			LIBEXPORT int CallConv			GetNetworkStatus ( );
            
            LIBEXPORT EBOOL CallConv        ApplyModuleToSettings ( const char * moduleName, char ** target, bool testInstance = true );
            
			LIBEXPORT EBOOL CallConv		SetUseEncoder ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		SetUseDecoder ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		SetUseRenderer ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		SetUseCapturer ( const char * moduleName );

            LIBEXPORT void * CallConv		GetFileNative ( int nativeID, int fileID, void * buffer, int * capacity );
            LIBEXPORT void * CallConv		GetFile ( int deviceID, const char * projName, const char * appName, int fileID, void * buffer, int * capacity );
            
            LIBEXPORT BSTR CallConv			GetFilePathNative ( int nativeID, int fileID );
            LIBEXPORT BSTR CallConv			GetFilePath ( int deviceID, const char * projName, const char * appName, int fileID );


			LIBEXPORT EBOOL CallConv		GetPortalInfo ( void * buffer );
			LIBEXPORT EBOOL CallConv		SetPortalInfo ( void * buffer );
            
            LIBEXPORT EBOOL CallConv		SetPortalOverlayARGB ( int nativeID, int portalID, int layerID, int left, int top,
                                                                  int width, int height, int stride, void * renderData, int alpha, bool positionDevice );
			LIBEXPORT void CallConv			FreeNativeMemory ( void * obj );

			LIBEXPORT void * CallConv		GetDevices ( int fromType );
            LIBEXPORT void * CallConv		GetDevice ( int deviceID, const char * projName, const char * appName, int fromType );
            LIBEXPORT void * CallConv		GetDeviceForPortal1 ( int portalID );
            
            LIBEXPORT int CallConv			GetDevicesCount ( int fromType );

            LIBEXPORT void * CallConv		GetDeviceDisplayProps1 ( int nativeID );

			LIBEXPORT void * CallConv		GetDeviceBestMatch1 ( int deviceID );

			LIBEXPORT int CallConv			GetConnectedDevicesCount ( );

			LIBEXPORT int CallConv			GetDeviceInstanceSize ();
			LIBEXPORT int CallConv			GetDevicesHeaderSize ();

			//LIBEXPORT void CallConv			deallocNative ();
			LIBEXPORT void CallConv			InitStorage ( const char * path );
			LIBEXPORT void CallConv			InitWorkDir ( const char * path );
            LIBEXPORT void CallConv			SetGCM ( const char * gcm );

            LIBEXPORT int CallConv			GetDeviceWidth ( int nativeID );
            LIBEXPORT int CallConv			GetDeviceHeight ( int nativeID );
            
            LIBEXPORT EBOOL CallConv		InitRequestPortalStream ( int nativeID, int async, int typeID );
            LIBEXPORT EBOOL CallConv		ProvidePortalStream1 ( int nativeID, int async, int portalID );

			LIBEXPORT EBOOL CallConv		StartPortalStream1 ( int async, int portalID );
			LIBEXPORT EBOOL CallConv		PausePortalStream1 ( int async, int portalID );
			LIBEXPORT EBOOL CallConv		StopPortalStream1 ( int async, int portalID );
            
            /**
             * Acquire a native layer send identifier (and increase instanceLock on device) to be used in sendTcpPortal. This resource must be released on disposal of the portal generator.
             *
             * @param deviceID      The device id of the target device.
             * @param projectName	Project name of the application environment
             * @param appName		Application name of the application environment
             * @param portalUnitType	e.g. MSG_TYPE_STREAM
             * @param portalID    Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
             * @return sendID
             */
            LIBEXPORT int CallConv          AcquirePortalSendID ( int portalUnitType, int portalID );
            
            /**
             * Release a native layer send identifier that was acquired by a call to acquireTcpPortalSendID.
             *
             * @param sendID      The portal send id resource to be released.
             */
            LIBEXPORT void CallConv         ReleasePortalSendID ( int sendID );
            
            /**
             * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
             *
             * @param sendID      		The device id of the target device.
             * @param portalUnitFlags	Flags that will be or-ed with the portalUnitType
             * @param prefixBuffer    	A prefix that prepend the actual buffer.
             * @param prefixSize   		The size of the content within the prefixbuffer.
             * @param buffer        	A buffer to be send.
             * @param offset        	An offset into the buffer.
             * @param contentSize   	The size of the content within the buffer.
             * @return success
             */
            LIBEXPORT EBOOL CallConv		SendTcpPortal ( int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize );
            
			LIBEXPORT EBOOL CallConv		SetRenderSurface1 ( int portalID, void * renderSurface, int width, int height );
			LIBEXPORT EBOOL CallConv		ReleaseRenderSurface1 ( int async, int portalID );
			LIBEXPORT EBOOL CallConv		SetRenderCallback ( int async, int portalID, long callback, int type );

#ifdef __cplusplus
		}
#endif

#endif // ANDROID - include only the jni-exports

	}
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_API_H



