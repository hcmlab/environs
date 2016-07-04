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

using namespace std;
using namespace environs;


#define CLASS_NAME  "AppDelegate"

AppDelegate * appDelegate = nil;

extern RemoteTouchView * appView;


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
- (void) applicationDidFinishLaunching:(NSNotification *) aNotification {
    appDelegate = self;
    
    [self StartEnvirons];
}


- (void) applicationWillTerminate:(NSNotification *) aNotification
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


- (void) StartEnvirons
{
    if ( !env ) {
        env = environs::Loader::CreateInstance ();
    }
    
    if ( env ) {
        env->SetApplicationName ( "RemoteTouch" );
        env->SetAreaName ( "Environs" );
        env->LoadSettings ();
        
        observer = make_shared < Observer > ();
        if ( observer ) {
            if ( env ) {
                env->AddObserver ( observer.get() );
                
                env->AddObserverForMessages ( observer.get() );
            }
        }
        
        env->SetUseMediatorAnonymousLogon ( true );
        
        env->Start ();
    }
}


/**
 * Environs observers
 *
 */

/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param status      A status constant of type STATUS_*
 */
void Observer::OnStatus ( Status_t status )
{
    CVerbArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    if ( appView )
        [appView UpdateUI];
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
    
    if ( appView )
        [appView UpdateStatusMessage:message];
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
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        if ( appView )
            [appView UpdateDeviceList:appDelegate];
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


/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param deviceID      The device id of the sender device.
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @param notification  The notification
 * @param sourceIdent   A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
void Observer::OnNotifyExt ( environs::ObserverNotifyContext * context )
{
    CVerbArgN ( "onNotify: [%8X] [%10i] | %s", context->destID, context->sourceIdent, environs::resolveName(context->notification) );
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        if ( appView )
            [appView UpdateDeviceList:appDelegate];
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
    
    if ( portal == 0 )
        return;
    
    /*PortalInstance * portal = (PortalInstance *)portalInstance;
     
     //[portal AddObserver:appView];
     [portal Establish:true];
     */
    
}



@end











