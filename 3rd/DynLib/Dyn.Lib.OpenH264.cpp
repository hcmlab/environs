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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Native.h"
#include "Environs.Obj.h"
#include "Interop.h"
#include "Dyn.Lib.OpenH264.h"
using namespace environs;

#include <errno.h>

// Disable this flag to use library as statically linked library again
#ifndef ENVIRONS_IOS
#   define USE_DYNAMIC_LIB
#endif

#define CLASS_NAME	"Dyn.Lib.OpenH264 . . . ."

#define	MODLIBNAME	"libopenh264"

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

namespace environs
{
	bool							openh264_LibInitialized		= false;
	HMODULE							hLibOpenH264				= 0;

	pWelsCreateSVCEncoder			dWelsCreateSVCEncoder		= 0;
	pWelsDestroySVCEncoder			dWelsDestroySVCEncoder		= 0;
	//pWelsGetDecoderCapability		dWelsGetDecoderCapability	= 0;
	pWelsCreateDecoder				dWelsCreateDecoder			= 0;
	pWelsDestroyDecoder				dWelsDestroyDecoder			= 0;
	pWelsGetCodecVersion			dWelsGetCodecVersion		= 0;


	bool VerifyLibOpenH264Access ()
	{
		if ( !dWelsCreateSVCEncoder || !dWelsDestroySVCEncoder || !dWelsCreateDecoder // || !dWelsGetDecoderCapability
			|| !dWelsDestroyDecoder || !dWelsGetCodecVersion
			) {
			CWarn ( "VerifyLibOpenH264Access: One of the openh264 functions could not be loaded!" );
			return false;
		}

		return true;
	}


	void ReleaseLibOpenH264 ()
	{
		CVerb ( "ReleaseLibOpenH264" );

		openh264_LibInitialized		= false;

		dWelsCreateSVCEncoder		= 0;
		dWelsDestroySVCEncoder		= 0;
		//dWelsGetDecoderCapability	= 0;
		dWelsCreateDecoder			= 0;
		dWelsDestroyDecoder			= 0;
		dWelsGetCodecVersion		= 0;

		if ( hLibOpenH264 ) {
			dlclose ( hLibOpenH264 );
			hLibOpenH264 = 0;
		}
	}


#ifdef USE_DYNAMIC_LIB


	bool InitLibOpenH264 ( Instance * env, int deviceID )
	{
		CVerb ( "InitLibOpenH264" );

		if ( openh264_LibInitialized ) {
			CVerbID ( "InitLibOpenH264: already initialized." );
			return true;
		}

		HMODULE				hLib	= 0;
		bool				ret = false;

		hLib = LocateLoadModule ( MODLIBNAME, deviceID, env );
		if ( !hLib ) {
#ifdef _WIN32
			CWarnArgID ( "InitLibOpenH264: Loading of " MODLIBNAME " FAILED with error [0x%.8x]", GetLastError () );
#else
			CWarnArgID ( "InitLibOpenH264: Loading of " MODLIBNAME " FAILED with error [0x%.8x]", errno );
#endif
			goto Finish;
		}

		dWelsCreateSVCEncoder		= (pWelsCreateSVCEncoder) dlsym ( hLib, "WelsCreateSVCEncoder" );
		dWelsDestroySVCEncoder		= (pWelsDestroySVCEncoder) dlsym ( hLib, "WelsDestroySVCEncoder" );
		//dWelsGetDecoderCapability	= (pWelsGetDecoderCapability) dlsym ( hLib, "WelsGetDecoderCapability" );
		dWelsCreateDecoder			= (pWelsCreateDecoder) dlsym ( hLib, "WelsCreateDecoder" );
		dWelsDestroyDecoder			= (pWelsDestroyDecoder) dlsym ( hLib, "WelsDestroyDecoder" );
		dWelsGetCodecVersion		= (pWelsGetCodecVersion) dlsym ( hLib, "WelsGetCodecVersion" );

		if ( !VerifyLibOpenH264Access () ) {
			goto Finish;
		}

		ret = true;

	Finish:
		if ( ret ) {
			hLibOpenH264 = hLib;
			openh264_LibInitialized = true;
			CVerbID ( "InitLibOpenH264: successfully initialized access to " MODLIBNAME );
		}
        else {
            CErrID ( "InitLibOpenH264: Failed to initialize " MODLIBNAME );
			ReleaseLibOpenH264 ();
		}

		return ret;
	}


#else

#ifdef _WIN32
#pragma comment ( lib, "libopenh264.lib" )
#endif

	bool InitLibOpenH264 ( int deviceID )
	{
		CVerbID ( "InitLibOpenH264" );

		if ( openh264_LibInitialized ) {
			CVerbID ( "InitLibOpenH264: already initialized." );
			return true;
		}
        
#ifndef ENABLE_IOS_NATIVE_H264_ONLY
        
		dWelsCreateSVCEncoder		= (pWelsCreateSVCEncoder)		WelsCreateSVCEncoder;
		dWelsDestroySVCEncoder		= (pWelsDestroySVCEncoder)		WelsDestroySVCEncoder;
		//dWelsGetDecoderCapability	= (pWelsGetDecoderCapability)	WelsGetDecoderCapability;
		dWelsCreateDecoder			= (pWelsCreateDecoder)			WelsCreateDecoder;
		dWelsDestroyDecoder			= (pWelsDestroyDecoder)			WelsDestroyDecoder;
		dWelsGetCodecVersion		= (pWelsGetCodecVersion)		WelsGetCodecVersion;
#endif
        
        if ( !VerifyLibOpenH264Access ( ) ) {
            CErrID ( "InitLibOpenH264: Failed to initialize " MODLIBNAME );
			goto Failed;
		}

		openh264_LibInitialized = true;

		CVerbID ( "InitLibOpenH264: successfully initialized access to " MODLIBNAME );
		return true;

	Failed:
		ReleaseLibOpenH264 ();
		return false;
	}
#endif



} // -> namespace environs

#endif

