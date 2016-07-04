/**
 * DeviceInstance iOSX CPP API internal
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_INTERNAL_API_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_INTERNAL_API_H

#import "Device.Instance.iOSX.h"
#import "File.Instance.iOSX.h"
#import "Message.Instance.iOSX.h"

/**
 *	DeviceInstance iOSX CPP API internal
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface DeviceInstance (internal)

@property (readonly, nonatomic) environs::DeviceInfo * info;

- (bool) SetInst : ( sp ( environs::lib::DeviceInstance ) & ) inst;

- (environs::lib::DeviceInstance *) GetInstancePtr;

- (void) NotifyObserversForData:(FileInstance *) fileInst Flags:(environs::FileInfoFlag_t) changedFlags;

- (void) NotifyObserversForMessage:(MessageInstance *) message Flags:(environs::MessageInfoFlag_t) changedFlags;

- (void) NotifyObservers:(id)sender changed:(environs::DeviceInfoFlag_t) changedFlags;

- (void) NotifyObservers:(environs::DeviceInfoFlag_t) changedFlags;

- (void) NotifyObserversForSensorData:(environs::SensorFrame *) pack;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance;
#endif

@end




#endif	/// INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_INTERNAL_API_H

