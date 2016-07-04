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
#include "Environs.h"

#define USE_CHATUSER_CHANGE_FLAG
//#define USE_IMPROVED_UPDATER_THREAD

@interface ChatUser : NSObject <DeviceObserver, MessageObserver>
{
@public
    DeviceInstance *    device;
    NSInteger           row;
    long                enabled;

    int                 initState;

#ifdef USE_CHATUSER_CHANGE_FLAG
    bool                changeAvailable;
#endif
}

extern bool    showDebugUserTest;

@property (readonly) NSMutableArray * messages;
@property (readonly) NSString * userName;
@property (readonly) UIImage * userPic;
@property (readonly) NSString * lastMessage;
@property (readonly) NSString * lastStatus;

+ (ChatUser *) initWithDevice:(DeviceInstance *) device;

- (bool) Init;
- (void) Release;

- (bool) NeedsUpdate;

- (void) UpdateProfileText;
- (NSString *) copyOfProfileText;
- (NSString *) ProfileText;

- (__weak UIImage *) GetProfileImage;

- (void) RequestProfile: (bool)useLock enforce:(bool)enforce;

+ (NSString *) ToBase64:(NSData *) rawData;
+ (NSData *) FromBase64:(NSString *) base64;

@end

