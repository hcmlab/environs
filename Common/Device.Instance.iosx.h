/**
 * DeviceInstance iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H

#include "Device.Instance.h"

#include "Environs.iOSX.Imp.h"
#include "Device.Display.Decl.h"
#import "Portal.Instance.iOSX.h"
#import "Device.observer.iOSX.h"
using namespace environs;




/**
 *	DeviceInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface DeviceInstance : NSObject
{

@public
    
    /** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
    int             async;
    
    DeviceInfo      info;
    bool            disposed;
    int             connectProgress;
    
    DeviceDisplay   display;
    
    int             directStatus;
    NSLock  *       devicePortalsLock;
    
    /** A collection of PortalInstances that this device has established or is managing. */
    id              devicePortals;
    
    /** Application defined contexts for arbitrary use. */
    int             appContext0;
    id              appContext1;
    id              appContext2;
}


- (void) AddObserver:(id<DeviceObserver>) observer;
- (void) RemoveObserver:(id<DeviceObserver>) observer;

- (void) AddObserverForData:(id<DataObserver>) observer;
- (void) RemoveObserverForData:(id<DataObserver>) observer;

- (void) AddObserverForMessages:(id<MessageObserver>) observer;
- (void) RemoveObserverForMessages:(id<MessageObserver>) observer;

- (NSString *) GetIP;
- (NSString *) GetIPe;

- (bool) EqualsAppEnv:(DeviceInfo *) equalTo;
- (bool) EqualsAppEnv:(const char *)projectName App:(const char *)appName;

- (bool) LowerThanAppEnv:(DeviceInfo *) compareTo;
- (bool) LowerThanAppEnv:(const char *)projectName App:(const char *)appName;

- (bool) EqualsID:(DeviceInstance *) equalTo;
- (bool) EqualsID:(int) deviceID Project:(const char *)projectName App:(const char *)appName;


+ (NSString *) DeviceTypeString:(DeviceInfo *) info;
- (NSString *) DeviceTypeString;

- (const char *) GetBroadcastString:(bool) fullText;

/**
 * Connect to this device asynchronously.
 *
 * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					true: A new connection has been triggered and is in progress
 */
- (bool) Connect;

/**
 * Disconnect the device with the given id and a particular application environment.
 *
 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
 */
- (bool) Disconnect;

/**
 * Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
 *
 * @return PortalInstance-object
 */
- (DeviceDisplay) GetDisplayProps;

/**
 * Load the file that is assigned to the fileID received by deviceID into an byte array.
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @param size        An int pointer, that receives the size of the returned buffer.
 * @return byte-array
 */
- (char *) GetFile:(int) fileID Size:(int *)size;

/**
 * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @return absolutePath
 */
- (const char *) GetFilePath:(int) fileID;


/**
 * Creates a portal instance.
 *
 * @param request   The portal request.
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalCreate:(int) request;

/**
 * Creates a portal instance.
 *
 * @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
 * @param portalType	        Project name of the application environment
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalCreate:(int) Environs_PORTAL_DIR type:(PortalType::PortalType) portalType;

/**
 * Creates a portal instance with a given portalID.
 *
 * @param portalID   The portalID received from native layer.
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalCreateID:(int) portalID;


/**
 * Query the first PortalInstance that manages an outgoing portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetOutgoing;

/**
 * Query the first PortalInstance that manages an incoming portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetIncoming;

/**
 * Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
 *
 * @return PortalInstance-object
 */
-(PortalInstance *) PortalGetWaiting:(bool) outgoing;

/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param filePath      The path to the file to be send.
 * @return success
 */
- (bool) SendFile:(int)fileID Desc:(const char *)fileDescriptor Path:(const char *)filePath;

/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param buffer        A buffer to be send.
 * @param bytesToSend number of bytes in the buffer to send
 * @return success
 */
- (bool) SendBuffer:(int)fileID Desc:(const char *)fileDescriptor Data:(char *)buffer Size:(int)bytesToSend;

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
 * a registered EnvironsObserver instance.
 *
 * @param message       A message to send.
 * @param length       Length of the message to send.
 * @return success
 */
- (bool) SendMessage:(const char *)msg Size:(int)length;

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
 * a registered EnvironsObserver instance.
 *
 * @param message       A message to be send.
 * @return success
 */
- (bool) SendMessage:(NSString *)message;

@end


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H

