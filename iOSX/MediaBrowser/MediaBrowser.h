/**
 * MediaBrowser.h
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
#import <UIKit/UIKit.h>
#import "Environs.iOSX.h"


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
@interface MediaBrowser : UIResponder <UIApplicationDelegate, EnvironsObserver, EnvironsMessageObserver, EnvironsDataObserver,PortalObserver>
{
}
#else
@interface MediaBrowser : UIResponder <UIApplicationDelegate, EnvironsObserver, EnvironsMessageObserver, EnvironsDataObserver>
{
}
#endif

extern MediaBrowser * app;

#ifdef ENVIRONS_IOSX_USE_CPP_API
extern Environs * env;
#endif

@property (strong, nonatomic) UIWindow * window;

+ (MediaBrowser *) getInstance;

- (void) setTopViewController:(UIViewController *) viewController;
- (UIViewController *) getTopViewController;

+ (void) setMainTab:(id) inst;
+ (void) setSettingsTab:(id) inst;

+ (void) EnableSettings;
+ (void) DisableSettings;

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance;
- (void) OnPortalChanged:(id) sender Notify:(int)Environs_NOTIFY_PORTAL_;
#endif

+ (void) UpdateViewBG:(UIView *) view;

@end
