/**
 * iOSX H264 Decoder
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

#ifndef INCLUDE_HCM_ENVIRONS_DECODER_IOSX_H264
#define	INCLUDE_HCM_ENVIRONS_DECODER_IOSX_H264

#import <Foundation/Foundation.h>
#import <Environs.iOSX.Imp.h>
#import <AVFoundation/AVFoundation.h>


@interface DecoderIOSXH264 : NSObject
{
    @public
    bool outputRGBA;
    
    void * parentDecoder;
}

- (bool) Init;
- (bool) Start;

- (bool) SetRenderSurface:(void *) newSurface;
- (int) Perform:(int) type withData:(char *) payload withSize:(int) payloadSize;

- (void) Stop;

@end


#endif // --> INCLUDE_HCM_ENVIRONS_DECODER_IOSX_H264