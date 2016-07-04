/**
 * SensorDataView
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

#import "SensorDataView.h"
#import "AppDelegate.h"
#import "SensorOsziView.h"

#include "Environs.Native.h"

#define CLASS_NAME  "SensorDataView"

SensorDataView * appView = nil;

extern AppDelegate * appDelegate;


@interface SensorDataView ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
    DeviceList  *   deviceList;
    
    bool            run;
}

@property (weak) IBOutlet NSTextField *EventLogText;
@property (weak) IBOutlet NSButtonCell *ButtonStartStop;
@property (weak) IBOutlet NSTableView *deviceTableView;
@property (weak) IBOutlet NSTextField *label_WiFi;
@property (weak) IBOutlet NSButton *buttonDisConnect;

@property (weak) IBOutlet SensorOsziView *accelView1;
@property (weak) IBOutlet SensorOsziView *accelView2;
@property (weak) IBOutlet SensorOsziView *accelView3;

@property (weak) IBOutlet SensorOsziView *magneticView1;
@property (weak) IBOutlet SensorOsziView *magneticView2;
@property (weak) IBOutlet SensorOsziView *magneticView3;

@property (weak) IBOutlet SensorOsziView *gyroView1;
@property (weak) IBOutlet SensorOsziView *gyroView2;
@property (weak) IBOutlet SensorOsziView *gyroView3;

@property (weak) IBOutlet SensorOsziView *orientationView1;
@property (weak) IBOutlet SensorOsziView *orientationView2;
@property (weak) IBOutlet SensorOsziView *orientationView3;

@end


@implementation SensorDataView


- (void) viewDidLoad
{
    [super viewDidLoad];
    
    appView = self;
    
    logText = @"";
    logTextLines = 0;
    logTextLock = [[NSLock alloc] init];
    
    [self.deviceTableView sizeLastColumnToFit];
    
    [self.buttonDisConnect setEnabled:FALSE];
    
    deviceList = nil;
    run = false;
    
    NSImage * bg = [NSImage imageNamed:@"hcmbg.jpg"];
    if ( bg ) {
        [self.view setWantsLayer:YES];
        self.view.layer.contents = (id)bg;
    }
    
    if ( self.deviceTableView ) {
        [self.deviceTableView setHeaderView:nil];
    }
    
    [self UpdateUI];
    
    [self.accelView1 SetTitle:@"Acc X"];
    [self.accelView2 SetTitle:@"Acc Y"];
    [self.accelView3 SetTitle:@"Acc Z"];
        
    [self.magneticView1 SetTitle:@"Compass X"];
    [self.magneticView2 SetTitle:@"Compass Y"];
    [self.magneticView3 SetTitle:@"Compass Z"];
    
    [self.gyroView1 SetTitle:@"Gyro X"];
    [self.gyroView2 SetTitle:@"Gyro Y"];
    [self.gyroView3 SetTitle:@"Gyro Z"];
    
    [self.orientationView1 SetTitle:@"Or X"];
    [self.orientationView2 SetTitle:@"Or Y"];
    [self.orientationView3 SetTitle:@"Or Z"];
    
    dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
    dispatch_async ( queue, ^(void) {
        run = true;
        [self updateOsziTimer];
    });
    
}


- (void) viewWillDisappear {
    run = false;
    Sleep(66);
}


- (void) updateOsziTimer
{
    while (run) {
        [self.accelView1 IncreaseTimer];
        [self.accelView2 IncreaseTimer];
        [self.accelView3 IncreaseTimer];
        
        [self.magneticView1 IncreaseTimer];
        [self.magneticView2 IncreaseTimer];
        [self.magneticView3 IncreaseTimer];
        
        [self.gyroView1 IncreaseTimer];
        [self.gyroView2 IncreaseTimer];
        [self.gyroView3 IncreaseTimer];
        
        [self.orientationView1 IncreaseTimer];
        [self.orientationView2 IncreaseTimer];
        [self.orientationView3 IncreaseTimer];
        
        Sleep(33);
    }
}


/**
 * Devicelist handlers
 *
 */
- (void) ReInitDeviceList
{
    if ( !env )
        return;
    
    deviceList = [env CreateDeviceList: environs::DeviceClass::All ];
    
    if ( deviceList )
        [deviceList AddObserver:self];
}


- (void) ReleaseDeviceList
{
    deviceList = nil;
}


- (IBAction) ButtonStartStopPushed:(id) sender
{
    if ( !env )
        return;
    
    Environs * env1 = env;
    
    if ( [env1 GetStatus] >= environs::Status::Started ) {
        [env1 Stop];
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
                [device RemoveObserverForSensors:self];
            }
        }
    }
    
    if ( appearedDevices != nil )
    {
        for ( int i=0; i<[appearedDevices count]; i++ )
        {
            DeviceInstance * device = (DeviceInstance *) [appearedDevices objectAtIndex:i];
            [device AddObserver:self];
            [device AddObserverForSensors:self];
        }
    }
    
    [self UpdateDeviceList:self];
}


- (void) OnDeviceChanged:(id) sender Flags:(environs::DeviceInfoFlag_t) flags
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !sender )
        return;

    if ( (flags & environs::DeviceInfoFlag::IsConnected) == environs::DeviceInfoFlag::IsConnected )
    {
        [self.accelView1 ResetY];
        [self.accelView2 ResetY];
        [self.accelView3 ResetY];
        
        [self.magneticView1 ResetY];
        [self.magneticView2 ResetY];
        [self.magneticView3 ResetY];
        
        [self.gyroView1 ResetY];
        [self.gyroView2 ResetY];
        [self.gyroView3 ResetY];
        
        [self.orientationView1 ResetY];
        [self.orientationView2 ResetY];
        [self.orientationView3 ResetY];
    }
    
    else if ( flags == (environs::DeviceInfoFlag_t) environs::FileInfoFlag::SendProgress )
    {
        //FileInstance * fileInst = (FileInstance *) sender;
        
        //CVerbArgN ( "OnDeviceChanged: File progress [%i]", fileInst.sendProgress );
        return;
    }
    else if ( flags == (environs::DeviceInfoFlag_t) environs::FileInfoFlag::ReceiveProgress ) {
        return;
    }
    
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
}
#endif


- (void) OnPortalChanged:(id) sender Notify:(environs::Notify::Portal_t)notification
{
    CLogN ( "OnPortalChanged" );
    
}


/**
 * OnSensorData is called whenever new sensor data has been received.
 *
 * @param fileInst     The corresponding file object of type FileInstance
 */
- (void) OnSensorData:(environs::SensorFrame *) sensorFrame
{
    if ( !sensorFrame ) return;
    
    environs::lib::SensorFrame * frame = (environs::lib::SensorFrame *) sensorFrame;
    
    switch ( frame->type ) {
        case environs::SensorType::Accelerometer :
            [self.accelView1 UpdateValue:frame->data.floats.f1];
            [self.accelView2 UpdateValue:frame->data.floats.f2];
            [self.accelView3 UpdateValue:frame->data.floats.f3];
            break;
            
        case environs::SensorType::MagneticField:
            [self.magneticView1 UpdateValue:frame->data.floats.f1];
            [self.magneticView2 UpdateValue:frame->data.floats.f2];
            [self.magneticView3 UpdateValue:frame->data.floats.f3];
            break;
            
        case environs::SensorType::Gyroscope:
            [self.gyroView1 UpdateValue:frame->data.floats.f1];
            [self.gyroView2 UpdateValue:frame->data.floats.f2];
            [self.gyroView3 UpdateValue:frame->data.floats.f3];
            break;
            
        default:
            if ( frame->type == environs::SensorType::Location ) {
                environs::lib::SensorFrameExt * frameExt = (environs::lib::SensorFrameExt *) frame;
                
                [self.orientationView1 UpdateValue:frameExt->doubles.d1]; // latitude
                [self.orientationView2 UpdateValue:frameExt->doubles.d2]; // longitude
                [self.orientationView3 UpdateValue:frameExt->doubles.d3]; // altitude
                break;
            }
            [self.orientationView1 UpdateValue:frame->data.floats.f1]; // Light: in Lux
            [self.orientationView2 UpdateValue:frame->data.floats.f2];
            [self.orientationView3 UpdateValue:frame->data.floats.f3];
            break;
    }
}


- (IBAction) ButtonDisConnect:(id)sender
{
    if ( deviceList == nil )
        return;
    
    NSInteger row = [self.deviceTableView selectedRow];
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


- (void) UpdateStatusMessage: (const char *) message
{
    NSString * msg = [[NSString alloc ] initWithCString:message encoding:NSUTF8StringEncoding];
    
    [logTextLock lock];
    
    if ( logTextLines > 4 ) {
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
        [self.EventLogText setStringValue:[[NSString alloc ] initWithString:logText]];
        [self.EventLogText sizeToFit];
    });
    
    
    [logTextLock unlock];
}


- (void) UpdateUI
{
    CVerbN ( "UpdateUI" );
    
    if ( !env )
        return;
    
    environs::Status_t status = [env GetStatus];
    
    NSString * ssid = [env GetSSIDDesc];
    
    [self.label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];
    
    if ( status >= environs::Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.ButtonStartStop.enabled = FALSE;
            [self.ButtonStartStop setTitle:@"Stop"];
            self.ButtonStartStop.enabled = TRUE;
            
            [self.view.window setTitle:[[NSString alloc ] initWithFormat:@"SensorData 0x%X", [env GetDeviceID] ]];
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.ButtonStartStop.enabled = FALSE;
            [self.ButtonStartStop setTitle:@"Start"];
            self.ButtonStartStop.enabled = TRUE;
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
    
    NSInteger row = [self.deviceTableView selectedRow];
    if ( row < 0 ) {
        [self.buttonDisConnect setTitle:@"Connect"];
        [self.buttonDisConnect setEnabled:FALSE];
        return;
    }
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    if ( device ) {
        if ( device.isConnected ) {
            [self.buttonDisConnect setTitle:@"Disconnect"];
            [self.buttonDisConnect setEnabled:TRUE];
        }
        else {
            [self.buttonDisConnect setTitle:@"Connect"];
            [self.buttonDisConnect setEnabled:TRUE];
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
    NSInteger row = [self.deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    DeviceInstance * device = [deviceList GetItem:(int)row];
    
    if ( !device || device.disposed ) {
        return;
    }
    
    if ( [device Connect:environs::Call::Wait] ) {
        NSLog ( @"TestAction: connect ok." );
    }
    else  {
        NSLog ( @"TestAction: connect faile." );
    }
}



@end
