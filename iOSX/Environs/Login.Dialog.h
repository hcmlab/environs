/**
 * LoginDialog.h
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
#import <Foundation/Foundation.h>
#import "Environs.iOSX.h"

#ifdef ENVIRONS_IOS
    //******** iOS *************
    #import <UIKit/UIKit.h>


@interface LoginDialog : NSObject<UIAlertViewDelegate>
{
    NSString    *   userName;
    NSString    *   password;
    
    UIAlertView *   dlg;
    
@public
    environs::lib::Environs    *   env;
}

#else
    //******** OSX *************
    #import <Cocoa/Cocoa.h>

@interface LoginDialog : NSObject<NSAlertDelegate>
{
    
@public
    environs::lib::Environs    *   env;
}
#endif



/**
 * Create an instance of the login dialog with the given parameters.
 * The dialog has to invoked/shown using ShowResult.
 *
 * @param message       The message shown within the dialog.
 * @param title         The title of the dialog.
 * @param userName		The username if already known. This may be null.
 * @return              An instance of the login dialog.
 */
+ (LoginDialog *) SingletonInstance: (NSString *) message Title:(NSString *) title UserName:(NSString *) userName;


/**
 * Show the login dialog within a thread and unblock the calling thread.
 *
 * @return  returns always true
 */
- (bool) ShowResult;

@end
