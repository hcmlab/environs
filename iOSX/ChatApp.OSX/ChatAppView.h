/**
 * ChatAppView
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
#include "Environs.h"

#import "ChatUser.h"

@interface ChatAppView : NSViewController <NSTableViewDelegate, NSTableViewDataSource, ListObserver, PortalObserver>
{
@public
    DeviceList  *   deviceList;
}

extern ChatAppView         *   chatAppView;

- (void) UpdateUI : (environs::Status_t) status;

- (IBAction) UpdateList;
- (IBAction) UpdateRow:(NSInteger) row;

- (void) InitDeviceList;
- (void) ClearDeviceList;

- (void) StopListThread;
- (void) StopInitThread;

+ (void) UpdateMessageList : (ChatUser *) user;


@end


