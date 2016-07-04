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
 *
 * --------------------------------------------------------------------
 */

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "AppDelegate.h"
#import "SettingsTab.h"
#import "MessagesList.h"
#import "ChatAppView.h"
#import "ChatUser.h"
#import "ChatUserWatch.h"
#import "Tabs.h"

#include "Environs.native.h"
#import "Environs.iOSX.h"

#define CLASS_NAME  "AppDelegate"

AppDelegate             *   appDelegate = nil;
Environs                *   env         = nil;

extern MessagesList     *   messageList;
extern ChatAppView      *   chatAppView;

@interface AppDelegate ()
{
    DeviceList  *   deviceList;
    int             devicesOnWatch;
}
@end


@implementation AppDelegate


- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    CVerbN ( "didFinishLaunchingWithOptions" );
    
    appDelegate = self;
    
    devicesOnWatch = -1;
    
    env = [ Environs CreateInstance:"ChatApp" Area:"Environs" ];
    if ( !env )
        return NO;        
    
    [env AddObserver:self];
    [env AddObserverForMessages:self];
    [env AddObserverForData:self];

    [env SetMediatorFilterLevel: environs::MediatorFilter::None];
    
    [env SetUserName:"t@t.t"];
    [env SetUseAuthentication:true];
    
    [env Start];
    
    return YES;
}

							
- (void) applicationWillResignActive:(UIApplication *)application
{
    CVerbN ( "applicationWillResignActive" );
}


- (void) applicationDidEnterBackground:(UIApplication *)application
{
    CVerbN ( "applicationDidEnterBackground" );
    
    if ( !env )
        return;
    
    [env Stop];
}


- (void) applicationWillEnterForeground:(UIApplication *)application
{
    CVerbN ( "applicationWillEnterForeground" );
    
    if ( !env )
        return;
    
    [env Start];
    
    if (chatAppView)
        [chatAppView InitDeviceList];
}


- (void) applicationDidBecomeActive:(UIApplication *)application
{
    CVerbN ( "applicationDidBecomeActive" );
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    CVerbN ( "applicationWillTerminate" );
    
    if ( !env )
        return;
    
    [env Stop];
    [env Dispose];
}


- (void) application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
    CVerbN ( "UpdateViewBG" );
    
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
    CVerbN ( "UpdateViewBG" );
    
    UIImage * bg = [self GetBackground:view];
    if ( bg ) {
        UIImageView *backgroundView = [[UIImageView alloc] initWithImage:bg];
        [view insertSubview:backgroundView atIndex:0];
        
    }
}


+ (UIImage *) GetBackground:(UIView *) view
{
    CVerbN ( "GetBackground" );
    
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


+ (void) UpdateMessageList:(id) device
{
    CVerbN ( "UpdateMessageList" );
    
    if ( messageList == nil )
        return;
    [messageList ReloadMessagesList:device];
}


-(void) OnData:(ObserverDataContext *) context
{
    CVerbN ( "OnData" );
}


-(void) OnMessage:(ObserverMessageContext *) context
{
    CVerbN ( "OnMessage" );
}


-(void) OnMessageExt:(ObserverMessageContext *) context
{
    CVerbN ( "OnMessageExt" );
}


-(void) OnStatus:(environs::Status_t) status
{
    CVerbN ( "OnStatus" );
}


-(void) OnStatusMessage:(const char *) message
{
    CVerbN ( "OnStatusMessage" );
}


-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    CVerbN ( "OnPortalRequestOrProvided" );
}


- (void) OnPortalChanged:(id) sender Notify:(environs::Notify::Portal_t) notification
{
    CVerbN ( "OnPortalChanged" );
    
}


-(void) OnNotify:(ObserverNotifyContext *)context
{
    CVerbN ( "OnNotify" );
}


-(void) OnNotifyExt:(ObserverNotifyContext *)context
{
    CVerbN ( "OnNotifyExt" );
}



- (void) application:(UIApplication *)application handleWatchKitExtensionRequest:(NSDictionary *)userInfo reply:(void(^)(NSDictionary *replyInfo))reply
{
    CVerbN ( "handleWatchKitExtensionRequest" );
    
    [ChatUserWatch handleWatchKitExtensionRequest:userInfo reply:reply];
}

@end
