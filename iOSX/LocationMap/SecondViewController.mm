//
//  SecondViewController.m
//  LocationMap
//
//  Created by chi-tai on 27.06.16.
//  Copyright © 2016 HCMLab. All rights reserved.
//

#import "SecondViewController.h"
#import "AppDelegate.h"

SecondViewController * secondView = nil;

@interface SecondViewController ()

@end

@implementation SecondViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    secondView = self;

    self.latitudeValue.text = [NSString stringWithFormat:@"%lf", lastLatitude];

    self.longitudeValue.text = [NSString stringWithFormat:@"%lf", lastLongitude];

    self.altitudeValue.text = [NSString stringWithFormat:@"%lf", lastAltitude];

    self.speedValue.text = [NSString stringWithFormat:@"%lf", lastSpeed];

    self.latitudeAccValue.text = [NSString stringWithFormat:@"%lf", lastLatitudeAcc];

    self.longitudeAccValue.text = [NSString stringWithFormat:@"%lf", lastLongitudeAcc];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) UpdateSensorData:(environs::SensorFrame *) sensorFrame
{
    if ( sensorFrame->frame.type == environs::SensorType::Location )
    {
        dispatch_async(dispatch_get_main_queue(), ^{

            self.latitudeAccValue.text = [NSString stringWithFormat:@"%lf", sensorFrame->frame.data.floats.f1];

            self.longitudeAccValue.text = [NSString stringWithFormat:@"%lf", sensorFrame->frame.data.floats.f2];

            self.speedValue.text = [NSString stringWithFormat:@"%lf", sensorFrame->frame.data.floats.f3];

            self.latitudeValue.text = [NSString stringWithFormat:@"%.16lf", sensorFrame->frame.doubles.d1];

            self.longitudeValue.text = [NSString stringWithFormat:@"%.16lf", sensorFrame->frame.doubles.d2];

            self.altitudeValue.text = [NSString stringWithFormat:@"%.16lf", sensorFrame->frame.doubles.d3];
        });
    }
}

@end
