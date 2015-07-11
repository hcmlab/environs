/**
 *	Interop thread helper implementation
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
#include "./Threads.h"
#include <errno.h>
#include <stdio.h>

#ifndef _WIN32
#include <sys/time.h>
#endif
#include "Environs.h"
#include "Environs.Native.h"

#ifdef USE_PTHREADS_FOR_WINDOWS
#pragma comment ( lib, "pthreadVC2.lib" )
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Threads"

namespace environs
{
#ifdef ANDROID
	INCLINEFUNC long ___sync_val_compare_and_swap ( long * destination, unsigned int compare, unsigned int swap )
	{
		return __sync_val_compare_and_swap ( destination, compare, swap );
	}
#endif

#ifdef WINDOWS_PHONE

	static HANDLE threadSleepEvent = 0;

	INCLINEFUNC void Sleep ( DWORD millis )
	{
		WaitForSingleObjectEx ( threadSleepEvent, millis, false );
	}
#endif

	bool InitInteropThread ()
	{
#ifdef WINDOWS_PHONE
		if ( !threadSleepEvent ) {
			threadSleepEvent = CreateEventEx ( 0, 0, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS );
		}
		return (threadSleepEvent != 0);
#else
		return true;
#endif
	}

	void DisposeInteropThread ()
	{
#ifdef WINDOWS_PHONE
		if ( threadSleepEvent ) {
			CloseHandle ( threadSleepEvent );
			threadSleepEvent = 0;
		}
#endif
	}


	INCLINEFUNC bool pthread_valid ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
#ifdef WINDOWS_PHONE
		if ( thread )
			return true;
#else
		DWORD exitCode = 0;

		if ( thread && GetExitCodeThread ( thread, &exitCode ) ) {
			if ( exitCode == STILL_ACTIVE )
				return true;
		}
#endif
#else
#ifdef _WIN32
		if ( thread.p )
			return true;
#else
		if ( thread )
			return true;
#endif
#endif
		return false;
	}


#ifdef USE_CRIT_SEC_MUTEX
	INCLINEFUNC bool pthread_mutex_init ( CRITICAL_SECTION  * critSEc, void * arg )
	{
		InitializeCriticalSection ( critSEc );
		return false;
	}

	INCLINEFUNC bool pthread_mutex_destroy ( CRITICAL_SECTION  * critSEc )
	{
		DeleteCriticalSection ( critSEc );
		return false;
	}
#endif


	INCLINEFUNC bool pthread_is_self_thread ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( !thread )
			return false;
#ifdef WINDOWS_PHONE
		DWORD id = (DWORD)thread;
#else
		DWORD id = GetThreadId ( thread );
#endif
		if ( !id ) // Failed
			return true;

		DWORD our_id = GetCurrentThreadId ();

		return (our_id == id);
#else
		pthread_t our_id = pthread_self ();

		return (memcmp ( &our_id, &thread, sizeof(pthread_t) ) == 0);
#endif
	}


	INCLINEFUNC bool pthread_wait_one ( pthread_cond_t &cond, pthread_mutex_t &lock )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) 
#ifdef WINDOWS_PHONE
		return (WaitForSingleObjectEx ( cond, INFINITE, TRUE ) == WAIT_OBJECT_0 );
#else
		return (WaitForSingleObject ( cond, INFINITE ) == WAIT_OBJECT_0 );
#endif
#else
		bool ret = false;

		if ( pthread_mutex_lock ( &lock ) )
			return false;

		if ( !pthread_cond_wait ( &cond, &lock ) )
			ret = true;

		pthread_mutex_unlock ( &lock );
		return ret;
#endif
	}
    
    
	INCLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		return pthread_cond_timedwait ( cond, lock, &timeout );
#else
		return pthread_cond_timedwait_sec ( cond, lock, timeout / 1000 );
#endif
	}
    
    
	INCLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		unsigned int waitTime = timeout * 1000;
#else
		struct timeval	now;
		struct timespec waitTime;
        
		gettimeofday ( &now, NULL );
        
		waitTime.tv_sec = now.tv_sec + timeout;
		waitTime.tv_nsec = now.tv_usec * 1000;
#endif
		return pthread_cond_timedwait ( cond, lock, &waitTime );
	}


#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

#ifdef USE_CRIT_SEC_MUTEX
	INCLINEFUNC int pthread_mutex_lock ( pthread_mutex_t * lock )
	{
		EnterCriticalSection(lock);
		return 0;
	}


	INCLINEFUNC int pthread_mutex_trylock ( pthread_mutex_t * lock )
	{
		return !TryEnterCriticalSection(lock);
	}


	INCLINEFUNC int pthread_mutex_unlock ( pthread_mutex_t * lock )
	{
		LeaveCriticalSection(lock);
		return 0;
	}
#endif


	INCLINEFUNC int pthread_cond_timedwait ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int * timeout )
	{
		int ret = 0;
		
#ifdef USE_CRIT_SEC_MUTEX
		LeaveCriticalSection ( lock ); 

#ifdef WINDOWS_PHONE
		if ( WaitForSingleObjectEx ( *cond, *timeout, TRUE ) != WAIT_OBJECT_0 )
#else
		if ( WaitForSingleObject ( *cond, *timeout ) != WAIT_OBJECT_0 )
#endif
			ret = ETIMEDOUT;

		EnterCriticalSection ( lock );
#else
		ReleaseMutex ( *lock ); 

		if ( WaitForSingleObject ( *cond, *timeout ) != WAIT_OBJECT_0 )
			ret = ETIMEDOUT;

		WaitForSingleObject ( *lock, WAIT_TIME_FOR_MUTEX_ACQUIRE );
#endif
		return ret;
	}


#endif


#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif

	bool env_sem_create ( sem_tp * sem, int iniVal, const char * name, unsigned int name1, int name2, int name3 )
	{
#ifdef _WIN32
#ifdef WINDOWS_PHONE
		static char buffer [48];

		int lenA = sprintf ( buffer, "sem-%s%u-%i-%u", name, name1, name2, name3 );
		if ( lenA <= 0 )
			return false;

		static TCHAR tbuffer [128];
		int lenW = ::MultiByteToWideChar ( CP_ACP, 0, buffer, lenA, tbuffer, lenA );
		if ( lenW <= 0 )
			return false;

		sem_tp tsem = CreateSemaphoreEx ( NULL, 1, 3, tbuffer, 0, SEMAPHORE_ALL_ACCESS );
#else
		sem_tp tsem = CreateSemaphore ( NULL, 0, 3, NULL );
#endif
		if ( !tsem )
			return false;
		*sem = tsem;
#endif
		///
#ifdef __APPLE__
		static char buffer [48];

		sprintf ( buffer, "sem-%s%u-%i-%u", name, name1, name2, name3 );
		sem_unlink ( buffer );

		sem_t * tsem = sem_open ( buffer, O_CREAT | O_EXCL, 0644, iniVal );
		if ( tsem == SEM_FAILED ) {
			return false;
		}

		*sem = tsem;
#endif

#ifdef ANDROID
		int ret = sem_init ( sem, 0, iniVal );
		if ( ret == -1 ){
			return false;
		}
#endif
		return true;
	}


	void DisposeThread ( pthread_t &threadID, const char * threadName )
	{
		CVerb ( "DisposeThread" );

		pthread_t thrd = threadID;

		pthread_reset ( threadID );

		if ( !pthread_valid ( thrd ) ) {
			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		CLogArg ( "Waiting for %s ...", threadName );

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) && !defined(WINDOWS_PHONE)

		//DWORD dw = WaitForSingleObject ( thrd, WAIT_TIME_FOR_THREAD_CLOSING );
		DWORD dw = WaitForSingleObject ( thrd, INFINITE );
		if ( dw == WAIT_OBJECT_0 || dw != WAIT_TIMEOUT ) {
			CloseHandle ( thrd );
		}
		else {
			CErrArg ( "DisposeThread: Waiting for %s thread TIMED OUT!", threadName );
			if ( TerminateThread ( thrd, 0 ) ) {
				CWarnArg ( "DisposeThread: Terminated %s thread.", threadName );
			}
			else {
				CErrArg ( "DisposeThread: Failed to terminate %s thread!", threadName );
			}
		}
#else
		pthread_join ( thrd, NULL );
		//pthread_kill ( thrd, SIGUSR1 );
		//pthread_detach_handle ( thrd );
#endif
	}


	void DisposeThread ( pthread_t &threadID, const char * threadName, pthread_cond_t &threadEvent )
	{
		if ( !pthread_valid ( threadID ) ) {
			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		if ( pthread_cond_valid ( threadEvent ) ) {
			CVerbArg ( "DisposeThread: Signaling %s thread...", threadName );

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
			SetEvent ( threadEvent );
#else
			pthread_cond_signal ( &threadEvent );
#endif
		}
		DisposeThread ( threadID, threadName );
	}

#ifdef _WIN32
#pragma warning( pop )
#endif

}
