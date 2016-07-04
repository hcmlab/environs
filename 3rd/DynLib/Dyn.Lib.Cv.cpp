/**
 * Dynamically accessing LibAv
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

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Interop.h"
#include "Interop/Export.h"

#include "dyn.Lib.Cv.h"
using namespace environs;

#include <errno.h>

// Disable this flag to use library as statically linked library again
#if !defined(ENVIRONS_IOS)
#   define USE_DYNAMIC_LIB
#endif

//#define USE_CV3

#define CLASS_NAME  "Dyn.Lib.Cv . . . . . . ."

#ifdef __APPLE__
#   define	CV_VER          ".3.0.0"
#   ifdef USE_CV3
#       undef USE_CV3
#   endif
#else
#   ifdef USE_CV3
#       define	CV_VER		"300"
#   else
#       ifdef NDEBUG
#           define	CV_VER	"2410"
#       else
#           define	CV_VER	"2410d"
#       endif
#   endif
#endif
#define CV_NAME             "opencv_"

#ifdef __APPLE__
#   define LIB_SEP          "."
#else
#   define LIB_SEP          "_"
#endif

#ifdef _WIN32
#   define LIBPREFIX		""
#   include "windows.h"

#else 
#   include <dlfcn.h>

#   ifdef __APPLE__
#       define LIBPREFIX	"lib"
#   else
#       define LIBPREFIX	"lib"
#   endif

#endif

#define LIB_CV(name)	LIBPREFIX CV_NAME #name CV_VER

#define	LIBNAME_CVCORE		LIB_CV(core)
#define	LIBNAME_CVGUI		LIB_CV(highgui)
#define	LIBNAME_CVWORLD		LIB_CV(world)


namespace environs
{
	bool							libCv_LibInitialized		= false;
	LONGSYNC						libCv_Initialized			= 0;

	HMODULE							hLibAvCvCore				= 0;
	HMODULE							hLibAvCvHighgui				= 0;

	pcvInitSystem					dcvInitSystem				= 0;
	pcvInitImageHeader				dcvInitImageHeader			= 0;
	pcvCloneImage					dcvCloneImage				= 0;
	pcvSetData						dcvSetData				= 0;

	pcvCreateVideoWriter			dcvCreateVideoWriter		= 0;
	pcvWriteFrame					dcvWriteFrame				= 0;
	pcvReleaseVideoWriter			dcvReleaseVideoWriter		= 0;

	pcvStartWindowThread			dcvStartWindowThread		= 0;
	pcvNamedWindow					dcvNamedWindow				= 0;
	pcvDestroyWindow				dcvDestroyWindow			= 0;
	pcvDestroyAllWindows			dcvDestroyAllWindows		= 0;
	pcvShowImage					dcvShowImage				= 0;
	pcvWaitKey						dcvWaitKey					= 0;


bool VerifyLibCvAccess ( )
{
	if ( !dcvInitImageHeader || !dcvInitSystem || !dcvNamedWindow || !dcvShowImage || !dcvCloneImage
		|| !dcvDestroyWindow || !dcvDestroyAllWindows || !dcvStartWindowThread || !dcvSetData
		|| !dcvWaitKey || !dcvCreateVideoWriter || !dcvWriteFrame || !dcvReleaseVideoWriter
		) {
		CWarn ( "VerifyLibCvAccess: One of the cv functions could not be loaded!" );
		return false;
	}
	return true;
}


void ReleaseCvLib ( )
{
	CLog ( "ReleaseCvLib" );
	
	long count = __sync_sub_and_fetch ( &libCv_Initialized, 1 );
	if ( count < 0 ) {
		__sync_add_and_fetch ( &libCv_Initialized, 1 ); return;
	}

	if ( count != 0 )
		return;

	libCv_LibInitialized		= false;

	dcvInitImageHeader			= 0;

	dcvSetData					= 0;
	dcvInitSystem				= 0;
	dcvCloneImage				= 0;
	dcvStartWindowThread		= 0;
	dcvNamedWindow				= 0;
	dcvDestroyWindow			= 0;
	dcvDestroyAllWindows		= 0;
	dcvShowImage				= 0;
	dcvWaitKey					= 0;
	dcvCreateVideoWriter		= 0;
	dcvWriteFrame				= 0;
	dcvReleaseVideoWriter		= 0;

	if ( hLibAvCvCore ) {
		dlclose ( hLibAvCvCore );
		hLibAvCvCore = 0;
	}

	if ( hLibAvCvHighgui ) {
		dlclose ( hLibAvCvHighgui );
		hLibAvCvHighgui = 0;
	}
}


#ifdef USE_DYNAMIC_LIB

bool InitCvLib ( Instance * env, int deviceID )
{
	CVerbID ( "InitCvLib" );

	if ( __sync_add_and_fetch ( &libCv_Initialized, 1 ) != 1 )
		return libCv_LibInitialized;

	if ( libCv_LibInitialized ) {
		CLogID ( "InitCvLib: already initialized." );
		return true;
	}
	
	HMODULE				hLib	= 0;
	bool				ret = false;

#ifdef USE_CV3
	hLib = LocateLoadModule ( LIBNAME_CVWORLD, deviceID );
	if ( !hLib ) {
		CWarnID ( "InitCvLib: Loading of " LIBNAME_CVWORLD " failed." );
		goto Finish;
	}
#else
	hLib = LocateLoadModule ( LIBNAME_CVCORE, deviceID, env );
	if ( !hLib ) {
		CWarnID ( "InitCvLib: Loading of " LIBNAME_CVCORE " failed." );
		goto Finish;
	}
#endif

	dcvInitImageHeader				= (pcvInitImageHeader) dlsym ( hLib, "cvInitImageHeader" );
	dcvCloneImage					= (pcvCloneImage) dlsym ( hLib, "cvCloneImage" );
	dcvSetData						= (pcvSetData) dlsym ( hLib, "cvSetData" );


	hLibAvCvCore					= hLib;

#ifndef USE_CV3
	hLib = LocateLoadModule ( LIBNAME_CVGUI, deviceID, env );
	if ( !hLib ) {
		CWarnID ( "InitCvLib: Loading of " LIBNAME_CVGUI " failed." );
		goto Finish;
	}
#endif
	
	dcvInitSystem					= (pcvInitSystem) dlsym ( hLib, "cvInitSystem" );
	dcvNamedWindow					= (pcvNamedWindow) dlsym ( hLib, "cvNamedWindow" );
	dcvDestroyWindow				= (pcvDestroyWindow) dlsym ( hLib, "cvDestroyWindow" );
	dcvDestroyAllWindows			= (pcvDestroyAllWindows) dlsym ( hLib, "cvDestroyAllWindows" );
	dcvShowImage					= (pcvShowImage) dlsym ( hLib, "cvShowImage" );
	dcvStartWindowThread			= (pcvStartWindowThread) dlsym ( hLib, "cvStartWindowThread" );
	dcvWaitKey						= (pcvWaitKey) dlsym ( hLib, "cvWaitKey" );

	dcvCreateVideoWriter			= (pcvCreateVideoWriter) dlsym ( hLib, "cvCreateVideoWriter" );
	dcvWriteFrame					= (pcvWriteFrame) dlsym ( hLib, "cvWriteFrame" );
	dcvReleaseVideoWriter			= (pcvReleaseVideoWriter) dlsym ( hLib, "cvReleaseVideoWriter" );

#ifndef USE_CV3
	hLibAvCvHighgui					= hLib;
#endif
	
	if ( !VerifyLibCvAccess () ) {
		goto Finish;
	}

	ret = true;

Finish:
	if ( ret ) {
		libCv_LibInitialized = true;
		CLogID ( "InitCvLib: successfully initialized access to opencv." );
	}
	else {
		ReleaseCvLib ();
	}

	return ret;
}


#else

#ifdef _WIN32
#define LIB_CVL(name)	LIBPREFIX CV_NAME #name CV_VER ".lib"

#define	LIBNAME_CVCOREL		LIB_CVL(core)
#define	LIBNAME_CVGUIL		LIB_CVL(highgui)

#pragma comment ( lib, LIBNAME_CVCOREL )
#pragma comment ( lib, LIBNAME_CVGUIL )
#endif

bool InitCvLib ( int deviceID )
{
	CLog ( "InitCvLib" );

	if ( libCv_LibInitialized ) {
		CLog ( "InitCvLib: already initialized." );
		return true;
	}
	
	dcvInitImageHeader			= (pcvInitImageHeader)cvInitImageHeader;
	dcvCloneImage				= (pcvCloneImage)cvCloneImage;
	dcvSetData					= (pcvSetData)cvSetData;

	dcvInitSystem				= (pcvInitSystem)cvInitSystem;
	dcvStartWindowThread		= (pcvStartWindowThread)cvStartWindowThread;
	dcvNamedWindow				= (pcvNamedWindow)cvNamedWindow;
	dcvDestroyWindow			= (pcvDestroyWindow)cvDestroyWindow;
	dcvDestroyAllWindows		= (pcvDestroyAllWindows)cvDestroyAllWindows;
	dcvShowImage				= (pcvShowImage)cvShowImage;
	dcvWaitKey					= (pcvWaitKey)cvWaitKey;

	dcvCreateVideoWriter		= (pcvCreateVideoWriter) cvCreateVideoWriter;
	dcvWriteFrame				= (pcvWriteFrame) cvWriteFrame;
	dcvReleaseVideoWriter		= (pcvReleaseVideoWriter) cvReleaseVideoWriter;

	if ( !VerifyLibCvAccess () ) {
		goto Failed;
	}

	libCv_LibInitialized = true;

	CLog ( "InitCvLib: successfully initialized access to opencv library." );
	return true;

Failed:
	ReleaseCvLib ();
	return false;
}
#endif


} // -> namespace environs



