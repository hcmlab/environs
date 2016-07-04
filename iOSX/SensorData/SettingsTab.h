/**
 * SettingsTab.h
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
#import <UIKit/UIKit.h>

@interface SettingsTab : UIViewController<UITextFieldDelegate>
{
    
}


@property (weak, nonatomic) IBOutlet UILabel *labelVersionString;


@property (weak, nonatomic) IBOutlet UILabel *labelAccelerometer;
@property (weak, nonatomic) IBOutlet UISwitch *switchAccelerometer;

@property (weak, nonatomic) IBOutlet UILabel *labelCompass;
@property (weak, nonatomic) IBOutlet UISwitch *switchCompass;

@property (weak, nonatomic) IBOutlet UILabel *labelGyro;
@property (weak, nonatomic) IBOutlet UISwitch *switchGyro;

@property (weak, nonatomic) IBOutlet UILabel *labelOrientation;
@property (weak, nonatomic) IBOutlet UISwitch *switchOrientation;

@property (weak, nonatomic) IBOutlet UILabel *labelLocation;
@property (weak, nonatomic) IBOutlet UISwitch *switchLocation;

@property (weak, nonatomic) IBOutlet UILabel *labelLight;
@property (weak, nonatomic) IBOutlet UISwitch *switchLight;

@property (weak, nonatomic) IBOutlet UISwitch *switchHeading;
@property (weak, nonatomic) IBOutlet UISwitch *switchAltimeter;
@property (weak, nonatomic) IBOutlet UISwitch *switchMotionAttRot;
@property (weak, nonatomic) IBOutlet UISwitch *switchMotionGravAcc;
@property (weak, nonatomic) IBOutlet UISwitch *switchMotionMagneticField;

- (IBAction) switchAccelerometer:(id)sender;
- (IBAction) switchCompass:(id)sender;
- (IBAction) switchGyro:(id)sender;
- (IBAction) switchOrientation:(id)sender;
- (IBAction) switchLocation:(id)sender;
- (IBAction) switchLight:(id)sender;

- (IBAction) switchHeading:(id)sender;
- (IBAction) switchAltimeter:(id)sender;
- (IBAction) switchMotionAttRot:(id)sender;
- (IBAction) switchMotionGravAcc:(id)sender;
- (IBAction) switchMotionMagneticField:(id)sender;

@end
