/**
 * Dynamically accessing openh264
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
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENHX264_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENHX264_H

#include "Interop/Export.h"
#include "openh264/codec_api.h"

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

namespace environs
{
	typedef int (CallConv * pWelsCreateSVCEncoder) ( ISVCEncoder ** enc );

	typedef void (CallConv * pWelsDestroySVCEncoder) (ISVCEncoder * enc);

	typedef int (CallConv * pWelsGetDecoderCapability) (SDecoderCapability * caps);

	typedef long (CallConv * pWelsCreateDecoder) (ISVCDecoder ** dec);

	typedef void (CallConv * pWelsDestroyDecoder) (ISVCDecoder * dec);

	typedef OpenH264Version ( CallConv * pWelsGetCodecVersion ) ();

	extern void ReleaseLibOpenH264 ();
    
#ifdef __cplusplus
    
    class Instance;
    
	extern bool InitLibOpenH264 ( Instance * env, int deviceID );
#endif
    
	extern bool								openh264_LibInitialized;

	extern pWelsCreateSVCEncoder			dWelsCreateSVCEncoder;
	extern pWelsDestroySVCEncoder			dWelsDestroySVCEncoder;
	extern pWelsGetDecoderCapability		dWelsGetDecoderCapability;
	extern pWelsCreateDecoder				dWelsCreateDecoder;
	extern pWelsDestroyDecoder				dWelsDestroyDecoder;
	extern pWelsGetCodecVersion				dWelsGetCodecVersion;

} // -> namespace environs

#endif

#endif	/// INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENHX264_H
