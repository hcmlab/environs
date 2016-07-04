/**
 * RemoteTouchView
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

#import <Cocoa/Cocoa.h>

@interface RemoteTouchView : NSViewController <NSTableViewDelegate, NSTableViewDataSource>

- (void) ReInitDeviceList;
- (void) ReleaseDeviceList;

- (void) UpdateStatusMessage: (const char *) msg;
- (void) UpdateUI;

- (IBAction) UpdateDeviceList: (id)sender;

@end

