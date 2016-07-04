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
#import "AppDelegate.h"
#import "ChatAppView.h"
#import "Environs.iosx.h"
#include "Environs.native.h"

#define CLASS_NAME  "SettingsTab. . . . . . ."


extern AppDelegate      *   appDelegate;
extern ChatAppView      *   chatAppView;
extern NSString         *   statusMessage;
extern NSString         *   loginUserName;

@interface SettingsTab ()

@property (weak, nonatomic) IBOutlet UISwitch *switchParticipate;

@end

@implementation SettingsTab

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerbN ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}


- (void) viewWillAppear:(BOOL)animated
{
    CVerbN ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    
    [self loadSettings];
    [self updateUI];
}


- (IBAction) ToggleEnvirons:(id)sender
{
    CVerbN ( "ToggleEnvirons" );
    
#ifdef CHATAPP1
    if ( !env )
        return;
    
    int status = [env GetStatus];
    
    if ( status >= environs::Status::Started ) {
        [env Stop];
        [self EnableSettings];
    }
    else {
        [env Start];
        [self DisableSettings];
        
        if (chatAppView)
            [chatAppView InitDeviceList];
    }
#else
#endif
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    [super viewDidLoad];
    
#ifdef CHATAPP1
    if ( env )
        [_labelVersionString setText:[[NSString alloc] initWithFormat:@"Environs: %@", [[NSString alloc] initWithUTF8String: [env GetVersionString]]]];
#else
#endif
    
    [AppDelegate UpdateViewBG:self.view];
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}


- (BOOL) textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return NO;
}


- (IBAction) dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}


- (void) updateUI
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    environs::Status_t status = [env GetStatus];
#else
    environs::Status_t status = environs::Status::Uninitialized;
#endif
    
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
    
    dispatch_async(dispatch_get_main_queue(), ^{
#ifdef CHATAPP1
        if ( !env )
            return;
        
        [_switchVideoStreaming setOn:[env GetUseH264]];
        [_switchSensors setOn:[env GetUseSensors]];
        
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
#else
#endif
    });
}


- (void) EnableSettings1
{
    
}

- (void) EnableSettings
{
    NSLog ( @"EnableSettings" );
    
#ifdef CHATAPP1
    if ( !env )
        return;
#else
#endif
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_textUserStatus setText:statusMessage];
        [_textNickName setText:loginUserName];
        
        [_switchVideoStreaming setEnabled:true];
        [_switchSensors setEnabled:true];
        [_switchDefaultMediator setEnabled:true];
        [_switchCustomMediator setEnabled:true];
        
        [_labelVideoStreaming setEnabled:true];
        [_labelSensors setEnabled:true];
        [_labelCustomMediator setEnabled:true];
        [_labelDefaultMediator setEnabled:true];
        
        [_labelMediatorUsername setEnabled:true];
        [_textMediatorUsername setEnabled:true];
        
        [_labelMediatorPassword setEnabled:true];
        [_textMediatorPassword setEnabled:true];
        
#ifdef CHATAPP1
        bool status = [env GetUseCustomMediator];
#else
        bool status = false;
#endif
        [_labelMediatorIP setEnabled:status];
        [_textMediatorIP setEnabled:status];
        [_labelMediatorPort setEnabled:status];
        [_textMediatorPort setEnabled:status];
    });
}


- (void) DisableSettings
{
    NSLog ( @"DisableSettings" );
    
#ifdef CHATAPP1
    if ( !env )
        return;
#else
#endif
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_switchVideoStreaming setEnabled:false];
        [_switchSensors setEnabled:false];
        [_switchDefaultMediator setEnabled:false];
        [_switchCustomMediator setEnabled:false];
        
        [_labelVideoStreaming setEnabled:false];
        [_labelSensors setEnabled:false];
        [_labelCustomMediator setEnabled:false];
        [_labelDefaultMediator setEnabled:false];
        
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
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseH264:[sw isOn]];
#else
#endif
}

- (IBAction) switchSensors:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseSensors:[sw isOn]];
#else
#endif
}


- (IBAction) switchMediator:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UISwitch * sw = (UISwitch *)sender;
    bool status = [sw isOn];
    [env SetUseCustomMediator:status];
#else
    bool status = false;
#endif
    
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
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UISwitch * sw = (UISwitch *)sender;
    bool status = [sw isOn];
    [env SetUseDefaultMediator:status];
#else
#endif
}

- (IBAction) textMediatorUsername:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UITextField * tf = (UITextField *)sender;
    [env SetMediatorUserName:[tf text]];
#else
#endif
}

- (IBAction) textMediatorPassword:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UITextField * tf = (UITextField *)sender;
    [env SetMediatorPassword:[tf text]];
#else
#endif
    
//    [[self view] endEditing:YES];
}

- (IBAction) textCustomMediator:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UITextField * tf = (UITextField *)sender;
    [env SetMediator:[tf text] Port:0];
#else
#endif
}

- (IBAction)switchShowDebugLogs:(id)sender {
}

- (IBAction)switchLogIntoFile:(id)sender {
}

- (IBAction) textCustomMediatorPort:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    
    UITextField * tf = (UITextField *)sender;
    [env SetMediator:0 Port: [[tf text] intValue]];
#else
#endif
}

- (IBAction) textNickName:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    loginUserName = [tf text];
    
    NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
    if ( !prefs ) {
        return;
    }
    
    [prefs setValue:[tf text] forKey:@"userNickName"];
    [prefs synchronize];
}

- (IBAction) textUserStatus:(id)sender
{
    UITextField * tf = (UITextField *)sender;
    statusMessage = [tf text];
    
    NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
    if ( !prefs ) {
        return;
    }
    
    [prefs setValue:[tf text] forKey:@"userStatus"];
    [prefs synchronize];
}


- (IBAction) switchCLSForDevices:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseCLSForDevices:[sw isOn]];
#else
#endif
}


- (IBAction) switchCLSForDevicesEnforce:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseCLSForDevicesEnforce:[sw isOn]];
#else
#endif
}


- (IBAction) switchCLSForAllTraffic:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseCLSForAllTraffic:[sw isOn]];
#else
#endif
}


- (IBAction) switchDebugStatus:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseNotifyDebugMessage:[sw isOn]];
#else
#endif
}


- (IBAction) switchLogFile:(id)sender
{
#ifdef CHATAPP1
    if ( !env )
        return;
    UISwitch * sw = (UISwitch *)sender;
    [env SetUseLogFile:[sw isOn]];
#else
#endif
}


@end
