/**
* Log
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
#include "stdafx.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef ANDROID
#include <android/log.h>
#endif

#include "Environs.Native.h"
#include "Interop/Threads.h"
#include "Interop/Stat.h"
#include "Environs.h"
#include "Core/Callbacks.h"
using namespace environs;

#define CLASS_NAME   "Log"

#define ENVIRONS_LOGFILE_TEMP_MAXSIZE               200000

char            *   environsTemporaryLogBuffer      = 0;
size_t              environsTemporaryLogBufferSize  = 0;
FILE            *   environsLogFileHandle           = 0;
int                 environsLogFileErrCount         = 0;


namespace environs
{
	pthread_mutex_t     environsLogMutex;


    bool OpenLog ()
    {
        CVerbN ( "OpenLog" );
        
        if ( !environs.workDir || !*environs.workDir )
            return false;
        
        char fileName [ 1024 ];
        
        sprintf ( fileName, "%s/environs.log", environs.workDir );
        
#ifdef _WIN32
        struct _stat st;
#else
        struct stat st;
#endif
        if ( stat ( fileName, &st ) == 0 )
        {
            size_t fileSize = (size_t) st.st_size;
            if ( fileSize > 10000000 ) {
                if ( unlink ( fileName ) ) {
                    if ( environsLogFileErrCount < 10 ) {
                        CLogN ( "OpenLog: ERROR ---> Failed to remove old logfile." );
                        environsLogFileErrCount++;
                    }
                    return false;
                }
            }
        }
        
        
        FILE * fp = fopen ( fileName, "a" );
        if ( !fp ) {
            if ( environsLogFileErrCount < 10 ) {
                CLogN ( "OpenLog: ERROR ---> Failed to open/create logfile." );
                //CVerbArgN ( "OpenLog: Error %d", errno );
                environsLogFileErrCount++;
            }
            else
                environs.opt_useLogFile = false;
            
            return false;
        }
        
        environsLogFileHandle = fp;
        
        if ( environsTemporaryLogBuffer ) {
            fwrite ( environsTemporaryLogBuffer, 1, environsTemporaryLogBufferSize, fp );
            /*
            if ( opt_useNotifyDebugMessage ) {
                if ( Callbacks.doOnStatusMessage )
                    Callbacks.OnStatusMessage ( environsTemporaryLogBuffer );
            }
            */
            
            free ( environsTemporaryLogBuffer );
            environsTemporaryLogBuffer = 0;
            environsTemporaryLogBufferSize = 0;
        }
        return true;
    }
    
    
    void CloseLog ( bool useLock )
    {
        CVerbN ( "CloseLog" );
        
		if ( useLock && pthread_mutex_lock ( &environsLogMutex ) ) {
            CLogN ( "CloseLog: ERROR ---> Failed to lock mutex." );
        }
        
        if ( environsLogFileHandle ) {
            fclose ( environsLogFileHandle );
            environsLogFileHandle = 0;
        }
        
		if ( useLock && pthread_mutex_unlock ( &environsLogMutex ) ) {
            CLogN ( "CloseLog: ERROR ---> Failed to unlock mutex." );
        }
        CVerbN ( "CloseLog done" );
    }
    
    
    void DisposeLog ()
    {
        CVerbN ( "DisposeLog" );
        
		if ( pthread_mutex_lock ( &environsLogMutex ) ) {
            CLogN ( "DisposeLog: ERROR ---> Failed to lock mutex." );
        }
        
        CloseLog ( false );
        
        if ( environsTemporaryLogBuffer ) {
            free ( environsTemporaryLogBuffer );
            environsTemporaryLogBuffer = 0;
            environsTemporaryLogBufferSize = 0;
        }
        
		if ( pthread_mutex_unlock ( &environsLogMutex ) ) {
            CLogN ( "DisposeLog: ERROR ---> Failed to unlock mutex." );
        }
        CVerbN ( "DisposeLog done" );
    }
    
    
#ifdef ANDROID
    void COutLog ( int tag, const char * msg, int length, bool useLock )
#else
    void COutLog ( const char * msg, int length, bool useLock )
#endif
    {
		if ( useLock && pthread_mutex_lock ( &environsLogMutex ) ) {
            return;
        }
        
        if ( !length )
            length = (int)strlen ( msg );
        
        if ( environs.opt_useLogFile ) {
            if ( !environsLogFileHandle && !OpenLog () ) {
                if ( !environsTemporaryLogBuffer ) {
                    environsTemporaryLogBuffer = (char *) malloc ( ENVIRONS_LOGFILE_TEMP_MAXSIZE );
                    
                    if ( environsTemporaryLogBuffer ) {
                        *environsTemporaryLogBuffer = 0;
                        environsTemporaryLogBufferSize = 0;
                    }
                }
                
                if ( environsTemporaryLogBuffer ) {
                    strcat_s ( environsTemporaryLogBuffer, ENVIRONS_LOGFILE_TEMP_MAXSIZE, msg );
                    environsTemporaryLogBufferSize += length;
                }
            }
            else {
                if ( length <= 0 )
                    length = (int)strlen ( msg );
                
                fwrite ( msg, 1, length, environsLogFileHandle );
            }
        }
        
#ifdef _WIN32
        OutputDebugStringA ( msg );
#endif // -> end-_WIN32
        
        
#ifdef ANDROID
        __android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", msg );
#endif  // -> end-_ANDROID
        
        
#if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux
        printf ( "%s", msg );
#endif  // -> end-_ANDROID
        
        if ( environs.opt_useNotifyDebugMessage ) {
			if ( environs.callbacks.doOnStatusMessage )
				environs.callbacks.OnStatusMessage ( msg );
        }
        
        if ( useLock )
			pthread_mutex_unlock ( &environsLogMutex );
    }
    
    
#ifdef ANDROID
    void COutArgLog ( int tag, const char * format, ... )
#else
    void COutArgLog ( const char * format, ... )
#endif
    {
        char buffer [1024];
        va_list marker;
        
		if ( pthread_mutex_lock ( &environsLogMutex ) ) {
            return;
        }
        
        va_start ( marker, format );
        int len = (int) vsnprintf ( buffer, 1024, format, marker );
        
#ifdef ANDROID
        COutLog ( tag, buffer, len, false );
#else
        COutLog ( buffer, len, false );
#endif
        va_end ( marker );
        
		pthread_mutex_unlock ( &environsLogMutex );
    }
    
    
    
    /*
     #if defined(_WIN32) && !defined(MEDIATORDAEMON)
     #pragma warning( push )
     #pragma warning( disable: 4996 )
     
     void COutArg ( const char * format, ... )
     {
     char buffer [1024];
     va_list marker;
     
     va_start ( marker, format );
     vsnprintf ( buffer, 1024, format, marker );
     OutputDebugStringA ( buffer );
     va_end ( marker );
     }
     
     #pragma warning( pop )
     #endif
     */
}





