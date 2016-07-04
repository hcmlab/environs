/**
 * SettingsTab.mm
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
/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "SettingsTab.h"
#import "MediaBrowser.h"
#import "MainTab.h"
#import "Environs.iosx.h"


extern id                   appCurrentView;
extern MediaBrowser     *   mediaBrowser;

@interface SettingsTab ()

@end

@implementation SettingsTab

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    appCurrentView = self;
    
    [self loadSettings];
    [self updateUI];
    
    MediaBrowser * appDelegate = (MediaBrowser *)[[UIApplication sharedApplication] delegate];
    appDelegate.topViewController = self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    if ( env ) {
        [_labelVersionString setText:[[NSString alloc] initWithFormat:@"Environs: %@", [[NSString alloc] initWithUTF8String: [env GetVersionString]]]];
    }
    
    [MediaBrowser UpdateViewBG:self.view];
    
    [MediaBrowser setSettingsTab:self];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return NO;
}


-(IBAction)dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}


- (void) updateUI
{
    int status = 0;
    
    if ( env ) {
        status = [env GetStatus];
    }
    
    if ( status >= environs::Status::Started )
    {
        [self DisableSettings];
    }
    else
    {
        [self EnableSettings];
    }
}


- (void) loadSettings
{
    NSLog ( @"loadSettings" );
    
    if ( !env )
        return;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        [_switchVideoStreaming setOn:[env GetUseH264]];
        [_switchSensors setOn:[env GetUseSensors]];
        [_switchTCPPortal setOn:[env GetPortalTCP]];
        
        [_switchPortalAutostart setOn:[env GetPortalAutoStart]];
        [_switchNativeResolution setOn:[env GetPortalNativeResolution]];
        [_switchNativeDecoder setOn:[env GetUseNativeDecoder]];
        
        
        [_textMediatorIP setText:[env GetMediatorIP]];
        [_textMediatorPort setText:[@([env GetMediatorPort]) description]];
        
        [_switchDefaultMediator setOn:[env GetUseDefaultMediator]];
        
        bool status = [env GetUseCustomMediator];
        [_switchCustomMediator setOn:status];
        
        [_labelMediatorIP setEnabled:status];
        [_textMediatorIP setEnabled:status];
        _textMediatorIP.delegate = self;
        [[self textMediatorIP] addTarget:self action:@selector(textCustomMediator:) forControlEvents:UIControlEventEditingChanged];
        
        [_labelMediatorPort setEnabled:status];
        [_textMediatorPort setEnabled:status];
        _textMediatorPort.delegate = self;
        [[self textMediatorPort] addTarget:self action:@selector(textCustomMediatorPort:) forControlEvents:UIControlEventEditingChanged];
        
        [[self textMediatorUsername] addTarget:self action:@selector(textMediatorUsername:) forControlEvents:UIControlEventEditingChanged];
        [_textMediatorUsername setText:[env GetMediatorUserName]];
        _textMediatorUsername.delegate = self;
        [self.textMediatorUsername resignFirstResponder];
        
        [[self textMediatorPassword] addTarget:self action:@selector(textMediatorPassword:) forControlEvents:UIControlEventEditingChanged];
        _textMediatorPassword.delegate = self;
        [self.textMediatorPassword resignFirstResponder];
        
        [_switchShowDebugStatus setOn:[env GetUseNotifyDebugMessage]];
        [_switchLogFile setOn:[env GetUseLogFile]];
        
        [_switchCLSForDevicesEnforce setOn:[env GetUseCLSForDevicesEnforce]];
        [_switchCLSForDevices setOn:[env GetUseCLSForDevices]];
        [_switchCLSForAllTraffic setOn:[env GetUseCLSForAllTraffic]];
    });
}

- (void) EnableSettings
{
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_switchVideoStreaming setEnabled:true];
        [_switchSensors setEnabled:true];
        [_switchDefaultMediator setEnabled:true];
        [_switchCustomMediator setEnabled:true];
        [_switchTCPPortal setEnabled:true];
        
        [_labelVideoStreaming setEnabled:true];
        [_labelSensors setEnabled:true];
        [_labelCustomMediator setEnabled:true];
        [_labelDefaultMediator setEnabled:true];
        [_labelTCPPortal setEnabled:true];
        [_labelNativeResolution setEnabled:true];
        [_labelNativeDecoder setEnabled:true];
        
        [_labelMediatorUsername setEnabled:true];
        [_textMediatorUsername setEnabled:true];

        [_labelMediatorPassword setEnabled:true];
        [_textMediatorPassword setEnabled:true];
        
        bool status = 0;
        if ( env ) {
            status = [env GetUseCustomMediator];
        }
        
        [_labelMediatorIP setEnabled:status];
        [_textMediatorIP setEnabled:status];
        [_labelMediatorPort setEnabled:status];
        [_textMediatorPort setEnabled:status];
    });
    
}

- (void) DisableSettings
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [_switchVideoStreaming setEnabled:false];
        [_switchSensors setEnabled:false];
        [_switchDefaultMediator setEnabled:false];
        [_switchCustomMediator setEnabled:false];
        [_switchTCPPortal setEnabled:false];
        
        [_labelVideoStreaming setEnabled:false];
        [_labelSensors setEnabled:false];
        [_labelCustomMediator setEnabled:false];
        [_labelDefaultMediator setEnabled:false];
        [_labelTCPPortal setEnabled:false];
        [_labelNativeResolution setEnabled:false];
        [_labelNativeDecoder setEnabled:false];
        
        [_labelMediatorUsername setEnabled:false];
        [_textMediatorUsername setEnabled:false];
        
        [_labelMediatorPassword setEnabled:false];
        [_textMediatorPassword setEnabled:false];

        [_labelMediatorIP setEnabled:false];
        [_textMediatorIP setEnabled:false];
        [_labelMediatorPort setEnabled:false];
        [_textMediatorPort setEnabled:false];
    });
    
}



- (IBAction) switchVideoStreaming:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseH264:[sw isOn]];
    }
}

- (IBAction) switchSensors:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseSensors:[sw isOn]];
    }
}

- (IBAction) switchTcpPortal:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetPortalTCP:[sw isOn]];
    }
}

- (IBAction) switchNativeResolution:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetPortalNativeResolution:[sw isOn]];
    }
}

- (IBAction) switchNativeDecoder:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( [sw isOn] ) {
        [sw setOn:FALSE];
    }
}

- (IBAction) switchMediator:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    bool status = [sw isOn];
    
    if ( env ) {
        [env SetUseCustomMediator:status];
    }
    
    [_labelMediatorIP setEnabled:status];
    [_labelMediatorIP setOpaque:!status];
    [_textMediatorIP setEnabled:status];
    [_textMediatorIP setOpaque:!status];
    
    [_labelMediatorPort setEnabled:status];
    [_labelMediatorPort setOpaque:!status];
    [_textMediatorPort setEnabled:status];
    [_textMediatorPort setOpaque:!status];
}

- (IBAction) switchDefaultMediator:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    bool status = [sw isOn];
    
    if ( env ) {
        [env SetUseDefaultMediator:status];
    }
}

- (IBAction) textMediatorUsername:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    
    if ( env ) {
        [env SetMediatorUserName:[tf text]];
    }
}

- (IBAction) textMediatorPassword:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    
    if ( env ) {
        [env SetMediatorPassword:[tf text]];
    }
}

- (IBAction) textCustomMediator:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    
    if ( env ) {
        [env SetMediator:[tf text] Port:0];
    }
}

- (IBAction)switchShowDebugLogs:(id)sender {
}

- (IBAction)switchLogIntoFile:(id)sender {
}

- (IBAction) textCustomMediatorPort:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    
    if ( env ) {
        [env SetMediator:0 Port: [[tf text] intValue]];
    }
}


- (IBAction) switchPortalAutoStart:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetPortalAutoStart:[sw isOn]];
    }
}


- (IBAction) switchCLSForDevices:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseCLSForDevices:[sw isOn]];
    }
}


- (IBAction) switchCLSForDevicesEnforce:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseCLSForDevicesEnforce:[sw isOn]];
    }
}


- (IBAction) switchCLSForAllTraffic:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseCLSForAllTraffic:[sw isOn]];
    }
}


- (IBAction) switchDebugStatus:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseNotifyDebugMessage:[sw isOn]];
    }
}


- (IBAction) switchLogFile:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    
    if ( env ) {
        [env SetUseLogFile:[sw isOn]];
    }
}


@end
