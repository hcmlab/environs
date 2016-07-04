//
//  AppDelegate.h
//  LocationMap
//
//  Created by chi-tai on 27.06.16.
//  Copyright © 2016 HCMLab. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Environs.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate, EnvironsObserver, EnvironsSensorObserver>
{
@public
    Environs * env;
}

@property (strong, nonatomic) UIWindow *window;

extern AppDelegate * appDelegate;
extern double lastLatitude;
extern double lastLongitude;
extern float lastLatitudeAcc;
extern float lastLongitudeAcc;
extern double lastAltitude;
extern float lastSpeed;

@end

