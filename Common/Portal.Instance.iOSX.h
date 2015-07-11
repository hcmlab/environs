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

#import "Portal.Info.h"
#include "Environs.Types.h"

#import "Environs.Observer.h"
using namespace environs;


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
@public
    /** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
    int                     async;
    
    int                     portalID;
    bool                    disposed;
    id                      device;
    PortalInfo              info;
    
    int                     status;
    bool                    disposeOngoing;
    
    bool                    outgoing;
    PortalType::PortalType  portalType;
    
    /** Application defined contexts for arbitrary use. */
    id                      appContext1;
    id                      appContext2;
    id                      appContext3;
    id                      appContext4;
}

- (void) AddObserver:(id<PortalObserver>) observer;
- (void) RemoveObserver:(id<PortalObserver>) observer;

- (bool) Establish:(bool) askForType;

- (bool) Start;
- (bool) Stop;

- (bool) SetRenderSurface:(id)surface;
- (bool) ReleaseRenderSurface;

+ (PortalInfoBase *) GetPortalInfo:(int) portalID;
+ (bool) SetPortalInfo:(PortalInfoBase *) infoBase;

@end


#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H

