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
#include "Environs.Platforms.h"

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )

#   import <Foundation/Foundation.h>

#   ifdef ENVIRONS_IOS
        //******** iOS *************
#       import <UIKit/UIKit.h>
#   else
        //******** OSX *************
#       import <Cocoa/Cocoa.h>
#       define UIView NSView
#       define UIEvent NSEvent
#       define UIGestureRecognizer NSGestureRecognizer
#   endif

typedef void (^TouchesEventEntity)(NSSet * touches, UIEvent * event);


#ifdef __cplusplus

namespace environs
{
    class Instance;
}

#endif

@interface TouchListener : UIGestureRecognizer
{
    TouchesEventEntity      touchesBeganCallback;
    
@public
#ifdef __cplusplus
    environs::Instance            *   env;
#endif
    
    int                     hEnvirons;
}

- (void) UpdateView: (UIView *)view  Portal:(id)portal;

@property(copy) TouchesEventEntity touchesBeganCallback;

@end

#endif