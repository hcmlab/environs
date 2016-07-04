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

#import "MediaBrowser.h"
#import "MainTab.h"
#import "SettingsTab.h"
#import "DeviceCell.h"
#import "DevicesListView.h"
#import "Tabs.h"

#include "Environs.native.h"
#import "Environs.iosx.h"

#define CLASS_NAME  "MediaBrowser . . . . . ."

MainTab             * mainTab       = 0;
static SettingsTab  * settingsTab   = 0;
extern DeviceCell   * deviceCellInstance;

extern Tabs         * TabBarInstance;

id                    appCurrentView = nil;

MediaBrowser        * app           = nil;
Environs            * env           = 0;


@interface MediaBrowser ()
{
    UIViewController * topViewController;
    
}
@end


@implementation MediaBrowser


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    app = self;
    
    env = [ Environs CreateInstance:"MediaBrowser" Area:"Environs" ];
    if ( !env )
        return false;
    
    [env AddObserver:self];
    [env AddObserverForMessages:self];
    [env AddObserverForData:self];
    
    //[env SetMediatorFilterLevel: environs::MediatorFilter::None];
    [env SetMediatorFilterLevel: environs::MediatorFilter::AreaAndApp];

    //[env SetUseAuthentication:true];
    
    [env SetUseTouchRecognizer:"libEnv-RecGestureBezelTouch.dylib" Status:true];
    [env SetUseTouchRecognizer:"libEnv-RecGestureThreeTouch.dylib" Status:true];

    application.applicationSupportsShakeToEdit = YES;
    
    return YES;
}

							
- (void)applicationWillResignActive:(UIApplication *)application
{
}


- (void)applicationDidEnterBackground:(UIApplication *)application
{
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    appCurrentView = self;
}


- (void)applicationDidBecomeActive:(UIApplication *)application
{
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    if ( env ) {
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


- (void) setTopViewController:(UIViewController *) viewController
{
    topViewController = viewController;
}

- (UIViewController *) getTopViewController
{
    return topViewController;
}


+ (MediaBrowser *) getInstance
{
    return self;
}


+ (void) setMainTab:(id) inst
{
    mainTab = (MainTab *)inst;
}


+ (void) setSettingsTab:(id) inst
{
    settingsTab = (SettingsTab *)inst;
}


+ (void) EnableSettings
{
    if ( settingsTab == 0 )
        return;
    [settingsTab EnableSettings];
}


+ (void) DisableSettings
{
    if ( settingsTab == 0 )
        return;
    [settingsTab DisableSettings];
}


int centerX = 300;
int centerY = 400;


-(void) OnData:(ObserverDataContext *) context
{
    
}


-(void) OnMessage:(ObserverMessageContext *) context
{
    if ( context->message != 0 ) {
        [mainTab AddStatusMessage:context->message];
        
        if ( !env )
            return;
        
        if ( context->message [0] == '$' )
            return;
            
        [env ShowMessage:[[NSString alloc] initWithFormat:@"%i", context->destID] Message:context->message Length:context->length];
    }
}


-(void) OnMessageExt:(ObserverMessageContext *) context
{
    if ( context->message != 0 ) {
        [mainTab AddStatusMessage:context->message];
        
        if ( !env )
            return;
        
        if ( context->message [0] == '$' )
            return;
        
        NSString * sender = [[NSString alloc] initWithFormat:@"%i (%s)", context->destID, context->appName ? context->appName : "" ];
        
        [env ShowMessage:sender Message:context->message Length:context->length];
    }
}


int lastStatus = 0;

-(void) OnStatus:(environs::Status_t) status
{
    if ( mainTab )
        [mainTab UpdateUI];

    if ( deviceCellInstance )
        [deviceCellInstance UpdateUI];

    if ( status >= environs::Status::Started )
    {
        if ( settingsTab)
            [settingsTab DisableSettings];
        
        if ( TabBarInstance ) {
            if ( lastStatus < status ) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [TabBarInstance setSelectedIndex: 1];
                });
            }
        }
        
    }
    else
    {
        if ( settingsTab)
            [settingsTab EnableSettings];
    }
    
    lastStatus = status;
}


-(void) OnStatusMessage:(const char *) message
{
    if ( message != 0 ) {
        [mainTab AddStatusMessage:message];
    }
}


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    if ( portalInstance == nil )
        return;
    
    PortalInstance * portal = (PortalInstance *)portalInstance;
    
    CLogArg ( "OnPortal: [ 0x%X ]", portal.portalID );
    
    [portal AddObserver:self];
    [portal Establish:true];
}


- (void) OnPortalChanged:(id) sender Notify:(int)Environs_NOTIFY_PORTAL_
{
    CVerbArg ( "OnPortalChanged: [ %s ]", environs::resolveName(Environs_NOTIFY_PORTAL_) );
    
    PortalInstance * portal = (PortalInstance *)sender;
    
    if ( portal != nil && portal.outgoing ) {
        return;
    }
    
    
    FullscreenController * controllerFullscreen = [FullscreenController getInstance];
    
    
    if ( portal == nil || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::Disposed || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamPaused
        || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamStopped )
    {
        if (controllerFullscreen)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [controllerFullscreen close];
            });
            [FullscreenController resetInstance];
        }
    }
    else if ( Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamIncoming || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::ImagesIncoming )
    {
        if (!mainTab)
            return;
        
        if (controllerFullscreen == 0)
        {
            [FullscreenController SetPortalInstance:portal];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [FullscreenController resetInstance];
                
                if ( [appCurrentView isKindOfClass:[MainTab class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"transit2Fullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[DeviceCell class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"deviceToFullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[DeviceListView class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"listToFullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[SettingsTab class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"settingsToFullscreen" sender: appCurrentView];
                }                
            });
        }
    }
}
#endif


-(void) OnNotify:(ObserverNotifyContext *)context
{
    CVerbArg ( "OnNotify:    nativeID [0x%X]\t[%s]\t[%i] [%i]", context->destID, environs::resolveName(context->notification), context->sourceIdent, context->contextPtr );
    
    unsigned int notifyType = context->notification & MSG_NOTIFY_CLASS;

    if ( notifyType == environs::Notify::Connection::type )
    {
        if ( deviceCellInstance )
        {
            if ( context->notification != environs::Notify::Connection::Progress )
                [deviceCellInstance UpdateUI];
        }
    }
    else if ( context->notification == environs::Notify::Network::Changed )
    {
        if (mainTab) {
            [mainTab UpdateUI];
        }
    }
}


-(void) OnNotifyExt:(ObserverNotifyContext *)context
{
    CVerbArg ( "onNotifyExt: deviceID [0x%X] \t%i] [%s] [%i]", context->destID, context->sourceIdent, environs::resolveName(context->notification), context->contextPtr );
}



@end
