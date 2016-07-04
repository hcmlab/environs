/**
 * iOS H264 Decoder
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "Decoder.iOSX.H264.h"
#import <VideoToolbox/VideoToolbox.h>
#import "Environs.iOSX.Imp.h"

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Environs.Types.h"
#include "Interfaces/IPortal.Decoder.h"
using namespace environs;

#include "Environs.crypt.h"

#define	CLASS_NAME 	"Decoder.iOSX.H264. . . ."


#define NALU_TYPE_NON_IDR_SLICE 0x1
#define NALU_TYPE_IDR_SLICE     0x5
#define NALU_TYPE_SEI           0x6
#define NALU_TYPE_SPS           0x7
#define NALU_TYPE_PPS           0x8

@interface DecoderIOSXH264 ()
{
    unsigned int                    deviceID;
    
    bool                            doDecode;
    UIView                      *   renderSurface;
    
    AVSampleBufferDisplayLayer  *   displayLayer;
    
    CMVideoFormatDescriptionRef     formatDesc;
    VTDecompressionSessionRef       vtSession;
    
    CMBlockBufferRef                blockBuffer;
    char                        *   nalUnits;
    int                             nalUnitsSize;
    int                             nalUnitsCapacity;
    
    char                        *   sps;
    int                             spsSize;
    int                             spsCapacity;
    char                        *   pps;
    int                             ppsSize;
    int                             ppsCapacity;
    
    int                             contentNaluType;
    int                             contentNalus;
    bool                            hasSPS;
    bool                            hasPPS;
    
    bool                            allocated;
    pthread_mutex_t                 mutex;
}
@end


const char * NALUTypeNames[] =
{
    "0: Unspecified (non-VCL)",
    "1: Coded slice of a non-IDR picture (VCL)",
    "2: Coded slice data partition A (VCL)",
    "3: Coded slice data partition B (VCL)",
    "4: Coded slice data partition C (VCL)",
    "5: Coded slice of an IDR picture (VCL)",
    "6: Supplemental enhancement information (SEI) (non-VCL)",
    "7: Sequence parameter set (non-VCL)",
    "8: Picture parameter set (non-VCL)",
    "9: Access unit delimiter (non-VCL)",
    "10: End of sequence (non-VCL)",
    "11: End of stream (non-VCL)",
    "12: Filler data (non-VCL)",
    "13: Sequence parameter set extension (non-VCL)",
    "14: Prefix NAL unit (non-VCL)",
    "15: Subset sequence parameter set (non-VCL)",
    "16: Reserved (non-VCL)",
    "17: Reserved (non-VCL)",
    "18: Reserved (non-VCL)",
    "19: Coded slice of an auxiliary coded picture without partitioning (non-VCL)",
    "20: Coded slice extension (non-VCL)",
    "21: Coded slice extension for depth view components (non-VCL)",
    "22: Reserved (non-VCL)",
    "23: Reserved (non-VCL)",
    "24: STAP-A Single-time aggregation packet (non-VCL)",
    "25: STAP-B Single-time aggregation packet (non-VCL)",
    "26: MTAP16 Multi-time aggregation packet (non-VCL)",
    "27: MTAP24 Multi-time aggregation packet (non-VCL)",
    "28: FU-A Fragmentation unit (non-VCL)",
    "29: FU-B Fragmentation unit (non-VCL)",
    "30: Unspecified (non-VCL)",
    "31: Unspecified (non-VCL)",
};



@implementation DecoderIOSXH264

-(id) init
{
    CLog ( "init" );
    
    deviceID                = 0;
    outputRGBA              = false;
    allocated               = false;
    
    renderSurface           = 0;
    displayLayer            = 0;
    formatDesc              = 0;
    vtSession               = nil;
    blockBuffer             = 0;
    doDecode                = false;
    
    nalUnits                = 0;
    nalUnitsSize            = 0;
    nalUnitsCapacity        = 0;
    
    sps                     = 0;
    spsSize                 = 0;
    spsCapacity             = 0;
    
    pps                     = 0;
    ppsSize                 = 0;
    ppsCapacity             = 0;
    
    contentNaluType         = 0;
    contentNalus            = 0;
    hasSPS                  = false;
    hasPPS                  = false;
    
    parentDecoder           = 0;
    return self;
}


- (void) dispose
{
    CVerbID ( "dispose" );
    
    doDecode = false;
}


- (void) dealloc
{
    CVerbID ( "dealloc" );
    
    if ( formatDesc != nil ) {
        CFRelease(formatDesc);
        formatDesc = nil;
    }
    
    if ( vtSession ) {
        VTDecompressionSessionInvalidate ( vtSession );
        vtSession = nil;
    }
    
    if ( blockBuffer ) {
        CFRelease ( blockBuffer );
        blockBuffer = 0;
    }
    
    if ( nalUnits ) {
        free ( nalUnits );
        nalUnits = 0;
    }
    nalUnitsSize = 0;
    nalUnitsCapacity = 0;
    
    if ( sps ) {
        free ( sps );
        sps = 0;
    }
    spsSize = 0;
    spsCapacity = 0;
    
    if ( pps ) {
        free ( pps );
        pps = 0;
    }
    ppsSize = 0;
    ppsCapacity = 0;
    
    parentDecoder = 0;
    
    [self dispose];
    
    if ( allocated ) {
        allocated = false;
        
        LockDispose ( &mutex );        
    }

    IOSX_SUPER_DEALLOC ();
}


- (bool) Init
{
    if ( !allocated ) {
        if ( !LockInit ( &mutex ) )
            return false;
        allocated = true;
    }
    
    return true;
}


- (bool) Start
{
    doDecode = true;
    
    return true;
}


- (void) Stop
{
    doDecode = false;
    
}


- (bool) CreateVTDecompressor
{
    if ( vtSession ) {
        VTDecompressionSessionInvalidate ( vtSession );
        vtSession = NULL;
    }
    
    VTDecompressionOutputCallbackRecord callBack;
    callBack.decompressionOutputCallback = DecompCallback;
    
    callBack.decompressionOutputRefCon = (__bridge void *)self;
    
    NSDictionary * pixelBufferAtts = nil;
    
#ifdef ENVIRONS_IOS
    pixelBufferAtts = [NSDictionary dictionaryWithObjectsAndKeys:
                       [NSNumber numberWithBool:YES],
                       (id)kCVPixelBufferOpenGLESCompatibilityKey,
                       nil];
#else
    if ( outputRGBA ) {
        pixelBufferAtts = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSNumber numberWithInt:kCVPixelFormatType_24RGB],
                           (id)kCVPixelBufferPixelFormatTypeKey,
                           nil];
        
    }
    else {
        pixelBufferAtts = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSNumber numberWithBool:YES],
                           (id)kCVPixelBufferOpenGLCompatibilityKey,
                           nil];
    }
#endif
    
    OSStatus status =  VTDecompressionSessionCreate ( NULL, formatDesc, NULL,
                                                     (__bridge CFDictionaryRef)(pixelBufferAtts),
                                                     &callBack,
                                                     &vtSession);
    
    
    if ( status != noErr ) {
        CErrArg ( "CreateVTDecompressor: status [%i]", (int)status );
        return false;
    }
    else {
        CVerbArg ( "CreateVTDecompressor: status [%i]", (int)status );
    }
    return true;
}


- (bool) SetRenderSurface:(void *) newSurface
{
    if ( !newSurface )
        renderSurface = nil;
    else
        renderSurface = (__bridge UIView *) newSurface;
    
    if ( outputRGBA )
        return true;
    
    if ( !LockAcquireA ( mutex, "SetRenderSurface" ) ) {
        return false;
    }
    
    AVSampleBufferDisplayLayer  * prevLayer = displayLayer;
    
    displayLayer = [[AVSampleBufferDisplayLayer alloc] init];
    
    CGRect bounds = renderSurface.bounds;
    
    /*bounds.origin.x = 0;
    bounds.origin.y = 0;
    bounds.size.width = environs::environs.device_width;
    bounds.size.height = environs::environs.device_height;
     */
    
    displayLayer.bounds = bounds;
    displayLayer.position = CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
    displayLayer.videoGravity = AVLayerVideoGravityResize;
    
    CMTimebaseRef controlTimebase;
    CMTimebaseCreateWithMasterClock(CFAllocatorGetDefault(), CMClockGetHostTimeClock(), &controlTimebase);
    
    displayLayer.controlTimebase = controlTimebase;
    CMTimebaseSetTime(displayLayer.controlTimebase, kCMTimeZero);
    CMTimebaseSetRate(displayLayer.controlTimebase, 1.0);
    
    if ( prevLayer )
        [[renderSurface layer] replaceSublayer:prevLayer with:displayLayer];
    else
        [[renderSurface layer] addSublayer:displayLayer];
    
    if ( !LockReleaseA ( mutex, "SetRenderSurface" ) ) {
        return false;
    }
    return true;
}


- (void) Render:(CMSampleBufferRef) sampleBuffer
{
    if ( !outputRGBA )
    {
        if ( displayLayer ) {
            if (displayLayer.status == AVQueuedSampleBufferRenderingStatusFailed) {
                CErrArgID ( "Render: [%s]", [[displayLayer.error localizedDescription] UTF8String] );
                
                doDecode = false;
            }
            else {
                [displayLayer enqueueSampleBuffer:sampleBuffer];
            }
        }
    }
    else {
        VTDecodeFrameFlags flags = kVTDecodeFrame_EnableAsynchronousDecompression;
        VTDecodeInfoFlags flagOut;
        /*
         CFDictionaryRef frameInfo = NULL;
         */
        NSDate * now = [NSDate date];
        if ( now ) {
            //VTDecompressionSessionDecodeFrame ( vtSession, sampleBuffer, flags, (void*)CFBridgingRetain(now), &flagOut );
            VTDecompressionSessionDecodeFrame ( vtSession, sampleBuffer, flags, (__bridge void *) now, &flagOut );
            now = nil;
        }
    }
}


- (int) Perform:(int) type withData:(char *) payload withSize:(int) payloadSize
{
    if ( !doDecode )
        return 1;
    
    
    if ( pthread_mutex_lock(&mutex) ) {
        CErr ( "Perform: Failed to lock mutex." );
        return 0;
    }
    
    int success = [self TransformToMP4:payload withSize:payloadSize];
    
    if ( success )
        success = [self Perform];
    else
        success = true;
    
    if ( blockBuffer ) {
        CFRelease ( blockBuffer );
        blockBuffer = nil;
    }
    
    if ( pthread_mutex_unlock(&mutex) ) {
        CErr ( "Perform: Failed to unlock mutex." );
        return 0;
    }
    
    return success;
}


inline bool RefactorMemory ( char * &source, int &capacity, int &sourceSize, int reqSize, int destSize )
{
    if ( !source || (reqSize < capacity) ) {
        sourceSize = 0;
        if ( source )
            free ( source );
        source = (char *) malloc ( destSize );
        if ( !source )
            return false;
        capacity = destSize;
    }
    return true;
}


- (bool) AddMP4Unit:(char *)data length:(int)unitSize
{
    OSStatus success;
    size_t curLength = CMBlockBufferGetDataLength ( blockBuffer );
    
    success = CMBlockBufferAppendMemoryBlock ( blockBuffer, NULL,
                                              4,
                                              kCFAllocatorDefault, NULL, 0,
                                              4, 0);
    if ( success != noErr ) {
        CErrArgID ( "AddMP4Unit: CMBlockBufferAppendMemoryBlock failed: [%i]", (int)success );
        return false;
    }
    
    const uint8_t lengthBytes[] = {(uint8_t)(unitSize >> 24), (uint8_t)(unitSize >> 16),
        (uint8_t)(unitSize >> 8), (uint8_t)unitSize};
    
#ifdef ENVIRONS_OSX
    success = CMBlockBufferAssureBlockMemory ( blockBuffer );
    if ( success != noErr ) {
        CErrArgID ( "AddMP4Unit: CMBlockBufferAssureBlockMemory failed: [%i]", (int)success );
        return false;
    }
#endif
    
    success = CMBlockBufferReplaceDataBytes ( lengthBytes, blockBuffer, curLength, 4 );
    if ( success != noErr ) {
        CErrArgID ( "AddMP4Unit: CMBlockBufferReplaceDataBytes failed: [%i]", (int)success );
        return false;
    }
    
    success = CMBlockBufferAppendMemoryBlock ( blockBuffer, data,
                                            unitSize,
                                            kCFAllocatorNull,
                                            NULL, 0, unitSize, 0 );
    if (success != noErr) {
        CErrArgID ( "AddMP4Unit: CMBlockBufferAppendMemoryBlock failed: [%i]", (int)success );
        return false;
    }
    
    return true;
}


- (bool) TransformToMP4:(char *)payload withSize:(int)sourceSize
{
    CVerbVerb ( "TransformToMP4" );
    
    int headerLength = 0;
    int count       = 0;
    bool ret        = false;
    
    contentNaluType = 0;
    contentNalus    = 0;
    hasSPS          = false;
    hasPPS          = false;
    
    int lastNaluType    = 0;
    int lastNaluStart   = 0;
    
    int naluIndex       = 0;
    
    int i = 0;
    
    
    OSStatus success = CMBlockBufferCreateEmpty ( NULL, 0, 0, &blockBuffer );
    if ( success != noErr ) {
        CErrArgID ( "TransformToMP4: CMBlockBufferCreateEmpty failed [%i]", (int)success );
        return false;
    }
    
    if ( !RefactorMemory ( nalUnits, nalUnitsCapacity, nalUnitsSize, sourceSize * 1.5, sourceSize * 2 ) )
        goto Done;
    
    for ( ; i < sourceSize - 4 ; i++)
    {
        if ( payload[i] == 0 && payload[i+1] == 0 )
        {
            if (payload[i+2] == 0x00 && payload[i+3] == 0x01) {
                headerLength = 4;
            }
            else if (payload[i+2] == 0x01) {
                headerLength = 3;
            }
            else
                continue;
            
            int naluType = (payload[i + headerLength] & 0x1F);
            CVerbVerbArgID ( "TransformToMP4: Found NAL unit type [%s] with headerlength [%i].", NALUTypeNames[naluType], headerLength );
            
            if ( lastNaluType != 0 )
            {
                if ( !lastNaluStart ) {
                    CErrID ( "TransformToMP4: Invalid state. Missing start position from previous NAL unit." );
                    break;
                }
                
                if ( lastNaluType == NALU_TYPE_SPS ) {
                    // SPS
                    int reqSize = i - lastNaluStart;
                    
                    if ( !RefactorMemory ( sps, spsCapacity, spsSize, reqSize, reqSize * 2 ) )
                        goto Done;
                    
                    if ( !spsSize || spsSize != reqSize || !memcmp ( sps, payload + lastNaluStart, reqSize ) ) {
                        memcpy ( sps, payload + lastNaluStart, reqSize );
                        spsSize = reqSize;
                        hasSPS = true;
                    }
                }
                else if ( lastNaluType == NALU_TYPE_PPS ) {
                    // PPS
                    int reqSize = i - lastNaluStart;
                    
                    if ( !RefactorMemory ( pps, ppsCapacity, ppsSize, reqSize, reqSize * 2 ) )
                        goto Done;
                    
                    if ( !ppsSize || ppsSize != reqSize || !memcmp ( pps, payload + lastNaluStart, reqSize ) ) {
                        memcpy ( pps, payload + lastNaluStart, reqSize );
                        ppsSize = reqSize;
                        hasPPS = true;
                    }
                }
                else {
                    if ( lastNaluType == NALU_TYPE_IDR_SLICE || lastNaluType == NALU_TYPE_NON_IDR_SLICE )
                    {
                        int unitSize = i - lastNaluStart;
                        
                        if ( ! [self AddMP4Unit:payload + lastNaluStart length:unitSize] )
                            goto Done;
                        
                        naluIndex += unitSize + 4;
                        
                        contentNaluType = lastNaluType;
                        contentNalus++;
                    }
                    else {
                        CVerbVerbArgID ( "TransformToMP4: Ingoring NALU type [%i / %s].", lastNaluType, NALUTypeNames[naluType] );
                    }
                }
            }
            
            lastNaluType    = naluType;
            
            i               += headerLength;
            lastNaluStart = i;
            count++;
        }
    }
    
    if ( i >= sourceSize - 4 ) {
        int unitSize = sourceSize - lastNaluStart;
        
        
        if ( ! [self AddMP4Unit:payload + lastNaluStart length:unitSize] )
            goto Done;
        
        naluIndex += unitSize + 4;
        contentNaluType = lastNaluType;
        contentNalus++;
    }
    
    if ( contentNalus > 0 ) {
        nalUnitsSize = naluIndex;
        ret = true;
    }
Done:
    
    CVerbVerbArgID ( "TransformToMP4: Received [%i] NALUs and [%i] content NALUs", count, contentNalus );
    return ret;
}


- (bool) Perform
{
    OSStatus            success         = noErr;
    CMSampleBufferRef   sampleBuffer    = NULL;
    
    if ( contentNalus <= 0 )
        return true;
        
    if ( hasSPS && hasPPS ) {
        char *  setPointers [ 2 ]   = { sps, pps };
        size_t  setSizes [ 2 ]      = {(size_t)spsSize, (size_t)ppsSize};
        
        if ( formatDesc != nil ) {
            CFRelease ( formatDesc );
            formatDesc = 0;
        }
        
        success = CMVideoFormatDescriptionCreateFromH264ParameterSets ( kCFAllocatorDefault,
                                                                     2,
                                                                     (const uint8_t *const*)setPointers,
                                                                     setSizes,
                                                                     4,
                                                                     &formatDesc );
        
        CVerbVerbArgID ( "Perform: CMVideoFormatDescription status [%i]", (int)success );
        
        if ( success != noErr ) {
            CErrArgID ( "Perform: CMVideoFormatDescription error [%i]", (int)success );
            
            return false;
        }
        
        if ( (success == noErr) && outputRGBA && (vtSession == NULL) )
        {
            [self CreateVTDecompressor];
        }
    }
    
    if ( success == noErr )
    {
        const size_t sampleSize = nalUnitsSize;
        success = CMSampleBufferCreate ( kCFAllocatorDefault,
                                       blockBuffer, true, NULL, NULL,
                                       formatDesc, 1, 0, NULL, 1,
                                       &sampleSize, &sampleBuffer);
        
        CVerbVerbArgID ( "Perform: CMSampleBufferCreate status [%i]", (int)success );
    }
    
    if ( success == noErr )
    {
        CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, YES);
        CFMutableDictionaryRef dict = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
        
        CFDictionarySetValue(dict, kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);
        CFDictionarySetValue(dict, kCMSampleAttachmentKey_IsDependedOnByOthers, kCFBooleanTrue);
        /*
        if (contentNaluType == 1) {
            // P-frame
            CFDictionarySetValue(dict, kCMSampleAttachmentKey_NotSync, kCFBooleanTrue);
            CFDictionarySetValue(dict, kCMSampleAttachmentKey_DependsOnOthers, kCFBooleanTrue);
        } else {
            // I-frame
            CFDictionarySetValue(dict, kCMSampleAttachmentKey_NotSync, kCFBooleanFalse);
            CFDictionarySetValue(dict, kCMSampleAttachmentKey_DependsOnOthers, kCFBooleanFalse);
        }
        */
        
        // either send the samplebuffer to a VTDecompressionSession or to an AVSampleBufferDisplayLayer
        [self Render:sampleBuffer];
    }
    
    if ( sampleBuffer )
        CFRelease ( sampleBuffer );
    
    
    return (success == noErr);
}


void DecompCallback ( void *decompRefCon, void *sourceFrameRefCon,
                      OSStatus status,
                      VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer,
                      CMTime timeStamp, CMTime duration )
{
    if (status != noErr)
    {
        NSError *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        
        CErrArg ( "DecompCallback: status [%s]", [[error localizedDescription] UTF8String] );
    }
    else
    {
        CVerbVerb ( "DecompCallback: ok" );
        
        // prepare an EnvironsAvContext and leave rendering to the surface to the platform layer
        
        DecoderIOSXH264 * decoder = (__bridge DecoderIOSXH264 *)decompRefCon;
        if ( !decoder || !decoder->renderSurface )
            return;
        
        IPortalDecoder * portalDec = (IPortalDecoder *) decoder->parentDecoder;
        if ( !portalDec->renderCallback )
            return;
        
        CVReturn ret = CVPixelBufferLockBaseAddress ( imageBuffer, 0 );
        if ( ret != kCVReturnSuccess )
            return;
        
        while ( portalDec )
        {
            uint8_t * data = (uint8_t *) CVPixelBufferGetBaseAddress ( imageBuffer) ;
            if ( !data )
                break;
            
            //size_t bytesPerRow = CVPixelBufferGetBytesPerRow ( imageBuffer );
            //size_t width = CVPixelBufferGetWidth ( imageBuffer );
            //size_t height = CVPixelBufferGetHeight ( imageBuffer );
            
            size_t dataSize = CVPixelBufferGetDataSize ( imageBuffer );
            
            //CVerbVerbArg ( "DecompCallback: size [%u]", dataSize );
            
            if ( !portalDec->avContext ) {
                portalDec->avContext = (char *) malloc ( dataSize );
                if ( !portalDec->avContext )
                    break;
            }
            
            memcpy ( portalDec->avContext, data, dataSize );
            break;
        }
        
        CVPixelBufferUnlockBaseAddress ( imageBuffer, 0 );

        /*
        CIImage * ciImage = [CIImage imageWithCVPixelBuffer:imageBuffer];
        CIContext *temporaryContext = [CIContext contextWithOptions:nil];
        CGImageRef videoImage = [temporaryContext
                                 createCGImage:ciImage
                                 fromRect:CGRectMake(0, 0,
                                                     CVPixelBufferGetWidth(imageBuffer),
                                                     CVPixelBufferGetHeight(imageBuffer))];
        
        UIImage *image = [[UIImage alloc] initWithCGImage:videoImage];
 
        
        
        try {
            @autoreleasepool
            {
                DecoderIOSXH264 * decoder = (__bridge DecoderIOSXH264 *)decompRefCon;
                
                UIGraphicsBeginImageContext ( decoder->renderSurface.frame.size );
                //CGContextRef context = UIGraphicsGetCurrentContext ();
                
                [image drawInRect:decoder->renderSurface.bounds];
                UIImage *imageStretch = UIGraphicsGetImageFromCurrentImageContext ( );
                
                //CGContextFlush(context);
                UIGraphicsEndImageContext ( );
                
                image = 0;
                
                if ( imageStretch ) {
                    dispatch_sync ( dispatch_get_main_queue ( ), ^ {
                        decoder->renderSurface.backgroundColor = [[UIColor alloc] initWithPatternImage:imageStretch];
                    } );
                    
                    imageStretch = 0;
                }
            }
        }
        catch ( NSException * ie ) {
            NSLog ( @"DecompCallback: Exception: %@", ie );
        }
        
        CGImageRelease(videoImage);
         */
    }
}


@end



