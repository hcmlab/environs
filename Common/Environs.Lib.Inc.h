/**
 * Environs Native Layer API common declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_API_DECLS_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_API_DECLS_H

/**
*	Include API export macros that defines LIBEXPORT/CallConv if not declared by ANDROID
*
* */
#include "Interop/Export.h"

/**
*	Include interop declarations such as EBOOL, BSTR, etc...
*
* */
#include "Interop.h"


#if defined(ANDROID)
// Include automatically generated API exports for Android.
#	include "Environs.Native.jni.h"

#define ANDROID_ASSERT(check,exp)               if (check) { exp; }
#define ANDROID_ASSERT_ErRrtf(check,msg)        if (check) { CErr ( msg ); return false; }
#define ANDROID_ASSERT_ErRrtv(check,msg)        if (check) { CErr ( msg ); return; }

#	define EnvironsAndroidNullEnv()             JNIEnv * jenv = 0; jclass jcls = 0;
#	define EnvironsCallArg(name, ...)			Java_environs_Environs_##name(jenv, jcls, __VA_ARGS__)
#	define EnvironsCall(name)					Java_environs_Environs_##name(jenv, jcls)

#	define INIT_PCHAR(szname,ref)				const char * szname = 0; if (ref) szname = jenv->GetStringUTFChars ( ref, NULL );
#	define RELEASE_PCHAR(szname,ref)			if (ref && szname) jenv->ReleaseStringUTFChars ( ref, szname ); \
													szname = NULL;

#	define INIT_BYTEARR(name,ref)				char * name = 0; if (ref) name = (char *)jenv->GetByteArrayElements ( ref, NULL );
#	define RELEASE_BYTEARR(name,ref,mode)		if (ref && name) jenv->ReleaseByteArrayElements ( ref, (jbyte *)name, mode ); \
													name = NULL;
#else
#	define	EnvironsAndroidNullEnv()

/**
 *	API exports for all platforms but Android
 *
 * */
#   define ANDROID_ASSERT(check,exp)
#   define ANDROID_ASSERT_ErRrtf(check,msg)
#   define ANDROID_ASSERT_ErRrtv(check,msg)

#	define EnvironsCallArg(name, ...)			name(__VA_ARGS__)
#	define EnvironsCall(name)					name()

#	define INIT_PCHAR(szname,ref)				const char * szname = (const char *) ref;
#	define RELEASE_PCHAR(szname,ref)			
//szname = NULL;

#	define INIT_BYTEARR(name,ref)				char * name = (char *) ref;
#	define RELEASE_BYTEARR(name,ref,mode)		
//name = NULL;

#	define INIT_VOIDARR(name,ref)				void * name = (void *) ref;
#	define RELEASE_VOIDARR(name,ref,mode)		
// name = NULL;

#endif // ANDROID - include only the jni-exports



#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_API_DECLS_H



