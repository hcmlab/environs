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
//#   define DEBUGVERBVerb
#endif

#import "AppDelegate.h"
#import "RemoteTouchView.h"

#include "Observer.h"
#include "Environs.Native.h"
#include "Environs.iOSX.Log.h"

using namespace environs;

#define CLASS_NAME  "RemoteTouchView"

RemoteTouchView * appView = nil;

extern AppDelegate * appDelegate;


@interface RemoteTouchView ()
{
    NSString    *   logText;
    int             logTextLines;
    NSLock      *   logTextLock;
    
    sp ( DeviceList ) deviceList;
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
    
    deviceList = 0;
    
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
    if ( appDelegate->env->GetStatus () >= environs::Status::Started ) {
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
    
    RemoteTouchView * app = appView;
    if ( !app ) {
        return;
    }
    
    sp ( DeviceList ) deviceList = app->deviceList;
    
    if ( deviceList != 0 && deviceList->disposed () ) {
        deviceList = 0;
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
        }
    }
    
    [appView UpdateDeviceList:nil];
}


void Observer::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags )
{
    CVerb ( "OnDeviceChanged" );
    
    if ( !device )
        return;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        int row = device->appContext0;
        
        [appView.deviceTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:0] ];
    });
}


- (void) OnPortalChanged:(id) sender Notify:(int)notification
{
    CVerb ( "OnPortalChanged" );
    
}


- (IBAction) ButtonDisConnect:(id)sender
{
    CVerb ( "ButtonDisConnect" );
    
    if ( deviceList == 0 )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonDisConnect: selected [%i]", (int)row );
  
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int) row );
    if ( device ) {
        if ( device->isConnected () )
            device->Disconnect ();
        else
            device->Connect ();
    }
}


- (IBAction) ButtonPortalIn:(id)sender
{
    CVerb ( "ButtonPortalIn" );
    
    if ( deviceList == 0 )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    CLogArgN ( "ButtonPortalIn: selected [%i]", (int)row );
    
    /*
    sp ( lib::DeviceInstance ) device = deviceList->GetItem ( (int) row );

    if ( device && device->isConnected () ) {
        if ( device->IsPortalOutActive () )
            device->PortalOutStop();
        else
            device->PortalOutRequest();
    }
    */
}


- (IBAction) ButtonPortalOut:(id)sender
{
    CVerb ( "ButtonPortalOut" );
    
}


- (void) setRepresentedObject:(id)representedObject {
    CVerb ( "setRepresentedObject" );
    
    [super setRepresentedObject:representedObject];
}


- (void) UpdateStatusMessage: (const char *) message
{
    CVerbVerb ( "UpdateStatusMessage" );
    
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
    CVerb ( "UpdateUI" );
    
    if ( !appDelegate )
        return;
    
    sp ( Environs ) env = appDelegate->env;
    
    if ( env == 0 )
        return;
    
    int status = env->GetStatus ();
    
    std::string sssid = env->GetSSIDDesc ();

    NSString * ssid = [[NSString alloc ] initWithUTF8String:sssid.c_str ()];
    
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


- (NSString *) BuildRowText:( sp ( DeviceInstance ) ) device
{
    CVerb ( "BuildRowText" );
    
    if ( !device )
        return @"ERROR";
  
    DeviceInfo * info = device->info ();
    
    if ( info->ipe != info->ip ) {
        return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s/%s]", info->isConnected ? "** " : "", info->deviceID,
                device->GetBroadcastString ( false ), info->deviceName,
                info->appName, info->areaName, device->ips ().c_str (), device->ipes ().c_str () ];
    }
    return [[NSString alloc ] initWithFormat:@"%s0x%0X: %s %s [%s/%s] [%s]", info->isConnected ? "** " : "", info->deviceID,
            device->GetBroadcastString ( false ), info->deviceName,
            info->appName, info->areaName, device->ipes ().c_str () ];
}


- (IBAction) tableViewSelectionDidChange:(NSNotification *) notification
{
    CVerb ( "tableViewSelectionDidChange" );
    
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
    
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
    
    if ( device != 0 ) {
        if ( device->isConnected () ) {
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
    
    CVerb ( "tableView: numberOfRowsInSection" );
    
    if ( deviceList == 0 )
        return 0;
    
    return deviceList->GetCount ();
}


- (NSView *) tableView:(NSTableView *) tableView  viewForTableColumn:(NSTableColumn *)tableColumn  row:(NSInteger)row
{
    CVerb ( "tableView: viewForTableColumn" );
    
    if ( deviceList == 0 )
        return nil;
    
    NSTableCellView * cell = [tableView makeViewWithIdentifier:@"mainCell" owner:self];
    
    if ( cell ) {
        //NSTextField * cell  = [[NSTextField alloc] initWithFrame:NSMakeRect(0,0,550,40)];
        
        //cell.layer.backgroundColor = [[NSColor clearColor] CGColor];
        
        sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
        
        NSString * deviceInfo = 0;
        
        if ( device != 0 ) {
            device->appContext0 = (int)row;
            
            device->AddObserver ( appDelegate->observer.get () );
            
            deviceInfo = [self BuildRowText:device];
            NSLog ( @"%@", deviceInfo );
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
    CVerb ( "TestAction" );
    
    NSInteger row = [_deviceTableView selectedRow];
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
