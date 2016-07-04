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

#include "Environs.Revision.h"
#include "Environs.Build.Macros.h"
#include "Interop/Export.h"


#define	BUILD_MAJOR_VERSION		0 
#define	BUILD_MINOR_VERSION		9 
#define	BUILD_RELEASE_COUNTER	9 


#define BUILD_ATTR1

#ifdef __APPLE__
#	include "TargetConditionals.h"
#endif

//// Declare the symbol NDEBUG for release builds (either in the project settings or as compile C_FLAGS, i.e. -DNDEBUG)
#ifndef NDEBUG
//#	define NDEBUG
#endif


#ifdef NDEBUG
#	define	BUILD_TYPE	" Release"

#	ifdef DEBUGVERB
#		undef DEBUGVERB
#	endif

#	ifdef DEBUGVERBVerb
#		undef DEBUGVERBVerb
#	endif

#else
#	define	BUILD_TYPE	" Debug"

#	include "Environs.Build.Opts.h"

#endif

#define ENVIRONS_VERSION_STRING	ENVIRONS_TOSTRING(BUILD_MAJOR_VERSION) "." ENVIRONS_TOSTRING(BUILD_MINOR_VERSION) "." ENVIRONS_TOSTRING(BUILD_RELEASE_COUNTER) "." ENVIRONS_TOSTRING(BUILD_REVISION) BUILD_TYPE

#define BUILD_VERSION_STRING	"Environs: " ENVIRONS_VERSION_STRING " (" ENVIRONS_TOSTRING(ENVIRONS_BUILD_CRT) ")"



//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif


