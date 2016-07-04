/**
 * AppDelegate of RemoteTouch
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
#import "RemoteTouchView.h"

#include "Environs.Native.h"

#define CLASS_NAME  "AppDelegate"

AppDelegate * appDelegate = nil;

extern RemoteTouchView * appView;


@interface AppDelegate ()
{
    Environs * env;
}


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


/**
 * Application delegates
 *
 */
- (void) applicationDidFinishLaunching:(NSNotification *) aNotification
{
    appDelegate = self;

    [self StartEnvirons];
}


- (void) applicationWillTerminate:(NSNotification *) aNotification
{
    if ( env ) {
        [env Stop];
        env = nil;
    }
}


- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}


- (void) StartEnvirons
{
    if ( !env ) {
        env = [Environs CreateInstance];
    }
    
    if ( env ) {
        [env SetApplicationName: "RemoteTouch" ];
        [env SetAreaName: "Environs" ];
        [env LoadSettings];
        
        [env AddObserver:self];
        [env AddObserverForMessages:self];
        [env AddObserverForData:self];
        
        [env SetUseMediatorAnonymousLogon:true];
        
        [env SetUseEncoder:"libEnv-EncOpenH264"];
        
        [env Start];
    }
}


/**
* Environs observers
*
*/
- (void) OnData:(environs::ObserverDataContext *) context
{
    
}


- (void) OnMessage:(environs::ObserverMessageContext *) context
{
}


- (void) OnMessageExt:(environs::ObserverMessageContext *) context
{
}


- (void) OnStatus:(environs::Status_t) status
{
    CVerbArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    if ( appView )
        [appView UpdateUI];
}


- (void) OnStatusMessage:(const char *) message
{
    if ( !message )
        return;
    
    if ( appView )
        [appView UpdateStatusMessage:message];
}


- (void) OnNotify:(environs::ObserverNotifyContext *)context
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        if ( appView )
            [appView UpdateDeviceList:self];
    }
    
    else if ( context->notification == NOTIFY_START_SUCCESS )
    {
        if ( appView )
            [appView ReInitDeviceList];
    }
    
    else if ( context->notification == NOTIFY_STOP_SUCCESS ) {
        if ( appView )
            [appView ReleaseDeviceList];
    }
}


- (void) OnNotifyExt:(environs::ObserverNotifyContext *)context
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        if ( appView )
            [appView UpdateDeviceList:self];
    }
}


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
    
    if ( portalInstance == nil )
        return;
    
    PortalInstance * portal = (PortalInstance *)portalInstance;

    [portal AddObserver:appView];
    [portal Establish:true];
}
#endif

@end
