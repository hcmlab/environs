/**
 * DeviceLists iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_H

#import "Device.Instance.iosx.h"


/**
 *	DeviceLists iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface DeviceLists : NSObject
{
@public
    bool disposed;
}

- (void) SetListType:(int)listType;

- (DeviceInstance *) GetItem:(int) position;
- (int) GetCount;

- (void) AddObserver:(id<DeviceObserver>) observer;
- (void) RemoveObserver:(id<DeviceObserver>) observer;


+ (DeviceInstance *) RefreshItem:(DeviceInstance *) source Observer:(id<DeviceObserver>)observer;

/**
 * Get a collection that holds all available devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
+ (NSMutableArray *) GetDevices;

/**
 * Query the number of all available devices within the environment (including those of the Mediator)
 *
 * @return numberOfDevices
 */
+ (int) getDevicesCount;

/**
 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
 *
 * @param deviceID      The device id of the target device.
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDevice:(int) deviceID Project:(const char *)projectName App:(const char *)appName;



/**
 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
 *
 * @param nativeID      The device id of the target device.
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDevice:(int) nativeID;

/**
 * Query a DeviceInstance object that best match the deviceID only.
 * Usually the one that is in the same app environment is picked up.
 * If there is no matching in the app environment,
 * then the project environments are searched for a matchint deviceID.
 *
 * @param deviceID      The portalID that identifies an active portal.
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDeviceBestMatch:(int) deviceID;

/**
 * Query a DeviceInstance object that best match the deviceID only from native layer.
 * Usually the one that is in the same app environment is picked up.
 * If there is no matching in the app environment,
 * then the project environments are searched for a matchint deviceID.
 *
 * @param deviceID      The portalID that identifies an active portal.
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDeviceBestMatchNative:(int) deviceID;


/**
 * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
+ (NSMutableArray *) GetDevicesNearby;

/**
 * Query the number of nearby (broadcast visible) devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) GetDevicesNearbyCount;

/**
 * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
 *
 * @param nativeID      The device id of the target device.
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDeviceNearby:(int) nativeID;

/**
 * Release the ArrayList that holds the nearby devices.
 */
+ (void) ReleaseDevicesNearby;

/**
 * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
+ (NSMutableArray *) GetDevicesFromMediator;

/**
 * Query a DeviceInstance object of Mediator managed devices within the environment.
 *
 * @param nativeID      The device id of the target device.
 * @return DeviceInstance-object
 */
+ (DeviceInstance *) GetDeviceFromMediator:(int) nativeID;

/**
 * Query the number of Mediator managed devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) GetDevicesFromMediatorCount;

/**
 * Release the ArrayList that holds the Mediator server devices.
 */
+ (void) ReleaseDevicesMediator;

/// <summary>
/// Refresh all device lists
/// </summary>
+ (void) RefreshDeviceLists;


@end




#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_H

