/**
 * MessagesList
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
 *
 * --------------------------------------------------------------------
 */

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "MessagesList.h"
#import "AppDelegate.h"
#import "ChatUser.h"

#import "Environs.iosx.h"
#include "Environs.native.h"


#define CLASS_NAME  "MessageList. . . . . . ."

DeviceInstance  *   deviceInstanceToSet = 0;
MessagesList    *   messageList  = 0;
DeviceList      *   deviceCellList      = 0;
ChatUser        *   chatUser = nil;

extern AppDelegate * appDelegate;


@interface MessagesList ()
{
    DeviceInstance  *   device;
    CGPoint             viewCenter;
    CGRect              viewFrame;
}

@property (weak, nonatomic) IBOutlet UINavigationItem *messagesNavigationItem;
@property (weak, nonatomic) IBOutlet UITextView *textMessage;
@property (weak, nonatomic) IBOutlet UIProgressView *progressView;

@property (weak, nonatomic) IBOutlet UITableView *messagesTableView;

@end

@implementation MessagesList

@synthesize textMessage;
@synthesize progressView;


+ (void) SetDeviceAndList:(id) device
{
    CVerbN ( "SetDeviceAndList" );
    
    if ( device ) {
        deviceInstanceToSet = device;
        
        if ( deviceInstanceToSet.appContext1 )
            chatUser = deviceInstanceToSet.appContext1;
    }
}

- (void) UpdateDevice:(id) updDevice
{
    CVerbN ( "UpdateDevice" );
    
    if ( !device )
        return;
    
    if ( [device EqualsID:updDevice] ) {
        device = updDevice;
        chatUser = device.appContext1;
    }
}


- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    return self;
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    viewCenter = self.view.center;
    viewFrame = self.view.frame;
    
    [AppDelegate UpdateViewBG:self.view];
}



-(void) viewWillAppear: (BOOL)animated
{
    CVerbN ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    
    if ( deviceInstanceToSet ) {
        device = deviceInstanceToSet;
        deviceInstanceToSet = nil;
    }
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWasShown:)
                                                 name:UIKeyboardDidShowNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillBeHidden:)
                                                 name:UIKeyboardWillHideNotification object:nil];
    
    messageList= self;
    progressView.progress = 0.0f;
    
    [self UpdateDeviceInfo:-1];
    
    [self UpdateUI];
}


- (void) UpdateDeviceInfo:(int) flags
{
    CVerbN ( "UpdateDeviceInfo" );
    
    if ( device == nil ) return;

    if ( device.disposed ) {
        return;
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{        
        if ( flags & environs::DeviceInfoFlag::ConnectProgress )
        {
            [self UpdateConnectProgress:device.connectProgress];
        }
        
        if ( flags & environs::DeviceInfoFlag::IsConnected )
        {
            if ( !device.isConnected )
                [self UpdateConnectProgress:1];            
        }
    });
}


- (void) UpdateConnectProgress:(int) progress
{
    if (progress > 1000) {
        progress -= 1000;
    }
    progressView.progress = ((float)progress / 100.0f);
}


- (void) dealloc
{
    CVerbN ( "dealloc" );
    
    if ( device ) {
        device = nil;
    }
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}


- (void) UpdateUI
{
    CVerbN ( "UpdateUI" );
    
    if (device == nil) return;
    
    if (device.disposed) {
        return;
    }
    
    
    dispatch_async(dispatch_get_main_queue(), ^{
        ChatUser * chatUser = device.appContext1;
        if ( !chatUser )
            return;
        _messagesNavigationItem.title = chatUser.userName;
    });
}


- (IBAction) SendMessage:(id)sender
{
    if ( device == nil || device.disposed ) return;
    
    [device SendMessage:[textMessage text]];
    
    [self dismissKeyboardOnTap:nil];
}


- (IBAction) DeleteAll:(id)sender
{
    [self dismissKeyboardOnTap:nil];
    
    if ( device == nil || device.disposed ) return;
    
    [device ClearMessages];
    [device ClearStorage];
    
    ChatUser * chatUser = device.appContext1;
    if ( !chatUser )
        return;
    
    [chatUser.messages removeAllObjects];
    [self ReloadMessagesList:device];
}


- (IBAction) dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}


- (void) keyboardWasShown:(NSNotification*) notification
{
    @autoreleasepool {
        NSDictionary* info = [notification userInfo];
        CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
        
        CGPoint upCenter = viewCenter;
        upCenter.y -= kbSize.height;
        self.view.center = upCenter;
    }
    
    /*CGRect frame = viewFrame;
    frame.origin.y -= kbSize.height;
    self.view.frame = frame;*/
}


- (void) keyboardWillBeHidden:(NSNotification*) notification
{
    self.view.center = viewCenter;
    //self.view.frame = viewFrame;
}



- (void) ReloadMessagesList:(id) deviceInst
{
    CVerbN ( "ReloadMessagesList" );
    
    if ( deviceInst != device ) return;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            @autoreleasepool {
                [_messagesTableView reloadData];
                
                NSInteger rows = [_messagesTableView numberOfRowsInSection:0] - 1;
                
                if (rows >= 0) {
                    NSIndexPath * path = [NSIndexPath indexPathForRow:rows inSection:0];
                    [_messagesTableView scrollToRowAtIndexPath:path atScrollPosition:UITableViewScrollPositionTop animated:NO];
                }
            }
        });
    });
    
}


- (NSInteger) tableView:(UITableView *) tableView numberOfRowsInSection:(NSInteger)section
{
    CVerbN ( "numberOfRowsInSection" );
    
    if ( chatUser ) {
        return [chatUser.messages count];
    }
    
    return 0;
}


- (UITableViewCell *) tableView:(UITableView *) tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CVerbN ( "cellForRowAtIndexPath" );
    
    static NSString *simpleTableIdentifier = @"MessageCell";
    
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
        
        if ( chatUser != nil && chatUser.messages != nil ) {
            try {
                @autoreleasepool {
                    MessageInstance * msgInst = [chatUser.messages objectAtIndex:indexPath.row];
                    if ( msgInst ) {
                        cell.textLabel.text = msgInst.text;
                        
                        if (!msgInst.sent) {
                            CLogN ("tableView: received message");
                            cell.textLabel.text = [[NSString alloc] initWithFormat:@"@%@: %@", chatUser.userName, msgInst.text ];
                        }
                        else
                            cell.textLabel.text = msgInst.text;
                    }                    
                }
            } catch (...) {
                
            }
        }
        return cell;
    }
}


@end
