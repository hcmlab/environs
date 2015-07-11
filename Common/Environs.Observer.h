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
#import <Foundation/Foundation.h>
#import "Device.Observer.iOSX.h"


@protocol EnvironsObserver <NSObject>
/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param nativeID      The native identifier that targets the device.
 * @param notification  The notification
 * @param sourceIdent   A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
- (void) OnNotify:(int) nativeID withNotify:(int) notification withSource:(int)source withContext:(void *)context;


/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param deviceID      The device id of the sender device.
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @param notification  The notification
 * @param sourceIdent   A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
- (void) OnNotifyExt:(int) deviceID Project:(const char *) projectName App:(const char *) appName withNotify:(int) notification withSource:(int)source withContext:(int)context;


/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The PortalInstance object.
 * @return	accept		 The called method must return true, if it takes over this request. If false is returned, then the next observer (if any) is called.
 */
- (bool) OnPortalRequestOrProvided:(id) portalInstance;

@end



@protocol EnvironsMessageObserver <NSObject>
/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param nativeID      The native identifier that targets the device.
 * @param type	        Determines the source (either from a device, environs, or native layer)
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessage:(int) nativeID withType:(int)type withMsg:(const char *) message withLength:(int)msgLength;

/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param deviceID      The device id of the sender device.
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @param type	        Determines the source (either from a device, environs, or native layer)
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessageExt:(int) deviceID Project:(const char *) projectName App:(const char *) appName withType:(int)type withMsg:(const char *) message withLength:(int)msgLength;

/**
 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
 *
 * @param msg      The status as a text message.
 */
- (void) OnStatusMessage:(const char *) message;

@end




@protocol EnvironsDataObserver <NSObject>
/**
 * OnData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param nativeID      The native identifier that targets the device.
 * @param type          The type of the data
 * @param fileID        A user-customizable id that identifies the file
 * @param fileDescriptor    A text that describes the file
 * @param size          The size in bytes
 */
- (void) OnData:(int) nativeID withType:(int) type withFileID:(int) fileID withDescriptor:(const char *) descriptor withSize:(int) size;
@end




@protocol PortalObserver <NSObject>
/**
 * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
 *
 * @param sender        The PortalInstance object.
 * @param notification	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
 */
- (void) OnPortalChanged:(id) sender Notify:(int)notification;

@end


@protocol MessageObserver <NSObject>
/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param sender        The device instance which sent the data.
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessage:(id) sender Message:(const char *) message Length:(int) length;

@end


@protocol DataObserver <NSObject>
/**
 * OnData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param sender        The device instance which sent the data.
 * @param type          The type of the data
 * @param fileID        A user-customizable id that identifies the file
 * @param fileDescriptor    A text that describes the file
 * @param size          The size in bytes
 */
- (void) OnData:(id) sender Type:(int) type ID:(int) fileID Desc:(const char *) fileDescriptor Size:(int) size;

@end