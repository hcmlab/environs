/**
 * DeviceController
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_SELECTOR_H_
#define INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_SELECTOR_H_

#include "Environs.Platforms.h"

#ifdef DISPLAYDEVICE
/// DISPLAYDEVICE
#include "Device.Display.Win.h"
#include "Device.OSX.h"
#include "Device.Linux.h"


#else
/// NOT DISPLAYDEVICE

#ifdef ANDROID
/// ANDROID
#include "Device.Android.h"

#else
/// NOT ANDROID

#ifdef ENVIRONS_IOS
/// ENVIRONS_IOS
#include "Device.iOS.h"

#else
/// NOT ENVIRONS_IOS

#ifdef WIN32
#include "Device.Win.Phone.h"

#else

#include "Device.iOS.h"
//WARNINGT ( "Platform not recognized!!!" )

#endif

#endif

#endif

#endif /// end->DISPLAYDEVICE


#endif /* INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_SELECTOR_H_ */
