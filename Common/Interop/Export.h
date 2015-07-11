/**
 *	Environs library export declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NANDROID_EXPORT_INTEROP_H
#define INCLUDE_HCM_ENVIRONS_NANDROID_EXPORT_INTEROP_H

#define ENVIRONSAPI		extern "C" 

#include "Interop.h"

#ifdef ENVIRONS_CORE_LIB

// Declaration for environs core library
extern HMODULE LocateLoadEnvModule ( const char * module, unsigned int deviceID );

#define LocateLoadModule(module,deviceID)   LocateLoadEnvModule ( module, deviceID )

#define CreateEnvInstance(module,index,deviceID)  CreateInstance ( module, index, deviceID )

#else
// Declaration for external modules
#define LocateLoadModule(module,deviceID)   ((environs::EnvironsLib *)pEnvirons)->LocateLoadEnvModule ( module, deviceID )

#define CreateEnvInstance(module,index,deviceID)  ((environs::EnvironsLib *)pEnvirons)->CreateInstance ( module, index, deviceID )

#endif

typedef HMODULE ( *pLocateLoadModule )( COBSTR module, unsigned int deviceID );

#define	ENVMODPREFIX					"libEnv-"

#define	LIBNAME_EXT_DIR					"libs"

#ifdef __APPLE__
    #define	LIBEXTENSION				".dylib"
#else
    #ifdef _WIN32
        #define	LIBEXTENSION			".dll"
    #else
        #define	LIBEXTENSION			".so"
    #endif

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )

//#define STRING2(x) #x
//#define STRING(x) STRING2(x) 
//#pragma message( "TS:" STRING(_MSC_FULL_VER) )
//#pragma message( "TS:" STRING(_MSC_VER) )
//WARNING () 
/// _MSC_VER = 1600 v100
/// _MSC_VER = 1800 v120 v120_xp

#if (_MSC_VER <= 1600)
	#define ENVIRONS_TSDIR			"v100"
#else
	#if (_MSC_VER <= 1700)
		#define ENVIRONS_TSDIR		"v110"
	#else
		#if (_MSC_VER <= 1800)
			#define ENVIRONS_TSDIR	"v120"
		#else
			#define ENVIRONS_TSDIR	""
		#endif
	#endif
#endif
#endif

#endif

#ifdef ENVIRONS_NATIVE_MODULE

/// OS specific export macros
#ifdef _WIN32

#define LIBEXPORT		__declspec(dllexport)
#define CallConv		__cdecl

#else //_WIN32

#ifdef DISPLAYDEVICE
#define CallConv		
#else

#if defined(JNICALL)
#define CallConv		JNICALL
#endif

#if !defined(CallConv)
#define CallConv		
//#define CallConv		__attribute__((pcs("aapcs")))
#endif
#endif

#if defined(JNIEXPORT)
#define LIBEXPORT		JNIEXPORT
#endif

#if !defined(JNIEXPORT)
#define LIBEXPORT		__attribute__((visibility("default")))
#endif

#endif /// end-_WIN32
#else

#define LIBEXPORT
#define CallConv

#endif



#ifdef ANDROID
#define EnvironsFunc(FuncName, ...)	\
	CallConv Java_hcm_environs_Environs_##FuncName ( JNIEnv * jenv, jclass jcls, __VA_ARGS__ )
#define EnvironsProc(FuncName)	\
	CallConv Java_hcm_environs_Environs_##FuncName(JNIEnv * jenv, jclass jcls)
#else
#define EnvironsFunc(FuncName, ...)	\
	CallConv FuncName ( __VA_ARGS__ )
#define EnvironsProc(FuncName)	\
	CallConv FuncName ( )
#endif /// end-ANDROID

#endif /// end-INCLUDE_HCM_ENVIRONS_NANDROID_EXPORT_INTEROP_H
