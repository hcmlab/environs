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
#include "Environs.h"

#ifdef CHATAPP1
#   define APP_NAME "ChatApp"
#else
#   define APP_NAME "ChatApp2"
#endif

@interface AppDelegate : NSObject <NSApplicationDelegate, EnvironsObserver, EnvironsMessageObserver>
{
    @public
}

@property (readonly) Environs *	env;

- (void) UpdateUI:(environs::Status_t) status;
- (void) UpdateLogView;
- (void) DisposeViews;

extern AppDelegate  *   appDelegate;


@end

