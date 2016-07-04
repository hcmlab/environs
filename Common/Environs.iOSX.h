/**
 * Environs.iOSX.h
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
#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"

#import <Environs.iOSX.Imp.h>

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )

#   if __has_feature(modules)
        @import Foundation;
        @import CoreLocation;

#       if ( defined(ENVIRONS_IOS) )
            @import CoreMotion;
            @import VideoToolbox;
            @import AVFoundation;
            @import SystemConfiguration;

#           ifdef ENABLE_IOS_HEALTHKIT_SUPPORT
                @import HealthKit;
#           endif
#       endif

#   else
#       import <Foundation/Foundation.h>
#       import <CoreLocation/CoreLocation.h>
#   endif

#   import "Environs.Observer.iOSX.h"

#if (!defined(DISABLE_ENVIRONS_OBJC_API))
#   import "Device.List.iOSX.h"
#   import "Device.Instance.iOSX.h"
#   import "Message.Instance.iOSX.h"
#   import "File.Instance.iOSX.h"
#   import "Portal.Instance.iOSX.h"

#   include "Device.Display.Decl.h"

#   ifdef __cplusplus
#       define ENVIRONS_NAMES  environs::
#   else
#       define ENVIRONS_NAMES
#   endif

bool CreateAppID ( char * buffer, unsigned int bufSize );

@interface Environs :
#   ifdef ENVIRONS_IOS
        NSObject<UIAlertViewDelegate>
#   else
        NSObject
#   endif
{
}

- (bool) opt:(NSString *) key;


#ifndef ENVIRONS_CORE_LIB

- (instancetype) init __attribute__((unavailable("Call [Environs CreateInstance]; or [Environs New]; to create Environs objects.")));

+ (instancetype) alloc __attribute__((unavailable("Call [Environs CreateInstance]; or [Environs New]; to create Environs objects.")));

#endif


#ifdef __cplusplus

/** Perform calls to the Environs object asynchronously. If set to Environs.CALL_WAIT, then all commands will block until the call finishes.
 * If set to Environs.CALL_NOWAIT, then certain calls (which may take longer) will be performed asynchronously. */
@property (nonatomic) environs::Call_t   async;

#endif


/**
 * Instructs the framework to perform a quick shutdown (with minimal wait times)
 *
 * @param enable      true / false
 */
- (void) SetAppShutdown : (bool) enable;


/**
 * Create an Environs object.
 *
 * @return   An instance handle that refers to the created Environs object
 */
+ (Environs *) CreateInstance;


/**
 * Create an Environs object.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   An instance handle that refers to the created Environs object
 */
+ (Environs *) CreateInstance: (const char *) appName Area:(const char *) areaName;


/**
 * Create an Environs object.
 *
 * @return   An instance handle that refers to the created Environs object
 */
+ (Environs *) New;


/**
 * Create an Environs object.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   An instance handle that refers to the created Environs object
 */
+ (Environs *) New: (const char *) appName Area:(const char *) areaName;


/**
 * Load settings for the given application environment from settings storage,
 * if any have been saved previously.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   success
 */
- (bool) LoadSettings: (const char *) appName Area:(const char *) areaName;


/**
 * Load settings. Prior to this call, an application environment MUST be given
 * using SetApplicationName and SetAreaName.
 *
 * @return   success
 */
- (bool) LoadSettings;


- (void) ShowMessage:(NSString *) sender Message:(const char *) message Length:(int)msgLength;

/**
 * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
 * Tasks:
 *  - Request display always on state, hence disable standby and power management functions.
 *  - Load sensor managers and sensor services
 *  - Initialize google cloud messaging
 *
 @returns success
 */
- (bool) Init;


/**
 * Dispose the storage, that is remove all data and messages received in the data store.
 *
 */
- (void) ClearStorage;


/**
 * Reset crypt layer and all created resources. Those will be recreated if necessary.
 * This method is intended to be called directly after creation of an Environs instance.
 *
 */
- (void) ResetCryptLayer;


/**
 * Get the native version of Environs.
 *
 * @return		version string
 */
- (const char *) GetVersionString;

/**
 * Get the native major version of Environs.
 *
 * @return		major version
 */
- (int) GetVersionMajor;

/**
 * Get the native minor version of Environs.
 *
 * @return		minor version
 */
- (int) GetVersionMinor;

/**
 * Get the native revision of Environs.
 *
 * @return		revision
 */
- (int) GetVersionRevision;


/**
 * Query whether the native layer was build for release (or debug).
 *
 * @return	true = Release build, false = Debug build.
 */
- (bool) GetIsReleaseBuild;


/**
 * Instruct Environs to output verbose debug logging.
 *
 * @param level      debug level 0 ... 16
 */
- (void) SetDebug : (int) value;

/**
 * Get output debug level.
 *
 * @return level      debug level 0 ... 16
 */
- (int) GetDebug;


/**
* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
* Increasing this value may help to handle worse networks which suffer from large latencies.
*
* @param   timeout
*/
- (void) SetNetworkConnectTimeout : (int) timeout;


/**
 * Get platform that the app is running on.
 *
 * @return 	enum of type Environs.Platforms
 */
- (int) GetPlatform;

/**
 * Set the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Valid type are enumerated in Environs.Platforms.*
 *
 * @param	platform	Environs.Platforms.*
 */
- (void) SetPlatform: (int) platform;

/**
 * Set/Remove the location-node flag to the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Flag: Environs.Platforms.LocationNode_Flag
 *
 * @param	isLocationNode	true or false
 */
- (void) SetIsLocationNode: (bool) isLocationNode;

/**
 * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
 *
 * @param 	username
 * @return	success
 */
- (bool) SetMediatorUserName: (NSString *) userName;
- (bool) SetUserName: (const char *) username;

/**
 * Query UserName used to authenticate with a Mediator.
 *
 * @return UserName
 */
- (NSString *) GetMediatorUserName;

/**
 * Enable or disable anonymous logon to the Mediator.
 *
 * @param 	enable A boolean that determines the target state.
 */
- (void) SetUseMediatorAnonymousLogon: (bool) enable;

/**
 * Retrieve a boolean that determines whether Environs makes use of anonymous logon to Mediator services.
 *
 * @return		true = yes, false = no
 */
- (bool) GetUseMediatorAnonymousLogon;

/**
 * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
 *
 * @param 	password
 * @return	success
 */
- (bool) SetMediatorPassword: (NSString *) password;
- (bool) SetUserPassword: (const char *) password;

/**
 * Enable or disable authentication with the Mediator using username/password.
 *
 * @param 	enable
 */
- (void) SetUseAuthentication: (bool) enable;


#ifdef __cplusplus

/**
 * Query the filter level for device management within Environs.
 *
 * return level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
 */
- (environs::MediatorFilter_t) GetMediatorFilterLevel;

/**
 * Set the filter level for device management within Environs.
 *
 * @param level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
 */
- (void) SetMediatorFilterLevel : (environs::MediatorFilter_t) level;

#endif


/**
 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
 *
 * @return		true = yes, false = no
 */
- (bool) GetUseMediatorLoginDialog;

/**
 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseMediatorLoginDialog: (bool) enable;

/**
 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @return		true = yes, false = no
 */
- (bool) GetMediatorLoginDialogDismissDisable;

/**
 * Instruct Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMediatorLoginDialogDismissDisable: (bool) enable;


/**
 * Register at known Mediator server instances.
 *
 * @return success state
 */
- (bool) RegisterAtMediators;



/**
 * Enable or disable device list update notifications from Mediator layer.
 * In particular, mobile devices should disable notifications if the devicelist is not
 * visible to users or the app transitioned to background.
 * This helps recuding cpu load and network traffic when not required.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMediatorNotificationSubscription : (bool) enable;


/**
 * Get subscription status of device list update notifications from Mediator layer.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetMediatorNotificationSubscription;


/**
 * Enable or disable short messages from Mediator layer.
 * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
 * This helps recuding cpu load and network traffic when not necessary.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMessagesSubscription : (bool) enable;


/**
 * Get subscription status of short messages from Mediator layer.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetMessagesSubscription;


/** Default value for each DeviceInstance after object creation. */
- (bool) GetAllowConnectDefault;

/** Default value for each DeviceInstance after object creation. */
- (void) SetAllowConnectDefault:(bool) value;


/**
 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
 * 		this call executes the start tasks within a thread.&nbsp;
 * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
 *
 */
- (void) Start;


#ifdef __cplusplus

/**
 * Query the status of Environs.&nsbp;Valid values are environs::Status
 *
 * @return environs::Status
 */
- (environs::Status_t)  GetStatus;

#endif


/**
 * Stop Environs and dispose all acquired resources.
 */
- (void) Stop;

/**
 * Stop Environs and dispose all acquired resources.
 */
- (void) Dispose;

/**
 * Add an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
- (void) AddObserver:(id<EnvironsObserver>) observer;

/**
 * Remove an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
- (void) RemoveObserver:(id<EnvironsObserver>) observer;

/**
 * Add an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
- (void) AddObserverForMessages:(id<EnvironsMessageObserver>) observer;

/**
 * Remove an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
- (void) RemoveObserverForMessages:(id<EnvironsMessageObserver>) observer;

/**
 * Add an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) AddObserverForData:(id<EnvironsDataObserver>) observer;

/**
 * Remove an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) RemoveObserverForData:(id<EnvironsDataObserver>) observer;


/**
 * Add an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsSensorObserver.
 */
- (void) AddObserverForSensorData:(id<EnvironsSensorObserver>) observer;

/**
 * Remove an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsSensorObserver.
 */
- (void) RemoveObserverForSensorData:(id<EnvironsSensorObserver>) observer;


/**
 * Set the ports that the local instance of Environs shall use for listening on connections.
 *
 * @param	tcpPort
 * @param	udpPort
 * @return success
 */
- (void) SetPorts:(int)comPort DataPort:(int)dataPort;


/**
 * Set the area name that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
- (void) SetAreaName: (const char *) name;

/**
 * Get the area name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	areaName
 */
- (const char *) GetAreaName;


/**
 * Set the application name of that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
- (void) SetApplication: (const char *) name;
- (void) SetApplicationName: (const char *) name;

/**
 * Get the application name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	appName
 */
- (const char *) GetApplicationName;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	deviceName
 * @return	success
 */
- (void) SetDeviceName: (const char *) name;


- (unsigned int) GetIPAddress;
- (unsigned int) GetSubnetMask;

- (NSString *) GetSSID;
- (NSString *) GetSSIDDesc;


/**
 * Set the device id that is assigned to the instance of Environs.
 *
 * @param		deviceID
 */
- (void) SetDeviceID : (int) deviceID;

/**
 * Get the device id that the application has assigned to the instance of Environs.
 *
 * @return		deviceID
 */
- (int) GetDeviceID;

/**
 * Request a device id from mediator server instances that have been provided before this call.
 * Prior to this call, the area and application name must be provided as well,
 * in order to get an available device id for the specified application environment.
 * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
 *
 * @return		deviceID
 */
- (int) GetDeviceIDFromMediator;

/**
 * Query whether the name of the current device has been set before.
 *
 * @return	has DeviceUID
 */
- (bool) HasDeviceUID;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	uid
 * @return	success
 */
- (bool) SetDeviceUID : (const char *) name;


/**
 * Determines whether to use environs default Mediator predefined by framework developers or not.
 *
 * @param enable 	true = use the default Mediator
 */
- (void) SetUseDefaultMediator : (bool) usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
- (bool) GetUseDefaultMediator;

/**
 * Determines whether to use given Mediator by setMediator()
 *
 * @param enable 	true = enable, false = disable
 */
- (void) SetUseCustomMediator : (bool) usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
- (bool) GetUseCustomMediator;

/**
 * Query ip of custom Mediator.
 *
 * @return ip
 */
- (NSString *) GetMediatorIP;

/**
 * Query port of custom Mediator.
 *
 * @return Port
 */
- (int) GetMediatorPort;

/**
 * Set custom Mediator to use.
 *
 * @param ip
 * @param port
 */
- (void) SetMediator:(NSString *)ip Port:(unsigned short) port;

/**
 * Determines whether to use Crypto Layer Security for Mediator connections.
 * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
 *
 * @param	enable
 */
- (void) SetUseCLSForMediator : (bool) usage;

/**
 * Query whether to use Crypto Layer Security for Mediator connections.
 *
 * @return	enabled
 */
- (bool) GetUseCLSForMediator;

/**
 * Determines whether to use Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
- (void) SetUseCLSForDevices : (bool) usage;

/**
 * Query whether to use Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
- (bool) GetUseCLSForDevices;

/**
 * Determines whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
- (void) SetUseCLSForDevicesEnforce : (bool) usage;

/**
 * Query whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
- (bool) GetUseCLSForDevicesEnforce;

/**
 * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
 *
 * @param	enable
 */
- (void) SetUseCLSForAllTraffic : (bool) usage;

/**
 * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
 *
 * @return	enabled
 */
- (bool) GetUseCLSForAllTraffic;

- (void) SetUseH264 : (bool) usage;
- (bool) GetUseH264;

/**
 * Instruct Environs to show log messages in the status log.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseNotifyDebugMessage : (bool) usage;

/**
 * Query Environs settings that show log messages in the status log.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetUseNotifyDebugMessage;

/**
 * Instruct Environs to create and write a log file in the working directory.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseLogFile : (bool) usage;

/**
 * Query Environs settings that create and write a log file in the working directory.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetUseLogFile;


/**
* Instruct Environs to log to stdout.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseLogToStdout : ( bool ) enable;


/**
* Query Environs settings whether to log to stdout.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseLogToStdout;


/**
* Instruct Environs to create DeviceLists that are used as UIAdapter by client code.
* Any changes of those lists are made within the applications main / UI thread context.
* Only DeviceList objects that are created after this call are affected.
* DeviceList objects created before this call remain using the setting at the time they are created.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseDeviceListAsUIAdapter : ( bool ) enable;


/**
* Query Environs settings whether to create DeviceLists that are used as UIAdapter by client code.
* Any changes of those lists are made within the applications main / UI thread context.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseDeviceListAsUIAdapter;



#ifdef ENVIRONS_OSX
/**
* Instruct Environs to use headless mode without worrying about UI thread.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseHeadless : ( bool ) enable;


/**
* Query Environs settings whether to use headless mode without worrying about UI thread.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseHeadless;


/**
* Check for mediator logon credentials and query on command line if necessary.
*
* @param success      true = successful, false = failed
*/
-( bool ) QueryMediatorLogonCommandLine;
#endif


/**
 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
- (bool) SetUseTouchRecognizer : (const char *) moduleName Status:(bool)enable;


/**
 * Use default encoder, decoder, capture, render modules.
 *
 * @return  success
 */
- (bool) SetUsePortalDefaultModules;


/**
 * Use encoder module with the name moduleName. (libEnv-Enc...).
 *
 * @param	moduleName  The module name
 * @return  success
 */
- (bool) SetUseEncoder: (const char *) moduleName;


/**
 * Use decoder module with the name moduleName. (libEnv-Dec...).
 *
 * @param	moduleName  The module name
 * @return  success
 */
- (bool) SetUseDecoder: (const char *) moduleName;


/**
 * Use capture module with the name moduleName. (libEnv-Cap...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
- (bool) SetUseCapturer: (const char *) moduleName;


/**
 * Use render module with the name moduleName. (libEnv-Rend...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
- (bool) SetUseRenderer: (const char *) moduleName;



#ifdef ENVIRONS_IOS
/**
 * Determines whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @param	enable
 */
- (void) SetUseNativeDecoder:(bool)usage;

/**
 * Query whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @return	enabled
 */
- (bool) GetUseNativeDecoder;

/**
 * Determines whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @param	enable
 */
- (void) SetUseSensors:(bool)usage;

/**
 * Query whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @return	enabled
 */
- (bool) GetUseSensors;

/**
 * Query whether to use native display resolution for the portal stream.
 *
 * @return enabled
 */
- (bool) GetPortalNativeResolution;

/**
 * Determines whether to use native display resolution for the portal stream.
 *
 * @param enable 	true = enable, false = disable
 */
- (void) SetPortalNativeResolution:(bool)enable;

/**
 * Query whether to automatically start a portal stream on successful device connection or not.
 *
 * @return enabled 	true = enable, false = disable
 */
- (bool) GetPortalAutoStart;

/**
 * Determine whether to automatically start a portal stream on successful device connection or not.
 *
 * @param enable 	true = enable, false = disable
 */
- (void) SetPortalAutoStart:(bool)enable;

/**
 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
 *
 * @param enable
 */
- (void) SetPortalTCP:(bool)usage;

/**
 * Query whether to use TCP for portal streaming (UDP otherwise)
 *
 * @return enabled
 */
- (bool) GetPortalTCP;

- (void) SetUsePortalTCP:(bool)usage;
- (bool) GetUsePortalTCP;
#else


#ifdef __cplusplus
- (int) SetUseTracker:(environs::Call_t) async module:(const char *) moduleName;

- (int) GetUseTracker:(const char *) moduleName;

- (EBOOL) DisposeTracker:(environs::Call_t) async module:(const char *) moduleName;

- (EBOOL) PushTrackerCommand:(environs::Call_t) async module: (int) index cmd:(int) command;
#endif

#endif


-(ENVIRONS_NAMES DeviceDisplay *) GetDeviceDisplayProps:(int) nativeID;

#ifdef __cplusplus

/**
 * Connect to device with the given ID and a particular application environment.
 *
 * @param deviceID	Destination device ID
 * @param areaName Project name of the application environment
 * @param appName	Application name of the application environment
 * @param async	    Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @return status	0: Connection can't be conducted (maybe environs is stopped or the device ID is invalid) &nbsp;
 * 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					2: A new connection has been triggered and is in progress
 */
-(int) DeviceConnect:(int) deviceID areaName:(const char *) areaName appName:(const char *) appName async:(environs::Call_t) async;


/**
 * Set render callback.
 *
 * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
 * @param callback		The pointer to the callback.
 * @param callbackType	A value of type RENDER_CALLBACK_TYPE_* that tells the portal receiver what we actually can render..
 * @return				true = success, false = failed.
 */
-(bool) SetRenderCallback:(environs::Call_t) async portalID:(int) portalID callback:(void *)callback callbackType:(environs::RenderCallbackType_t) callbackType;


/**
 * Release render callback delegate or pointer
 *
 * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
 * @param callback		A delegate that manages the callback.
 * @return				true = success, false = failed.
 */
-(bool) ReleaseRenderCallback:(environs::Call_t) async portalID:(int) portalID;

/**
 * Start streaming of portal to or from the portal identifier (received in notification).
 *
 * @param async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
 * @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
 *
 * @return success
 */
-(bool) StartPortalStream:(environs::Call_t) async portalID:(int) portalID;


/**
 * Stop streaming of portal to or from the portal identifier (received in notification).
 *
 * @param 	async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
 * @param 	nativeID    The native device id of the target device.
 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 					    Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
-(bool) StopPortalStream:(environs::Call_t) async nativeID:(int) nativeID portalID:(int) portalID;

#endif


/**
 * Get the status, whether the device (id) has established an active portal
 *
 * @param 	nativeID    The device id of the target device.
 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
 * @return	success 	true = yes, false = no
 */
-(bool) GetPortalEnabled:(int) nativeID portalType:(int) portalType;


/**
 * Get the number of devices that are currently connected to our device.
 *
 * @return	Count of connected devices
 */
-(int) GetConnectedDevicesCount;


/**
 * Get enabled status for stream encoding.
 *
 * @return	enabled
 */
-(bool) GetUseStream;


#ifdef DISPLAYDEVICE
/**
 * Get platform support for OpenCL.
 *
 * @return	enabled
 */
-(bool) GetUseOpenCL;


/**
 * Switch platform support for OpenCL rendering.
 *
 * @param enable
 */
-(void) SetUseOpenCL:(bool) enable;

#endif


-(const char *) GetFilePathNative:(int) nativeID fileID:(int)fileID;


-(char *) GetFilePath:(int) nativeID  fileID:(int) fileID;



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
-(NSData *) GetFile: (bool) nativeIDOK nativeID:(int)nativeID deviceID:(int)deviceID areaName:(const char *)areaName appName:(const char *)appName fileID:(int)fileID size:(int *)size;


#ifdef __cplusplus

/**
 * Create a new collection that holds all devices of given list type. This list ist updated dynamically by Environs.
 * After client code is done with the list, the list->Release () method MUST be called by the client code,
 * in order to release the resource (give ownership) back to Environs.
 *
 * @return Collection of IDeviceInstance objects
 */
- (DeviceList *) CreateDeviceList : (environs::DeviceClass_t) MEDIATOR_DEVICE_CLASS_;


/**
 * Enable dispatching of sensor events from ourself.
 * Events are send if Environs instance is started stopped if the Environs instance has stopped.
 *
 * Note: If you request GPS locations, then you must add the following keys to your plist
    <key>NSLocationWhenInUseUsageDescription</key>
    <string>Reason as text</string>
 *
 * @param sensorType            A value of type environs.SensorType.
 * @param enable 				true = enable, false = disable.
 *
 * @return success true = enabled, false = failed.
 */
- (bool) SetSensorEvent: (environs::SensorType_t) sensorType enable:(bool) enable;


/**
 * Determine whether the given sensorType is available.
 *
 * @param sensorType A value of type environs::SensorType_t.
 *
 * @return success true = available, false = not available.
 */
- (bool) IsSensorAvailable: (environs::SensorType_t) sensorType;

#endif

@end

#endif


#endif
