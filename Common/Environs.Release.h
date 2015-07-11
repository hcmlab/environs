/**
* Environs release native declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_RELEASE_NATIVE_COMMON_H
#define INCLUDE_HCM_ENVIRONS_RELEASE_NATIVE_COMMON_H

#define ENABLE_GPL

#define	BUILD_MAJOR_VERSION		0
#define	BUILD_MINOR_VERSION		7
#define	BUILD_RELEASE_COUNTER	1

#include "Environs.Revision.h"

#define BUILD_ATTR1

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

//// Declare the symbol NDEBUG for release builds (either in the project settings or as compile C_FLAGS, i.e. -DNDEBUG)
#ifndef NDEBUG
//#define NDEBUG
#endif


#ifdef NDEBUG
#define	BUILD_TYPE	" Release"

#ifdef DEBUGVERB
#undef DEBUGVERB
#endif

#ifdef DEBUGVERBVerb
#undef DEBUGVERBVerb
#endif

#else
#define	BUILD_TYPE	" Debug"

#ifdef _WIN32
#ifdef MEDIATORDAEMON
#define _USE_VLD
#endif
//#define _USE_VLD
//#define USE_CRT_MLC

#ifdef _USE_VLD
#define _C_X64
#endif

#ifdef _USE_VLD
#ifdef _C_X64
#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#else
#include "C:/Program Files/Visual Leak Detector/include/vld.h"
#endif
#endif

#ifdef USE_CRT_MLC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#endif

#endif

#define ENVIRONS_TTOSTRING(t)	#t
#define ENVIRONS_TOSTRING(val)	ENVIRONS_TTOSTRING(val)

#define ENVIRONS_VERSION_STRING	ENVIRONS_TOSTRING(BUILD_MAJOR_VERSION) "." ENVIRONS_TOSTRING(BUILD_MINOR_VERSION) "." ENVIRONS_TOSTRING(BUILD_RELEASE_COUNTER) "." ENVIRONS_TOSTRING(BUILD_REVISION) BUILD_TYPE

#define BUILD_VERSION_STRING	"Environs: " ENVIRONS_VERSION_STRING



//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif


