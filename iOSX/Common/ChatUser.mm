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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "ChatUser.h"
#import "ChatAppView.h"
#import "AppDelegate.h"

#include "Environs.Native.h"

#ifndef NDEBUG
//#   define DEBUG_TRACK_CHATUSER
#endif

#define CLASS_NAME  "ChatUser . . . . . . . ."

extern NSString     *   loginUserName;

extern NSString     *   statusMessage;
extern NSString     *   userImageBase64;

static NSString  * const defaultProfileText = @"Loading ...";

bool    showDebugUserTest = false;

#ifdef DEBUG_TRACK_CHATUSER
long    debugChatObjCount = 0;

#endif

@interface ChatUser ()
{
    CFAbsoluteTime      lastPingSent;
    int                 pingsSent;
    CFAbsoluteTime      lastUpdateSent;
    int                 updatesSent;

    NSString *          lastStatus;
    NSString *          lastMessage;
    
    NSLock   *          profileTextLock;
    NSString *          profileText;
    
    UIImage  *          profilePic;
    
    NSLock   *          messagesLock;
    NSMutableArray *    messages;
    
    NSString *          userName;
    
    int                 availableDetails;
    bool                userReady;
    int                 userReadyToSubmits;
    
    CFAbsoluteTime    lastStatusRequested;
    CFAbsoluteTime    profilePicRequested;
    CFAbsoluteTime    userNameRequested;
}

@end


@implementation ChatUser


@synthesize messages;
@synthesize userName;
@synthesize userPic = profilePic;
@synthesize lastStatus;
@synthesize lastMessage;


- (instancetype) init
{
    CVerbN ( "init" );
    
#ifdef DEBUG_TRACK_CHATUSER
    long alives =
    __sync_add_and_fetch ( &debugChatObjCount, 1 );
    
    CLogArgN ( "Construct: Alive [ %i ]", (int) alives );
    
#endif
    
    self = [super init];
    if (self)
    {
        lastPingSent    = 0;
        updatesSent     = 0;
        lastUpdateSent  = 0;

        initState   = 0;
        pingsSent   = 0;

#ifdef USE_CHATUSER_CHANGE_FLAG
        changeAvailable = true;
#endif
        device      = nil;

        userName    = @"Unknown";
        lastStatus  = @"No Status";
        profilePic  = nil;
        lastMessage = nil;
        
        row         = 0;
        profileText = defaultProfileText;
        enabled     = 1;
        userReady   = false;
        userReadyToSubmits  = 3;
        availableDetails    = 0;
        lastStatusRequested = 0;
        profilePicRequested = 0;
        userNameRequested   = 0;
        
        messages            = [[NSMutableArray alloc] init];
        messagesLock        = [[NSLock alloc] init];
        profileTextLock     = [[NSLock alloc] init];
        
        if ( !messages || !messagesLock || !profileTextLock )
            return nil;
    }
    return self;
}


- (void) dealloc
{
    CVerbN ( "dealloc" );
    
#ifdef DEBUG_TRACK_CHATUSER
    __sync_sub_and_fetch ( c_ref debugChatObjCount, 1 );
#endif
    
    [self Release];
}


- (void) Release
{
    CVerbN ( "Release" );

    initState   = 10;

    if ( __sync_val_compare_and_swap ( &enabled, 1, 0 ) != 1 )
        return;
    
    @autoreleasepool
    {
        profilePic  = nil;
        userName    = nil;
        lastStatus  = nil;
        lastMessage = nil;
        
        [profileTextLock lock];
        
        profileText = nil;
        
        [profileTextLock unlock];
        
        NSMutableArray * msgs = messages;
        messages = nil;
        
        if ( msgs ) {
            [msgs removeAllObjects];
            msgs = nil;
        }

        DeviceInstance * userDevice = device;
        device = nil;

        if ( userDevice ) {
#ifndef CHATAPP1
            userDevice.appContext1 = nil;
#endif
            [userDevice RemoveObserverForMessages:self];
            [userDevice RemoveObserver:self];

            userDevice = nil;
        }
    }
}


- (bool) IsChatCommand:(NSString *) msg
{
    if ( [msg length] < 5 )
        return false;
    
    CVerbN ( "IsChatCommand: true" );
    
    return [msg hasPrefix:@"$ca$"];
}


- (bool) IsUserNameEmpty
{
    return ( userName == nil || userName.length == 0 || [userName isEqualToString:@"Unknown"] );
}


- (bool) IsStatusEmpty
{
    return ( lastStatus == nil || lastStatus.length == 0 || [lastStatus isEqualToString:@"No Status"] );
}


+ (ChatUser *) initWithDevice:(DeviceInstance *) userDevice
{
    if ( !userDevice )
        return nil;
    
    if ( !userDevice.appContext0 )
    {
        userDevice.appContext0 = 1;

        // Create a ChatUser object and attach it as the appContext to the DeviceInstance
        ChatUser * chatUser = [[ChatUser alloc] init];
        if ( chatUser )
        {
            chatUser->device = userDevice;
#ifndef CHATAPP1
            userDevice.appContext1 = chatUser;
#endif
            // Let's observe device changes, e.g. disposal, connections, etc..
            // If we miss the disposal notif before the thread starts, then memory leak may happen
            [userDevice AddObserver:chatUser];

            return chatUser;
        }
    }
    
    return nil;
}


- (bool) Init
{
    CVerbN ( "Init" );

    if ( initState == 0 )
    {
        @autoreleasepool
        {
            // Get or Request Username
            // Get or Request profile image
            // All chat internal messages start with $ca$type$data
            DeviceInstance * userDevice = device;
            if ( !userDevice )
                return true;

            bool changed = false;

            [messagesLock lock];

            // Let's listen to messages from this device
            [userDevice AddObserverForMessages:self];

#ifndef NDEBUG
            userDevice.async = environs::Call::Wait;

            [userDevice ClearMessages];
            [userDevice ClearStorage];

            userDevice.async = environs::Call::NoWait;
#endif
            NSMutableArray * msgs = [device GetMessagesInStorage];
            if ( msgs != nil )
            {
                @try
                {
                    for ( int i=(int)[msgs count]-1; i>=0; i-- )
                    {
                        MessageInstance * msgInst = (MessageInstance *)[msgs objectAtIndex:i];
                        if ( !msgInst )
                            continue;

                        @autoreleasepool
                        {
                            NSString * msg = msgInst.text;
                            if ( !msg )
                                continue;

                            if ( ![self IsChatCommand:msg] ) {
                                if ( (lastMessage == nil || [lastMessage isEqualToString:@""] ) && !msgInst.sent ) {
                                    lastMessage = msg;
                                    changed = true;
#ifdef USE_CHATUSER_CHANGE_FLAG
                                    changeAvailable = true;
#endif
                                }
                                [messages insertObject:msgInst  atIndex:0];
                                continue;
                            }
                            if ( msgInst.sent )
                                continue;

                            if ( [self HandleChatCommand:msg incoming:false useLock:false] ) {
                                changed = true;
#ifdef USE_CHATUSER_CHANGE_FLAG
                                changeAvailable = true;
#endif
                            }
                        }
                    }
                }
                @catch ( NSException * ex ) {
                    NSLog ( @"%@", ex.reason );
                }
            }

            [userDevice DisposeStorageCache];

            [messagesLock unlock];

            if ( changed ) {
                [self UpdateProfileText];

                ChatAppView * chatApp = chatAppView;
                if ( chatApp )
#ifdef CHATAPP1
                    [chatApp UpdateRow:row];
#else
                [chatApp UpdateList];
#endif

                [ChatAppView UpdateMessageList:self];
            }

            if ( userDevice.sourceType == environs::DeviceSourceType::Broadcast )
            {
                /// Ask user whether he/she is ready for commands
                [userDevice SendMessage : @"$ca$8" ];

                initState = 1;
            }
            else {
                initState = 2; lastUpdateSent = CFAbsoluteTimeGetCurrent();
                return false;
            }
        }
    }

    if ( initState == 1 )
    {
        CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();

        if ( ( now - lastPingSent ) * 1000 < 1000 )
            return false;
        lastPingSent = now;

        @autoreleasepool
        {
            DeviceInstance * userDevice = device;
            if ( !userDevice || userReady || !enabled || userDevice.disposed || userDevice.isMessageObserverReady )
                return true;

            if ( userDevice.sourceType == environs::DeviceSourceType::Broadcast )
            {
                @autoreleasepool
                {
                    /// Ask user whether he/she is ready for commands
                    [userDevice SendMessage : @"$ca$8" ];

                    pingsSent++;
                }

                if ( pingsSent > 20 ) {
                    initState = 2; lastUpdateSent = CFAbsoluteTimeGetCurrent();
                    return false;
                }
                else
                    return false;
            }
            else {
                initState = 2; lastUpdateSent = CFAbsoluteTimeGetCurrent();
                return false;
            }
        }
    }

    if ( initState == 2 )
    {
        if ( ![self NeedsUpdate] ) {
            ChatAppView * chatApp = chatAppView;
            if ( chatApp )
                [chatApp UpdateRow:row];
            return true;
        }

        CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();

        if ( ( ( now - lastUpdateSent ) * 1000 ) < 10000 )
            return false;
        lastUpdateSent = now;

        @autoreleasepool
        {
            DeviceInstance * userDevice = device;
            if ( !userDevice )
                return true;

            if ( !enabled || userDevice.disposed )
                return true;

            [self RequestProfile:false enforce:true];
            updatesSent++;

            if ( updatesSent > 500 )
                return true;
            else
                return false;
        }
        return false;
    }
    return true;
}


- (bool) NeedsUpdate
{
    return (availableDetails < 3);
}


- (void) RequestProfile: (bool)useLock enforce:(bool)enforce
{
    CVerbN ( "RequestProfile" );
    
    if ( availableDetails >= 7 )
        return;
    
    DeviceInstance * localDevice = device;
    if ( !localDevice )
        return;
    
    bool observerReady = localDevice.isMessageObserverReady;
    
    bool ready = (enforce || userReady || observerReady || localDevice.sourceType == environs::DeviceSourceType::Broadcast);
    
    if ( useLock ) [messagesLock lock];
    
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    
    @autoreleasepool
    {
        if ( (enforce || userNameRequested == 0 || ( (now - userNameRequested) * 1000 > 1000 )) && [self IsUserNameEmpty] ) {
            // Username not found. Let's request that.
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting username" );
                [device SendMessage:@"$ca$0"];
                
                if ( observerReady )
                    userNameRequested = now;
            }
        }
        
        if ( (enforce || lastStatusRequested == 0 || ( (now - lastStatusRequested) * 1000 > 1000 )) && [self IsStatusEmpty] ) {
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting status" );
                // status message not found. Let's request that.
                [device SendMessage:@"$ca$4"];
                
                if ( observerReady )
                    lastStatusRequested = now;
            }
        }
        
        if ( profilePic == nil && (enforce || profilePicRequested == 0 || ( (now - profilePicRequested) * 1000 > 1000 ) ) ) {
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting profilePic" );
                [device SendMessage:@"$ca$2"];
                
                if ( observerReady )
                    profilePicRequested = now;
            }
        }
    }
    
    if ( useLock ) [messagesLock unlock];
}


- (bool) HandleChatCommand:(NSString *) msg incoming:(bool)processIncoming useLock:(bool)useLock
{
    CVerbN ("HandleChatCommand" );
    
    @autoreleasepool
    {
        char type = [msg characterAtIndex:4];
        if ( type == '1' ) { // Update the username if required
            CVerbN ("HandleChatCommand: incoming username" );
            if ( processIncoming || [self IsUserNameEmpty] )
            {
                CVerbArgN ("HandleChatCommand: incoming username [%s]", [msg UTF8String]);
                
                @autoreleasepool {
                    userName = [msg substringFromIndex:6];
                }
                availableDetails |= 0x1;
                return true;
            }
        }
        else if ( type == '3' ) { // Update the profile picture if required
            CVerbArgN ("HandleChatCommand: incoming picture [%i]", processIncoming );
            if ( profilePic == nil  || processIncoming ) {
                CVerbN ("HandleChatCommand: incoming picture" );
                
                @autoreleasepool {
                    NSString * profilePicString = [msg substringFromIndex:6];
                    
                    // Decode the base64 sting to a byte array
                    NSData * profilePicData = [[NSData alloc] initWithBase64EncodedString:profilePicString options:0];
                    if ( profilePicData )
                    {
                        @autoreleasepool {
                            // Let's create an image from our byte array
                            profilePic = [[UIImage alloc] initWithData:profilePicData];
                        }
                        
                        CVerbN ("HandleChatCommand: incoming picture ok" );
                        availableDetails |= 0x4;
                        return true;
                    }
                }
            }
        }
        else if ( type == '5' ) { // Update the status message if required
            CVerbN ("HandleChatCommand: incoming status" );
            if ( processIncoming || [self IsStatusEmpty] )
            {
                CVerbArgN ("HandleChatCommand: incoming status [%s]", [msg UTF8String]);
                
                @autoreleasepool {
                    lastStatus = [msg substringFromIndex:6];
                }
                availableDetails |= 0x2;
                return true;
            }
        }
        
        if ( processIncoming ) { // If this is an incoming request, then reply accordingly
            if ( type == '0' ) { // Resonse with our username
                CVerbArgN ("HandleChatCommand: sending username [%s]", [msg UTF8String]);
                
                [device SendMessage:[[NSString alloc ] initWithFormat:@"$ca$1$%@", loginUserName]];
            }
            else if ( type == '2' ) { // Resonse with a base64 encoded string of our profile picture
                CVerbN ("HandleChatCommand: sending picture");
                
                [device SendMessage:[[NSString alloc ] initWithFormat:@"$ca$3$%@", userImageBase64]];
            }
            else if ( type == '4' ) { // Resonse with our status message
                CVerbArgN ("HandleChatCommand: sending status [%s]", [msg UTF8String]);
                
                [device SendMessage:[[NSString alloc ] initWithFormat:@"$ca$5$%@", statusMessage]];
            }
            else if (type == '8') // Send ready
            {
                if ( !userReady || userReadyToSubmits > 0 ) {
                    userReady = true;
                    userReadyToSubmits--;
                    
                    [device SendMessage:@"$ca$8"];
                    //NSLog ( @"HandleChatCommand: Enabled [%@]", [device ToString] );
                }
            }        
            [self RequestProfile:useLock enforce:false];
        }
    }
    return false;
}


- (void) UpdateProfileText
{
	bool connected = false;

	DeviceInstance * localDevice = device;
	if (localDevice != nil && localDevice.isConnected)
		connected = true;
		
    [profileTextLock lock];
    
    @autoreleasepool
    {
        if ( showDebugUserTest )
            profileText = [[NSString alloc ] initWithFormat:@"%@", [localDevice toString] ];
        else
            profileText = [[NSString alloc ] initWithFormat:@"%@%@ (%@)\n%@", connected ? @"* " : @"", userName, lastStatus != nil ? lastStatus : @"", lastMessage != nil ? lastMessage : @"" ];

#ifdef USE_CHATUSER_CHANGE_FLAG
        changeAvailable = true;
#endif
    }
    
    [profileTextLock unlock];
}


- (NSString *) ProfileText
{
    return profileText;
}


- (NSString *) copyOfProfileText
{
    NSString * copy = nil;
    
    [profileTextLock lock];
    
    if ( profileText )
        copy = [profileText copy];
    else
        copy = [defaultProfileText copy];
    
    [profileTextLock unlock];
    
    return copy;
}


- (UIImage *) GetProfileImage
{
    return profilePic;
}


- (void) OnDeviceChanged:(id) sender Flags:(environs::DeviceInfoFlag_t)  flags;
{
    CVerbVerbN ( "OnDeviceChanged" );
    
    if ( flags == environs::DeviceInfoFlag::Disposed ) {
        [self Release];
    }
    else if ( flags == environs::DeviceInfoFlag::Flags ) {
        [self RequestProfile:true enforce:false];
    }
    else if ( (flags & environs::DeviceInfoFlag::IsConnected) == environs::DeviceInfoFlag::IsConnected ) {
        [self UpdateProfileText];
        
        [self RequestProfile:false enforce:true];
        
        ChatAppView * chatApp = chatAppView;
        if ( chatApp )
            [chatApp UpdateRow:row];
    }
}


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
}
#endif


/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param sender        The device instance which sent the data.
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessage:(MessageInstance *) msg Flags:(environs::MessageInfoFlag_t)  MESSAGE_INFO_ATTR_changed
{
    CVerbArgN ( "OnMessage: %s", [[msg shortText] UTF8String] );
    
    [messagesLock lock];
    
    @autoreleasepool
    {
        NSString * text = msg.text;
        if ( text != nil )
        {
            CVerbN ( "OnMessage: text is ok."  );
            
            if ( [self IsChatCommand:text] ) {
                CVerbN ( "OnMessage: check 1."  );
                if ( !msg.sent && [self HandleChatCommand:text incoming:true useLock:false] ) {
                    
                    CVerbN ( "OnMessage: Updating profile"  );
                    [self UpdateProfileText];

                    [messagesLock unlock];
                    
                    ChatAppView * chatApp = chatAppView;
                    if ( chatApp )
                        [chatApp UpdateRow:row];
                    return;
                }
            }
            else {
                CVerbN ( "OnMessage: not a command"  );
                [messages addObject:msg];
                
                [messagesLock unlock];
                
                if ( !msg.sent ) {
                    lastMessage = text;
                    [self UpdateProfileText];
                    
                    ChatAppView * chatApp = chatAppView;
                    if ( chatApp )
                        [chatApp UpdateRow:row];
                }
                
                [ChatAppView UpdateMessageList:self];
                
                return;
            }
        }
        else {
            CVerbN ( "OnMessage: text is nil."  );
        }
    }
    
    [messagesLock unlock];
}


+ (NSString *) ToBase64:(NSData *) rawData
{
    return [rawData base64EncodedStringWithOptions:0];
}


+ (NSData *) FromBase64:(NSString *) base64
{
    return [[NSData alloc] initWithBase64EncodedString:base64 options:0];
}







@end
