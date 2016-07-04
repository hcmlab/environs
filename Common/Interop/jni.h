/**
 *	Platform interop definitions for jni
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
#ifndef ENVIRONS_NANDROID_JNI_INTEROP_H
#define ENVIRONS_NANDROID_JNI_INTEROP_H

#if defined(ANDROID)
#   include <jni.h>

#   define jintp           jint
#   define juint           jint
#   define juintp          jint
#   define jvoidArray      jbyteArray
#   define jtstring        jstring
#   define jsstring        jstring

#   define	DUMBJENV()
#   define	DUMBJCLASS()

#   define FAKEJNI()		JNIEnv * jenv; jclass jcls;
//#define	DUMBJCLASS()	jclass jcls = 0;
#else

//#define JNIEXPORT
#   define jboolean        bool
#   define jint            int
#   define jintp           int *
/// The reason for intruducing juint is that java does not support unsigned. Hence, we take it as signed and cast the value to unsigned within the native layer.
#   define juint           unsigned int
#   define juintp          unsigned int *
#   define jlong           long
#   define jfloat          float
#   define jdouble         double
#   define jshort          unsigned short
#   define jbyte           unsigned char
#   define jsstring        char *
#   define jstring         const char *
#   define jtstring        TCHAR	*
#   define jbyteArray      unsigned char	*
#   define jintArray       int		*
#   define jfloatArray     float	*
#   define JNIEnv          void
#   define jobject         void	*
#   define jchar           char
#   define jclass          void	*

#   ifdef CLI_CPP
#       define jstring_ptr  IntPtr
#       define jvoidArray   void	*
#   else
#       define jstring_ptr  const void *
#       define jvoidArray   void	*
#   endif
//#define JNICALL

#   define	DUMBJENV()		void * jenv = 0;
#   define	DUMBJCLASS()	jclass jcls = 0;

#   define FAKEJNI()
#endif



#endif // ENVIRONS_NANDROID_JNI_INTEROP_H
