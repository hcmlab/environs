/**
 * SettingsTab.mm
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

#import "SettingsTab.h"
#import "AppDelegate.h"
#import "SensorDataView.h"
#import "Environs.iosx.h"
#include "Environs.native.h"

#define CLASS_NAME  "SettingsTab. . . . . . ."


extern AppDelegate      *   appDelegate;
extern SensorDataView   *   sensorDataView;

@interface SettingsTab ()

@property (weak, nonatomic) IBOutlet UISwitch *switchParticipate;

@end

@implementation SettingsTab

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    CVerbN ( "initWithNibName" );
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}


- (void) viewWillAppear:(BOOL)animated
{
    CVerbN ( "viewWillAppear" );
    
    [super viewWillAppear:animated];
    
    [self updateUI];
}


- (IBAction) ToggleEnvirons:(id)sender
{
    CVerbN ( "ToggleEnvirons" );
    
    if ( !env )
        return;
    
    int status = [env GetStatus];
    
    if ( status >= environs::Status::Started ) {
        [env Stop];
    }
    else {
        [env Start];
        
        if (sensorDataView)
            [sensorDataView InitSensorData];
    }
}


- (void) viewDidLoad
{
    CVerbN ( "viewDidLoad" );
    
    [super viewDidLoad];
    
    if ( env )
        [self.labelVersionString setText:[[NSString alloc] initWithFormat:@"Environs: %@", [[NSString alloc] initWithUTF8String: [env GetVersionString]]]];
    
    [self LoadSettings];
    
    [AppDelegate UpdateViewBG:self.view];
}


- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}


- (BOOL) textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return NO;
}


- (IBAction) dismissKeyboardOnTap:(id)sender
{
    [[self view] endEditing:YES];
}


- (void) updateUI
{
    if ( !env )
        return;
    [self LoadSettings];
}


- (void) LoadSettings
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.switchAccelerometer setOn:enableSensorAccelerometer];
        [self.switchCompass setOn:enableSensorMagneticField];
        [self.switchGyro setOn:enableSensorGyroscope];
        [self.switchLocation setOn:enableSensorLocation];
        [self.switchLight setOn:enableSensorLight];
        [self.switchOrientation setOn:enableSensorOrientation];
        
        [self.switchHeading setOn:enableSensorHeading];
        [self.switchAltimeter setOn:enableSensorAltimeter];
        [self.switchMotionAtt setOn:enableSensorMotionAtt];
        [self.switchMotionRot setOn:enableSensorMotionRot];
        [self.switchMotionGrav setOn:enableSensorMotionGrav];
        [self.switchMotionAcc setOn:enableSensorMotionAcc];
        [self.switchMotionMagneticField setOn:enableSensorMotionMagnetic];
    });    
}


- (bool) ChangeSensorEventState : (environs::SensorType_t) type state:(bool) state
{
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                             (unsigned long)NULL), ^(void) {
        
        if ( sensorDataView == nil )
            return;
        
        NSArray * list = [sensorDataView->deviceList GetDevices];
        
        for ( int i=0; i<[list count]; i++ )
        {
            DeviceInstance * device = (DeviceInstance *) [list objectAtIndex:i];
            if ( device ) {
                [device SetSensorEventSending:type enable:state];
            }
        }
    });
    
    return state;
}


- (IBAction) switchAccelerometer:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorAccelerometer = [self ChangeSensorEventState : environs::SensorType::Accelerometer state:[sw isOn]];
}


- (IBAction)switchCompass:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMagneticField = [self ChangeSensorEventState : environs::SensorType::MagneticField state:[sw isOn]];
}

- (IBAction)switchGyro:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorGyroscope = [self ChangeSensorEventState : environs::SensorType::Gyroscope state:[sw isOn]];
}


- (IBAction) switchOrientation:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorOrientation = [self ChangeSensorEventState : environs::SensorType::Orientation state:[sw isOn]];
}


- (IBAction) switchLocation:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorLocation = [self ChangeSensorEventState : environs::SensorType::Location state:[sw isOn]];
}


- (IBAction) switchLight:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorLight = [self ChangeSensorEventState : environs::SensorType::Light state:[sw isOn]];
}


- (IBAction) switchHeading:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorHeading = [self ChangeSensorEventState : environs::SensorType::Heading state:[sw isOn]];
}


- (IBAction) switchAltimeter:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorAltimeter = [self ChangeSensorEventState : environs::SensorType::Pressure state:[sw isOn]];
}


- (IBAction) switchMotionAtt:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMotionAtt = [self ChangeSensorEventState : environs::SensorType::Attitude state:[sw isOn]];
}


- (IBAction) switchMotionRot:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMotionRot = [self ChangeSensorEventState : environs::SensorType::Rotation state:[sw isOn]];
}


- (IBAction) switchMotionGrav:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMotionGrav = [self ChangeSensorEventState : environs::SensorType::Gravity state:[sw isOn]];
}


- (IBAction) switchMotionAcc:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMotionAcc = [self ChangeSensorEventState : environs::SensorType::Acceleration state:[sw isOn]];
}


- (IBAction) switchMotionMagneticField:(id)sender
{
    UISwitch * sw = (UISwitch *)sender;
    enableSensorMotionMagnetic = [self ChangeSensorEventState : environs::SensorType::MagneticFieldMotion state:[sw isOn]];
}

@end
