/**
 * DeviceCell.h
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
#import <UIKit/UIKit.h>
#import "Environs.Observer.iosx.h"
#include "Environs.Build.Opts.h"

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
@interface DeviceCell : UIViewController <DeviceObserver, ListObserver>
{

}
#else
@interface DeviceCell : UIViewController <DeviceObserver, ListObserver, PortalObserver>
{

}
#endif

extern DeviceCell * deviceCellInstance;

@property (nonatomic, strong) IBOutlet UILabel * deviceIdentifierLabel;
@property (nonatomic, strong) IBOutlet UILabel * appLabel;
@property (nonatomic, strong) IBOutlet UILabel * projectLabel;
@property (nonatomic, strong) IBOutlet UILabel * ipLabel;

@property (nonatomic, strong) IBOutlet UILabel * portsLabel;

@property (nonatomic, strong) IBOutlet UILabel * networkLabel;


@property (weak, nonatomic) IBOutlet UIButton * buttonConnect;
@property (weak, nonatomic) IBOutlet UIButton * buttonPortalIn;
@property (weak, nonatomic) IBOutlet UIButton * buttonPortalOut;
@property (weak, nonatomic) IBOutlet UIButton * buttonTest;

@property (weak, nonatomic) IBOutlet UITextView * textMessage;

@property (weak, nonatomic) IBOutlet UIProgressView * progressView;

- (IBAction) dismissKeyboardOnTap: (id)sender;

- (IBAction) Connect: (id)sender;
- (IBAction) PortalIn: (id)sender;
- (IBAction) Test: (id)sender;
- (IBAction) SendMessage: (id)sender;

+ (void) SetDeviceAndList:(id) device list:(id)deviceList;

- (void) UpdateUI;

- (void) OnListChanged:(id) oldDevices appeared:(id) newDevices;
- (void) OnDeviceChanged:(id) sender Flags:(int) flags;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance;
- (void) OnPortalChanged:(id) sender Notify:(int)Environs_NOTIFY_PORTAL_;
#endif

@end
