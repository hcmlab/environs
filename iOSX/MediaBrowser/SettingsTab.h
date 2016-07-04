/**
 * SettingsTab.h
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

@interface SettingsTab : UIViewController<UITextFieldDelegate>
{
    
}


@property (weak, nonatomic) IBOutlet UILabel *labelVersionString;

@property (weak, nonatomic) IBOutlet UILabel *labelVideoStreaming;
@property (weak, nonatomic) IBOutlet UISwitch *switchVideoStreaming;

@property (weak, nonatomic) IBOutlet UILabel *labelSensors;
@property (weak, nonatomic) IBOutlet UISwitch *switchSensors;

@property (weak, nonatomic) IBOutlet UILabel *labelCustomMediator;
@property (weak, nonatomic) IBOutlet UISwitch *switchCustomMediator;

@property (weak, nonatomic) IBOutlet UILabel *labelMediatorIP;
@property (weak, nonatomic) IBOutlet UITextField *textMediatorIP;

@property (weak, nonatomic) IBOutlet UILabel *labelMediatorPort;
@property (weak, nonatomic) IBOutlet UITextField *textMediatorPort;

@property (weak, nonatomic) IBOutlet UILabel *labelDefaultMediator;
@property (weak, nonatomic) IBOutlet UISwitch *switchDefaultMediator;

@property (weak, nonatomic) IBOutlet UILabel *labelMediatorUsername;
@property (weak, nonatomic) IBOutlet UITextField *textMediatorUsername;

@property (weak, nonatomic) IBOutlet UILabel *labelMediatorPassword;
@property (weak, nonatomic) IBOutlet UITextField *textMediatorPassword;

@property (weak, nonatomic) IBOutlet UILabel *labelTCPPortal;
@property (weak, nonatomic) IBOutlet UISwitch *switchTCPPortal;

@property (weak, nonatomic) IBOutlet UILabel *labelNativeResolution;
@property (weak, nonatomic) IBOutlet UISwitch *switchNativeResolution;

@property (weak, nonatomic) IBOutlet UILabel *labelNativeDecoder;
@property (weak, nonatomic) IBOutlet UISwitch *switchNativeDecoder;

@property (weak, nonatomic) IBOutlet UILabel *labelPortalAutostart;
@property (weak, nonatomic) IBOutlet UISwitch *switchPortalAutostart;

@property (weak, nonatomic) IBOutlet UILabel *labelCLSForDevices;
@property (weak, nonatomic) IBOutlet UISwitch *switchCLSForDevices;

@property (weak, nonatomic) IBOutlet UILabel *labelCLSForDevicesEnforce;
@property (weak, nonatomic) IBOutlet UISwitch *switchCLSForDevicesEnforce;

@property (weak, nonatomic) IBOutlet UILabel *labelCLSForAllTraffic;
@property (weak, nonatomic) IBOutlet UISwitch *switchCLSForAllTraffic;

@property (weak, nonatomic) IBOutlet UILabel *labelShowDebugStatus;
@property (weak, nonatomic) IBOutlet UISwitch *switchShowDebugStatus;

@property (weak, nonatomic) IBOutlet UILabel *labelLogFile;
@property (weak, nonatomic) IBOutlet UISwitch *switchLogFile;

- (void) EnableSettings;
- (void) DisableSettings;

- (IBAction) switchVideoStreaming:(id)sender;
- (IBAction) switchSensors:(id)sender;
- (IBAction) switchTcpPortal:(id)sender;
- (IBAction) switchMediator:(id)sender;
- (IBAction) switchDefaultMediator:(id)sender;
- (IBAction) textCustomMediator:(id)sender;
- (IBAction) textCustomMediatorPort:(id)sender;
- (IBAction) textMediatorUsername:(id)sender;
- (IBAction) textMediatorPassword:(id)sender;

- (IBAction) switchNativeResolution:(id)sender;
- (IBAction) switchNativeDecoder:(id)sender;
- (IBAction) switchPortalAutoStart:(id)sender;
- (IBAction) switchCLSForDevices:(id)sender;
- (IBAction) switchCLSForDevicesEnforce:(id)sender;
- (IBAction) switchCLSForAllTraffic:(id)sender;
- (IBAction) switchLogFile:(id)sender;
- (IBAction) switchDebugStatus:(id)sender;

@end
