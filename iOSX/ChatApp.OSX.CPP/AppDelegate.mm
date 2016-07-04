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

#include "Environs.Native.h"

using namespace std;
using namespace environs;

#define CLASS_NAME  "AppDelegate"

extern ChatAppView      *   chatAppView;
extern LogView          *   logView;

AppDelegate             *   appDelegate = nil;


@interface AppDelegate ()
{
    LogWindow   *   logWindow;
    
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
}

@property (weak) IBOutlet NSMenuItem *menuItemStartStop;

@end




@implementation AppDelegate

- (id) init
{
    CVerbN ( "init" );
    
    self = [super init];
    
    if ( self ) {
        appDelegate = self;
        
        INIT_ENVIRONS_LOG ();
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
    
    if ( env ) {
        ChatAppView * chatApp = chatAppView;
        if ( chatApp ) {
            chatApp->currentDevice = 0;
            chatApp->deviceList = 0;

            [chatApp StopInitThread];
            [chatApp StopListThread];
        }
        
        env->RemoveObserver ( observer.get () );
        env->RemoveObserverForMessages ( observer.get () );
        env->RemoveObserverForData ( observer.get () );

        env->SetAppShutdown ( true );
        env->Stop ();
        env->DisposeInstance ();
        env = 0;
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
    
    if ( status < 0 && env )
        status = env->GetStatus ();
    
    @autoreleasepool {
        NSString * startStopTitle = nil;
        
        if ( status == environs::Status::Started )
        {
            startStopTitle = @"Stop Environs";
        }
        else if ( status <= environs::Status::Stopped )
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


- (IBAction) ActionRestart:(id) sender
{
    if ( env ) {
        env->Stop ();
        
        if ( chatAppView && chatAppView->deviceList )
            chatAppView->deviceList = nil;
    }

    [self StartEnvirons];
}


- (void) StartStopEnvirons
{
    if ( !env )
        return;
    
    if ( env -> GetStatus () >= environs::Status::Started ) {
        env -> Stop ();
        
        if ( chatAppView ) {
            if ( chatAppView->deviceList )
                chatAppView->deviceList = nil;
            
            chatAppView->currentDevice = nil;
        }
    }
    else {
        [self StartEnvirons];
        
        if ( chatAppView && chatAppView->deviceList )
            chatAppView->deviceList->Reload ();
    }
}


- (void) StartEnvirons
{
    if ( !observer )
        observer = make_shared < Observer > ();
    
    if ( observer ) {
        if ( !env )
            env = environs::Loader::CreateInstance ();
        
        if ( env ) {
            env->LoadSettings ( "ChatApp", "Environs" );

            env->SetUseCLSForDevicesEnforce ( false );
            
#ifdef CHATAPP1
            env->SetUseDeviceListAsUIAdapter ( false );
#endif
            env->AddObserver ( observer.get () );
            env->AddObserverForMessages ( observer.get () );
            env->AddObserverForData ( observer.get () );
            
            env->SetUseMediatorAnonymousLogon ( true );
            
            env->SetUseLogFile ( true );
            
            if ( chatAppView )
                [chatAppView InitDeviceList];
            
            env->Start ();
        }
    }
}

- (IBAction) ActionReloadDevicelist:(id) sender
{
}


/**
 * Environs observers
 *
 */

/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param status      A status constant of type environs::Status
 */
void Observer::OnStatus ( environs::Status_t status )
{
    CVerbArgN ( "OnStatus: [%i]", status );
    
    @autoreleasepool {
        if ( appDelegate )
            [appDelegate UpdateUI:status];
        else if ( chatAppView )
            [chatAppView UpdateUI:status];
    }
}


/**
 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
 *
 * @param msg      The status as a text message.
 */
void Observer::OnStatusMessage ( const char * message )
{
    if ( !message )
        return;
    
    if ( appDelegate )
        [appDelegate UpdateStatusMessage:message];
}


/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param nativeID      The native identifier that targets the device.
 * @param notification  The notification
 * @param sourceIdent   A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
void Observer::OnNotify ( environs::ObserverNotifyContext * context )
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %i", context->destID, context->sourceIdent, context->notification );
    
    if ( !chatAppView )
        return;
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        [chatAppView UpdateList];
    }
    
    else if ( context->notification == NOTIFY_START_SUCCESS )
    {
        [chatAppView InitDeviceList];
    }
    
    else if ( context->notification == NOTIFY_STOP_SUCCESS ) {
        [chatAppView InitDeviceList];
    }
}


/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The PortalInstance object.
 */
void Observer::OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal )
{
    CLogN ( "OnPortal" );
    
    if ( portal == 0 || !appDelegate )
        return;
    
    portal->AddObserver ( appDelegate->observer.get () );
    portal->Establish ( true );
}


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
    @autoreleasepool {
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
        
        [self UpdateLogView];
        
        [logTextLock unlock];
    }
}

@end







