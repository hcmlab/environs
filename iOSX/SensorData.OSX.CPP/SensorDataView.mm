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
//#   define DEBUGVERBVerb
#endif

#import "SensorDataView.h"
#import "AppDelegate.h"
#import "SensorOsziView.h"

#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME  "SensorDataView"

SensorDataView * appView = nil;

extern AppDelegate * appDelegate;


@interface SensorDataView ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
    sp ( DeviceList ) deviceList;
    
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
    
    if ( _deviceTableView ) {        
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
    if ( appDelegate && appDelegate->env ) {
        deviceList = appDelegate->env->CreateDeviceList ( DeviceClass::All );
        if ( deviceList ) {
            deviceList->AddObserver ( appDelegate->observer.get () );
        }
    }
}


- (void) ReleaseDeviceList
{
    deviceList = nil;
}


- (IBAction) ButtonStartStopPushed:(id) sender
{
    if ( appDelegate->env->GetStatus () >= Status::Started ) {
        appDelegate->env->Stop ();
    }
    else {
        if ( appDelegate )
            [appDelegate StartEnvirons];
    }
}


void Observer::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
    CVerb ( "OnListChanged" );
    
    if ( appView->deviceList != 0 && appView->deviceList->disposed () ) {
        appView->deviceList = 0;
    }
    
    /// Iterate over all OLD items and remove the ChatUser
    if ( vanished != 0 )
    {
        for ( size_t i=0; i<vanished->size (); i++ )
        {
            sp ( DeviceInstance ) device = vanished->at ( i );
            if ( device ) {
                if ( appDelegate )
                    device->RemoveObserver ( appDelegate->observer.get () );
                
                device->appContext1 = 0;
            }
        }
    }
    
    /// Iterate over all NEW devices and attach a ChatUser
    if ( appeared != 0 )
    {
        for ( size_t i=0; i<appeared->size (); i++ )
        {
            sp ( DeviceInstance ) device = appeared->at ( i );
            
            device->AddObserver ( appDelegate->observer.get () );
            device->AddObserverForSensors ( appDelegate->observer.get () );
        }
    }
    
    [appView UpdateDeviceList:nil];
}


void Observer::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags )
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !device )
        return;
    
    if ( (flags & DeviceInfoFlag::IsConnected) == DeviceInfoFlag::IsConnected )
    {
        [appView->_accelView1 ResetY];
        [appView->_accelView2 ResetY];
        [appView->_accelView3 ResetY];
        
        [appView->_magneticView1 ResetY];
        [appView->_magneticView2 ResetY];
        [appView->_magneticView3 ResetY];
        
        [appView->_gyroView1 ResetY];
        [appView->_gyroView2 ResetY];
        [appView->_gyroView3 ResetY];
        
        [appView->_orientationView1 ResetY];
        [appView->_orientationView2 ResetY];
        [appView->_orientationView3 ResetY];
    }
    
    else if ( flags == (DeviceInfoFlag_t) FileInfoFlag::SendProgress ) {
        return;
    }
    else if ( flags == (DeviceInfoFlag_t) FileInfoFlag::ReceiveProgress ) {
        return;
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        int row = device->appContext0;
        
        [appView.deviceTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:0] ];
    });
}


/**
 * OnSensorData is called whenever new sensor data has been received.
 *
 * @param sensorFrame     The corresponding sensorFrame
 */
void Observer::OnSensorData ( environs::SensorFrame * sensorFrame )
{
    //CVerbN ( "OnSensorData" );
    
    if ( !sensorFrame ) return;
    
    environs::lib::SensorFrame * frame = (environs::lib::SensorFrame *) sensorFrame;
    
    switch ( frame->type ) {
        case SensorType::Accelerometer :
            [appView->_accelView1 UpdateValue:frame->data.floats.f1];
            [appView->_accelView2 UpdateValue:frame->data.floats.f2];
            [appView->_accelView3 UpdateValue:frame->data.floats.f3];
            break;
            
        case SensorType::MagneticField:
            [appView->_magneticView1 UpdateValue:frame->data.floats.f1];
            [appView->_magneticView2 UpdateValue:frame->data.floats.f2];
            [appView->_magneticView3 UpdateValue:frame->data.floats.f3];
            break;
            
        case SensorType::Gyroscope:
            [appView->_gyroView1 UpdateValue:frame->data.floats.f1];
            [appView->_gyroView2 UpdateValue:frame->data.floats.f2];
            [appView->_gyroView3 UpdateValue:frame->data.floats.f3];
            break;
            
        default:
            if ( frame->type == SensorType::Location ) {
                environs::lib::SensorFrameExt * frameExt = (environs::lib::SensorFrameExt *) frame;
                
                [appView->_orientationView1 UpdateValue:frameExt->doubles.d1]; // latitude
                [appView->_orientationView2 UpdateValue:frameExt->doubles.d2]; // longitude
                [appView->_orientationView3 UpdateValue:frameExt->doubles.d3]; // altitude
                break;
            }
            
            [appView->_orientationView1 UpdateValue:frame->data.floats.f1]; // Light: in Lux
            [appView->_orientationView2 UpdateValue:frame->data.floats.f2];
            [appView->_orientationView3 UpdateValue:frame->data.floats.f3];
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
    
    CVerbArg ( "ButtonDisConnect: selected [%i]", (int)row );
    
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int) row );
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
    CVerb ( "UpdateUI" );
    
    if ( !appDelegate )
        return;
    
    sp ( Environs ) env = appDelegate->env;
    
    if ( env == 0 ) return;
    
    Status_t status = env->GetStatus ();
    
    std::string sssid = env->GetSSIDDesc ();
    
    NSString * ssid = [[NSString alloc ] initWithUTF8String:sssid.c_str ()];
    
    [self.label_WiFi setStringValue: (ssid != nil ? ssid : @"Unknown") ];
    
    if ( status >= Status::Started )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.ButtonStartStop.enabled = FALSE;
            [self.ButtonStartStop setTitle:@"Stop"];
            self.ButtonStartStop.enabled = TRUE;
            
            [self.view.window setTitle:[[NSString alloc ] initWithFormat:@"SensorData 0x%X", env->GetDeviceID () ]];
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
    CVerb ( "UpdateDeviceListThread" );
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.deviceTableView reloadData];
    });
}


- (IBAction) UpdateDeviceList: (id)sender
{
    CVerb ( "UpdateDeviceList" );
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        [self UpdateDeviceListThread];
    });
}


-(void) UpdateCellViews
{
    CVerb ( "UpdateCellViews" );
}


- (NSString *) BuildRowText : ( sp ( DeviceInstance ) ) device
{
    CVerb ( "BuildRowText" );
    
    if ( !device )
        return @"ERROR";
    
    DeviceInfo * info = device->info ();
    
    if ( info->ipe != info->ip ) {
        return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s/%s]", info->isConnected ? "* " : "", info->deviceID,
                device->GetBroadcastString ( false ), info->deviceName,
                info->appName, info->areaName, device->ips ().c_str (), device->ipes ().c_str () ];
    }
    return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s]", info->isConnected ? "* " : "", info->deviceID,
            device->GetBroadcastString ( false ), info->deviceName,
            info->appName, info->areaName, device->ips ().c_str () ];
}


- (IBAction) tableViewSelectionDidChange:(NSNotification *) notification
{
    CVerb ( "tableViewSelectionDidChange" );
    
    if ( deviceList == nil )
        return;
    
    NSInteger row = [self.deviceTableView selectedRow];
    if ( row < 0 ) {
        [self.buttonDisConnect setTitle:@"Connect"];
        [self.buttonDisConnect setEnabled:FALSE];
        return;
    }
    
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
    
    if ( device ) {
        if ( device->isConnected () ) {
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
    
    CVerb ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList == 0 )
        return 0;
    
    return deviceList->GetCount ();
}


- (NSView *) tableView:(NSTableView *) tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    if ( deviceList == nil )
        return nil;
    
    NSTableCellView * cell = [tableView makeViewWithIdentifier:@"mainCell" owner:self];
    
    if ( cell ) {
        //NSTextField * cell  = [[NSTextField alloc] initWithFrame:NSMakeRect(0,0,550,40)];
        
        //cell.layer.backgroundColor = [[NSColor clearColor] CGColor];
        
        sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
        
        NSString * deviceInfo = 0;
        
        if ( device ) {
            device->appContext0 = (int)row;
            
            device->AddObserver ( appDelegate->observer.get () );
            
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
    
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
    
    if ( !device || device->disposed () ) {
        return;
    }
    
    if ( device->Connect ( Call::Wait ) ) {
        NSLog ( @"TestAction: connect ok." );
    }
    else  {
        NSLog ( @"TestAction: connect faile." );
    }
}



@end
