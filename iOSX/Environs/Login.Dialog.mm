/**
 * LoginDialog.mm
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

#import "Login.Dialog.h"
#import "Environs.iOSX.h"
#include "Environs.h"
#include "Environs.Native.h"

#define CLASS_NAME  "Login.Dialog.iOSX. . . ."

@interface LoginDialog ()
{
    
#ifdef ENVIRONS_IOS
    
#else
    NSString    *   title;
    NSString    *   userName;
    NSString    *   password;
    
    NSAlert     *   dlg;
    NSSecureTextField   * passw;
    NSTextField   * user;
    
#endif
    
}
@end


@implementation LoginDialog
{
    bool            passwordFocus;
    NSTimer     *   noActivityTimer;
}

NSObject        *   classLock   = [NSObject alloc];
unsigned int        count       = 0;


/**
 * Create an instance of the login dialog with the given parameters. 
 * The dialog has to invoked/shown using ShowResult.
 *
 * @param message       The message shown within the dialog.
 * @param title         The title of the dialog.
 * @param userName		The username if already known. This may be null.
 * @return              An instance of the login dialog.
 */
+ (LoginDialog *) SingletonInstance: (NSString *) message Title:(NSString *) title UserName:(NSString *) userName
{
    if ( !message || !title )
        return 0;
    
    @synchronized (classLock)
    {
        if (count != 0)
            return 0;
        count = 1;
    }
    
    LoginDialog * instance = [LoginDialog alloc];
    if ( !instance )
        return 0;
    
    instance->userName = userName;
    instance->noActivityTimer = nil;
    
#ifdef ENVIRONS_IOS
    dispatch_sync(dispatch_get_main_queue(), ^{
        
        UIAlertView * dlg = [[UIAlertView alloc]
                                  initWithTitle:title
                                  message:message
                                  delegate:instance
                                  cancelButtonTitle:@"Cancel"
                             otherButtonTitles:@"OK", nil ];
        instance->dlg = dlg;
        
        dlg.alertViewStyle = UIAlertViewStyleLoginAndPasswordInput;
        instance->passwordFocus = false;
        
        UITextField * tfPassword = [dlg textFieldAtIndex:1];
        if ( !tfPassword )
            return;
        tfPassword.placeholder = @"Password";
        
        UITextField * tfUsername = [dlg textFieldAtIndex:0];
        if ( !tfUsername )
            return;
        tfPassword.keyboardType = UIKeyboardTypeDefault;
        
        if ( userName && [userName length] > 3 )
        {
            tfUsername.text = userName;
            
            instance->passwordFocus = true;
            return;
        }
        
        tfUsername.placeholder = @"Username or Email";
        tfUsername.keyboardType = UIKeyboardTypeEmailAddress;
    });
#else
        instance->userName = userName;
        instance->title = title;
#endif
    
    return instance;
}


#ifdef ENVIRONS_OSX
- (void) BuildUserNameDlg
{
    NSRect              rect;
    NSAlert             * alert =    0;
    NSTextField         * tfUserName = nil;
    
    tfUserName =   [[NSTextField alloc] initWithFrame:NSMakeRect(0,0,350,20)];
    [tfUserName sizeToFit];
    
    rect =          [tfUserName frame];
    [tfUserName setFrameSize:(NSSize){300,rect.size.height}];
    
    alert = [NSAlert alertWithMessageText:title
                          defaultButton:@"OK"
                        alternateButton:@"Cancel"
                            otherButton:nil
              informativeTextWithFormat:@"Please enter the username"];
    
    [alert setAccessoryView:tfUserName];
    [alert layout];
    [[alert window] setInitialFirstResponder:tfUserName];
    
    dlg = alert;
    user = tfUserName;
}


- (void) BuildPasswordDlg
{
    NSRect              rect;
    NSAlert             * alert =    0;
    NSSecureTextField   * tfPassw = nil;
    
    tfPassw =   [[NSSecureTextField alloc] initWithFrame:NSMakeRect(0,0,350,20)];
    [tfPassw sizeToFit];
    
    rect =          [tfPassw frame];
    [tfPassw setFrameSize:(NSSize){300,rect.size.height}];
    
    alert = [NSAlert alertWithMessageText:title
                          defaultButton:@"OK"
                        alternateButton:@"Cancel"
                            otherButton:nil
              informativeTextWithFormat:@"Please enter the password"];
    
    [alert setAccessoryView:tfPassw];
    [alert layout];
    [[alert window] setInitialFirstResponder:tfPassw];
    
    dlg = alert;
    passw = tfPassw;
}

#endif

/**
 * Reset the static login dialog instance.
 *
 */
+ (void) ResetClose
{
    @synchronized (classLock)
    {
        count = 0;
    }
}


/**
 * Show the login dialog within a thread and unblock the calling thread.
 *
 * @return  returns always true
 */
- (bool) ShowResult
{
    dispatch_queue_t queue = dispatch_get_global_queue ( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
    dispatch_async ( queue, ^(void) {
        [self ShowResultThreaded];
    });
    
    return true;
}


/**
 * The thread instance that shows the login dialog.
 * Start a no activity timer before showing the dialog.
 *
 * @return  returns always true
 */
- (bool) ShowResultThreaded
{
    [self ReScheduleTimer];
    
    dispatch_sync(dispatch_get_main_queue(), ^{
#ifdef ENVIRONS_IOS
        [self->dlg show];
#else
        NSInteger retVal;
        
        if ( userName == nil || [userName length] <= 0 || [userName hasPrefix:@"anonymou"]) {
            [self BuildUserNameDlg];
            
            retVal = [self->dlg runModal];
            if (retVal != NSAlertDefaultReturn)
                return;
            
            userName = [self->user stringValue];
            if ( [userName length] <= 0 )
                return;
            
            env->SetUserName ( [userName UTF8String] );
        }
        
        [self BuildPasswordDlg];
        
        retVal = [self->dlg runModal];
        if (retVal != NSAlertDefaultReturn)
            return;
        
        NSString * passwd = [self->passw stringValue];
        
        if ( passwd && passwd.length > 1 )
        {
            env->SetMediatorPassword ( [passwd UTF8String] );
            
            env->RegisterAtMediators ();
            return;
        }
#endif
    });
    return true;
}


/**
 * Dispose the no activity timer.
 *
 */
- (void) DisposeTimer
{
    CVerb ( "DisposeTimer" );
    
    if ( self->noActivityTimer ) {
        [self->noActivityTimer invalidate];
        self->noActivityTimer = nil;
    }
}


/**
 * Schedules a no activity timer that fires after the seconds declared by ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT.
 * Dispose a timer if a timer has been invoked before.
 *
 */
- (void) ReScheduleTimer
{
    CVerb ( "ReScheduleTimout" );
    
    [self DisposeTimer];
    
    self->noActivityTimer = [NSTimer timerWithTimeInterval:ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT target:self selector:@selector(noActivityTimeout) userInfo:nil repeats:NO];
    
    [[NSRunLoop mainRunLoop] addTimer:self->noActivityTimer forMode:NSDefaultRunLoopMode];
}


/**
 * The no activity timeout is fired after the seconds declared by ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT has passed.
 * In such a case, we reschedule the no activity timer.
 *
 */
-(void) noActivityTimeout
{
    CVerb ( "noActivityTimeout" );
    
    [self DisposeTimer];
    
#ifdef ENVIRONS_IOS
    [dlg dismissWithClickedButtonIndex:0 animated:YES];
#else
    //[dlg dismissWithClickedButtonIndex:0 animated:YES];
#endif
}


#ifdef ENVIRONS_IOS

/**
 * This selector is called on user input, e.g. input a character.
 * In such a case, we reschedule the no activity timer.
 *
 * @return  returns always true
 */
- (BOOL) alertViewShouldEnableFirstOtherButton:(UIAlertView *)alertView
{
    CVerb ( "alertViewShouldEnableFirstOtherButton" );
    
    [self ReScheduleTimer];
    
    return true;
}


/**
 * If the Ok button has been pressed, then username and password are passed to Environs
 * and registration of Mediators are requested.
 * If the Cancel button has been pressed, then custom and default mediator are disabled
 * and not used anymore until explicitly enabled again.
 *
 * @param   alertView   The alertview instance.
 * @param   buttonIndex The button index that has been pressed.
 */
- (void) alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    [self DisposeTimer];
    
    [LoginDialog ResetClose];
    
    if ( buttonIndex == 1 )
    {
        UITextField * textField = [alertView textFieldAtIndex:0];
        if ( textField && textField.text && textField.text.length > 3 )
        {
            env->SetMediatorUserName ( [textField.text UTF8String] );
            
            textField = [alertView textFieldAtIndex:1];
            if ( textField && textField.text && textField.text.length > 1 )
            {
                env->SetMediatorPassword ( [textField.text UTF8String] );
                
                env->RegisterAtMediators ();
                return;
            }
        }
    }
    
    if ( env->GetMediatorLoginDialogDismissDisable () ) {
        env->SetUseCustomMediator ( false );
        env->SetUseDefaultMediator ( false );
    }
}


/**
 * This method is called if the dialog is presented and sets the focus on the password or username respectively.
 *
 * @param   alertView   The alertview instance.
 */
- (void) didPresentAlertView:(UIAlertView *)alertView
{
    if ( passwordFocus )
        [[alertView textFieldAtIndex:1] becomeFirstResponder];
    
}
#endif

@end
