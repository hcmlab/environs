/**
 * Utils and common stuff
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <stdio.h>
#include <stdarg.h>
#include "Environs.Native.h"
#include "Interop/Threads.h"
#include "Interop/Stat.h"
#include "Environs.Obj.h"
#include "Environs.Utils.h"
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
#	include <io.h>
#	define access(a,b)	_access(a,b)
#	define F_OK			0
#else
#   include <signal.h>
#   include <sys/time.h>
#endif

#define CLASS_NAME  "Utils. . . . . . . . . ."


namespace environs
{
    
	char * BuildDataStorePath ( const char * fileName )
	{
		if ( !fileName )
			return 0;

		static char absolutePath [1024];

		*absolutePath = 0;
		
        if ( native.dataStore ) {
            if ( native.dataStore == (char *)opt_dataStoreDefault ) {
                CWarnArg ( "BuildDataStorePath: DataStore path has not been set. Using default [ %s ].", opt_dataStoreDefault );
            }
            
			strlcat ( absolutePath, native.dataStore, 1024 );
        }
		strlcat ( absolutePath, fileName, 1024 );

		CVerbArg ( "BuildDataStorePath: [ %s ]", absolutePath );
		return absolutePath;
	}


    bool DirectoryPathExist ( char * path )
    {
        if ( !path )
            return false;        
#ifdef _WIN32
		DWORD atts = GetFileAttributesA(path);

		return (atts != INVALID_FILE_ATTRIBUTES && (atts & FILE_ATTRIBUTE_DIRECTORY));
#else
        if ( access ( path, F_OK ) == 0 )
            return true;

        return false;
		/*STAT_STRUCT(st);

		if (stat(path, &st) != 0)
			return false;

		return true;*/
#endif
    }
    

	bool CreateDataDirectory ( char * dir )
	{
		if ( !dir )
			return false;

		if (DirectoryPathExist(dir))
			return true;

		CVerbArg ( "CreateDataDirectory: [ %s ]", dir );

		if ( mkdir ( dir, 0777 ) == 0 )
			return true;

		if ( errno == EEXIST )
			return true;

		return true;
	}


	bool InitStorageUtil ( const char * storagePath )
	{
		if ( !storagePath ) {
			CErr ( "InitStorageUtil: Invalid parameter for storagePath!" );
			return false;
		}

		size_t len = strlen ( storagePath );
		if ( len < 2 ) {
			CErr ( "InitStorageUtil: Length of storagePath is < 2!" );
			return false;
		}

		size_t defLen = strlen ( opt_dataStoreDefault );
#ifndef NDEBUG // Code analysis seem to allow size_t to be negative
		if ( defLen <= 0 )
			defLen = 0;
#endif
		char * storagePathNew = ( char * ) malloc ( len + defLen + 4 );
		if ( !storagePathNew ) {
			CErr ( "InitStorageUtil: Failed to allocate memory for new storagePath!" );
			return false;
		}

		memcpy ( storagePathNew, storagePath, len );

		size_t last = len - 1;

		// Ensure that we have a slash as last character
		if ( storagePathNew [ last ] != '/' ) {
			storagePathNew [ last + 1 ] = '/';
			last += 2;
		}
		else
			last++;

		// skipping the ./ -> +-2
		memcpy ( storagePathNew + last, opt_dataStoreDefault + 2, defLen - 2 );

		last += defLen - 2;
		// Append a terminating zero
		storagePathNew [ last ] = 0;

		native.SetDataStore ( storagePathNew );

		CVerbArg ( "InitStorageUtil: Storage [ %s ]", native.dataStore );

		CreateDataDirectory ( native.dataStore );
		return true;
	}


#ifndef _WIN32
#define	GetCurrentThreadId()	pthread_self ( )
#endif

#ifdef _WIN32
	DWORD
#else
	pthread_t
#endif
		environsUtilsMainThreadID;

	
	void RegisterMainThreadUtil ( )
	{
		CVerb ( "RegisterMainThreadUtil" );

		environsUtilsMainThreadID = GetCurrentThreadId ( );
	}
    
    
#if ( !defined(__APPLE__) || defined(LINUX) )
	bool IsUIThread ()
	{
		return (environsUtilsMainThreadID == GetCurrentThreadId ( ));
	}
#endif


	typedef struct _AsyncTimedJoinContext
	{
		pthread_t		waitThreadID;
		pthread_t		threadID;
		pthread_cond_t	signal;
		pthread_mutex_t	lock;
	}
	AsyncTimedJoinContext;


	void * TimedJoinAsync ( void * arg )
	{
		AsyncTimedJoinContext * context = (AsyncTimedJoinContext *)arg;
		if ( !context )
			return 0;

		CVerb ( "TimedJoinAsync: Started..." );
		pthread_join ( context->threadID, 0 );

		// Send the signal in case we have joined successfully
		if ( pthread_mutex_lock ( &context->lock ) ) {
			CErr ( "TimedJoinAsync: Failed to lock mutex." );
			return 0;
		}

		CVerb ( "TimedJoinAsync: signaling..." );

		if ( pthread_cond_signal ( &context->signal ) ) {
			CErr ( "TimedJoinAsync: Failed to signal event." );
		}

		if ( pthread_mutex_unlock ( &context->lock ) ) {
			CErr ( "TimedJoinAsync: Failed to unlock mutex." );
		}

		return 0;
	}


	int pthread_timedjoin_env ( pthread_t threadID, unsigned int milis )
	{
		if ( !pthread_valid ( threadID ) ) {
			CVerb ( "pthread_timedjoin_env: Thread is already closed." );
			return 1;
		}

		int rc = -1, threadrc;

		AsyncTimedJoinContext context;
		Zero ( context );

		context.threadID	= threadID;

		Zero ( context.lock );
		if ( pthread_mutex_init ( &context.lock, 0 ) ) {
			CErr ( "pthread_timedjoin_env: Failed to init lock!" );
			return 0;
		}

		Zero ( context.signal );
		if ( pthread_cond_manual_init ( &context.signal, 0 ) ) {
			CErr ( "pthread_timedjoin_env: Failed to init signal!" );
			return 0;
		}

		if ( pthread_mutex_lock ( &context.lock ) ) {
			CErr ( "pthread_timedjoin_env: Failed to aquire mutex!" );
			return 0;
		}

		// Win32: reset the receive signal in preparation
		pthread_cond_preparev ( &context.signal );

		// Create the waiter thread
		threadrc = pthread_create ( &context.waitThreadID, 0, &TimedJoinAsync, (void *)&context );
		if ( threadrc != 0 ) {
			CErrArg ( "pthread_timedjoin_env: Error creating thread (pthread_create [%i])", threadrc );
			goto Finish;
		}

		rc = pthread_cond_timedwait_msec ( &context.signal, &context.lock, milis );
		if ( rc ) {
			if ( rc == ETIMEDOUT ) {
				CWarn ( "pthread_timedjoin_env: Wait failed due to TIMEOUT" );
			}
			else if ( rc == EPERM ) {
				CErr ( "pthread_timedjoin_env: Wait failed due to Mutex not locked by caller" );
			}
			else if ( rc == EINVAL ) {
				CErr ( "pthread_timedjoin_env: Wait failed due to INVALID input parameters" );
			}

			// Terminate the thread
			CVerb ( "pthread_timedjoin_env: Terminating waiter and given thread..." );

#if !defined(USE_ENVIRONS_POSIX_THREADS)
			if ( TerminateThread ( context.waitThreadID, 0 ) ) {
				CVerb ( "pthread_timedjoin_env: Terminated waiter thread." );
			}
			else {
				CErr ( "pthread_timedjoin_env: Failed to terminate waiter thread!" );
			}

			if ( TerminateThread ( threadID, 0 ) ) {
				CVerb ( "pthread_timedjoin_env: Terminated given thread." );
			}
			else {
				CErr ( "pthread_timedjoin_env: Failed to terminate given thread!" );
			}
#else

#ifdef ANDROID
			pthread_kill ( context.waitThreadID, SIGUSR1 );
			pthread_kill ( threadID, SIGUSR1 );
#else
			pthread_cancel ( context.waitThreadID );
			pthread_cancel ( threadID );
#endif
  
#endif
			rc = 0;
		}
		else {
			CVerb ( "pthread_timedjoin_env: Thread terminated gracefully." );
			rc = 1;
			pthread_detach_handle ( context.waitThreadID );
		}

	Finish:
		pthread_detach_handle ( threadID );
		return rc;
	}


	void CreateCopyString ( const char * src, char ** dest )
	{
		if ( !dest || !src || !*src || *dest )
			return;

		unsigned int length = (unsigned int)strlen ( src );

		char * name = (char *) malloc ( length + 1 );
		if ( !name ) {
			CErr ( "CreateCopyString: Failed to allocate memory." );
			return;
		}

		memcpy ( name, src, length );
		name [length] = 0;

		if ( *dest )
			free ( *dest );
		*dest = name;
	}
}
