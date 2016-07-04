/**
 * iOS Camera capture
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
#ifdef ENVIRONS_OSX
#   ifndef ENVIRONS_NATIVE_MODULE
#       define ENVIRONS_NATIVE_MODULE
#   endif
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifndef _WIN32

/// We include Environs.Native.h only for the debug functions.
/// Third party plugins may use their own debug log methods.
#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Environs.Types.h"

#include "Interop/Export.h"

#include "Portal.Worker.Stages.h"
#include "pthread.h"
using namespace environs;

#import "Cam.iOS.h"


#define	CLASS_NAME 	"iOS.Cam. . . . . . . . ."


static const char					*		IOSCam_extensionNames[]	= { "iOS Camera Capture", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	IOSCam_interfaceTypes[]	= { InterfaceType::Encoder, InterfaceType::Unknown };


/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( IOSCam_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( IOSCam_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( IOSCam );


/**
 * SetEnvironsMethods
 *
 *	Injects environs runtime methods.
 *
 */
BUILD_INT_SETENVIRONSOBJECT ();

#endif


extern "C" void * CallConv CreateInstanceIOSCam ( unsigned int index, unsigned int deviceID )
{
    if ( index == 0 ) {
        environs::IOSCam * obj = new environs::IOSCam ( );
        if ( obj ) {
            obj->deviceID = deviceID;
            return obj;
        }
        else {
            CErrID ( "CreateInstanceIOSCam: Failed to create an interface object." );
        }
    }
    return 0;
}


@implementation iOSCam

- (id) init
{
    CLog ( "init" );
    
    capturing   = false;
    disposed    = false;
    camera      = 0;
    session     = 0;
    capture     = 0;
    deviceID    = 0;
    camNumber   = 0;

    return self;
}


- (void) dispose
{
    CLogID ( "dispose" );

}


- (void) dealloc
{
    CLogID ( "dealloc" );

    [self dispose];
}


#ifdef ENVIRONS_IOS
/**
 * Get camera device on iOS.
 *
 * @param cameraType        A value of type PortalType::PortalType. If Any is requested, then we select the front facing camera.
 * @return AVCaptureDevice	A capture device if available and found.
 */
- (AVCaptureDevice *) GetCameraDevice:(int) cameraType
{
    NSArray * capDevs = [AVCaptureDevice devices];
    if ( [capDevs count] <= 0 )
        return nil;
    
    NSInteger facing = (!cameraType || cameraType == PORTAL_TYPE_FRONT_CAM) ? AVCaptureDevicePositionFront : AVCaptureDevicePositionBack;
    camNumber = (int)facing;
    
    for (AVCaptureDevice * capDev in capDevs) {
        
        CLogArgID ( "GetCameraDevice: Device name: [%s]", [[capDev localizedName] UTF8String]);
        
        if ([capDev hasMediaType:AVMediaTypeVideo]) {
            
            if ([capDev position] == facing) {
                return capDev;
            }
        }
    }
    
    return 0;
}
#else
/**
 * Get camera device on OSX.
 *
 * @param cameraType        A value of type PortalType::PortalType.
 * @return AVCaptureDevice	A capture device if available and found.
 */
- (AVCaptureDevice *) GetCameraDevice:(int) cameraType
{
    NSArray * capDevs = [AVCaptureDevice devices];
    if ( [capDevs count] <= 0 )
        return nil;
    
    int targetCam = (cameraType >> 12);
    if ( [capDevs count] < targetCam )
        targetCam = (int)([capDevs count] - 1);
    
    int curCam = 0;
    
    for (AVCaptureDevice * capDev in capDevs) {
        
        CLogArgID ( "GetCameraDevice: Device name: [%s]", [[capDev localizedName] UTF8String]);
        
        if ([capDev hasMediaType:AVMediaTypeVideo]) {
            
            if (curCam == targetCam) {
                camNumber = targetCam;
                return capDev;
            }
            curCam++;
        }
    }
    
    return 0;
}
#endif


- (bool) PreInitCamera:(int) cameraType
{
    CLogID ( "PreInitCamera" );
    
    session = [[AVCaptureSession alloc] init];
    //session.sessionPreset = AVCaptureSessionPresetPhoto;
    
    camera = [self GetCameraDevice:cameraType];
    if ( !camera ) {
        return false;
    }
    
    int targetWidth = capture->width;
    int targetHeight = capture->height;
    
    int destWidth = 0;
    int destHeight = 0;
    
    NSArray * formats = camera.formats;

#ifdef ENVIRONS_IOS
    AVCaptureDeviceFormat * bestFormat = nil;
#endif
    
    for ( AVCaptureDeviceFormat * format in formats )
    {
        CMFormatDescriptionRef cmForm = format.formatDescription;
        
        //CFStringRef formatName = (CFStringRef)CMFormatDescriptionGetExtension([format formatDescription], kCMFormatDescriptionExtension_FormatName);
        //NSLog ( (__bridge NSString *)formatName );

        CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions ( cmForm );
        
        CLogArgN  ( "Width: %i / Height: %i", dims.width, dims.height );
        
        if ( dims.width > destWidth || dims.height > destHeight ) {
            if ( dims.width < targetWidth && dims.height < targetHeight ) {
                destWidth = dims.width;
                destHeight = dims.height;
#ifdef ENVIRONS_IOS
                bestFormat = format;
#endif
            }
        }
    }
    
#ifdef ENVIRONS_IOS
    NSError * error = nil;
    
    if ( bestFormat && [camera lockForConfiguration:&error] )
    {
        [camera setActiveFormat:bestFormat];
        
        [camera unlockForConfiguration ];
    }
#endif
    capture->width = destWidth;
    capture->height = destHeight;
    return true;
}


- (bool) InitCamera
{
    CLogID ( "InitCamera" );

    NSError * error = nil;
    
  //  [camera lockForConfiguration:nil];
  //  [camera setExposureMode:AVCaptureExposureModeLocked];
//    [camera setManualExposureSupportEnabled:YES];
 //   [camera setExposureGain:3.0];
  //  [camera unlockForConfiguration];
    
    AVCaptureDeviceInput * input = [AVCaptureDeviceInput deviceInputWithDevice:camera error:&error];
    if ( !input ) {
        return false;
    }
    
    [session addInput:input];
    
    // create an output for YUV output with self as delegate
    dispatch_queue_t captureQueue = dispatch_queue_create("uk.co.gdcl.cameraengine.capture", DISPATCH_QUEUE_SERIAL);
    
    AVCaptureVideoDataOutput * output = [[AVCaptureVideoDataOutput alloc] init];
    if ( !output ) {
        return false;
    }

    [output setSampleBufferDelegate:self queue:captureQueue];

    // kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
    // kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
    
    // not supported
    // kCVPixelFormatType_420YpCbCr8PlanarFullRange
    // kCVPixelFormatType_420YpCbCr8Planar
    
    
    NSDictionary * opts = 0;
    //kCVPixelFormatType_420YpCbCr8BiPlanarFullRange (missing colors)
    //kCVPixelFormatType_420YpCbCr8PlanarFullRange (only black screen)
    //kCVPixelFormatType_420YpCbCr8Planar (working)
    //kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange  (missing colors)
    if ( capture->outputType == environs::PortalBufferType::YUV420 )
        opts = [NSDictionary dictionaryWithObjectsAndKeys:
                [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar], kCVPixelBufferPixelFormatTypeKey,
#ifdef ENVIRONS_OSX
                [NSNumber numberWithInt:capture->width], (id)kCVPixelBufferWidthKey,
                [NSNumber numberWithInt:capture->height], (id)kCVPixelBufferHeightKey,
#endif
                nil];
    else /// BGRA or cvpixelbuffer
        opts = [NSDictionary dictionaryWithObjectsAndKeys:
                [NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey,
#ifdef ENVIRONS_OSX
                [NSNumber numberWithInt:capture->width], (id)kCVPixelBufferWidthKey,
                [NSNumber numberWithInt:capture->height], (id)kCVPixelBufferHeightKey,
#endif
                nil];
    
    output.videoSettings = opts;
    [session addOutput:output];
    
    AVCaptureConnection * conn = [output connectionWithMediaType:AVMediaTypeVideo];
    if ( !conn ) {
        return false;
    }
    
#ifdef ENVIRONS_IOS
    if ( camNumber == AVCaptureDevicePositionFront )
        [conn setVideoOrientation:AVCaptureVideoOrientationPortrait];
    else
        [conn setVideoOrientation:AVCaptureVideoOrientationLandscapeLeft];
#else
    [conn setVideoOrientation:AVCaptureVideoOrientationLandscapeLeft];
#endif
    
    NSDictionary* actual = output.videoSettings;
    capture->width = (unsigned int) [[actual objectForKey:@"Width"] integerValue];
    capture->height = (unsigned int) [[actual objectForKey:@"Height"] integerValue];
    
    CMTime targetFPS = CMTimeMake( 1, capture->minFPS );
    NSArray * fpsRanges = [camera.activeFormat videoSupportedFrameRateRanges];
    
    bool setFPS = false;
    
    for ( AVFrameRateRange * range in fpsRanges ) {
        CVerbArgN  ( "Min: %i / Max: %i", (int)range.minFrameDuration.value, (int)range.maxFrameDuration.value );
        
        if (CMTIME_COMPARE_INLINE(targetFPS, >=, range.minFrameDuration) &&
            CMTIME_COMPARE_INLINE(targetFPS, <=, range.maxFrameDuration)) {
            setFPS = true;
            break;
        }
    }
    
    if ( setFPS && [camera lockForConfiguration:&error] )
    {
        [camera setActiveVideoMinFrameDuration: targetFPS ];
        
        [camera setActiveVideoMaxFrameDuration: targetFPS ];
        
        [camera unlockForConfiguration ];
    }
    
    return true;
}


- (void) SetCapture:(environs::IOSCam *) newCapture
{
    CVerbID ( "SetCapture" );
    
    capture = newCapture;
    deviceID = capture->deviceID;
}


- (bool) Start
{
    CVerbID ( "Start" );
    
    if ( !session )
        return false;

    [session startRunning];
    return true;
}


- (void) Stop
{
    CVerbID ( "Stop" );
    
    disposed = true;
    
    if ( !session )
        return;
    
    // Wait until no capture activity is ongoing
    int maxWait = 300;
    
    while ( capturing && maxWait > 0 ) {
        CVerbID ( "Stop: Waiting 100ms ..." );
        usleep ( 100000 );
        maxWait--;
    }
    
    for ( AVCaptureInput * input in session.inputs ) {
        [session removeInput:input];
    }
    
    for ( AVCaptureOutput * output in session.outputs ) {
        [session removeOutput:output];
    }
    
    [session stopRunning];
}


- (void) Release
{
    CVerbID ( "Release" );
    
    session = nil;
    capture = nil;
    camera = nil;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput
                        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
                        fromConnection:(AVCaptureConnection *)connection
{
    if ( !capture || disposed )
        return;
    
    capturing = true;
    
    if ( capture->outputType == environs::PortalBufferType::CVPixelBufferIOSX )
    {
#ifdef USE_PORTAL_THREADS_FOR_IOSX_CAM
        capture->PerformIOSX ( (void *) sampleBuffer );
#else
        capture->data = (void *) sampleBuffer;
        
        ((environs::WorkerStages *)((environs::CamBase *)capture)->stages)->encoder->Perform ();
#endif
    }
    else {
        CVImageBufferRef buffer = CMSampleBufferGetImageBuffer(sampleBuffer);
        if ( !buffer ) {
            CWarnID ( "captureOutput: Null-Pointer received!");
            return;
        }
        
        size_t payloadSize = CVPixelBufferGetDataSize(buffer);
        if ( !payloadSize ) {
            CWarnID ( "captureOutput: 0-sized memory!");
            return;
        }
        
        CVPixelBufferLockBaseAddress(buffer, 0);
        
        char * payload = (char *)CVPixelBufferGetBaseAddress(buffer);
        if ( !payload ) {
            CWarnID ( "captureOutput: Null-Pointer within the buffer received!");
            goto Finish;
        }
        
        ((environs::CamBase *)capture)->PerformBase ( payload, (unsigned int) payloadSize );
        
    Finish:
        CVPixelBufferUnlockBaseAddress ( buffer, 0 );
    }
    
    capturing = false;
}


@end


#ifdef CLASS_NAME
#undef CLASS_NAME
#endif

#define	CLASS_NAME 	"IOSCam"

namespace environs
{
    PortalBufferType_t	iOSCam_outputTypeSupport [] = { PortalBufferType::CVPixelBufferIOSX, PortalBufferType::YUV420, PortalBufferType::BGRA };

    
    IOSCam::IOSCam ()
    {
        camera              = nil;
        name                = IOSCam_extensionNames[0];

        outputTypes			= iOSCam_outputTypeSupport;
		outputTypesLength	= sizeof(iOSCam_outputTypeSupport) / sizeof(iOSCam_outputTypeSupport[0]);
    }
    
    
    IOSCam::~IOSCam ()
    {
        CVerbID ( "Destruct" );
        
        Release ();
    }
    
    
    int IOSCam::PreInit ()
    {
        CVerbID ( "PreInit" );
        
        int ret = 0;
        
        if ( camera )
            camera = nil;
        
        camera = [[iOSCam alloc] init];
        if ( !camera ) {
            CErrID ( "Init: Failed to create a camera object." );
            return 0;
        }
        
        [camera SetCapture:this];
        
        ret = [camera PreInitCamera:(portalID & PORTAL_TYPE_MASK)];
        
        return ret;
    }
    
    
    int IOSCam::Init ()
    {
        CVerbID ( "Init" );
        
        int ret = 0;
        
        ret = [camera InitCamera];
        
        return ret;
    }

    
    
    void IOSCam::Release ( )
    {
        CVerbID ( "Release" );
        
        Stop ();
        
        if ( camera ) {
            [camera Release];
            camera = nil;
        }
    }
    
    
    int IOSCam::Start ( )
    {
        CVerbID ( "Start" );
        
        if ( !camera )
            return 0;
            
        if ( ![camera Start] )
            return 0;

        return 1;
    }
    
    
    int IOSCam::Stop ( )
    {
        CVerbID ( "Stop" );

        if ( camera ) {
            [camera Stop];
        }
        
        return 1;
    }

}


#endif


