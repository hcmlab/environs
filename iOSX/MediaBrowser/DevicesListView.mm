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

#import "DevicesListView.h"
#import "DeviceCell.h"
#import "MediaBrowser.h"

#import "Environs.iOSX.h"
#include "Environs.Native.h"


#define CLASS_NAME  "DeviceListView . . . . ."

DeviceListView      *   deviceListInstance  = 0;
extern id               appCurrentView;


@interface DeviceListView ()
{
    DeviceList  *  deviceList;
}

@end


@implementation DeviceListView

@synthesize     devicesTableView        = _devicesTableView;

- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerb ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if ( self )
        deviceListInstance = self;
    
    return self;
}


- (void) viewDidLoad
{
    CVerb ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    deviceListInstance = self;
    
    [self InitDeviceList];
    
    [MediaBrowser UpdateViewBG:self.view];
    
    [self updateDeviceList:self];
}


- (void) dealloc
{
    CLog ( "dealloc" );
    
    [deviceList RemoveObserver:self];
    deviceList = nil;
    
    deviceListInstance = nil;
}


- (void) InitDeviceList
{
    if ( !env )
        return;
    
    deviceList = [ env CreateDeviceList : environs::DeviceClass::All ];
    
    [deviceList AddObserver:self];
}


- (void) OnListChanged:(NSArray *) vanishedDevices appeared:(NSArray *)appearedDevices
{
    CVerb ( "OnListChanged" );
    
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
            if ( device )
                [device AddObserver:self];
        }
    }
    
    [self updateDeviceListThread];
}


- (void) OnDeviceChanged:(id) sender Flags:(int) flags
{
    if ( !sender )
        return;
    
    [self ReloadRow:sender];    
}


- (void) ReloadRow:(id) sender
{
    dispatch_async(dispatch_get_main_queue(), ^{
        
        [self.devicesTableView reloadData];
        /*
        DeviceInstance * device = (DeviceInstance *)sender;
        
        CLogArgN ( "Reload [%i] - Rows; %i", device.appContext0, (int)[_devicesTableView numberOfRowsInSection:0] );
        if ( device.disposed || device.appContext0 >= [_devicesTableView numberOfRowsInSection:0] ) /// Otherwise we reload a row that does not exist anymore (causing a crash)s
            return;
        
        NSIndexPath * row   = [NSIndexPath indexPathForRow:device.appContext0 inSection:0];
        NSArray     * rows  = [NSArray arrayWithObjects:row, nil];
        
        [_devicesTableView reloadRowsAtIndexPaths:rows withRowAnimation:UITableViewRowAnimationNone];
         */
    });
}


- (void) updateDeviceListThread
{
    CVerb ( "updateDeviceListThread" );
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.devicesTableView reloadData];
    });
}


- (IBAction) updateDeviceList: (id)sender
{
    CVerbVerb ( "updateDeviceList" );
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        [self updateDeviceListThread];
    });
}


- (void) didReceiveMemoryWarning
{
    CVerb ( "didReceiveMemoryWarning" );
    
    [super didReceiveMemoryWarning];
    
    deviceList = nil;
}


-(void) updateCellViews
{
    CVerbVerb ( "updateCellViews" );
    
    NSArray *cells = [self.devicesTableView visibleCells];
    
    for (UITableViewCell *cell in cells)
    {
        cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:8.0];
    }
}


-(void) viewWillLayoutSubviews
{
    CVerb ( "viewWillLayoutSubviews" );

    [super viewWillLayoutSubviews];
}


-(void) viewWillAppear: (BOOL)animated
{
    CVerb ( "viewWillAppear" );

    [super viewWillAppear:animated];
    [self updateDeviceList:self];
    
    appCurrentView = self;
    
    if ( env )
        [env SetMediatorNotificationSubscription:true];
}



- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    CVerb ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList ) {
        return [deviceList GetCount];
    }
    
    return 0;
}


NSString * buildRowText ( DeviceInstance * device )
{
    CVerb ( "buildRowText" );
    
    if ( !device )
        return @"ERROR";
    
    NSString *IP = [device ips];
    NSString *IPe = [device ipes];
    
    if ( device.ipe != device.ip ) {
        return [[NSString alloc] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%@/%@]", device.isConnected ? "* " : "", device.deviceID,
                [device GetBroadcastString:false], device.deviceName,
                device.appName, device.areaName, IP, IPe ];
    }
    return [[NSString alloc] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%@]", device.isConnected ? "* " : "", device.deviceID,
            [device GetBroadcastString:false], device.deviceName,
            device.appName, device.areaName, IP ];
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CVerb ( "tableView" );

    static NSString *simpleTableIdentifier = @"DeviceCell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }

    if ( cell ) {
        cell.textLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:12.0];
        cell.textLabel.lineBreakMode = NSLineBreakByWordWrapping;
        cell.textLabel.numberOfLines = 2;
        cell.textLabel.textColor = [UIColor greenColor];
        //cell.textLabel.backgroundColor = [UIColor blackColor];
        cell.backgroundColor = [UIColor clearColor];
        
        //UIView *backgroundSelectedCell = [[UIView alloc] init];
        //[backgroundSelectedCell setBackgroundColor:[UIColor darkGrayColor]];
        
        //cell.selectedBackgroundView = backgroundSelectedCell;
        
        NSString * deviceInfo = 0;
        
        if ( deviceList != nil ) {
            DeviceInstance * device = [deviceList GetItem:(int)indexPath.row];
            if ( device ) {
                deviceInfo = buildRowText ( device );
                
                device.appContext0 = (int) indexPath.row;
            }
        }
        
        if ( !deviceInfo )
            deviceInfo = @"ERROR";
        
        cell.textLabel.text = deviceInfo;
    }
    else
        cell = [[UITableViewCell alloc] init];
    
    return cell;
}


- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([segue.identifier isEqualToString:@"showDeviceDetails"])
    {
        NSIndexPath *indexPath = [self.devicesTableView indexPathForSelectedRow];
        
        if ( deviceList ) {
            DeviceInstance * device = [deviceList GetItem:(int) indexPath.row];
            [DeviceCell SetDeviceAndList:device list:deviceList];
        }
        
//        DeviceDetailsController *destViewController = segue.destinationViewController;
        
        //destViewController.recipeName = [recipes objectAtIndex:indexPath.row];
    }
}
@end
