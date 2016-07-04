/**
 * Environs Log helper
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
#ifndef INCLUDE_HCM_ENVIRONS_LOG_HELPER_H
#define INCLUDE_HCM_ENVIRONS_LOG_HELPER_H

#include "Interop/Export.h"

#ifndef ENVIRONS_NATIVE_MODULE
#   define USE_ENVIRONS_LOG_POINTERS
#endif

#ifdef USE_STATIC_ENVIRONS_LOG
#   ifdef USE_ENVIRONS_LOG_POINTERS
#       undef USE_ENVIRONS_LOG_POINTERS
#   endif
#endif

/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif
    
    void DisposeLog ();
    void CloseLog ( bool lock );
	void CheckLog ();
    
#ifdef ANDROID
    typedef void (CallConv * pCOutLog) ( int tag, const char * msg, int length, bool useLock );
    
    typedef void (CallConv * pCOutArgLog) ( int tag, const char * format, ... );
#else
    typedef void (CallConv * pCOutLog) ( const char * msg, int length, bool useLock );
    
    typedef void (CallConv * pCOutArgLog) ( const char * format, ... );
#endif
    
    
#if defined(ENVIRONS_CORE_LIB) 
	//|| !defined(ENVIRONS_MODULE)

#   ifdef ANDROID
        void COutLog ( int tag, const char * msg, int length, bool useLock );
        void COutArgLog ( int tag, const char * format, ... );
#   elif CLI_CPP
        void COutLog ( CString_ptr className, CString_ptr prefix, CString_ptr msg );
        void COutLog ( CString_ptr msg );
#   else
        void COutLog ( const char * msg, int length, bool useLock );
        void COutArgLog ( const char * format, ... );
#   endif

#else
    
#   ifdef USE_ENVIRONS_LOG_POINTERS
        extern pCOutLog                 COutLog;
        extern pCOutArgLog              COutArgLog;
    
#       define INIT_ENVIRONS_LOG()		environs::Loader::GetLogMethods ( (void **)&COutLog, (void **)&COutArgLog )
#   else
        void COutLog ( const char * msg, int length, bool useLock );
        void COutArgLog ( const char * format, ... );
#   endif
#endif
    
#ifdef __cplusplus
} /* namespace */
#endif


#endif	/// INCLUDE_HCM_ENVIRONS_LOG_HELPER_H