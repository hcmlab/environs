/**
 *	Threads log macros
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_THREADS_LOG_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_THREADS_LOG_H


#if (defined(ENVIRONS_CORE_LIB) || defined(ENVIRONS_NATIVE_MODULE))

#   ifndef CLI_CPP
#       include "Environs.Obj.h"
#   endif
#else
#	ifdef CErr
#		undef CErr
#	endif
#	define CErr(msg) printf(msg)

#	ifdef CVerb
#		undef CVerb
#	endif
#   ifdef DEBUGVERB
#       define CVerb(msg) printf(msg)
#   else
#       define CVerb(msg)
#   endif

#	ifdef CVerbArg
#		undef CVerbArg
#	endif
#   ifdef DEBUGVERB
#       define CVerbArg(msg,...) printf(msg,__VA_ARGS__)
#   else
#       define CVerbArg(msg,...)
#	endif

#	ifdef CLogArg
#		undef CLogArg
#	endif
#	define CLogArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef CErrArg
#		undef CErrArg
#	endif
#	define CErrArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef CWarnArg
#		undef CWarnArg
#	endif
#	define CWarnArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef DEBUGVERBLocks
#		ifdef CVerbsLockArg
#			undef CVerbsLockArg
#		endif
#		define CVerbsLockArg(l,msg,...) printf(msg,__VA_ARGS__)
#	else
#		ifdef CVerbsLockArg
#			undef CVerbsLockArg
#		endif
#		define CVerbsLockArg(msg,...)
#	endif
#endif

#if ( !defined(DEBUGVERBLocks) )
#   if  ( defined(ENVIRONS_CORE_LIB) && !defined(MEDIATORDAEMON) )
#       ifdef CVerbsLockArg
#           undef CVerbsLockArg
#       endif
#       define CVerbsLockArg(l,msg,...)  if ( g_Debug > l ) { CLogArg(msg,__VA_ARGS__); }
#   else
#       ifdef CVerbsLockArg
#           undef CVerbsLockArg
#       endif
#       define CVerbsLockArg(msg,...)
#   endif
#endif


#ifndef CVerbsLockArg
#   ifdef NDEBUG
#       define CVerbsLockArg(l,msg,...)
#   else
#       if defined(ENVIRONS_CORE_LIB)
#           define CVerbsLockArg(l,msg,...)     if ( g_Debug > l ) { CLogArg(msg,__VA_ARGS__); }
#       else
#           define CVerbsLockArg(l,msg,...)     CVerbLockArg(msg,__VA_ARGS__)
#       endif
#   endif
#endif

#ifdef MEDIATORDAEMON
#	ifndef DEBUGVERB
#		ifdef CVerb
#			undef CVerb
#		endif
#		define CVerb(msg)

#		ifdef CVerbArg
#			undef CVerbArg
#		endif
#		define CVerbArg(msg,...)
#	endif
#endif

#ifdef CLI_CPP
#	undef CWarnsArg
#	define CWarnsArg(l,m,f)
#	undef CVerbsArg
#	define CVerbsArg(l,m,f)
#	undef CErrArg
#	define CErrArg(m,f)
#	undef CErrN
#	define CErrN(m)
#	undef CVerbVerbArg
#	define CVerbVerbArg(m,f)
#endif

#endif // INCLUDE_HCM_ENVIRONS_INTEROP_THREADS_LOG_H
