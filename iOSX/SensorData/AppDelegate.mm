/**
 * MediaBrowser.mm
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
#import "SettingsTab.h"
#import "SensorDataView.h"
#import "Tabs.h"

#import "Environs.iOSX.h"
#include "Environs.Native.h"

#define CLASS_NAME  "AppDelegate. . . . . . ."


AppDelegate             *   appDelegate = nil;
Environs                *   env         = nil;

extern SensorDataView      *   sensorDataView;


@interface AppDelegate ()
{
    DeviceList  *   deviceList;
}
@end


@implementation AppDelegate


- (Environs *) env { return env; }


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    appDelegate = self;
    
    [self StartEnvirons];
    return YES;
}


- (void) StartStopEnvirons
{
    if ( !env ) return;
    
    if ( [env GetStatus] >= environs::Status::Started ) {
        [env Stop];
    }
    else {
        [self StartEnvirons];
    }
}


- (void) StartEnvirons
{
    if ( !env )
        env = [ Environs CreateInstance:"SensorData" Area:"Environs" ];
    
    if ( !env )
        return;
    
    [env AddObserver:self];
    
    [env SetUseMediatorAnonymousLogon:true];
    
    [env Start];
}

							
- (void)applicationWillResignActive:(UIApplication *)application
{
}


- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if ( env ) {
        [env Stop];
    }
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    if ( env ) {
        [env Start];
    }
    
    if (sensorDataView)
        [sensorDataView InitSensorData];
}


- (void)applicationDidBecomeActive:(UIApplication *)application
{
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    if ( env ) {
        [env RemoveObserver:self];
        
        [env Stop];
        [env Dispose];
    }
}


- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
    
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:[[[NSBundle mainBundle] infoDictionary]   objectForKey:@"CFBundleName"]
                                                        message:notification.alertBody
                                                       delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
    
    [alertView show];
    
    application.applicationIconBadgeNumber = 0;
}


+ (void) UpdateViewBG:(UIView *) view
{
    UIImage * bg = [self GetBackground:view];
    if ( bg ) {
        UIImageView *backgroundView = [[UIImageView alloc] initWithImage:bg];
        [view insertSubview:backgroundView atIndex:0];
        
    }
}


+ (UIImage *) GetBackground:(UIView *) view
{
    UIImage * img = [UIImage imageNamed:@"LaunchImage"];
    if ( !img )
        return nil;
    
    CGSize size = CGSizeMake ( view.bounds.size.width, view.bounds.size.height );
    
    UIGraphicsBeginImageContext ( size );
    
    [img drawInRect:CGRectMake ( 0, 0, size.width, size.height ) ];
    
    UIImage * newImage = UIGraphicsGetImageFromCurrentImageContext ();
    if ( !newImage )
        return nil;
    
    UIGraphicsEndImageContext ();
    
    return newImage;
}


-(void) OnData:(ObserverDataContext *) context
{
    
}


-(void) OnMessage:(ObserverMessageContext *) context
{
}


-(void) OnMessageExt:(ObserverMessageContext *) context
{
}


-(void) OnStatus:(environs::Status_t) status
{
}


-(void) OnStatusMessage:(const char *) message
{
}


-(void) OnPortalRequestOrProvided:(id) portalInstance
{
}


- (void) OnPortalChanged:(id) sender Notify:(environs::Notify::Portal_t) notification
{
    CVerb ( "OnPortalChanged" );
    
}


-(void) OnNotify:(ObserverNotifyContext *)context
{
}


-(void) OnNotifyExt:(ObserverNotifyContext *)context
{
}



@end
