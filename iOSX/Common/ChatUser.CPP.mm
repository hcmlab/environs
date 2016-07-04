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

#import "ChatUser.CPP.h"
#import "ChatAppView.h"
#import "AppDelegate.h"

#include "Environs.Native.h"

using namespace std;
using namespace environs;

ChatUser            *   chatUser;

#define CLASS_NAME  "ChatUser"

#ifndef NDEBUG
//#   define DEBUG_TRACK_CHATUSER
#endif

extern NSString     *   loginUserName;

extern NSString     *   statusMessage;
extern NSString     *   userImageBase64;
extern NSString     *   defaultUserImage;

static NSString     *   defaultProfileText = @"Loading ...";

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
    
    NSLock   * messagesLock;
    
    NSString * lastStatus;
    NSString * lastMessage;
    NSLock   * profileTextLock;
    NSString * profileText;
    NSImage  * profilePic;
    
    vsp ( MessageInstance ) messages;
    
    NSString *  userName;

    bool userReady;
    int userReadyToSubmits;    
    
    int                 availableDetails;
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


- (instancetype)init
{    
#ifdef DEBUG_TRACK_CHATUSER
    long alives =
    __sync_add_and_fetch ( &debugChatObjCount, 1 );
    
    CLogArgN ( "Construct: Alive [ %i ]", (int) alives );
    
#endif
    
    self = [super init];
    if (self) {
        chatUser        = self;
        device          = nil;
        lastPingSent    = 0;
        updatesSent     = 0;
        lastUpdateSent  = 0;

        initState       = 0;
        pingsSent       = 0;
        
        availableDetails    = 0;
        lastStatusRequested = 0;
        profilePicRequested = 0;
        userNameRequested   = 0;
        
        userName        = @"Unknown";
        lastStatus      = @"No Status";
        profilePic      = nil;
        lastMessage     = nil;
        profileText     = defaultProfileText;
        
        enabled         = 1;
        userReady       = false;
        userReadyToSubmits = 3;
        
        messages.clear();
        messagesLock    = [[NSLock alloc] init];
        profileTextLock = [[NSLock alloc] init];
        
        if ( !messagesLock || !profileTextLock )
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
    
    if ( __sync_val_compare_and_swap ( &enabled, 1, 0 ) != 1 )
        return;
    
    @autoreleasepool {
        if ( device ) {
            device->RemoveObserver ( appDelegate->observer.get () );
            device->RemoveObserverForMessages ( appDelegate->observer.get () );

#ifndef CHATAPP1
            if ( device->appContext1 ) {
                ChatUser * chat = (__bridge_transfer ChatUser *) device->appContext1;
                device->appContext1 = nil;
                if ( chat )
                    chat = nil;
            }
#endif
            device = nil;
        }
        
        profilePic  = nil;
        userName    = nil;
        lastStatus  = nil;
        lastMessage = nil;
        
        [profileTextLock lock];
        
        profileText = nil;
        
        [profileTextLock unlock];
        
        messages.clear ();
        messagesLock = nil;
    }
}


- (bool) IsChatCommand:(NSString *) msg
{
    if ( !msg || [msg length] < 5 )
        return false;
    
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


+ (ChatUser *) initWithDevice : ( const sp ( DeviceInstance ) & ) userDevice
{
    if ( !userDevice )
        return nil;

    if ( !userDevice->appContext0 )
    {
        userDevice->appContext0 = 1;

        // Create a ChatUser object and attach it as the appContext to the DeviceInstance
        ChatUser * chatUser = [[ChatUser alloc] init];
        if ( !chatUser )
            return nil;

        // Let's listen to device changes
        // If we miss the disposal notif before the thread starts, then memory leak may happen
        userDevice->AddObserver ( appDelegate->observer.get () );

        chatUser->device = userDevice;

#ifndef CHATAPP1
        userDevice->appContext1 = (__bridge_retained void *) self;
#endif
        
        return chatUser;
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
            sp ( DeviceInstance ) userDevice = device;
            if ( !userDevice )
                return true;

            bool changed = false;

            [messagesLock lock];

            // Let's listen to messages from this device
            userDevice->AddObserverForMessages ( appDelegate->observer.get () );

            // Get or Request Username
            // Get or Request profile image
            // All chat internal messages start with $ca$type$data

            sp ( MessageList ) msgs = userDevice->GetMessagesInStorage ();
            if ( msgs && msgs->size () > 0 )
            {
                @try
                {
                    for ( int i=(int)msgs->size ()-1; i>=0; i-- )
                    {
                        sp ( MessageInstance ) msgInst = msgs->at(i);
                        if ( !msgInst )
                            continue;

                        if ( msgInst->text() == 0 )
                            continue;

                        @autoreleasepool {
                            NSString * msg = [[NSString alloc ] initWithUTF8String:msgInst->text () ];

                            if ( ![self IsChatCommand:msg] ) {
                                if ( (lastMessage == nil || [lastMessage isEqualToString:@""] ) && !msgInst->sent() ) {
                                    lastMessage = msg;
                                    changed = true;
                                }

                                messages.insert ( messages.begin(), msgInst );
                                continue;
                            }
                            if ( msgInst->sent() )
                                continue;

                            if ( [self HandleChatCommand:msg incoming:false] )
                                changed = true;
                        }
                    }
                }
                @catch ( NSException * ex ) {
                    NSLog ( @"%@", ex.reason );
                }
            }
            
            userDevice->DisposeStorageCache ();
            
            [messagesLock unlock];

            if ( changed ) {
                [self UpdateProfileText];

                ChatAppView *   appView = chatAppView;
                if ( appView ) {
#ifdef CHATAPP1
                    [appView UpdateRow:device->appContext0];
#else
                    [appView UpdateList];
#endif
                    [ChatAppView UpdateMessageList:self];
                }
            }

            if ( userDevice->sourceType () == environs::DeviceSourceType::Broadcast )
            {
                /// Ask user whether he/she is ready for commands
                userDevice->SendMessage ( "$ca$8" );

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
            sp ( DeviceInstance ) userDevice = device;
            if ( !userDevice || userReady || !enabled || userDevice->disposed () || userDevice->isMessageObserverReady () )
                return true;

            if ( userDevice->sourceType () == environs::DeviceSourceType::Broadcast )
            {
                @autoreleasepool
                {
                    /// Ask user whether he/she is ready for commands
                    userDevice->SendMessage ( "$ca$8" );

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
                [chatApp UpdateRow: (int)row];
            return true;
        }

        CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();

        if ( ( ( now - lastUpdateSent ) * 1000 ) < 10000 )
            return false;
        lastUpdateSent = now;

        @autoreleasepool
        {
            sp ( DeviceInstance ) userDevice = device;
            if ( !userDevice )
                return true;
            
            if ( !enabled || userDevice->disposed () )
                return true;
            
            [self RequestProfile:true];
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


- (void) RequestProfile : (bool) enforce
{
    if ( availableDetails >= 7 )
        return;
    
    //[messagesLock lock];
    
    sp ( DeviceInstance ) localDevice = device;
    if ( !localDevice )
        return;
    
    bool observerReady = localDevice->isMessageObserverReady ();
    
    bool ready = (enforce || userReady || observerReady || localDevice->sourceType () == environs::DeviceSourceType::Broadcast);
    
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    
    
    @autoreleasepool {
        if ( (enforce || userNameRequested == 0 || ( (now - userNameRequested) * 1000 > 1000 ) )  && [self IsUserNameEmpty] ) {
            // Username not found. Let's request that.
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting username" );
                device->SendMessage ( "$ca$0" );
                
                if ( observerReady )
                    userNameRequested = now;
            }
        }
        
        if ( ( enforce || lastStatusRequested == 0 || ( (now - lastStatusRequested) * 1000 > 1000 )) && [self IsStatusEmpty] ) {
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting status" );
                // status message not found. Let's request that.
                device->SendMessage ( "$ca$4" );
                
                if ( observerReady )
                    lastStatusRequested = now;
            }
        }
        
        if ( profilePic == nil && ( enforce || profilePicRequested == 0 || ( (now - profilePicRequested) * 1000 > 1000 ) ) ) {
            if ( ready ) {
                CVerbVerbN ("RequestProfile: Requesting profilePic" );
                device->SendMessage ( "$ca$2" );
                
                if ( observerReady )
                    profilePicRequested = now;
            }
        }
    }
    
    //[messagesLock unlock];
}


- (bool) NeedsUpdate
{
    return (availableDetails < 3);
}


- (bool) HandleChatCommand:(NSString *) msg incoming:(bool)processIncoming
{
    @autoreleasepool
    {
        bool changed = false;
        
        char type = [msg characterAtIndex:4];
        if ( type == '1' ) { // Update the username if required
            CVerbArgN ("HandleChatCommand: incoming username [%s]", [msg UTF8String]);
            if ( processIncoming || [self IsUserNameEmpty] )
            {
                @autoreleasepool {
                    userName = [msg substringFromIndex:6];
                }
                changed = true;
                availableDetails |= 0x1;
            }
        }
        else if ( type == '3' ) { // Update the profile picture if required
            CVerbN ("HandleChatCommand: incoming picture" );
            if ( processIncoming || profilePic == nil )
            {
                @autoreleasepool {
                    NSString * profilePicString = [msg substringFromIndex:6];
                    
                    // Decode the base64 sting to a byte array
                    NSData * profilePicData = [ChatUser FromBase64:profilePicString];
                    if ( profilePicData )
                    {
                        @autoreleasepool {
                            // Let's create an image from our byte array
                            profilePic = [[NSImage alloc] initWithData:profilePicData];
                        }
                        changed = true;
                        availableDetails |= 0x4;
                    }
                }
            }
        }
        else if ( type == '5' ) { // Update the status message if required
            CVerbArgN ("HandleChatCommand: incoming status [%s]", [msg UTF8String]);
            if ( processIncoming || [self IsStatusEmpty] )
            {
                @autoreleasepool {
                    lastStatus = [msg substringFromIndex:6];
                }
                availableDetails |= 0x2;
                changed = true;
            }
        }
        
        if ( processIncoming ) { // If this is an incoming request, then reply accordingly
            if ( type == '0' ) { // Resonse with our username
                CVerbArgN ("HandleChatCommand: sending username [%s]", [msg UTF8String]);
                
                @autoreleasepool {
                    NSString * msgtext = [[NSString alloc ] initWithFormat:@"$ca$1$%@", loginUserName];
                
                    device->SendMessage ( [msgtext UTF8String] );
                }
            }
            else if ( type == '2' ) { // Resonse with a base64 encoded string of our profile picture
                CVerbN ("HandleChatCommand: sending picture");
                
                @autoreleasepool {
                    NSString * msgtext = [[NSString alloc ] initWithFormat:@"$ca$3$%@", userImageBase64];
                    
                    device->SendMessage ( [msgtext UTF8String] );
                }
            }
            else if ( type == '4' ) { // Resonse with our status message
                CVerbArgN ("HandleChatCommand: sending status [%s]", [msg UTF8String]);
                
                @autoreleasepool {
                    NSString * msgtext = [[NSString alloc ] initWithFormat:@"$ca$5$%@", statusMessage];
                
                    device->SendMessage ( [msgtext UTF8String] );
                }
            }
            else if (type == '8') // Send ready
            {
                if ( !userReady || userReadyToSubmits > 0 ) {
                    userReady = true;
                    userReadyToSubmits--;
                    
                    device->SendMessage ( "$ca$8$" );
                }
            }
            
            [self RequestProfile:false];
        }
        return changed;
    }
}


- (void) UpdateProfileText
{
	bool connected = false;

	sp ( DeviceInstance ) localDevice = device;
    
	if ( localDevice != nil && localDevice->isConnected () )
        connected = true;
    
    [profileTextLock lock];
    
    @autoreleasepool {
        profileText = [[NSString alloc ] initWithFormat:@"%@%@ (%@)\n%@", connected ? @"* " : @"", userName, lastStatus != nil ? lastStatus : @"", lastMessage != nil ? lastMessage : @"" ];
    }
        
    [profileTextLock unlock];
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


- (NSString *) ProfileText
{
    return profileText;
}


- (NSImage *) GetProfileImage
{
    return profilePic;
}



/**
 * OnMessage is called whenever a text message has been received from a device.
 *
 * @param messageInst   The corresponding message object of type MessageInstance
 */
void Observer::OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t changedFlags )
{
    CVerbArgN ( "OnMessage: %s", msg->shortText () );
    
    sp ( DeviceInstance ) device = msg->device();
    if ( !device )
        return;
    
    ChatUser * chat = (__bridge ChatUser *) device->appContext1;
    if ( !chat )
        return;
    
    [chat->messagesLock lock];
    
    @autoreleasepool
    {
        NSString * text = [[NSString alloc ] initWithUTF8String:msg->text() ];
        
        if ( text != nil )
        {
            if ( [chat IsChatCommand:text  ] ) {
                if ( !msg->sent() && [chat HandleChatCommand:text incoming:true] )
                {
                    [chat UpdateProfileText];
                    [chat->messagesLock unlock];
                    
                    ChatAppView *   appView = chatAppView;
                    if ( appView )
#ifdef CHATAPP1
                        [appView UpdateRow:device->appContext0];
#else
                    [appView UpdateList];
#endif
                    return;
                }
            }
            else {
                chat->messages.push_back(msg);
                
                [chat->messagesLock unlock];
                
                if ( !msg->sent() ) {
                    chat->lastMessage = text;
                    
                    [chat UpdateProfileText];
                    
                    ChatAppView *   appView = chatAppView;
                    if ( appView )
#ifdef CHATAPP1
                        [appView UpdateRow:device->appContext0];
#else
                    [appView UpdateList];
#endif
                }
                
                [ChatAppView UpdateMessageList:chat];
                return;
            }
        }
    }
    
    [chat->messagesLock unlock];
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
