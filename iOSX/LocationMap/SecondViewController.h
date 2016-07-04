//
//  SecondViewController.h
//  LocationMap
//
//  Created by chi-tai on 27.06.16.
//  Copyright © 2016 HCMLab. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Environs.h"

@interface SecondViewController : UIViewController
{

}

extern SecondViewController * secondView;

@property (weak, nonatomic) IBOutlet UILabel *latitudeValue;
@property (weak, nonatomic) IBOutlet UILabel *longitudeValue;
@property (weak, nonatomic) IBOutlet UILabel *latitudeAccValue;
@property (weak, nonatomic) IBOutlet UILabel *longitudeAccValue;
@property (weak, nonatomic) IBOutlet UILabel *altitudeValue;
@property (weak, nonatomic) IBOutlet UILabel *speedValue;

- (void) UpdateSensorData:(environs::SensorFrame *) sensorFrame;

@end

