/**
 * Environs.h
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT__IMPL_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT__IMPL_H

#include "Environs.Platforms.h"
#include "Environs.Platform.Decls.h"
#include "Environs.Build.Opts.h"
#include "Location.Decl.h"

/** Include iOSX declarations */

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )
#   if ( defined(__OBJC__) )
#       import <Environs.iOSX.h>
#   endif
#endif


/** Declare CPP API if it has not been disabled by symbol */

#if ( defined(__cplusplus) && !defined(DISABLE_ENVIRONS_CPP_API))

#	include "Notify.Context.h"
#	include "Notify.Context.Cli.h"
#	include "Environs.Observer.CLI.h"
#	include "Environs.Msg.Types.h"
#	include "Environs.Cli.Base.h"
#	include "Environs.Cpp.Base.h"

#ifndef CLI_CPP
#	include "Interfaces/IEnvirons.h"
#	include "Environs.Loader.h"
#	include "Environs.Observer.h"
#	include "Device.List.h"
#	include "Device.Instance.h"
#	include "File.Instance.h"
#	include "Message.Instance.h"
#	include "Portal.Instance.h"
#	include "Environs.Cpp.Base.h"

#	include <queue>
#	include "Queue.List.h"
#	include "Queue.Vector.h"

#else
#	include "Environs.Platforms.h"
#	include "Interop/Smart.Pointer.h"
#	include "Interfaces/IEnvirons.Dispose.h"
#	include "Environs.Types.h"
#	include "Device.Info.h"
#	include "Environs.Types.h.Cli.h"
#	include "Environs.Types.h"

	using namespace System;
	using namespace System::Collections::ObjectModel;
	using namespace System::Runtime::InteropServices;
#endif
	

namespace environs
{
#ifndef CLI_CPP    
    class EnvironsNative;
#endif
        
    extern bool IsUIThread ();
    
	namespace lib
	{
		CLASS Environs;

		CLASS ListCommandContext C_Only ( : public IQueueItem )
		{
        public:
            
#ifdef DEBUG_TRACK_LIST_NOTIFIER_CONTEXT
            ListCommandContext ();
            ~ListCommandContext ();
#endif
			int					hEnvirons;
			EnvironsPtr         envObj;
			OBJIDType			destID;
			int					type;

			STRING_T			areaName;
			STRING_T			appName;
			int					sourceIdent;
#ifndef CLI_CPP
			spvc ( lib::IIListObserver OBJ_ptr ) observerList;
#else
			spv ( lib::IIEnvironsObserver OBJ_ptr ) observerList;
#endif
            int                 notification;
            
            environs::DeviceClass_t     listType;

			sp ( environs::DeviceInfo ) device;
		};
        
        
        PUBLIC_CLASS ListContext
        {
		public:
#ifdef CLI_CPP
			ListContext() { lock = gcnew Object(); };
#endif
            environs::DeviceClass_t             type;
            pthread_mutex_t  cli_OBJ_ptr        lock;
            
            NLayerVecType ( DeviceInstanceEP )  vanished;
            NLayerVecType ( DeviceInstanceEP )  appeared;
        };
        

		PUBLIC_CLASS Environs DERIVE_c_only ( environs::Environs ) DERIVE_DISPOSABLE DERIVE_TYPES
		{
            MAKE_FRIEND_CLASS ( environs::EnvironsNative );
            MAKE_FRIEND_CLASS ( DeviceList );
            MAKE_FRIEND_CLASS ( PortalInstance );
			MAKE_FRIEND_CLASS ( DeviceInstance );
            MAKE_FRIEND_CLASS ( EnvironsProxy );
            MAKE_FRIEND_CLASS ( FactoryProxy );
            MAKE_FRIEND_CLASS ( EnvironsNative );

        public:
            
#ifdef CLI_CPP
            /** Perform calls to the Environs object asynchronously. If set to Environs.CALL_WAIT, then all commands will block until the call finishes.
             * If set to Environs.CALL_NOWAIT, then certain calls (which may take longer) will be performed asynchronously. */
            Environs::Call					async;
#endif
			/**
			 * Construction of Environs objects have to be done using Environs.CreateInstance() or Environs.New()
			 */
			ENVIRONS_LIB_API Environs ();

			ENVIRONS_LIB_API ~Environs ();

			/**
			* Load settings for the given application environment from settings storage,
			* if any have been saved previously.
			*
			* @param	hInst		The handle to a particular native Environs instance.
			* @param 	appName		The application name for the application environment.
			* @param  	areaName	The area name for the application environment.
			*
			* @return   success
			*/
			ENVIRONS_LIB_API bool LoadSettings ( CString_ptr appName, CString_ptr areaName );


			/**
			 * Load settings. Prior to this call, an application environment MUST be given
			 * using SetApplicationName and SetAreaName.
			 *
			 * @return   success
			 */
			ENVIRONS_LIB_API bool LoadSettings ();


			/**
			* Instruct Environs to output verbose debug logging.
			*
			* @param level      debug level 0 ... 16
			*/
            ENVIRONS_LIB_API void SetDebug ( int level );
            
            
            /**
             * Instructs the framework to perform a quick shutdown (with minimal wait times)
             *
             * @param enable      true / false
             */
            ENVIRONS_LIB_API void SetAppShutdown ( bool enable );
            
            
            /**
             * Dispose the storage, that is remove all data and messages received in the data store.
             *
             */
            ENVIRONS_LIB_API void ClearStorage ();
            
            
            /**
             * Get output debug level.
             *
             * @return level      debug level 0 ... 16
             */
            ENVIRONS_LIB_API int GetDebug ();

			/**
			* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
			* Increasing this value may help to handle worse networks which suffer from large latencies.
			*
			* @param   timeout
			*/
			ENVIRONS_LIB_API void SetNetworkConnectTimeout ( int timeout );
            
            
            /**
             * Get platform that the app is running on.
             *
             * @return 	enum of type Environs.Platforms
             */
            ENVIRONS_LIB_API int GetPlatform ();
            
            /**
             * Set the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
             * Valid type are enumerated in Environs.Platforms.*
             *
             * @param	platform	Environs.Platforms.*
             */
            ENVIRONS_LIB_API void SetPlatform ( int platform );
            
            /**
             * Set/Remove the location-node flag to the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
             * Flag: Environs.Platforms.LocationNode_Flag
             *
             * @param	isLocationNode	true or false
             */
            ENVIRONS_LIB_API void SetIsLocationNode ( bool isLocationNode );
            
			ENVIRONS_LIB_API Location cli_OBJ_ptr GetLocation ();
            
            /**
             * Enable or disable device list update notifications from Mediator layer.
             * In particular, mobile devices should disable notifications if the devicelist is not
             * visible to users or the app transitioned to background.
             * This helps recuding cpu load and network traffic when not required.
             *
             * @param enable      true = enable, false = disable
             */
            ENVIRONS_LIB_API void SetMediatorNotificationSubscription ( bool enable );
            
            
            /**
             * Get subscription status of device list update notifications from Mediator layer.
             *
             * @return enable      true = enable, false = disable
             */
            ENVIRONS_LIB_API bool GetMediatorNotificationSubscription ( );
            
            
            /**
             * Enable or disable short messages from Mediator layer.
             * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
             * This helps recuding cpu load and network traffic when not necessary.
             *
             * @param enable      true = enable, false = disable
             */
            ENVIRONS_LIB_API void SetMessagesSubscription ( bool enable );
            
            
            /**
             * Get subscription status of short messages from Mediator layer.
             *
             * @return enable      true = enable, false = disable
             */
            ENVIRONS_LIB_API bool GetMessagesSubscription ( );


			/**
			* Ignore autodetection of the actual runtime platform and enforce the given platform.
			*
			* @param		platform of type Environs.platform
			*/
			ENVIRONS_LIB_API void SetPlatform ( environs::Platforms_t platform );


			/**
			 * Instruct Environs to show log messages in the status log.
			 *
			 * @param enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseNotifyDebugMessage ( int enable );


			/**
			 * Query Environs settings that show log messages in the status log.
			 *
			 * @return enable      true = enabled, false = disabled
			 */
			ENVIRONS_LIB_API bool GetUseNotifyDebugMessage ();


			/**
			 * Instruct Environs to create and write a log file in the working directory.
			 *
			 * @param enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseLogFile ( bool enable );


			/**
			 * Query Environs settings that create and write a log file in the working directory.
			 *
			 * @return enable      true = enabled, false = disabled
			 */
            ENVIRONS_LIB_API bool GetUseLogFile ();
            
            
            /**
             * Instruct Environs to log to stdout.
             *
             * @param enable      true = enable, false = disable
             */
            ENVIRONS_LIB_API void SetUseLogToStdout ( bool enable );
            
            
            /**
             * Query Environs settings whether to log to stdout.
             *
             * @return enable      true = enabled, false = disabled
             */
            ENVIRONS_LIB_API bool GetUseLogToStdout ();


			/**
			* Instruct Environs to use headless mode without worrying about UI thread.
			*
			* @param enable      true = enable, false = disable
			*/
			ENVIRONS_LIB_API void SetUseHeadless ( bool enable );


			/**
			* Query Environs settings whether to use headless mode without worrying about UI thread.
			*
			* @return enable      true = enabled, false = disabled
			*/
			ENVIRONS_LIB_API bool GetUseHeadless ();
            
            
            /**
             * Check for mediator logon credentials and query on command line if necessary.
             *
             * @param success      true = successful, false = failed
             */
            ENVIRONS_LIB_API bool QueryMediatorLogonCommandLine ();


			/**
			* Instruct Environs to create DeviceLists that are used as UIAdapter by client code.
			* Any changes of those lists are made within the applications main / UI thread context.
			* Only DeviceList objects that are created after this call are affected.
			* DeviceList objects created before this call remain using the setting at the time they are created.
			*
			* @param enable      true = enable, false = disable
			*/
			ENVIRONS_LIB_API void SetUseDeviceListAsUIAdapter ( bool enable );


			/**
			* Query Environs settings whether to create DeviceLists that are used as UIAdapter by client code.
			* Any changes of those lists are made within the applications main / UI thread context.
			*
			* @return enable      true = enabled, false = disabled
			*/
            ENVIRONS_LIB_API bool GetUseDeviceListAsUIAdapter ();
            
            
            /** Default value for each DeviceInstance after object creation. */
            ENVIRONS_LIB_API bool GetAllowConnectDefault ();
            
            /** Default value for each DeviceInstance after object creation. */
            ENVIRONS_LIB_API void SetAllowConnectDefault ( bool value );


			/**
			 * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
			 * Tasks:
			 *  - Request display always on state, hence disable standby and power management functions.
			 *  - Load sensor managers and sensor services
			 *
			 * @return success    true = success, false = failed
			 */
			ENVIRONS_LIB_API bool Init ();

            
            /**
             * Reset crypt layer and all created resources. Those will be recreated if necessary.
             * This method is intended to be called directly after creation of an Environs instance.
             *
             */
			ENVIRONS_LIB_API void ResetCryptLayer ();


			/**
			 * Get the native version of Environs.
			 *
			 * @return		version string
			 */
            ENVIRONS_LIB_API CString_ptr GetVersionString ();
            
            
            /**
             * Release memory allocated by Environs to be temporarily used by client code.
             *
             * @param		mem
             */
            ENVIRONS_LIB_API void ReleaseEnvMemory ( Addr_obj mem );


			/**
			 * Get the native major version of Environs.
			 *
			 * @return		major version
			 */
			ENVIRONS_LIB_API int GetVersionMajor ();

			/**
			 * Get the native minor version of Environs.
			 *
			 * @return		minor version
			 */
			ENVIRONS_LIB_API int GetVersionMinor ();

			/**
			 * Get the native revision of Environs.
			 *
			 * @return		revision
			 */
			ENVIRONS_LIB_API int GetVersionRevision ();


			/**
			 * Query whether the native layer was build for release (or debug).
			 *
			 * @return	true = Release build, false = Debug build.
			 */
			ENVIRONS_LIB_API bool GetIsReleaseBuild ();


			/**
			 * Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
			 * Valid type are enumerated in Environs.DEVICE_TYPE_*
			 * --> Deprecated <--
			 *
			 * @param	type	Environs.DEVICE_TYPE_*
			 */
			//ENVIRONS_LIB_API void SetDeviceType ( char value );


			/**
			 * Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
			 * Valid type are enumerated in Types.DEVICE_TYPE_*
			 * --> Deprecated <--
			 *
			 * @return	type	Environs.DEVICE_TYPE_*
			 */
			//ENVIRONS_LIB_API char GetDeviceType ();


			/**
			 * Set the ports that the local instance of Environs shall use for listening on connections.
			 *
			 * @param	tcpPort communication channel
			 * @param	udpPort data channel
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetPorts ( int tcpPort, int udpPort );


            /**
             * Set the base port that the local instance of Environs shall use for communication with other instances.
             * This option enables spanning of multiple multi surface environsments separated by the network stacks.
             *
             * @param	port The base port.
             * @return success
             */
            ENVIRONS_LIB_API bool SetBasePort ( int port );


			ENVIRONS_LIB_API unsigned int GetIPAddress ();
			ENVIRONS_LIB_API unsigned int GetSubnetMask ();

			ENVIRONS_LIB_API CString_ptr GetSSID ();
			ENVIRONS_LIB_API CString_ptr GetSSIDDesc ();


			/**
			 * Set the device id that is assigned to the instance of Environs.
			 *
			 * @param   deviceID
			 */
			ENVIRONS_LIB_API void SetDeviceID ( int deviceID );


			/**
			 * Get the device id that the application has assigned to the instance of Environs.
			 *
			 * @return	deviceID
			 */
			ENVIRONS_LIB_API int GetDeviceID ();


			/**
			 * Request a device id from mediator server instances that have been provided before this call.
			 * Prior to this call, the area and application name must be provided as well,
			 * in order to get an available device id for the specified application environment.
			 * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
			 *
			 * @return	deviceID
			 */
			ENVIRONS_LIB_API int GetDeviceIDFromMediator ();


			/**
			 * Query whether the name of the current device has been set before.
			 *
			 * @return	has DeviceUID
			 */
			ENVIRONS_LIB_API bool HasDeviceUID ();


			/**
			 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			 *
			 * @param 	name    A unique device identifier.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetDeviceUID ( CString_ptr name );


			/**
			 * Query ip of custom Mediator.
			 *
			 * @return ip
			 */
			ENVIRONS_LIB_API unsigned int GetMediatorIPValue ();


			/**
			 * Query ip of custom Mediator.
			 *
			 * @return ip
			 */
			ENVIRONS_LIB_API CString_ptr GetMediatorIP ();


			/**
			 * Query port of custom Mediator.
			 *
			 * @return port
			 */
			ENVIRONS_LIB_API int GetMediatorPort ();


			/**
			 * Determines whether to use Crypto Layer Security for Mediator connections.
			 * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForMediator ( bool enable );


			/**
			 * Query whether to use Crypto Layer Security for Mediator connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForMediator ();


			/**
			 * Determines whether to use Crypto Layer Security for device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForDevices ( bool enable );


			/**
			 * Query whether to use Crypto Layer Security for device-to-device connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForDevices ();


			/**
			 * Determines whether to enforce Crypto Layer Security for device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForDevicesEnforce ( bool enable );


			/**
			 * Query whether to enforce Crypto Layer Security for device-to-device connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForDevicesEnforce ();


			/**
			 * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForAllTraffic ( bool enable );


			/**
			 * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForAllTraffic ();


			/**
			 * Determines whether to use environs default Mediator predefined by framework developers or not.
			 *
			 * @param enable 	true = use the default Mediator
			 */
			ENVIRONS_LIB_API void SetUseDefaultMediator ( bool enable );


			/**
			 * Query whether to use given Mediator by setMediator()
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUseDefaultMediator ();


			/**
			 * Determines whether to use given Mediator by setMediator()
			 *
			 * @param enable 	true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseCustomMediator ( bool enable );


			/**
			 * Query whether to use given Mediator by setMediator()
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUseCustomMediator ();


			/**
			 * Set custom Mediator to use.
			 *
			 * @param ip
			 * @param port
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediator ( CString_ptr ip, unsigned short port );


			/**
			 * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
			 *
			 * @param 	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediatorUserName ( CString_ptr name );
			ENVIRONS_LIB_API bool SetUserName ( CString_ptr name );


			/**
			 * Query UserName used to authenticate with a Mediator.
			 *
			 * @return UserName
			 */
			ENVIRONS_LIB_API CString_ptr GetMediatorUserName ();


			/**
			 * Enable or disable anonymous logon to the Mediator.
			 *
			 * @param 	enable A boolean that determines the target state.
			 */
			ENVIRONS_LIB_API void SetUseMediatorAnonymousLogon ( bool enable );


			/**
			 * Get setting of anonymous logon to the Mediator.
			 *
			 * @return 	enabled A boolean that determines the target state.
			 */
			ENVIRONS_LIB_API bool GetUseMediatorAnonymousLogon ();


			/**
			 * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
			 *
			 * @param 	password
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediatorPassword ( CString_ptr password );
			ENVIRONS_LIB_API bool SetUserPassword ( CString_ptr password );


			/**
			 * Enable or disable authentication with the Mediator using username/password.
			 *
			 * @param 	enable
			 */
			ENVIRONS_LIB_API void SetUseAuthentication ( bool enable );


			/**
			 * Query the filter level for device management within Environs.
			 *
			 * return   level	can be one of the values of MediatorFilter
			 */
			ENVIRONS_LIB_API environs::MediatorFilter_t GetMediatorFilterLevel ();


			/**
			 * Set the filter level for device management within Environs.
			 *
			 * @param   level	can be one of the values of MediatorFilter
			 */
			ENVIRONS_LIB_API void SetMediatorFilterLevel ( environs::MediatorFilter_t level );


			/**
			 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
			 *
			 * @return	true = yes, false = no
			 */
			ENVIRONS_LIB_API bool GetUseMediatorLoginDialog ();


			/**
			 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
			 *
			 * @param   enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseMediatorLoginDialog ( bool enable );


			/**
			 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
			 *
			 * @return	true = yes, false = no
			 */
			ENVIRONS_LIB_API bool GetMediatorLoginDialogDismissDisable ();


			/**
			* Instruct Environs disable Mediator settings on dismiss of the login dialog.
			*
			 * @param   enable      true = enable, false = disable
			*/
			ENVIRONS_LIB_API void SetMediatorLoginDialogDismissDisable ( bool enable );


			/**
			 * Register at known Mediator server instances.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RegisterAtMediators ();


            /**
             * Determines whether touch events should be translated to mouse events. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
             *
             * @param   enable      true = enable, false = disable
             */
			ENVIRONS_LIB_API void SetUseMouseEmulation ( bool enable );


            /**
             * Determines whether touch events should be visualized as rounded circles on the desktop Window. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
             *
             * @param   enable      true = enable, false = disable
             */
			ENVIRONS_LIB_API void SetUseTouchVisualization ( bool enable );

            /**
			 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
			 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
			 * 		this call executes the start tasks within a thread.&nbsp;
			 * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
			 *
			 */
			ENVIRONS_LIB_API void Start ();


			/**
			 * Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
			 *
			 * @return  Environs.Status
			 */
			ENVIRONS_LIB_API environs::Status_t GetStatus ();


			/**
			 * Stop Environs and release all acquired resources.
			 */
			ENVIRONS_LIB_API void Stop ();


			/**
			 * Stop Environs and release all acquired resources.
			 */
			ENVIRONS_LIB_API void DisposeInstance ();


			/**
			 * Set the area name that the local instance of Environs shall use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @param	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetAreaName ( CString_ptr name );

			/**
			 * Get the area name that the local instance of Environs use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @return	areaName
			 */
			ENVIRONS_LIB_API CString_ptr GetAreaName ();


			/**
			 * Set the application name of that the local instance of Environs shall use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @param	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API void SetApplication ( CString_ptr name );
			ENVIRONS_LIB_API void SetApplicationName ( CString_ptr name );


			/**
			 * Get the application name that the local instance of Environs use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @return	appName
			 */
			ENVIRONS_LIB_API CString_ptr GetApplicationName ();


			/**
			 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			 *
			 * @param 	deviceName
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetDeviceName ( CString_ptr name );


			/**
			 * Use default encoder, decoder, capture, render modules.
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetUsePortalDefaultModules ();


			ENVIRONS_LIB_API void SetUseH264 ( bool enable );
			ENVIRONS_LIB_API bool GetUseH264 ();


			/**
			 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
			 *
			 * @param   enable
			 */
			ENVIRONS_LIB_API void SetUsePortalTCP ( bool enable );


			/**
			 * Query whether to use TCP for portal streaming (UDP otherwise)
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUsePortalTCP ();


			/**
			 * Use encoder module with the name moduleName. (libEnv-Enc...).
			 *
			 * @param	name  The module name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseEncoder ( CString_ptr name );


			/**
			 * Use decoder module with the name moduleName. (libEnv-Dec...).
			 *
			 * @param	name  The module name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseDecoder ( CString_ptr name );


			/**
			 * Use capture module with the name moduleName. (libEnv-Cap...).
			 *
			 * @param	name	the name of the module
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseCapturer ( CString_ptr name );


			/**
			 * Use render module with the name moduleName. (libEnv-Rend...).
			 *
			 * @param	name	the name of the module
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseRenderer ( CString_ptr name );

			/**
			 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
			 *
			 * @param	moduleName  The module name
			 * @param	enable      Enable or disable
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetUseTouchRecognizer ( CString_ptr name, bool enable );


			ENVIRONS_LIB_API int SetUseTracker ( Call_t async, CString_ptr name );

			ENVIRONS_LIB_API int GetUseTracker ( CString_ptr name );

			ENVIRONS_LIB_API bool DisposeTracker ( Call_t async, CString_ptr name );

			ENVIRONS_LIB_API bool PushTrackerCommand ( Call_t async, int moduleIndex, int command );

#ifndef CLI_CPP
			/**
			 * Add an observer for communication with Environs and devices within the environment.
			 *
			 * @param   observer Your implementation of EnvironsObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserver ( environs::EnvironsObserver OBJ_ptr observer );

			/**
			 * Remove an observer for communication with Environs and devices within the environment.
			 *
			 * @param   observer Your implementation of EnvironsObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserver ( environs::EnvironsObserver OBJ_ptr observer );

			/**
			 * Add an observer for receiving messages.
			 *
			 * @param   observer Your implementation of EnvironsMessageObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForMessages ( environs::EnvironsMessageObserver * observer );

			/**
			 * Remove an observer for receiving messages.
			 *
			 * @param   observer Your implementation of EnvironsMessageObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForMessages ( environs::EnvironsMessageObserver * observer );

			/**
			 * Add an observer for receiving data buffers and files.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForData ( environs::EnvironsDataObserver * observer );

			/**
			 * Remove an observer for receiving data buffers and files.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForData ( environs::EnvironsDataObserver * observer );

			/**
			 * Add an observer for receiving sensor data of all devices.
			 * Please note: This observer reports sensor data of all devices that are connected and send to us.
			 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
			 *
			 * @param   observer Your implementation of EnvironsSensorObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForSensorData ( environs::EnvironsSensorObserver * observer );

			/**
			 * Remove an observer for receiving data buffers and files.
			 * Please note: This observer reports sensor data of all devices that are connected and send to us.
			 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
			 *
			 * @param   observer Your implementation of EnvironsSensorObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForSensorData ( environs::EnvironsSensorObserver * observer );
#endif

			/**
			 * Create a new DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
			 * After client code is done with the list, the list->Release () method MUST be called by the client code,
			 * in order to release the resource (give ownership) back to Environs.
			 *
			 * @param MEDIATOR_DEVICE_CLASS_	A value of type environs::DeviceClass that determines the list type

			 * @return A DeviceList object
			 */
			ENVIRONS_LIB_API EPSPACE DeviceList OBJ_ptr CreateDeviceList ( environs::DeviceClass_t MEDIATOR_DEVICE_CLASS_ );

#ifndef CLI_CPP
			/**
			 * Create a new DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
			 * After client code is done with the list, the list->Release () method MUST be called by the client code,
			 * in order to release the resource (give ownership) back to Environs.
			 *
			 * @param MEDIATOR_DEVICE_CLASS_	A value of type environs::DeviceClass that determines the list type

			 * @return An interface of type IDeviceList
			 */
			ENVIRONS_LIB_API DeviceList OBJ_ptr CreateDeviceListRetained ( environs::DeviceClass_t MEDIATOR_DEVICE_CLASS_ );
#endif

			ENVIRONS_LIB_API bool GetPortalNativeResolution ();
			ENVIRONS_LIB_API void SetPortalNativeResolution ( bool enable );

			ENVIRONS_LIB_API bool GetPortalAutoStart ();
			ENVIRONS_LIB_API void SetPortalAutoStart ( bool enable );

			ENVIRONS_LIB_API DeviceDisplay OBJ_ptr GetDeviceDisplayProps ( int nativeID );
            
            
			/**
			* Connect to device with the given ID and a particular application environment.
			*
			* @param deviceID	Destination device ID
			* @param areaName	Project name of the application environment
			* @param appName	Application name of the application environment
			* @param async		(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @return status	0: Connection can't be conducted (maybe environs is stopped or the device ID is invalid) &nbsp;
			* 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
			* 					2: A new connection has been triggered and is in progress
			*/
			ENVIRONS_LIB_API int DeviceConnect ( int deviceID, CString_ptr areaName, CString_ptr appName, Call_t async );


			/**
			* Set render callback.
			*
			* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
			* @param callback		The pointer to the callback.
			* @param callbackType	A value of type Environs.RenderCallbackType that tells the portal receiver what we actually can render..
			* @return				true = success, false = failed.
			*/
#ifdef CLI_CPP
			bool SetRenderCallback ( Call_t async, int portalID, PortalSinkSource ^ callback, RenderCallbackType_t callbackType );
#else
			ENVIRONS_LIB_API bool SetRenderCallback ( Call_t async, int portalID, void * callback, RenderCallbackType_t callbackType );
#endif


			/**
			* Release render callback delegate or pointer
			*
			* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
			* @param callback		A delegate that manages the callback.
			* @return				true = success, false = failed.
			*/
			ENVIRONS_LIB_API bool ReleaseRenderCallback ( Call_t async, int portalID );

			/**
			* Start streaming of portal to or from the portal identifier (received in notification).
			*
			* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
			*
			* @return success
			*/
			ENVIRONS_LIB_API bool StartPortalStream ( Call_t async, int portalID );


			/**
			* Stop streaming of portal to or from the portal identifier (received in notification).
			*
			* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @param 	nativeID    The native device id of the target device.
			* @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
			* 						It is provided within the notification listener as sourceIdent.&nbsp;
			* 					    Applications should store them in order to address the correct portal within Environs.
			* @return success
			*/
			ENVIRONS_LIB_API bool StopPortalStream ( Call_t async, int nativeID, int portalID );


			/**
			* Get the status, whether the device (id) has established an active portal
			*
			* @param 	nativeID    The device id of the target device.
			* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
            *
			* @return	success 	true = yes, false = no
			*/
			ENVIRONS_LIB_API bool GetPortalEnabled ( int nativeID, int portalType );

            
            /**
             * Get the number of devices that are currently connected to our device.
             *
             * @return	Count of connected devices
             */
            ENVIRONS_LIB_API int GetConnectedDevicesCount ();


            /**
             * Get enabled status for stream encoding.
             *
             * @return	enabled
             */
			ENVIRONS_LIB_API bool GetUseStream ();

            
            /**
             * Get platform support for OpenCL.
             *
             * @return	enabled
             */
			ENVIRONS_LIB_API bool GetUseOpenCL ();


            /**
             * Switch platform support for OpenCL rendering.
             *
             * @param enable
             */
            ENVIRONS_LIB_API void SetUseOpenCL ( bool enable );


            /**
             * Option for whether to observe wifi networks to help location based services.
             *
             * @param	enable  A boolean that determines the target state.
             */
            ENVIRONS_LIB_API void SetUseWifiObserver ( bool enable );


            /**
             * Query option for whether to observe wifi networks to help location based services.
             *
             * @return enabled.
             */
            ENVIRONS_LIB_API bool GetUseWifiObserver ();

			/**
			* Determines the interval for scanning of wifi networks.
			*
			* @param	interval  A millisecond value for scan intervals.
			*/
            ENVIRONS_LIB_API void SetUseWifiInterval ( int interval );

            /**
             * Get the interval for scanning of wifi networks.
             *
             * @return	interval  A millisecond value for scan intervals.
             */
            ENVIRONS_LIB_API int GetUseWifiInterval ();
            
#ifndef CLI_CPP
            /**
             * Get a collection that holds all available wifi APs. This list is NOT updated dynamically.
             *
             * @return WifiList with WifiItem objects
             */
            ENVIRONS_LIB_API WifiList * GetWifisRetained ();

			/**
			* Get a collection that holds all available Bluetooth devices. This list is NOT updated dynamically.
			*
			* @return BtList with BtItem objects
			*/
			ENVIRONS_LIB_API BtList * GetBtsRetained ();
#endif
            /**
             * Option for whether to observe blueooth to help location based services.
             *
             * @param	enable  A boolean that determines the target state.
             */
            ENVIRONS_LIB_API void SetUseBtObserver ( bool enable );
            
            
            /**
             * Query option for whether to observe blueooth to help location based services.
             *
             * @return enabled.
             */
            ENVIRONS_LIB_API bool GetUseBtObserver ();

			/**
			* Determines the interval for scanning of bluetooth devices.
			*
			* @param	interval  A millisecond value for scan intervals.
			*/
            ENVIRONS_LIB_API void SetUseBtInterval ( int interval );

            /**
             * Get the interval for scanning of bluetooth devices.
             *
             * @return	interval  A millisecond value for scan intervals.
             */
            ENVIRONS_LIB_API int GetUseBtInterval ();
            
            
            
			ENVIRONS_LIB_API CString_ptr GetFilePathNative ( int nativeID, int fileID );
            

            ENVIRONS_LIB_API String_ptr GetFilePath ( int nativeID, int fileID );

            
			/**
			* Load the file that is assigned to the fileID into a byte array.
			*
			* @param nativeIDOK		Indicates that the nativeOrDeviceID represents a nativeID.
			* @param nativeID		The native id of the device.
			* @param deviceID		The device id of the device.
			* @param areaName		Area name of the application environment.
			* @param appName		Application name of the application environment.
			* @param fileID			The id of the file to load (given in the onData receiver).
			* @param size			An int pointer, that receives the size of the returned buffer.
			* @return byte-array
			*/
			ENVIRONS_LIB_API UCharArray_ptr GetFile ( bool nativeIDOK, int nativeID, int deviceID, CString_ptr areaName, CString_ptr appName, int fileID, int OBJ_ref size );
			

            ENVIRONS_LIB_API int GetHandle () { return hEnvirons; };

#ifndef CLI_CPP
			ENVIRONS_LIB_API void * LoadPicture ( const char * filePath );
#endif
            /**
             * Enable dispatching of sensor events from ourself.
             * Events are send if Environs instance is started stopped if the Environs instance has stopped.
             *
             * @param sensorType            A value of type environs.SensorType.
             * @param enable 				true = enable, false = disable.
             *
             * @return success true = enabled, false = failed.
             */
            bool SetSensorEvent ( environs::SensorType_t sensorType, bool enable );


            /**
             * Determine whether the given sensorType is available.
             *
             * @param sensorType A value of type environs::SensorType_t.
             *
             * @return success true = available, false = not available.
             */
            bool IsSensorAvailable ( environs::SensorType_t sensorType );


            /**
             * Set use of Tcp transport channel of the given sensorType.
             *
             * @param sensorType    A value of type environs::SensorType_t.
             * @param enable        true = TCP, false = UDP.
             *
             */
            void SetUseSensorChannelTcp ( environs::SensorType_t sensorType, bool enable );


            /**
             * Get use of Tcp transport channel of the given sensorType.
             *
             * @param sensorType    A value of type environs::SensorType_t.
             * @return success      1 = TCP, 0 = UDP, -1 = error.
             *
             */
            int GetUseSensorChannelTcp ( environs::SensorType_t sensorType );


            /**
             * Set sample rate of the given sensorType in microseconds.
             *
             * @param sensorType        A value of type environs::SensorType_t.
             * @param microseconds      The sensor sample rate in microseconds.
             *
             */
            void SetUseSensorRate ( environs::SensorType_t sensorType, int microseconds );


            /**
             * Get sample rate of the given sensorType in microseconds.
             *
             * @param sensorType        A value of type environs::SensorType_t.
             *
             * @return microseconds     The sensor sample rate in microseconds. -1 means error.
             */
            int GetUseSensorRate ( environs::SensorType_t sensorType );


            /**
             * Release ownership on this interface and mark it disposable.
			 * Release must be called once for each Interface that the Environs framework returns to client code.
			 * Environs will dispose the underlying object if no more ownership is hold by anyone.
			 *
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		INTERNAL:
			bool                                            allocated;

#ifndef CLI_CPP
			Instance                                    *   env;
        
            static Environs                             *   instancesAPI [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];
#endif
			int                                             hEnvirons;

			STRING_T                                        envAppName;
            STRING_T                                        envAreaName;
            
            /** Default value for each DeviceInstance after object creation. */
            bool                                            allowConnectDefault;

			void			SetInstance ( int hInst );

			virtual bool	InitPlatform ();

#ifndef CLI_CPP
			spv ( lib::IIEnvironsObserver * )             environsObservers;

			spv ( lib::IIEnvironsMessageObserver * )      environsObserversForMessages;

			spv ( lib::IIEnvironsDataObserver * )         environsObserversForData;

			spv ( lib::IIEnvironsSensorObserver * )       environsObserversForSensor;
#endif
			/**
			* BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
			* 		such as when a connection has been established or closed.
			*
			* @param hInst			A handle to the Environs instance
			* @param objID          The native device id of the sender device.
			* @param notification  The received notification.
			* @param sourceIdent   A value of the enumeration type Types.EnvironsSource
            * @param contextPtr    A value that provides additional context information (if available).
            * @param context       A value that provides additional context information (if available).
			*/
			CLI_NO_STATIC void BridgeForNotify ( int hInst, OBJIDType objID, int notification, int sourceIdent, Addr_obj contextPtr, int context );


			/**
			* BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
			* 		such as when a connection has been established or closed.
			*
			* @param hInst			A handle to the Environs instance
			* @param deviceID      The device id of the sender device.
			* @param areaName		Area name of the application environment.
			* @param appName		Application name of the application environment.
			* @param notification  The received notification.
			* @param sourceIdent   A value of the enumeration type Types.EnvironsSource
			* @param context       A value that provides additional context information (if available).
			*/
			CLI_NO_STATIC void BridgeForNotifyExt ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int notification, int sourceIdent, Addr_obj contextPtr );


			/**
			* BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
			*
			* @param hInst			A handle to the Environs instance
			* @param objID          The native device id of the sender device.
			* @param type          The type of this message.
			* @param msg           The message.
			* @param length        The length of the message.
			*/
			CLI_NO_STATIC void BridgeForMessage ( int hInst, OBJIDType objID, int type, CVString_ptr message, int length );


			/**
			* BridgeForMessageExt static method to be called by native layer in order to notify about incoming messages.
			*
			* @param hInst			A handle to the Environs instance
			* @param deviceID      The device id of the sender device.
			* @param areaName		Area name of the application environment
			* @param appName		Application name of the application environment
			* @param type          The type of this message.
			* @param msg           The message.
			* @param length        The length of the message.
			*/
			CLI_NO_STATIC void BridgeForMessageExt ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int type, CVString_ptr message, int length );


			/**
			* BridgeForStatusMessage static method to be called by native layer in order to drop a status messages.
			*
			* @param hInst			A handle to the Environs instance
			* @param msg           A status message of Environs.
			*/
			CLI_NO_STATIC void BridgeForStatusMessage ( int hInst, CString_ptr message );


			/**
			* BridgeForData static method to be called by native layer in order to notify about data received from a device.
			*
			* @param hInst			A handle to the Environs instance
			* @param objID			The object id of the sender device.
			* @param nativeID		The native device id of the sender device.
			* @param type			The type of this message.
			* @param fileID			A fileID that was attached to the buffer.
			* @param descriptor		A descriptor that was attached to the buffer.
			* @param size          The size of the data buffer.
			*/
			CLI_NO_STATIC void BridgeForData ( int hInst, OBJIDType objID, int nativeID, int type, int fileID, CString_ptr descriptor, int size );
            
            
            /**
             * BridgeForUdpData static method to be called by native layer in order to notify about udp data received from a device.
             *
             * @param hInst			A handle to the Environs instance
             * @param objID         The native device id of the sender device.
             * @param pack          A udp data structure containing the received udp or sensor data.
             * @param packSize      The size of the data buffer in number of bytes.
             */
			CLI_NO_STATIC void BridgeForUdpData ( int hInst, OBJIDType objID, Addr_obj pack, int packSize );

			pthread_mutex_t             queryLock;
			pthread_mutex_t             listLock;
			pthread_mutex_t             listCommandThreadLock;

			long                        listAllUpdate;
			long                        listNearbyUpdate;
			long                        listMediatorUpdate;

            bool                        isUIAdapter;
			pthread_mutex_t             listAllLock;
            
            
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
        public:
            devList ( DeviceInstanceEP )listAll;
        private:
#else
            devList ( DeviceInstanceEP )listAll;
#endif

			pthread_mutex_t             listNearbyLock;
			devList ( DeviceInstanceEP )listNearby;

			pthread_mutex_t             listMediatorLock;
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
		public:
			devList ( DeviceInstanceEP )listMediator;
		private:
#else
			devList ( DeviceInstanceEP )listMediator;
#endif
            
            ListContext                 contextAll;
            ListContext                 contextMediator;
            ListContext                 contextNearby;

            ListContext OBJ_ptr GetListContext ( environs::DeviceClass_t listType );
            
			spv ( lib::IIListObserver * )		listAllObservers;
			spv ( lib::IIListObserver * )       listNearbyObservers;
			spv ( lib::IIListObserver * )       listMediatorObservers;

			envQueueVector ( ListCommandContextPtr )  listCommandQueue;
            
            bool                                listCommandThreadRun;
            ThreadSync                          listCommandThread;

            void								ListCommandQueueClear ();
            void                                DisposeListCommandContext ( ListContext OBJ_ptr ctx );
            
            void                                DisposeListContainer ( NLayerVecType ( DeviceInstanceEP ) c_ref list );
            
            void                                AddToListContainer ( DeviceClass_t listType, NLayerVecType ( DeviceInstanceEP ) c_ref vanished, NLayerVecType ( DeviceInstanceEP ) c_ref appeared );
            
			envQueueVector ( DeviceNotifierContextPtr ) deviceNotifierQueue;
            pthread_mutex_t						deviceNotifierLock;
            bool                                deviceNotifierThreadRun;
            ThreadSync                          deviceNotifierThread;
            
			pthread_t_id						startStopThread;
			bool								IsDisposalContextSafe ();
            void								DeviceNotifierQueueClear ();
            void                                DisposeNotifierContext ( DeviceNotifierContextPtr c_ref ctx );

            void                                DisposeDevice ( DeviceInstanceReferenceSP device );
            
			static bool             objetAPIInit INIT_to_false_in_cli;

			static bool				ObjectAPIInit ();
			static void				ObjectAPIDispose ();

			/**
			* Update device flags to native layer and populate them to the environment.
			*
			* @param	hInst    The handle to the environs instance.
            * @param	objID    The identifier for the native device object.
            * @param	flags    The internal flags to set or clear.
            * @param	set    	 true = set, false = clear.
			*/
			void					SetDeviceFlags ( int objID, int flags, bool set );

			static void c_OBJ_ptr	EnvironsStart ( pthread_param_t arg );

            void                    DoStop ();
            static void c_OBJ_ptr   EnvironsStop ( pthread_param_t arg );
            
            void					DisposeLists ( bool releaseList );
            
            void					DisposeList ( int listType );

			void					ReloadLists ();

			devList ( DeviceInstanceEP ) c_ref GetDevices ( int type );

			devList ( DeviceInstanceEP ) c_ref GetDevicesBest ( pthread_mutex_t OBJ_ptr OBJ_ref lock );

			void GetDeviceByObjectID ( DeviceInstanceESP OBJ_ref device, OBJIDType objID );

			void GetDeviceByDeviceID ( DeviceInstanceESP OBJ_ref device, int deviceID );

			void GetDeviceByNativeID ( DeviceInstanceESP OBJ_ref device, int nativeID );


			/**
			 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator).
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			void GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType objID );

			void GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType nativeOrDeviceID, bool isNativeID );


			/**
			 * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			void GetDeviceNearby ( DeviceInstanceESP OBJ_ref device, OBJIDType objID );


			/**
			 * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			c_const devList ( DeviceInstanceEP ) c_ref GetDevicesNearby ();


			/**
			 * Query a DeviceInstance object of Mediator managed devices within the environment.
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			void GetDeviceFromMediator ( DeviceInstanceESP OBJ_ref device, OBJIDType objID );


			/**
			 * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			c_const devList ( DeviceInstanceEP ) c_ref GetDevicesFromMediator ();

#ifndef CLI_CPP
			static void NotifyListObservers ( int hInst, c_const spv ( lib::IIListObserver * ) c_ref observerList, svsp ( DeviceInstance ) vanished, svsp ( DeviceInstance ) appeared, bool enqueue );

#endif
			static void c_OBJ_ptr DeviceListUpdateThreadStarter ( pthread_param_t pack );

			void * DeviceListUpdateThread ( pthread_param_t pack );

            EnvSignal deviceListUpdateSignal;

			void OnDeviceListUpdate ();
			void DeviceListUpdate ( int listType );

			void UpdateConnectProgress ( OBJIDType objID, int progress );

			void OnDeviceListNotification ( environs::ObserverNotifyContext OBJ_ptr ctx );

			void OnDeviceListNotification1 ( c_const DeviceInstanceESP c_ref device, environs::ObserverNotifyContext OBJ_ptr ctx );

			void UpdateMessage ( environs::ObserverMessageContext OBJ_ptr ctx );



			void UpdateData ( environs::ObserverDataContext OBJ_ptr ctx );

			void UpdateUdpData ( UdpDataContext OBJ_ptr udpData );
            
            /**
             * Enable sending of sensor events to this DeviceInstance.
             * Events are send if the device is connected and stopped if the device is disconnected.
             *
             * @param nativeID 				Destination native device id
             * @param objID 				Destination object device id
             * @param sensorType            A value of type environs.SensorType.
             * @param enable 				true = enable, false = disable.
             *
             * @return success true = enabled, false = failed.
             */
            //bool SetSensorEventSender ( int nativeID, int objID, environs::SensorType_t sensorType, bool enable );
            
            //void SetSensorEventSenderFlags ( int nativeID, int objID, int flags, bool enable );
		};
	}
}
#endif


#endif






















