/**
 * iOS Touch Listener (Gesture Listener)
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
#import <Foundation/Foundation.h>
#include "Environs.platforms.h"

#ifdef ENVIRONS_IOS
//******** iOS *************
#import <UIKit/UIKit.h>

#else
//******** OSX *************
#import <Cocoa/Cocoa.h>

#define UIView NSView
#define UIEvent NSEvent
#define UIGestureRecognizer NSGestureRecognizer

#endif

typedef void (^TouchesEventEntity)(NSSet * touches, UIEvent * event);


@interface TouchListener : UIGestureRecognizer
{
    TouchesEventEntity      touchesBeganCallback;
    unsigned int            deviceID;
}

- (void) updateView: (UIView *)view;

@property(copy) TouchesEventEntity touchesBeganCallback;

@end
