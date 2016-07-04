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
#import "ChatAppView.h"
#import "LogView.h"
#import "LogWindow.h"

#define USE_STATIC_ENVIRONS_LOG

#include "Environs.h"
#include "Environs.Native.h"
#include "Log.h"


#define CLASS_NAME  "AppDelegate"

extern ChatAppView      *   chatAppView;
extern LogView          *   logView;

AppDelegate             *   appDelegate = nil;


@interface AppDelegate ()
{    
    Environs    *   env;
    
    LogWindow   *   logWindow;
    
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


- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
    CVerbN ( "applicationDidFinishLaunching" );
    
    logWindow = nil;
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [self StartEnvirons];
}


- (void) applicationWillTerminate:(NSNotification *)aNotification
{
    CVerbN ( "applicationWillTerminate" );
    
    ChatAppView * chatApp = chatAppView;
    if ( chatApp ) {
        [chatApp StopInitThread];
        [chatApp StopListThread];
    }
    
    if ( env ) {
        [env RemoveObserver:self];
        [env RemoveObserverForMessages:self];

        env.async = environs::Call::Wait;
        [env SetAppShutdown:true];
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
- (void) UpdateUI:(environs::Status_t) status
{
    CVerbN ( "UpdateUI" );
    
    if ( env )
        status = [env GetStatus];
    
    @autoreleasepool {
        NSString * startStopTitle = nil;
        
        if ( status == environs::Status::Started )
        {
            startStopTitle = @"Stop Environs";
        }
        else if ( status == environs::Status::Stopped )
        {
            startStopTitle = @"Start Environs";
        }
        
        if ( startStopTitle != nil ) {
            dispatch_async(dispatch_get_main_queue(), ^{
                
                @autoreleasepool {
                    [_menuItemStartStop setTitle:startStopTitle];
                }
            });
        }
        
        if ( chatAppView )
            [chatAppView UpdateUI:status];        
    }
}


/**
 * Region: Start and Stop (or Restart) of Environs.
 *
 */
- (IBAction) ActionStartStop:(id) sender
{
    [self StartStopEnvirons];
}

#define TEST_DISPOSE

- (IBAction) ActionRestart:(id) sender
{
    if ( env ) {
        env.async = environs::Call::Wait;
        [env Stop];
        
#ifdef TEST_DISPOSE
        if ( chatAppView ) {
            [chatAppView ClearDeviceList];
            
            if ( chatAppView->deviceList )
                chatAppView->deviceList = nil;
        }
        
        env = nil;
        return;
#else
        env.async = environs::Call::NoWait;
        if ( chatAppView ) {
            [chatAppView ClearDeviceList];
#endif
    }
    
    [self StartEnvirons];
}


- (void) StartStopEnvirons
{
    if ( !env )
        return;
    
    if ( [env GetStatus] >= environs::Status::Started ) {        
            
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                                     (unsigned long)NULL), ^(void) {
                Environs * envObj = env;
                if ( !envObj )
                    return;
                
                [envObj Stop];
                
                if ( chatAppView )
                    [chatAppView ClearDeviceList];
            });
    }
    else {
        [self StartEnvirons];
        
        if ( chatAppView->deviceList )
            [chatAppView->deviceList Reload];
    }
}


- (void) StartEnvirons
{
    if ( !env ) {
        env = [Environs CreateInstance];
    }
    
    if ( env ) {
        [env ClearStorage];
        
        CLog ( "Loading Settings ..." );
        
        [env LoadSettings: "ChatApp" Area:"Environs" ];
        
        [env SetUseDefaultMediator:true];
        [env SetUseCustomMediator:false];
        
        /*[env SetUseDefaultMediator:false];
        
        [env SetMediator:@"127.0.0.1" Port:5898];
        
        [env SetUseCustomMediator:true];
        */
        [env SetUseCLSForDevicesEnforce:false];
        
        [env SetUseMediatorAnonymousLogon:true];

#ifdef CHATAPP1
        [env SetUseDeviceListAsUIAdapter:false];
#endif        
        [env AddObserver:self];
        [env AddObserverForMessages:self];
        
        [env SetUseLogFile:true];
        
        if ( chatAppView )
            [chatAppView InitDeviceList];
        
        [env Start];
    }
}


- (IBAction) ActionReloadDevicelist:(id) sender
{
    if ( chatAppView->deviceList )
        [chatAppView UpdateList];
}



/**
 * Region: Environs observers.
 *
 */
-(void) OnStatus:(environs::Status_t) status
{
    CVerbArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    @autoreleasepool {
        if ( appDelegate )
            [appDelegate UpdateUI:status];
        else if ( chatAppView )
            [chatAppView UpdateUI:status];
    }
}

-(void) OnStatusMessage:(const char *) message
{
    if ( !message )
        return;
    
    [self UpdateStatusMessage:message];
}


-(void) OnNotify:(environs::ObserverNotifyContext *)context
{
    if ( context->notification == NOTIFY_CONNECTION_ESTABLISHED_ACK ) {
        CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
        //CLogN ( "OnNotify: OK" );
    }
    
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );

    if ( context->notification == NOTIFY_START_SUCCESS )
        [chatAppView InitDeviceList];
    
    else if ( context->notification == NOTIFY_STOP_SUCCESS )
        [chatAppView InitDeviceList];
}


/**
 * OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param deviceID      The device id of the sender device.
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @param notification  The notification
 * @param source   		A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
- (void) OnNotifyExt:(environs::ObserverNotifyContext *)context
{
    
}


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
    
    if ( portalInstance == nil )
        return;
    
    PortalInstance * portal = (PortalInstance *)portalInstance;

    [portal AddObserver:chatAppView];
    [portal Establish:true];    
}
#endif


/**
 * Region: Log window logic. Not necessary for real apps.
 *
 */
- (IBAction) OpenLogView:(id)sender
{
    if ( logWindow != nil ) {
        logWindow = nil;
        return;
    }
    
    NSStoryboard * storyBoard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    if ( !storyBoard )
        return;
    
    logWindow = [storyBoard instantiateControllerWithIdentifier:@"LogWindowController"];
    if ( !logWindow )
        return;
    
    [logWindow showWindow:self];
    
    // Reposition window
    if ( chatAppView != nil ) {
        NSRect frame = logWindow.window.frame;
        
        NSArray * wins = [[NSApplication sharedApplication] windows];
        if ( [wins count] >= 1 ) {
            NSWindow * mWin = [wins objectAtIndex:0];
            
            frame.origin.y = mWin.frame.origin.y;
            frame.origin.x = mWin.frame.origin.x - frame.size.width - 10;
            
            [logWindow.window setFrame:frame display:YES animate:YES];
            
        }
    }
}
    
    
- (void) DisposeViews {
    @autoreleasepool {
        logWindow = nil;
        
    }
}


- (void) UpdateLogView
{
    @autoreleasepool {
        if ( logView != nil )
            [logView UpdateStatusMessage:logText];
    }
}


- (void) UpdateStatusMessage: (const char *) message
{
    NSString * msg = [[NSString alloc ] initWithCString:message encoding:NSUTF8StringEncoding];
    
    [logTextLock lock];
    
    @autoreleasepool
    {
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
        
        [self UpdateLogView];
    }
    
    [logTextLock unlock];
}


/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param nativeID      The native identifier that targets the device.
 * @param type	        Determines the source (either from a device, environs, or native layer)
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessage:(environs::ObserverMessageContext * )context
{
}

/**
 * OnMessageExt is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param deviceID      The device id of the sender device.
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @param type	        Determines the source (either from a device, environs, or native layer)
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessageExt:(environs::ObserverMessageContext *)context
{
}


@end
