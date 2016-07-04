/**
 * DeviceCell.mm
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

#import "DeviceCell.h"
#import "MediaBrowser.h"
#import "MainTab.h"
#import "SettingsTab.h"
#import "DevicesListView.h"
#import "FullscreenController.h"

#import "Environs.iosx.h"
#include "Environs.native.h"


#define CLASS_NAME  "DeviceCell . . . . . . ."

DeviceInstance  *   deviceInstanceToSet = 0;
DeviceInstance  *   deviceInstance      = 0;

DeviceCell      *   deviceCellInstance  = 0;

DeviceList      *   deviceCellList      = 0;

extern id           appCurrentView;
extern MainTab  *   mainTab;

NSLock          *   deviceLock          = [[NSLock alloc] init];


@interface DeviceCell ()
{
    bool uiDisposedInfo;
}

@end

@implementation DeviceCell

@synthesize deviceIdentifierLabel;
@synthesize appLabel;
@synthesize projectLabel;
@synthesize ipLabel;
@synthesize portsLabel;
@synthesize textMessage;
@synthesize progressView;


- (void) OnListChanged:(NSArray *) vanished appeared:(NSArray *) appeared
{
    CVerbVerb ( "OnListChanged" );
    
    if ( !deviceInstance )
        return;
    
    for ( int i=0; i<[vanished count]; i++ ) {
        DeviceInstance * dev = (DeviceInstance *) [vanished objectAtIndex:i];
        if ( !dev )
            continue;
        
        [dev RemoveObserver:self];
    }
    
    for ( int i=0; i<[appeared count]; i++ ) {
        DeviceInstance * dev = (DeviceInstance *) [appeared objectAtIndex:i];
        if ( !dev )
            continue;
        
        if ( [dev EqualsID:deviceInstance] ) {
            deviceInstance = dev;
            [dev AddObserver:self];
            
            dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
            dispatch_async ( queue, ^(void) {
                [self UpdateDeviceInfo:-1];
            });
            break;
        }
    }
}


- (void) OnDeviceChanged:(id) device Flags:(int) DEVICE_INFO_ATTR_changed
{
    CVerbVerb ( "OnDeviceChanged" );
    
    dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
    dispatch_async ( queue, ^(void) {
        [self UpdateDeviceInfo:DEVICE_INFO_ATTR_changed];
        
        [self UpdateUI];
    });
}


+ (void) SetDeviceAndList:(id) device list:(id)deviceList
{
    CVerb ( "SetDeviceAndList" );
    
    [deviceLock lock];
    
    if ( device )
        deviceInstanceToSet = device;

    if ( deviceList )
        deviceCellList = deviceList;
    
    [deviceLock unlock];
}


- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerb ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    return self;
}


- (void) viewDidLoad
{
    CVerb ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    [MediaBrowser UpdateViewBG:self.view];
}



-(void) viewWillAppear: (BOOL)animated
{
    CVerb ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    
    appCurrentView = self;
    
    if ( deviceInstanceToSet ) {
        deviceInstance = deviceInstanceToSet;
        deviceInstanceToSet = nil;
        [deviceInstance AddObserver:self];
    }
    
    MediaBrowser * appDelegate = (MediaBrowser *) [[UIApplication sharedApplication] delegate];
    [appDelegate setTopViewController:self];
    
    uiDisposedInfo = false;
    deviceCellInstance = self;
    progressView.progress = 0.0f;
    
    [deviceCellList AddObserver:self];
    
    [self UpdateDeviceInfo:-1];
    
    [self UpdateUI];
}


- (void) DisposeDeviceInfo
{
    CVerbVerb ( "DisposeDeviceInfo" );
    
    if (uiDisposedInfo)
        return;
    uiDisposedInfo = true;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [deviceLock lock];
        
        deviceIdentifierLabel.text = @"---";
        
        appLabel.text = @"---";
        projectLabel.text = @"---";
        ipLabel.text = @"---";
        ipLabel.text = @"---";
        portsLabel.text = @"---";
        
        [deviceLock unlock];
    });
    
}


- (void) UpdateDeviceInfo:(int) flags
{
    CVerbVerb ( "UpdateDeviceInfo" );
    
    if ( deviceInstance == nil ) return;

    if ( deviceInstance.disposed ) {
        if ( !uiDisposedInfo ) {
            [self DisposeDeviceInfo];
        }
        return;
    }
    
    if (uiDisposedInfo) {
        flags = -1;
        uiDisposedInfo = false;
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        [deviceLock lock];
        
        if ( flags & environs::DeviceInfoFlag::Identity || flags & environs::DeviceInfoFlag::Platform ) {
            deviceIdentifierLabel.text = [[NSString alloc] initWithFormat:@"0x%0X | %s - %s", deviceInstance.deviceID,
                                          [[deviceInstance DeviceTypeString] UTF8String], deviceInstance.deviceName ];
            
            appLabel.text = [[NSString alloc] initWithFormat:@"%s", deviceInstance.appName ];
            projectLabel.text = [[NSString alloc] initWithFormat:@"%s", deviceInstance.areaName ];
        }
        
        if ( flags & environs::DeviceInfoFlag::IP || flags & environs::DeviceInfoFlag::IPe ) {
            if ( deviceInstance.ip == deviceInstance.ipe ) {
                ipLabel.text = [deviceInstance ips];
            }
            else
                ipLabel.text = [[NSString alloc] initWithFormat:@"%@ [ext: %@]", ipLabel.text, [deviceInstance ips] ];
        }
        
        if ( flags & environs::DeviceInfoFlag::UdpPort || flags & environs::DeviceInfoFlag::TcpPort ) {
            portsLabel.text = [[NSString alloc] initWithFormat:@"tcp [%d] udp [%d]", deviceInstance.tcpPort, deviceInstance.udpPort ];
        }
        
        if ( flags & environs::DeviceInfoFlag::ConnectProgress ) {
            [self UpdateConnectProgress:deviceInstance.connectProgress];
        }
        
        if ( flags & environs::DeviceInfoFlag::IsConnected ) {
            if ( !deviceInstance.isConnected )
                [self UpdateConnectProgress:1];            
        }
        
        [deviceLock unlock];
    });
}


- (void) UpdateConnectProgress:(int) progress
{
    CVerbVerb ( "UpdateConnectProgress" );
    
    if (progress > 1000) {
        progress -= 1000;
    }
    progressView.progress = ((float)progress / 100.0f);
}


- (void) dealloc
{
    CLog ( "dealloc" );
    
    if ( deviceInstance ) {
        [deviceInstance RemoveObserver:self];
        deviceInstance = nil;
    }
}


- (void) didReceiveMemoryWarning
{
    CLog ( "didReceiveMemoryWarning" );
    
    [super didReceiveMemoryWarning];
}


-(NSString *) getNetworkStatus
{
    if ( deviceInstance == nil || deviceInstance.disposed )
        return @"Undefined";
    
    return [[NSString alloc] initWithFormat:@"%s %s", deviceInstance.sourceType ? "On same network" : "Mediator detected",
            deviceInstance.isConnected ? "-- Connected --" : "" ];
}


- (void) UpdateUI
{
    if ( !env )
        return;
    
    int status = [env GetStatus];
    
    if (deviceInstance == nil) return;
    
    if (deviceInstance.disposed) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [deviceLock lock];
            
            [[self buttonConnect] setTitle:@"---" forState:UIControlStateNormal];
            [[self buttonPortalIn] setEnabled:true];
            [[self buttonPortalIn] setTitle:@"---" forState:UIControlStateNormal];
            [[self buttonPortalOut] setTitle:@"---" forState:UIControlStateNormal];
            
            [deviceLock unlock];
        });
        return;
    }
    
    if ( status == environs::Status::Connected )
    {
        bool connected = deviceInstance.isConnected;
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [deviceLock lock];
            
            [[self buttonConnect] setEnabled:true];
            if ( connected ) {
                [[self buttonConnect] setTitle:@"Disconnect" forState:UIControlStateNormal];
                [[self buttonPortalIn] setEnabled:true];
                [[self buttonPortalIn] setTitle:@"Portal in" forState:UIControlStateNormal];
                [[self buttonPortalOut] setTitle:@"Portal out" forState:UIControlStateNormal];
                
                //[[self buttonTest] setEnabled:true];
                //[[self buttonTest] setTitle:@"Test" forState:UIControlStateNormal];
            }
            else {
                [[self buttonConnect] setTitle:@"Connect" forState:UIControlStateNormal];
                [[self buttonPortalIn] setEnabled:false];
                [[self buttonPortalIn] setTitle:@"---" forState:UIControlStateNormal];
                [[self buttonPortalOut] setTitle:@"---" forState:UIControlStateNormal];
                
                //[[self buttonTest] setEnabled:false];
                //[[self buttonTest] setTitle:@"---" forState:UIControlStateNormal];
            }
            _networkLabel.text = [self getNetworkStatus];
            
            [deviceLock unlock];
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [deviceLock lock];
            
            [[self buttonConnect] setEnabled:true];
            [[self buttonConnect] setTitle:@"Connect" forState:UIControlStateNormal];
            [[self buttonPortalIn] setEnabled:false];
            [[self buttonPortalIn] setTitle:@"---" forState:UIControlStateNormal];
            [[self buttonPortalOut] setTitle:@"---" forState:UIControlStateNormal];
            
            //[[self buttonTest] setEnabled:false];
            //[[self buttonTest] setTitle:@"---" forState:UIControlStateNormal];
            _networkLabel.text = [self getNetworkStatus];
            
            [deviceLock unlock];
        });
    }
}


- (IBAction) Connect:(id)sender
{
    if ( deviceInstance == nil || deviceInstance.disposed ) return;
    
    if ( deviceInstance.isConnected )
    {
        [deviceInstance Disconnect];
    }
    else
        [deviceInstance Connect];
}


- (IBAction) PortalIn:(id)sender
{
    if ( deviceInstance == nil || deviceInstance.disposed || !app ) return;
    
    PortalInstance * portal = [deviceInstance PortalGetIncoming];
    if ( portal ) {
        [portal Stop];
        return;
    }
    
    portal = [deviceInstance PortalRequest:environs::PortalType::Any];
    if ( !portal )
        return;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
    [portal AddObserver:self];
#else
    [portal AddObserver:app];
#endif
    [portal Establish:true];
}


- (IBAction) PortalOut:(id)sender
{
    if ( deviceInstance == nil || deviceInstance.disposed || !app ) return;
    
    PortalInstance * portal = [deviceInstance PortalGetOutgoing];
    if ( portal ) {
        [portal Stop];
        return;
    }
    
    portal = [deviceInstance PortalProvide:environs::PortalType::Any];
    if ( !portal )
        return;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
    [portal AddObserver:self];
#else
    [portal AddObserver:app];
#endif
    [portal Establish:true];
}


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
-(void) OnPortalRequestOrProvided:(id) portalInstance
{
    if ( portalInstance == nil )
        return;

    PortalInstance * portal = (PortalInstance *)portalInstance;

    CLogArg ( "OnPortal: [ 0x%X ]", portal.portalID );

    [portal AddObserver:self];
    [portal Establish:true];
}


- (void) OnPortalChanged:(id) sender Notify:(int)Environs_NOTIFY_PORTAL_
{
    CVerbArg ( "OnPortalChanged: [ %s ]", environs::resolveName(Environs_NOTIFY_PORTAL_) );

    PortalInstance * portal = (PortalInstance *)sender;

    if ( portal != nil && portal.outgoing ) {
        return;
    }


    FullscreenController * controllerFullscreen = [FullscreenController getInstance];


    if ( portal == nil || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::Disposed || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamPaused
        || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamStopped )
    {
        if (controllerFullscreen)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [controllerFullscreen close];
            });
            [FullscreenController resetInstance];
        }
    }
    else if ( Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::StreamIncoming || Environs_NOTIFY_PORTAL_ == environs::Notify::Portal::ImagesIncoming )
    {
        if (!mainTab)
            return;

        if (controllerFullscreen == 0)
        {
            [FullscreenController SetPortalInstance:portal];

            dispatch_async(dispatch_get_main_queue(), ^{
                [FullscreenController resetInstance];

                if ( [appCurrentView isKindOfClass:[MainTab class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"transit2Fullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[DeviceCell class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"deviceToFullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[DeviceListView class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"listToFullscreen" sender: appCurrentView];
                }
                else if ( [appCurrentView isKindOfClass:[SettingsTab class]] ) {
                    [appCurrentView performSegueWithIdentifier: @"settingsToFullscreen" sender: appCurrentView];
                }
            });
        }
    }
}
#endif


bool doTestRun = false;

- (IBAction) Test:(id)sender
{
    CLog ( "Test" );
    
    if ( deviceInstance ) {
        [deviceInstance ClearStorage];
        [deviceInstance ClearMessages];
    }
    /*
    if ( doTestRun ) {
        doTestRun = false;
        return;
    }
    
    doTestRun = true;
    
    dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
    dispatch_async ( queue, ^(void) {
        [self TestSend];
    });
     */
}


- (void) TestConnect
{
    bool conNext = true;
    int trys = 0;
    int maxTrys = 150;
 //   int did = 3333;
    int conns = 0;
    
    if ( deviceInstance == nil || deviceInstance.disposed ) return;
    
    while (doTestRun) {
        if (conNext) {
            if ( deviceInstance.isConnected ) {
                trys = 0;
                conNext = !conNext;
                conns++;
                CLogArg ( "Connected %i times...", conns );
                continue;
            }
            trys++;
            if ( trys > maxTrys ) {
                doTestRun = false;
                CLogArg ( "Connect failed after %i connections", conns );
                [deviceInstance Disconnect];
                break;
            }
            if ( trys == 1 ) {
                CLog ("----------------------------------------------------------------------------");
                [deviceInstance Connect];
            }
            usleep(200000);
        }
        else {
            if ( deviceInstance.isConnected ) {
                CLog ("----------------------------------------------------------------------------");
                
                [deviceInstance Disconnect];
                usleep(200000);
                continue;
            }
            conNext = !conNext;
            trys = 0;
        }
    }
}



- (void) TestSend
{
    if ( deviceInstance == nil || deviceInstance.disposed ) return;
    
    unsigned char * testBuffer = (unsigned char *) malloc ( 200000 );
    if ( !testBuffer )
        return;
    
    [deviceInstance SendBuffer:189 Desc:"testBuffer" Data:testBuffer Size:200000];
    
    free ( testBuffer );
}


- (IBAction) SendMessage:(id)sender
{
    if ( deviceInstance == nil || deviceInstance.disposed ) return;
    
    [deviceInstance SendMessage:[textMessage text]];
    
}


-(IBAction) dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}

@end
