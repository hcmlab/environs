/**
 * FullscreenController.mm
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

#import "FullscreenController.h"
#import "MediaBrowser.h"
#import "DeviceCell.h"

#import "Environs.iosx.h"
#include "Environs.native.h"


#define	CLASS_NAME 	"FullscreenController . ."

FullscreenController    *   instance = nil;
PortalInstance          *   portal = nil;

extern id                   appCurrentView;

@interface FullscreenController ()

@end

@implementation FullscreenController

@synthesize buttonLeft;
@synthesize buttonRight;
@synthesize buttonTop;
@synthesize buttonBottom;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerb ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

+ (void) SetPortalInstance:(id)portalSource
{
    portal = (PortalInstance *) portalSource;
}

- (void) dealloc
{
    CVerb ( "dealloc" );
    
    if ( portal != nil ) {
        [portal Stop];
    }
}


- (void)viewDidLoad
{
    CVerb ( "viewDidLoad" );
    
    instance = self;
    
    [super viewDidLoad];
}


- (void) didReceiveMemoryWarning
{
    CVerb ( "didReceiveMemoryWarning" );
    
    [super didReceiveMemoryWarning];
}


-(void) viewDidAppear: (BOOL)animated
{
    CVerb ( "viewDidAppear" );
    
    appCurrentView = self;
    
    [super viewDidAppear:animated];
    [self becomeFirstResponder];
    
    if ( portal == nil )
        return;
    
    [portal SetRenderSurface:[self view] Width:0 Height:0];
    [portal Start];
}


- (void) close
{
    CVerb ( "close" );
    
    
    dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
    dispatch_async ( queue, ^(void) {
        
        if ( portal != nil ) {
            [portal ReleaseRenderSurface];
            [portal Stop];
        }
        
        dispatch_sync ( dispatch_get_main_queue ( ), ^ {
            [self dismissViewControllerAnimated:YES completion:nil];
        } );
    });
}


- (void) viewWillDisappear: (BOOL)animated
{
    CVerb ( "viewWillDisappear" );
    
    [self resignFirstResponder];
    [super viewWillDisappear:animated];
}


-(BOOL) canBecomeFirstResponder {
    CVerb ( "canBecomeFirstResponder" );
    
    return NO;
}


- (void) motionEnded: (UIEventSubtype)motion withEvent:(UIEvent *)event
{
    CVerb ( "motionEnded" );
    
    if (motion == UIEventSubtypeMotionShake)
    {
        CLog ( "motionEnded: detected shake" );

        [self close];
    }
}


- (void) prepareForSegue: (UIStoryboardSegue *)segue sender:(id)sender
{
    CVerb ( "prepareForSegue" );
    
}


+ (FullscreenController *) getInstance
{
    return instance;
}


+ (void) resetInstance
{
    instance = 0;
}


void updatePortalLocation ( int dX, int dY )
{
    CVerb ( "updatePortalLocation" );
    
    if ( portal == nil )
        return;
    
    portal.info->SetLocation ( portal.info->base.centerX + dX, portal.info->base.centerY + dY );

}


- (IBAction)buttonRight:(id)sender
{
    CVerb ( "buttonRight" );
    
    updatePortalLocation ( 10, 0 );
}


- (IBAction)buttonLeft:(id)sender
{
    CVerb ( "buttonLeft" );
    
    updatePortalLocation ( -10, 0 );
}


- (IBAction)buttonTop:(id)sender
{
    CVerb ( "buttonTop" );
    
    updatePortalLocation ( 0, -10 );
}


- (IBAction)buttonBottom:(id)sender
{
    CVerb ( "buttonBottom" );
    
    updatePortalLocation ( 0, 10 );
}


@end
