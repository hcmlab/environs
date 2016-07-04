/**
 * AppDelegate of ChatApp
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

#import "AppDelegate.h"
#import "CamPortalView.h"

#include "Environs.Native.h"

#define CLASS_NAME  "AppDelegate. . . . . . ."

AppDelegate             *   appDelegate = nil;


@interface AppDelegate ()
{
    Environs * env;
    
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
}

@property (weak) IBOutlet NSMenuItem *menuItemStartStop;

@end




@implementation AppDelegate

- (Environs *) env { return env; }

- (id) init
{
    CVerbN ( "init" );
    
    self = [super init];
    
    if ( self ) {
        appDelegate = self;
    }
    
    return self;
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    appDelegate = self;
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [self StartEnvirons];
}


- (void) applicationWillTerminate:(NSNotification *)aNotification
{
    if ( env ) {
        [env Stop];
        env = nil;
    }
}


- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}


/**
 * Update menu items of the main menu
 *
 */
- (void) UpdateUI:(int) status
{
    CVerbN ( "UpdateUI" );
    
    if ( status < 0 ) {
        if ( env )
            status = [env GetStatus];
    }
    
    NSString * startStopTitle = nil;
    
    if ( status >= environs::Status::Started )
    {
        if ( [[_menuItemStartStop title] hasPrefix:@"Start"] )
            startStopTitle = @"Stop Environs";
    }
    else
    {
        if ( [[_menuItemStartStop title] hasPrefix:@"Stop"] )
            startStopTitle = @"Start Environs";
    }
    
    if ( startStopTitle != nil ) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [_menuItemStartStop setTitle:startStopTitle];
        });
    }
    
    if ( appView )
        [appView UpdateUI];
}


+ (void) UpdateMessageList:(DeviceInstance *)device
{    
}


/**
 * Region: Start and Stop (or Restart) of Environs.
 *
 */
- (IBAction) ActionStartStop:(id) sender
{
    [self StartStopEnvirons];
}


- (IBAction) ActionRestart:(id) sender
{
    if ( env )
        [env Stop];
    
    [self StartEnvirons];
}


- (void) StartStopEnvirons
{
    if ( !env )
        return;
    
    if ( [env GetStatus] >= environs::Status::Started ) {
        [env Stop];
    }
    else {
        [self StartEnvirons];
        
        if ( appView->deviceList )
            [appView->deviceList Reload];
    }
}


- (void) StartEnvirons
{
    if ( !env ) {
        env = [Environs CreateInstance: "CamPortal" Area: "Environs" ];
    }
    
    if ( env ) {        
        [env AddObserver:self];
        
        [env SetUsePortalDefaultModules];
        
        [env SetUseCapturer:"libEnv-CapCamera"];
        
        [env SetUseRenderer:"libEnv-RendNull"];
        
        [env SetUseDecoder:"libEnv-DecOpenH264"];
        [env SetUseEncoder:"libEnv-EncOpenH264"];
        
        [env Start];
    }
    
    if ( appView )
        [appView InitDeviceList];
}



/**
 * Region: Environs observers.
 *
 */
-(void) OnStatus:(environs::Status_t) status
{
    [self UpdateUI:status];
}


- (void) OnNotify:(environs::ObserverNotifyContext *)context
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
}


- (void) OnNotifyExt:(environs::ObserverNotifyContext *)context
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
}


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
    
    if ( portalInstance == nil )
        return;
    
    PortalInstance * portal = (PortalInstance *)portalInstance;

    [portal AddObserver:appView];

    if ( portal.isIncoming ) {
        [portal SetRenderSurface: [appView GetRenderView]];
        
        portal.startIfPossible = true;
    }
    
    [portal Establish:true];
}
#endif


- (void) UpdateStatusMessage: (const char *) message
{
    NSString * msg = [[NSString alloc ] initWithCString:message encoding:NSUTF8StringEncoding];
    
    [logTextLock lock];
    
    if ( logTextLines > 40 ) {
        NSRange range = [logText rangeOfString:@"\n"];
        if ( range.length != NSNotFound ) {
            logText = [logText substringFromIndex: range.location + 1];
        }
        else
            return;
    }
    else
        logTextLines++;
    
    if ( [msg hasSuffix:@"\n"] )
        logText = [[NSString alloc ] initWithFormat:@"%@%@", logText, msg];
    else
        logText = [[NSString alloc ] initWithFormat:@"%@\n%@", logText, msg];
    
    [logTextLock unlock];
}

@end
