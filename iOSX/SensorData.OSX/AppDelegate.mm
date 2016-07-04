/**
 * AppDelegate of SensorData
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
#import "SensorDataView.h"

#include "Environs.Native.h"

#define CLASS_NAME  "AppDelegate"


AppDelegate * appDelegate   = nil;
Environs    * env           = nil;


extern SensorDataView * appView;


@interface AppDelegate ()
{
}


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
    if ( !env )
        env = [Environs CreateInstance: "SensorData" Area: "Environs" ];
    
    if ( env ) {        
        [env AddObserver:self];
        [env AddObserverForMessages:self];
        [env AddObserverForData:self];
        
        [env SetUseMediatorAnonymousLogon:true];
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


- (void) UpdateDeviceID
{
    if ( env != nil && appView != nil ) {
        int deviceID = [env GetDeviceID];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [appView setTitle: [ [NSString alloc ] initWithFormat:@"SensorData 0x%X", deviceID ] ];
        });
    }
}


- (void) OnStatus:(environs::Status_t) status
{
    CVerbArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    if ( appView )
        [appView UpdateUI];
    
    if ( status >= environs::Status::Started ) {
        [self UpdateDeviceID];
    }
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
    
    else if ( context->notification == NOTIFY_MEDIATOR_SERVER_CONNECTED )
    {
        [self UpdateDeviceID];
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


- (void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
}


@end
