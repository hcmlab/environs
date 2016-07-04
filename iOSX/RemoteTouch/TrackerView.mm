/**
 * TrackerView
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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#import "TrackerView.h"
#import "AppDelegate.h"

#import "Environs.iOSX.h"
#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME  "TrackerView"


@interface TrackerView ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
    DeviceLists  *  deviceList;
    
    int             trackerIndex;
}

@property (weak) IBOutlet NSTextField *EventLogText;
@property (weak) IBOutlet NSButtonCell *ButtonStartStop;
@property (weak) IBOutlet NSTableView *deviceTableView;
@property (weak) IBOutlet NSTextField *label_WiFi;
@property (weak) IBOutlet NSButton *buttonDisConnect;

@property (weak) IBOutlet NSButton *buttonStartTracker;
@property (weak) IBOutlet NSButton *buttonShowDepth;
@property (weak) IBOutlet NSButton *buttonShowRGB;
@property (weak) IBOutlet NSButton *buttonTakeBackground;

@end


@implementation TrackerView


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [_deviceTableView sizeLastColumnToFit];
    
    [_buttonDisConnect setEnabled:FALSE];
    
    _buttonStartTracker.enabled = FALSE;
    _buttonShowDepth.enabled = FALSE;
    _buttonShowRGB.enabled = FALSE;
    _buttonTakeBackground.enabled = FALSE;
    
    deviceList = nil;
    trackerIndex = -1;
    
    [self UpdateUI];
    [self StartEnvirons];
}


- (void) StartEnvirons
{
    [Environs SetApplicationName: "Kinect2Tracker" ];
    [Environs SetProjectName: "Environs" ];
    
    [Environs AddObserver:self];
    [Environs AddObserverForMessages:self];
    [Environs AddObserverForData:self];
    
    [Environs SetMediatorFilterLevel:MEDIATOR_FILTER_NONE];
    
    /// Hardcoded custom mediators for testing purposes
    //[Environs setMediator:@"127.0.0.1" withPort:3389];
    //    [Environs setUseDefaultMediator:false];
    //    [Environs setUseCustomMediator:true];
    //    [Environs setMediator:@"192.168.16.9" withPort:0];
    //    [Environs setMediator:@"192.168.16.6" withPort:0];
    
    [Environs SetUseAuthentication:true];
    
    [Environs Start];
}


- (IBAction) ButtonStartStopPushed:(id)sender
{
    if ( [Environs GetStatus] >= environs::Status::Started ) {
        [Environs Stop];
    }
    else {
        [self StartEnvirons];
    }
}

- (void) OnListChanged
{
    CVerbN ( "OnListChanged" );
    
    if ( deviceList != nil && deviceList->disposed ) {
        deviceList = nil;
    }
    
    [self UpdateDeviceList:self];
}


- (void) OnItemChanged:(id) sender Changed:(int) flags;
{
    CVerbN ( "OnItemChanged" );
    
    if ( !sender )
        return;
    
    DeviceInstance * device = (DeviceInstance *)sender;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        int row = (int)device->appContext0;
        
        [self.deviceTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:0] ];
    });
}




-(bool) OnPortalRequestOrProvided:(id) portalInstance
{
    CLogN ( "OnPortal" );
    
    if ( portalInstance == nil )
        return false;
    
    PortalInstance * portal = (PortalInstance *)portalInstance;
    
    if ( portal->portalID < 0 )
    {
        [portal AddObserver:self];
        [portal Establish:true];
    }
    else
        [portal Start];
    
    return true;
}


- (void) OnPortalChanged:(id) sender Notify:(int)notification
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
        if ( device->info.isConnected )
            [device Disconnect];
        else
            [device Connect];
    }
}


- (IBAction) ButtonStartTracker:(id)sender
{
    if ( trackerIndex < 0 ) {
        trackerIndex = [Environs SetUseTracker:"libEnv-TrackKinect2"];
    }
    else {
        [Environs DisposeTracker:"libEnv-TrackKinect2"];
        trackerIndex = -1;
    }
    
    [self UpdateTrackerButtons];
}


- (IBAction) ButtonShowDepth:(id)sender
{
    if ( trackerIndex < 0 )
        return;
    
    [Environs PushTrackerCommand:trackerIndex cmd:'s'];
}


- (IBAction) ButtonShowRGB:(id)sender
{
    if ( trackerIndex < 0 )
        return;
    
}


- (IBAction) ButtonTakeBackground:(id)sender
{
    if ( trackerIndex < 0 )
        return;
}


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateStatusMessage: (const char *) message
{
    NSString * msg = [NSString stringWithCString:message encoding:NSUTF8StringEncoding];
    
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
        logText = [NSString stringWithFormat:@"%@%@", logText, msg];
    else
        logText = [NSString stringWithFormat:@"%@\n%@", logText, msg];

    dispatch_async(dispatch_get_main_queue(), ^{
        [_EventLogText setStringValue:[NSString stringWithString:logText]];
        [_EventLogText sizeToFit];
    });
    
    
    [logTextLock unlock];
}


- (void) UpdateTrackerButtons
{
    CVerbN ( "UpdateTrackerButtons" );
    
    int status = [Environs GetStatus];
    
    if ( status >= environs::Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            _buttonStartTracker.enabled = TRUE;
            
            
            if ( trackerIndex >= 0 ) {
                [_buttonStartTracker setTitle:@"Stop Tracker"];
                
                _buttonShowDepth.enabled = TRUE;
                _buttonShowRGB.enabled = TRUE;
                _buttonTakeBackground.enabled = TRUE;
            }
            else {
                [_buttonStartTracker setTitle:@"Start Tracker"];
                
                _buttonShowDepth.enabled = FALSE;
                _buttonShowRGB.enabled = FALSE;
                _buttonTakeBackground.enabled = FALSE;
            }
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            _buttonStartTracker.enabled = FALSE;
            _buttonShowDepth.enabled = FALSE;
            _buttonShowRGB.enabled = FALSE;
            _buttonTakeBackground.enabled = FALSE;
        });
    }
}


- (void) UpdateUI
{
    CVerbN ( "UpdateUI" );
    
    int status = [Environs GetStatus];
    
    NSString * ssid = [Environs GetSSIDDesc];
    [_label_WiFi setStringValue:ssid];
    
    if ( status >= environs::Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            //[buttonStart setAttributedTitle:nil forState:UIControlStateNormal];
            [_ButtonStartStop setTitle:@"Stop"];
            _ButtonStartStop.enabled = TRUE;
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [_ButtonStartStop setTitle:@"Start"];
            _ButtonStartStop.enabled = TRUE;
        });
    }
    
    [self UpdateTrackerButtons];
    [self UpdateDeviceList:self];
}


-(void) OnData:(Byte[]) buffer widthLength:(int)length withPacketType:(int)packetType
{
    
}

-(void) OnData:(int) deviceID Project:(const char *) projectName App:(const char *) appName withType:(int) type withFileID:(int) fileID withDescriptor:(const char *) descriptor withSize:(int) size
{
    
}

-(void) OnData:(int) nativeID withType:(int) type withFileID:(int) fileID withDescriptor:(const char *) descriptor withSize:(int) size
{
    
}


-(void) OnMessage:(int) nativeID withType:(int)type withMsg:(const char *) message withLength:(int) msgLength
{
}


-(void) OnMessageExt:(int) deviceID Project:(const char *) projectName App:(const char *) appName withType:(int)type withMsg:(const char *) message withLength:(int) msgLength
{
}


-(void) OnStatus:(int) status
{
    CLogArgN ( "OnStatus: [%s]", environs::resolveName(status) );
    
    [self UpdateUI];
}


-(void) OnStatusMessage:(const char *) message
{
    if ( !message )
        return;
    
    [self UpdateStatusMessage:message];
}



-(void) OnNotify:(int)nativeID withNotify: (int)notification withSource:(int)sourceIdent withContext:(void *)context
{
    if ( notification == NOTIFY_CONNECTION_ESTABLISHED_ACK ) {
        CVerbArgN ( "onNotify: [0x%X] [%i] [%s]", nativeID, sourceIdent, environs::resolveName(notification) );
        CLogN ( "OnNotify: OK" );
    }
    
    if (nativeID == 0 && IsStatus(notification) ) {
        [self OnStatus:GetStatus(notification)];
        return;
    }
    
    CLogArgN ( "onNotify: [0x%X] [%i] [%s]", nativeID, sourceIdent, environs::resolveName(notification) );
    
    unsigned int notifyType = notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        [self UpdateDeviceList:self];
    }

    else if ( notification == NOTIFY_START_SUCCESS )
    {
        deviceList = [[DeviceLists alloc] init];
        [deviceList SetListType:MEDIATOR_DEVICE_CLASS_ALL];
        [deviceList AddObserver:self];
    }
    
    else if ( notification == NOTIFY_STOP_SUCCESS ) {
        deviceList = nil;
    }
}



-(void) OnNotifyExt:(int)deviceID Project:(const char *) projectName App:(const char *) appName withNotify: (int)notification withSource:(int)sourceIdent withContext:(int)context
{
    CVerbArgN ( "onNotify: [0x%X] [%i] [%s]", deviceID, sourceIdent, environs::resolveName(notification) );
    
    unsigned int notifyType = notification & MSG_NOTIFY_CLASS;
    
    if ( notifyType == NOTIFY_TYPE_CONNECTION ) {
        [self UpdateDeviceList:self];
    }
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
    
    /*NSArray *cells = [self.deviceTableView visibleCells];
    
    for (UITableViewCell *cell in cells)
    {
        cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:8.0];
    }
    */
}


- (NSString *) BuildRowText:(DeviceInstance *) device
{
    CVerbN ( "BuildRowText" );
    
    if ( !device )
        return @"ERROR";
    
    DeviceInfo * info = &device->info;
    
    if ( info->ipe != info->ip ) {
        return [NSString stringWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s/%s]", info->isConnected ? "** " : "", info->deviceID,
                [device GetBroadcastString:false], info->deviceName,
                info->appName, info->projectName, [[device GetIP] UTF8String], [[device GetIPe] UTF8String] ];
    }
    return [NSString stringWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s]", info->isConnected ? "** " : "", info->deviceID,
            [device GetBroadcastString:false], info->deviceName,
            info->appName, info->projectName, [[device GetIP] UTF8String] ];
}


- (IBAction)tableViewSelectionDidChange:(NSNotification *)notification
{
    CVerbN ( "tableViewSelectionDidChange" );
    
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 ) {
        [_buttonDisConnect setTitle:@"Connect"];
        [_buttonDisConnect setEnabled:FALSE];
        return;
    }
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    if ( device ) {
        if ( device->info.isConnected ) {
            [_buttonDisConnect setTitle:@"Disconnect"];
            [_buttonDisConnect setEnabled:TRUE];
        }
        else {
            [_buttonDisConnect setTitle:@"Connect"];
            [_buttonDisConnect setEnabled:TRUE];
        }
    }
    
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    
    CVerbN ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList == nil )
        return 0;
    
    return [deviceList GetCount];
}


- (NSView *)tableView:(NSTableView *)tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    if ( deviceList == nil )
        return nil;
    
    //NSTableCellView *cell = [tableView makeViewWithIdentifier:@"MainCell" owner:self];
    
    NSTextField * cell  = [[NSTextField alloc] initWithFrame:NSMakeRect(0,0,550,40)];
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    NSString * deviceInfo = 0;
    
    if ( device ) {
        device->appContext0 = (int)row;
        [device AddObserver:self];
        deviceInfo = [self BuildRowText:device];
    }
    
    if ( !deviceInfo )
        deviceInfo = @"ERROR";
    cell.stringValue = deviceInfo;
    
    return cell;
    
}



@end
