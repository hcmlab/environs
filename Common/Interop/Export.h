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

#ifdef __cplusplus
	namespace environs {
		class Instance;
	}
#endif

#if (defined(ENVIRONS_CORE_LIB) || defined(MEDIATORDAEMON))

// Declaration for environs core library
    extern HLIB LocateLoadEnvModule ( const char * module, int deviceID, environs::Instance * obj );

#	define LocateLoadModule(module,deviceID,obj)   LocateLoadEnvModule ( module, deviceID, obj )

#	define CreateEnvInstance(module,index,type,deviceID)  CreateInstance ( module, index, type, deviceID )
#else
// Declaration for external modules
#	define LocateLoadModule(module,deviceID,obj)   ((environs::Instance *)pEnvirons)->LocateLoadEnvModule ( module, deviceID, obj )

#	define CreateEnvInstance(module,index,type,deviceID)  ((environs::Instance *)pEnvirons)->CreateInstance ( module, index, type, deviceID )
#endif

#ifdef __cplusplus
	typedef HLIB ( *pLocateLoadModule )( COBSTR module, int deviceID, environs::Instance * obj  );
#endif

#define	ENVMODPREFIX					"libEnv-"

#define	LIBNAME_EXT_DIR					"libs"

#define ENVIRONS_BUILD_CRT              0

#ifdef __APPLE__
#   define	LIBEXTENSION				".dylib"
#   define	ENVLIBPREFIX				"lib"
#else
#   ifdef _WIN32
#       define	LIBEXTENSION			".dll"
#       define	ENVLIBPREFIX
#   else
#       define	LIBEXTENSION			".so"
#       define	ENVLIBPREFIX			"lib"
#   endif

#	ifdef _WIN32
#       undef ENVIRONS_BUILD_CRT

#		pragma warning( push )
#		pragma warning( disable: 4996 )
/// _MSC_VER = 1600 v100
/// _MSC_VER = 1800 v120 v120_xp	
#		if (_MSC_VER <= 1600)
#			define ENVIRONS_BUILD_CRT		100
#			define NO_ATOMIC_CPP
#		else
#		if (_MSC_VER <= 1700)
#			define ENVIRONS_BUILD_CRT		110
#			define NO_ATOMIC_CPP
#		else
#			if (_MSC_VER <= 1800)
#			define ENVIRONS_BUILD_CRT		120
#			else
#				if (_MSC_VER <= 1900)
#				define ENVIRONS_BUILD_CRT	140
#				else
#					define ENVIRONS_BUILD_CRT	90
#				endif
#			endif
#		endif
#		endif
#	endif
#endif

#ifdef ANDROID
#   define NO_ATOMIC_CPP
#endif

#ifndef NO_ATOMIC_CPP
#   ifdef LINUX
#       define NO_ATOMIC_CPP
#   endif
#endif


#define ENVIRONS_TSDIR                  "v" ENVIRONS_TOSTRING(ENVIRONS_BUILD_CRT)

/// OS specific export macros
#if (defined(ENVIRONS_NATIVE_MODULE) || defined(ENVIRONS_CORE_LIB))
#	ifdef _WIN32
#		define LIBEXPORT		__declspec(dllexport)
#		define CallConv			__cdecl
#	else //_WIN32
#	ifdef DISPLAYDEVICE
#		define CallConv		
#	else
#		if defined(JNICALL)
#			define CallConv		JNICALL
#		endif
#		if !defined(CallConv)
#			define CallConv		
//#			define CallConv		__attribute__((pcs("aapcs")))
#		endif
#	endif
#	if defined(JNIEXPORT)
#		define LIBEXPORT		JNIEXPORT
#	endif
#	if !defined(JNIEXPORT)
#		define LIBEXPORT		__attribute__((visibility("default")))
#	endif
#endif /// end-_WIN32
#else
#	define LIBEXPORT
#	define CallConv
#endif

#ifdef ANDROID
#	define EnvironsFunc(FuncName, ...)	\
		CallConv Java_environs_Environs_##FuncName ( JNIEnv * jenv, jclass jcls, __VA_ARGS__ )
#	define EnvironsProc(FuncName)	\
		CallConv Java_environs_Environs_##FuncName(JNIEnv * jenv, jclass jcls)
#else
#	define EnvironsFunc(FuncName, ...)	\
		CallConv FuncName ( __VA_ARGS__ )
#	define EnvironsProc(FuncName)	\
		CallConv FuncName ( )
#endif /// end-ANDROID

#endif /// end-INCLUDE_HCM_ENVIRONS_NANDROID_EXPORT_INTEROP_H
