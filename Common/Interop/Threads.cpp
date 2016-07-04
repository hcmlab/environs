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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
//#   define DEBUGVERBLocks
#endif

#if ( !defined(NDEBUG) )
#	define	USE_LOCK_LOG
#endif

#include "./Threads.h"

#ifndef CLI_CPP
#	include <errno.h>
#	include <stdio.h>
#endif

#ifdef _WIN32
#   ifdef USE_ENVIRONS_POSIX_THREADS
#       pragma comment ( lib, "pthreadVC2.lib" )
#   endif
#else
#   include <sys/time.h>
#   include "./Sock.h"
#endif

#include "Environs.Native.h"
#include "Threads.Int.h"

#ifdef USE_THREADSYNC_OWNER_NAME
#	define CThreadVerbArg(msg,...)		CVerbVerbArg ( msg, __VA_ARGS__ )
#	define CThreadLogArg(msg,...)		CLogArg ( msg, __VA_ARGS__ )
#else
#	define CThreadVerbArg(msg,...)	
#	define CThreadLogArg(msg,...)			
#endif

#ifdef MEASURE_LOCK_ACQUIRE
extern unsigned int maxMeasureDiff;
#	include "Environs.Utils.h"
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Threads. . . . . . . . ."

namespace environs
{
#ifdef MEDIATORDAEMON
	extern size_t GetTimeString ( char * timeBuffer, unsigned int bufferSize );
#endif

#if ( !defined(ENVIRONS_CORE_LIB) || defined(ENVIRONS_NATIVE_MODULE) )
	int g_Debug = 0;
#endif

#ifdef ANDROID
	INLINEFUNC long ___sync_val_compare_and_swap ( long * destination, unsigned int compare, unsigned int swap )
	{
		return __sync_val_compare_and_swap ( destination, compare, swap );
	}
#endif

#ifdef WINDOWS_PHONE

	static HANDLE threadSleepEvent = 0;

	INLINEFUNC void Sleep ( DWORD millis )
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
		return ( threadSleepEvent != 0 );
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

#ifdef CLI_CPP

	int pthread_create_cli ( pthread_t % threadID, void *, System::Threading::ParameterizedThreadStart ^ startRoutine, pthread_param_t arg )
	{
		threadID = gcnew Thread ( startRoutine );
		if ( threadID == nullptr )
			return 1;

		threadID->Start ( arg );
		return 0;
	}


	bool LockAcquireBool ( pthread_mutex_t ^ mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName )
	{
		if ( mtx == nill ) {
			//CErr ( "Lock: lock invalid " + className + "." + funcName + " " + mutexName );
			//MutexErrorLog ( "lock", mutexName, className, funcName );
			return false;
		}
		CVerb ( "Lock: " + className + "." + funcName + " " + mutexName );
		Monitor::Enter ( mtx );
		return true;
	}

	bool LockReleaseBool ( pthread_mutex_t ^ mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName )
	{
		if ( mtx == nill ) {
			//CErr ( "Unlock: lock invalid " + className + "." + funcName + " " + mutexName );
			//MutexErrorLog ( "unlock", mutexName, className, funcName );
			return false;
		}

		CVerb ( "Unlock: " + className + "." + funcName + " " + mutexName );
		Monitor::Exit ( mtx );
		return true;
	}


#else

	INLINEFUNC bool pthread_valid ( pthread_t thread )
	{
#if defined(USE_ENVIRONS_POSIX_THREADS)
#	ifdef _WIN32
		if ( thread.p )
			return true;
#	else
		if ( thread )
			return true;
#	endif
#else
		if ( thread )
			return true;
#endif
		return false;
	}


#ifdef USE_CRIT_SEC_MUTEX
#	ifndef WINDOWS_PHONE
	INLINEFUNC bool pthread_mutex_init ( CRITICAL_SECTION  * critSEc, void * arg )
	{
		InitializeCriticalSection ( critSEc );
		return false;
	}

	INLINEFUNC bool pthread_mutex_destroy ( CRITICAL_SECTION  * critSEc )
	{
		DeleteCriticalSection ( critSEc );
		return false;
	}
#	endif
#endif


	INLINEFUNC pthread_t_id getSelfThreadID ()
	{
#if defined(USE_ENVIRONS_POSIX_THREADS)
		return pthread_self ();
#else
		return GetCurrentThreadId ();
#endif
    }
    
    
    INLINEFUNC bool areWeTheThread ( pthread_t thread )
    {
#if defined(USE_ENVIRONS_POSIX_THREADS)
		pthread_t our_thread = pthread_self ();

		return pthread_equal ( our_thread, thread );
#else
        if ( !thread )
            return false;
#ifdef WINDOWS_PHONE
        DWORD threadID = ( DWORD ) thread;
#else
        DWORD threadID = GetThreadId ( thread );
#endif
        if ( !threadID ) // Failed
            return true;
        
        return areWeTheThreadID ( threadID );
#endif
    }
    
    
    INLINEFUNC bool areWeTheThreadID ( pthread_t_id threadID )
    {
#if defined(USE_ENVIRONS_POSIX_THREADS)
		pthread_t our_id = pthread_self ();

		return pthread_equal ( our_id, threadID );
		//return ( memcmp ( &our_id, &threadID, sizeof ( pthread_t ) ) == 0 );
#else
        if ( !threadID )
            return false;
        
        DWORD our_id = GetCurrentThreadId ();
        
        return ( our_id == threadID );
#endif
    }


	INLINEFUNC bool pthread_wait_one ( pthread_cond_t &cond, pthread_mutex_t &lock )
	{
#if defined(USE_ENVIRONS_POSIX_THREADS)
		bool ret = false;

		if ( pthread_mutex_lock ( &lock ) )
			return false;

		if ( !pthread_cond_wait ( &cond, &lock ) )
			ret = true;

		pthread_mutex_unlock ( &lock );
		return ret;
#else
#ifdef WINDOWS_PHONE
		return ( WaitForSingleObjectEx ( cond, INFINITE, TRUE ) == WAIT_OBJECT_0 );
#else
		return ( WaitForSingleObject ( cond, INFINITE ) == WAIT_OBJECT_0 );
#endif
#endif
    }


	INLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(USE_ENVIRONS_POSIX_THREADS)
		return pthread_cond_timedwait_sec ( cond, lock, timeout / 1000 );
#else
		return pthread_cond_timedwait ( cond, lock, &timeout );
#endif
	}


	INLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(USE_ENVIRONS_POSIX_THREADS)
		struct timeval	now;
		struct timespec waitTime;

		int rc = gettimeofday ( &now, NULL );
		if ( rc ) {
			CErrArg ( "pthread_cond_timedwait_sec: failed [ %i ]", rc );
		}

		waitTime.tv_sec = now.tv_sec + timeout;
		waitTime.tv_nsec = now.tv_usec * 1000;
#else
		unsigned int waitTime = timeout * 1000;
#endif
		return pthread_cond_timedwait ( cond, lock, &waitTime );
	}


#if !defined(USE_ENVIRONS_POSIX_THREADS)

#ifdef USE_CRIT_SEC_MUTEX

#if _MSC_VER >= 1800
	_When_ ( return == 0, _Acquires_lock_ ( lock ) )
#endif
	INLINEFUNC int pthread_mutex_lock ( pthread_mutex_t * lock )
	{
		EnterCriticalSection ( lock );
		return 0;
	}


#if _MSC_VER >= 1800
	_When_ ( return == 0, _Acquires_lock_ ( *lock ) )
#endif
	INLINEFUNC int pthread_mutex_trylock ( pthread_mutex_t * lock )
	{
		return !TryEnterCriticalSection ( lock );
	}


#if _MSC_VER >= 1800
	_When_ ( return == 0, _Releases_lock_ ( *lock ) )
#endif
	INLINEFUNC int pthread_mutex_unlock ( pthread_mutex_t * lock )
	{
		LeaveCriticalSection ( lock );
		return 0;
	}
#endif


	INLINEFUNC int pthread_cond_timedwait ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int * timeout )
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
		static char buffer [ 48 ];

		int lenA = sprintf ( buffer, "sem-%s%u-%i-%i", name, name1, name2, name3 );
		if ( lenA <= 0 )
			return false;

		static TCHAR tbuffer [ 128 ];
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
		static char buffer [ 48 ];

		sprintf ( buffer, "sem-%s%u-%i-%i", name, name1, name2, name3 );
		sem_unlink ( buffer );

		sem_t * tsem = sem_open ( buffer, O_CREAT | O_EXCL, 0644, iniVal );
		if ( tsem == SEM_FAILED ) {
			return false;
		}

		*sem = tsem;
#endif

#ifdef ANDROID
		int ret = sem_init ( sem, 0, iniVal );
		if ( ret == -1 ) {
			return false;
		}
#endif
		return true;
	}


	/**
	*	Detach a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DetachThread ( pthread_mutex_t * lock, LONGSYNC * threadState, pthread_t &thread, const char * threadName )
	{
        CVerb ( "DetachThread" );

		if ( threadState != nill && ( ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) )
            return;
        
        if ( lock ) {
            LockAcquire ( lock, "DetachThread" );
        }

        if ( pthread_valid ( thread ) ) {
            // We cannot reset the source as right now, a new (next) thread may have already been started
            //pthread_reset ( thread );
            
            pthread_detach_handle ( thread );

			Zeroh ( thread );
        }
        else {
            CVerb ( "DetachThread: Thread is already detached." );
        }
        
        if ( lock ) {
            LockRelease ( lock, "DetachThread" );
        }
	}


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void JoinThread ( pthread_mutex_t * lock, LONGSYNC * threadState, pthread_t &thread, pthread_t_id threadID, const char * threadName )
	{
		CVerb ( "JoinThread" );

		if ( !pthread_valid ( thread ) ) {
			if ( threadState && ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) {
				CVerbVerb ( "JoinThread: Thread has not been started." );
			}

			CVerb ( "JoinThread: Thread is already closed." );
			return;
		}

		bool threadMatchesCaller	= false;

#if !defined(USE_ENVIRONS_POSIX_THREADS)

		if ( threadID )
			threadMatchesCaller	= areWeTheThreadID ( threadID );
		else
#else
		//pthread_t our_id = pthread_self ();        
		//threadMatchesCaller		= ( memcmp ( &our_id, &thread, sizeof ( pthread_t ) ) == 0 );
#endif
			threadMatchesCaller		= areWeTheThread ( thread );

		if ( threadMatchesCaller ) {
			CVerbArg ( "JoinThread: Skip waiting for [ %s ] ...", threadName );
			return;
		}

		if ( threadState != nill )
		{
			if ( ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
				return;

			if ( !pthread_valid ( thread ) ) {
				CVerb ( "JoinThread: Thread is already closed." );
				return;
			}
		}
        
        // We cannot reset the source as at this point, a new thread may have been already started
		//pthread_reset ( thread );

		CLogsArg ( 2, "JoinThread: Waiting for %s ...", threadName );

        if ( lock ) {
            LockAcquire ( lock, "JoinThread" );
        }
#if defined(USE_ENVIRONS_POSIX_THREADS)
		pthread_join ( thread, NULL );

		Zeros ( thread );
#else
		DWORD dw = WAIT_OBJECT_0;

		if ( !GetExitCodeThread ( thread, &dw ) )
			dw = WAIT_OBJECT_0;

		if ( dw == STILL_ACTIVE )
			WaitForSingleObject ( thread, INFINITE );

		CloseHandle ( thread );	 
#endif
		Zeroh ( thread );

        if ( lock ) {
            LockRelease ( lock, "JoinThread" );
        }
		CLogsArg ( 3, "JoinThread: Waiting for %s done.", threadName );
	}


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DisposeThread ( LONGSYNC * threadState, pthread_t &thread, pthread_t_id threadID, const char * threadName, pthread_cond_t &threadEvent )
	{
		if ( !pthread_valid ( thread ) ) {
			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		if ( pthread_cond_valid ( threadEvent ) ) {
			CVerbArg ( "DisposeThread: Signaling %s thread...", threadName );

#if defined(USE_ENVIRONS_POSIX_THREADS)
			pthread_cond_signal ( &threadEvent );
#else
			SetEvent ( threadEvent );
#endif
		}
        
		JoinThread ( nill, threadState, thread, threadID, threadName );
	}


#ifdef _WIN32
#pragma warning( pop )
#endif


	/**
	*	Initialize a condition variable and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool CondInitBool ( pthread_cond_t * cond, const char * name )
	{
#else
	bool CondInitBool ( pthread_cond_t * cond )
	{
#endif
		if ( !cond ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondInitBool: Invalid cond object for [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondInitBool: Invalid cond object" );
#endif
			return false;
		}

		memset ( cond, 0, sizeof ( pthread_cond_t ) );

		if ( pthread_cond_init ( cond, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondInitBool: Failed to init [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondInitBool: Failed to init" );
#endif
			return false;
		}
		return true;
	}


	/**
	*	Dispose a condition variable and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool CondDisposeBool ( pthread_cond_t * cond, const char * name )
	{
#else
	bool CondDisposeBool ( pthread_cond_t * cond )
	{
#endif
		if ( !cond || pthread_cond_destroy ( cond ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondDisposeBool: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondDisposeBool: Failed to destroy" );
#endif
			return false;
		}
		return true;
	}


	/**
	*	Initialize a mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool LockInitBool ( pthread_mutex_t * mtx, const char * name )
	{
#else
	bool LockInitBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "LockInit: Invalid mutex object for [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "LockInit: Invalid mutex object" );
#endif
			return false;
		}

		memset ( mtx, 0, sizeof ( pthread_mutex_t ) );

		if ( pthread_mutex_init ( mtx, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "LockInit: Failed to init [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "LockInit: Failed to init" );
#endif
			return false;
		}
		return true;
	}


	/**
	*	Dispose a mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool LockDisposeBool ( pthread_mutex_t * mtx, const char * name )
	{
#else
	bool LockDisposeBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_destroy ( mtx ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "LockDispose: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "LockDispose: Failed to destroy" );
#endif
			return false;
		}
		return true;
	}


	/**
	*	Log lock error message.
	*
	*/
#ifdef USE_LOCK_LOG
	void MutexErrorLog ( const char * operation, const char * mutexName, const char * className, const char * funcName )
	{
		CErrArg ( "%s.%s: Failed to %s [ %s ]", className ? className : "Unknown", funcName ? funcName : "Unknown", mutexName ? mutexName : "Unknown", operation );
	}
#else
	void MutexErrorLog ( const char * operation )
	{
		CErrArg ( "MutexErrorLog: Failed to %s ", operation );
	}
#endif

#undef CLASS_NAME
#define CLASS_NAME	""


	/**
	*	Release lock on mutex.
	*
	*/
#ifdef USE_LOCK_LOG
	void LockAcquireVoid ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName )
	{
		CVerbsLockArg ( 12, "        -------> Lock   [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#else
	void LockAcquireVoid ( pthread_mutex_t * mtx )
	{
#endif

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int start = GetEnvironsTickCount32 ();
#endif
		if ( !mtx || pthread_mutex_lock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "lock", className, funcName, mutexName );
			return;
#else
			MutexErrorLog ( "lock" );
#endif
		}

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int end = GetEnvironsTickCount32 ();

		unsigned int diff = end - start;
		if ( diff > maxMeasureDiff ) {
			char timeString [ 256 ];
			GetTimeString ( timeString, sizeof ( timeString ) );

			printf ( "%sLockAcquireVoid: [ %u ms ]\t[ %s : %s : %s ]\n", timeString, diff, className, funcName, mutexName );
		}
#endif

#ifdef USE_LOCK_LOG
		CVerbsLockArg ( 12, "        -------> Locked [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#endif
	}


	/**
	*	Acquire lock on mutex.
	*
	*/
#ifdef USE_LOCK_LOG
	void LockReleaseVoid ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName )
	{
		CVerbsLockArg ( 12, "       <------   Unlock [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#else
	void LockReleaseVoid ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_unlock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "unlock", className, funcName, mutexName );
#else
			MutexErrorLog ( "unlock" );
#endif
		}
	}


	/**
	*	Release lock on mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool LockAcquireBool ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName )
	{
		CVerbsLockArg ( 12, "        -------> Lock   [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#else
	bool LockAcquireBool ( pthread_mutex_t * mtx )
	{
#endif

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int start = GetEnvironsTickCount32 ();
#endif
		if ( !mtx || pthread_mutex_lock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "lock", className, funcName, mutexName );
#else
			MutexErrorLog ( "lock" );
#endif
			return false;
		}

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int end = GetEnvironsTickCount32 ();

		unsigned int diff = end - start;
		if ( diff > maxMeasureDiff ) {
			char timeString [ 256 ];
			GetTimeString ( timeString, sizeof ( timeString ) );

			printf ( "%sLockAcquireBool: [ %u ms ]\t[ %s : %s : %s ]\n", timeString, diff, className, funcName, mutexName );
		}
#endif

#ifdef USE_LOCK_LOG
		CVerbsLockArg ( 12, "        -------> Locked [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#endif
		return true;
	}


	/**
	*	Acquire lock on mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool LockReleaseBool ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName )
	{
		CVerbsLockArg ( 12, "       <------   Unlock [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#else
	bool LockReleaseBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_unlock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "unlock", className, funcName, mutexName );
#else
			MutexErrorLog ( "unlock" );
#endif
			return false;
		}
		return true;
	}

#endif


#undef  CLASS_NAME
#define CLASS_NAME	"EnvThread. . . . . . . ."

	EnvThread::EnvThread ()
	{
		Zeroh ( thread );

#ifndef CLI_CPP
        //Zero ( threadLock );
#endif
		Win32_Only ( threadID = 0; )

        allocated   = false;
		state       = ENVIRONS_THREAD_NO_THREAD;
    }
        
    
    bool EnvThread::Init ()
    {
#ifndef CLI_CPP
        if ( !allocated )
        {
            if ( !LockInitA ( threadLock ) )
                return false;

            allocated = true;
        }
#endif            
        return true;
    }
    
    
    EnvThread::~EnvThread ()
    {
		CThreadVerbArg ( "Destruct: [ %s ]", owner );

        Detach ( "ThreadBase" );

#ifndef CLI_CPP
        if ( allocated ) {
            LockDisposeA ( threadLock );
        }
#endif
    }


	/*
	* Run	Create a thread with the given thread routine
	* @return	1	success, thread is running or was already runing. (if wait is requested, then wait was successful)
	*			0	failed
	*			-1	failed, thread was started and is probably running (soon). However, wait for thread start failed.
	*/
	int EnvThread::Run ( pthread_start_routine_t startRoutine, pthread_param_t arg, CString_ptr func )
	{
		if ( ___sync_val_compare_and_swap ( c_ref state, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
		{
			CWarnsArg ( 6, "Run: [ %s ] Thread is already running!", func );
			return 0;
        }

#ifndef CLI_CPP
		if ( pthread_mutex_trylock ( Addr_of ( threadLock ) ) ) {
			CErrArg ( "Run: [%s] Failed to lock thread for start!", func );
			return 0;
		}
#endif        
		//Zeros ( thread );

        int retCode = pthread_create_tid ( c_Addr_of ( thread ), 0, startRoutine, arg, threadID );

#ifndef CLI_CPP
        LockRelease ( Addr_of ( threadLock ), "Run" );
#endif
		if ( retCode ) {
			CErrArg ( "Run: [ %s ] Failed to create thread!", func );
		}
		else
			return 1;

		___sync_val_set ( c_ref state, ENVIRONS_THREAD_NO_THREAD );
//		state = ENVIRONS_THREAD_NO_THREAD;
		return 0;
	}


	bool EnvThread::isRunning ()
	{
		return ( state != ENVIRONS_THREAD_NO_THREAD );
	}


	void EnvThread::Join ( CString_ptr func )
	{
#ifndef CLI_CPP
#	ifdef _WIN32
		JoinThread ( &threadLock, &state, thread, threadID, func );
#	else
		JoinThread ( &threadLock, &state, thread, 0, func );
#	endif
#else
		if ( thread == nill )
			return;

		if ( Thread::CurrentThread == thread )
			return;

		if ( ___sync_val_compare_and_swap ( state, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) {
			return;
		}

		if ( thread == nill )
			return;

		thread->Join ();
		thread = nill;
#endif
	}


	void EnvThread::Detach ( CString_ptr func )
	{
#ifndef CLI_CPP
		DetachThread ( &threadLock, &state, thread, func );
#else
		thread = nill;
		state = ENVIRONS_THREAD_NO_THREAD;
#endif
	}


	bool EnvThread::areWeTheThread ()
	{
#ifdef CLI_CPP
		return ( Thread::CurrentThread == thread );
#else
#if defined(USE_ENVIRONS_POSIX_THREADS)
		return environs::areWeTheThread ( thread );
		//pthread_t selfThread = pthread_self ();

		//return pthread_equal ( selfThread, thread );
		//return ( memcmp ( &selfThread, &thread, sizeof ( pthread_t ) ) == 0 );
#else
		return areWeTheThreadID ( threadID );
#endif
#endif
	}


#undef  CLASS_NAME
#define CLASS_NAME	"EnvLock. . . . . . . . ."
    
    EnvLock::EnvLock ()
    {
        allocated = false;
        
#ifdef USE_THREADSYNC_OWNER_NAME
        owner       = "Unknown";
#endif
    }
    
    
    bool EnvLock::Init ()
    {
        if ( allocated )
            return true;
        
        if ( !LockInitA ( lockObj ) )
            return false;
        
        allocated = true;
        return true;
    }
    
    
    EnvLock::~EnvLock ()
    {
        CThreadLogArg ( "Destruct: [ %s ]", owner );
        
        DisposeInstance ();
    }
    
    
    void EnvLock::DisposeInstance ()
    {
        CThreadLogArg ( "DisposeInstance: [ %s ]", owner );
        
        if ( allocated ) {
            allocated = false;
            
            LockDisposeA ( lockObj );
        }
    }
    
    
    bool EnvLock::Lock ( CString_ptr func )
    {
        CThreadVerbArg ( "Lock: [ %s ]", owner );

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int start = GetEnvironsTickCount32 ();
#endif        
        if ( pthread_mutex_lock ( Addr_of ( lockObj ) ) ) {
            CErrArg ( "Lock: Failed [ %s ]", func );
            return false;
        }

#ifdef MEASURE_LOCK_ACQUIRE
		unsigned int end = GetEnvironsTickCount32 ();

		unsigned int diff = end - start;
		if ( diff > maxMeasureDiff ) {
			char timeString [ 256 ];
			GetTimeString ( timeString, sizeof ( timeString ) );

			printf ( "%sEnvLock::Lock: [ %u ms ]\t[ %s ]\n", timeString, diff, func );
		}
#endif
        return true;
    }
    
    
    bool EnvLock::Unlock ( CString_ptr func )
    {
        CThreadVerbArg ( "Unlock: [ %s ]", owner );
        
        if ( pthread_mutex_unlock ( Addr_of ( lockObj ) ) ) {
            CErrArg ( "Unlock: Failed [%s]", func );
            return false;
        }
        return true;
    }
        
#ifdef VS2010    
    bool EnvLock::lock ()
    {
		return Lock ( "" );
    }
    
    
    bool EnvLock::unlock ()
    {
		return Unlock ( "" );
    }
#endif
        
#undef  CLASS_NAME
#define CLASS_NAME	"EnvSignal. . . . . . . ."
    
    EnvSignal::EnvSignal ()
    {
        allocated = false;
        autoreset = false;
        
#ifndef _WIN32
        signalState = false;
#endif
		Zeros ( signal );
    }
        

	bool EnvSignal::Init ()
    {
        if ( !EnvLock::Init () )
            return false;

        if ( allocated )
            return true;

		if ( pthread_cond_manual_init ( c_Addr_of ( signal ), NULL ) ) {
			CErr ( "Init: Failed to init signal!" );
			return false;
		}

		allocated = true;
		return true;
	}


	EnvSignal::~EnvSignal ()
	{
		CThreadLogArg ( "Destruct: [ %s ]", owner );

		DisposeInstance ();
	}


	void EnvSignal::DisposeInstance ()
	{
		CThreadLogArg ( "DisposeInstance: [ %s ]", owner );
        
		if ( allocated ) {
			allocated = false;

			CondDisposeA ( signal );
        }
        
        EnvLock::DisposeInstance ();
	}


	bool EnvSignal::LockCond ( CString_ptr func )
	{
		CThreadVerbArg ( "LockCond: [ %s ]", owner );

		if ( pthread_cond_mutex_lock ( c_Addr_of ( lockObj ) ) ) {
			CErrArg ( "LockCond: Failed [ %s ]", func );
			return false;
		}
		return true;
	}


	bool EnvSignal::IsSetDoReset ()
	{
		bool isSet;
        
#ifndef _WIN32
		if ( !LockCond ( "" ) )
			return false;
#endif

#ifndef _WIN32
		isSet = signalState;
		signalState = false;
#else
#	ifdef CLI_CPP
		isSet = signal->WaitOne ( 0 );
		signal->Reset ();
#	else
		if ( WaitForSingleObject ( signal, 0 ) == WAIT_OBJECT_0 ) {
			isSet = true;
			ResetEvent ( signal );
		}
		else
			isSet = false;
#	endif
#endif
        
#ifndef _WIN32
		if ( !UnlockCond ( "" ) )
			return false;
#endif
		return isSet;
	}


	bool EnvSignal::ResetSync ( CString_ptr func, bool useLock, bool keepLocked )
	{
#ifndef _WIN32
		if ( useLock && !LockCond ( func ) )
			return false;
#endif
        
#ifndef _WIN32
        signalState = false;
#else
        if ( pthread_cond_prepare ( c_Addr_of ( signal ) ) ) {
            CErrArg ( "ResetSync: Failed [ %s ]", func );
#ifndef _WIN32
            if ( useLock )
                UnlockCond ( func );
#endif
            return false;
        }
#endif
        
#ifndef _WIN32
		if ( useLock && !keepLocked && !UnlockCond ( func ) )
			return false;
#endif
		return true;
	}


    /**
     * Wait for a given amount of time (or infinite if not given).
     *
     * @return 1 - success, 0 - error, -1 - timeout
     */
	int EnvSignal::WaitOne ( CString_ptr func, int ms, bool useLock, bool keepLocked )
	{
#ifndef _WIN32
		if ( useLock && !LockCond ( func ) )
			return 0;
#endif
		int waitRes = WaitLocked ( func, ms );
		if ( waitRes )
		{
#ifndef _WIN32
			if ( !useLock || keepLocked || UnlockCond ( func ) )
#endif
				return waitRes;
		}
#ifndef _WIN32
		else {
			if ( !keepLocked )
				UnlockCond ( func );
		}
#endif
		return 0;
	}


    /**
     * Wait for a given amount of time (or infinite if not given).
     *
     * @return 1 - success, 0 - error, -1 - timeout
     */
    int EnvSignal::WaitLocked ( CString_ptr func, int ms )
	{
#ifdef _WIN32
#	ifdef CLI_CPP
		bool waitRes;

		if ( ms == ENV_INFINITE_MS )
			waitRes = signal->WaitOne ();
		else
			waitRes = signal->WaitOne ( ms );
#	else
		DWORD waitRes;

		if ( ms == ENV_INFINITE_MS )
#ifdef WINDOWS_PHONE
			waitRes = WaitForSingleObjectEx ( signal, INFINITE, TRUE );
#else
			waitRes = WaitForSingleObject ( signal, INFINITE );
#endif
		else
#ifdef WINDOWS_PHONE
			waitRes = WaitForSingleObjectEx ( signal, ms, TRUE );
#else
			waitRes = WaitForSingleObject ( signal, ms );
#endif
#	endif
#else
		int waitRes;
        
        if ( autoreset || !signalState )
        {
            if ( ms == ENV_INFINITE_MS )
                waitRes = pthread_cond_wait ( &signal, &lockObj );
            else
                waitRes = pthread_cond_timedwait_msec ( &signal, &lockObj, ms );
        }
        else
            waitRes = 0;
#endif
		if ( autoreset ) {
			pthread_cond_preparev ( c_Addr_of ( signal ) );
		}

#ifdef _WIN32
#	ifdef CLI_CPP
		if ( ms != ENV_INFINITE_MS )
			return -1; // No signal means timeout
		if ( waitRes )
			return 1;
#	else
		if ( waitRes == WAIT_OBJECT_0 )
			return 1;
		else if ( waitRes == WAIT_TIMEOUT ) {
			CVerbsArg ( 3, "WaitLocked: Time out [ %s ]", func );
			return -1;
		}
#	endif
#else
		if ( waitRes == 0 )
			return 1;
		else if ( waitRes == ETIMEDOUT ) {
			CVerbsArg ( 3, "WaitLocked: Time out [ %s ]", func );
			return -1;
		}
#endif

#if ( defined(ENVIRONS_CORE_LIB) )
		CErrArg ( "WaitLocked: Failed [ %s ]", func );
#endif
		return 0;
	}


	bool EnvSignal::Notify ( CString_ptr func, bool useLock )
	{
		CThreadVerbArg ( "Notify.Lock: [ %s ]", owner );
        
#ifndef _WIN32
        if ( useLock && pthread_cond_mutex_lock ( c_Addr_of ( lockObj ) ) ) {
            CErrArg ( "Notify.Lock: Failed [ %s ]", func );
			return false;
		}
        
        signalState = true;
#endif
        bool success = true;
        
		if ( pthread_cond_broadcast ( c_Addr_of ( signal ) ) ) {
			CErr ( "Notify: Failed to signal thread_signal!" );
			success = false;
		}
        else {
            CVerb ( "Notify: Thread signaled." );
        }

		CThreadVerbArg ( "Notify.Unlock: [ %s ]", owner );
        
#ifndef _WIN32
		if ( useLock && pthread_cond_mutex_unlock ( c_Addr_of ( lockObj ) ) ) {
			CErr ( "Notify: Failed to release mutex" );
			success = false;
        }
#endif
		return success;
	}


	bool EnvSignal::UnlockCond ( CString_ptr func )
	{
		CThreadVerbArg ( "UnlockCond: [ %s ]", owner );

		if ( pthread_cond_mutex_unlock ( &lockObj ) ) {
			CErrArg ( "UnlockCond: Failed [%s]", func );
			return false;
		}
		return true;
	}
        
        
#undef  CLASS_NAME
#define CLASS_NAME	"ThreadSync . . . . . . ."
        
    
    bool ThreadSync::Init ()
    {
        if ( !EnvSignal::Init () )
            return false;
        
        if ( !EnvThread::Init () )
            return false;
        return true;
    }
        
        
    /*
     * Run	Create a thread with the given thread routine
	* @return	1	success, thread is running or was already runing. (if wait is requested, then wait was successful)
	*			0	failed
	*			-1	failed, thread was started and is probably running (soon). However, wait for thread start failed.
	*/
	int ThreadSync::Run ( pthread_start_routine_t startRoutine, pthread_param_t arg, CString_ptr func, bool waitForStart )
	{
		if ( ___sync_val_compare_and_swap ( c_ref state, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
		{
			CWarnsArg ( 6, "Run: [%s] Thread already running!", func );
			return 0;
        }

		//Zeros ( thread );

        int retCode;

		if ( waitForStart && !ResetSync ( func, true, true ) )
			goto Failed;

#ifndef CLI_CPP
		if ( pthread_mutex_trylock ( Addr_of ( threadLock ) ) ) {
			CErrArg ( "Run: [%s] Failed to lock thread for start!", func );
			goto Failed;
		}
#endif        
		retCode = pthread_create_tid ( c_Addr_of ( thread ), 0, startRoutine, arg, threadID );

#ifndef CLI_CPP
        LockRelease ( Addr_of ( threadLock ), "Run" );
#endif
		if ( retCode ) {
			CErrArg ( "Run: [%s] Failed to create thread!", func );
            
#ifndef _WIN32
			if ( waitForStart )
                UnlockCond ( func );
#endif
		}
		else {
			if ( !waitForStart )
				return 1;

			int waitRes = WaitOne ( func, 4000, false, false );
            
#ifndef _WIN32
			UnlockCond ( func );
#endif
			if ( waitRes <= 0 ) {
				CErrArg ( "Run: [%s] Failed to wait for thread start!", func );
			}

			if ( waitRes != 0 )
				return waitRes;
		}

    Failed:
        ___sync_val_set ( c_ref state, ENVIRONS_THREAD_NO_THREAD );
//		state = ENVIRONS_THREAD_NO_THREAD;
		return 0;
	}

}








