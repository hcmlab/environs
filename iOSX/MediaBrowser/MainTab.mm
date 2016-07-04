/**
 * MainTab.mm
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

#import "MainTab.h"
#import "MediaBrowser.h"
#import "FullScreenController.h"
#import "DevicesListView.h"

#import "Environs.iosx.h"
#include "Environs.native.h"

#define CLASS_NAME  "MainTab. . . . . . . . ."

extern id               appCurrentView;
extern DeviceListView      *   deviceListInstance;

@interface MainTab ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
}

@property (weak, nonatomic) IBOutlet UILabel *label_WiFi;

@end


@implementation MainTab

@synthesize viewFullScreen;
@synthesize buttonStart;


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [_labelStatus setNumberOfLines:0];
    CGSize size = _labelStatus.frame.size;
    size.width = self.view.frame.size.width - 10;
    _labelStatus.frame.size = size;
    //[_labelStatus sizeToFit];
    
    [MediaBrowser UpdateViewBG:self.view];
    
    [MediaBrowser setMainTab:self];    
}


- (void) UpdateUI
{
    if ( !env )
        return;
    
    int status = [env GetStatus];

    if ( status >= environs::Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            //[buttonStart setAttributedTitle:nil forState:UIControlStateNormal];
            buttonStart.enabled = FALSE;
            [buttonStart setTitle:@"Stop" forState:UIControlStateNormal];
            buttonStart.enabled = TRUE;
            [buttonStart setNeedsLayout];
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            buttonStart.enabled = FALSE;
            [buttonStart setTitle:@"Start" forState:UIControlStateNormal];
            buttonStart.enabled = TRUE;
            [buttonStart setNeedsLayout];
        });
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        [logTextLock lock];
        [_labelStatus setText:logText];
        //[_labelStatus sizeToFit];
        [logTextLock unlock];
        
        NSString * ssid = [env GetSSIDDesc];
        [_label_WiFi setText:ssid];
        
        CVerbArg ( "updateUI: Currently conntected to [%s]", [ssid UTF8String] );
    });
}


- (void) viewWillAppear:(BOOL)animated
{
    CVerb ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    
    appCurrentView = self;
    
    if ( env ) {
        [[self textDeviceID] setText:[[NSString alloc] initWithFormat:@"%x", [env GetDeviceID]]];
        
        [env SetMediatorNotificationSubscription:false];
    }
    [[self textDeviceID] addTarget:self action:@selector(deviceIDChanged:) forControlEvents:UIControlEventEditingChanged];

    [self UpdateUI];
    
    MediaBrowser * appDelegate = (MediaBrowser *) [[UIApplication sharedApplication] delegate];
    [appDelegate setTopViewController:self];
    
    /*if ([[self.navigationController navigationBar] isHidden] ) {
        [[self.navigationController navigationBar] setHidden:false];
    }
    */
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    CVerb ( "didReceiveMemoryWarning" );
}


+ (MainTab *) getInstance
{
    CVerb ( "getInstance" );
    
    return self;
}


- (IBAction) deviceIDChanged:(id)sender
{
    CVerb ( "deviceIDChanged" );

    UITextField * tf = (UITextField *)sender;

    NSString * deviceIDString = [tf text];
    NSScanner* scanner = [NSScanner scannerWithString:[[NSString alloc] initWithFormat:@"0x%@", deviceIDString]];
    
    unsigned int value = 0;
    [scanner scanHexInt:&value];
    
    if ( env ) {
        [env SetDeviceID:value];
    }
}


- (BOOL) textFieldShouldReturn: (UITextField *)textField
{
    [_textDeviceID resignFirstResponder];
    return NO;
}


- (IBAction) StartEnvirons: (id)sender
{
    CVerb ( "StartEnvirons" );
    
    if ( env ) {
        [env Start];
    }
    
    if (deviceListInstance)
        [deviceListInstance InitDeviceList];
}


- (IBAction) AddMessage: (NSString *) msg
{
    [logTextLock lock];

    if ( logTextLines > 23 ) {
        NSRange range = [logText rangeOfString:@"\n"]; // options:NSBackwardsSearch
        if ( range.length != NSNotFound ) {
            //logText = [logText substringToIndex: range.location + 1];
            logText = [logText substringFromIndex: range.location + 1];
        }
        else
            return;
    }
    else
        logTextLines++;
    
    if ( [msg hasSuffix:@"\n"] )
        logText = [[NSString alloc] initWithFormat:@"%@%@", logText, msg];
    else
        logText = [[NSString alloc] initWithFormat:@"%@\n%@", logText, msg];
    
    //logText = [[NSString alloc] initWithFormat:@"%@\n%@", msg, logText];
//    logText = [[NSString alloc] initWithString:[[[NSString alloc] initWithString:[logText stringByAppendingString:@"\n"]] stringByAppendingString:msg]];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_labelStatus setText:[[NSString alloc] initWithString:logText]];
        //[_labelStatus sizeToFit];
    });

    [logTextLock unlock];

}


- (IBAction) AddStatusMessage: (const char *) message
{
    [self AddMessage:[[NSString alloc] initWithCString:message encoding:NSUTF8StringEncoding]];
}


-(IBAction) dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}

@end
