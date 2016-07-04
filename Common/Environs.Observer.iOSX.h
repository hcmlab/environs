/**
 * EnvironsObserver
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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_IOSX_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_IOSX_H

#import <Foundation/Foundation.h>
#include "Notify.Context.h"
#include "Environs.Types.h"
#include "Environs.Msg.Types.h"



/**
 * EnvironsObserver: Attachable to **Environs** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
 */
@protocol EnvironsObserver <NSObject>

#ifdef __cplusplus
/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param Environs_STATUS_      A status constant of type STATUS_*
 */
- (void) OnStatus:(environs::Status_t) Environs_STATUS_;

#endif

/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param context		An object reference of type ObserverNotifyContext.
 */
- (void) OnNotify:(ObserverNotifyContext *)context;


/**
 * OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param context		An object reference of type ObserverNotifyContext.
 */
- (void) OnNotifyExt:(ObserverNotifyContext *) context;


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The PortalInstance object.
 */
- (void) OnPortalRequestOrProvided:(id) portal;
#endif

@end


/**
 * ListObserver: Attachable to **DeviceList** objects in order to receive list changes of a particular IDeviceList.
 * Important: You must not call methods of DeviceList objects within the context of the observer callback. Otherwise deadlocks might occur.
 */
@protocol ListObserver <NSObject>

/**
 * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
 * Important: You must not call methods of DeviceList objects within the context of the observer callback. Otherwise deadlocks might occur.
 *
 * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
 * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
 */
- (void) OnListChanged:(NSArray *) vanished appeared:(NSArray *) appeared;

@end


/**
* DeviceObserver: Attachable to **DeviceInstance** objects in order to receive changes of a particular device.
*/
@protocol DeviceObserver <NSObject>

#ifdef __cplusplus

/**
* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
*
* @param device         The DeviceInstance object that sends this notification.
* @param changedFlags   The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
*/
- (void) OnDeviceChanged:(id) device Flags:(environs::DeviceInfoFlag_t) changedFlags;


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The PortalInstance object.
 */
- (void) OnPortalRequestOrProvided:(id) portal;
#endif

#endif


@end


/**
 * EnvironsMessageObserver: Attachable to **Environs** objects in order to receive all messages that the Environs instance received.
 */
@protocol EnvironsMessageObserver <NSObject>
/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param context		An object reference of type ObserverMessageContext.
 */
- (void) OnMessage:(ObserverMessageContext *) context;

/**
 * OnMessageExt is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param context		An object reference of type ObserverMessageContext.
 */
- (void) OnMessageExt:(ObserverMessageContext *) context;

/**
 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
 *
 * @param message      The status as a text message.
 */
- (void) OnStatusMessage:(const char *) message;

@end


/**
* MessageObserver: Attachable to **DeviceInstance** objects in order to receive messages of a particular device.
*/
@protocol MessageObserver <NSObject>

#ifdef __cplusplus

/**
 * OnMessage is called whenever a text message has been received from a device.
 *
 * @param msg           The corresponding message object of type MessageInstance
 * @param changedFlags  Flags that indicate the object change.
 */
- (void) OnMessage:(id) msg Flags:(environs::MessageInfoFlag_t) changedFlags;

#endif

@end


/**
 * EnvironsDataObserver: Attachable to **Environs** objects in order to receive all data transmissions that the Environs instance received.
 */
@protocol EnvironsDataObserver <NSObject>
/**
 * OnData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param context		An object reference of type ObserverDataContext.
 */
- (void) OnData:(ObserverDataContext *) context;
@end


/**
* DataObserver: Attachable to **DeviceInstance** objects in order to receive data transmissions of a particular device.
*/
@protocol DataObserver <NSObject>

#ifdef __cplusplus

/**
 * OnData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param fileData                  The corresponding file object of type FileInstance
 * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
 */
- (void) OnData:(id) fileData Flags:(environs::FileInfoFlag_t) changedFlags;

#endif

@end


/**
 * EnvironsSensorObserver: Attachable to **Environs** objects in order to receive all sensor data that the Environs instance received.
 */
@protocol EnvironsSensorObserver <NSObject>
/**
 * OnSensorData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param nativeID      The native identifier that targets the device. If nativeID is zero, then sensor source is ourself
 * @param sensorFrame   The frame containing the sensor data
 */
#ifdef __cplusplus

- (void) OnSensorData:(int) nativeID Frame:(environs::SensorFrame *) sensorFrame;

#endif

@end


/**
* SensorObserver: Attachable to **DeviceInstance** objects in order to receive sensor data from a particular device.
*/
@protocol SensorObserver <NSObject>
/**
 * OnSensorData is called whenever new sensor data has been received.
 *
 * @param sensorFrame     The corresponding SensorFrame of sensor data
 */
#ifdef __cplusplus

- (void) OnSensorData:(environs::SensorFrame *) sensorFrame;

#endif

@end


/**
 * PortalObserver: Attachable to **IPortalInstance** objects in order to receive changes of a particular interactive portal.
 */
@protocol PortalObserver <NSObject>

#ifdef __cplusplus

/**
 * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
 *
 * @param portal    The PortalInstance object.
 * @param notify	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
 */
- (void) OnPortalChanged:(id) portal Notify:(environs::Notify::Portal_t) notify;

#endif

@end

#endif
