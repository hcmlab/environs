/**
 * FullscreenController.h
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

@interface FullscreenController : UIViewController
{
}

+ (void) SetPortalInstance:(id)portalSource;

@property UIViewController *  parentController;

@property (strong, nonatomic) IBOutlet UIView *displayView;
@property (weak, nonatomic) IBOutlet UIButton *buttonMain;

@property (weak, nonatomic) IBOutlet UIButton *buttonRight;
@property (weak, nonatomic) IBOutlet UIButton *buttonLeft;
@property (weak, nonatomic) IBOutlet UIButton *buttonTop;
@property (weak, nonatomic) IBOutlet UIButton *buttonBottom;

+ (FullscreenController *) getInstance;
+ (void) resetInstance;

- (void) close;
- (IBAction) buttonRight:(id)sender;
- (IBAction) buttonLeft:(id)sender;
- (IBAction) buttonTop:(id)sender;
- (IBAction) buttonBottom:(id)sender;

@end
