/**
 * ChatAppView
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

#import "ChatAppView.h"
#import "LogView.h"
#import "LogWindow.h"
#import "AppDelegate.h"
#import "ChatUser.h"
#import <AddressBook/AddressBook.h>
#include <stdlib.h>

#include "Environs.Native.h"


#define CLASS_NAME  "ChatAppView"

ChatAppView         *   chatAppView     = nil;
NSString            *   statusMessage   = @"Hi there!";
NSString            *   loginUserName   = @"Unknown";
NSString            *   userImageBase64 = nil;
ChatUser            *   currentUser     = nil;
id                      keyEventMonitor = nil;


@interface ChatAppView ()
{
#ifdef CHATAPP1
    NSLock          *   userListLock;
    NSMutableArray  *   userList;
#endif

    bool                updaterEnabled;
    bool                listChanged;
    environs::ThreadSync updaterThread;


    NSMutableArray  *   chatsToInit;
    NSLock          *   initLock;
    bool                initThreadEnabled;

    environs::ThreadSync initThread;
}


@property (weak) IBOutlet NSTextField   *   EventLogText;
@property (weak) IBOutlet NSButtonCell  *   ButtonStartStop;
@property (weak) IBOutlet NSTableView   *   deviceTableView;
@property (weak) IBOutlet NSTextField   *   label_WiFi;
@property (weak) IBOutlet NSButton      *   buttonDeleteMessages;
@property (weak) IBOutlet NSButton      *   buttonDeleteFiles;

@property (weak) IBOutlet NSTableView   *   messagesTableView;
@property (weak) IBOutlet NSButton      *   buttonSend;

@property (weak) IBOutlet NSTextField   *   textStatusMessage;
@property (unsafe_unretained) IBOutlet NSTextView *messageText;

@end


@implementation ChatAppView

- (id) init
{
    CVerbN ( "init" );
    
    self = [super init];
    
    if ( self ) {
    }
    
    return self;
}


- (void) StopInitThread
{
    CVerbN ( "StopInitThread" );

    initThreadEnabled = false;

    initThread.Notify ( "StopInitThread" );

    initThread.WaitOne ( "StopInitThread" );
}


- (void) StopListThread
{
    CVerbN ( "StopListThread" );

    updaterEnabled = false;

    updaterThread.Notify ( "StopListThread" );

    updaterThread.WaitOne ( "StopListThread" );
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    chatAppView = self;
    
    [_deviceTableView sizeLastColumnToFit];
    
    deviceList      = nil;

#ifdef CHATAPP1
    userList        = [[NSMutableArray alloc] init];
    userListLock    = [[NSLock alloc] init];
#endif
    
    @autoreleasepool
    {
        NSHost * host   = [NSHost currentHost];

        srand ( (unsigned int) (size_t) &host );
        int r = rand()%20;

        loginUserName = [[NSString alloc ] initWithFormat:@"%@.%@.%i", NSUserName (), [host name], r];

        NSImage * bg = [NSImage imageNamed:@"hcmbg.jpg"];
        if ( bg ) {
            [self.view setWantsLayer:YES];
            self.view.layer.contents = (id)bg;
        }
        
        _textStatusMessage.stringValue = statusMessage;
        _textStatusMessage.bezeled         = NO;
        //_textStatusMessage.editable        = NO;
        
        [_messageText setTextColor:[NSColor yellowColor]];
        
        NSString * defaultUserPicPath = [[NSBundle mainBundle] pathForResource:@"user" ofType:@"png"];
        if ( defaultUserPicPath )
        {
            NSData * userTemplate = [NSData dataWithContentsOfFile:defaultUserPicPath];
            if ( userTemplate ) {
                userImageBase64 = [ChatUser ToBase64:userTemplate];
            }
        }
        
        NSEvent * ( ^handler )( NSEvent * );
        
        handler = ^NSEvent * (NSEvent * theEvent ) {
            
            int keyCode = [theEvent keyCode];
            
            switch ( keyCode )
            {
                case 38:
                    if ( [theEvent modifierFlags] & NSControlKeyMask )
                    {
                        showDebugUserTest = !showDebugUserTest;
#ifdef CHATAPP1
                        [userListLock lock];

                        try
                        {
                            for ( int i = 0; i < [userList count]; ++i )
                            {
                                @autoreleasepool {
                                    ChatUser * user = [userList objectAtIndex:i];

                                    if ( user != nil )
                                    {
                                        [user UpdateProfileText];
                                    }
                                }
                            }
                        } catch ( ... ) {
                            NSLog ( @"viewDidLoad: Exception." );
                        }
                        
                        [userListLock unlock];
#else
                        @autoreleasepool {
                            if ( !deviceList )
                                break;

                            NSArray * list = [deviceList GetDevices];
                            if ( !list )
                                break;
                            try
                            {
                                for ( int i = 0; i < [list count]; ++i )
                                {
                                    @autoreleasepool {
                                        DeviceInstance * device = [list objectAtIndex:i];

                                        if ( device != nil )
                                        {
                                            ChatUser * user = device.appContext1;
                                            if ( user )
                                                [user UpdateProfileText];
                                        }
                                    }
                                }
                            } catch ( ... ) {
                                NSLog ( @"viewDidLoad: Exception." );
                            }
                        }
#endif
                    }
                    break;
                    
                case 40:
                    if ( [theEvent modifierFlags] & NSControlKeyMask )
                    {
#ifdef CHATAPP1
                        [userListLock lock];
                        
                        try
                        {
                            for ( int i = 0; i < [userList count]; ++i )
                            {
                                @autoreleasepool {
                                    ChatUser * user = [userList objectAtIndex:i];
                                    
                                    if ( user != nil )
                                    {
                                        DeviceInstance * device = user->device;
                                        if ( device ) {
                                            [device Connect: environs::Call::NoWait];
                                        }
                                    }
                                }
                            }
                        } catch ( ... ) {
                            NSLog ( @"viewDidLoad: Exception." );
                        }
                        
                        [userListLock unlock];
#endif
                    }
                    break;
                    
                default:
                    break;
            }
            return theEvent;
        };
        
        keyEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:handler];
    }

    [self InitDeviceList];
    [self UpdateUI : environs::Status::Stopped];
    
    [self.view.window makeFirstResponder:self.messageText ];

    listChanged     = false;
    updaterEnabled  = false;
    updaterThread.Init ();
    updaterThread.Run ( pthread_make_routine ( &ListThread ), (__bridge  void * ) self, "viewDidLoad", false );

    initLock        = [[NSLock alloc] init];
    chatsToInit     = [[NSMutableArray alloc] init];

    initThreadEnabled = false;
    initThread.Init ();
    initThread.Run ( pthread_make_routine ( &InitThread ), (__bridge  void * ) self, "viewDidLoad", false );
}


- (void) viewDidAppear
{
}


- (void) viewWillDisappear
{
    CVerbN ( "viewWillDisappear" );
    
    if ( deviceList ) {
        [deviceList DisposeList];
        deviceList = nil;
    }
    
    [appDelegate DisposeViews];
}


void * InitThread ( void * arg )
{
    ChatAppView * appView = (__bridge ChatAppView *) arg;
    if ( appView )
        [appView InitThread];

    return 0;
}


- (void) InitThread
{
    CVerbN ( "InitThread" );
    
    initThreadEnabled   = true;
    NSInteger count;

    while ( initThreadEnabled )
    {
        int needsPingCount = 0;

        @autoreleasepool
        {
            [initLock lock];

            NSArray * chats = [chatsToInit copy];

            [initLock unlock];

            if ( [chats count] > 0 )
            {
                NSMutableArray * chatsToRemove = [[NSMutableArray alloc] init];

                for ( ChatUser * chat in chats )
                {
                    if ( [chat Init] )
                        [chatsToRemove addObject:chat];
                    else {
                        if ( chat->initState == 1 )
                            needsPingCount++;
                    }
                }

                [initLock lock];

                if ( [chatsToRemove count] > 0 ) {
                    [chatsToInit removeObjectsInArray:chatsToRemove];
                }

                count = [chatsToInit count];

                [initLock unlock];
            }
            else
                count = 0;
        }

        int timeout = -1;
        if ( count > 0 )
        {
            if ( needsPingCount > 0 )
                timeout = 1000;
            else
                timeout = 10000;
        }
        //CLogArgN ( "OnListChanged: timeout [ %i ]", timeout );

        initThread.WaitOne ( "InitThread", timeout );
        initThread.ResetSync ( "InitThread" );
    }

    @autoreleasepool
    {
        [initLock lock];

        [chatsToInit removeAllObjects];

        [initLock unlock];
    }
}


- (void) OnListChanged:(NSArray *) vanished appeared:(NSArray *)appeared
{
    CVerbN ( "OnListChanged" );

    bool update = false;

    @autoreleasepool {
        if ( deviceList != nil && deviceList.disposed ) {
            deviceList = nil;
        }
        
        /// Iterate over all OLD items and remove the ChatUser
        if ( vanished != nil && [vanished count] > 0 )
        {
            update = true;

            ChatUser * user = currentUser;
            if ( user ) {
                DeviceInstance * chatDevice = user->device;
                if ( chatDevice )
                {
                    for ( int i=0; i<[vanished count]; i++ )
                    {
                        DeviceInstance * device = (DeviceInstance *) [vanished objectAtIndex:i];

                        if ( device && [chatDevice EqualsID:device] ) {
                            currentUser = nil;
                            break;
                        }
                    }
                }
            }
        }
    }

    @autoreleasepool {
        /// Iterate over all NEW devices and assing to a ChatUser
        if ( appeared != nil && [appeared count] > 0  )
        {
            update = true;

            @autoreleasepool
            {
                NSMutableArray * chatsToAdd = [[NSMutableArray alloc] init];
#ifdef CHATAPP1
                [userListLock lock];
#endif
                try
                {
                    for ( int i=0; i<[appeared count]; i++ )
                    {
                        @autoreleasepool
                        {
                            DeviceInstance * device = (DeviceInstance *) [appeared objectAtIndex:i];
                            if ( !device.appContext0 )
                            {
                                ChatUser * user = [ChatUser initWithDevice:device];
                                if ( user != nil )
                                {
                                    if ( initThreadEnabled )
                                        [chatsToAdd addObject:user];
#ifdef CHATAPP1
                                    [userList addObject:user];
#endif
                                }
                            }
                        }
                    }
                } catch ( ... ) {
                    NSLog ( @"OnListChanged: Exception." );
                }

#ifdef CHATAPP1
                [userListLock unlock];
#endif
                if ( [chatsToAdd count] > 0 ) {
                    [initLock lock];

                    if ( initThreadEnabled )
                        [chatsToInit addObjectsFromArray:chatsToAdd];

                    [initLock unlock];
                }
            }

            initThread.Notify ( "OnListChanged" );
        }

        if ( update && updaterEnabled )
        {
            if ( !listChanged )
            {
                //[userListLock lock];

                listChanged = true;

                //[userListLock unlock];

                updaterThread.Notify ( "OnListChanged" );
            }
        }
    }
}


- (void) OnPortalChanged: (id)sender Notify:(environs::Notify::Portal_t)notification
{
    CLogN ( "OnPortalChanged" );
    
}


- (IBAction) DisConnect: (id)sender
{
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonDisConnect: selected [%i]", (int)row );
    
#ifdef CHATAPP1
    DeviceInstance * device = nil;

    ChatUser * chatUser = nil;

    try {
        if ( row < [userList count] )
            chatUser = (ChatUser *) [userList objectAtIndex:row];
    } catch ( ... ) {
        NSLog ( @"DisConnect: Exception." );
    }
    
    if ( chatUser )
        device = chatUser->device;
#else
    DeviceInstance * device = [deviceList GetItem:(int)row];
#endif
    if ( device ) {
        if ( device.isConnected )
            [device Disconnect];
        else
            [device Connect];
    }
}


- (void) setRepresentedObject: (id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateUI : (environs::Status_t) status
{
    CVerbN ( "UpdateUI" );
    
    @autoreleasepool
    {
        NSString * ssid = @"Unknown";
        
        if ( appDelegate && appDelegate.env )
            ssid = [appDelegate.env GetSSIDDesc];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            @autoreleasepool
            {
                [_label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];

                if ( status == environs::Status::Started ) {
                    [chatAppView.view.window setTitle:[[NSString alloc ] initWithFormat:@APP_NAME " 0x%X | %@", [appDelegate.env GetDeviceID], loginUserName ]];
                }
                
            }
        });        
    }
}


- (IBAction) StatusChanged: (id)sender {
    CVerbN ( "StatusChanged" );
    
    @autoreleasepool {
        statusMessage = _textStatusMessage.stringValue;        
    }
}


- (IBAction) DeleteMessages: (id)sender {
    CVerbN ( "DeleteMessages" );
    if ( currentUser ) {
        DeviceInstance * device = currentUser->device;
        
        if ( device )
            [device ClearMessages];
    }
}


- (IBAction) DeleteFiles: (id)sender {
    CVerbN ( "DeleteFiles" );
    if ( currentUser ) {
        DeviceInstance * device = currentUser->device;
        
        if ( device )
            [device ClearStorage];
    }
}


- (IBAction) ButtonSend: (id)sender {
    CVerbN ( "ButtonSend" );
    if ( !currentUser )
        return;
    
    DeviceInstance * device = currentUser->device;
    if ( !device )
        return;
    
    NSString * msg = [[_messageText textStorage] string];
    if ( !msg )
        return;
    
    [device SendMessage:msg];
    
    [_messageText setString:@""];
}


- (void) InitDeviceList
{
    CVerbN ( "InitDeviceList" );
    
    @autoreleasepool {
        if ( !appDelegate || !appDelegate.env )
            return;
        
        if ( !deviceList )
            deviceList = [appDelegate.env CreateDeviceList: environs::DeviceClass::All ];
        
        if ( deviceList )
            [deviceList AddObserver:self];
    }
}


- (void) ClearDeviceList
{
    CVerbN ( "ClearDeviceList" );
    
    @autoreleasepool {
        if ( deviceList != nil ) {
            [deviceList DisposeList];
            deviceList = nil;
            
#ifdef CHATAPP1
            [userListLock lock];
            
            if ( userList )
                [userList removeAllObjects];
            
            [userListLock unlock];
#endif
            [self UpdateList];
        }
        
        ChatUser * user = currentUser;
        if ( user )
            [user Release];
    }
}


void * ListThread ( void * arg )
{
    ChatAppView * appView = (__bridge ChatAppView *) arg;
    if ( appView )
        [appView UpdateListThread];

    return 0;
}


- (void) UpdateListThread
{
    CVerbN ( "UpdateListThread" );

    updaterEnabled = true;

    while ( updaterEnabled )
    {
        updaterThread.WaitOne ( "UpdateListThread" );
        updaterThread.ResetSync ( "UpdateListThread" );

#ifdef CHATAPP1
        [userListLock lock];

        @autoreleasepool
        {
            try
            {
                if ( listChanged )
                {
#ifdef USE_IMPROVED_UPDATER_THREAD

                    int count = [deviceList GetCount];

                    if ( count == 0 && [userList count] != 0 )
                    {
                        [userList removeAllObjects];
                    }
                    else {
                        NSInteger row = 0;

                        for ( int i = 0; i < [userList count]; ++i )
                        {
                            @autoreleasepool {
                                ChatUser * user = [userList objectAtIndex:i];

                                bool remove = true;

                                if ( user != nil && user->enabled > 0 )
                                {
                                    DeviceInstance * device = user->device;

                                    if ( device != nill && !device.disposed && device.appContext0 < 2 )
                                        remove = false;
                                }

                                if ( remove ) {
                                    [userList removeObjectAtIndex:i];
                                    i--;
                                }
#ifdef USE_CHATUSER_CHANGE_FLAG
                                if ( user->row != row ) {
                                    user->row = row; user->changeAvailable = true;
                                }
                                    row++;
#else
                                user->row = row++;
#endif
                            }
                        }
                    }
#else
                    NSArray * list = [deviceList GetDevices];

                    if ( !list || ([list count] == 0 && [userList count] != 0) )
                    {
                        [userList removeAllObjects];
                    }
                    else {
                        NSInteger row = 0;

                        for ( int i = 0; i < [userList count]; ++i )
                        {
                            @autoreleasepool {
                                ChatUser * user = [userList objectAtIndex:i];

                                bool remove = true;

                                if ( user != nil && user->enabled > 0 )
                                {
                                    DeviceInstance * device = user->device;

                                    if ( device != nill && !device.disposed && device.appContext0 < 2 )
                                        remove = false;
                                }

                                if ( remove ) {
                                    [userList removeObjectAtIndex:i];
                                    i--;
                                }
#ifdef USE_CHATUSER_CHANGE_FLAG
                                if ( user->row != row ) {
                                    user->row = row; user->changeAvailable = true;
                                }
                                row++;
#else
                                user->row = row++;
#endif
                            }
                        }
                    }
                    list = nil;
#endif
                }
            } catch ( ... ) {
                NSLog ( @"UpdateListThread: Exception." );
            }
        }
        
        listChanged = false;
        
        [userListLock unlock];
#endif
        
        dispatch_sync(dispatch_get_main_queue(), ^{

            @autoreleasepool {
                NSTableView * table = chatAppView.deviceTableView;
                if ( table != nil )
                {
#ifndef CHATAPP1
                    if ( listChanged && deviceList ) {
                        @autoreleasepool {

                            NSArray * list = [deviceList GetDevices];
                            if ( list )
                            {
                                try
                                {
                                    for ( int i = 0; i < [list count]; ++i )
                                    {
                                        @autoreleasepool {
                                            DeviceInstance * device = [list objectAtIndex:i];

                                            if ( device != nil )
                                            {
                                                ChatUser * user = device.appContext1;
#ifdef USE_CHATUSER_CHANGE_FLAG
                                                if ( user && user->row != i ) {
                                                    user->row = i; user->changeAvailable = true;
                                                }
#else
                                                if ( user )
                                                    user->row = i;
#endif
                                            }
                                        }
                                    }
                                } catch ( ... ) {
                                    NSLog ( @"UpdateListThread: Exception." );
                                }
                            }
                        }
                        listChanged = false;
                    }
#else
                    [userListLock lock];
#endif
                    ChatUser * user = currentUser;

                    try
                    {
                        NSInteger listCount = [chatAppView.deviceTableView numberOfRows];
#ifdef CHATAPP1
                        NSInteger count = [userList count];
#else
                        NSInteger count = [deviceList GetCount];
#endif

#if defined(USE_CHATUSER_CHANGE_FLAG) && defined(CHATAPP1)
                        if ( count < 14 || count != listCount ) {
                            //if ( !listCount || count < listCount || (count - listCount) > 4 )
                                [table reloadData];
                            /*else {
                                NSIndexSet * rows = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange ( count, (count - listCount) ) ];
                                NSIndexSet * cols = [NSIndexSet indexSetWithIndex:0];

                                [chatAppView.deviceTableView reloadDataForRowIndexes:rows columnIndexes:cols];
                            }*/
                        }
                        else {
                            NSScrollView * scrollView   = [chatAppView.deviceTableView enclosingScrollView];
                            CGRect      visibleRect     = scrollView.contentView.visibleRect;
                            NSRange     range           = [chatAppView.deviceTableView rowsInRect:visibleRect];

                            if ( range.location == 0 && range.length < 14 )
                                [table reloadData];
                            else {
                                // Go through userlist and check whether something has changed
                                NSInteger rStart = 0;
                                NSInteger rEnd = 0;

                                for ( int i = (int)range.location; i < range.length; ++i )
                                {
                                    @autoreleasepool {
                                        ChatUser * user = [userList objectAtIndex:i];

                                        if ( user != nil && user->changeAvailable )
                                        {
                                            if ( !rStart )
                                                rStart = i;
                                            else
                                                rEnd = i;
                                        }
                                    }
                                }

                                if ( !rEnd )
                                    rEnd = rStart;

                                NSIndexSet * rows = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange ( rStart, rEnd )];
                                NSIndexSet * cols = [NSIndexSet indexSetWithIndex:0];

                                [chatAppView.deviceTableView reloadDataForRowIndexes:rows columnIndexes:cols];
                            }
                        }
#else
                        if ( count < 14 || count != listCount )
                            [table reloadData];
                        else {
                            NSScrollView * scrollView   = [chatAppView.deviceTableView enclosingScrollView];
                            CGRect      visibleRect     = scrollView.contentView.visibleRect;
                            NSRange     range           = [chatAppView.deviceTableView rowsInRect:visibleRect];

                            if ( range.location == 0 && range.length < 14 )
                                [table reloadData];
                            else {
                                NSIndexSet * rows = [NSIndexSet indexSetWithIndexesInRange:range];
                                NSIndexSet * cols = [NSIndexSet indexSetWithIndex:0];

                                [chatAppView.deviceTableView reloadDataForRowIndexes:rows columnIndexes:cols];
                            }
                        }
#endif
                    } catch ( ... ) {
                        NSLog ( @"UpdateListThread: Exception." );
                    }
#ifdef CHATAPP1
                    [userListLock unlock];
#endif
                    if ( user == nil )
                        return;

                    DeviceInstance * device = user->device;

                    if ( device == nill || device.disposed ) {
                        currentUser = nil;
                        return;
                    }

                    NSInteger row = user->row;
                    if ( row >= [chatAppView.deviceTableView numberOfRows] )
                        return;

                    NSIndexSet * set = [NSIndexSet indexSetWithIndex:row];

                    [chatAppView.deviceTableView selectRowIndexes:set byExtendingSelection:NO];
                }
            }
        });
    }
}


- (IBAction) UpdateList
{
    CVerbVerbN ( "UpdateList" );
    
    if ( !self.deviceTableView )
        return;

    if ( updaterEnabled )
        updaterThread.Notify ( "UpdateList" );
}


- (IBAction) UpdateRow:(NSInteger) row
{
    //[self UpdateDeviceList:nil];
    
    @autoreleasepool {
        if ( chatAppView.deviceTableView ) {
            dispatch_async(dispatch_get_main_queue(), ^{

                //NSInteger count = [userList count];
                
                //if ( row >= [chatAppView.deviceTableView numberOfRows] || row >= count )
                //    return;

                NSScrollView * scrollView = [chatAppView.deviceTableView enclosingScrollView];

                CGRect visibleRect = scrollView.contentView.visibleRect;

                NSRange range = [chatAppView.deviceTableView rowsInRect:visibleRect];
                if ( row < range.location || row > range.location +  range.length )
                    return;

                @autoreleasepool {
                    NSIndexSet * rowi = [NSIndexSet indexSetWithIndex:row];
                    NSIndexSet * coli = [NSIndexSet indexSetWithIndex:0];
                    
                    [chatAppView.deviceTableView reloadDataForRowIndexes:rowi columnIndexes:coli];
                }
            });
        }
    }
}


+ (void) UpdateMessageList : (ChatUser *) user
{
    CVerbN ( "UpdateMessageList" );
    
    @autoreleasepool {
        if ( chatAppView == nil || user != currentUser )
            return;
        
        dispatch_async(dispatch_get_main_queue(), ^{
            
            [chatAppView.messagesTableView reloadData];
            
            NSInteger rows = [chatAppView.messagesTableView numberOfRows];
            
            if (rows > 0) {
                [chatAppView.messagesTableView scrollRowToVisible:rows - 1];
            }
        });
    }
}


- (IBAction) tableViewSelectionIsChanging:(NSNotification *)notification
{
    CVerbN ( "tableViewSelectionIsChanging" );
    
    @autoreleasepool {
        if ( [notification object] == self.deviceTableView )
        {
            if ( deviceList == nil )
                return;
            
            NSInteger row = [_deviceTableView selectedRow];
            
            //NSLog ( @"tableViewSelectionIsChanging: %i", row );

            ChatUser * user = nil;

#ifdef CHATAPP1
            [userListLock lock];

            try {
                if ( row < [userList count] )
                    user = (ChatUser *) [userList objectAtIndex:row];
            } catch ( ... ) {
                NSLog ( @"tableViewSelectionIsChanging: Exception." );
            }
#else
            DeviceList * list = deviceList;
            if ( list )
            {
                DeviceInstance * device = nil;

                try {
                    if ( row < [list GetCount] ) {
                        device = (DeviceInstance *) [list GetItem: (int) row];
                        if ( device )
                            user = device.appContext1;
                    }
                } catch ( ... ) {
                    NSLog ( @"tableViewSelectionIsChanging: Exception." );
                }
            }
#endif
            ChatUser * prevUser = currentUser;
            
            currentUser = user;
            if ( currentUser != nil ) {
                self.buttonDeleteMessages.enabled = TRUE;
                self.buttonDeleteFiles.enabled = TRUE;
            }
            else {
                self.buttonDeleteMessages.enabled = FALSE;
                self.buttonDeleteFiles.enabled = FALSE;
            }
            
            if ( prevUser != currentUser ) {
                if ( user ) {
                    [ChatAppView UpdateMessageList:user];
                    
                    [user RequestProfile:false  enforce:true];
                }
            }

#ifdef CHATAPP1
            [userListLock unlock];
#endif

            return;
        }
    }
}


- (NSInteger) numberOfRowsInTableView: (NSTableView *)tableView {
    
    CVerbN ( "tableView: numberOfRowsInSection" );
    
    @autoreleasepool {
        if ( deviceList == nil )
            return 0;
        
        if ( tableView == self.deviceTableView )
        {
#ifdef CHATAPP1
            return [userList count];
#else
            return [deviceList GetCount];
#endif
        }
        
        if ( tableView == self.messagesTableView )
        {
            if ( currentUser ) {
                return [currentUser.messages count];
            }
        }
        return 0;
    }
}


- (NSView *) tableView:(NSTableView *)tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    //NSLog ( @"Reload: %i", (int)row );

    @autoreleasepool
    {
        if ( tableView == self.deviceTableView ) {
            NSTableCellView * cell = [tableView makeViewWithIdentifier:@"userCell" owner:self];
            if ( !cell )
                return [[NSTableCellView alloc] init];

            ChatUser * user = nil;

#ifdef CHATAPP1
            if ( userList != nil )
            {
                try {
                    if ( row < [userList count] )
                        user = (ChatUser *) [userList objectAtIndex:row];
                } catch ( ... ) {
                    NSLog ( @"tableView: Exception." );
                }
            }
#else
            DeviceList * list = deviceList;
            if ( list != nil )
            {
                @autoreleasepool
                {
                    DeviceInstance * device = nil;

                    try {
                        if ( row < [list GetCount] ) {
                            device = (DeviceInstance *) [list GetItem: (int) row];
                            if ( device )
                                user = device.appContext1;
                        }
                    } catch ( ... ) {
                        NSLog ( @"tableView: Exception." );
                    }
                }
            }
#endif
            if ( user )
            {
                @autoreleasepool
                {
                    NSString * userText = nil;

                    if ( user ) {
                        user->row = row;

                        userText = [user copyOfProfileText];

                        cell.imageView.image = [user GetProfileImage];
                    }

                    if ( !userText ) {
                        userText = @"Loading ...";
                        cell.imageView.image = nil;
                    }
                    cell.textField.stringValue = userText;
                }

            }
            return cell;
        }

        if ( tableView == self.messagesTableView )
        {
            if ( currentUser )
            {
                ChatUser * chatUser = currentUser;
                if ( chatUser ) {
                    NSTableCellView * cell = [tableView makeViewWithIdentifier:@"messageCell" owner:self];
                    if ( cell ) {
                        @try {
                            MessageInstance * msgInst = [chatUser.messages objectAtIndex:row];

                            if (!msgInst.sent) {
                                CLogN ("tableView: received message");
                                cell.textField.stringValue = [[NSString alloc ] initWithFormat:@"@%@: %@", chatUser.userName, msgInst.text ];
                            }
                            else {
                                cell.textField.stringValue = msgInst.text;

                                [cell.textField setTextColor:[NSColor yellowColor]];
                            }
                            return cell;
                        }
                        @catch ( NSException * ex ) {
                            NSLog ( @"%@", ex.reason );
                        }
                    }
                }
            }
        }

        return [[NSTableCellView alloc] init];
    }
}



@end


