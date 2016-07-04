/**
 * CamPortalView
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

#import "CamPortalView.h"
#import "AppDelegate.h"
#import <AddressBook/AddressBook.h>

#include "Environs.Native.h"


#define CLASS_NAME  "CamPortalView. . . . . ."

CamPortalView       *   appView = nil;


DeviceInstance      *   currentDevice = nil;

@interface CamPortalView ()
{
}

@property (weak) IBOutlet NSTableView   *   deviceTableView;
@property (weak) IBOutlet NSTextField   *   label_WiFi;
@property (weak) IBOutlet NSButton      *   buttonDeleteMessages;
@property (weak) IBOutlet NSButton      *   buttonDeleteFiles;

@property (weak) IBOutlet NSButton      *   buttonSend;
@property (unsafe_unretained) IBOutlet NSTextView *messageText;

@property (weak) IBOutlet NSImageView   *   portalImageView;
@property (weak) IBOutlet NSView *portalView;
@end


@implementation CamPortalView


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    appView = self;
    
    NSImage * bg = [NSImage imageNamed:@"hcmbg.jpg"];
    if ( bg ) {
        [self.view setWantsLayer:YES];
        self.view.layer.contents = (id)bg;
    }
    
    [_deviceTableView sizeLastColumnToFit];
    
    deviceList = nil;
    
    [self UpdateUI];
}


- (NSView *) GetRenderView
{
    return _portalImageView;
}


- (void) OnListChanged:(NSArray *) vanishedDevices appeared:(NSArray *)appearedDevices
{
    CVerbN ( "OnListChanged" );
    
    if ( deviceList != nil && deviceList.disposed ) {
        deviceList = nil;
    }
    
    /// Iterate over all OLD items and remove the ChatUser
    if ( vanishedDevices != nil )
    {
        for ( int i=0; i<[vanishedDevices count]; i++ )
        {
            DeviceInstance * device = (DeviceInstance *) [vanishedDevices objectAtIndex:i];
            if ( device ) {
                [device RemoveObserver:self];
            }
        }
    }
    
    /// Iterate over all NEW devices and attach a ChatUser
    if ( appearedDevices != nil )
    {
        for ( int i=0; i<[appearedDevices count]; i++ )
        {
            DeviceInstance * device = (DeviceInstance *) [appearedDevices objectAtIndex:i];
            if ( !device )
                continue;
            
            [device AddObserver:self];
            
            if ( currentDevice && [currentDevice EqualsID:device] )
                currentDevice = device;
        }
    }
    
    [self UpdateDeviceList:self];
}


- (void) OnDeviceChanged:(id) sender Flags:(environs::DeviceInfoFlag_t) flags
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !sender )
        return;
    
    if ( flags == (environs::DeviceInfoFlag_t) FILE_INFO_ATTR_SEND_PROGRESS )
    {
        return;
    }
    else if ( flags == (environs::DeviceInfoFlag_t) FILE_INFO_ATTR_RECEIVE_PROGRESS ) {
        return;
    }
    
    DeviceInstance * device = (DeviceInstance *)sender;
    
    int row = (int)device.appContext0;
    
    if ( row >= [self.deviceTableView numberOfRows] )
        return;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.deviceTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:0] ];
    });
}


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );

    if ( portalInstance == nil )
        return;

    PortalInstance * portal = (PortalInstance *)portalInstance;

    [portal AddObserver:self];

    if ( portal.isIncoming ) {
        [portal SetRenderSurface: [appView GetRenderView]];

        portal.startIfPossible = true;
    }

    [portal Establish:true];
}
#endif


- (void) OnPortalChanged:(id) sender Notify:(environs::Notify::Portal_t) notification
{
    CLogN ( "OnPortalChanged" );
    
}


- (IBAction) DisConnect:(id)sender
{
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonDisConnect: selected [%i]", (int)row );
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    if ( device ) {
        if ( device.isConnected )
            [device Disconnect];
        else
            [device Connect];
    }
}


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateUI
{
    CVerbN ( "UpdateUI" );
    
    NSString * ssid = @"Unknown";
    
    if ( appDelegate && appDelegate.env )
        ssid = [appDelegate.env GetSSIDDesc];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];
    });
    
    [self UpdateDeviceList:self];
    
}

- (void) UpdateStatusMessage: (NSString *) message
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [_messageText setString:message];
    });
    
}

- (IBAction) StatusChanged:(id)sender {
}


- (IBAction) DeleteMessages:(id)sender {
    if ( currentDevice ) {
        [currentDevice ClearMessages];
    }
}


- (IBAction) DeleteFiles:(id)sender {
    if ( currentDevice ) {
        [currentDevice ClearStorage];
    }
}


- (IBAction)ButtonSend:(id)sender {
    if ( !currentDevice )
        return;
    
    NSString * msg = [[_messageText textStorage] string];
    if ( !msg )
        return;
    
    [currentDevice SendMessage:msg];
    
    [_messageText setString:@""];
}


- (void) InitDeviceList
{
    if ( !appDelegate || !appDelegate.env )
        return;
    
    deviceList = [appDelegate.env CreateDeviceList: environs::DeviceClass::All ];
    
    if ( deviceList )
        [deviceList AddObserver:self];
}


- (void) ClearDeviceList
{
    deviceList = nil;
}


- (void) UpdateDeviceListThread
{
    CVerbN ( "UpdateDeviceListThread" );
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.deviceTableView reloadData];
    });
}


- (IBAction) UpdateDeviceList: (id)sender
{
    CVerbN ( "UpdateDeviceList" );
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        [self UpdateDeviceListThread];
    });
}


- (void) EstablishThread:(DeviceInstance *) device
{
    CVerbN ( "EstablishThread" );
    
    if (device.disposed)
        return;
    
    if (device.isConnected) {
        [device Disconnect];
        return;
    }
    
    [device Connect: environs::Call::Wait];
    
    sleep(1);
    
    if (!device.isConnected)
        return;
    
    PortalInstance * portal = [device PortalRequest:environs::PortalType::Any];
    if ( !portal )
        return;
    
    [portal AddObserver:self];
    
    [portal SetRenderSurface: [self GetRenderView]];
    
    portal.startIfPossible = true;
    
    [portal Establish:false];
}


- (IBAction) DeviceSelected:(id)sender {
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    currentDevice = device;
    if ( currentDevice != nil ) {
        _buttonDeleteMessages.enabled = TRUE;
        _buttonDeleteFiles.enabled = TRUE;
    }
    else {
        _buttonDeleteMessages.enabled = FALSE;
        _buttonDeleteFiles.enabled = FALSE;
    }

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        [self EstablishThread:device];
    });
}


- (IBAction) tableViewSelectionDidChange:(NSNotification *)notification
{
    CVerbN ( "tableViewSelectionDidChange" );
    
    [self DeviceSelected:nil];
}


- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView {
    
    CVerbN ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList == nil )
        return 0;
    
    if ( tableView == _deviceTableView )
        return [deviceList GetCount];
    return 0;
}


- (NSView *) tableView:(NSTableView *)tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    if ( deviceList == nil )
        return nil;
    
    if ( tableView == _deviceTableView ) {
        NSTableCellView * cell = [tableView makeViewWithIdentifier:@"userCell" owner:self];
        if ( !cell )
            return nil;
        
        DeviceInstance * device = [deviceList GetItem:(int)row];
        
        NSString * userText = nil;
        
        if ( device ) {
            device.appContext0 = (int) row;
            
            userText = device.toString;
        }
        
        if ( !userText )
            userText = @"ERROR";
        cell.textField.stringValue = userText;
        
        return cell;
    }
    
    return nil;
}



@end
