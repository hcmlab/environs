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
#	define WINDOWS_8
#endif

/**
* Dynamic library interoperability
* - function macros
*/
#if defined(_WIN32)
#	if _MSC_VER <= 1600
#		define VS2010
#	endif

// dlload defines
#	ifdef WINDOWS_PHONE
#		define dlsym(mod,sym)				GetProcAddress(mod,sym)
#		define dlopen(mod,RTLD_LAZY)		LoadPackagedLibrary ( (LPWSTR) mod, 0 )
#		define dlclose(mod)					FreeLibrary(mod)
#	else
#		define dlsym(mod,sym)				GetProcAddress(mod,sym)
#		define dlopen(mod,RTLD_LAZY)		LoadLibraryA(mod)
#		define dlclose(mod)					FreeLibrary(mod)
#	endif
#else
#	include <dlfcn.h>
#endif

/**
* Misc declarations
*/
#ifdef ANDROID
#    define  JSTRINGNEW(s)					jenv->NewStringUTF ( s )
#    define  JSTRINGNEW_DISPOSE(s)			jstring nst = jenv->NewStringUTF ( s ); free ( s ); return nst
#else
#    define  JSTRINGNEW(s)					s
#    define  JSTRINGNEW_DISPOSE(s)			return s
#endif


#ifdef CLI_CPP
#   define	___sync_val_compare_and_swap(a,c,v)	System::Threading::Interlocked::CompareExchange(a,v,c)

#	define __sync_add_and_fetch(dest,inc)	System::Threading::Interlocked::Increment(dest)

#	define __sync_sub_and_fetch(dest,inc)	System::Threading::Interlocked::Decrement(dest)

#   define ___sync_val_set(dest,val)        System::Threading::Interlocked::Exchange(dest, val)

#elif (defined(_WIN32))
//
// Note: It's fine to cast from singed to unsigned type as the memory size is the same, 
//		 but semantically we can better see whether some lock counts are unbalanced
//
#	define	___sync_val_compare_and_swap(dest,comp,valset) InterlockedCompareExchange (  (unsigned long volatile *) dest, valset, comp )

#	define __sync_add_and_fetch(dest,inc)	InterlockedIncrement ( (unsigned long volatile *) dest )

#	define __sync_sub_and_fetch(dest,inc)	InterlockedDecrement ( (unsigned long volatile *) dest )

#	define ___sync_test_and_set(dest,val)	InterlockedExchange ( (unsigned long volatile *) dest, val )

#	define ___sync_val_set(dest,val)        InterlockedExchange ( (unsigned long volatile *) dest, val )

#else
#	define ___sync_test_and_set(dest,val) \
								{ long ___ostas; do { ___ostas=*dest; } while (__sync_val_compare_and_swap(dest, ___ostas, val) != ___ostas); }

#   define ___sync_val_set(dest,val)    __sync_lock_test_and_set ( dest, val ); __sync_lock_release ( dest )

#	ifndef ANDROID
#		define	___sync_val_compare_and_swap(dest,comp,valset) \
											__sync_val_compare_and_swap(dest,comp,valset)
#	endif
#endif

#if !defined(CLI_CPP)
#	define virtualNoCLI						virtual
#else
#	define virtualNoCLI
#endif

#if (defined(_WIN32) && !defined(CLI_CPP))
#	define HLIB								HMODULE
#	define virtualW32						virtual
#else
#	define HLIB								void *
#	define virtualW32
#endif

#include "Interop/Time.Val.h"

#	define OBJIDType          				int
#	define OBJIDTypeV          				volatile unsigned int


#if (defined(_WIN32) && !defined(CLI_CPP))
#	define	IWinRelease(IInst)				if (IInst) {IInst->Release(); IInst = 0;}

#	define	CloseWSAHandle_n(h)				if (h != WSA_INVALID_EVENT) WSACloseEvent(h)
#	define	CloseWSAHandle_m(h)				if (h != WSA_INVALID_EVENT) { WSACloseEvent(h); h = NULL; }
#	define	CreateWSAHandle(h,r)			if (h == WSA_INVALID_EVENT) { h = WSACreateEvent(); if (h == WSA_INVALID_EVENT) return r; }
#	define	CreateWSAHandle_n(h)			if (h == WSA_INVALID_EVENT) { h = WSACreateEvent(); }

#	ifdef WINDOWS_PHONE
#		include "minwindef.h"
#		include "wtypes.h"
#		define COBSTR           			LPCWSTR
#		define WNDHANDLE           			void *
#	else /// <- WINDOWS_PHONE
#		ifndef CLI_CPP
#			include "windows.h"
#		endif
#		define COBSTR           			const char *
#		define WNDHANDLE           			HWND
#	endif /// <- WINDOWS_PHONE

#	define LONGSYNC           				LONG volatile
#	define LONGSYNCNV          				LONG

/*
#	ifdef MEDIATORDAEMON
#		define SOCKETSYNC           		LONG volatile
#		define SOCKETSYNCNV           		LONG
#	else
#		define SOCKETSYNC           		int
#		define SOCKETSYNCNV           		int
#	endif
 */
#	define LONGSYNCNV          				LONG

/// Note: EBOOL represents platform specific boolean type.
/// Since Objective-C BOOL conflicts with win32 BOOL, we introduce EBOOL to resolve name conflicts
#	ifdef WINDOWS_PHONE
#		define EBOOL						bool
#		define ToEBOOL(e)					( e != 0 )
#	else
#		define EBOOL						BOOL
#		define ToEBOOL(e)					e
#	endif

#	define EVAL_TO_STRING(s)				#s
#	define EXP_TO_STRING(s)					EVAL_TO_STRING(s)
// __FILE__ ":" EXP_TO_STRING(__LINE__)

#	define WARNING(msg) 					__pragma(message(__FILE__ "(" EXP_TO_STRING(__LINE__) ") : " #msg  ))

#	ifdef WINDOWS_PHONE
#		define INLINEFUNC
#	else
#		define INLINEFUNC					inline
#	endif

#else

#	define	CloseWSAHandle_n(h)
#	define	CloseWSAHandle_m(h)

#	define	CreateWSAHandle(h,r)
#	define	CreateWSAHandle_n(h)

# ifdef CLI_CPP
#	define LONGSYNCNV          				__int64
#	define LONGSYNC           				__int64

#   define SOCKETSYNC           			__int64

#	define WNDHANDLE           				IntPtr
#else
#	define LONGSYNCNV          				long
#	define LONGSYNC           				long

/*#	ifdef MEDIATORDAEMON
#		define SOCKETSYNC           		long volatile
#		define SOCKETSYNCNV           		long
#	else
#		define SOCKETSYNC           		int
#		define SOCKETSYNCNV           		int
#	endif
 */

#	define WNDHANDLE           				void *
#endif

/// NOT _WIN32
#	define COBSTR           				const char *


#	ifdef CLI_CPP
#		define HMODULE						HINSTANCE
#	else
#		define HMODULE						void *
#	endif

#	ifdef __APPLE____
#		define WARNING(msg) 				_Pragma("warning(__FILE__ \"(\" EXP_TO_STRING(__LINE__) \") : \" #msg)")
#	else
#	define WARNING(msg)
#	endif

#	define INLINEFUNC
//#define	__forceinline					inline
#	define	__forceinline

#	if (!defined(TCHAR) && !defined(CLI_CPP))
#		define TCHAR						char
#	endif

#	if defined(ANDROID) || !defined(__APPLE__)
#		define EBOOL						jboolean
#       define ToEBOOL(e)                   e
#		define BSTR							jstring
#	else /// !ANDROID

/// Use bool as default for BOOL otherwise
#	define EBOOL							bool
#	define ToEBOOL(e)                       ( e != 0 )

#	define BSTR								const char *
#	endif  /// end-ANDROID

#	endif

#	ifdef CLI_CPP
#		ifdef EBOOL
#			undef EBOOL
#		endif
#		define EBOOL						bool
#	endif


// Not sure whether these are neccessary

#	if !defined(_WIN32)
#		define HANDLE						void *
#		define TCHAR						char

// for now required for DeviceDisplay.h
#		define CALLBACK
//#	define BOOLEAN							unsigned char
//#	define GetEncoderClsid(a,b)


#	define vsnprintf_s(dest,destSize,maxChar,format,vlist)		vsnprintf ( dest, destSize, format, vlist )
#	define sscanf_s(...)					sscanf(__VA_ARGS__)
#	define strtok_s(buf,delim,context)		strtok_r(buf,delim,context)
#	define localtime_s(a,b)					localtime_r(b,a)

#   ifdef __APPLE__
#       define Sleep(ms)					usleep(ms * 1000)
#   else
#       define Sleep(ms)					usleep(ms * 1000)
#   endif

#	endif // _WIN32



// mathematical C++11 functions reverts to C99 functions for windows
#	if defined(_WIN32)

#		if _MSC_VER < 1700
#			define fmax(a,b)		max(a,b)
#			define fmin(a,b)		min(a,b)
#		endif
#		if _MSC_VER < 1900
#			define snprintf(...)	_snprintf(__VA_ARGS__)
#		endif

#	endif	/// end-_WIN32


#if defined(CLI_CPP)
	using namespace System;
	using namespace System::Runtime::InteropServices;
	using namespace System::Text;

#	define INTERNAL							internal
#	define free_m(m)
#	define free_n(m)
#	define free_plt(m)                      if (m != nill) { environs::API::FreeNativeMemoryN(m); m = nill; }

#	define CLASS							ref class
#	define PUBLIC_CLASS						public ref class
#	define PUBLIC_STRUCT					public value struct
#	define STRUCT							value struct
#	define CLI_ABSTRACT						abstract
#	define c_const
#	define DERIVE_c_only(v)
#	define DERIVE_cli_only(v)				, v
#	define OBJ_ptr							^
#	define cli_OBJ_ptr                      ^
#	define OBJ_p_cli(v,m)					v->m
#	define c_OBJ_ptr
#	define OBJ_ref							%
#	define cli_ref							%
#	define c_ref
#	define CLI_OVERRIDE						override
#	define EPSPACE                          environs::
#	define CLIBSPACE

#	define PlatformIntPtr					IntPtr
#	define StringToPlatformIntPtr(s)		Marshal::StringToHGlobalAnsi ( s )
//#	define StringToCChar(s)					( const char* ) ( Marshal::StringToHGlobalAnsi ( s ) ).ToPointer ()
//#	define DisposePlatCChar(v)				Marshal::FreeHGlobal ( IntPtr ( (void *) v ) )
#	define CCharToString(s)					Marshal::PtrToStringAnsi ( IntPtr ( (void *) (s) ) )

#	define STRING_T							System::String ^
#	define STRING_get(s)					s
#	define STRING_length(s)					s->Length
#	define STRING_empty(s)					System::String::IsNullOrEmpty(s)
#	define STRING_get_cstr(s)				StringToCChar ( s )

	// Returns an IntPtr (of which we call ToPointer())
#	define ToPlatformType(v)				Marshal::StringToHGlobalAnsi ( v )
#	define DisposePlatPointer(iptr)			Marshal::FreeHGlobal ( iptr )
#	define ToPlatAddr(v)					v.ToPointer ()

#	define Addr_ptr							System::IntPtr ^
#	define Addr_obj							System::IntPtr
#	define Addr_pvalue(v)					v->ToPointer()
#	define Addr_pvalued(v)					v.ToPointer()
#	define Addr_of(v)						(%v)
#	define c_Addr_of(v)                     v
#	define Addr_value(v)					v->ToInt32()
#	define CLI_NO_STATIC

#	define CString_ptr						System::String ^
#	define CString_obj						System::String
#	define CString_ptr_empty(s)				STRING_empty(s)
#	define CVString_ptr						System::String ^
#	define String_ptr						System::String ^
#	define String_ptr_reset(s)				s = nill;
#	define CString_length(l)				l->Length
#	define CString_compare(l,r,s)			System::String::Compare ( l, r )
#	define CString_copy(l,s,r)				l = r
#	define CString_contains(s,c)			s->Contains ( c )
#	define CString_get_cstr(s)				s

#	define new__UCharArray(size)            gcnew cli::array<unsigned char>(size)
#	define UCharArray_ptr					cli::array<unsigned char> ^
#	define PIN_PTR(p,t,n)					pin_ptr<t> p = &n [ 0 ]

#	define new__obj(type)					gcnew type()
#	define new__obj2(type,a1,a2)			gcnew type(a1,a2)
#	define delete__obj(o)					o = nill
#	define delete__obj_n(o)					o = nill

// using namespace System::Runtime::InteropServices;
#	define CLI_INC							[System::Security::SuppressUnmanagedCodeSecurityAttribute ()] \
											[DllImport ( "Environs.dll", CallingConvention = CallingConvention::Cdecl )]
#	define CLI_INCM(m)						[System::Security::SuppressUnmanagedCodeSecurityAttribute ()] \
											[DllImport ( "Environs.dll", CallingConvention = CallingConvention::Cdecl, m )]
#	define CLI_OVERRIDE						override
#	define CLI_VIRTUAL						virtual

#	define TCHAR							char
#	define nill								nullptr
#   define C_Only(v)
#   define Cli_Only(v)                      v
#   define Win32_Only(v)

#	define ENVOBSERVER(t1,t2)				event t2 ^
#	define CPP_CLI(t1,t2)					t2

#	define INIT_to_false_in_cli				= false
#	define INIT_to_EXP_in_cli(exp)			= exp
#	define EnvironsAPI(i)					environs::Environs::instancesAPI [ i ]

#   define MAKE_FRIEND_CLASS(c)
#	define DeviceInstancePropertyNotify(i,e)	OnPropertyChanged ( i, e )
//#	define DeviceInstanceNotify(i,f)

#else

#	define INTERNAL							private
#	define free_m(m)						if (m != nill) { free(m); m = nill; }
#	define free_n(m)						if (m != nill) { free(m); }
#	define free_plt(m)                      free_m(m)
#	define CLASS							class
#	define PUBLIC_CLASS						class
#	define PUBLIC_STRUCT					struct
#	define STRUCT							struct
#	define CLI_ABSTRACT
#	define c_const                          const
#	define DERIVE_c_only(v)					: public v
#	define DERIVE_cli_only(v)
#	define OBJ_ptr							*
#	define c_OBJ_ptr                        *
#	define cli_OBJ_ptr
#	define OBJ_p_cli(v,m)					v.m
#	define OBJ_ref							&
#	define cli_ref
#	define c_ref                            &
#	define CLI_OVERRIDE
#	define EPSPACE
#	define CLIBSPACE						environs::lib::

#	define CString_ptr						const char *
#	define CString_obj						const char *
#	define CString_ptr_empty(s)				!(*s)
#	define CVString_ptr						const void *
#	define CString_length(l)				strlen ( l )
#	define CString_compare(l,r,s)			strncmp ( l, r, s )
#	define CString_copy(l,s,r)				strlcpy ( l, r, s )
#	define CString_contains(s,c)			strstr ( s, c )
#	define CString_get_cstr(s)				s.c_str ()

#	define STRING_T							std::string
#	define STRING_empty(s)					s.empty ()
#	define STRING_length(s)					s.length ()
#	define STRING_get(s)					s.c_str ()
#	define STRING_get_cstr(s)				s.c_str ()

#	define PlatformIntPtr					void *
#	define StringToPlatformIntPtr(s)		s
#	define ToPlatformType(v)				( (void *) v )
#	define DisposePlatPointer(v)
#	define ToPlatAddr(v)					v
#	define Addr_ptr							void *
#	define Addr_obj							void *
#	define Addr_pvalue(v)					v
#	define Addr_pvalued(v)					v
#	define Addr_of(v)						(&v)
#	define c_Addr_of(v)                     (&v)
#	define Addr_value(v)					((int) (size_t) v)
#	define CLI_NO_STATIC					static
#	define String_ptr						char *
#	define String_ptr_reset(s)				if (s != nill) { free(s); s = nill; }
#	define new__UCharArray(size)            new unsigned char [ size ]
#	define UCharArray_ptr					unsigned char *
#	define PIN_PTR(p,t,n)					t * p = n

#	define new__obj(type)					new type()
#	define new__obj2(type,a1,a2)			new type(a1,a2)
#	define delete__obj(o)					if (o != nill)  { delete o; o = nill; }
#	define delete__obj_n(o)					delete o;

#	define CLI_INC
#	define CLI_INCM(m)
#	define CLI_OVERRIDE
#	define CLI_VIRTUAL
#	define nill							NULL

#	define StringToCChar(s)					s
#	define DisposePlatCChar(v)
#	define CCharToString(s)					s

#   define C_Only(v)                        v
#   define Cli_Only(v)

#	ifdef _WIN32
#   define Win32_Only(v)                    v
#	define WIN32_NO_CLI
#	else
#   define Win32_Only(v)
#	endif

#	define ENVOBSERVER(t1,t2)				vct ( t1 * )
#	define CPP_CLI(t1,t2)					t1

#	define INIT_to_false_in_cli
#	define INIT_to_EXP_in_cli(exp)
#	define EnvironsAPI(i)					environs::lib::Environs::instancesAPI [ i ]

#   define MAKE_FRIEND_CLASS(c)            friend class c
#	define DeviceInstancePropertyNotify(i,e)
#	define BuildDeviceDisplayProps(i,f)		i
#	define ParsePortalInfo(i)				i

#endif

#ifdef _WIN32
#	ifdef CLI_CPP
#		define NET_PACK_PUSH1
#		define NET_PACK_POP
#		define NET_PACK_ALIGN
#	else
#		define NET_PACK_PUSH1			__pragma(pack(push,1))
#		define NET_PACK_POP				__pragma(pack(pop))
#		define NET_PACK_ALIGN
#	endif
#else
#   define NET_PACK_PUSH1
#   define NET_PACK_POP
#   define NET_PACK_ALIGN               __attribute__ ((packed))
#endif

// always map to IEEE 754
#define float32_t						float
#define double64_t						double


#if defined(_M_IX86) || defined(__i386) || defined(_X86_)|| defined(_M_X64) || defined(__x86_64__)
#define UNALIGNED_MEMORY_ACCESS
#else
#undef UNALIGNED_MEMORY_ACCESS
#endif

#ifdef UNALIGNED_MEMORY_ACCESS
#define GetStructPointerFromBuffer(type, var, buf)		type* var = reinterpret_cast< type* > ( buf );
#else
#define GetStructPointerFromBuffer(type, var, buf)		type _##var; memcpy ( &_##var, buf, sizeof ( type ) ); type* var = &_##var;
#endif


#if !defined(_WIN32) || _MSC_VER < 1800
#	define _Acquires_lock_(a)
#	define _Acquires_exclusive_lock_(a)
#	define _Acquires_nonreentrant_lock_(a)
#	define _Releases_lock_(a)
#	define _Releases_exclusive_lock_(a)
#	define _Releases_nonreentrant_lock_(a)
#	define _Requires_lock_not_held_(a)
#	define _Requires_lock_held_(a)
#	define _When_(a,b)

#	if !defined(_WIN32)
#		define _In_
#		define _In_opt_
#		define _In_z_
#		define _In_opt_z_
#		define _Out_
#		define _Out_opt_
#		define _Inout_
#		define _Inout_opt_
#		define _Outptr_
#		define _Outptr_opt_
#	endif
#endif

#endif // INCLUDE_HCM_ENVIRONS_INTEROP_H
