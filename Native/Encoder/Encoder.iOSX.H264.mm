/**
 * iOS H264 Encoder
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
 *
 * --------------------------------------------------------------------
 */
#include "stdafx.h"

#ifdef __APPLE__

#ifdef ENVIRONS_OSX
#define ENVIRONS_NATIVE_MODULE
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#import "Encoder.iOSX.H264.h"
#import <VideoToolbox/VideoToolbox.h>

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Environs.Types.h"
#include "Interop/Export.h"
#include "Environs.Lib.h"
#include "Portal.Worker.Stages.h"
using namespace environs;

#define	CLASS_NAME 	"Encoder.iOS.H264.Env . ."

static const char					*		EncoderIOSH264_extensionNames[]	= { "iOSX h264 Encoder", "End" };


#ifndef ENVIRONS_CORE_LIB

static const InterfaceType::InterfaceType	EncoderIOSH264_interfaceTypes[]	= { InterfaceType::Encoder, InterfaceType::Unknown };


/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( EncoderIOSH264_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( EncoderIOSH264_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( EncoderIOSH264Env );


/**
 * SetEnvironsMethods
 *
 *	Injects environs runtime methods.
 *
 */
BUILD_INT_SETENVIRONSOBJECT ();

#endif


namespace environs
{
    PortalBufferType_t	EncoderIOSH264_inputTypeSupport[] = { PortalBufferType::CVPixelBufferIOSX };
    
    
    EncoderIOSH264Env::EncoderIOSH264Env ()
    {
        name				= EncoderIOSH264_extensionNames [0];
        
        CVerbArgID ( "Construct: [%s]", name );
        
        requireSendID       = true;
        
        inBufferType		= EncoderBufferType::YUV420;
        encodedType         = DATA_STREAM_H264_NALUS;
        encoder				= 0;
        iencoder			= nil;
        bitRate             = 5000000;
        
        inputTypes			= EncoderIOSH264_inputTypeSupport;
        inputTypesLength	= sizeof ( EncoderIOSH264_inputTypeSupport ) / sizeof ( EncoderIOSH264_inputTypeSupport [0] );
        
    }
    
    
    EncoderIOSH264Env::~EncoderIOSH264Env ()
    {
        CLogID ( "Destruct..." );
        
        Dispose ();
        
        CVerbID ( "Destruct destroyed." );
    }
    
    
    void DisposeInstance ( EncoderIOSH264 * toDispose )
    {
        CVerb ( "DisposeInstance" );
        
        // Disposal  may take longer and block the destructor.
        // Therefore, we create a thread and leave disposal to that thread
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,
                                                 (unsigned long)NULL), ^(void) {
            [toDispose Stop];
            
            CVerb ( "DisposeInstance: done" );
        });
    }
    
    void EncoderIOSH264Env::Dispose ()
    {
        CVerbID ( "Dispose" );
        
        if ( iencoder ) {
            delete iencoder;
            iencoder = 0;
        }
        
        if ( encoder ) {
            DisposeInstance ( encoder );
            encoder = nil;
        }
        
        CVerbID ( "Dispose: done" );
    }
    
    
    bool EncoderIOSH264Env::Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate )
    {
        this->deviceID = deviceID;
        
        CVerbID ( "Init" );
        
        if ( !Width || !Height || !BitRate || !FrameRate)
            return false;
        
        Dispose ();
        
        width = Width; height = Height;
        bitRate = BitRate;
    
#ifdef ENVIRONS_IOS
#ifndef ENABLE_IOS_NATIVE_H264_ONLY
//        if ( )
#endif
#else
        if ( env->mod_PortalEncoder ) {
            
        }
#endif
        if ( !encoder && !iencoder ) {
            encoder = [[EncoderIOSH264 alloc]init];
            
            if ( encoder && [encoder Init:this] ) {
                
                if ( ![encoder Start] ) {
                    return false;
                }
            }
        }
        
        CVerbID ( "Init successful." );
        return true;
    }
    
    
    int EncoderIOSH264Env::Perform ( RenderContext * context )
    {
        if ( context->renderedData ) {
            [encoder EncodeImageBuffer:(CVImageBufferRef)context->renderedData];
            return 1;
        }
        return 0;
    }
    
    
    int EncoderIOSH264Env::Perform ( )
    {
        IPortalCapture * capture = ((environs::WorkerStages *)stages)->capture;
        
        if ( capture && capture->data ) {
            [encoder EncodeImageBuffer:(CVImageBufferRef)capture->data];
            return 1;
        }
        
        return 0;
    }
    
}


#undef CLASS_NAME
#define	CLASS_NAME 	"EncoderIOSH264"

#define NALU_TYPE_NON_IDR_SLICE 0x1
#define NALU_TYPE_IDR_SLICE     0x5
#define NALU_TYPE_SEI           0x6
#define NALU_TYPE_SPS           0x7
#define NALU_TYPE_PPS           0x8

@interface EncoderIOSH264 ()
{
    int                             deviceID;
    IPortalEncoder              *   parent;
    
    bool                            doEncode;
    
    VTCompressionSessionRef         vtSession;
    
    char                        *   nalUnits;
    int                             nalUnitsSize;
    int                             nalUnitsCapacity;
    
    char                        *   spsPps;
    int                             spsPpsSize;
    int                             spsPpsCapacity;
    
    bool                            allocated;
    pthread_mutex_t                 mutex;
}
@end


@implementation EncoderIOSH264

-(id) init
{
    CLog ( "init" );
    
    deviceID                = 0;
    parent                  = 0;
    
    allocated               = false;
    vtSession               = nil;
    doEncode                = false;
    
    nalUnits                = 0;
    nalUnitsSize            = 0;
    nalUnitsCapacity        = 0;
    
    spsPps                  = 0;
    spsPpsSize              = 0;
    spsPpsCapacity          = 0;
    
    return self;
}


- (void) dispose
{
    CVerbID ( "dispose" );
    
    doEncode = false;
}


- (void) dealloc
{
    CVerbID ( "dealloc" );
    
    doEncode = false;
    
    if ( nalUnits ) {
        free ( nalUnits );
        nalUnits = 0;
    }
    nalUnitsSize = 0;
    nalUnitsCapacity = 0;
    
    if ( spsPps ) {
        free ( spsPps );
        spsPps = 0;
    }
    spsPpsSize = 0;
    spsPpsCapacity = 0;
    
    [self dispose];
    
    if ( allocated ) {
        allocated = false;
        
        LockDisposeA ( mutex );
    }
    
    CVerbID ( "dealloc: done" );

    IOSX_SUPER_DEALLOC ();
}


- (bool) Init:(IPortalEncoder *) parentEncoder
{
    self->deviceID = parentEncoder->deviceID;
    parent = parentEncoder;
    
    if ( !allocated ) {        
        if ( !LockInitA ( mutex ) )
            return false;
        allocated = true;
    }
    
    spsPpsCapacity = 128;
    spsPps = (char *) malloc ( spsPpsCapacity );
    if ( !spsPps )
        return false;
    spsPpsSize = 0;
    
    return true;
}


- (bool) Start
{
    CVerbID ( "Start" );
    
    if (![self CreateVTEncoder])
        return false;
    
    doEncode = true;
    
    return true;
}


- (void) Stop
{
    CVerbID ( "Stop" );
    
    doEncode = false;    
    
    if ( vtSession ) {
        //VTCompressionSessionCompleteFrames ( vtSession, CMTime{0, 0, 0, 0} );
        
        VTCompressionSessionInvalidate ( vtSession );
        CFRelease ( vtSession );
        vtSession = nil;
    }
    
    CVerbID ( "Stop: done" );
}


- (void) handleEncoderOutput:(CMSampleBufferRef) sampleBuffer
{
    CVerbVerbID ( "handleEncoderOutput" );
    
    OSStatus result;
    
    int flags = 0;
    bool isKeyFrame = false;
    
    CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer ( sampleBuffer );
    if ( !blockBuffer ) {
        CErrID ( "callbackOutput: Failed to get blockBuffer." );
        return;
    }
    
    CMFormatDescriptionRef formatDesc = CMSampleBufferGetFormatDescription(sampleBuffer);
    if ( !formatDesc ) {
        CErrID ( "callbackOutput: Failed to get format description." );
        return;
    }
    
    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray ( sampleBuffer, false );
    if ( attachments )
    {
        CFDictionaryRef attachment = (CFDictionaryRef) CFArrayGetValueAtIndex ( attachments, 0 );
        if ( attachment )
        {
            CFBooleanRef depends = (CFBooleanRef) CFDictionaryGetValue ( attachment, kCMSampleAttachmentKey_DependsOnOthers);
            isKeyFrame = (depends == kCFBooleanFalse);
        }
    }
    
    size_t paramSetCount;
    int nalHeaderBytes;
    
    result = CMVideoFormatDescriptionGetH264ParameterSetAtIndex ( formatDesc, 0, nullptr, nullptr, &paramSetCount, &nalHeaderBytes );
    
    if (result == kCMFormatDescriptionError_InvalidParameter) {
        paramSetCount = 2;
        nalHeaderBytes = 4;
    } else if (result != noErr) {
        CErrID ( "callbackOutput: Failed to get H264ParameterSet." );
        return;
    }
    
    size_t spsPpsReqSize = 0;
    
    if ( isKeyFrame )
    {
        flags = DATA_STREAM_IFRAME;
        
        const uint8_t * paramSet;
        size_t paramSetSize;
        
    ReAllocSpsPps:
        static const char prefixCode[4] = {0, 0, 0, 1};
        
        if ( spsPpsReqSize > spsPpsSize ) {
            if ( spsPps ) {
                free ( spsPps );
                spsPps = 0;
            }
            
            size_t size = (paramSetCount * 4) + spsPpsReqSize;
            spsPps = (char *) malloc ( size );
            
            if ( !spsPps ) {
                CErrID ( "callbackOutput: Failed to allocate memory for sps/pps." );
                return;
            }
            spsPpsCapacity = 0;
            spsPpsSize = (int)size;
        }
        
        size_t curIndex = 0;
        size_t remainBytes = spsPpsCapacity;
        
        for ( size_t i = 0; i < paramSetCount; i++ ) {
            result = CMVideoFormatDescriptionGetH264ParameterSetAtIndex ( formatDesc, i, &paramSet, &paramSetSize, nullptr, nullptr );
            if (result != noErr) {
                CErrArgID ( "callbackOutput: Failed to get param set at index [%i].", i );
                return;
            }
            
            if ( sizeof(prefixCode) + paramSetSize > remainBytes ) {
                spsPpsReqSize = curIndex + (sizeof(prefixCode) + paramSetSize) * 2;
                
                CLogArgID ( "callbackOutput: Reallocating memory of size [%i].", spsPpsReqSize );
                goto ReAllocSpsPps;
            }
            
            memcpy ( spsPps + curIndex, prefixCode, sizeof(prefixCode) );
            
            curIndex += sizeof(prefixCode);
            
            memcpy ( spsPps + curIndex, reinterpret_cast<const char*>(paramSet), paramSetSize );
            
            curIndex += paramSetSize;
        }
        
        spsPpsSize = (int)curIndex;
    }
    
    
    if ( !CMBlockBufferIsRangeContiguous(blockBuffer, 0, 0) ) {
        CErrID ( "callbackOutput: Problem !CMBlockBufferIsRangeContiguous." );
    }
    
    
    char * bufferData = 0;
    size_t bufferSize = 0;
    
    result = CMBlockBufferGetDataPointer ( blockBuffer, 0, NULL, &bufferSize, &bufferData );
    if (result != noErr) {
        CErrID ( "callbackOutput: CMBlockBufferGetDataPointer failed." );
        return;
    }
    
    if ( ![self buildNaluBuffer:bufferData size:(int)bufferSize length:nalHeaderBytes] ) {
        CErrID ( "callbackOutput: buildNaluBuffer failed." );
        return;
    }
    
    environs::API::SendTcpPortalN ( parent->sendID, flags, spsPps, spsPpsSize, nalUnits, 0, nalUnitsSize );    
}


- (bool) buildNaluBuffer:(char *) bufferData size:(int)bufferSize length:(int)headerLength
{
    CVerbVerbID ( "buildNaluBuffer" );
    
    if ( bufferSize + 128 > nalUnitsCapacity ) {
        if ( nalUnits ) {
            free ( nalUnits );
            nalUnits = 0;
        }
        nalUnits = (char *) malloc ( bufferSize * 2 );
        if ( !nalUnits ) {
            CErrArgID ( "buildNaluBuffer: Memory allocation [%i] failed.", bufferSize * 2 );
            return false;
        }
        nalUnitsCapacity = bufferSize * 2;
    }
    
    static const char prefixCode[3] = {0, 0, 1};
    
    
    size_t curIndex = 0;
    size_t curLength = 0;
    
    int bytesRemain = bufferSize;
    while ( bytesRemain > 0 ) {
        curLength = 0;
        
        /*const char * str = ConvertToHexSpaceString ( bufferData, headerLength );
        
        CLogArg ( "buildNaluBuffer: [%s]", str );*/
        
        // Get size
        for ( int i=0; i<headerLength; i++ ) {
            curLength <<= 8;
            curLength |= (uint8_t)bufferData[i];
        }
        
        if ( curLength > bytesRemain )
            return false;
        
        bytesRemain -= headerLength;
        bufferData += headerLength;
        
        memcpy ( nalUnits + curIndex, prefixCode, sizeof(prefixCode) );
        
        curIndex += sizeof(prefixCode);
        
        memcpy ( nalUnits + curIndex, bufferData, curLength );
        
        curIndex += curLength;
        bufferData += curLength;
        bytesRemain -= curLength;
    }
    
    nalUnitsSize = (int)curIndex;
    return true;
}


static void callbackForOutput ( void * callbackOutputRef, void * sourceFrameRefCon, OSStatus result,
                            VTEncodeInfoFlags infoFlags, CMSampleBufferRef sampleBuffer )
{
    CVerbVerbArg ( "callbackForOutput: status [%i]", (int)result );
    
    if ( result != noErr ) {
        return;
    }
    
    EncoderIOSH264 * encoder = (__bridge EncoderIOSH264 *)callbackOutputRef;
    if ( !encoder ) {
        CErr ( "callbackForOutput: Invalid encoder reference." );
        return;
    }
    
    if ( !sampleBuffer ) {
        CErr ( "callbackForOutput: Invalid sampleBuffer." );
        return;
    }
    
    [encoder handleEncoderOutput:sampleBuffer];
    
    encoder = nil;
    
    CVerbVerb ( "callbackForOutput: done" );
}


-(bool) CreateVTEncoder
{
    if ( vtSession ) {
        VTCompressionSessionInvalidate ( vtSession );
        vtSession = nil;
    }
    
    NSDictionary* bufferOptions = @{
                                    (NSString*) kCVPixelBufferWidthKey : @(parent->width),
                                    (NSString*) kCVPixelBufferHeightKey : @(parent->height),
#ifdef ENVIRONS_IOS
                                    (NSString*) kCVPixelBufferOpenGLESCompatibilityKey : @YES,
#endif
                                    (NSString*) kCVPixelBufferIOSurfacePropertiesKey : @{}};
    
    OSStatus status = VTCompressionSessionCreate ( NULL, parent->width, parent->height, kCMVideoCodecType_H264, NULL, (__bridge CFDictionaryRef)bufferOptions,
                                                  NULL, callbackForOutput, (__bridge void *)self, &vtSession );
    
    if ( status != noErr ) {
        CErrArg ( "CreateVTEncoder: status [%i]", (int)status );
        return false;
    }
    else {
        CVerbArg ( "CreateVTEncoder: status [%i]", (int)status );
    }
    
    CFNumberRef cfNumber = CFNumberCreate ( NULL, kCFNumberSInt32Type, &((EncoderIOSH264Env *)parent)->bitRate );
    if ( NULL == cfNumber ) {
        CErr ( "CreateVTEncoder: Failed to create bitRate number." );
        return false;
    }
    
    status = VTSessionSetProperty ( vtSession, kVTCompressionPropertyKey_AverageBitRate, cfNumber );
    
    CFRelease ( cfNumber );
    cfNumber = nil;
    
    if ( status != noErr ) {
        CErrArg ( "CreateVTEncoder: Set bitrate prop; status [%i]", (int)status );
        return false;
    }
    
    status = VTSessionSetProperty ( vtSession, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_Baseline_AutoLevel );
    if ( status != noErr ) {
        CErrArg ( "CreateVTEncoder: Set profile prop; status [%i]", (int)status );
        return false;
    }
    
    status = VTSessionSetProperty ( vtSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue );
    if ( status != noErr ) {
        CErrArg ( "CreateVTEncoder: Set realtime prop; status [%i]", (int)status );
        return false;
    }
    
    int keyFrame = 1;
    cfNumber = CFNumberCreate ( NULL, kCFNumberSInt32Type, &keyFrame );
    
    status = VTSessionSetProperty ( vtSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, cfNumber );
    
    CFRelease ( cfNumber );
    cfNumber = nil;
    
    if ( status != noErr ) {
        CErrArg ( "CreateVTEncoder: Set keyFrame intervall prop; status [%i]", (int)status );
        return false;
    }
    
    return true;
}


- (int) EncodeImageBuffer:(CVImageBufferRef) imageData
{
    if ( !doEncode )
        return 1;
    
    CVerbVerbID ( "EncodeImageBuffer" );
    
    CMTime duration = CMTimeMake(1, 16);
    
    CMTime presentationTimeStamp = CMSampleBufferGetPresentationTimeStamp((CMSampleBufferRef)imageData);
    CVPixelBufferRef pixelbufferPassing= CMSampleBufferGetImageBuffer((CMSampleBufferRef)imageData);
    
    OSStatus status = VTCompressionSessionEncodeFrame ( vtSession,
                                                      pixelbufferPassing,
                                                      presentationTimeStamp,
                                                      duration,
                                                      NULL,
                                                      NULL,
                                                      NULL );
    
    if ( noErr != status ) {
        CErrArg ( "EncodeImageBuffer: Encoding failed; status [%i]", (int)status );
        return 0;
    }
    
    CVerbVerbID ( "EncodeImageBuffer: done" );
    return 1;
}

@end

#endif


