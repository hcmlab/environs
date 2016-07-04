/**
 * ChatUserWatch
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

#import "ChatUserWatch.h"
#import "ChatUser.h"
#import "Environs.iosx.h"
#include "Environs.Native.h"
#include "Environs.iOSX.Log.h"


@implementation ChatUserWatch
{
    
    ChatUser * chatUser;
    NSString * userNick;
    NSString * userText;
    
    bool lastStatusRequested;
    bool profilePicRequested;
    bool userNameRequested;
}

@synthesize userNick;
@synthesize userText;

int watchRespSequence = -1;
DeviceList * deviceList;

- (instancetype)init
{
    self = [super init];
    if (self) {
        chatUser = nil;
        userText = nil;
        userNick = nil;
        
        lastStatusRequested = false;
        profilePicRequested = false;
        userNameRequested = false;
    }
    return self;
}


+ (void) handleWatchKitExtensionRequest:(NSDictionary *)userInfo reply:(void(^)(NSDictionary *replyInfo))reply
{
    CVerb ( "handleWatchKitExtensionRequest" );
    
    NSNumber * seq = [userInfo objectForKey:@"seq"];
    if ( seq ) {
        watchRespSequence = (int)[seq integerValue];
    }
    
    NSString * wCommand = [userInfo objectForKey:@"get"];
    
    if ( [wCommand isEqualToString:@"users"] ) {
        reply([self GetUserDict]);
    }
}


+ (void) SetDeviceList:(DeviceList *) list
{
    deviceList = list;
}


+ (NSMutableDictionary *) GetUserDict
{
    
    NSMutableDictionary * dict = [[NSMutableDictionary alloc] init];
    
    [dict setValue:[NSNumber numberWithInt:watchRespSequence] forKey:@"seq"];
    [dict setValue:[NSNumber numberWithInt:(int)deviceList.devices.count] forKey:@"userCount"];
    
    if ( deviceList.devices.count > 0 ) {
        int i = 0;
        NSMutableArray * reloads = [[NSMutableArray alloc] init];
        
        for ( DeviceInstance * device in deviceList.devices )
        {
            ChatUser * chatUser = device.appContext1;
            if ( !chatUser )
                continue;
            
            [reloads addObject:[NSNumber numberWithInt:i]];
            
            NSString * key = [[NSString alloc] initWithFormat:@"%inick", i];
            
            [dict setValue:chatUser.userName forKey:key];
            NSLog ( @"GetUserDict: add userName to [%i : %@]", i, chatUser.userName );
            
            key = [[NSString alloc] initWithFormat:@"%itext", i];
            
            NSString * text = @"";
            if ( chatUser.lastMessage )
                text = chatUser.lastMessage;
            else if ( chatUser.lastStatus )
                text = chatUser.lastStatus;
            
            [dict setValue:text forKey:key];
            NSLog ( @"GetUserDict: add text to [%i : %@]", i, text );
            
            if ( chatUser.userPic ) {
                NSLog ( @"GetUserDict: add picture to [%i]", i );
                key = [NSString stringWithFormat:@"%ipic", i];
                
                [dict setValue:chatUser.userPic forKey:key];
            }
            i++;
        }
        
        [dict setObject:reloads forKey:@"reloads"];
    }
    return dict;
}


+ (void) ReloadUsers
{
    if ( !deviceList )
        return;
    
    NSLog ( @"OnListChanged: sending reload data" );
    watchRespSequence++;
    
    CFNotificationCenterPostNotification ( CFNotificationCenterGetDarwinNotifyCenter (), (__bridge CFStringRef)@"environs.chatapp.watch.listchanged", NULL, NULL, YES );
    
}


+ (void) UpdateUser:(DeviceInstance *) device
{
    if ( !device )
        return;
    
    ChatUser * chatUser = (ChatUser *) device.appContext1;
    if ( !chatUser )
        return;
    
    ChatUserWatch * user = device.appContext2;
    if ( !user )
        return;
    
    bool chaged = false;
    
    if ( chatUser.userName && (!user.userNick || ![user.userNick isEqualToString:chatUser.userName]) ) {
        user.userNick = chatUser.userName;
        chaged = true;
    }
    
    NSString * text = @"";
    if ( chatUser.lastMessage )
        text = chatUser.lastMessage;
    else if ( chatUser.lastStatus )
        text = chatUser.lastStatus;
    user.userText = text;
    
    if ( !user.userText || ![user.userText isEqualToString:text] ) {
        user.userText = text;
        chaged = true;
    }
    
    if ( chaged )
        [self ReloadUsers];
}


+ (void) AddUserWatch:(DeviceInstance *) device
{
    if ( !device )
        return;
    
    ChatUser * chatUser = (ChatUser *) device.appContext1;
    if ( !chatUser )
        return;
    
    ChatUserWatch * user = [[ChatUserWatch alloc] init];
    
    user->chatUser = chatUser;
    device.appContext2 = user;
    
    user.userNick = chatUser.userName;
    
    NSString * text = @"";
    if ( chatUser.lastMessage )
        text = chatUser.lastMessage;
    else if ( chatUser.lastStatus )
        text = chatUser.lastStatus;
    user.userText = text;
    
    [self ReloadUsers];
}



@end
