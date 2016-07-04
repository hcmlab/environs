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

using namespace environs;

#define CLASS_NAME  "CamPortalView. . . . . ."

CamPortalView       *   appView = nil;

sp ( DeviceInstance )   currentDevice = nil;

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
    
    
    
    [self UpdateUI:Status::Stopped];
}


- (NSView *) GetRenderView
{
    return _portalImageView;
}


void Observer::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
    CVerbN ( "OnListChanged" );
    
    if ( !appView) return;
    
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
            
            if ( !device->appContext1 )
            {
                if ( appDelegate )
                    device->AddObserver ( appDelegate->observer.get () );
                
                if ( currentDevice && currentDevice->EqualsID ( device.get() ) )
                    currentDevice = device;
            }
        }
    }
    
    [appView UpdateDeviceList:appView];
}


void Observer::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags )
{
    CVerbN ( "OnDeviceChanged" );
    
    if ( !device )
        return;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        int row = device->appContext0;
        
        [appView.deviceTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:0] ];
    });
}


/**
 * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
 *
 * @param sender    The PortalInstance object.
 * @param notify	The notification that indicates the change.
 */
void Observer::OnPortalChanged ( const sp ( PortalInstance ) &sender, Notify::Portale_t notify )
{
    
}


- (IBAction) DisConnect:(id)sender
{
    if ( deviceList == nil )
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


- (void) setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


- (void) UpdateUI : (environs::Status_t) status
{
    CVerbN ( "UpdateUI" );
    
    
    if ( !appDelegate || !appDelegate->env )
        return;
    
    NSString * ssid = [ [NSString alloc ] initWithUTF8String : appDelegate->env->GetSSIDDesc () ];
    
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
        currentDevice->ClearMessages ();
    }
}


- (IBAction) DeleteFiles:(id)sender {
    if ( currentDevice ) {
        currentDevice->ClearStorage ();
    }
}


- (IBAction)ButtonSend:(id)sender {
    if ( !currentDevice )
        return;
    
    NSString * msg = [[_messageText textStorage] string];
    if ( !msg )
        return;
    
    currentDevice->SendMessage ( [msg UTF8String] );
    
    [_messageText setString:@""];
}


- (void) InitDeviceList
{
    if ( !appDelegate || !appDelegate->env )
        return;
    
    if ( !deviceList )
        deviceList =  appDelegate->env->CreateDeviceList ( DeviceClass::All );
    
    if ( deviceList ) {
        deviceList->AddObserver ( appDelegate->observer.get () );
    }
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


- (void) EstablishThread:( sp ( DeviceInstance ) ) device
{
    CVerbN ( "EstablishThread" );
    
    if (device->disposed ())
        return;
    
    if (device->isConnected ()) {
        device->Disconnect ();
        return;
    }
    
    device->Connect ( Call::Wait );
    
    sleep(1);
    
    sp ( PortalInstance ) portal = device->PortalRequest ( PortalType::Any );
    if ( !portal )
        return;
 
    portal->AddObserver ( appDelegate->observer.get () );
    
    portal->SetRenderSurface ( (__bridge void *) [appView GetRenderView] );
    
    portal->startIfPossible = true;
    
    portal->Establish ( false );
}


- (IBAction) DeviceSelected:(id)sender {
    if ( deviceList == nil )
        return;
    
    NSInteger row = [_deviceTableView selectedRow];
    if ( row < 0 )
        return;
    
    sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row);
    
    currentDevice = device;
    if ( currentDevice != nil ) {
        _buttonDeleteMessages.enabled = TRUE;
        _buttonDeleteFiles.enabled = TRUE;
    }
    else {
        _buttonDeleteMessages.enabled = FALSE;
        _buttonDeleteFiles.enabled = FALSE;
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
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
        return deviceList->GetCount ();
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
        
        sp ( DeviceInstance ) device = deviceList->GetItem ( (int)row );
        
        NSString * userText = nil;
        
        if ( device ) {
            device->appContext0 = (int) row;
            
            userText = [ [NSString alloc ] initWithUTF8String:device->toString ().c_str () ];
        }
        
        if ( !userText )
            userText = @"ERROR";
        cell.textField.stringValue = userText;
        
        return cell;
    }
    
    return nil;
}



@end
