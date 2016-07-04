/**
 * Utils and common stuff
 * --------------------------------------------------------------------
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
#ifndef INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H
#define INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H

#include "Interop/Time.Val.h"
#include "Interop.h"

namespace environs
{
	extern void RefactorBuffer ( char * &curStart, char * bufferStart, unsigned int remainingMsg, char * &curEnd );
    
	extern size_t GetSizeOfFile ( CString_ptr filePath );
	extern char * LoadBinary ( const char * fileName, int * size );

	extern char * LoadPrivateBinary ( const char * fileName, int * size );
	extern bool SavePrivateBinary ( const char * fileName, const char * buffer, int size );

	extern unsigned int getRandomValue ( void * value );

	// Number of milliseconds since system has started
	//
	INTEROPTIMEVAL GetEnvironsTickCount ();

    extern unsigned long long   GetUnixEpoch ();
    extern unsigned int GetEnvironsTickCount32 ();
    
#ifdef _WIN32
#	if (!defined(CLI_CPP) && !defined(WINDOWS_PHONE))
    struct ::timeval;
    
    int gettimeofday ( struct timeval *tv, void * );
#	endif

#endif
    
    extern bool CreateDataDirectory ( char * dir );
	extern void CreateCopyString ( const char * src, char ** dest );

#if ( !defined(__APPLE__) && (defined(_WIN32) || defined(LINUX)) )
	size_t strlcpy ( char *d, char const *s, size_t n );

	size_t strlcat ( char *d, char const *s, size_t n );
#endif

    extern void ClearStorage ( const char * dataStore );
    
    
#ifdef __cplusplus
    
    namespace API
    {
        int Environs_LoginDialogCommandLine ( int hInst );
    }
    
#endif
    
//#ifndef _WIN32
//#   ifndef ANDROID
//#       ifndef __APPLE__
//			size_t strlcpy ( char *d, char const *s, size_t n );
//#       endif
//    // <- Linux/iOS includes
//#   endif
//    // <- POSIX includes
//#endif
}



#endif	/// INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H