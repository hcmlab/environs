/**
* iOSX Log declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_IOSX_LOG_H_
#define INCLUDE_HCM_ENVIRONS_IOSX_LOG_H_

#ifdef DEBUGVERB
#   ifdef CVerb
#       undef CVerb
#   endif
#   ifdef CVerbArg
#       undef CVerbArg
#   endif

#   define CVerb(a)     NSLog(@a)
#   define CVerbArg(a,...)    NSLog(@a,__VA_ARGS__)

#else

#   ifdef CVerb
#       undef CVerb
#   endif
#   define CVerb(a) 

#   ifdef CVerbArg
#       undef CVerbArg
#   endif
#   define CVerbArg(a,...)

#endif

#ifdef DEBUGVERBVerb
#   ifdef CVerbVerb
#       undef CVerbVerb
#   endif

#   define CVerbVerb(a)    NSLog(@a)

#else

#   ifdef CVerbVerb
#       undef CVerbVerb
#   endif
#   define CVerbVerb(a)

#endif

/*
#ifdef CLog
#   undef CLog
#endif

#define CLog(a)     CLog(@a)
 */

#endif /* INCLUDE_HCM_ENVIRONS_IOSX_LOG_H_ */
