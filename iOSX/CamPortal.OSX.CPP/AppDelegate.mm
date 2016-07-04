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

using namespace std;
using namespace environs;

#define CLASS_NAME  "AppDelegate. . . . . . ."

AppDelegate             *   appDelegate = nil;


@interface AppDelegate ()
{
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
- (void) UpdateUI : (environs::Status_t) status
{
    CVerbN ( "UpdateUI" );
        
    if ( status < 0 && env )
        status = env->GetStatus ();
    
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
        [appView UpdateUI:status];
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
        env->Stop ();
    
    [self StartEnvirons];
}


- (void) StartStopEnvirons
{
    if ( !env )
        return;
    
    if ( env -> GetStatus () >= environs::Status::Started ) {
        env -> Stop ();
    }
    else {
        [self StartEnvirons];
        
        if ( appView && appView->deviceList )
            appView->deviceList->Reload ();
    }
}


- (void) StartEnvirons
{
    if ( !env ) {
        //env = Environs_CreateInstanceStaticLinked();
        
        env = environs::Loader::CreateInstance ( "CamPortal", "Environs" );
        if ( !env )
            return;
        
        observer = make_shared < Observer > ();
        if ( observer )
        {
            env->AddObserver ( observer.get() );
            
            env->AddObserverForMessages ( observer.get() );
        }
    }
    
    if ( env ) {        
        env->SetUseMediatorAnonymousLogon ( true );
        env->SetUsePortalDefaultModules ();
        
        env->SetUseCapturer ( "libEnv-CapCamera" );
        env->SetUseRenderer ( "libEnv-RendNull" );
        
        env->Start ();
        
        if ( appView )
            [appView InitDeviceList];
    }
}



/**
 * Region: Environs observers.
 *
 */
/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param status      A status constant of type STATUS_*
 */
void Observer::OnStatus ( environs::Status_t status )
{
    CVerbArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    if ( appDelegate )
        [appDelegate UpdateUI:status];
}


/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The IPortalInstance object.
 */
void Observer::OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal )
{
    CLogN ( "OnPortal" );
    
    if ( portal == nil || !appDelegate || !appView )
        return;

    portal->AddObserver ( appDelegate->observer.get () );

    if ( portal->isIncoming () ) {
        portal->SetRenderSurface ( (__bridge void *)[appView GetRenderView] );
        
        portal->startIfPossible = true;
    }
    
    portal->Establish ( true );
}


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
