/**
 * DeviceLists iOSX CPP API internal
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_INTERNAL_API_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_INTERNAL_API_H

#import "Device.List.iOSX.h"
#include "Device.List.h"

/**
 *	DeviceLists iOSX CPP API internal
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface DeviceList (internal)

- (bool) SetInst : ( sp ( environs::lib::DeviceList ) ) inst;
- (void) NotifyListObservers:(NSArray *)vanished appeared:(NSArray *)appeared;

- (void) DisposeLists;
@end


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_IOSX_INTERNAL_H

