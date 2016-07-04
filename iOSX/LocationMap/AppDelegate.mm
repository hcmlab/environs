//
//  AppDelegate.m
//  LocationMap
//
//  Created by chi-tai on 27.06.16.
//  Copyright © 2016 HCMLab. All rights reserved.
//

#import "AppDelegate.h"
#import "FirstViewController.h"
#import "SecondViewController.h"

#define CLASS_NAME  "AppDelegate. . . . . . ."

AppDelegate * appDelegate = nil;
double lastLatitude = 0.0;
double lastLongitude = 0.0;
float lastLatitudeAcc = 0.0;
float lastLongitudeAcc = 0.0;
double lastAltitude = 0.0;
float lastSpeed = 0.0;

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.

    appDelegate = self;
    env = nil;

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


- (void) OnSensorData:(int) nativeID Frame:(environs::SensorFrame *) sensorFrame
{
    CLogArgN ( "OnSensorData: nativeID [ %i ]", nativeID );

    if ( secondView ) {
        [secondView UpdateSensorData:sensorFrame];
    }
    else {
        lastLatitudeAcc = sensorFrame->frame.data.floats.f1;
        lastLongitudeAcc = sensorFrame->frame.data.floats.f2;

        lastLatitude = sensorFrame->frame.doubles.d1;
        lastLongitude = sensorFrame->frame.doubles.d2;
    }
}


-(void) OnStatus:(environs::Status_t) status
{
    static environs::Status_t lastStatus = environs::Status::Uninitialized;

    if ( !mainView )
        return;

    if ( status == lastStatus )
        return;
    lastStatus = status;

    dispatch_async(dispatch_get_main_queue(), ^{

        mainView.buttonStart.enabled = FALSE;

        if ( status >= environs::Status::Started ) {
            [mainView.buttonStart setTitle:@"Stop" forState:UIControlStateNormal];
        }
        else {
            [mainView.buttonStart setTitle:@"Start" forState:UIControlStateNormal];
        }

        mainView.buttonStart.enabled = TRUE;
        [mainView.buttonStart setNeedsLayout];
    });
}


-(void) OnStatusMessage:(const char *) message
{
}


-(void) OnNotify:(ObserverNotifyContext *)context
{
}


-(void) OnNotifyExt:(ObserverNotifyContext *)context
{
}

@end
