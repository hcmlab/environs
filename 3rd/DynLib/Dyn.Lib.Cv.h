/**
 * Dynamically accessing OpenCV
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
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_LIBOPENCV_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_LIBOPENCV_H

#include "Interop/export.h"

#ifdef _USE_VLD
#	undef _USE_VLD
#endif

#ifdef USE_CRT_MLC
#	undef USE_CRT_MLC
#	undef _CRTDBG_MAP_ALLOC
#endif

#if defined(_WIN32)
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif

#include "opencv/cv.h"
#include "opencv/highgui.h"
using namespace cv;

#if defined(_WIN32)
#pragma warning( pop )
#endif

namespace environs
{
	/// -------------
	/// Core
	/// -------------
	typedef IplImage * (CV_CDECL * pcvInitImageHeader) (IplImage * image, CvSize size, int depth,
		int channels, int origin, /*  CV_DEFAULT ( 0 ) */
		int align /* CV_DEFAULT ( 4 ) */ 
		);


	typedef CvVideoWriter * (CV_CDECL * pcvCreateVideoWriter) ( const char* filename, int fourcc,
		double fps, CvSize frame_size,
		int is_color /*CV_DEFAULT ( 1 )*/ );

	typedef int (CV_CDECL * pcvWriteFrame) ( CvVideoWriter* writer, const IplImage* image );

	typedef void (CV_CDECL * pcvReleaseVideoWriter) (CvVideoWriter** writer);

	typedef IplImage* (CV_CDECL * pcvCloneImage) (const IplImage* image);

	typedef void (CV_CDECL * pcvSetData) (CvArr* arr, void* data, int step);

	/// -------------
	/// Highgui
	/// -------------

	typedef int (CV_CDECL * pcvStartWindowThread) ( void );

	typedef int (CV_CDECL * pcvNamedWindow) ( const char* name, int flags /*CV_DEFAULT ( CV_WINDOW_AUTOSIZE )*/ );

	typedef void (CV_CDECL * pcvShowImage) (const char* name, const CvArr* image);

	typedef int (CV_CDECL * pcvWaitKey) ( int delay /*CV_DEFAULT ( 0 )*/ );

	typedef void (CV_CDECL * pcvDestroyWindow) (const char* name);

	typedef void (CV_CDECL * pcvDestroyAllWindows) ( void );

	typedef int (CV_CDECL * pcvInitSystem) (int argc, char** argv);



extern void ReleaseCvLib ();
    
#ifdef __cplusplus
    
    class Instance;
    
extern bool InitCvLib ( Instance * env, int deviceID );

#endif
    
extern bool								libCv_LibInitialized;

/// Core
extern pcvInitImageHeader				dcvInitImageHeader;
extern pcvCloneImage					dcvCloneImage;
extern pcvSetData						dcvSetData;

extern pcvCreateVideoWriter				dcvCreateVideoWriter;
extern pcvWriteFrame					dcvWriteFrame;
extern pcvReleaseVideoWriter			dcvReleaseVideoWriter;

/// Highgui
extern pcvInitSystem					dcvInitSystem;
extern pcvStartWindowThread				dcvStartWindowThread;
extern pcvNamedWindow					dcvNamedWindow;
extern pcvDestroyWindow					dcvDestroyWindow;
extern pcvDestroyAllWindows				dcvDestroyAllWindows;
extern pcvShowImage						dcvShowImage;
extern pcvWaitKey						dcvWaitKey;

} // -> namespace environs


#endif	/// INCLUDE_HCM_ENVIRONS_DYNAMIC_LIBOPENCV_H
