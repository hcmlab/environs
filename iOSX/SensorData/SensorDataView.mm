/**
 * SensorDataView.mm
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

#import "Environs.iOSX.h"
#include "Environs.Native.h"

#include <sys/types.h>
#include <sys/sysctl.h>

// IMPORTANT: for location sensors to work
// read http://stackoverflow.com/questions/20664928/ios-core-location-is-not-asking-users-permission-while-installing-the-app-get

#define CLASS_NAME  "ChatAppView. . . . . . ."


SensorDataView      *   sensorDataView  = nil;


bool enableSensorAccelerometer  = true;
bool enableSensorMagneticField  = true;
bool enableSensorGyroscope      = true;
bool enableSensorOrientation    = false;
bool enableSensorLocation       = false;
bool enableSensorLight          = false;

bool enableSensorHeading        = true;
bool enableSensorAltimeter      = false;
bool enableSensorMotionAtt      = false;
bool enableSensorMotionRot      = false;
bool enableSensorMotionGrav     = false;
bool enableSensorMotionAcc      = false;
bool enableSensorMotionMagnetic = false;


@interface SensorDataView ()
{
}

@property (weak, nonatomic) IBOutlet UITableView *devicesTableView;

@end


@implementation SensorDataView


- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerbN ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
    return self;
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    sensorDataView  = self;
    deviceList      = 0;
    
    [super viewDidLoad];
    
    [self InitSensorData];
}


- (void) InitSensorData
{
    CVerbN ( "InitSensorData" );
    
    if ( !appDelegate || !appDelegate.env )
        return;
    
    deviceList = [appDelegate.env CreateDeviceList : environs::DeviceClass::All ];
    
    if ( deviceList )
        [deviceList AddObserver:self];
    
    [AppDelegate UpdateViewBG:self.view];
    
    [self UpdateDeviceList:self];
}


- (void) dealloc
{
    CVerbN ( "dealloc" );
    
    if ( deviceList ) {
        [deviceList RemoveObserver:self];
        deviceList = nil;
    }
    
    sensorDataView = nil;
}


- (void) OnListChanged:(NSArray *) vanishedDevices appeared:(NSArray *)appearedDevices
{
    CVerbVerbN ( "OnListChanged" );
    
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
                device.appContext1 = nil;
                [device RemoveObserver : self];
            }
        }
    }
    
    /// Iterate over all NEW devices and attach a ChatUser
    if ( appearedDevices != nil )
    {
        for ( int i=0; i<[appearedDevices count]; i++ )
        {
            DeviceInstance * device = (DeviceInstance *) [appearedDevices objectAtIndex:i];
            if ( !device.appContext1 )
            {
                [device AddObserver:self];            
            }
        }
    }
    
    [self UpdateDeviceList:self];
}



- (void) OnDeviceChanged:(id) sender Flags:(environs::DeviceInfoFlag_t) flags
{
    CVerbVerbN ( "OnDeviceChanged" );
    
    if ( !sender )
        return;
    
    DeviceInstance * device = (DeviceInstance *) sender;
    
    if ((flags & environs::DeviceInfoFlag::IsConnected) != 0 && device.isConnected)
    {
        [device SetSensorEventSending : environs::SensorType::Accelerometer enable:enableSensorAccelerometer];
        
        [device SetSensorEventSending : environs::SensorType::MagneticField enable:enableSensorMagneticField];
        
        [device SetSensorEventSending : environs::SensorType::Gyroscope enable:enableSensorGyroscope];
        
        [device SetSensorEventSending : environs::SensorType::Orientation enable:enableSensorOrientation];
        
        [device SetSensorEventSending : environs::SensorType::Location enable:enableSensorLocation];
        
        [device SetSensorEventSending : environs::SensorType::Light enable:enableSensorLight];
        
        
        [device SetSensorEventSending : environs::SensorType::Heading enable:enableSensorHeading];
        
        [device SetSensorEventSending : environs::SensorType::Pressure enable:enableSensorAltimeter];
        
        [device SetSensorEventSending : environs::SensorType::Attitude enable:enableSensorMotionAtt];

        [device SetSensorEventSending : environs::SensorType::Rotation enable:enableSensorMotionRot];
        
        [device SetSensorEventSending : environs::SensorType::Gravity enable:enableSensorMotionGrav];

        [device SetSensorEventSending : environs::SensorType::Acceleration enable:enableSensorMotionAcc];
        
        [device SetSensorEventSending : environs::SensorType::MagneticFieldMotion enable:enableSensorMotionMagnetic];
    }
    
    [self ReloadRow:sender];
}


- (void) ReloadRow:(id) sender
{
    CVerbVerbN ( "ReloadRow" );
    
    [self UpdateDeviceList:sender];
}


- (void) UpdateDeviceListThread
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        
        [self.devicesTableView reloadData];
        
    });
}


- (IBAction) UpdateDeviceList: (id)sender
{
    CVerbVerbN ( "UpdateDeviceList" );

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        [self UpdateDeviceListThread];
    });
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    deviceList = nil;
}


- (void) updateCellViews
{
    NSArray *cells = [self.devicesTableView visibleCells];
    
    for (UITableViewCell *cell in cells)
    {
        cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:8.0];
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
    [self UpdateDeviceList:self];
}

DeviceInstance * currentDevice = nil;

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ( deviceList != nil ) {
        DeviceInstance * device = [deviceList GetItem:(int)indexPath.row];
        
        if ( !device )
            return;
        
        if ( device != currentDevice ) {
            if ( currentDevice.isConnected )
                [currentDevice Disconnect];
        }
        
        currentDevice = device;
        if ( device.isConnected )
            [device Disconnect];
        else
            [device Connect];
    }
}


- (NSInteger) tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    CVerbVerbN ( "tableView" );
    
    if ( deviceList ) {
        return [deviceList GetCount];
    }
    
    return 0;
}


- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CVerbVerbN ( "tableView" );
    
    static NSString *simpleTableIdentifier = @"UserCell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }

    bool isLoading = true;

    cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:12.0];
    cell.textLabel.lineBreakMode = NSLineBreakByWordWrapping;
    cell.textLabel.numberOfLines = 2;
    cell.textLabel.textColor = [UIColor greenColor];
    cell.backgroundColor = [UIColor clearColor];
    
    if ( deviceList != nil ) {
        DeviceInstance * device = [deviceList GetItem:(int)indexPath.row];
        if ( device )
        {
            cell.textLabel.text = [device toString];
            isLoading = false;
        }
    }

    if ( isLoading )
        cell.textLabel.text = @"Loading ...";
    return cell;
}


- (void) prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    /*if ([segue.identifier isEqualToString:@"showMessages"])
    {
        NSIndexPath *indexPath = [self.devicesTableView indexPathForSelectedRow];
        
    }*/
}
@end
