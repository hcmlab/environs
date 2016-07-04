//
//  FirstViewController.m
//  LocationMap
//
//  Created by chi-tai on 27.06.16.
//  Copyright © 2016 HCMLab. All rights reserved.
//

#import "FirstViewController.h"
#import "AppDelegate.h"

FirstViewController * mainView = nil;

@interface FirstViewController ()

@end

@implementation FirstViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    mainView = self;
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (IBAction)ButtonStart:(id)sender {

    if ( !appDelegate )
        return;

    Environs * env = appDelegate->env;

    if ( !env )
    {
        env = [ Environs CreateInstance:"Chi-Tai" Area:"Environs" ];
        if ( !env )
            return;

        appDelegate->env = env;

        [env AddObserver:appDelegate];

        [env AddObserverForSensorData:appDelegate];

        [env SetUseMediatorAnonymousLogon:true];

        if ( [env IsSensorAvailable:environs::SensorType::Location] )
            [env SetSensorEvent:environs::SensorType::Location enable:true];
    }

    if ( [env GetStatus] < environs::Status::Started ) {
        [env Start];
    }
    else {
        [env Stop];
    }
}

@end
