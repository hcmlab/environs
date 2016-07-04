/**
 *	Portal Receiver for iOS
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

#if defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX)

#include "Portal.Receiver.iOSX.h"


/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#import "Decoder/Decoder.iOSX.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Receiver.iOSX . ."


namespace environs 
{

	int PortalReceiveriOSX::CreateDecoder ( )
	{
		CVerbID ( "CreateDecoder" );

        decoder = new DecoderIOSX ( );
        if ( decoder ) {
            return DATA_STREAM_IMAGE_JPEG | DATA_STREAM_IMAGE_PNG | DATA_STREAM_H264_NALUS | DATA_STREAM_H265_NALUS;
        }
        
		return 0;
	}
	

} /// -> namespace environs

#endif