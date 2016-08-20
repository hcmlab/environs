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

	bool LoadConfig ( Instance * env );
	bool SaveConfig ();

	namespace API 
	{
		void UpdateNetworkStatus ();
        void SetDeviceID ( int hInst, int deviceID );

		/**
		*	API exports that are used for every platform
		*
		* */
		CLI_INC ENVIRONSAPI LIBEXPORT void CallConv SetCallbacksN ( int hInst, void * HumanInputCallback, void * SensorInputCallback, void * MessageCallback, void * MessageExtCallback, void * NotifyCallback, void * NotifyExtCallback, void * DataCallback, void * StatusMessageCallback );

		CLI_INC ENVIRONSAPI LIBEXPORT EBOOL CallConv GetPortalInfoN ( int hInst, void * portalInfo );

		CLI_INC ENVIRONSAPI LIBEXPORT EBOOL CallConv SetPortalInfoN ( int hInst, void * portalInfo );

		CLI_INC ENVIRONSAPI LIBEXPORT void CallConv SetDeviceID2 ( int hInst, int deviceID );

		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		CLI_INC ENVIRONSAPI LIBEXPORT EBOOL CallConv GetIsReleaseBuildN ();
        
        
        EBOOL SetMediatorUserNameNM ( int hInst, const char * name );
        
        EBOOL SetMediatorPasswordNM ( int hInst, const char * pass );
/**
*	API exports for all platforms but Android
*
* */
#if !defined(ANDROID)
#include "Interop/jni.h"

#ifdef __cplusplus
		extern "C"
		{
#endif
			CLI_INC LIBEXPORT jstring CallConv		GetVersionStringN ();

			CLI_INC LIBEXPORT int CallConv			GetVersionMajorN ();

			CLI_INC LIBEXPORT int CallConv			GetVersionMinorN ();

			CLI_INC LIBEXPORT int CallConv			GetVersionRevisionN ();

			CLI_INC LIBEXPORT int CallConv			GetPlatformN ();

            CLI_INC LIBEXPORT void CallConv			SetPlatformN ( int platform );
            
            CLI_INC LIBEXPORT void CallConv			SetIsLocationNodeN ( bool isLocationNode );

			CLI_INC LIBEXPORT EBOOL CallConv		GetDisposingN ( int hInst );

			CLI_INC LIBEXPORT void CallConv			SetDisposingN ( int hInst, EBOOL enable );

			CLI_INC LIBEXPORT const char * CallConv	ResolveNameN ( int notification );

			/**
			* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
			* Increasing this value may help to handle worse networks which suffer from large latencies.
			*
			* @param   timeout
			*/
			CLI_INC LIBEXPORT void CallConv			SetNetworkConnectTimeoutN ( int value );

			CLI_INC LIBEXPORT void CallConv			LogN ( CString_ptr msg, int length );


			/**
			* Create a native Environs object and return a handle to the object.
			* A return value of 0 means Error
			*
			* @return   An instance handle that refers to the created Environs object
			*/
			CLI_INC LIBEXPORT int CallConv			CreateEnvironsN ();


			/**
			* Load settings for the given application environment
			*
			* @param	hInst		The handle to a particular native Environs instance.
			* @param 	appName		The application name for the application environment.
			* @param  	areaName	The area name for the application environment.
			*
			* @return   success
			*/
			CLI_INC LIBEXPORT EBOOL CallConv        LoadSettingsN ( int hInst, CString_ptr app, CString_ptr area );
            
            
            CLI_INC LIBEXPORT void CallConv			ClearStorageN ();
            

			CLI_INC LIBEXPORT void CallConv			RegisterMainThreadN ( int hInst );

            CLI_INC LIBEXPORT void CallConv			SetDebugN ( int mode );
            
            CLI_INC LIBEXPORT int CallConv			GetDebugN ();

			CLI_INC LIBEXPORT void CallConv			SetOSLevelN ( int level );

            /**
             * Reset crypt layer and all created resources. Those will be recreated if necessary.
             * This method is intended to be called directly after creation of an Environs instance.
             *
             */
			CLI_INC LIBEXPORT void CallConv			ResetCryptLayerN ();

			CLI_INC LIBEXPORT int CallConv			GetAppAreaIDN ( int hInst );

			CLI_INC LIBEXPORT void CallConv			SetDeviceIDN ( int hInst, int myDeviceID );

			CLI_INC LIBEXPORT int CallConv			GetDeviceIDN ( int hInst );

			CLI_INC LIBEXPORT int CallConv			GetDeviceIDFromMediatorN ( int hInst );


			CLI_INC LIBEXPORT int CallConv			GetMediatorFilterLevelN ( int hInst );

			CLI_INC LIBEXPORT void CallConv			SetMediatorFilterLevelN ( int hInst, int level );

            
            
            /**
             * Enable or disable device list update notifications from Mediator layer.
             * In particular, mobile devices should disable notifications if the devicelist is not
             * visible to users or the app transitioned to background.
             * This helps recuding cpu load and network traffic when not required.
             *
             * @param enable      true = enable, false = disable
             */
            CLI_INC LIBEXPORT void SetMediatorNotificationSubscriptionN ( int hInst, int enable );
            
            
            /**
             * Get subscription status of device list update notifications from Mediator layer.
             *
             * @return enable      true = enable, false = disable
             */
            CLI_INC LIBEXPORT int GetMediatorNotificationSubscriptionN ( int hInst );
            
            
            /**
             * Enable or disable short messages from Mediator layer.
             * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
             * This helps recuding cpu load and network traffic when not necessary.
             *
             * @param enable      true = enable, false = disable
             */
            CLI_INC LIBEXPORT void SetMessagesSubscriptionN ( int hInst, int enable );
            
            
            /**
             * Get subscription status of short messages from Mediator layer.
             *
             * @return enable      true = enable, false = disable
             */
            CLI_INC LIBEXPORT int GetMessagesSubscriptionN ( int hInst );
            
            
			CLI_INC LIBEXPORT int CallConv			GetIPAddressN ();

			CLI_INC LIBEXPORT int CallConv			GetSubnetMaskN ();

			CLI_INC LIBEXPORT void CallConv			SetDeviceDimsN ( int width, int height, int width_mm, int height_mm, int leftpos, int toppos );


            /**
             * Set the ports that the local instance of Environs shall use for listening on connections.
             *
             * @param	tcpPort The tcp port.
             * @param	udpPort The udp port.
             * @return success
             */
			CLI_INC LIBEXPORT EBOOL CallConv		SetPortsN ( int hInst, int tcpPort, int udpPort );


            /**
             * Set the base port that the local instance of Environs shall use for communication with other instances.
             * This option enables spanning of multiple multi surface environsments separated by the network stacks.
             *
             * @param	port The base port.
             * @return success
             */
            CLI_INC LIBEXPORT EBOOL CallConv		SetBasePortN ( int hInst, int port );
            

			//CLI_INC LIBEXPORT void CallConv			SetDeviceTypeN ( char type );

			//CLI_INC LIBEXPORT char CallConv			GetDeviceTypeN ();


			/**
			* Update device flags to native layer and populate them to the environment.
			*
            * @param	hInst    The handle to the environs instance.
            * @param	async    Asyncronous using AsyncWorker or call it directly.
            * @param	objID    The identifier for the native device object.
            * @param	flags    The internal flags to set or clear.
            * @param	set    	 true = set, false = clear.
			*/
			CLI_INC LIBEXPORT  void SetDeviceFlagsN ( int hInst, int async, int objID, int flags, jboolean set );


			/**
			* Query whether the name of the current device has been set before.
			*
			* @return	has DeviceUID
			*/
			CLI_INC LIBEXPORT EBOOL CallConv		HasDeviceUIDN ();

			/**
			* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			*
			* @param 	deviceUID A unique identifier to identify this device.
			* @return	success
			*/
			CLI_INC LIBEXPORT EBOOL CallConv		SetDeviceUIDN ( CString_ptr name );

			/**
			* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			*
			* @param 	deviceName  The device name.
			* @return	success
			*/
			CLI_INC LIBEXPORT EBOOL CallConv		SetDeviceNameN ( CString_ptr name );

			CLI_INC LIBEXPORT EBOOL CallConv		SetUseTouchRecognizerN ( int hInst, CString_ptr name, bool enable );

			CLI_INC LIBEXPORT EBOOL CallConv		SetAreaNameN ( int hInst, CString_ptr name );

			CLI_INC LIBEXPORT jstring CallConv		GetAreaNameN ( int hInst );

			CLI_INC LIBEXPORT EBOOL CallConv		SetApplicationNameN ( int hInst, CString_ptr name );
			CLI_INC LIBEXPORT jstring CallConv		GetApplicationNameN ( int hInst );

			CLI_INC LIBEXPORT EBOOL CallConv		SetGCMAPIKeyN ( CString_ptr key );

			CLI_INC LIBEXPORT int CallConv			InitN ( int hInst );
			CLI_INC LIBEXPORT void CallConv			SetAppStatusN ( int hInst, int status );
			CLI_INC LIBEXPORT int CallConv			SetMainAppWindowN ( int hInst, WNDHANDLE hWnd );

			CLI_INC LIBEXPORT EBOOL CallConv		SetPortalSourceWindowN ( WNDHANDLE hWnd, int nativeID );
			CLI_INC LIBEXPORT void CallConv			UpdateAppWindowSizeN ( int hInst );

			/**
			* Instruct Environs native layer to prepare required portal resources to base on generation within the platform layer.
			*
			* @param enable      true = enable, false = disable
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetUsePlatformPortalGeneratorN ( int hInst, EBOOL enabled );

			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseOpenCLN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetUseOpenCLN ( int hInst, EBOOL enable );

			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseStreamN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetUseStreamN ( int hInst, EBOOL enable );

			CLI_INC
				LIBEXPORT int CallConv			GetDeviceConnectStatusN ( int hInst, int nativeID );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetDirectContactStatusN ( int hInst, int nativeID );

			/**
			* Get the status, whether the device (id) has established an active portal
			*
			* @param 	deviceID    The device id of the target device.
			* @param 	areaName	Area name of the application environment
			* @param 	appName		Application name of the application environment
			* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
			* @return	success 	true = yes, false = no
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetPortalEnabledN ( int hInst, int nativeID, int portalType );

			/**
			* Get the portalID of the first active portal
			*
			* @param 	nativeID    The device id of the target device.
			* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
			*
			* @return	portalID 	The portal ID.
			*/
			CLI_INC
				LIBEXPORT int CallConv          GetPortalIDN ( int hInst, int nativeID, int portalType );
            
            
            /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
             Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
            CLI_INC
                LIBEXPORT int CallConv			AllowConnectN ( int hInst, int objID, int value );
            
            /** Default value for each DeviceInstance after object creation. */
            CLI_INC
                LIBEXPORT int CallConv			AllowConnectDefaultN ( int hInst, int value );
            
            
			/**
			* Connect to device with the given ID and a particular application environment. Return value is of type enum Types.DeviceStatus
			*
			* @param	deviceID	Destination device ID
			* @param	areaName	Area name of the application environment
			* @param	appName		Application name of the application environment
			* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @return	0	Connection can't be established (maybe environs is stopped or the device ID is invalid)
			* @return	1	A new connection has been triggered and is in progress
			* @return	2	A connection to the device already exists or a connection task is already in progress)
			*/
			CLI_INC
				LIBEXPORT int CallConv			DeviceConnectN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int async );
			CLI_INC
				LIBEXPORT EBOOL CallConv		DeviceDisconnectN ( int hInst, int nativeID, int async );

			/**
			* Connect to device with the given ID and a particular application environment. Return value is of type enum Types.DeviceStatus
			*
			* @param	deviceID	Destination device ID
			* @param	areaName	Area name of the application environment
			* @param	appName		Application name of the application environment
			* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	x
			* @param 	y
			* @param 	angle
			* @return	0	Connection can't be established (maybe environs is stopped or the device ID is invalid)
			* @return	1	A new connection has been triggered and is in progress
			* @return	2	A connection to the device already exists or a connection task is already in progress)
			*/
			CLI_INC
				LIBEXPORT int CallConv			DeviceDetectedN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int Environs_CALL_, int x, int y, float angle );
			CLI_INC
				LIBEXPORT void CallConv			DeviceUpdatedN ( int hInst, int nativeID, int portalID, int async, int x, int y, float angle );
			CLI_INC
				LIBEXPORT void CallConv			DevicePositionUpdatedN ( int hInst, int nativeID, int portalID, int async, int x, int y );
			CLI_INC
				LIBEXPORT void CallConv			DeviceAngleUpdatedN ( int hInst, int nativeID, int portalID, int async, float angle );
			CLI_INC
				LIBEXPORT void CallConv			DeviceRemovedN ( int hInst, int nativeID, int portalID, int async, int x, int y, float angle );
			CLI_INC
				LIBEXPORT void CallConv			DeviceRemovedIDN ( int hInst, int nativeID, int portalID, int async );
                        
			CLI_INC
				LIBEXPORT int CallConv			GetStatusN ( int hInst );
			CLI_INC
				LIBEXPORT int CallConv			StartN ( int hInst );
			CLI_INC
				LIBEXPORT int CallConv			StopN ( int hInst );
			CLI_INC
                LIBEXPORT void CallConv			StopNetLayerN ( int hInst );
            
            /**
             * Instructs the framework to perform a quick shutdown (with minimal wait times)
             *
             * @param enable      true / false
             */
            CLI_INC
                LIBEXPORT void CallConv			SetAppShutdownN ( int enable );

			/**
			* Dispose/Release a native Environs object with the given object handle.
			*/
			CLI_INC
				LIBEXPORT void CallConv			DisposeN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetStreamOverUdpN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetStreamOverTcpN ( int hInst );
			CLI_INC
				LIBEXPORT EBOOL CallConv		StreamToggleQualityN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetStreamJpegsN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetStreamPngsN ( int hInst );

			CLI_INC
				LIBEXPORT EBOOL CallConv		SendBufferN ( int hInst, int nativeID, int async, int fileID, CString_ptr fileDescriptor, jbyteArray buffer, int size );
			CLI_INC
				LIBEXPORT EBOOL CallConv		SendFileN ( int hInst, int nativeID, int async, int fileID, CString_ptr fileDescriptor, CString_ptr file );

			CLI_INC
				LIBEXPORT EBOOL CallConv		SendMessageN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int async, jvoidArray message, int length );
            
            
            /**
             * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
             *
             * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
             * @param buffer        A buffer to be send.
             * @param offset        A user-customizable id that identifies the file to be send.
             * @param bytesToSend number of bytes in the buffer to send
             * @return success
             */
            CLI_INC
                LIBEXPORT EBOOL CallConv		SendDataUdpN ( int hInst, int nativeID, int async, jbyteArray buffer, int offset, int size );
            
			CLI_INC
				LIBEXPORT EBOOL CallConv		SendPushNotificationN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, CString_ptr message );

			CLI_INC
				LIBEXPORT EBOOL CallConv		RegisterAtMediatorsN ( int hInst );

			CLI_INC
				LIBEXPORT EBOOL CallConv		SetMediatorN ( int hInst, CString_ptr ip, int port );
			CLI_INC
				LIBEXPORT jstring CallConv		GetMediatorIPN ( int hInst );
			CLI_INC
				LIBEXPORT unsigned int CallConv GetMediatorIPValueN ( int hInst );
			CLI_INC
				LIBEXPORT int CallConv			GetMediatorPortN ( int hInst );
			
			/**
			* Enable or disable anonymous logon to the Mediator.
			*
			* @param 	enable A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetUseMediatorAnonymousLogonN ( int hInst, EBOOL usage );

			/**
			* Get setting of anonymous logon to the Mediator.
			*
			* @param 	enable A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseMediatorAnonymousLogonN ( int hInst );

			/**
			* Set the user name for authentication with a mediator service.&nbsp;Usually the user's email address is used as the user name.
			*
			* @param 	username    The user name for authentication at the Mediator.
			* @return	success
			*/
			CLI_INC
                LIBEXPORT EBOOL CallConv		SetMediatorUserNameN ( int hInst, CString_ptr name );

			/**
			* Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
			*
			* @param 	password    The mediator password.
			* @return	success
			*/
			CLI_INC
                LIBEXPORT EBOOL CallConv		SetMediatorPasswordN ( int hInst, CString_ptr name );
            
			/**
			* Enable or disable authentication with the Mediator using username/password.
			*
			* @param 	enable A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetUseAuthenticationN ( int hInst, EBOOL usage );

			/**
			* Query UserName used to authenticate with a Mediator.
			*
			* @return UserName The user name for authentication at the Mediator.
			*/
			CLI_INC 
				LIBEXPORT jstring CallConv		GetMediatorUserNameN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseDefaultMediatorN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseDefaultMediatorN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseCustomMediatorN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseCustomMediatorN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseMediatorLoginDialogN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseMediatorLoginDialogN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetMediatorLoginDialogDismissDisableN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetMediatorLoginDialogDismissDisableN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			ShowLoginDialogN ( int hInst, CString_ptr userName );

			CLI_INC
				LIBEXPORT void CallConv			SetUseCLSForMediatorN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseCLSForMediatorN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseCLSForDevicesN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseCLSForDevicesN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseCLSForDevicesEnforceN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseCLSForDevicesEnforceN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseCLSForAllTrafficN ( int hInst, EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseCLSForAllTrafficN ( int hInst );

			CLI_INC
				LIBEXPORT void CallConv			SetUseNotifyDebugMessageN ( EBOOL usage );
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetUseNotifyDebugMessageN ();

			CLI_INC
				LIBEXPORT void CallConv			SetUseLogFileN ( EBOOL usage );
			CLI_INC
                LIBEXPORT EBOOL CallConv		GetUseLogFileN ();
            
            /**
             * Instruct Environs to log to stdout.
             *
             * @param enable      true = enable, false = disable
             */
            CLI_INC
                LIBEXPORT  void                 SetUseLogToStdoutN ( EBOOL enable );            
            
            /**
             * Query Environs settings whether to log to stdout.
             *
             * @return enable      true = enabled, false = disabled
             */
            CLI_INC
                LIBEXPORT EBOOL CallConv		GetUseLogToStdoutN ();


			/**
			* Instruct Environs to use headless mode without worrying about UI thread.
			*
			* @param enable      true = enable, false = disable
			*/
            CLI_INC
				LIBEXPORT  void                 SetUseHeadlessN ( int enable );

			/**
			* Query Environs settings whether to use headless mode without worrying about UI thread.
			*
			* @return enable      true = enabled, false = disabled
			*/
			CLI_INC
				LIBEXPORT int CallConv			GetUseHeadlessN ();


            /**
             * Check for mediator logon credentials and query on command line if necessary.
             *
             * @param success      true = successful, false = failed
             */
            CLI_INC
                LIBEXPORT int CallConv			QueryMediatorLogonCommandLineN ( int hInst );


			CLI_INC
				LIBEXPORT void CallConv			SetNetworkStatusN ( int netStat );
			CLI_INC
				LIBEXPORT int CallConv			GetNetworkStatusN ();

			CLI_INC
				LIBEXPORT EBOOL CallConv        ApplyModuleToSettingsN ( int hInst, CString_ptr moduleName, char ** target, int environs_InterfaceType, bool testInstance = true );

			/**
			* Use default encoder, decoder, capture, render modules.
			*
			* @return  success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetUsePortalDefaultModulesN ( int hInst );

			/**
			* Use encoder module with the name moduleName. (libEnv-Enc...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetUseEncoderN ( int hInst, CString_ptr moduleName );

			/**
			* Use decoder module with the name moduleName. (libEnv-Dec...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetUseDecoderN ( int hInst, CString_ptr moduleName );

			/**
			* Use render module with the name moduleName. (libEnv-Rend...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetUseRendererN ( int hInst, CString_ptr moduleName );

			/**
			* Use capture module with the name moduleName. (libEnv-Cap...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetUseCapturerN ( int hInst, CString_ptr moduleName );

			/**
			* Load the file into a byte array that belonging to the fileID and received from the deviceID.
			*  The memory MUST BE FREED by the callee.
			*
			* @param
			* @return
			*/
			CLI_INC
				LIBEXPORT void * CallConv		GetFileNativeN ( int hInst, int nativeID, int fileID, void * buffer, int * capacity );

			/**
			* Load the file into a byte array that belonging to the fileID and received from the deviceID.
			*  The memory MUST BE FREED by the callee.
			*
			* @param
			* @return
			*/
			CLI_INC
				LIBEXPORT void * CallConv		GetFileN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int fileID, void * buffer, int * capacity );

			/**
			* Load the file at given path into a byte array.
			*  The memory MUST BE FREED by the callee.
			*
			* @param
			* @return
			*/
			CLI_INC
				LIBEXPORT char * CallConv		LoadBinaryN ( CString_ptr filePath, int * size );

			/**
			* Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
			*  The resulting memory is managed by Environs.
			*
			* @param    nativeID
			* @param    fileID
			*
			* @return
			*/
			CLI_INC
				LIBEXPORT jstring CallConv GetFilePathNativeN ( int hInst, int nativeID, int fileID );

			/**
			* Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
			*  The resulting memory is managed by Environs.
			*
			* @param   deviceID
			* @param   areaName
			* @param   appName
			* @param   fileID
			*
			* @return
			*/
			CLI_INC
				LIBEXPORT jstring CallConv GetFilePathN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int fileID );

			/**
			* Query the absolute path name to the storage of a given device identity.
			*  The memory MUST BE FREED by the callee.
			*
			* @param    deviceID
			* @param    areaName
			* @param    appName
			*
			* @return
			*/
			CLI_INC
				LIBEXPORT char * CallConv		GetFilePathForStorageN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName );

			CLI_INC
				LIBEXPORT void CallConv			ReleaseStringN ( void * ptr );


			CLI_INC
				LIBEXPORT EBOOL CallConv		GetPortalInfoN ( int hInst, void * buffer );
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetPortalInfoN ( int hInst, void * buffer );

			CLI_INC
				LIBEXPORT void CallConv			FreeNativeMemoryN ( void * obj );

			CLI_INC
				LIBEXPORT void * CallConv		GetDevicesN ( int hInst, int listType );

			CLI_INC
                LIBEXPORT void * CallConv		GetDeviceN ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int fromType );
            CLI_INC
                LIBEXPORT void * CallConv		GetDeviceByObjIDN ( int hInst, OBJIDType objID );
			CLI_INC
				LIBEXPORT void * CallConv		GetDeviceForPortalN ( int hInst, int portalID );

			CLI_INC
				LIBEXPORT int CallConv			GetDevicesCountN ( int hInst, int fromType );

			CLI_INC
				LIBEXPORT void * CallConv		GetDeviceDisplayPropsN ( int hInst, int nativeID );

			CLI_INC
				LIBEXPORT void * CallConv		GetDeviceBestMatchN ( int hInst, int deviceID );

			CLI_INC
				LIBEXPORT int CallConv			GetConnectedDevicesCountN ( int hInst );

			CLI_INC
				LIBEXPORT int CallConv			GetDeviceInstanceSizeN ();
			CLI_INC
				LIBEXPORT int CallConv			GetDevicesHeaderSizeN ();

			//LIBEXPORT void CallConv			deallocNative ();
			CLI_INC
				LIBEXPORT void CallConv			InitStorageN ( CString_ptr path );
			CLI_INC
				LIBEXPORT void CallConv			InitLibDirN ( CString_ptr path );
			CLI_INC
				LIBEXPORT void CallConv			InitWorkDirN ( CString_ptr path );
			CLI_INC
				LIBEXPORT void CallConv			SetGCMN ( CString_ptr gcm );

			CLI_INC
				LIBEXPORT int CallConv			GetDeviceWidthN ( int nativeID );
			CLI_INC
				LIBEXPORT int CallConv			GetDeviceHeightN ( int nativeID );
            

			/**
			* Request a portal stream from the device with the given id.&nbsp;The device must be connected before with deviceConnect ().
			*
			* @param 	nativeID		The native id of the target device.
			* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalDetails	An application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
			* @param 	width       	The width of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
			*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
			* @param 	height      	The height of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
			*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
			*
			* @return 	success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		RequestPortalStreamN ( int hInst, int nativeID, int async, int portalDetails, int width, int height );


			/**
			* Provide a portal stream to the device with the given id.&nbsp;
			* The device must be in a connected state by means of prior call to deviceConnect ().
			*
			* @param 	nativeID		The native id of the target device.
			* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
			*
			* @return success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		ProvidePortalStreamN ( int hInst, int nativeID, int async, int portalDetails );


			/**
			* Provide a portal stream to be requested by the device with the given id.&nbsp;
			* The device must be in a connected state by means of prior call to deviceConnect ().
			*
			* @param 	nativeID		The native id of the target device.
			* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
			*
			* @return success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		ProvideRequestPortalStreamN ( int hInst, int nativeID, int async, int portalDetails );


			/**
			* Find a free portalID slot for the direction encoded into the given portalDetails.
			*
			* @param 	nativeID    	The native device id of the target device.
			* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
			* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
			*/
			CLI_INC
				LIBEXPORT int CallConv			GetPortalIDFreeSlotN ( int hInst, int nativeID, int portalDetails );

			/**
			* Start streaming of portal to or from the portal identifier (received in notification).
			*
			* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
			* @return 	success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		StartPortalStreamN ( int hInst, int async, int portalID );

			/**
			* Pause streaming of portal to or from the portal identifier (received in notification).
			*
			* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
			* 						It is provided within the notification listener as sourceIdent.&nbsp;
			* 						Applications should store them in order to address the correct portal within Environs.
			* @return success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		PausePortalStreamN ( int hInst, int async, int portalID );

			/**
			* Stop streaming of portal to or from the portal identifier (received in notification).
			*
			* @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param nativeID   The native device id of the target device.
			* @param portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
			* 					It is provided within the notification listener as sourceIdent.&nbsp;
			* 					Applications should store them in order to address the correct portal within Environs.
			* @return success
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		StopPortalStreamN ( int hInst, int async, int nativeID, int portalID );

			/**
			* Acquire a native layer send identifier (and increase instanceLock on device) to be used in sendTcpPortal. This resource must be released on disposal of the portal generator.
			*
			* @param deviceID      The device id of the target device.
			* @param areaName		Area name of the application environment
			* @param appName		Application name of the application environment
			* @param portalUnitType	e.g. MSG_TYPE_STREAM
			* @param portalID    Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
			* @return sendID
			*/
			CLI_INC
				LIBEXPORT int CallConv          AcquirePortalSendIDN ( int portalUnitType, int portalID );

			/**
			* Release a native layer send identifier that was acquired by a call to acquireTcpPortalSendID.
			*
			* @param sendID      The portal send id resource to be released.
			*/
			CLI_INC
				LIBEXPORT void CallConv         ReleasePortalSendIDN ( int sendID );

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
			CLI_INC
				LIBEXPORT EBOOL CallConv		SendTcpPortalN ( int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize );
			
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetRenderSurfaceN ( int hInst, int portalID, void * renderSurface, int width, int height );
			CLI_INC
				LIBEXPORT EBOOL CallConv		ReleaseRenderSurfaceN ( int hInst, int async, int portalID );
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetRenderCallbackN ( int hInst, int async, int portalID, void * callback, int type );

			/**
			* Determine whether Environs shall automatically adapt the layout dimensions of
			* the View provided for the portal with the deviceID.
			* The layout dimensions are in particular important for proper mapping of TouchDispatch contact points
			* on the remote portal.
			* If enable is set to false, then custom applications must adapt the layout parameters
			* by means of calling SetPortalViewDimsN().
			*
			* @param enable    A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetPortalViewDimsAutoN ( int hInst, EBOOL enable );

			/**
			* Query the option whether Environs adapts the portal according to the size/location
			* of its view within the layout.
			*
			* @return enabled
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		GetPortalViewDimsAutoN ( int hInst );

			/**
			* Set the location (and its size) of the portal that belongs to the nativeID.
			* Such values are usually provided within the onLayoutChangeListener of a View.
			*
			* @param portalID      The portal device id of the target device.
			* @param left          The left coordinate
			* @param top           The top coordinate
			* @param right         The right coordinate
			* @param bottom        The bottom coordinate
			*
			* @return success		This call will fail, if the touchsource (and portal resources) have not been initialized.
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetPortalViewDimsN ( int hInst, int portalID, int left, int top, int right, int bottom );

			CLI_INC
				LIBEXPORT int CallConv			SetUseTrackerN ( int hInst, int async, CString_ptr moduleName );
			CLI_INC
				LIBEXPORT int CallConv			GetUseTrackerN ( int hInst, CString_ptr moduleName );
			CLI_INC
				LIBEXPORT EBOOL CallConv		DisposeTrackerN ( int hInst, int async, CString_ptr moduleName );

			CLI_INC
				LIBEXPORT EBOOL CallConv		GetTrackerEnabledN ( int hInst, int index );
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetTrackerParamsN ( int hInst, int index, int channels, int width, int height, int stride );
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetTrackerImageN ( int hInst, int index, void * rawImage, int size );
			CLI_INC
				LIBEXPORT EBOOL CallConv		PushTrackerCommandN ( int hInst, int async, int index, int command );



			CLI_INC
				LIBEXPORT EBOOL CallConv		GetPortalNativeResolutionN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetPortalNativeResolutionN ( int hInst, EBOOL enable );

			CLI_INC
				LIBEXPORT EBOOL CallConv		GetPortalAutoStartN ( int hInst );
			CLI_INC
				LIBEXPORT void CallConv			SetPortalAutoStartN ( int hInst, EBOOL enable );


			/**
			* Option for whether to observe wifi networks to help location based services.
			*
			* @param	enable  A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT void CallConv         SetUseWifiObserverN ( EBOOL enable );

			/**
			* Query option for whether to observe wifi networks to help location based services.
			*
			* @return enabled.
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv        GetUseWifiObserverN ();

			/**
			* Determines the interval for scanning of wifi networks.
			*
			* @param	interval  A millisecond value for scan intervals.
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetUseWifiIntervalN ( int interval );

			/**
			* Query interval for scanning of wifi networks.
			*
			* @return interval in milliseconds.
			*/
			CLI_INC
				LIBEXPORT int CallConv			GetUseWifiIntervalN ();

			/**
			* Query array of WifiItems.
			*
			* @return array of WifiItems.
			*/
			CLI_INC
				LIBEXPORT void * CallConv		GetWifisN ();

			/**
			* Query array of BtItems.
			*
			* @return array of BtItems.
			*/
			CLI_INC
				LIBEXPORT void * CallConv		GetBtsN ();

			/**
			* Option for whether to observe blueooth to help location based services.
			*
			* @param	enable  A boolean that determines the target state.
			*/
			CLI_INC
				LIBEXPORT void CallConv         SetUseBtObserverN ( EBOOL enable );

			/**
			* Query option for whether to observe blueooth to help location based services.
			*
			* @return enabled.
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv        GetUseBtObserverN ();

			/**
			* Determines the interval for scanning of bluetooth devices.
			*
			* @param	interval  A millisecond value for scan intervals.
			*/
			CLI_INC
				LIBEXPORT void CallConv			SetUseBtIntervalN ( int interval );

			/**
			* Query interval for scanning of bluetooth devices.
			*
			* @return interval in milliseconds.
			*/
			CLI_INC
				LIBEXPORT int CallConv			GetUseBtIntervalN ();

#ifdef __cplusplus
		}
#endif

#endif // ANDROID - include only the jni-exports

	}
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_API_H



