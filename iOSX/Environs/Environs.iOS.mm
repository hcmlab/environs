/**
 * Environs.ios.mm
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifdef ENVIRONS_IOS

#import <UIKit/UIKit.h>
#import <Environs.iOS.h>
#import <Environs.iOSX.h>
#import "Touch.Listener.h"
#import <SystemConfiguration/CaptiveNetwork.h>
#import <CoreMotion/CoreMotion.h>
#import <CoreLocation/CoreLocation.h>
#import <NetworkExtension/NetworkExtension.h>

#ifdef ENABLE_IOS_HEALTHKIT_SUPPORT
#   import <HealthKit/HealthKit.h>
#endif

#include "Environs.Obj.h"
#include "Wifi.Observer.h"
using namespace environs;

#include "Environs.Lib.h"
#include "Environs.Sensors.h"
using namespace environs::API;

#include "Core/Byte.Buffer.h"

#include <sys/types.h>
#include <sys/sysctl.h>

#define	CLASS_NAME 	"Environs.iOS . . . . . ."


@interface iOSXSensors : NSObject<CLLocationManagerDelegate>
{
}

@end

@interface iOSXSensors ()
{
}

@end



namespace environs
{
    double                   environsTouchXFactor   = 1;
    double                   environsTouchYFactor   = 1;
    
    static CMMotionManager      *   motionManager   = nil;
    
    static iOSXSensors          *   iOSXSensor      = nil;
    static CLLocationManager    *   locationManager = nil;
    
    static CMAltimeter          *   altimeter       = nil;
    
    extern bool CreateAppID ( char * buffer, unsigned int bufSize );
    extern bool AllocNativePlatformIOSX ();

    
    bool AllocNativePlatform ()
    {
        CVerb ( "AllocNativePlatform" );
        
        /// Parse the identifier here for the DeviceUID
        
        if ( !*native.deviceUID ) {
            CreateAppID ( native.deviceUID, sizeof ( native.deviceUID ) );
            
            SetDeviceUIDN ( native.deviceUID );
        }

        if ( !AllocNativePlatformIOSX () )
            return false;
        
        return AllocNativePlatformMobile ();
    }
    
    
    void DeAllocNativePlatform ()
    {
        CVerb ( "DeAllocNativePlatform" );
        
        StopSensorListeningN ( 0, (environs::SensorType_t) -1 );
    }
    
    
    namespace API
    {
        NSString * GetNSSID ( bool desc )
        {
            NSArray * iNames = CFBridgingRelease ( CNCopySupportedInterfaces () );
            
            if ( iNames )
            {
                for ( NSString * name in iNames )
                {
                    NSDictionary * infos = CFBridgingRelease ( CNCopyCurrentNetworkInfo ( (__bridge CFStringRef) name ) );
                    
                    if ( infos && infos.count > 0 )
                    {
                        @autoreleasepool {
                            NSString * bssid = infos[@"BSSID"];
                            if ( bssid )
                            {
                                const char * _bssid = [bssid UTF8String];
                                if ( _bssid )
                                {
                                    MediatorClient::wifiCurrentBSSID = environs::GetBSSIDFromColonMac ( _bssid );
                                }
                            }
                        }

                        return [[NSString alloc ] initWithString:infos[@"SSID"]];
                    }
                }
            }
            return nil;
        }
        
        const char * GetSSID ( bool desc )
        {
            NSString * ssid = GetNSSID ( desc );
            
            if ( ssid != nil )
            {                
                static char pssid [64];
                
                if ( snprintf ( pssid, sizeof(pssid), "%s", [ssid UTF8String] ) > 0 )
                    return pssid;
            }
            return "";
        }
        
        
        
        bool CreateMotionManager ()
        {
            if ( motionManager )
                return true;
            
            dispatch_sync(dispatch_get_main_queue(), ^ {
                motionManager = [[CMMotionManager alloc] init];
            });
            
            if ( motionManager ) {
                motionManager.accelerometerUpdateInterval   = .2;
                motionManager.gyroUpdateInterval            = .2;
                motionManager.magnetometerUpdateInterval    = .2;
                motionManager.deviceMotionUpdateInterval    = .2;
                return true;
            }
            return false;
        }
        
        
        bool CreateLocationManager ( int hInst )
        {
            if ( !iOSXSensor ) {
                iOSXSensor = [[iOSXSensors alloc] init];
                if ( !iOSXSensor )
                    return false;
            }
            
            if ( locationManager )
                return true;
            
            dispatch_sync(dispatch_get_main_queue(), ^ {
                locationManager = [[CLLocationManager alloc] init];
            });
            
            if ( locationManager ) {
                locationManager.delegate = iOSXSensor;
                locationManager.headingFilter = 0.1; // 1 degrees
                locationManager.distanceFilter = 0.1;
                return true;
            }
            
            return false;
        }
        
        
        bool CreateAltimeter ()
        {
            if ( altimeter )
                return true;
            
            dispatch_sync(dispatch_get_main_queue(), ^ {
                altimeter = [[CMAltimeter alloc] init];
            });
            
            if ( altimeter ) {
                return true;
            }
            return false;
        }
        
        
        bool CreateAmbientLightManager ()
        {
            return false;
        }
        
        
        double GetHeartRate ()
        {
#ifdef ENABLE_IOS_HEALTHKIT_SUPPORT
            HKUnit * unit = [[HKUnit countUnit] unitDividedByUnit:[HKUnit minuteUnit]];
            
            HKQuantity * quantity = [HKQuantity quantityWithUnit:unit doubleValue:100];
            
            HKQuantityType * type = [HKQuantityType quantityTypeForIdentifier : HKQuantityTypeIdentifierHeartRate];
            
            NSDate * now = [NSDate date];
            
            HKQuantitySample * sample = [HKQuantitySample quantitySampleWithType:type quantity:quantity startDate:now endDate:now];
            
            double rate = [[sample quantity] doubleValueForUnit:unit];
            
            return rate;
#else
            return 0;
#endif
        }
        
        
        void onAccelerometerChanged ( CMAcceleration sensorData )
        {
            if ( !native.coresStarted )
                return;
            
            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_ACCELEROMETER )
                PushSensorDataN ( ENVIRONS_SENSOR_TYPE_ACCELEROMETER, sensorData.x, sensorData.y, sensorData.z ); // x, y, z
        }
        
        
        
        void onMagnetometerChanged ( CMMagneticField sensorData )
        {
            if ( !native.coresStarted )
                return;

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_MAGNETICFIELD )
                PushSensorDataN ( ENVIRONS_SENSOR_TYPE_MAGNETICFIELD, sensorData.x, sensorData.y, sensorData.z ); // x, y, z
        }
        
        
        
        void onGyroChanged ( CMRotationRate sensorData )
        {
            if ( !native.coresStarted )
                return;

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_GYROSCOPE )
                PushSensorDataN ( ENVIRONS_SENSOR_TYPE_GYROSCOPE, sensorData.x, sensorData.y, sensorData.z ); // x, y, z
        }
        
        
        
        void onAltimeterChanged ( CMAltitudeData * sensorData )
        {
            if ( !native.coresStarted )
                return;

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_PRESSURE )
                PushSensorDataN ( ENVIRONS_SENSOR_FLAG_PRESSURE, sensorData.pressure.doubleValue, sensorData.relativeAltitude.doubleValue, 0); // x, y, z
        }
        
        
        
        void onDeviceMotionChanged ( CMDeviceMotion * sensorData )
        {
            if ( !native.coresStarted )
                return;

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_ATTITUDE )
                PushSensorDataDoublesN ( ENVIRONS_SENSOR_FLAG_ATTITUDE, sensorData.attitude.roll, sensorData.attitude.pitch, sensorData.attitude.yaw );

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_GRAVITY )
                PushSensorDataDoublesN ( ENVIRONS_SENSOR_FLAG_GRAVITY, sensorData.gravity.x, sensorData.gravity.y, sensorData.gravity.z );

            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_ROTATION )
                PushSensorDataExtN ( ENVIRONS_SENSOR_FLAG_ROTATION, 0, 0, 0, sensorData.rotationRate.x, sensorData.rotationRate.y, sensorData.rotationRate.z );
            
            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_ACCELERATION )
                PushSensorDataDoublesN ( ENVIRONS_SENSOR_FLAG_ACCELERATION, sensorData.userAcceleration.x, sensorData.userAcceleration.y, sensorData.userAcceleration.z );
            
            if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_MAGNETICFIELD_MOTION )
                PushSensorDataN ( ENVIRONS_SENSOR_FLAG_MAGNETICFIELD_MOTION, sensorData.magneticField.field.x, sensorData.magneticField.field.y, sensorData.magneticField.field.z );
        }
        
        
        double lastLongitude    = 0;
        double lastLatitude     = 0;
        double lastAltitude     = 0;
        double lastSpeed        = 0;
        
        double lastHorizontalAccuracy   = 0;
        double lastVerticalAccuracy     = 0;

        map<int,int> sensorLocationInstances;
        
        
        void onLocationChanged ( CLLocation * sensorData, bool forceSend )
        {
            if ( !native.coresStarted )
                return;
            
            if ( forceSend || sensorRegistered & ENVIRONS_SENSOR_FLAG_LOCATION )
            {                
                lastLatitude    = sensorData.coordinate.latitude;
                lastLongitude   = sensorData.coordinate.longitude;
                
                lastAltitude    = sensorData.altitude;
                lastSpeed       = sensorData.speed;
                
                lastHorizontalAccuracy  = sensorData.horizontalAccuracy;
                lastVerticalAccuracy    = sensorData.verticalAccuracy;

                PushSensorDataExtN ( ENVIRONS_SENSOR_TYPE_LOCATION, lastLatitude, lastLongitude, lastAltitude, lastHorizontalAccuracy, lastVerticalAccuracy, lastSpeed );
            }
        }
        
        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailableImpl ( int hInst, environs::SensorType_t sensorType )
        {
            CVerbArg ( "IsSensorAvailableImpl: type [ %i ]", sensorType );
            
            bool available = false;

            switch ( sensorType )
            {
                case SensorType::Accelerometer:
                    if ( !CreateMotionManager () )
                        break;
                    
                    available = motionManager.accelerometerAvailable;
                    break;
                    
                case SensorType::MagneticField :
                    if ( !CreateMotionManager () )
                        break;
                    
                    available = motionManager.magnetometerAvailable;
                    break;
                    
                case SensorType::Gyroscope :
                    if ( !CreateMotionManager () )
                        break;
                    
                    available = motionManager.gyroAvailable;
                    break;

                case SensorType::Attitude :
                case SensorType::Rotation :
                case SensorType::Gravity :
                case SensorType::Acceleration :
                case SensorType::MagneticFieldMotion :
                    if ( !CreateMotionManager () )
                        break;
                    
                    available = motionManager.deviceMotionAvailable;
                    break;
                    
                case SensorType::Pressure :
                    available = [CMAltimeter isRelativeAltitudeAvailable];
                    break;
                    
                case SensorType::Heading :
                    available = [CLLocationManager headingAvailable];
                    break;
                    
                case SensorType::Location :
                    available = [CLLocationManager locationServicesEnabled];
                    break;
                    
                default:
                    break;
            }
            
            return available;
        }
        
        
        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param sensorType A value of type environs::SensorType_t.
         *
         */
        bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            CVerbArg ( "StartSensorListeningImpl:\tType [ %i : %s ]", sensorType, sensorName );
            
            bool registered = false;
            
            switch ( sensorType )
            {
                case SensorType::Accelerometer:
                    if ( !CreateMotionManager () )
                        break;
                    
                    [motionManager startAccelerometerUpdatesToQueue:[[NSOperationQueue alloc] init]
                                                        withHandler:^(CMAccelerometerData  *sensorData, NSError * error) {
                                                            onAccelerometerChanged ( sensorData.acceleration );
                                                        }];
                    registered = true;
                    break;
                    
                case SensorType::MagneticField :
                    if ( !CreateMotionManager () )
                        break;
                    
                    [motionManager startMagnetometerUpdatesToQueue:[[NSOperationQueue alloc] init]
                                                       withHandler:^(CMMagnetometerData *sensorData, NSError * error) {
                                                           onMagnetometerChanged ( sensorData.magneticField );
                                                       }];
                    registered = true;
                    break;
                    
                case SensorType::Gyroscope :
                    if ( !CreateMotionManager () )
                        break;
                    
                    [motionManager startGyroUpdatesToQueue:[[NSOperationQueue alloc] init]
                                               withHandler:^(CMGyroData *sensorData, NSError *error) {
                                                   onGyroChanged ( sensorData.rotationRate );
                                               }];
                    registered = true;
                    break;
                    
                case SensorType::Attitude :
                case SensorType::Rotation :
                case SensorType::Gravity :
                case SensorType::Acceleration :
                case SensorType::MagneticFieldMotion :
                    if ( !CreateMotionManager () )
                        break;
                    
                    if ( !motionManager.deviceMotionActive )
                        [motionManager startDeviceMotionUpdatesToQueue:[[NSOperationQueue alloc] init]
                                               withHandler:^(CMDeviceMotion * sensorData, NSError * error) {
                                                   onDeviceMotionChanged ( sensorData );
                                               }];
                    registered = true;
                    break;
                    
                case SensorType::Pressure :
                    if ( !CreateAltimeter () )
                        break;
                    
                    [altimeter startRelativeAltitudeUpdatesToQueue:[[NSOperationQueue alloc] init]
                                                       withHandler:^(CMAltitudeData * sensorData, NSError * error) {
                                                           onAltimeterChanged ( sensorData );
                                                       }];
                    registered = true;
                    break;
                    
                case SensorType::Heading :
                    if ( !CreateLocationManager ( hInst ) )
                        break;
                    
                    if ( [CLLocationManager headingAvailable] ) {
                        
                        if ( [CLLocationManager authorizationStatus] < kCLAuthorizationStatusAuthorizedAlways && [locationManager respondsToSelector:@selector(requestWhenInUseAuthorization)] )
                        {
                            [locationManager requestWhenInUseAuthorization];
                        }
                        [locationManager startUpdatingHeading];
                        
                        [locationManager requestLocation];
                        
                        CLHeading * heading = locationManager.heading;
                        if ( heading != nil ) {
                            PushSensorDataN ( ENVIRONS_SENSOR_TYPE_HEADING, heading.x, heading.y, heading.z );
                        }
                        registered = true;
                    }
                    
                    break;
                    
                case SensorType::Location :
                    if ( !CreateLocationManager ( hInst ) )
                        break;
                    
                    if ( [CLLocationManager locationServicesEnabled] ) {
                        
                        if ( [CLLocationManager authorizationStatus] < kCLAuthorizationStatusAuthorizedAlways && [locationManager respondsToSelector:@selector(requestWhenInUseAuthorization)] )
                        {
                            [locationManager requestWhenInUseAuthorization];
                        }
                        
                        [locationManager startUpdatingLocation];
                        
                        [locationManager requestLocation];
                        
                        CLLocation * currentLocation = locationManager.location;
                        
                        if ( currentLocation != nil ) {
                            onLocationChanged ( currentLocation, true );
                        }
                        
                        registered = true;
                    }
                    
                    break;
                    
                default:
                    break;
            }
            
            return registered;
        }
        
        
        /**
         * Deregister to sensor events and stop listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param sensorType A value of type environs::SensorType_t.
         *
         */
        void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            CVerbArg ( "StopSensorListeningImpl:\tType [ %i : %s ]", sensorType, sensorName );
            
            if ( sensorType == -1 ) {
                sensorRegistered = 0;
                
                if ( motionManager ) {
                    [motionManager stopAccelerometerUpdates];
                    [motionManager stopMagnetometerUpdates];
                    [motionManager stopGyroUpdates];
                    [motionManager stopDeviceMotionUpdates];
                }
                
                if ( locationManager ) {
                    [locationManager stopUpdatingLocation];
                    
                    [locationManager stopUpdatingHeading];
                }
                
                if ( altimeter )
                    [altimeter stopRelativeAltitudeUpdates];

                return;
            }

            int sumFlags;

            switch ( sensorType )
            {
                case SensorType::Accelerometer:
                    if ( motionManager )
                        [motionManager stopAccelerometerUpdates];
                    break;
                    
                case SensorType::MagneticField :
                    if ( motionManager )
                        [motionManager stopMagnetometerUpdates];
                    break;
                    
                case SensorType::Gyroscope :
                    if ( motionManager )
                        [motionManager stopGyroUpdates];
                    break;
                    
                case SensorType::Attitude :
                    sumFlags = SensorType::Rotation | SensorType::Gravity | SensorType::Acceleration | SensorType::MagneticFieldMotion;

                    if ( sensorRegistered & sumFlags ) {
                        break;
                    }
                    goto DisableDeviceMotion;

                case SensorType::Rotation :
                    sumFlags = SensorType::Attitude | SensorType::Gravity | SensorType::Acceleration | SensorType::MagneticFieldMotion;

                    if ( sensorRegistered & sumFlags ) {
                        break;
                    }
                    goto DisableDeviceMotion;
                    
                case SensorType::Gravity :
                    sumFlags = SensorType::Attitude | SensorType::Rotation | SensorType::Acceleration | SensorType::MagneticFieldMotion;

                    if ( sensorRegistered & sumFlags ) {
                        break;
                    }
                    goto DisableDeviceMotion;

                case SensorType::Acceleration :
                    sumFlags = SensorType::Attitude | SensorType::Rotation | SensorType::Gravity | SensorType::MagneticFieldMotion;

                    if ( sensorRegistered & sumFlags ) {
                        break;
                    }
                    goto DisableDeviceMotion;
                    
                case SensorType::MagneticFieldMotion :
                    sumFlags = SensorType::Attitude | SensorType::Rotation | SensorType::Gravity | SensorType::Acceleration;

                    if ( sensorRegistered & sumFlags ) {
                        break;
                    }
                DisableDeviceMotion:
                    if ( motionManager )
                        [motionManager stopDeviceMotionUpdates];
                    break;
                    
                case SensorType::Pressure :
                    if ( altimeter )
                        [altimeter stopRelativeAltitudeUpdates];
                    break;
                    
                case SensorType::Heading :
                    if ( locationManager )
                        [locationManager stopUpdatingHeading];
                    break;
                    
                case SensorType::Location :
                    if ( locationManager )
                        [locationManager stopUpdatingLocation];
                    break;
                    
                default:
                    break;
            }
        }
        
    }
    
    /**
     * Perform SDK checks to detect ...
     *
     */
    void DetectSDKs ( )
    {
        if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_7_1) {
            native.sdks = 8;
        }
        else if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_6_1) {
                native.sdks = 7;
            }
        else
            native.sdks = 6;
    }
    
    
    /**
     * Perform platform checks to detect ...
     *
     */
    void DetectPlatform ( )
    {
        if ( native.platform != Platforms::Unknown )
            return;
        
        int     platform        = 0;
        
        float   width_mm        = 60;
        float   height_mm       = 110;
        int     device_width    = 768;
        int     device_height   = 1024;
        
        
        if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad ) {
            platform = Platforms::iPad_Flag | Platforms::Tablet_Flag;
            
            size_t size;
            sysctlbyname ( "hw.machine", NULL, &size, NULL, 0 );
            
            if ( !size ) {
                return;
            }
            
            char * pcStr = (char *) malloc(size);
            if ( !pcStr )
                return;
            
            sysctlbyname ( "hw.machine", pcStr, &size, NULL, 0);
            
            NSString * pStr = [[NSString alloc ] initWithCString:pcStr encoding:NSUTF8StringEncoding];
            free(pcStr);
            
            /// iPad dimensions need to be corrected. Those are only templates.
            if ([pStr hasPrefix:@"iPad1"]) {
                platform |= Platforms::iPad1;
                native.display.dpi = 132.5f;
                
                device_width = 1024;
                device_height = 768;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad2,5"] || [pStr hasPrefix:@"iPad2,6"] || [pStr hasPrefix:@"iPad2,7"]) {
                platform |= Platforms::iPad2Mini;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad2"] ) {
                platform |= Platforms::iPad2;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad3,1"] || [pStr hasPrefix:@"iPad3,2"] || [pStr hasPrefix:@"iPad3,2"]) {
                platform |= Platforms::iPad3;
                native.display.dpi = 264;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad3"] ) {
                platform |= Platforms::iPad4;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad4,1"] || [pStr hasPrefix:@"iPad4,2"]) {
                platform |= Platforms::iPad4Air;
                
				/// 3 times (hcm-lab)
                device_width = 2048;
                device_height = 1536;
                width_mm = 147;
                height_mm = 197;
            }
            else if ([pStr hasPrefix:@"iPad4,4"] || [pStr hasPrefix:@"iPad4,5"]) {
                platform |= Platforms::iPad4Mini;
                native.display.dpi = 326;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad4"]) {
                platform |= Platforms::iPad4Mini3;
                native.display.dpi = 326;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            else if ([pStr hasPrefix:@"iPad5,3"] || [pStr hasPrefix:@"iPad5,4"]) {
                platform |= Platforms::iPad5Air2;
                native.display.dpi = 264;
                
                device_width = 2048;
                device_height = 1536;
                width_mm = 69;
                height_mm = 121;
            }
            
        } else if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone ) {
            platform = Platforms::iPhone_Flag | Platforms::Smartphone_Flag;            
            
            UIScreen * mainScreen = [UIScreen mainScreen];
            if ( !mainScreen )
                return;
            
            int height = [mainScreen bounds].size.height;
            
            if ( height == 568 ) {
                platform |= Platforms::iPhone5;
                native.display.dpi = 326;
                
                device_width = 640;
                device_height = 1136;
                width_mm = 52;
                height_mm = 90;
            }
            else if ( height == 667 ) {
                platform |= Platforms::iPhone6;
                native.display.dpi = 326;
                
                device_width = 750;
                device_height = 1334;
                width_mm = 58.4;
                height_mm = 103.8;
                
            }
            else if ( height == 736 ) {
                platform |= Platforms::iPhone6p;
                native.display.dpi = 401;
                
                device_width = 1080;
                device_height = 1920;
                width_mm = 69;
                height_mm = 121;
            }
            else {
                platform |= Platforms::iPhone4;
                native.display.dpi = 326;
                
                device_width = 640;
                device_height = 960;
                width_mm = 52;
                height_mm = 90;
            }
        }
        
        native.display.orientation = DISPLAY_ORIENTATION_PORTRAIT;
        
        native.platform = (environs::Platforms_t) platform;
        
        SetDeviceDimsN ( device_width, device_height, width_mm, height_mm, 0, 0 );
        
        CVerbArg("DetectPlatform: determined width [%i] px, height [%i] px, width_mm [%f], height_mm [%f]", device_width, device_height, width_mm, height_mm);
    }
    
    
    bool DetermineAndInitWorkDir ()
    {
        bool success = false;
        
        // Get path to Documents
        NSArray *paths = NSSearchPathForDirectoriesInDomains ( NSDocumentDirectory, NSUserDomainMask, YES );
        if ( paths )
        {
            NSString *dataStorePath = [paths objectAtIndex:0];
            if ( dataStorePath )
            {
                CVerbArg ( "DetermineAndInitWorkDir: storing data store path [%s]", [dataStorePath UTF8String] );
                
                InitStorageN ( [dataStorePath UTF8String] );
                InitWorkDirN ( [dataStorePath UTF8String] );
            }
            else {
                CErr ( "DetermineAndInitWorkDir: Failed to get path to documents" );
            }
        }
        else {
            CErr ( "DetermineAndInitWorkDir: Failed to search path to documents" );
        }
        
        return success;
    }
    
    
    void InitIOSX ()
    {
        CVerb ( "InitIOSX" );
        
        // TODO Do this only if we are visible. disable when appropriate
        [UIApplication sharedApplication].idleTimerDisabled = YES;
        
        /*NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
        if ( !prefs ) {
            CErr ( "InitIOSX: User defaults not accessible!" );
            SetDeviceID1 ( hInst, ENVIRONS_DEBUG_TAGID ); // Set to debug tagID
        }
         */
        
        NSString * deviceName = [[UIDevice currentDevice] name];
        
        if ( deviceName && [deviceName length] > 0 ) {
            SetDeviceNameN ( [deviceName UTF8String] );
        }
        
        CreateRandomUUIDTemplate ();        
        
        if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)])
        {
            [[UIApplication sharedApplication] registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:(UIUserNotificationTypeBadge|UIUserNotificationTypeAlert|UIUserNotificationTypeSound) categories:nil]];
        }
    }
    
    
    void UpdateDeviceParams ()
    {
        CVerb ( "UpdateDeviceParams" );
    }
    
    
    void SetRenderSurfaceIOSX ( void * surface )
    {
        CVerb ( "SetRenderSurfaceIOSX" );
    }
    
    
    OSStatus EncryptMessageX ( SecKeyRef publicKey, unsigned int certProp, char * msg, size_t msgLen, char * ciphers, size_t *ciphersLen )
    {
        CVerb ( "EncryptMessageX" );
        
        SecPadding pad = 0;
        if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
            pad = kSecPaddingOAEP;
        else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1 )
            pad = kSecPaddingPKCS1;
        else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1SHA1 )
            pad = kSecPaddingPKCS1SHA1;
        else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1SHA256 )
            pad = kSecPaddingPKCS1SHA256;
        else
            pad = kSecPaddingNone;
        
        return SecKeyEncrypt ( publicKey, pad, (uint8_t *) msg, msgLen, (uint8_t *) ciphers, ciphersLen);
    }
    
    
    bool RenderDecoderToSurface ( void * _surface, void * decoderOrByteBuffer )
    {
        CVerbVerb ( "RenderDecoderToSurface" );
        
        IPortalDecoder * decoder = (IPortalDecoder *) decoderOrByteBuffer;
        
        UIView * surface = (__bridge UIView *) _surface;
        
        bool ret = false;
        
        unsigned int        width;
        unsigned int        height;
        char            *   rawdata;
        
        if ( decoder->avContextType == DECODER_AVCONTEXT_TYPE_PIXELS ) {
            width = decoder->width;
            height = decoder->height;
            rawdata = (char *)decoder->avContext;
        }
        else
            return false;
                
        CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
        
        CFDataRef data = CFDataCreateWithBytesNoCopy ( kCFAllocatorDefault,
                                                      (const UInt8 *)rawdata,
                                                      width * height,
                                                      kCFAllocatorNull );
        
        CGDataProviderRef provider = CGDataProviderCreateWithCFData ( data );
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB ( );
        
        CGImageRef cgImage = CGImageCreate ( width,
                                            height,
                                            8,
                                            24,
                                            width * 3,
                                            colorSpace,
                                            bitmapInfo,
                                            provider,
                                            NULL,
                                            NO,
                                            kCGRenderingIntentDefault );
        CGColorSpaceRelease ( colorSpace );
        
        UIImage * image = [UIImage imageWithCGImage:cgImage];
        CGImageRelease ( cgImage );
        CGDataProviderRelease ( provider );
        CFRelease ( data );
        
        
        try {
            @autoreleasepool
            {
                UIGraphicsBeginImageContext ( surface.frame.size );
                //CGContextRef context = UIGraphicsGetCurrentContext ();
                
                [image drawInRect:surface.bounds];
                UIImage *imageStretch = UIGraphicsGetImageFromCurrentImageContext ( );
                
                //CGContextFlush(context);
                UIGraphicsEndImageContext ( );
                
                image = 0;
                
                if ( imageStretch ) {
                    dispatch_sync ( dispatch_get_main_queue ( ), ^ {
                        surface.backgroundColor = [[UIColor alloc] initWithPatternImage:imageStretch];
                    } );
                    
                    imageStretch = 0;
                    ret = true;
                }
            }
        }
        catch ( NSException * ie ) {
            NSLog ( @"RenderDecoderToSurface: Exception: %@", ie );
        }
        
        return ret;
    }
    
    
    bool RenderImageToSurface ( void * _surface, void * byteBuffer )
    {
        CVerbVerb ( "RenderImageToSurface" );
        
        UIView * surface = (__bridge UIView *) _surface;
        if ( surface == nil )
            return false;
        
        ByteBuffer * buffer = (ByteBuffer *)byteBuffer;
        
        try {
            UIImage *image = [UIImage imageWithData:[NSData dataWithBytes: (BYTEBUFFER_DATA_POINTER_START ( buffer) + 4) length:(buffer->payloadSize - 4)]];
            
            if (image)
            {
                @autoreleasepool
                {
                    UIGraphicsBeginImageContext(surface.frame.size);
                    //CGContextRef context = UIGraphicsGetCurrentContext();
                    
                    [image drawInRect:surface.bounds];
                    UIImage *imageStretch = UIGraphicsGetImageFromCurrentImageContext();
                    
                    //CGContextFlush(context);
                    UIGraphicsEndImageContext();
                    
                    if (imageStretch) {
                        dispatch_sync(dispatch_get_main_queue(), ^{
                            surface.backgroundColor = [[UIColor alloc] initWithPatternImage:imageStretch];
                        });
                        imageStretch = 0;
                    }
                }
            }
            /*else
                [listener onImage:image];
            */
            
        } catch (NSException * ie) {
            NSLog(@"StreamReceiver: Exception: %@", ie);
        }
        return false;
    }


#ifdef NATIVE_WIFI_OBSERVER_THREAD

    extern ThreadSync	wifiThread;

    void * Thread_WifiObserver ()
    {
        CLog ( "WifiObserver: Created ..." );
        
        
        CLog ( "WifiObserver: bye bye ..." );
        return 0;
    }
#endif
}




@implementation iOSXSensors

- (void) locationManager:(CLLocationManager*)manager didUpdateHeading:(CLHeading*)heading
{
    if ( heading.headingAccuracy > 0 )
    {
        if ( sensorRegistered & ENVIRONS_SENSOR_FLAG_HEADING )
            PushSensorDataN ( ENVIRONS_SENSOR_TYPE_HEADING, heading.x, heading.y, heading.z );
    }
}


- (BOOL) locationManagerShouldDisplayHeadingCalibration: (CLLocationManager *)manager
{
    return false;
}


- (void) locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
}


- (void) locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray *)locations
{
    CLLocation * currentLocation = locations.lastObject;
    
    if ( currentLocation != nil ) {
        onLocationChanged ( currentLocation, false );
    }
}


- (void) locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
    CLLocation * currentLocation = newLocation;
    
    if ( currentLocation != nil ) {
        onLocationChanged ( currentLocation, false );
    }
}

@end

#endif




