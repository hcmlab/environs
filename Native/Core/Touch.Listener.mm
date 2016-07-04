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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Lib.Mobile.h"
#include "Human.Input.Decl.h"

#import "Touch.Listener.h"
#import "Environs.iOSX.h"
#import "Portal/Portal.Instance.iOSX.IAPI.h"
#import "Device/Device.Instance.iOSX.IAPI.h"

using namespace environs;

#ifdef ENVIRONS_IOS
#import <UIKit/UIGestureRecognizerSubclass.h>
#endif


#define	CLASS_NAME 	"TouchListener"


@interface TouchListener ()
{
    int             nativeID;
    ::PortalInstance * portal;
    
    UIView    *     touchView;
    int             withinFrameID;
    void        *   touchesCache;
    unsigned int    touchesCacheCount;
}
@end


@implementation TouchListener

@synthesize touchesBeganCallback;


-(id) init
{
    CVerbIDN ( "init" );
    
    if (self = [super init])
    {
        self.cancelsTouchesInView = NO;
        
        touchesCache        = 0;
        touchesCacheCount   = 0;
        touchView           = self.view;
        withinFrameID       = 0;
    }
    
    return self;
}


- (void) dealloc
{
    CVerbIDN ( "dealloc" );
    
    if ( touchesCache ) {
        free ( touchesCache );
        touchesCache = 0;
    }
}


- (NSString *)GetPhase: (UITouchPhase) phase
{
    switch(phase)
    {
        case UITouchPhaseBegan:
            return @"Begin";
        case UITouchPhaseMoved:
            return @"Moved";
        case UITouchPhaseStationary:
            return @"Stationary";
        case UITouchPhaseEnded:
            return @"Ended";
        case UITouchPhaseCancelled:
            return @"Cancelled";
    }
    return @"Unknown";
}


- (void) UpdateView: (UIView *)view  Portal:(::PortalInstance *)portal_
{
    if ( !view || !portal_ )
        return;
    
    touchView = view;
    
    [view addGestureRecognizer:self];
    
    portal = portal_;
    nativeID = ((::DeviceInstance *)portal.device).nativeID;
    
    if ( view.bounds.size.width > 0 )
    {
        if ( view.bounds.size.width != environs::native.display.width )
            environsTouchXFactor = environs::native.display.width / view.bounds.size.width;
        else
            environsTouchXFactor = 1;
    }
    
    if ( view.bounds.size.height > 0 )
    {
        if ( self.view.bounds.size.height != environs::native.display.height )
            environsTouchYFactor = environs::native.display.height / view.bounds.size.height;
        else
            environsTouchYFactor = 1;
    }
    
}


- (bool) InitTouchesCache:(unsigned int)count
{
    if ( count < touchesCacheCount )
        return true;
    
    if ( touchesCache ) {
        free ( touchesCache );
        touchesCache = 0;
    }
    
    touchesCacheCount = 0;
    
    unsigned int reqCount = count + 10;
    
    touchesCache = calloc ( 1, reqCount * sizeof(environs::lib::InputPackRaw) );
    if ( !touchesCache )
        return false;
    
    touchesCacheCount = reqCount;
    return true;
}


- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    CVerbVerbIDN ( "touchesBegan" );

    if (env->environsState < environs::Status::Started)
        return;

    if (!withinFrameID)
        environs::API::TouchDispatchN(hEnvirons, portal.portalID, 0, 0, 0);
    
    NSSet *allTouches = [event allTouches];
    if (!allTouches)
        return;

    int touchCount = (int)[allTouches count];
    
    CVerbVerbArgIDN ( "touchesBegan: We got [%i] touches.", touchCount );

    if ( touchCount > touchesCacheCount ) {
        if ( ![self InitTouchesCache:touchCount] ) {
            return;
        }
    }
    
    int i = 0;
    bool init = (withinFrameID == 0);
    
    environs::lib::InputPackRaw * touchPacks = (environs::lib::InputPackRaw *) touchesCache;
    
    for (UITouch * touch in allTouches)
    {
        //CGPoint loc = [touch locationInView:self.view];
        CGPoint loc = [touch   locationInView:touchView];

        NSInteger phase = [touch phase];
        if ( phase == UITouchPhaseStationary )
            continue;
        
        int touchID = (int)(NSInteger) touch;
        
        touchPacks [i].id       = touchID;
        touchPacks [i].state    = (phase == UITouchPhaseBegan ? INPUT_STATE_ADD : INPUT_STATE_CHANGE );
        touchPacks [i].type      = INPUT_TYPE_FINGER;
        touchPacks [i].x        = ( loc.x * environsTouchXFactor);
        touchPacks [i].y        = ( loc.y * environsTouchYFactor);
        if (environs::native.sdks >= 8)
            touchPacks [i].size = [touch majorRadius];
        else
            touchPacks [i].size = 1;

        if ( !withinFrameID )
            withinFrameID = touchID;
        
        CVerbVerbArgIDN ( "touchesBegan: touch [%i] at x[%i]  y[%i]  [%s]", touchID, touchPacks [i].x, touchPacks [i].y, [[self GetPhase: [touch phase]] UTF8String] );
        i++;
    }
    
    environs::API::TouchDispatchN(hEnvirons, portal.portalID, touchPacks, i, init);
}


- (void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    CVerbVerbIDN ( "touchesCancelled" );

    [self reset];

    if (env->environsState < environs::Status::Started) {
        return;
    }
    

    withinFrameID = 0;
    environs::API::TouchDispatchN(hEnvirons, portal.portalID, 0, 0, 0);

    [self touchesEnded:touches withEvent:event];
}


- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    CVerbVerbIDN ( "touchesMoved" );
    
    if (env->environsState < environs::Status::Started) {
        return;
    }
    
    NSSet *allTouches = [event allTouches];
    if (!allTouches)
        return;
    
    int touchCount = (int)[allTouches count];

    CVerbVerbArgIDN ( "touchesMoved: We got [%i] touches.", touchCount );
    
    if ( touchCount > touchesCacheCount ) {
        if ( ![self InitTouchesCache:touchCount] ) {
            return;
        }
    }
    
    int i = 0;

    environs::lib::InputPackRaw * touchPacks = (environs::lib::InputPackRaw *) touchesCache;

    for (UITouch * touch in allTouches)
    {
        UITouchPhase phase = [touch phase];
        if (phase == UITouchPhaseStationary)
            continue;
        
        CGPoint loc = [touch locationInView:touchView];

        int touchID = (int)(NSInteger) touch;

        touchPacks [i].id   = touchID;
        
        if (phase == UITouchPhaseBegan ) {
            CVerbVerbArgIDN ( "touchesMoved: touch [%i] added at x [%f]  y[%f] (changed to moved)", touchID, loc.x, loc.y );
            touchPacks [i].state = INPUT_STATE_CHANGE;
        }
        
        // Moved
        else if (phase == UITouchPhaseMoved) {
            CVerbVerbArgIDN ( "touchesMoved: touch [%i] moved to x[%f] y[%f]", touchID, loc.x, loc.y );
            touchPacks [i].state = INPUT_STATE_CHANGE;
        }
        
        // Ended
        else {
            CVerbVerbArgIDN ( "touchesMoved: touch [%i] cancel/ended at x[%f] y[%f] phase [%s]", touchID, loc.x, loc.y, [[self GetPhase: [touch phase]] UTF8String] );
            touchPacks [i].state = INPUT_STATE_DROP;
        }
        touchPacks [i].type     = INPUT_TYPE_FINGER;
        touchPacks [i].x        = ( loc.x * environsTouchXFactor);
        touchPacks [i].y        = ( loc.y * environsTouchYFactor);
        if (environs::native.sdks >= 8)
            touchPacks [i].size = [touch majorRadius];
        else
            touchPacks [i].size = 1;
        i++;
    }
    
    environs::API::TouchDispatchN ( hEnvirons, portal.portalID, touchPacks, i, false );
    
}


- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    CVerbVerbIDN ( "touchesEnded" );
    
    [self reset];
    
    if (env->environsState < environs::Status::Started) {
        return;
    }
    
    NSSet * allTouches = [event allTouches];
    if (!allTouches)
        return;
    
    int touchCount = (int)[allTouches count];
    
    CVerbVerbArgIDN ( "touchesEnded: We got [%i] touches.", touchCount );
    
    if ( touchCount > touchesCacheCount ) {
        if ( ![self InitTouchesCache:touchCount] ) {
            return;
        }
    }
    
    int i = 0;
    int actuallyEnded = 0;
    
    environs::lib::InputPackRaw * touchPacks = (environs::lib::InputPackRaw *) touchesCache;
    
    for (UITouch * touch in allTouches)
    {
        CGPoint loc = [touch locationInView:touchView];
        
        NSInteger phase = [touch phase];
        if ( phase == UITouchPhaseStationary )
            continue;
        
        int touchID = (int)(NSInteger) touch;
        
        touchPacks [i].id       = touchID;
        
        if ( phase == UITouchPhaseEnded ) {
            actuallyEnded++;
            touchPacks [i].state = INPUT_STATE_DROP;
        }
        else
            touchPacks [i].state = INPUT_STATE_CHANGE;
        
        /*if ( touchPacks [i].state == INPUT_STATE_DROP )
            CVerbVerbID ( "touchesEnded: We got a drop." );
        */
        
        touchPacks [i].type      = INPUT_TYPE_FINGER;
        touchPacks [i].x        = ( loc.x * environsTouchXFactor);
        touchPacks [i].y        = ( loc.y * environsTouchYFactor);
        if (environs::native.sdks >= 8)
            touchPacks [i].size = [touch majorRadius];
        else
            touchPacks [i].size = 1;
        
        CVerbVerbArgIDN ( "touchesEnded: touch [%i] at x[%i]  y[%i]  [%s]", touchID, touchPacks [i].x, touchPacks [i].y, [[self GetPhase: [touch phase]] UTF8String] );
        i++;
    }
    
    environs::API::TouchDispatchN ( hEnvirons, portal.portalID, touchPacks, i, false );
    
    if ( (touchCount - actuallyEnded) <= 0 ) {
        withinFrameID = 0;
        //environs::kernel->touchSource->Flush();
    }
}


- (void) reset
{
    CVerbVerbIDN ( "reset" );
    
    if (self.state == UIGestureRecognizerStatePossible) {
        [self setState:UIGestureRecognizerStateFailed];
    }
}


- (void) ignoreTouch:(UITouch *)touch forEvent:(UIEvent*)event
{
    CVerbVerbIDN ( "ignoreTouch" );
    
}


- (BOOL) canBePreventedByGestureRecognizer:(UIGestureRecognizer *)preventingGestureRecognizer
{
    CVerbIDN ( "canBePreventedByGestureRecognizer" );
    return false;
}


- (BOOL) canPreventGestureRecognizer:(UIGestureRecognizer *)preventedGestureRecognizer
{
    CVerbIDN ( "canPreventGestureRecognizer" );
    return false;
}

@end
