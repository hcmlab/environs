/**
 * SensorDataView.h
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
#import "Environs.Observer.iOSX.h"
#import "Device.List.iOSX.h"
#import "Device.Instance.iOSX.h"

@interface SensorDataView : UIViewController <UITableViewDelegate, UITableViewDataSource, ListObserver, DeviceObserver>
{
    @public
    DeviceList  *   deviceList;
}

extern bool enableSensorAccelerometer;
extern bool enableSensorMagneticField;
extern bool enableSensorGyroscope;
extern bool enableSensorOrientation;
extern bool enableSensorLocation;
extern bool enableSensorLight;

extern bool enableSensorHeading;
extern bool enableSensorAltimeter;
extern bool enableSensorMotionAtt;
extern bool enableSensorMotionRot;
extern bool enableSensorMotionGrav;
extern bool enableSensorMotionAcc;
extern bool enableSensorMotionMagnetic;

extern SensorDataView         *   sensorDataView;

- (void) InitSensorData;
- (IBAction) UpdateDeviceList: (id)sender;


@end
