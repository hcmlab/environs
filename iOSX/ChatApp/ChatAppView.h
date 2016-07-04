/**
 * DeviceListView.h
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

@interface ChatAppView : UIViewController <UITableViewDelegate, UITableViewDataSource, ListObserver>
{
    
}

extern ChatAppView         *   chatAppView;

- (void) InitDeviceList;
- (IBAction) UpdateRow:(NSInteger) row;
- (IBAction) UpdateList;

- (void) StopListThread;
- (void) StopInitThread;

+ (void) UpdateMessageList : (ChatUser *) user;

@end
