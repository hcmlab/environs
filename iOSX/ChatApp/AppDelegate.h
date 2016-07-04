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
#import "Environs.h"


@interface AppDelegate : UIResponder <UIApplicationDelegate, EnvironsObserver, EnvironsMessageObserver, EnvironsDataObserver, PortalObserver>
{
}

@property (strong, nonatomic) UIWindow * window;

extern AppDelegate      *   appDelegate;

#ifdef CHATAPP1
@property (readonly, nonatomic) Environs *	env;

extern Environs         *   env;
#else

@property (readonly, nonatomic) Environs *	env;

extern Environs         *   env;
#endif

+ (void) UpdateViewBG:(UIView *) view;
+ (void) UpdateMessageList:(id) device;

@end
