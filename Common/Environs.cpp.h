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

#import <Environs.iOSX.Imp.h>

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

#import "Environs.Observer.h"
#import "Device.List.iOSX.h"
#import "Message.Instance.iOSX.h"


bool CreateAppID ( char * buffer, unsigned int bufSize );

@interface Environs :
#ifdef ENVIRONS_IOS
    NSObject<CLLocationManagerDelegate, UIAlertViewDelegate>
#else
    NSObject
#endif
{
}

+ (bool) opt:(NSString *) key;


+ (void) ShowMessage:(int) nativeID Message:(const char *) message withLength:(int)msgLength;

/**
 * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
 * Tasks:
 *  - Request display always on state, hence disable standby and power management functions.
 *  - Load sensor managers and sensor services
 *  - Initialize google cloud messaging
 *
 @returns success
 */
+ (bool) Init;

/**
 * Reset crypt layer and all created resources. Those will be recreated if necessary.
 * This method is intended to be called directly after creation of an Environs instance.
 *
 */
+ (void) ResetCryptLayer;

/**
 * Get the native version of Environs.
 *
 * @return		version string
 */
+ (const char *) GetVersionString;

/**
 * Get the native major version of Environs.
 *
 * @return		major version
 */
+ (int) GetVersionMajor;

/**
 * Get the native minor version of Environs.
 *
 * @return		minor version
 */
+ (int) GetVersionMinor;

/**
 * Get the native revision of Environs.
 *
 * @return		revision
 */
+ (int) GetVersionRevision;


/**
 * Query whether the native layer was build for release (or debug).
 *
 * @return	true = Release build, false = Debug build.
 */
+ (bool) GetIsReleaseBuild;


/**
* Set app status in order to enable or disable sleep mode, e.g. if the app runs in background.
* In APP_STATUS_SLEEPING, all network activities (except heartbeat packets) are disabled.
*
* @param 	status	One of the APP_STATUS_* values.
*/
+ (void) SetAppStatus:(int) status;

/**
* Set the render surface of type UIView as the target to render the portal to. The width and height of the UIView is defined as the size of a fullscreen view.
* The portal is determined through the deviceID and portalID.
* @param deviceID
* @param portalDeviceID
* @param newSurface
* @returns success
*/
+ (bool) SetRenderSurface:(int)portalID Surface:(UIView *)newSurface;

/**
*  Set the render surface of type UIView as the target to render the portal to. The UIView has a defined width and size.
*  The portal is determined through the deviceID and portalID.
*  @param portalID
*  @param newSurface
*  @param width
*  @param height
*  @returns success
*/
+ (bool) SetRenderSurface:(int)portalID Surface:(UIView *)newSurface Width:(int)width Height:(int)height;


/**
*  Release the render surface determined through the deviceID and portalID.
*  @param portalID
*  @returns success
*/
+ (bool) ReleaseRenderSurface:(int)portalDeviceID;


+ (void) SetDebug:(int)value;

/**
 * Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Valid type are enumerated in Environs.DEVICE_TYPE_*
 *
 * @param	type	Environs.DEVICE_TYPE_*
 */
+ (void) SetDeviceType:(char)value;

/**
 * Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
 * Valid type are enumerated in Types.DEVICE_TYPE_*
 *
 * @return	type	Environs.DEVICE_TYPE_*
 */
+ (char) GetDeviceType;

/**
 * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
 *
 * @param 	username
 * @return	success
 */
+ (bool) SetMediatorUserName: (NSString *) userName;
+ (bool) SetUserName: (const char *) username;

/**
 * Query UserName used to authenticate with a Mediator.
 *
 * @return UserName
 */
+ (NSString *) GetMediatorUserName;

/**
 * Enable or disable anonymous logon to the Mediator.
 *
 * @param 	enable A boolean that determines the target state.
 */
+ (void) SetUseMediatorAnonymousLogon: (bool) enable;

/**
 * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
 *
 * @param 	password
 * @return	success
 */
+ (bool) SetMediatorPassword: (NSString *) password;
+ (bool) SetUserPassword: (const char *) password;

/**
 * Enable or disable authentication with the Mediator using username/password.
 *
 * @param 	enable
 */
+ (void) SetUseAuthentication: (bool) enable;

/**
 * Query the filter level for device management within Environs.
 *
 * return level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
 */
+ (int) GetMediatorFilterLevel;

/**
 * Set the filter level for device management within Environs.
 *
 * @param level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
 */
+ (void) SetMediatorFilterLevel:(int)level;


/**
 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
 *
 * @return		true = yes, false = no
 */
+ (bool) GetUseMediatorLoginDialog;

/**
 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) SetUseMediatorLoginDialog: (bool) enable;

/**
 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @return		true = yes, false = no
 */
+ (bool) GetMediatorLoginDialogDismissDisable;

/**
 * Instruct Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) SetMediatorLoginDialogDismissDisable: (bool) enable;


/**
 * Register at known Mediator server instances.
 *
 * @return success state
 */
+ (bool) RegisterAtMediators;


/**
 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
 * 		this call executes the start tasks within a thread.&nbsp;
 * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
 *
 */
+ (void) Start;

/**
 * Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
 *
 * @return Environs.NATIVE_STATUS_*
 */
+ (int)  GetStatus;

/**
 * Stop Environs and release all acquired resources.
 */
+ (void) Stop;

/**
 * Stop Environs and release all acquired resources.
 */
+ (void) Release;

/**
 * Add an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
+ (void) AddObserver:(id<EnvironsObserver>) observer;

/**
 * Remove an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
+ (void) RemoveObserver:(id<EnvironsObserver>) observer;

/**
 * Add an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
+ (void) AddObserverForMessages:(id<EnvironsMessageObserver>) observer;

/**
 * Remove an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
+ (void) RemoveObserverForMessages:(id<EnvironsMessageObserver>) observer;

/**
 * Add an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
+ (void) AddObserverForData:(id<EnvironsDataObserver>) observer;

/**
 * Remove an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
+ (void) RemoveObserverForData:(id<EnvironsDataObserver>) observer;


/**
 * Set the ports that the local instance of Environs shall use for listening on connections.
 *
 * @param	tcpPort
 * @param	udpPort
 * @return success
 */
+ (void) SetPorts:(int)comPort withDataPort:(int)dataPort;


/**
 * Check whether the device with the given ID is connected to the local instance of Environs.
 *
 * @param deviceID	Destination device ID
 * @return success true: connected; false: not connected
 */
+ (bool) IsDeviceConnected:(int)deviceID;

/**
 * Get status whether the device with the given ID is connected to the local instance of Environs.&nbsp;
 * The return value is of enumeration type Types.DeviceStatus.*&nbsp;
 * That is e.g. Types.DeviceStatus.ConnectInProgress
 *
 * @param	nativeID    The native identifier that targets the device.
 * @return connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
 */
+ (int) GetDeviceConnectStatus:(int)nativeID;

/**
 * Query the number of devices that are connected to the local instance of Environs.
 *
 * @return numberOfConnectedDevices
 */
+ (int) GetConnectedDevicesCount;


+ (int) DeviceConnect:(int)deviceID doAsync:(int)async;
+ (int) DeviceConnect:(int)deviceID Area:(const char *) areaName App:(const char *) appName  doAsync:(int)async;

+ (bool) DeviceDisconnect:(int)nativeID doAsync:(int)async;

/**
 * Set the area name that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
+ (void) SetAreaName: (const char *) name;

/**
 * Get the area name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	areaName
 */
+ (const char *) GetAreaName;


/**
 * Set the application name of that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
+ (void) SetApplication: (const char *) name;
+ (void) SetApplicationName: (const char *) name;

/**
 * Get the application name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	appName
 */
+ (const char *) GetApplicationName;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	deviceName
 * @return	success
 */
+(void) SetDeviceName: (const char *) name;


/**
* Find a free portalID slot for the direction encoded into the given portalDetails.
*
* @param 	nativeID    	The native device id of the target device.
* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
*
* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
*/
+ (int) GetPortalIDFreeSlot:(int) nativeID details : (int) portalDetails;


/**
 * Request a portal stream from the device with the given id.&nbsp;
 * The device must be in a connected state by means of prior call to deviceConnect ().
 *
 * @param	nativeID    The native identifier that targets the device.
 * @param 	typeID		Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) RequestPortalStream:(int) nativeID doAsync:(int)async Type:(int) typeID Width:(int) width Height:(int) height;

/**
 * Start streaming of portal to this device.
 *
 * @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) StartPortalStream:(int)async PortalID:(int) portalID;

/**
 * Stop streaming of portal from this device.
 *
 * @param nativeID  The native identifier that targets the device.
 * @param portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 					It is provided within the notification listener as sourceIdent.&nbsp;
 * 					Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) StopPortalStream:(int)async NativeID:(int)nativeID PortalID:(int)portalID;

/**
 * Pause streaming of portal to this device.
 *
 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) PausePortalStream:(int)async PortalID:(int) portalID;


+ (unsigned int) GetIPAddress;
+ (unsigned int) GetSubnetMask;
+ (NSString *) GetSSID;
+ (NSString *) GetSSIDDesc;


/**
 * Send a string message to a device through one of the following ways.&nbsp;
 * If a connection with the destination device has been established, then use that connection.
 * If the destination device is not already connected, then distinguish the following cases:
 * (1) If the destination is within the same network, then try establishing a direct connection.
 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
 * 		a STUNT connection with the latest connection details that are available.
 *
 * On successful transmission, Environs returns true if the devices already had an active connection,
 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
 * a registered EnvironsListener instance.
 *
 * @param deviceID	The device id which the message should be send to.
 * @param message Message to send
 * @return success
 */
+ (bool) SendMessage: (int)deviceID doAsync:(int)async Msg:(NSString *) msg;


/**
 * Send a string message to a device through one of the following ways.&nbsp;
 * If a connection with the destination device has been established, then use that connection.
 * If the destination device is not already connected, then distinguish the following cases:
 * (1) If the destination is within the same network, then try establishing a direct connection.
 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
 * 		a STUNT connection with the latest connection details that are available.
 *
 * On successful transmission, Environs returns true if the devices already had an active connection,
 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
 * a registered EnvironsListener instance.
 *
 * @param deviceID	The device id which the message should be send to.
 * @param 	areaName	Area name of the application environment
 * @param 	appName		Application name of the application environment
 * @param message Message to send
 * @return success
 */
+ (bool) SendMessage: (int)deviceID Area:(const char *) areaName App:(const char *) appName doAsync:(int)async Msg:(NSString *) msg;


/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param	nativeID    The native identifier that targets the device.
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param path
 * @return success
 */
+ (bool) SendFile: (int)nativeID doAsync:(int)async FileID:(int)fileID Descriptor:(NSString *) descriptor Path:(NSString *) path;


/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param	nativeID    The native identifier that targets the device.
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param buffer
 * @param length number of bytes in the buffer to send
 * @return success
 */
+ (bool) SendBuffer: (int)nativeID doAsync:(int)async FileID:(int)fileID Descriptor:(NSString *) descriptor Buffer:(char *) buffer Length:(int)length;


/**
 * Query the width in pixel of the deviceID's screen.&nbsp;The device must be connected before with deviceConnect ().
 *
 * @param	nativeID    The native identifier that targets the device.
 * @return width in pixel
 */
+ (int) GetDeviceWidth:(int)nativeID;

/**
 * Query the height in pixel of the deviceID's screen. The device must be connected before with deviceConnect ()
 *
 * @param	nativeID    The native identifier that targets the device.
 * @return height in pixel
 */
+ (int) GetDeviceHeight:(int)nativeID;

/**
 * Query a DeviceScreen object of the device with the deviceID.&nbsp;
 * Note: The device must be connected in order to query the information.
 *
 * @param	nativeID    The native identifier that targets the device.
 * @return DeviceScreen
 */
+ (char *) GetDeviceDisplayProps:(int)nativeID;


/**
 * Query an array of DeviceInfo objects of available devices within the environment (including those of the Mediator)
 *
 * @return DeviceInfo-objects
 */
+ (char *) GetDevicesAvailable;

/**
 * Query an array of DeviceInfo objects with one DeviceInfo of available devices within the environment (including those of the Mediator)
 *
 * @param deviceID
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) GetDeviceAvailable:(int)deviceID Area:(const char *) areaName App:(const char *) appName;

/**
 * Query the number of available devices within the environment (including those of the Mediator)
 *
 * @return numberOfDevices
 */
+ (int) GetDevicesAvailableCount;

/**
 * Query an array of DeviceInfo objects of nearby (broadcast visible) devices within the environment.
 *
 * @return DeviceInfo-objects
 */
+ (char *) GetDevicesNearby;

/**
 * Query an array of DeviceInfo objects of nearby (broadcast visible) devices within the environment.
 *
 * @param deviceID
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) GetDeviceNearby:(int)deviceID Area:(const char *) areaName App:(const char *) appName;

/**
 * Query the number of nearby (broadcast visible) devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) GetDevicesNearbyCount;

/**
 * Query an array of DeviceInfo objects of Mediator managed devices within the environment.
 *
 * @return DeviceInfo-objects
 */
+ (char *) GetDevicesFromMediator;

/**
 * Query an array of DeviceInfo objects with one DeviceInfo of Mediator managed devices within the environment.
 *
 * @param deviceID
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) GetDeviceFromMediator:(int)deviceID Area:(const char *) areaName App:(const char *) appName;

/**
 * Query the number of Mediator managed devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) GetDevicesFromMediatorCount;


/**
 * Query a DeviceInfo object for the active portal identified by the portalID.
 *
 * @param portalID      The portalID that identifies an active portal.
 * @return DeviceInfo-object    IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
 */
+ (environs::DevicePackage *) GetDeviceForPortal:(int) portalID;


/**
 * Get the status, whether the device (id) has direct physical contact, such as lying on the surface
 *
 * @param	nativeID    The native identifier that targets the device.
 * @return		true = yes, false = no
 */
+ (bool) GetDirectContactStatus:(int)nativeID;

/**
 * Get the status, whether the device (id) has established an active portal
 *
 * @param	nativeID    The native identifier that targets the device.
 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
 * @return		true = yes, false = no
 */
+ (bool) GetPortalEnabled:(int)nativeID type:(int)portalType;


/**
 * Get the portalID of the first active portal
 *
 * @param	nativeID    The native identifier that targets the device.
 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
 * @return	portalID 	The portal ID.
 */
+ (int) GetPortalId:(int)nativeID type:(int)portalType;


/**
 * Get details about portal associated with the deviceID.
 *
 * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
 */
+ (environs::PortalInfoBase *) GetPortalInfo:(int)portalID;


/**
 * Set details for the portal associated with the deviceID.
 *
 * @param   A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
 * 			The deviceID and portalID members of the PortalInfo object must have valid values.
 * @return success
 */
+ (bool) SetPortalInfo:(environs::PortalInfoBase *)info;


/**
 * Set the device id that is assigned to the instance of Environs.
 *
 * @param		deviceID
 */
+ (void) SetDeviceID:(int)deviceID;

/**
 * Get the device id that the application has assigned to the instance of Environs.
 *
 * @return		deviceID
 */
+ (int) GetDeviceID;

/**
 * Request a device id from mediator server instances that have been provided before this call.
 * Prior to this call, the area and application name must be provided as well,
 * in order to get an available device id for the specified application environment.
 * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
 *
 * @return		deviceID
 */
+ (int) GetDeviceIDFromMediator;

/**
 * Query whether the name of the current device has been set before.
 *
 * @return	has DeviceUID
 */
+ (bool) HasDeviceUID;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	uid
 * @return	success
 */
+ (bool) SetDeviceUID:(const char *)name;


/**
 * Determines whether to use environs default Mediator predefined by framework developers or not.
 *
 * @param enable 	true = use the default Mediator
 */
+ (void) SetUseDefaultMediator:(bool)usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
+ (bool) GetUseDefaultMediator;

/**
 * Determines whether to use given Mediator by setMediator()
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) SetUseCustomMediator:(bool)usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
+ (bool) GetUseCustomMediator;

/**
 * Query ip of custom Mediator.
 *
 * @return ip
 */
+ (NSString *) GetMediatorIP;
//+ (void) setMediatorIP:(NSString *)value;

/**
 * Query port of custom Mediator.
 *
 * @return Port
 */
+ (int) GetMediatorPort;

//+ (void) setMediatorPort:(unsigned short) value;

/**
 * Set custom Mediator to use.
 *
 * @param ip
 * @param port
 */
+ (void) SetMediator:(NSString *)ip withPort:(unsigned short) port;

/**
 * Determines whether to use Crypto Layer Security for Mediator connections.
 * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
 *
 * @param	enable
 */
+ (void) SetUseCLSForMediator:(bool)usage;

/**
 * Query whether to use Crypto Layer Security for Mediator connections.
 *
 * @return	enabled
 */
+ (bool) GetUseCLSForMediator;

/**
 * Determines whether to use Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
+ (void) SetUseCLSForDevices:(bool)usage;

/**
 * Query whether to use Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
+ (bool) GetUseCLSForDevices;

/**
 * Determines whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
+ (void) SetUseCLSForDevicesEnforce:(bool)usage;

/**
 * Query whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
+ (bool) GetUseCLSForDevicesEnforce;

/**
 * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
 *
 * @param	enable
 */
+ (void) SetUseCLSForAllTraffic:(bool)usage;

/**
 * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
 *
 * @return	enabled
 */
+ (bool) GetUseCLSForAllTraffic;

+ (void) SetUseH264:(bool)usage;
+ (bool) GetUseH264;

/**
 * Instruct Environs to show log messages in the status log.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) SetUseNotifyDebugMessage:(bool)usage;

/**
 * Query Environs settings that show log messages in the status log.
 *
 * @return enable      true = enable, false = disable
 */
+ (bool) GetUseNotifyDebugMessage;

/**
 * Instruct Environs to create and write a log file in the working directory.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) SetUseLogFile:(bool)usage;

/**
* Query Environs settings that create and write a log file in the working directory.
*
* @return enable      true = enable, false = disable
*/
+ (bool) GetUseLogFile;


/**
 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
+ (bool) SetUseTouchRecognizer: (const char *) moduleName withStatus:(bool)enable;


/**
 * Use default encoder, decoder, capture, render modules.
 *
 * @return  success
 */
+ (bool) SetUsePortalDefaultModules;


/**
 * Use encoder module with the name moduleName. (libEnv-Enc...).
 *
 * @param	moduleName  The module name
 * @return  success
 */
+ (bool) SetUseEncoder: (const char *) moduleName;


/**
 * Use decoder module with the name moduleName. (libEnv-Dec...).
 *
 * @param	moduleName  The module name
 * @return  success
 */
+ (bool) SetUseDecoder: (const char *) moduleName;


/**
 * Use capture module with the name moduleName. (libEnv-Cap...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
+ (bool) SetUseCapturer: (const char *) moduleName;


/**
 * Use render module with the name moduleName. (libEnv-Rend...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
+ (bool) SetUseRenderer: (const char *) moduleName;



#ifdef ENVIRONS_IOS
/**
 * Determines whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @param	enable
 */
+ (void) SetUseNativeDecoder:(bool)usage;

/**
 * Query whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @return	enabled
 */
+ (bool) GetUseNativeDecoder;

/**
 * Determines whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @param	enable
 */
+ (void) SetUseSensors:(bool)usage;

/**
 * Query whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @return	enabled
 */
+ (bool) GetUseSensors;

/**
 * Query whether to use native display resolution for the portal stream.
 *
 * @return enabled
 */
+ (bool) GetPortalNativeResolution;

/**
 * Determines whether to use native display resolution for the portal stream.
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) SetPortalNativeResolution:(bool)usage;

/**
 * Query whether to automatically start a portal stream on successful device connection or not.
 *
 * @return enabled 	true = enable, false = disable
 */
+ (bool) GetPortalAutoStart;

/**
 * Determine whether to automatically start a portal stream on successful device connection or not.
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) SetPortalAutoStart:(bool)usage;

/**
 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
 *
 * @param enable
 */
+ (void) SetPortalTCP:(bool)usage;

/**
 * Query whether to use TCP for portal streaming (UDP otherwise)
 *
 * @return enabled
 */
+ (bool) GetPortalTCP;

+ (void) SetUsePortalTCP:(bool)usage;
+ (bool) GetUsePortalTCP;
#else


+ (int) SetUseTracker:(int) async module:(const char *) moduleName;

+ (int) GetUseTracker:(const char *) moduleName;

+ (EBOOL) DisposeTracker:(int) async module:(const char *) moduleName;

+ (EBOOL) PushTrackerCommand:(int) async module: (int) index cmd:(int) command;

#endif

@end

#endif
