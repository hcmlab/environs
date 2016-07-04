/**
 * LogView
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

#import "LogView.h"
#import "AppDelegate.h"
#import "ChatAppView.h"

#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME  "ChatApp"

LogView             *   logView = nil;

extern ChatAppView  *   chatAppView;
extern AppDelegate  *   appDelegate;


@interface LogView ()
{
}

@property (weak) IBOutlet NSTextField *     EventLogText;

@end


@implementation LogView


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    logView = self;
    
    NSImage * bg = [NSImage imageNamed:@"hcmbg.png"];
    if ( bg ) {
        [self.view setWantsLayer:YES];
        self.view.layer.contents = (id)bg;
    }
}


- (void) viewWillAppear
{
    if ( appDelegate != nil )
        [appDelegate UpdateLogView];
}


- (void) viewWillDisappear
{
    logView = nil;
}


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateStatusMessage: (NSString *) message
{
    dispatch_async(dispatch_get_main_queue(), ^{
        
        @autoreleasepool {
            [_EventLogText setStringValue:message];
            [_EventLogText sizeToFit];
        }
    });
}



@end
