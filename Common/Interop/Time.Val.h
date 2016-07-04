/**
 *	Interop stat declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_TIMEVAL_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_TIMEVAL_H

//#ifndef INLINEFUNC
//#	define INLINEFUNC						inline 
//#endif

#if ( defined(_WIN32) )

#	define INTEROPTIMEVAL           		unsigned long long
#	define INTEROPTIMEMS(ms)           		(ms)

#elif CLI_CPP
#	define INTEROPTIMEVAL           		DWORD
#	define INTEROPTIMEMS(ms)           		(ms)
#else
#	if defined(ANDROID) || !defined(__APPLE__)
#		define INTEROPTIMEMS(ms)           	(ms)
#	else
#		define INTEROPTIMEMS(ms)           	(ms)
#	endif

#	if defined(ANDROID) || defined(__APPLE__)
#		define INTEROPTIMEVAL           	uint64_t
#	else
#		define INTEROPTIMEVAL           	unsigned long long
#	endif
#endif

#endif /// INCLUDE_HCM_ENVIRONS_INTEROP_STAT_H
