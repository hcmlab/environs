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
 
 * --------------------------------------------------------------------
 */
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_ENCODER_IOS_H264
#define	INCLUDE_HCM_ENVIRONS_ENCODER_IOS_H264

#import <Foundation/Foundation.h>
#import <Environs.iOSX.Imp.h>
#import <AVFoundation/AVFoundation.h>

#include "Encoder/Encoder.Base.h"
using namespace environs;


@interface EncoderIOSH264 : NSObject
{
    @public
}

- (bool) Init:(IPortalEncoder *) parentEncoder;
- (bool) Start;

- (int) EncodeImageBuffer:(CVImageBufferRef) imageData;

- (void) Stop;

@end



namespace environs
{
    class EncoderIOSH264Env : implements IPortalEncoder
    {
    public:
        EncoderIOSH264Env ( );
        ~EncoderIOSH264Env ( );
        
        bool						Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate );
        int                         Perform ( RenderContext * context );
        int                         Perform ();
        
        int                         bitRate;
        
    private:
        IPortalEncoder          *   iencoder;
        EncoderIOSH264          *   encoder;
        int                         sendID;
        
        unsigned int				keyframeCounter;
        bool						keyframeHandled;
        
        void                        Dispose ();
    };
}


#endif // --> INCLUDE_HCM_ENVIRONS_ENCODER_IOS_H264