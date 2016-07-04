/**
 * RemoteTouchView
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
//# define DEBUGVERBVerb
#endif

#import "RemoteTouchView.h"
#import "AppDelegate.h"

#include "Environs.Native.h"

#define CLASS_NAME  "RemoteTouchView"

RemoteTouchView * appView = nil;

extern AppDelegate * appDelegate;


@interface RemoteTouchView ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
    DeviceList  *  deviceList;
}

@property (weak) IBOutlet NSTextField *EventLogText;
@property (weak) IBOutlet NSButtonCell *ButtonStartStop;
@property (weak) IBOutlet NSTableView *deviceTableView;
@property (weak) IBOutlet NSTextField *label_WiFi;
@property (weak) IBOutlet NSButton *buttonDisConnect;
@property (weak) IBOutlet NSButton *buttonPortalIn;
@property (weak) IBOutlet NSButton *buttonPortalOut;

@end


@implementation RemoteTouchView


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    appView = self;
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [_deviceTableView sizeLastColumnToFit];
    
    [_buttonDisConnect setEnabled:FALSE];
    [_buttonPortalIn setEnabled:FALSE];
    [_buttonPortalOut setEnabled:FALSE];
    
    deviceList = nil;
    
    NSImage * bg = [NSImage imageNamed:@"hcmbg.jpg"];
    if ( bg ) {
        [self.view setWantsLayer:YES];
        self.view.layer.contents = (id)bg;
    }
    
    if ( _deviceTableView ) {        
        //[_deviceTableView setBackgroundColor:[NSColor clearColor]];
        [_deviceTableView setHeaderView:nil];
    }
    
    [self UpdateUI];
}


/**
 * Devicelist handlers
 *
 */
- (void) ReInitDeviceList
{
    if ( !appDelegate || !appDelegate.env )
        return;
    
    deviceList = [appDelegate.env CreateDeviceList: environs::DeviceClass::All ];
    
    if ( deviceList )
        [deviceList AddObserver:self];
}


- (void) ReleaseDeviceList
{
    deviceList = nil;
}


- (IBAction) ButtonStartStopPushed:(id) sender
{
    if ( !appDelegate || ! appDelegate.env )
        return;
    
    Environs * env = appDelegate.env;
    
    if ( [env GetStatus] >= environs::Status::Started ) {
        [env Stop];
    }
    else {
        if ( appDelegate )
            [appDelegate StartEnvirons];
    }
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
            [device AddObserver:self];
        }
    }
    
    [self UpdateDeviceList:self];
}


- (void) OnDeviceChanged:(id) sender Flags:(environs::DeviceInfoFlag_t)  flags
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !sender )
        return;
    /*
    if ( flags == (environs::DeviceInfoFlag_t) FILE_INFO_ATTR_SEND_PROGRESS )
    {
        FileInstance * fileInst = (FileInstance *) sender;
        
        CVerbArgN ( "OnDeviceChanged: File progress [%i]", fileInst.sendProgress );
        return;
    }
    else if ( flags == (environs::DeviceInfoFlag_t) FILE_INFO_ATTR_RECEIVE_PROGRESS ) {
        return;
    }
    */
    
    DeviceInstance * device = (DeviceInstance *)sender;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        int row = device.appContext0;
        
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

    [portal AddObserver:appView];
    [portal Establish:true];
}
#endif


- (void) OnPortalChanged:(id) sender Notify:(environs::Notify::Portal_t) notification
{
    CLogN ( "OnPortalChanged" );
    
}


- (IBAction) ButtonDisConnect:(id)sender
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


- (IBAction) ButtonPortalIn:(id)sender
{
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonPortalIn: selected [%i]", (int)row );
    
    
    //DeviceInstance * device = [deviceList GetItem:row];
    /*
    DeviceInfo * di = getDeviceInfo ( (unsigned int) row );
    if ( di && di->isConnected ) {
        DeviceInfoObject * dio = (DeviceInfoObject *)di;
        
        if ( dio->IsPortalOutActive () )
            dio->PortalOutStop();
        else
            dio->PortalOutRequest();
    }
    */
    
    
}


- (IBAction) ButtonPortalOut:(id)sender
{
}


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateStatusMessage: (const char *) message
{
    NSString * msg = [[NSString alloc ] initWithCString:message encoding:NSUTF8StringEncoding];
    
    [logTextLock lock];
    
    if ( logTextLines > 40 ) {
        NSRange range = [logText rangeOfString:@"\n"];
        if ( range.length != NSNotFound ) {
            logText = [logText substringFromIndex: range.location + 1];
        }
        else
            return;
    }
    else
        logTextLines++;
    
    if ( [msg hasSuffix:@"\n"] )
        logText = [[NSString alloc ] initWithFormat:@"%@%@", logText, msg];
    else
        logText = [[NSString alloc ] initWithFormat:@"%@\n%@", logText, msg];

    dispatch_async(dispatch_get_main_queue(), ^{
        [_EventLogText setStringValue:[[NSString alloc ] initWithString:logText]];
        [_EventLogText sizeToFit];
    });
    
    
    [logTextLock unlock];
}


- (void) UpdateUI
{
    CVerbN ( "UpdateUI" );
    Environs * env = nil;
    
    if ( appDelegate && appDelegate.env )
        env = appDelegate.env;
    else
        return;
    
    int status = [env GetStatus];
    
    NSString * ssid = [env GetSSIDDesc];
    
    [_label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];
    
    if ( status >= environs::Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            //[buttonStart setAttributedTitle:nil forState:UIControlStateNormal];
            _ButtonStartStop.enabled = FALSE;
            [_ButtonStartStop setTitle:@"Stop"];
            _ButtonStartStop.enabled = TRUE;
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            _ButtonStartStop.enabled = FALSE;
            [_ButtonStartStop setTitle:@"Start"];
            _ButtonStartStop.enabled = TRUE;
        });
    }
    
    [self UpdateDeviceList:self];
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


-(void) UpdateCellViews
{
    CVerbN ( "UpdateCellViews" );
}


- (NSString *) BuildRowText:(DeviceInstance *) device
{
    CVerbN ( "BuildRowText" );
    
    if ( !device )
        return @"ERROR";
    
    DeviceInstance * info = device;
    
    if ( info.ipe != info.ip ) {
        return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s/%s]", info.isConnected ? "** " : "", info.deviceID,
                [device GetBroadcastString:false], info.deviceName,
                info.appName, info.areaName, [[device ips] UTF8String], [[device ipes] UTF8String] ];
    }
    return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s]", info.isConnected ? "** " : "", info.deviceID,
            [device GetBroadcastString:false], info.deviceName,
            info.appName, info.areaName, [[device ips] UTF8String] ];
}


- (IBAction) tableViewSelectionDidChange:(NSNotification *) notification
{
    CVerbN ( "tableViewSelectionDidChange" );
    
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 ) {
        [_buttonDisConnect setTitle:@"Connect"];
        [_buttonDisConnect setEnabled:FALSE];
        [_buttonPortalIn setEnabled:FALSE];
        [_buttonPortalOut setEnabled:FALSE];
        return;
    }
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    if ( device ) {
        if ( device.isConnected ) {
            [_buttonDisConnect setTitle:@"Disconnect"];
            [_buttonDisConnect setEnabled:TRUE];
            [_buttonPortalIn setEnabled:TRUE];
            [_buttonPortalOut setEnabled:TRUE];
        }
        else {
            [_buttonDisConnect setTitle:@"Connect"];
            [_buttonDisConnect setEnabled:TRUE];
            [_buttonPortalIn setEnabled:FALSE];
            [_buttonPortalOut setEnabled:FALSE];
        }
    }
    
}


- (NSInteger) numberOfRowsInTableView:(NSTableView *) tableView {
    
    CVerbN ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList == nil )
        return 0;
    
    return [deviceList GetCount];
}


- (NSView *) tableView:(NSTableView *) tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    if ( deviceList == nil )
        return nil;
    
    NSTableCellView * cell = [tableView makeViewWithIdentifier:@"mainCell" owner:self];
    
    if ( cell ) {
        //NSTextField * cell  = [[NSTextField alloc] initWithFrame:NSMakeRect(0,0,550,40)];
        
        //cell.layer.backgroundColor = [[NSColor clearColor] CGColor];
        
        DeviceInstance * device = [deviceList GetItem:(int)row];
        
        NSString * deviceInfo = 0;
        
        if ( device ) {
            device.appContext0 = (int)row;
            [device AddObserver:self];
            deviceInfo = [self BuildRowText:device];
        }
        
        if ( !deviceInfo )
            deviceInfo = @"ERROR";
        cell.textField.stringValue = deviceInfo;
        
    }
    
    return cell;
}


int testStatus = 0;

- (IBAction) TestAction:(id) sender
{
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    if ( !device || device.disposed || !device.isConnected ) {
        return;
    }
    
    const char * fileName = "test.png";
    [device SendFile:1234 Desc:fileName Path:fileName];
    /*
    if ( [device Connect:CALL_WAIT] ) {
        NSLog ( @"TestAction: connect ok." );
    }
    else  {
        NSLog ( @"TestAction: connect faile." );
    }
     */
}



@end
