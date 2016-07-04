/**
 * Environs module declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_ANDROID_MODULES_DECLARATIONS_H
#define INCLUDE_HCM_ENVIRONS_ANDROID_MODULES_DECLARATIONS_H

#include "Interop/Export.h"

#define LIBNAME_Decoder_LibOpenH264				ENVMODPREFIX "DecOpenH264" 
#define LIBNAME_Encoder_LibOpenH264				ENVMODPREFIX "EncOpenH264" 

#define LIBNAME_Decoder_Android                 ENVMODPREFIX "DecAndroid" 

#define	LIBNAME_Capture_Android_Camera			ENVMODPREFIX "CapCamera" 
#define	LIBNAME_Capture_Android_Camera44		ENVMODPREFIX "CapCamera4.4" 
#define	LIBNAME_Capture_Android_Camera43		ENVMODPREFIX "CapCamera4.3" 
#define	LIBNAME_Capture_Android_Camera411		ENVMODPREFIX "CapCamera4.1.1" 
#define	LIBNAME_Capture_Android_Camera30		ENVMODPREFIX "CapCamera3.0" 
#define	LIBNAME_Capture_Android_Camera22		ENVMODPREFIX "CapCamera2.2" 
#define	LIBNAME_Capture_Android_CameraInfo44	ENVMODPREFIX "CapCameraInfo4.4" 
#define	LIBNAME_Capture_Android_CameraInfo4		ENVMODPREFIX "CapCameraInfo4.2" 

#define	LIBNAME_Decoder_Android_HwH264			ENVMODPREFIX "DecHwH264" 
#define	LIBNAME_Encoder_Android_HwH264			ENVMODPREFIX "EncHwH264" 
#define	LIBNAME_Encoder_Android_HwH264_42		ENVMODPREFIX "EncHwH264.4.2" 
#define	LIBNAME_Encoder_Android_HwH264_23		ENVMODPREFIX "EncHwH264.2.3" 

#define	LIBNAME_Tracker_Surface					ENVMODPREFIX "TrackSurface" 

#define	LIBNAME_Decoder_Windows					ENVMODPREFIX  "DecWindows"
#define	LIBNAME_Encoder_MediaFoundationH264		ENVMODPREFIX  "EncMfH264"
#define	LIBNAME_Decoder_MediaFoundationH264		ENVMODPREFIX  "DecMfH264"


#endif  /// end-INCLUDE_HCM_ENVIRONS_ANDROID_MODULES_DECLARATIONS_H



