/**
 * PortalsInstance iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H

#import "Environs.iOSX.Imp.h"

#import "Environs.Observer.iOSX.h"

#ifdef __cplusplus

#include "Portal.Info.h"
#include "Environs.Types.h"
#endif


/**
 *	PortalsInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface PortalInstance :

#ifdef ENVIRONS_IOS
    NSObject<UIAlertViewDelegate>
#else
    NSObject
#endif
{
}

#ifdef __cplusplus
/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
@property (nonatomic) environs::Call_t  async;

@property (readonly, nonatomic) environs::PortalStatus_t  status;

#endif

/** An ID that identifies this portal across all available portals. */
@property (readonly, nonatomic) int     portalID;

/** true = Object is disposed and not updated anymore. */
@property (readonly, nonatomic) bool    disposed;

/** A DeviceInstance object that this portal relates to. */
@property (readonly) id                 device;

@property (readonly, nonatomic) bool    disposeOngoing;

@property (nonatomic) bool              startIfPossible;

/** true = outgoing (Generator), false = incoming (Receiver). */
@property (readonly, nonatomic) bool    outgoing;

@property (readonly, nonatomic) bool    isIncoming;
@property (readonly, nonatomic) bool    isOutgoing;

/** Application defined contexts for arbitrary use. */
@property (strong, nonatomic) id        appContext1;
@property (strong, nonatomic) id        appContext2;
@property (strong, nonatomic) id        appContext3;
@property (strong, nonatomic) id        appContext4;

// Exclude cpp related code from objective c headers
#ifdef __cplusplus

@property (readonly, nonatomic) environs::PortalInfo *  info;
@property (nonatomic) environs::PortalType_t  portalType;

- (environs::PortalInfoBase *) GetPortalInfo : (int)portalID;

- (bool) SetPortalInfo : (environs::PortalInfoBase *)infoBase;

#endif


- (void) AddObserver : (id<PortalObserver>) observer;
- (void) RemoveObserver : (id<PortalObserver>) observer;

- (bool) Establish : (bool) askForType;

- (bool) Start;
- (bool) Stop;

- (bool) SetRenderSurface : (id)surface;
- (bool) SetRenderSurface : (id)surface Width:(int)width Height:(int)height;
- (bool) ReleaseRenderSurface;

- (PortalInstance *) GetPortal:(int)nativeID  PortalID:(int)portalID;

@end


#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H

