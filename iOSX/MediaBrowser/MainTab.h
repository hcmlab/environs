/**
 * MainTab.h
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

#include "FullscreenController.h"

@interface MainTab : UIViewController<UITextFieldDelegate>
{
}


@property (weak, nonatomic) IBOutlet UILabel *labelStatus;
@property (nonatomic,retain) IBOutlet UIButton *buttonStart;
@property (weak, nonatomic) IBOutlet UITextField *textDeviceID;

@property (nonatomic, retain) IBOutlet UIView *viewFullScreen;


- (IBAction) StartEnvirons: (id)sender;
- (IBAction) AddStatusMessage: (const char *) message;

- (IBAction) deviceIDChanged: (id)sender;

- (IBAction) dismissKeyboardOnTap: (id)sender;

- (void) UpdateUI;


@end