/**
 * Tabs.mm
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
#include "Environs.native.h"

#define CLASS_NAME  "Tabs . . . . . . . . . ."

#import "Tabs.h"

Tabs * TabBarInstance = 0;

@interface Tabs ()

@end


@implementation Tabs

- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerbN ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    TabBarInstance = self;
}


- (void) didReceiveMemoryWarning
{
    CVerbN ( "didReceiveMemoryWarning" );
    
    [super didReceiveMemoryWarning];
}




@end
