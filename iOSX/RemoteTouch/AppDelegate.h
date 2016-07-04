/**
 * AppDelegate
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

#import <Cocoa/Cocoa.h>
#include "Environs.h"


@interface AppDelegate : NSObject <NSApplicationDelegate, EnvironsObserver, EnvironsMessageObserver, EnvironsDataObserver>
{
    @public
}

extern AppDelegate  *   appDelegate;

@property (readonly) Environs *	env;

- (void) StartEnvirons;

@end

