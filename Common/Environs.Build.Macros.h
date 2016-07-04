/**
* Environs build macro declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_BUILD_MACROS_NATIVE_COMMON_H
#define INCLUDE_HCM_ENVIRONS_BUILD_MACROS_NATIVE_COMMON_H

#define ENVIRONS_TTOSTRING(t)	#t
#define ENVIRONS_TOSTRING(val)	ENVIRONS_TTOSTRING(val)

#define ENVIRONS_BUILD_ARCH		""

#ifdef _M_X64
#	undef ENVIRONS_BUILD_ARCH
#	define ENVIRONS_BUILD_ARCH		"64"
#endif

#ifdef _MSC_PLATFORM_TOOLSET
#	define ENVIRONS_PLATFORM_TOOLSET_LIB	"../../bin" ENVIRONS_TOSTRING(ENVIRONS_BUILD_ARCH) "/libs/v" ENVIRONS_TOSTRING(_MSC_PLATFORM_TOOLSET) "/Environs.lib"
#endif

#endif // INCLUDE_HCM_ENVIRONS_BUILD_MACROS_NATIVE_COMMON_H






