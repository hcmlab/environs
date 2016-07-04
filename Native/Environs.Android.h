/**
 * Android JNI platform specific
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
#ifndef INCLUDE_HCM_ENVIRONS_ANDROID_JNI_COMMON_H
#define INCLUDE_HCM_ENVIRONS_ANDROID_JNI_COMMON_H

#ifdef ANDROID
#include "Interop/jni.h"

namespace environs
{
	extern int AttachJavaThread ( JNIEnv *& env );

	extern JavaVM *			g_JavaVM;
	extern jclass			g_EnvironsClass;
	extern jmethodID 		g_onEnvironsNotifyID;
	extern jmethodID 		g_onEnvironsMessageID;
	extern jmethodID 		g_onEnvironsStatusMessageID;
	extern jmethodID 		g_onEnvironsDataID;
	extern jmethodID 		g_onEnvironsOptSaveID;
	extern jmethodID 		g_onEnvironsOptLoadID;

	extern jmethodID 		g_java_mallocID;
	extern jmethodID 		g_java_freeID;

	extern jmethodID 		g_CreateInstance;
	extern jmethodID 		g_Encoder_Init;
	extern jmethodID 		g_DestroyInstance;
    
    bool                    AllocNativePlatform ( );
    
    void                    DetectSDKs ( );
    void                    DetectPlatform ( );

} /* namespace environs */

#endif


#endif  /// end-INCLUDE_HCM_ENVIRONS_ANDROID_JNI_COMMON_H



