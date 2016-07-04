/**
 * DeviceListView.mm
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
#import "MessagesList.h"
#import "AppDelegate.h"
#import "ChatUser.h"
#import "ChatUserWatch.h"

#import "Environs.iosx.h"
#include "Environs.native.h"

#include <sys/types.h>
#include <sys/sysctl.h>


#define CLASS_NAME  "ChatAppView. . . . . . ."


ChatAppView         *   chatAppView  = nil;
NSString            *   statusMessage = @"Hi there!";
NSString            *   userImageBase64 = nil;

NSString            *   loginUserName = @"Unknown";

extern MessagesList *   messageList;
extern int              watchRespSequence;

@interface ChatAppView ()
{
    DeviceList      *   deviceList;

    NSLock          *   userListLock;
    NSMutableArray  *   userList;

    bool                updaterEnabled;
    bool                listChanged;
    environs::ThreadSync updaterThread;


    NSMutableArray  *   chatsToInit;
    NSLock          *   initLock;
    bool                initThreadEnabled;

    environs::ThreadSync initThread;
}

@property (weak, nonatomic) IBOutlet UITableView *devicesTableView;

@end


@implementation ChatAppView


- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerbN ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
    return self;
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    chatAppView     = self;
    deviceList      = 0;

    userList        = [[NSMutableArray alloc] init];
    userListLock    = [[NSLock alloc] init];

    [super viewDidLoad];
    
    @autoreleasepool {
        NSString * defaultUserPicPath = [[NSBundle mainBundle] pathForResource:@"user" ofType:@"png"];
        if ( defaultUserPicPath )
        {
            NSData * userTemplate = [NSData dataWithContentsOfFile:defaultUserPicPath];
            if ( userTemplate ) {
                userImageBase64 = [ChatUser ToBase64:userTemplate];
            }
        }
        
        NSString * pStr = nil;
        
        size_t size;
        sysctlbyname ( "hw.machine", NULL, &size, NULL, 0 );
        
        if ( size )
        {
            char * pcStr = (char *) malloc(size);
            if ( pcStr )
            {
                sysctlbyname ( "hw.machine", pcStr, &size, NULL, 0);
                
                pStr = [[NSString alloc] initWithCString:pcStr encoding:NSUTF8StringEncoding];
                free(pcStr);
            }
        }
        
        srand ( (unsigned int) (size_t) &pStr );
        int r = rand()%20;
        
        NSString * uname = NSUserName ();
        if ( uname == nil || [uname isEqualToString:@""] )
            uname = @"Unknown";
        
        loginUserName = [[NSString alloc] initWithFormat:@"%@.%@.%i", uname, pStr, r];
        
        NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
        if ( !prefs ) {
            CErr ( "viewDidLoad: User defaults not accessible!" );
        }
        else {
            NSString * value = [prefs stringForKey:@"userNickName"];
            if ( value && ![value containsString:@"Unknown"] )
                loginUserName = value;
            
            value = [prefs stringForKey:@"userStatus"];
            if ( value )
                statusMessage = value;
        }
        
        [AppDelegate UpdateViewBG:self.view];
        
        [self InitDeviceList];
    }

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


- (void) InitDeviceList
{
    CVerbN ( "InitDeviceList" );

    if ( env) {
        if ( !deviceList ) {
            deviceList = [env CreateDeviceList:environs::DeviceClass::All];
            
            if ( deviceList ) {
                [deviceList AddObserver:self];
            }
        }
    }
    
    [ChatUserWatch SetDeviceList:deviceList];
    
    [self UpdateList];
}


- (void) dealloc
{
    CVerbN ( "dealloc" );
    
    if ( deviceList ) {
        [deviceList RemoveObserver:self];
        deviceList = nil;
    }
    
    chatAppView = nil;
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
    CVerbVerbN ( "OnListChanged" );
    
    if ( deviceList != nil && deviceList.disposed ) {
        deviceList = nil;
    }

    bool update = false;

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

        if ( !update ) {
            if ( vanished != nill && [vanished count] > 0 )
                update = true;
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
    
    if ( watchRespSequence >= 0 ) {
        [ChatUserWatch ReloadUsers];
    }
}


- (void) ReloadRow:(id) sender
{
    CVerbVerbN ( "ReloadRow" );

    [self UpdateList];
    /*
    dispatch_async(dispatch_get_main_queue(), ^{
        
        DeviceInstance * device = (DeviceInstance *)sender;
        
        if ( !device || device.disposed || device.appContext0 >= [_devicesTableView numberOfRowsInSection:0] ) /// Otherwise we reload a row that does not exist anymore (causing a crash)s
            return;
        
        NSIndexPath * row   = [NSIndexPath indexPathForRow:device.appContext0 inSection:0];
        NSArray     * rows  = [NSArray arrayWithObjects:row, nil];
        
        [_devicesTableView reloadRowsAtIndexPaths:rows withRowAnimation:UITableViewRowAnimationNone];
    });
     */
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
                                user->row = row++;
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
                                user->row = row++;
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
                UITableView * table = chatAppView.devicesTableView;
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
                    try
                    {
                        [table reloadData];
                    } catch ( ... ) {
                        NSLog ( @"UpdateListThread: Exception." );
                    }
#ifdef CHATAPP1
                    [userListLock unlock];
#endif
                }
            }
        });
    }
}


- (IBAction) UpdateList
{
    CVerbVerbN ( "UpdateUserList" );
    
    if ( !self.devicesTableView )
        return;

    if ( updaterEnabled )
        updaterThread.Notify ( "UpdateList" );
}


- (IBAction) UpdateRow:(NSInteger) row
{
    //[self UpdateDeviceList:nil];
    
    // This could be the root cause of not disposed devices (keeping an SP and not releasing it)
    if ( self.devicesTableView ) {

        if ( updaterEnabled )
            updaterThread.Notify ( "UpdateList" );
    }
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    deviceList = nil;
}


+ (void) UpdateMessageList:(DeviceInstance *)device
{
    CVerbN ( "UpdateMessageList" );
    
    if ( messageList == nil )
        return;
    [messageList ReloadMessagesList:device];
}


- (void) updateCellViews
{
    NSArray *cells = [self.devicesTableView visibleCells];
    
    for (UITableViewCell *cell in cells)
    {
        @autoreleasepool {
            cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:8.0];
        }
    }
}


- (void) viewWillLayoutSubviews
{
    [super viewWillLayoutSubviews];
}


- (void) viewWillAppear: (BOOL)animated
{
    CVerbN ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    [self UpdateList];
}


- (NSInteger) tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    CVerbVerbN ( "tableView" );
    
    if ( userList ) {
        return [userList count];
    }
    
    return 0;
}


- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CVerbVerbN ( "tableView" );
    
    static NSString *simpleTableIdentifier = @"UserCell";
    
    @autoreleasepool {
        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
        if (cell == nil) {
            cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
        }
        
        cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:12.0];
        cell.textLabel.lineBreakMode = NSLineBreakByWordWrapping;
        cell.textLabel.numberOfLines = 2;
        cell.textLabel.textColor = [UIColor greenColor];
        cell.backgroundColor = [UIColor clearColor];
        
        bool found = false;
        
        @autoreleasepool {
            NSString * userText = nil;
            
            if ( userList != nil )
            {
                ChatUser * user = [userList objectAtIndex:indexPath.row];
                
                if ( user ) {
                    user->row = indexPath.row;
                    
                    userText = [user copyOfProfileText];
                    
                    cell.imageView.image = [user GetProfileImage];
                    found = true;
                }
            }
            
            if ( !found ) {
                cell.imageView.image = nil;
            }
            
            if ( !userText )
                userText = @"Loading ...";
            
            cell.textLabel.text = userText;
        
        }
        return cell;
    }
}


- (void) prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([segue.identifier isEqualToString:@"showMessages"])
    {
        @autoreleasepool {
            NSIndexPath *indexPath = [self.devicesTableView indexPathForSelectedRow];
            
            if ( deviceList ) {
                DeviceInstance * device = [deviceList GetItem:(int) indexPath.row];
                [MessagesList SetDeviceAndList:device];
            }
        }
    }
}
@end
