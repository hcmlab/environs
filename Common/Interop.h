/**
 * Platform interop declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_H

// Determine whether we're compiling for windows 8 and using visual studio > 2010
#if defined(NTDDI_WIN8) && (_MSC_VER >= 1700)
#define WINDOWS_8
#endif

/**
* Dynamic library interoperability
* - function macros
*/
#if defined(_WIN32)
// dlload defines
#define dlsym(mod,sym)					GetProcAddress(mod,sym)     
#define dlopen(mod,RTLD_LAZY)			LoadLibraryA(mod)           
#define dlclose(mod)					FreeLibrary(mod) 
#else
#include <dlfcn.h>
#endif

/**
* Misc declarations
*/

#if defined(_WIN32)

#define	IWinRelease(IInst)				if (IInst) {IInst->Release(); IInst = 0;}

#ifdef WINDOWS_PHONE
#include "minwindef.h"
#include "wtypes.h"
#define COBSTR           				LPCWSTR
#define WNDHANDLE           			void *
#else /// <- WINDOWS_PHONE
#include "windows.h"
#define COBSTR           				const char *
#define WNDHANDLE           			HWND
#endif /// <- WINDOWS_PHONE

#define INTEROPTIMEVAL           		DWORD
#define INTEROPTIMEMS(ms)           	(ms)

/// Note: EBOOL represents platform specific boolean type. 
/// Since Objective-C BOOL conflicts with win32 BOOL, we introduce EBOOL to resolve name conflicts
#ifdef WINDOWS_PHONE
#define EBOOL							bool
#else
#define EBOOL							BOOL
#endif

#define EVAL_TO_STRING(s)				#s
#define EXP_TO_STRING(s)				EVAL_TO_STRING(s)
// __FILE__ ":" EXP_TO_STRING(__LINE__)

#define WARNING(msg) 					__pragma(message(__FILE__ "(" EXP_TO_STRING(__LINE__) ") : " #msg  ))
#define INCLINEFUNC						inline 

#define	___sync_val_compare_and_swap(dest,comp,valset) \
										InterlockedCompareExchange(dest,valset,comp)
#define __sync_add_and_fetch(dest,inc) \
                                        InterlockedIncrement(dest)
#define __sync_sub_and_fetch(dest,inc) \
                                        InterlockedDecrement(dest)
#define ___sync_test_and_set(dest,val) \
                                        InterlockedExchange(dest,val)

#else

#if defined(ANDROID) || defined(__APPLE__)
#define INTEROPTIMEVAL           		uint64_t
#else
#define INTEROPTIMEVAL           		unsigned long long
#endif

/// NOT _WIN32
#define COBSTR           				const char *

#define ___sync_test_and_set(dest,val) \
								{ long ___ostas; do { ___ostas=*dest; } while (__sync_val_compare_and_swap(dest, ___ostas, val) != ___ostas); }

#ifndef ANDROID
#define	___sync_val_compare_and_swap(dest,comp,valset) \
										__sync_val_compare_and_swap(dest,comp,valset)
#endif

#define WNDHANDLE           			void *
#define HMODULE							void *

#ifdef __APPLE____
#define WARNING(msg) 					_Pragma("warning(__FILE__ \"(\" EXP_TO_STRING(__LINE__) \") : \" #msg)")
#else
#define WARNING(msg)
#endif

#define INCLINEFUNC
//#define	__forceinline					inline
#define	__forceinline

#if !defined(TCHAR)
#define TCHAR							char
#endif

#if defined(ANDROID) || !defined(__APPLE__)
#define EBOOL							jboolean
#define BSTR							jstring
#define INTEROPTIMEMS(ms)           	(ms * 1000)
#else /// !ANDROID

/// Use bool as default for BOOL otherwise
#define EBOOL							bool
#define BSTR							const char *
#define INTEROPTIMEMS(ms)           	(ms)
#endif  /// end-ANDROID

#endif




// Not sure whether these are neccessary

#if !defined(_WIN32)
#define HANDLE							void *
#define TCHAR							char

// for now required for DeviceDisplay.h
#define CALLBACK
//#define BOOLEAN							unsigned char
//#define GetEncoderClsid(a,b)


#define vsnprintf_s(dest,destSize,maxChar,format,vlist)		vsnprintf ( dest, destSize, format, vlist )
#define sprintf_s(...)						snprintf(__VA_ARGS__)
#define sscanf_s(...)						sscanf(__VA_ARGS__)
#define strncpy_s(dest,size,source,maxs)	strncpy(dest,source,size)
#define strtok_s(buf,delim,context)			strtok_r(buf,delim,context)
#define localtime_s(a,b)					localtime_r(b,a)
#define Sleep(ms)							usleep(ms * 1000)

#ifdef XCODE
#define strcpy_s(dest,size,source)			strlcpy(dest,source,size)
#define strcat_s(dest,size,source)			strlcat(dest,source,size)
#else
#define strcpy_s(dest,size,source)			strcpy(dest,source)
#define strcat_s(dest,size,source)			strcat(dest,source)
#endif
#endif // _WIN32   



// mathematical C++11 functions reverts to C99 functions for windows
#if defined(_WIN32)

#if _MSC_VER < 1700 
#define fmax(a,b)	max(a,b)
#define fmin(a,b)	min(a,b)
#endif

#endif	/// end-_WIN32

#endif // INCLUDE_HCM_ENVIRONS_INTEROP_H
