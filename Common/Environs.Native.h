/**
 * Common native declarations
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

/**
 * Build flags
 * ****************************************************************************************
 */

//#define DEBUGVERB
//#define DEBUGVERBVerb
//#define DEBUGVERBLocks
//#define DEBUGCIPHERS
//#define DEBUGVERBList
//#define DEBUGVERBListener
//#define DEBUGVERBVerbCrypt

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Environs.Build.Macros.h"

/**
 * Include native type and data structure declarations
 * ****************************************************************************************
 */
#include "Environs.Types.h"

#ifdef ANDROID
#	include <android/log.h>
#else
// Logging/Tracing/Debug defines for Windows/Linux/iOS devices
#	ifndef WINDOWS_PHONE
#		include <stdlib.h>
#       ifdef DEBUG_EXT_HEAP_CHECK
#           include <crtdbg.h>
#       endif
#		include <string.h>
#	endif
#endif

/**
 * Log macros and forwards
 *
 **/
#include "Log.h"

#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H


/// Fake 42 test device entries for device information retrieval through the mediator
//#define FAKE42

/*
#if !defined(NDEBUG)
#   if defined(_WIN32)
#       define free_d(p) _free_dbg(p, _NORMAL_BLOCK); *(int*)&p = 0x1234;
#   else
#       define free_d(p) free(p); *(int*)&p = 0x1234;
#   endif
#endif
 */

#ifdef __cplusplus

namespace environs {
    extern void   *   pEnvirons;
	extern void   *   pNative;
}

#endif

/**
* Helper, Wrapper
* ****************************************************************************************
*/
#define		OBJ_RELEASE(obj)					if(obj) { obj->Release(); obj = 0; }

#define		Zero(mem)							memset(&mem,0,sizeof(mem))
#define		ZeroT(mem,type)						memset(mem,0,sizeof(type))

#ifdef _WIN32
#	define	Zeroh(mem)							mem = nill
#else
#	define	Zeroh(mem)							Zero(mem)
#endif

#ifdef CLI_CPP
#	define	Zeros(mem)							mem = nill
#else
#	define	Zeros(mem)							Zero(mem)
#endif

//
// Attention when using EmptyStructValue(), ZeroStruct() and ZeroStructArray():
//
// VS <= 2013 (VS 2015 is fine) does not comply to the standard,
// and will not initialize member variables, if "type" has an explicitly defined destructor,
// but no explicitly defined default constructor.
// http://stackoverflow.com/questions/3931312/value-initialization-and-non-pod-types
//
// When using these macros:
// explicit default constructor +  explicit destructor: OK
// explicit default constructor +  implicit destructor: OK
// implicit default constructor +  explicit destructor: NOT_OK !! -> add explicit default constructor
// implicit default constructor +  implicit destructor: OK
//
// Simple rule: When "type" has an user-defined destructor or has non-POD members add a default constructor,
// that initializes all member variables.

#ifdef VS2010
#	define	EmptyStructValue(type)				type()
#	define	EmptyStructZero(type)				type()
#else
#	define	EmptyStructValue(type)				type{}
#	define	EmptyStructZero(type)				{ }
#endif

#ifdef VS2010
// On VS2010 we know that floats can be memset and since the alternative is to call the default constructor
// we can also call memset (which is likely to be optimized by the compiler)
#	define	ZeroStruct(var,type)				Zero ( var )
#	define	ZeroStructStd(var,type)				var = type()
#else
#	define	ZeroStruct(var,type)				var = { }
#	define	ZeroStructStd(var,type)				type{}
#endif

#ifdef VS2010
#	define	ZeroStructArray(var,type)			std::fill(std::begin(var), std::end(var), type())
#else
#	define	ZeroStructArray(var,type)			std::fill(std::begin(var), std::end(var), type{})
#endif

#define		StackObject(type,obj)				type obj; memset(&obj,0,sizeof(obj))

#define		GetPortalIndex(p)					(p & 0xFF)
#define		RemovePortalIndex(p)				(p & 0xFFFFFF00)
#define		GetPortalDeviceID(p)				((p >> 24) & 0xFF)
#define		IsInvalidPortalDeviceID(p)			(p < 0 || p >= MAX_PORTAL_INSTANCES )
#define		IsInvalidPortalIndex(p)				(p < 0 || p >= MAX_PORTAL_STREAMS_A_DEVICE )
#define		IsValidPortalIndex(p)				(p >= 0 && p < MAX_PORTAL_STREAMS_A_DEVICE )
#define		PortalIndex()						(portalID & 0xFF)
#define		IsPortalGenerator()					(portalID & PORTAL_DIR_OUTGOING)
#define		IsPortalIDGenerator(p)				(p & PORTAL_DIR_OUTGOING)
#define		IsPortalReceiver()					(portalID & PORTAL_DIR_INCOMING)
#define		IsPortalIDReceiver()				(p & PORTAL_DIR_INCOMING)
#define		ClearPortalDir()					(portalID &= ~(PORTAL_DIR_INCOMING | PORTAL_DIR_OUTGOING))
#define     ReversePortalDirGet()				((portalID & ~PORTAL_DIR_MASK) | (~portalID & PORTAL_DIR_MASK))
#define     ReversePortalDir(portalID)          portalID = ((portalID & ~PORTAL_DIR_MASK) | (~portalID & PORTAL_DIR_MASK))
#define		ClearPortalDeviceID(p)				(p & 0xFFFFFF)

#define     IsStatus(n)                         (n < 0 && (n & 0xF))
#define     IsNotStatus(n)                      (n > 0 || (n & 0xF) == 0)
#define     GetStatusValue(n)                         (0 - n)

#define		ClearBit(val,mask)					(val &= ~mask)
#define		AddBit(val,mask)					(val |= mask)
#define		GetPad(val,align)					((val % align) ? (align - (val % align)) : 0)

#define		OBJECTSTATE_DELETEABLE_1			1
#define		OBJECTSTATE_DELETEABLE_2			2
#define		OBJECTSTATE_DELETED					0

//#define     OSX_USE_MANUAL_REF_COUNT

#ifdef OSX_USE_MANUAL_REF_COUNT
#   define IOSX_SUPER_DEALLOC()                 [super dealloc]
#else
#   define IOSX_SUPER_DEALLOC()
#endif

#ifdef NDEBUG
#   ifndef _EnvDebugBreak
#       define _EnvDebugBreak(msg)
#   endif
#else
    namespace environs {
        extern void _EnvDebugBreak ( const char * msg );
    }
#endif


// Defines
#define		TCP_RECV_CONNECT_BUFFER_SIZE_BEGIN	1024
#define		MSG_BUFFER_SEND_SIZE				1024
#define 	REC_BUF_SIZE						256000
#define 	REC_BUF_SIZE_MAX					2000000
#define		UDP_MAX_SIZE						66000
#define		UDP_DEVICEBASE_MAX_SIZE				4096
#define		TCP_DEVICEBASE_START_SIZE			4096
#define		DATA_BUFFER_COUNT					5
#define		DATA_BUFFER_SIZE					1000000
#define		STREAM_BUFFER_SIZE_MIN				100000
#define		TCP_MSG_PROTOCOL_VERSION			3
#define		UDP_MSG_PROTOCOL_VERSION			3
#define		UDP_MSG_EXT_PROTOCOL_VERSION		4

//#define     NATIVE_DEFAULT_TCP_PORT				5901
//#define     NATIVE_DEFAULT_UDP_PORT				5901

//#define     NATIVE_DEFAULT_TCP_PORT				5656
//#define     NATIVE_DEFAULT_UDP_PORT				5656

#define     NATIVE_DEFAULT_DEVICE_PORT			5959
#define     NATIVE_DEFAULT_BASE_PORT			5899

#ifdef MEDIATORDAEMON
#   define  GET_MEDIATOR_BASE_PORT              NATIVE_DEFAULT_BASE_PORT
#else
#   define  GET_MEDIATOR_BASE_PORT              env->basePort
#endif

#define     ENVIRONS_DEBUG_TAGID				0xDD

#define		PORTAL_SOCKET_BUFFER_SIZE_NORMAL	30000
#define		PORTAL_SOCKET_BUFFER_SIZE_NATRES	90000


#ifndef PI
#	define PI 3.14159265
#endif

#ifdef DEBUGVERB
#   define ASSERT_ENV(exp,fn,msg)   if ( !(exp) ) CErr (fn ": exp(" ENVIRONS_TOSTRING(exp) ") " msg);
#else
#   define ASSERT_ENV(exp,fn,msg)
#endif

#ifdef ANDROID
#	define	ENVIRONS_LOG_NL
#else
#	define	ENVIRONS_LOG_NL         "\n"
#endif



#ifdef ANDROID
#	define ENVIRONS_LOG_NRCMD(tag,expression)					__android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", expression )
#	define ENVIRONS_LOGARG_NRCMD(tag,expression,...)			__android_log_print ( expression, __VA_ARGS__ )
#else
#	ifdef _WIN32
#		define ENVIRONS_LOG_NRCMD(tag,expression)				OutputDebugStringA ( expression )
#		define ENVIRONS_LOGARG_NRCMD(tag,expression,...)		OutputDebugStringA ( expression ) //, __VA_ARGS__ )
#	else
#		if (defined(__APPLE__) && defined(USE_NSLOG))
#			define ENVIRONS_LOG_NRCMD(tag,expression)			NSLog ( @"%s", expression )
#			define ENVIRONS_LOGARG_NRCMD(tag,expression,...)	NSLog ( @expression, __VA_ARGS__ )
#		else
#			define ENVIRONS_LOG_NRCMD(tag,expression)			printf ( expression )
#			define ENVIRONS_LOGARG_NRCMD(tag,expression,...)	printf ( expression, __VA_ARGS__ )
#		endif
#	endif
#endif


#ifdef ENVIRONS_CORE_LIB


#   ifdef ANDROID
#       define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( tag, expression, 0, true )
#       define ENVIRONS_LOGARG_RCMD(tag,expression,...)         environs::COutArgLog ( tag, expression, __VA_ARGS__ )
#   elif CLI_CPP
#       define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( CLASS_NAME, expression )
#       define ENVIRONS_LOG_RCMD_SBL(logtxt)					environs::COutLog ( logtxt )
#       define ENVIRONS_LOGARG_RCMD(tag,expression)             environs::COutLog ( CLASS_NAME, expression )
#   else
#       define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( expression, 0, true )
#       define ENVIRONS_LOGARG_RCMD(tag,expression,...)         environs::COutArgLog ( expression, __VA_ARGS__ )
#   endif

#else // not environs core lib follows


#   ifdef MEDIATORDAEMON
namespace environs
{
	extern void MLog ( const char * msg );
	extern void MLogArg ( const char * msg, ... );
}

#       define ENVIRONS_LOG_RCMD(tag,expression)                    MLog ( expression )
#       define ENVIRONS_LOGARG_RCMD(tag,expression,...)             MLogArg ( expression, __VA_ARGS__ )
#   else

#       ifdef USE_ENVIRONS_LOG_POINTERS
#           ifdef ANDROID
#               define ENVIRONS_LOG_RCMD(tag,expression)            { if (COutLog) COutLog ( expression, 0, true ); }
#               define ENVIRONS_LOGARG_RCMD(tag,expression,...)     { if (COutArgLog) COutArgLog ( expression, __VA_ARGS__ ); }
#           else
#               define ENVIRONS_LOG_RCMD(tag,expression)            { if (COutLog) COutLog ( expression, 0, true ); }
#               define ENVIRONS_LOGARG_RCMD(tag,expression,...)     { if (COutArgLog) COutArgLog ( expression, __VA_ARGS__ ); }
#           endif
#       else
#           ifdef USE_STATIC_ENVIRONS_LOG
#               define ENVIRONS_LOG_RCMD(tag,expression)            environs::COutLog ( expression, 0, true )
#               define ENVIRONS_LOGARG_RCMD(tag,expression,...)     environs::COutArgLog ( expression, __VA_ARGS__ )
#           else
#               ifdef ANDROID
#                   define ENVIRONS_LOG_RCMD(tag,expression)        { if (pNative) ((environs::EnvironsNative *) pNative)->cOutLog ( tag, expression, 0, true ); }
#                   define ENVIRONS_LOGARG_RCMD(tag,expression,...) { if (pNative) ((environs::EnvironsNative *) pNative)->cOutArgLog ( tag, expression, __VA_ARGS__ ); }
#               else
#                   define ENVIRONS_LOG_RCMD(tag,expression)        { if (pNative) ((environs::EnvironsNative *) pNative)->cOutLog ( expression, 0, true ); }
#                   define ENVIRONS_LOGARG_RCMD(tag,expression,...) { if (pNative) ((environs::EnvironsNative *) pNative)->cOutArgLog ( expression, __VA_ARGS__ ); }
#               endif
#           endif
#       endif
#   endif

#endif // <-- ENVIRONS_CORE_LIB



#define ENVIRONS_LOG_TAG_ID							"Environs"
#define ENVIRONS_VERB_PREFIX						"[VERB]  "
#define ENVIRONS_LOG_PREFIX							"[TRACE] "
#define ENVIRONS_INFO_PREFIX						"[INFO]  "
#define ENVIRONS_WARN_PREFIX						"[WARN]  "
#define ENVIRONS_ERR_PREFIX							"[ERROR] -- E -->: "
#define ENVIRONS_LOCK_PREFIX						"[VERB] --> Lock   "
#define ENVIRONS_UNLOCK_PREFIX						"[VERB] <-- Unlock "

#define	ENVIRONS_LOGTAG_INFO						ANDROID_LOG_INFO
#define	ENVIRONS_LOGTAG_WARN						ANDROID_LOG_WARN
#define	ENVIRONS_LOGTAG_ERROR						ANDROID_LOG_ERROR
#define	ENVIRONS_LOGTAG_VERBOSE						ANDROID_LOG_VERBOSE

#ifdef CLI_CPP
#	define	ENVIRONS_MAKE_BODY(tag,msg)				tag, msg
#	define	ENVIRONS_MAKE_BODY_SBL(tag,msg)			sbl->Append(tag)->Append(" ")->Append(CLASS_NAME)->Append(" ") msg
#	define	ANDROID_LOG_INFO						1
#	define	ANDROID_LOG_WARN						1
#	define	ANDROID_LOG_ERROR						1
#	define	ANDROID_LOG_VERBOSE						1
#else
#	define	ENVIRONS_MAKE_BODY(tag,msg)				tag " " CLASS_NAME " " msg ENVIRONS_LOG_NL
#	define	ENVIRONS_MAKE_BODY_ID(tag,msg)			tag " " CLASS_NAME " [ %12X ] " msg ENVIRONS_LOG_NL
#endif

#define ENVIRONS_VERB_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )

#ifdef CLI_CPP
#	define ENVIRONS_VERB_SBL_CMD(expression)		{ StringBuilder ^ sbl = gcnew StringBuilder (); expression;	ENVIRONS_LOG_RCMD_SBL ( sbl->ToString() ); }

#	define DLEVEL(level)                            if ( environs::Utils::logLevel >= level )
#else
#	define DLEVEL(level)                            if ( g_Debug >= level )
#endif


#define ENVIRONS_VERB_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_LOG_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_LOG_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_INFO_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_INFO,		expression )
#define ENVIRONS_WARN_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_WARN,		expression )
#define ENVIRONS_ERR_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_ERROR,		expression )
#define ENVIRONS_ERR_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_ERROR,		expression )

#define ENVIRONS_VERBRG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_VERBRG_NCMD(expression,...)		ENVIRONS_LOGARG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_LOGARG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_INFOARG_CMD(expression,...)		ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_INFO,	expression, __VA_ARGS__ )
#define ENVIRONS_WARNARG_CMD(expression,...)		ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_WARN,	expression, __VA_ARGS__ )
#define ENVIRONS_ERRARG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_ERROR,	expression, __VA_ARGS__ )

#define CVerb(msg)									ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbs(level,msg)							CVerb ( msg )

#define CVerbN(msg)									ENVIRONS_VERB_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbVerb(msg)								ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbVerbN(msg)								ENVIRONS_VERB_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CLog(msg)									ENVIRONS_LOG_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CListLog(msg)								ENVIRONS_LOG_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CLogN(msg)									ENVIRONS_LOG_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CInfo(msg)									ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_INFO_PREFIX,	msg ) )
#define CWarn(msg)									ENVIRONS_WARN_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_WARN_PREFIX,	msg ) )
#define CWarns(level,msg)                           DLEVEL ( level ) { CWarn ( msg ); }

#define CErr(msg)									ENVIRONS_ERR_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg ) )
#define CErrN(msg)									ENVIRONS_ERR_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg ) )

#define CVerbArg(msg,...)							ENVIRONS_VERBRG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#define CVerbsArg(level,msg,...)					CVerbArg ( msg, __VA_ARGS__ )
#define ElseCVerbsArg(level,msg,...)				else { CVerbArg ( msg, __VA_ARGS__ ); }

#define CVerbArgN(msg,...)							ENVIRONS_VERBRG_NCMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )

#define CVerbVerbArg(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#define CVerbsVerbArg(level,msg,...)                CVerbVerbArg ( msg, __VA_ARGS__ )


#ifdef CLI_CPP
#	define CVerbVerbArg1(msg,name1,type1,arg1)		ENVIRONS_VERB_SBL_CMD( ENVIRONS_MAKE_BODY_SBL( ENVIRONS_VERB_PREFIX, ->Append(msg)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ]") ) )

#	define CVerbVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERB_SBL_CMD	( ENVIRONS_MAKE_BODY_SBL	( ENVIRONS_VERB_PREFIX,	\
	->Append(msg)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ] ") \
	->Append(name2)->Append(": [ ")->Append(arg2)->Append(" ]") ) )
#else
#	define CVerbVerbArg1(msg,name1,type1,arg1)		ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [ %" type1 " ]" ), arg1 )

#	define CVerbVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [ %" type1 " ] " name2 ": [ %" type2 " ] " ), arg1, arg2 )
#endif

#define CLogArg(msg,...)							ENVIRONS_LOGARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )
#define CLogsArg(level,msg,...)                     DLEVEL ( level ) { CLogArg ( msg, __VA_ARGS__ ); }

#define CListLogArg(msg,...)						ENVIRONS_LOGARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )
#define CLogArgN(msg,...)							ENVIRONS_VERBRG_NCMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )


#ifdef CLI_CPP
#	define CLogArg1(msgFormat,name1,type1,arg1)		\
		ENVIRONS_VERB_SBL_CMD ( ENVIRONS_MAKE_BODY_SBL ( ENVIRONS_LOG_PREFIX, \
			->Append(msgFormat)->Append(" ")->Append(name1)->Append(": [")->Append(arg1)->Append("]") ) )

#	define CLogArg2(msgFormat,name1,type1,arg1,name2,type2,arg2)		\
		ENVIRONS_VERB_SBL_CMD ( ENVIRONS_MAKE_BODY_SBL ( ENVIRONS_LOG_PREFIX,	\
			->Append(msgFormat)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ] ") \
			->Append(name2)->Append(": [ ")->Append(arg2)->Append(" ]") ) )

#else
#	define CLogArg1(msgFormat,name1,type1,arg1)			ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msgFormat " " name1 ": [ %" type1 " ]" ), arg1 )

#	define CLogArg2(msgFormat,name1,type1,arg1,name2,type2,arg2)		\
		ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msgFormat " " name1 ": [ %" type1 " ] " name2 ": [ %" type2 " ]" ), arg1, arg2 )
#endif


#ifdef CLI_CPP
#	define CVerbArg1(msg,name1,type1,arg1)			ENVIRONS_VERB_SBL_CMD( ENVIRONS_MAKE_BODY_SBL( ENVIRONS_VERB_PREFIX, ->Append(msg)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ]") ) )

#	define CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERB_SBL_CMD	( ENVIRONS_MAKE_BODY_SBL	( ENVIRONS_VERB_PREFIX,	\
	->Append(msg)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ] ") \
	->Append(name2)->Append(": [ ")->Append(arg2)->Append(" ]") ) )

#	define CVerbsArg2(level,msg,name1,type1,arg1,name2,type2,arg2)	CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2);
#	define ElseCVerbsArg2(level,msg,name1,type1,arg1,name2,type2,arg2)	else { CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2); }
#	define CVerbsArg1(level,msg,name1,type1,arg1)					CVerbArg1(msg,name1,type1,arg1);
#	define ElseCVerbsArg1(level,msg,name1,type1,arg1)				else { CVerbArg1(msg,name1,type1,arg1); }
#else
#	define CVerbArg1(msg,name1,type1,arg1)			ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [ %" type1 " ]" ), arg1 )

#	define CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
		ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [ %" type1 " ] " name2 ": [ %" type2 " ] " ), arg1, arg2 )

#	ifdef __APPLE__
#		define CVerbsArg2(level,...)				DLEVEL ( level ) { CLogArg2(__VA_ARGS__); }
#		define ElseCVerbsArg2(level,...)			else { DLEVEL ( level ) { CLogArg2(__VA_ARGS__); } }
#		define CVerbsArg1(level,...)				DLEVEL ( level ) { CLogArg1(__VA_ARGS__); }
#		define ElseCVerbsArg1(level,...)			else { DLEVEL ( level ) { CLogArg1(__VA_ARGS__); } }
#	else
#		define CVerbsArg2(level,msg,name1,type1,arg1,name2,type2,arg2)	DLEVEL ( level ) { CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2); }
#		define ElseCVerbsArg2(level,msg,name1,type1,arg1,name2,type2,arg2)	else { DLEVEL ( level ) { CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2); } }
#		define CVerbsArg1(level,msg,name1,type1,arg1)					DLEVEL ( level ) { CVerbArg1(msg,name1,type1,arg1); }
#		define ElseCVerbsArg1(level,msg,name1,type1,arg1)				else { DLEVEL ( level ) { CVerbArg1(msg,name1,type1,arg1); } }
#	endif
#endif


#define CListLogArg1(msg,name1,type1,arg1)			CLogArg1 ( msg,name1,type1,arg1)

#define CInfoArg(msg,...)							ENVIRONS_INFOARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_INFO_PREFIX,	msg), __VA_ARGS__ )
#define CWarnArg(msg,...)							ENVIRONS_WARNARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_WARN_PREFIX,	msg), __VA_ARGS__ )
#define CWarnsArg(level,msg,...)					CWarnArg(msg, __VA_ARGS__)
#define CErrArg(msg,...)							ENVIRONS_ERRARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg), __VA_ARGS__ )
#define CErrsArg(level,msg,...)                     DLEVEL ( level ) { CErrArg ( msg, __VA_ARGS__); }


#ifdef CLI_CPP
#	define CErrArg1(msg,name1,type1,arg1)			ENVIRONS_VERB_SBL_CMD( ENVIRONS_MAKE_BODY_SBL( ENVIRONS_ERR_PREFIX, ->Append(msg)->Append(" ")->Append(name1)->Append(": [")->Append(arg1)->Append("]") ) )

#	define CErrArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERB_SBL_CMD	( ENVIRONS_MAKE_BODY_SBL	( ENVIRONS_ERR_PREFIX,	\
	->Append(msg)->Append(" ")->Append(name1)->Append(": [ ")->Append(arg1)->Append(" ] ") \
	->Append(name2)->Append(": [ ")->Append(arg2)->Append(" ]") ) )

#else
#	define CErrArg1(msg,name1,type1,arg1)			ENVIRONS_ERRARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg " " name1 ": [ %" type1 " ]" ), arg1 )

#	define CErrArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
		ENVIRONS_ERRARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg " " name1 ": [ %" type1 " ] " name2 ": [ %" type2 " ] " ), arg1, arg2 )
#endif
#define CErrsArg2(level,msg,name1,type1,arg1,name2,type2,arg2)  DLEVEL ( level ) { CErrArg2 ( msg, name1, type1, arg1, name2, type2, arg2 ); }


#define CVerbID(msg)								ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), deviceID )
#define CVerbsID(level,msg)							CVerbID(msg)

#define CVerbVerbID(msg)							ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), deviceID )
#define CLogID(msg)									ENVIRONS_LOGARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg ), deviceID )
#define CInfoID(msg)								ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg ), deviceID )
#define CInfosID(level,msg)                         DLEVEL ( level ) { CInfoID ( msg ); }

#define CWarnID(msg)								ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg ), deviceID )
#define CWarnsID(level,msg)							DLEVEL ( level ) { CWarnID( msg ); }

#define CErrID(msg)									ENVIRONS_ERRARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg ), deviceID )
#define CErrsID(level,msg)                          DLEVEL ( level ) { CErrID ( msg ); }

#ifdef CLI_CPP
#	define CVerbIDN(msg)							CVerb(msg)
#	define CVerbVerbIDN(msg)						CVerb(msg)
#	define CLogIDN(msg)								CLog(msg)
#	define CInfoIDN(msg)							CInfo(msg)
#	define CWarnIDN(msg)							CWarn(msg)
#	define CErrIDN(msg)								CErr(msg)
#else
#	define CVerbIDN(msg)							ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), nativeID )
#	define CVerbVerbIDN(msg)						ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), nativeID )
#	define CLogIDN(msg)								ENVIRONS_LOGARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg ), nativeID )
#	define CInfoIDN(msg)							ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg ), nativeID )
#	define CWarnIDN(msg)							ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg ), nativeID )
#	define CErrIDN(msg)								ENVIRONS_ERRARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg ), nativeID )
#endif


#define CVerbArgID(msg,...)							ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CVerbsArgID(level,msg,...)					CVerbArgID ( msg, __VA_ARGS__ )
#define ElseCVerbsArgID(level,msg,...)				else { CVerbArgID ( msg, __VA_ARGS__ ); }

#define CVerbVerbArgID(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CVerbsVerbArgID(level,msg,...)				CVerbVerbArgID ( msg, __VA_ARGS__ )

#define CLogArgID(msg,...)							ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CLogsArgID(level,msg,...)                   DLEVEL ( level ) { CLogArgID ( msg, __VA_ARGS__); }


#define CInfoArgID(msg,...)							ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CWarnArgID(msg,...)							ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CWarnsArgID(level,msg,...)					DLEVEL ( level ) { CWarnArgID ( msg, __VA_ARGS__); }

#define CErrArgID(msg,...)							ENVIRONS_ERRARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define ElseCErrArgID(msg,...)						else { CErrArgID ( msg, __VA_ARGS__); }
#define CErrsArgID(level,msg,...)					DLEVEL ( level ) { CErrArgID ( msg, __VA_ARGS__); }

#define CVerbArgIDN(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CVerbVerbArgIDN(msg,...)					ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CLogArgIDN(msg,...)							ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CInfoArgIDN(msg,...)						ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CWarnArgIDN(msg,...)						ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CErrArgIDN(msg,...)							ENVIRONS_ERRARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define ElseCErrArgIDN(msg,...)						else { CErrArgIDN ( msg, __VA_ARGS__); }

#define CVerbLock(msg)								ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOCK_PREFIX,	msg ) )
#define CVerbLockArg(msg,...)						ENVIRONS_VERBRG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#define CVerbUnLock(msg)							ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_UNLOCK_PREFIX, msg ) )

#define CVerbLockPortalRecRes(msg)					ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOCK_PREFIX,	msg ": resourcesMutex" ) )
#define CVerbUnLockPortalRecRes(msg)				ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_UNLOCK_PREFIX, msg ": resourcesMutex" ) )


#if (!defined(DEBUGVERBList) || defined(NDEBUG))
#	undef	CListLog
#	define  CListLog(msg)
#	undef	CListLogArg
#	define  CListLogArg(msg,...)
#	undef	CListLogArg1
#	define  CListLogArg1(msg,...)
#endif

#if (!defined(DEBUGVERB) )
#   undef	CVerbN
#   undef	CVerb
#   undef	CVerbs
#   undef	CVerbArg
#   undef	CVerbsArg
#   undef	ElseCVerbsArg
#   undef	CVerbArg1
#   undef	CVerbArg2
#   undef	CVerbArgN
#   undef	CVerbID
#   undef	CVerbsID
#   undef	CVerbIDN
#   undef	CVerbArgID
#   undef	CVerbsArgID
#   undef	ElseCVerbsArgID
#   undef	CVerbArgIDN

#   if defined(NDEBUG)
#       define  CVerbN(msg)
#       define  CVerb(msg)
#       define  CVerbs(level,msg)
#       define  CVerbArg(msg,...)
#       define  CVerbsArg(level,msg,...)
#       define  ElseCVerbsArg(level,msg,...)
#       define  CVerbArg1(msg,...)
#       define  CVerbArg2(msg,...)
#       define  CVerbArgN(msg,...)
#       define  CVerbID(msg)
#       define  CVerbsID(level,msg)
#       define  CVerbIDN(msg)
#       define  CVerbArgID(msg,...)
#       define  CVerbsArgID(level,msg,...)
#       define  ElseCVerbsArgID(level,msg,...)
#       define  CVerbArgIDN(msg,...)
#   else
#       define  CVerbN(msg)
#       define  CVerb(msg)
#       define  CVerbs(level,msg)                           DLEVEL ( level ) { CLog ( msg ); }
#       define  CVerbArg(msg,...)
#       define  CVerbsArg(level,msg,...)                    DLEVEL ( level ) { CLogArg ( msg, __VA_ARGS__ ); }
#       define  ElseCVerbsArg(level,msg,...)                else { DLEVEL ( level ) { CLogArg ( msg, __VA_ARGS__ ); } }
#       define  CVerbArg1(msg,...)
#       define  CVerbArg2(msg,...)
#       define  CVerbArgN(msg,...)
#       define  CVerbID(msg)
#       define  CVerbsID(level,msg)                         DLEVEL ( level ) { CLogID ( msg ); }
#       define  CVerbIDN(msg)
#       define  CVerbArgID(msg,...)
#       define  CVerbsArgID(level,msg,...)					DLEVEL ( level ) { CLogArgID ( msg, __VA_ARGS__ ); }
#       define  ElseCVerbsArgID(level,msg,...)				else { DLEVEL ( level ) { CLogArgID ( msg, __VA_ARGS__ ); } }
#       define  CVerbArgIDN(msg,...)
#   endif
#endif

#if (!defined(DEBUGVERBVerb))
#   undef	CVerbVerb
#   undef	CVerbVerbN
#   undef	CVerbVerbArg
#   undef	CVerbsVerbArg
#   undef	CVerbVerbArg1
#   undef	CVerbVerbArg2
#   undef	CVerbVerbID
#   undef	CVerbVerbIDN
#   undef   CVerbsVerbArgID
#   undef	CVerbVerbArgID
#   undef	CVerbVerbArgIDN

#   if (defined(NDEBUG))
#       define  CVerbVerb(msg)
#       define  CVerbVerbN(msg)
#       define  CVerbVerbArg(msg,...)
#       define  CVerbsVerbArg(level,msg,...)
#       define  CVerbVerbArg1(msg,...)
#       define  CVerbVerbArg2(msg,...)
#       define  CVerbVerbID(msg)
#       define  CVerbVerbIDN(msg)
#       define  CVerbsVerbArgID(level,msg,...)
#       define  CVerbVerbArgID(msg,...)
#       define  CVerbVerbArgIDN(msg,...)
#   else
#       define  CVerbVerb(msg)
#       define  CVerbVerbN(msg)
#       define  CVerbVerbArg(msg,...)
#       define  CVerbsVerbArg(level,msg,...)                DLEVEL ( level ) { CLogArg ( msg, __VA_ARGS__ ); }
#       define  CVerbVerbArg1(msg,...)
#       define  CVerbVerbArg2(msg,...)
#       define  CVerbVerbID(msg)
#       define  CVerbVerbIDN(msg)
#       define  CVerbsVerbArgID(level,msg,...)				DLEVEL ( level ) { CLogArgID ( msg, __VA_ARGS__ ); }
#       define  CVerbVerbArgID(msg,...)
#       define  CVerbVerbArgIDN(msg,...)
#   endif
#endif



#if (!defined(DEBUGVERBLocks) || defined(NDEBUG))
#	undef	CVerbLock
#	define  CVerbLock(msg)
#	undef	CVerbUnLock
#	define  CVerbUnLock(msg)
#	undef	CVerbLockPortalRecRes
#	define  CVerbLockPortalRecRes(msg)
#	undef	CVerbUnLockPortalRecRes
#	define  CVerbUnLockPortalRecRes(msg)
#endif

#if ( !defined(DEBUG_TRACK_SOCKET) || defined(NDEBUG) )
#	undef	CSocketLog
#	define  CSocketLog(msg)
#	undef	CSocketTraceAdd
#	define  CSocketTraceAdd(s,msg)
#	define  CSocketTraceUpdate(s,msg)
#	define  CSocketTraceUpdateCheck(s,msg)
#	define  CSocketTraceVerbUpdate(s,msg)   
#	define  CSocketTraceRemove(s,msg,src)
#else
#	ifdef DEBUG_TRACK_SOCKET
#		define	CSocketLog(msg)						
#		define  CSocketTraceAdd(s,msg)              TraceSocket ( s, msg )
#		define  CSocketTraceUpdate(s,msg)           TraceSocketUpdate ( s, msg )
#		define  CSocketTraceUpdateCheck(s,msg)      if (msg) TraceSocketUpdate ( s, msg )
#		ifdef DEBUGVERB
#			define  CSocketTraceVerbUpdate(s,msg)   
#		else
#			define  CSocketTraceVerbUpdate(s,msg)   TraceSocketUpdate ( s, msg )
#		endif
#		define  CSocketTraceRemove(s,msg,src)       TraceSocketRemove ( s, msg, src )
#	else
#		define	CSocketLog(msg)						CLog ( msg )
#		define  CSocketTraceAdd(s,msg)
#		define  CSocketTraceUpdate(s,msg)
#		define  CSocketTraceUpdateCheck(s,msg)
#		define  CSocketTraceVerbUpdate(s,msg)   
#		define  CSocketTraceRemove(s,msg,src)
#	endif
#endif

#if ( !defined(DEBUGSocketCreateLog) || defined(NDEBUG) )
#	undef	CSocketCreateLogArg
#	define  CSocketCreateLogArg(msg,...)
#else
#	define  CSocketCreateLogArg(msg,...)			CLogArg ( msg, __VA_ARGS__ )
#endif

#if ( defined(NDEBUG) && defined(RELEASELIB) )
#	undef	CLog
#	define  CLog(msg)
#	undef	CLogArg
#	define  CLogArg(msg,...)
#	undef	CLogArg1
#	define  CLogArg1(msg,...)
#	undef	CLogID
#	define  CLogID(msg)
#	undef	CLogArgID
#	define  CLogArgID(msg,...)
#	undef	CLogN
#	define  CLogN(msg)
#	undef	CLogArgN
#	define  CLogArNg(msg,...)
#	undef	CLogIDN
#	define  CLogIDN(msg)
#	undef	CLogArgIDN
#	define  CLogArgIDN(msg,...)
#	undef	CInfo
#	define  CInfo(msg)
#	undef	CInfoID
#	define  CInfoID(msg)
#	undef	CInfoArg
#	define  CInfoArg(msg,...)
#	undef	CInfoArgID
#	define  CInfoArgID(msg,...)
#	undef	CInfoN
#	define  CInfoN(msg)
#	undef	CInfoIDN
#	define  CInfoIDN(msg)
#	undef	CInfoArgN
#	define  CInfoArgN(msg,...)
#	undef	CInfoArgIDN
#	define  CInfoArgIDN(msg,...)
#	undef	CWarn
#	define  CWarn(msg)
#	undef	CWarnID
#	define  CWarnID(msg)
#	undef	CWarnArg
#	define  CWarnArg(msg,...)
#	undef	CWarnsArg
#	define	CWarnsArg(level,msg,...)
#	undef	CWarnArgID
#	define  CWarnArgID(msg,...)
#	undef	CWarnN
#	define  CWarnN(msg)
#	undef	CWarnIDN
#	define  CWarnIDN(msg)
#	undef	CWarnArgN
#	define  CWarnArgN(msg,...)
#	undef	CWarnArgIDN
#	define  CWarnArgIDN(msg,...)
#	undef	CErr
#	define  CErr(msg)
#	undef	CErrID
#	define  CErrID(msg)
#	undef	CErrArg
#	define  CErrArg(msg,...)
#	undef	CErrArgID
#	define  CErrArgID(msg,...)
#	undef	ElseCErrArgID
#	define  ElseCErrArgID(msg,...)
#	undef	CListLog
#	define  CListLog(msg,...)
#	undef	CErrN
#	define  CErrN(msg)
#	undef	CErrIDN
#	define  CErrIDN(msg)
#	undef	CErrArgN
#	define  CErrArgN(msg,...)
#	undef	CErrArgIDN
#	define  CErrArgIDN(msg,...)
#	undef	ElseCErrArgIDN
#	define  ElseCErrArgIDN(msg,...)
#	undef	CListLogN
#	define  CListLogN(msg,...)

#	if (!defined(ENVIRONS_CORE_LIB))
#       undef   CLogsArg
#       define  CLogsArg(level,msg,...)
#	endif
#else
#	if (defined(ENVIRONS_CORE_LIB))
#		undef	CWarnsArg
#		define	CWarnsArg(level,msg,...)					DLEVEL ( level ) { CWarnArg(msg, __VA_ARGS__); }
#   else
#       undef   CLogsArg
#       define  CLogsArg(level,msg,...)
#	endif
#endif

#endif  /// end-INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H



