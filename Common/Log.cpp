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

#ifndef WINDOWS_PHONE
#	include <stdio.h>
#endif

#ifndef CLI_CPP
#	include <stdarg.h>

#	ifdef _WIN32
#		ifndef WINDOWS_PHONE
#			include <stdlib.h>
#			include "windows.h"
#		endif
#		include <time.h>
#	endif

#	ifdef ANDROID
#       include <android/log.h>
#	endif

#	ifdef __APPLE__
//#       define USE_NSLOG
#	endif

#	include "Environs.Obj.h"
#	include "Core/Callbacks.h"
#endif

#ifdef USE_NSLOG
#   include "Environs.iosx.imp.h"
#endif

#ifndef NDEBUG
//#   define ENABLE_BUFFERING
#endif

#include "Environs.Utils.h"
#include "Environs.Native.h"
#include "Interop/Threads.h"
#include "Interop/Stat.h"

using namespace environs;

#define CLASS_NAME  "Log. . . . . . . . . . ."


//#ifdef NDEBUG
//#   define LOG_TIMESTRING
//#endif

#define ENVIRONS_LOGFILE_TEMP_MAXSIZE               200000
#define ENVIRONS_LOGBUFFER_MAXCHARS					ENVIRONS_LOGBUFFER_MAXSIZE - 2
#define ENVIRONS_LOGBUFFER_OUTPUT_MINSIZE			ENVIRONS_LOGBUFFER_MAXSIZE >> 2
#define ENVIRONS_LOGBUFFER_OUTPUT_AX_MS				500


namespace environs
{
	pthread_mutex_t     environsLogMutex;
	FILE            *   environsLogFileHandle		= 0;
	int                 environsLogFileErrCount     = 0;
	
#ifndef ENABLE_BUFFERING
#	define		LOGBUFFER
#	define		LOGPOS		
#	define		LOGPOSADD	
#	define		ENVIRONS_LOGBUFFER_MAXSIZE		1024
#	define		LOGBUFFEREMAIN					ENVIRONS_LOGBUFFER_MAXSIZE
#	define		LOG_OUT_BUFFER_NAME				msg
#	define		LOG_OUT_BUFFER_NAME_LENGTH		length
#	define		LOG_OUT_BUFFER_NAME_ARG_CALL	log.buffer
#else
#	define		LOGBUFFER						log.
#	define		LOGPOS							log.length
#	define		LOGPOSADD						+ 	LOGPOS
#	define		ENVIRONS_LOGBUFFER_MAXSIZE		256000
#	define		LOGBUFFEREMAIN					(ENVIRONS_LOGBUFFER_MAXCHARS - LOGPOS)
#	define		LOG_OUT_BUFFER_NAME				log.buffer
#	define		LOG_OUT_BUFFER_NAME_LENGTH		log.length
#	define		LOG_OUT_BUFFER_NAME_ARG_CALL	0
#endif

	void env_printf ( const char * msg )
	{
#ifdef ENABLE_BUFFERING

#	ifdef _WIN32
		OutputDebugStringA ( msg );
#	endif // -> end-_WIN32


#	ifdef ANDROID
		__android_log_print ( 0, ENVIRONS_LOG_TAG_ID, "%s", msg );
#	endif  // -> end-_ANDROID


#	if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux

		printf ( "%s", msg );

#	endif  // -> end-_ANDROID
#endif

	}

	struct LogBuffer
	{
		bool			alive;
		bool			push;
		char			buffer [ ENVIRONS_LOGBUFFER_MAXSIZE ];

		int				length;

#ifdef ENABLE_BUFFERING
		INTEROPTIMEVAL	lastPrint;
#endif
		LogBuffer ()
		{
			env_printf ( CLASS_NAME ".LogBuffer: Construct\n" );

			if ( !LockInit ( &environsLogMutex ) )
				return;

			alive = true; 
			push = false;
			*buffer = 0;
			length = 0;

#ifdef ENABLE_BUFFERING
			lastPrint = 0;
#endif
		}

		~LogBuffer ()
		{
			env_printf ( CLASS_NAME ".LogBuffer: Destruct\n" );

			//native.useLogFile = false;

#ifdef ENABLE_BUFFERING
			if ( length > 0 ) {
				push = true;

#	ifdef ANDROID
				COutLog ( 0, buffer, length, false );
#	else
				COutLog ( buffer, length, false );
#	endif

			}
#endif
			//CloseLog ();

			alive = false;

			LockDispose ( &environsLogMutex );

			env_printf ( CLASS_NAME ".LogBuffer: Destruct done.\n" );
		}
	};

	LogBuffer	log;


#ifndef ANDROID
	size_t GetTimeString ( char * timeBuffer, unsigned int bufferSize )
	{
		time_t now;
		struct tm timeInfo;

		time ( &now );
#ifdef WIN32
		if ( localtime_s ( &timeInfo, &now ) ) {
#else
        //if ( !localtime_s ( &timeInfo, &now ) ) {
        if ( !localtime_r ( &now, &timeInfo ) ) {
#endif
			timeBuffer [ 0 ] = 'e';
			timeBuffer [ 1 ] = 0;
			return 1;
		}

		return strftime ( timeBuffer, bufferSize, "%a %b %d %H:%M:%S: ", &timeInfo );
	}
#endif

	
	void CheckLog ()
	{
		CVerbN ( "CheckLog" );

		if ( !native.workDir || !*native.workDir )
			return;

		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) )
				printf ( "CloseLog: ERROR ---> Failed to lock mutex." );
			else
				locked = true;
		}
			
		FILE * fp = 0;
		char fileName [ 1024 ];

		sprintf ( fileName, "%s/environs.log", native.workDir );

		size_t fileSize = GetSizeOfFile ( fileName );
		
		if ( fileSize > 500000000 ) {
			if ( environsLogFileHandle ) {
				fclose ( environsLogFileHandle );
				environsLogFileHandle = 0;
			}
			if ( unlink ( fileName ) ) {
				if ( environsLogFileErrCount < 10 ) {
					printf ( "CheckLog: ERROR ---> Failed to remove logfile." );
					environsLogFileErrCount++;
				}
			}
		}

		if ( !native.useLogFile || environsLogFileHandle )
			goto Return;

		fp = fopen ( fileName, "a" );
		if ( !fp ) {
			if ( environsLogFileErrCount < 10 ) {
				printf ( "OpenLog: ERROR ---> Failed to open/create logfile." );
				//CVerbArgN ( "OpenLog: Error %d", errno );
				environsLogFileErrCount++;
			}
			else
				native.useLogFile = false;

			goto Return;
		}

		environsLogFileHandle = fp;

	Return:
		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
			printf ( "CheckLog: ERROR ---> Failed to unlock mutex." );
		}
		CVerbN ( "CheckLog done" );
	}

        
    bool OpenLog ()
    {
        CVerbN ( "OpenLog" );
        
        if ( !native.workDir || !*native.workDir ) {
            CVerbVerbN ( "OpenLog: ERROR - Missing workDir for creating logfile!" );
            return false;
        }
        
        char fileName [ 1024 ];
        
        sprintf ( fileName, "%s/environs.log", native.workDir );

		STAT_STRUCT ( st );

        if ( stat ( fileName, &st ) == 0 )
        {
            size_t fileSize = (size_t) st.st_size;
            if ( fileSize > 10000000 ) {
                if ( unlink ( fileName ) ) {
                    if ( environsLogFileErrCount < 10 ) {
                        printf ( "OpenLog: ERROR ---> Failed to remove old logfile." );
                        environsLogFileErrCount++;
                    }
                    return false;
                }
            }
        }
        
#ifdef DEBUGVERB
        printf ( "OpenLog: Open logfile [ %s ]\n.", fileName );
#endif
        FILE * fp = fopen ( fileName, "a" );
        if ( !fp ) {
            if ( environsLogFileErrCount < 10 ) {
                printf ( "OpenLog: ERROR ---> Failed to open/create logfile." );
                //CVerbArgN ( "OpenLog: Error %d", errno );
                environsLogFileErrCount++;
            }
            else
                native.useLogFile = false;
            
            return false;
        }
        
        environsLogFileHandle = fp;
        
        return (fp != 0);
    }
    
    
    void CloseLog ( bool lock )
    {
        CVerbN ( "CloseLog" );
        
		bool locked = false;

		if ( lock && log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) )
				printf ( "CloseLog: ERROR ---> Failed to lock mutex." );
			else
				locked = true;
        }
        
        if ( environsLogFileHandle ) {
            fclose ( environsLogFileHandle );
            environsLogFileHandle = 0;
        }
        
		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "CloseLog: ERROR ---> Failed to unlock mutex." );
        }
        CVerbN ( "CloseLog done" );
    }


    void DisposeLog ()
    {
        CVerbN ( "DisposeLog" );

		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) )
				printf ( "DisposeLog: ERROR ---> Failed to lock mutex." );
			else
				locked = true;
		}
        
        CloseLog ( false );

		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
			printf ( "DisposeLog: ERROR ---> Failed to unlock mutex." );
		}

        CVerbN ( "DisposeLog done" );
    }
    
    
#ifdef ANDROID
    void COutLog ( int tag, const char * msg, int length, bool useLock )
#else
	void COutLog ( const char * msg, int length, bool useLock )
#endif
    {
#ifdef LINUX
        if ( !native.useStdout && !native.useLogFile )
            return;
#endif
		if ( length <= 0 ) {
			length = ( int ) strlen ( msg );
			//printf ( "COutLog: Length [%i].", length );

			if ( length <= 0 ) {
				printf ( "COutLog: ERROR ---> Invalid length [%i].", length );
				return;
			}
		}

		bool locked = false;
        
#if ( !defined(ANDROID) && defined(LOG_TIMESTRING) )
        char timeString [ 256 ];
        size_t timeLen = GetTimeString ( timeString, sizeof(timeString) );
#endif
        
		if ( useLock && log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) ) {
				printf ( "COutLog: ERROR ---> Failed to lock mutex." );
				return;
			}
			else
				locked = true;
		}

#ifdef ENABLE_BUFFERING
		if ( msg ) {
			int len = snprintf ( log.buffer LOGPOSADD, LOGBUFFEREMAIN - 1, "%s", msg );
			if ( len < 0 )
				len = 0;

			LOGPOS += len;
		}

		INTEROPTIMEVAL now = 0;

		// Output if min output size has been exceeded or 500ms has passed since last output
		if ( log.length < ENVIRONS_LOGBUFFER_OUTPUT_MINSIZE && ( ( now = GetEnvironsTickCount () ) - log.lastPrint ) < ENVIRONS_LOGBUFFER_OUTPUT_AX_MS ) 
		{
			if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
				printf ( "COutLog: ERROR ---> Failed to unlock mutex." );
			}
			return;
		}

		if ( !now )
			now = GetEnvironsTickCount ();
#endif
        if ( native.useLogFile ) {
            if ( environsLogFileHandle || OpenLog ())
            {
#if ( !defined(ANDROID) && defined(LOG_TIMESTRING) )
                fwrite ( timeString, 1, timeLen, environsLogFileHandle );
#endif
                fwrite ( LOG_OUT_BUFFER_NAME, 1, LOG_OUT_BUFFER_NAME_LENGTH, environsLogFileHandle );
            }
        }
        
#if defined ( _WIN32 )
        
#if ( defined(LOG_TIMESTRING) )
        OutputDebugStringA ( timeString );
#endif
        OutputDebugStringA ( LOG_OUT_BUFFER_NAME );
        
#elif defined ( ANDROID )
        __android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", LOG_OUT_BUFFER_NAME );

#elif defined ( USE_NSLOG )

        NSLog ( @"%s", LOG_OUT_BUFFER_NAME );
#else
        
//#if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux
        
        if ( native.useStdout )
#if ( defined(LOG_TIMESTRING) )
            printf ( "%s%s", timeString, LOG_OUT_BUFFER_NAME );
#else
            printf ( "%s", LOG_OUT_BUFFER_NAME );
#endif
        
#endif  // -> end-_ANDROID
        
        if ( native.useNotifyDebugMessage ) {
            for ( int i=1; i<ENVIRONS_MAX_ENVIRONS_INSTANCES; ++i )
            {
                Instance * env = instances[i];
                if ( !env )
                    break;
                
                if ( env->callbacks.doOnStatusMessage )
                    env->callbacks.OnStatusMessage ( i, LOG_OUT_BUFFER_NAME );
            }
        }

#ifdef ENABLE_BUFFERING
		log.lastPrint = now; log.length = 0;
#endif
		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
			printf ( "COutLog: ERROR ---> Failed to unlock mutex." );
		}
    }
    
    
#ifdef ANDROID
    void COutArgLog ( int tag, const char * format, ... )
#else
    void COutArgLog ( const char * format, ... )
#endif
    {
#ifdef LINUX
        if ( !native.useStdout && !native.useLogFile )
            return;
#endif
        va_list marker;

		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) ) {
				printf ( "COutArgLog: ERROR ---> Failed to lock mutex." );
				return;
			}
			locked = true;
        }
        
        va_start ( marker, format );
        int length = (int) vsnprintf ( log.buffer LOGPOSADD, LOGBUFFEREMAIN - 1, format, marker );

#ifdef ENABLE_BUFFERING
		LOGPOS += length;
#endif

#ifdef ANDROID
		COutLog ( tag, LOG_OUT_BUFFER_NAME_ARG_CALL, LOG_OUT_BUFFER_NAME_LENGTH, false );
#else
		COutLog ( LOG_OUT_BUFFER_NAME_ARG_CALL, LOG_OUT_BUFFER_NAME_LENGTH, false );
#endif
        va_end ( marker );
        
        if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "COutArgLog: ERROR ---> Failed to unlock mutex." );
        }
    }
    
}





