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
#import "ChatUser.CPP.h"

#import <AddressBook/AddressBook.h>

#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME  "ChatAppView"

ChatAppView         *   chatAppView     = nil;
NSString            *   statusMessage   = @"Hi there!";
NSString            *   userImageBase64 = nil;
NSString            *   loginUserName   = @"Unknown";
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


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    chatAppView = self;
    
    [_deviceTableView sizeLastColumnToFit];
    
    deviceList = nil;

#ifdef CHATAPP1
    userList        = [[NSMutableArray alloc] init];
    userListLock    = [[NSLock alloc] init];
#endif
    
    @autoreleasepool
    {
        NSHost * host = [NSHost currentHost];
        
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
                            sp ( DeviceList ) list = deviceList;
                            if ( !list )
                                break;
                            try
                            {
                                int count = list->GetCount ();
                                if ( count <= 0 )
                                    break;

                                for ( int i = 0; i < count; ++i )
                                {
                                    @autoreleasepool {
                                        const sp ( DeviceInstance ) &device = list->GetItem ( i );

                                        if ( device != nil )
                                        {
                                            ChatUser * user = (__bridge ChatUser *) device->appContext1;
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

    [self UpdateUI:Status::Stopped];
}


- (void) viewWillDisappear
{
    CVerbN ( "viewWillDisappear" );
    
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


void Observer::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
    CVerbN ( "OnListChanged" );

    bool update = false;
    
    @autoreleasepool {
        ChatAppView * chatApp = chatAppView;

        if ( chatApp )
        {
            if ( chatApp->deviceList != nil && chatApp->deviceList->disposed () ) {
                chatApp->deviceList = nil;
            }

            /// Iterate over all OLD items and remove the ChatUser
            if ( vanished != 0 && chatApp->currentDevice )
            {
                for ( size_t i=0; i<vanished->size (); i++ )
                {
                    const sp ( DeviceInstance ) &device = vanished->at ( i );
                    if ( device ) {
                        if ( chatAppView->currentDevice && chatAppView->currentDevice->EqualsID ( device.get() ) ) {
                            chatAppView->currentDevice = nil;
                            break;
                        }
                    }
                }
            }
        }
        
        /// Iterate over all NEW devices and attach a ChatUser
        if ( appeared != 0 && appeared->size () > 0 && chatApp )
        {
            update = true;

            NSMutableArray * chatsToAdd = [[NSMutableArray alloc] init];
#ifdef CHATAPP1
            [chatApp->userListLock lock];
#endif
            try
            {
                for ( size_t i=0; i<appeared->size (); i++ )
                {
                    @autoreleasepool
                    {
                        const sp ( DeviceInstance ) &device = appeared->at ( i );

                        if ( !device->appContext0 )
                        {
                            ChatUser * user = [ChatUser initWithDevice:device];
                            if ( user != nil )
                            {
                                if ( chatApp->initThreadEnabled )
                                    [chatsToAdd addObject:user];
#ifdef CHATAPP1
                                [chatApp->userList addObject:user];
#endif
                            }

                            if ( chatAppView && chatAppView->currentDevice && chatAppView->currentDevice->EqualsID ( device.get() ) )
                                chatAppView->currentDevice = device;
                        }
                    }
                }
            } catch ( ... ) {
                NSLog ( @"OnListChanged: Exception." );
            }

#ifdef CHATAPP1
            [chatApp->userListLock unlock];
#endif
            if ( [chatsToAdd count] > 0 ) {
                [chatApp->initLock lock];

                if ( chatApp->initThreadEnabled )
                    [chatApp->chatsToInit addObjectsFromArray:chatsToAdd];

                [chatApp->initLock unlock];
            }

            chatApp->initThread.Notify ( "OnListChanged" );
        }

        if ( update && chatApp->updaterEnabled )
        {
            if ( !chatApp->listChanged )
            {
                //[userListLock lock];

                chatApp->listChanged = true;

                //[userListLock unlock];

                chatApp->updaterThread.Notify ( "OnListChanged" );
            }
        }
    }
}


ChatUser * GetChatUser ( const sp ( DeviceInstance ) &device )
{
    ChatUser * chat = nil;

    void * obj = device->appContext1;
    if ( obj )
        chat = (__bridge ChatUser *) obj;

    return chat;
}


void Observer::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags )
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !device )
        return;
    
    @autoreleasepool {
        if ( flags == DeviceInfoFlag::Disposed ) {
            ChatUser * chat = GetChatUser ( device );
            if ( chat )
                [chat Release];
            return;
        }
        
        if ( flags == DeviceInfoFlag::AppContext ) {
            ChatUser * chat = GetChatUser ( device );
            if ( chat ) {
                ChatAppView *   appView = chatAppView;
                if ( appView )
                    [appView UpdateRow: (int) chat->row];
            }
            return;
        }
        
        if ( flags == DeviceInfoFlag::Flags ) {
            ChatUser * chat = GetChatUser ( device );
            if ( chat )
                [chat RequestProfile:false];
        }
        
        else if ( (flags & DeviceInfoFlag::IsConnected) == DeviceInfoFlag::IsConnected )
        {
            ChatUser * chat = GetChatUser ( device );
            if ( chat ) {
                [chat UpdateProfileText];
                
                [chat RequestProfile:true];
                
                ChatAppView *   appView = chatAppView;
                if ( appView )
                    [appView UpdateRow: (int) chat->row];
            }
        }
    }
}


/**
 * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
 *
 * @param sender    The PortalInstance object.
 * @param notify	The notification (environs::Notify::Portale) that indicates the change.
 */
void Observer::OnPortalChanged ( const sp ( PortalInstance ) &sender, environs::Notify::Portale_t notify )
{
    
}


- (IBAction) DisConnect:(id)sender
{
    if ( deviceList == 0 )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonDisConnect: selected [%i]", (int)row );
    
#ifdef CHATAPP1
    sp ( DeviceInstance ) device;

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
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
#endif
    if ( device ) {
        if ( device->isConnected () )
            device->Disconnect ();
        else
            device->Connect ();
    }
}


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateUI : (environs::Status_t) status
{
    CVerbN ( "UpdateUI" );
    
    @autoreleasepool {
        if ( !appDelegate || !appDelegate->env )
            return;
        
        NSString * ssid = [ [NSString alloc ] initWithUTF8String : appDelegate->env->GetSSIDDesc () ];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            
            @autoreleasepool {
                [_label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];
                
                if ( status >= environs::Status::Started ) {
                    [self.view.window setTitle:[[NSString alloc ] initWithFormat:@"ChatApp 0x%X | %@", appDelegate->env->GetDeviceID (), loginUserName ]];
                }
            }
        });
    }
}


- (IBAction) StatusChanged:(id)sender {
    CVerbN ( "StatusChanged" );
    statusMessage = _textStatusMessage.stringValue;
}


- (IBAction) DeleteMessages:(id)sender {
    CVerbN ( "DeleteMessages" );
    if ( currentDevice ) {
        currentDevice->ClearMessages ();
    }
}


- (IBAction) DeleteFiles:(id)sender {
    CVerbN ( "DeleteFiles" );
    if ( currentDevice ) {
        currentDevice->ClearStorage ();
    }
}


- (IBAction)ButtonSend:(id)sender {
    CVerbN ( "ButtonSend" );
    if ( !currentDevice )
        return;
    
    ChatUser * chatUser = (__bridge ChatUser *)currentDevice->appContext1;
    if ( !chatUser )
        return;
    
    NSString * msg = [[_messageText textStorage] string];
    if ( !msg )
        return;
    
    currentDevice->SendMessage ( [msg UTF8String] );
    
    [_messageText setString:@""];
}


- (void) InitDeviceList
{
    CVerbN ( "InitDeviceList" );
    
    @autoreleasepool {
        if ( !appDelegate || !appDelegate->env )
            return;
        
        if ( !deviceList )
            deviceList =  appDelegate->env->CreateDeviceList ( DeviceClass::All );
        
        if ( deviceList ) {
            deviceList->AddObserver ( appDelegate->observer.get () );
        }
    }
}


- (void) ClearDeviceList
{
    CVerbN ( "ClearDeviceList" );
    
    @autoreleasepool {
        if ( deviceList ) {
            deviceList->DisposeList ();
            deviceList = nil;
        }

#ifdef CHATAPP1
        [userListLock lock];

        if ( userList )
            [userList removeAllObjects];

        [userListLock unlock];
#endif

        currentDevice = nil;
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
                    int count = 0;
                    try
                    {
                        if ( deviceList )
                            count = deviceList->GetCount ();
                    }
                    catch (const std::out_of_range& e) {
                        return;
                    }
                    catch (...) {
                        return;
                    }

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
                                    const sp ( DeviceInstance ) &device = user->device;

                                    if ( device != nill && !device->disposed () && device->appContext0 < 2 )
                                        remove = false;
                                }

                                if ( remove ) {
                                    [userList removeObjectAtIndex:i];
                                    i--;
                                }
                                user->row = row++;
                            }
                        }
                    }
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

                            sp ( DeviceList ) list = deviceList;
                            if ( list )
                            {
                                try
                                {
                                    for ( int i = 0; i < list->GetCount (); ++i )
                                    {
                                        @autoreleasepool {
                                            const sp ( DeviceInstance ) &device = list->GetItem ( i );

                                            if ( device != nil )
                                            {
                                                ChatUser * user = (__bridge ChatUser *) device->appContext1;
                                                if ( user )
                                                    user->row = i;
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
#ifdef USE_CHATUSER_CHANGE_FLAG
                        [table reloadData];
#else

#ifdef CHATAPP1
                        if ( [userList count] < 14 )
                            [table reloadData];
#else
                        if ( deviceList->GetCount () < 14 )
                            [table reloadData];
#endif
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

                    const sp ( DeviceInstance ) &device = user->device;

                    if ( device == nill || device->disposed () ) {
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
    CVerbN ( "UpdateList" );

    if ( !self.deviceTableView )
        return;

    if ( updaterEnabled )
        updaterThread.Notify ( "UpdateList" );
}


- (IBAction) UpdateRow:(int) row
{
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
            sp ( DeviceInstance ) device;

            try {
                if ( row < deviceList->GetCount () ) {
                    device = deviceList->GetItem ( (int)row );
                    if ( device )
                        user = (__bridge ChatUser *) device->appContext1;
                }
            } catch ( ... ) {
                NSLog ( @"tableViewSelectionIsChanging: Exception." );
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

                    [user RequestProfile:true];
                }
            }

#ifdef CHATAPP1
            [userListLock unlock];
#endif
            return;
        }
    }
}


- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView {
    
    CVerbVerbN ( "tableView: numberOfRowsInSection" );
    
    @autoreleasepool {
        if ( deviceList == nil )
            return 0;

        if ( tableView == self.deviceTableView )
        {
#ifdef CHATAPP1
            return [userList count];
#else
            return deviceList->GetCount ();
#endif
        }

        if ( tableView == self.messagesTableView ) {
            if ( currentDevice ) {
                ChatUser * chatUser = (__bridge ChatUser *)currentDevice->appContext1;
                if ( chatUser ) {
                    return chatUser.messages.size();
                }            
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
            @autoreleasepool
            {
                sp ( DeviceInstance ) device;

                try {
                    if ( row < deviceList->GetCount () ) {
                        device = deviceList->GetItem ( (int)row );
                        if ( device )
                            user = (__bridge ChatUser *) device->appContext1;
                    }
                } catch ( ... ) {
                    NSLog ( @"tableView: Exception." );
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

        if ( tableView == self.messagesTableView ) {
            if ( currentDevice ) {
                ChatUser * chatUser = (__bridge ChatUser *) currentDevice->appContext1;
                if ( chatUser ) {
                    NSTableCellView * cell = [tableView makeViewWithIdentifier:@"messageCell" owner:self];
                    if ( cell ) {
                        @try {
                            sp ( MessageInstance ) msgInst = chatUser.messages.at ( row );

                            if ( !msgInst->sent () ) {
                                CVerbVerbN ("tableView: received message");
                                cell.textField.stringValue = [[NSString alloc ] initWithFormat:@"@%@: %s", chatUser.userName, msgInst->text () ];
                            }
                            else {
                                cell.textField.stringValue = [[NSString alloc ] initWithUTF8String:msgInst->text () ];

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



