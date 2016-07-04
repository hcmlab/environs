/**
 * ChatUser
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

#define DISABLE_ENVIRONS_OBJC_API
#include "Environs.h"

using namespace environs;


@interface ChatUser : NSObject
{
@public
    sp ( DeviceInstance ) device;
    
    NSInteger           row;
    long                enabled;

    int                 initState;
}

extern bool    showDebugUserTest;

@property (readonly) vsp ( environs::MessageInstance ) messages;

@property (readonly) NSString * userName;
@property (readonly) NSImage * userPic;
@property (readonly) NSString * lastMessage;
@property (readonly) NSString * lastStatus;

+ (ChatUser *) initWithDevice : (const sp ( environs::DeviceInstance ) &) device;

- (bool) Init;
- (void) Release;
- (void) RequestProfile : (bool) enforce;

- (bool) NeedsUpdate;

- (void) UpdateProfileText;
- (NSString *) copyOfProfileText;
- (NSString *) ProfileText;

- (NSImage *) GetProfileImage;

+ (NSString *) ToBase64:(NSData *) rawData;
+ (NSData *) FromBase64:(NSString *) base64;

@end

