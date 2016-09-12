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
#include <stdint.h>

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <stdio.h>

#if !defined(WINDOWS_PHONE) && !defined(_WIN32)
#   include <stdlib.h>
#endif

#include <stdarg.h>

#ifndef MEDIATORDAEMON
#   include "Environs.Lib.h"
#   include <direntw.h>
#endif

#include "Environs.Native.h"
#include "Interop/Stat.h"
#include "Interop.h"

#if defined(_WIN32)
#	ifndef WINDOWS_PHONE
#		include "windows.h"
#	endif
#else // <- _WIN32 ->
#   include <iostream>
#   include <string>
#   include <termios.h>
#   include <unistd.h>

#   if defined(__APPLE__) || defined(ANDROID)
#       include <time.h>
#   else  // <- __APPLE__ | ANDROID ->
#       include <time.h>
#       include <sys/times.h>
#   endif

#   include <errno.h>
#endif // <-- end of _WIN32

#ifdef __APPLE__
#   include <mach/mach_time.h>
#endif

#define CLASS_NAME	"Environs.Utils . . . . ."


namespace environs
{
	void RefactorBuffer ( char * &curStart, char * bufferStart, unsigned int bytesInBuffer, char * &curEnd )
	{
		CVerbVerbArg ( "RefactorBuffer: Refactoring memory usage, bytesInBuffer [ %i ]", bytesInBuffer );

		// Refactor memory usage
		int freeBytesFromStart = (int) (curStart - bufferStart);
		if ( freeBytesFromStart <= 0 ) {
			CVerbVerb ( "RefactorBuffer: Refactoring not neccessary since buffer is already pointing to the start." );
			return;
		}

		if ( bytesInBuffer < (unsigned int) freeBytesFromStart ) { // No overlap
			memcpy ( bufferStart, curStart, bytesInBuffer );
		}
		else { // Overlap, so alloc heap space
            void * tmp = malloc ( bytesInBuffer );
            if ( !tmp ) {
                CErrArg ( "RefactorBuffer: Allocation of memory failed [ %i ]", bytesInBuffer );
                return;
            }
            
            memcpy ( tmp, curStart, bytesInBuffer );
            
            memcpy ( bufferStart, tmp, bytesInBuffer );
            
            free ( tmp );
        }
        
		curStart    = bufferStart;
		curEnd      = bufferStart + bytesInBuffer;
	}


#ifdef _WIN32
#	pragma warning( push )
#	pragma warning( disable: 4996 )
#endif
    
    size_t GetSizeOfFile ( const char * filePath )
	{
#if (defined(_WIN32) && !defined(WINDOWS_PHONE))
		HANDLE handle = CreateFileA ( filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( handle == INVALID_HANDLE_VALUE)
			return 0;

		size_t fileSize = 0;

		LARGE_INTEGER size;

		if ( GetFileSizeEx( handle, &size ) )
		{
			fileSize = (size_t) size.QuadPart;
		}

		CloseHandle ( handle );
		return fileSize;
#else
		STAT_STRUCT ( st );

        if ( stat ( filePath, &st ) != 0 )
            return 0;
        return (size_t) st.st_size;
#endif
    }
    
    
	char * LoadBinary ( const char * filePath, int * size )
	{
		// Get filesize if it exists 
		size_t fileSize = 0;
		int bytesRead = 0;
		bool ret = false;

        fileSize = GetSizeOfFile ( filePath );
        CVerbVerbArg ( "LoadBinary: Filesize [%d]", fileSize );
        
        if ( fileSize < 1 )
            return 0;

		char * binary = (char *) malloc ( fileSize + 2 );
        if ( !binary ) {
            CVerbVerb ( "LoadBinary: malloc failed" );
			return 0;
		}

        FILE * fp = fopen ( filePath, "rb" );
        CVerbVerbArg ( "LoadBinary: Opening [%s]", filePath );
        
		if ( !fp )
            goto Finish;
        CVerbVerbArg ( "LoadBinary: Opened [%s]", filePath );

		//rewind ( fp );
		bytesRead = (unsigned int) fread ( binary, 1, fileSize, fp );
		if ( bytesRead != (int) fileSize ){
			CErrArg ( "LoadBinary: Read file [%s] failed read [%i] != expected [%i].", filePath, bytesRead, (int) fileSize );
			goto Finish;
        }
        CVerbVerb ( "LoadBinary: Read" );

		binary [fileSize] = 0;
		if ( size )
			*size = (int) fileSize;
		ret = true;

	Finish:
		if ( !ret ) {
			free ( binary );
			binary = 0;
		}
		if ( fp )
			fclose ( fp );
        
		return binary;
	}


	char * LoadPrivateBinary ( const char * fileName, int * size )
	{
		return LoadBinary ( fileName, size );
	}


	bool SavePrivateBinary ( const char * fileName, const char * buffer, int size )
	{
		if ( !fileName || !buffer ) {
			CErr ( "SavePrivateBinary: Invalid NULL parameters." );
			return 0;
		}

		int bytesWritten = 0;
		bool ret = false;
		
		FILE * fp = fopen ( fileName, "wb" );
		if ( !fp ) {
			CVerbArg ( "SavePrivateBinary: Cannot create file [%s].", fileName );
			return false;
		}

		bytesWritten = (int)fwrite ( buffer, 1, size, fp );
		if ( bytesWritten != size ) {
			CVerbArg ( "SavePrivateBinary: Write file [%s] failed. Written [%i] != expected [%i].", fileName, bytesWritten, size );
			goto Finish;
		}
        fflush ( fp );

		ret = true;

	Finish:
        fclose ( fp );
		return ret;
	}

#ifdef _WIN32
#   pragma warning( pop )
#endif

    
#ifdef __APPLE__
    mach_timebase_info_data_t environs_time_base_info;
#endif


#if ( !defined(__APPLE__) && (defined(_WIN32) || defined(LINUX)) )
	size_t strlcpy ( char *d, char const *s, size_t n )
	{
		if ( !d || !s || n <= 0 )
			return 0;

		size_t c = 0;

		for ( --n; n > 0 && *s ; --n, ++d, ++s, ++c )
			*d = *s;

		*d = 0;

		return c;
	}

	size_t strlcat ( char *d, char const *s, size_t n )
	{
		if ( !d || !s || n <= 0 )
			return 0;

		for ( --n; n > 0 && *d ; --n, ++d );

		if ( n <= 0 )
			return 0;

		size_t c = 0;

		for ( ; n > 0 && *s ; --n, ++d, ++s, ++c )
			*d = *s;

		*d = 0;

		return c;
	}
#endif
    
    
//#ifndef _WIN32    
//#   ifndef ANDROID
//#       ifndef __APPLE__
//    size_t strlcpy ( char *d, char const *s, size_t n )
//    {
//        return snprintf ( d, n, "%s", s );
//    }
//#       endif
//    // <- Linux/iOS includes
//#   endif
//    // <- POSIX includes
//#endif
    
#if (defined(_WIN32))
	// Number of milliseconds since system has started
	//
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        return ( INTEROPTIMEVAL ) GetTickCount64 ();
    }
    
#if !defined(WINDOWS_PHONE)
#pragma warning(push)
#pragma warning(disable:4310)
    
    int gettimeofday ( struct timeval *tv, void * )
    {
        if ( !tv )
            return 0;
        
        static const LONGLONG        UNIX_DELTA_IN_MICSECS = 116444736000000000;
        
        ULARGE_INTEGER  ul, bunix;
        FILETIME        ft;
        LONGLONG        usecs;
        
        GetSystemTimeAsFileTime ( &ft );
        ul.LowPart  = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        
        bunix.LowPart = (DWORD)UNIX_DELTA_IN_MICSECS;
        bunix.HighPart = UNIX_DELTA_IN_MICSECS >> 32;
        
        usecs = (LONGLONG)((ul.QuadPart - bunix.QuadPart) / 10);
        
        tv->tv_sec = (long)(usecs /   1000000);
        tv->tv_usec = (long)(usecs % 1000000);
        
        return 0;
    }
    
#pragma warning(pop) 
    
#endif
#endif
    
    
#ifdef __APPLE__
    // return milliseconds
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        //static double environs_time_base_factor = 0.000063999999999999997;

        if ( !environs_time_base_info.denom ) {
            mach_timebase_info ( &environs_time_base_info );

            if ( !environs_time_base_info.denom )
                return 0;

            //environs_time_base_factor = (double) environs_time_base_info.numer / ( (double) ( 15625 * environs_time_base_info.denom) );
        }

        //return ((mach_absolute_time() >> 6) * environs_time_base_factor);
        return ((mach_absolute_time() / 1000000) * environs_time_base_info.numer) / environs_time_base_info.denom;
    }
#endif
    
#if !defined(_WIN32) && !defined(__APPLE__)
    // return milliseconds
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        struct timespec	ts;
        
        if ( clock_gettime ( CLOCK_REALTIME, &ts ) != 0 ) {
            CErrArg ( "GetEnvironsTickCount: Failed to fetch time [%s]", strerror ( errno ) );
            return 0;
        }
        
		return ( ( ts.tv_sec * 1000 ) + ( ts.tv_nsec  > 0 ? ( ts.tv_nsec / 1000000 ) : 0 ) );
    }
#endif
    
    
    unsigned int GetEnvironsTickCount32 ()
    {
        return (GetEnvironsTickCount () & 0xFFFFFFFF);
    }
    
    
    unsigned long long   GetUnixEpoch ()
    {
        unsigned long long unixEpoch = 0;
        
#ifdef _WIN32
        SYSTEMTIME st;
        GetSystemTime( &st );
        
        FILETIME ft;
		if ( !SystemTimeToFileTime ( &st, &ft ) )
			return 0;
        
        unsigned long long ticks = (((unsigned long long) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
        
		unixEpoch = ticks / 10000000 - 11644473600LL;
#else
        unixEpoch = time ( NULL );
#endif
        return unixEpoch;
    }

    
	unsigned int getRandomValue ( void * value )
	{
		unsigned int now = (unsigned int) GetEnvironsTickCount ();
		return now + (unsigned int) (uintptr_t) value;
	}
    
    
#ifndef MEDIATORDAEMON
    
    bool IsValidDirectory ( const char * path, size_t length )
    {
        if ( length < 1 || !path )
            return false;
        
        const char * found = strstr ( path, ".." );
        if ( found )
        {
            if ( found == path && length <= 3 )
                return false;
            
            if ( length <= 4 )
                return false;
        }
        
        found = strstr ( path, "." );
        if ( found )
        {
            if ( found == path && length <= 2 )
                return false;
            
            if ( length <= 3 )
                return false;
        }
        
        return true;
    }
    
    
#define MAX_STORAGE_SIZE    1024
    
    
    void RmDir ( char * storage, size_t storageLen )
    {
        DIR * dir = opendir ( storage );
        if ( !dir )
            return;
        
        storage [ storageLen ] = '/';
        storage [ storageLen + 1 ] = 0;
        
        storageLen++;
        
        struct dirent * dirEntry;
        
        while ( ( dirEntry = readdir ( dir ) ) != 0 )
        {
            const char * d_name = dirEntry->d_name;
            
#ifdef _DIRENT_HAVE_D_NAMLEN
            int length = dirEntry->d_namlen;
#else
            size_t length = strlen ( d_name );
#endif
            if ( dirEntry->d_type == DT_DIR )
            {
                if ( !IsValidDirectory ( d_name, length ) )
                    continue;
                
                strlcpy ( storage + storageLen, d_name, MAX_STORAGE_SIZE - storageLen );
                
                size_t storageLenAppended = storageLen + length;
                
                RmDir ( storage, storageLenAppended );
                
                rmdir ( storage );
            }
            else {
                strlcpy ( storage + storageLen, d_name, MAX_STORAGE_SIZE - storageLen );
                
                unlink ( storage );
            }
        }
        
        closedir ( dir );
        
        storage [ storageLen - 1 ] = 0;
    }
    
    
    void ClearStorage ( const char * storagePath )
    {
        if ( !storagePath || !*storagePath )
            return;
        
        DIR * dir = opendir ( storagePath );
        if ( !dir )
            return;
        
        size_t storageLen = 0;

		char * storage = ( char * ) calloc ( 1, MAX_STORAGE_SIZE + 1 );
        if ( storage )
        {            
            strlcpy ( storage, storagePath, MAX_STORAGE_SIZE );
            
            storageLen = strlen ( storage );
            
            storage [ storageLen ] = '/';
            storage [ storageLen + 1 ] = 0;
            
            storageLen++;
            
            struct dirent * dirEntry;
            
            while ( ( dirEntry = readdir ( dir ) ) != 0 )
            {
                if ( dirEntry->d_type != DT_DIR )
                    continue;
                
                const char * d_name = dirEntry->d_name;
                
#ifdef _DIRENT_HAVE_D_NAMLEN
                int length = dirEntry->d_namlen;
#else
                size_t length = strlen ( d_name );
#endif
                if ( !IsValidDirectory ( d_name, length ) )
                    continue;
                
                strlcpy ( storage + storageLen, d_name, MAX_STORAGE_SIZE - storageLen );
                
                size_t storageLenAppended = storageLen + length;
                
                RmDir ( storage, storageLenAppended );
                
                rmdir ( storage );
            }
            
            free ( storage );
        }
    
        closedir ( dir );
    }
    
    
#endif
    
#ifndef MEDIATORDAEMON

    namespace API
    {
        int Environs_LoginDialogCommandLine ( int hInst )
        {
#ifndef WINDOWS_PHONE
            char user [ 128 ];
            char pass [ 128 ];
            
            printf ( ">> Logon to Mediator requires username and password.\n\n" );
            printf ( ">> Please enter a username: " );
            
            char * line = fgets ( user, sizeof(user), stdin );
            
            size_t length = 0;
            
            if ( !line || (length = strlen(line)) <= 0 ) {
                CErr ( "LoginDialog: Invalid username!" );
                printf ( ">> LoginDialog: Invalid username!\n" );
                return 0;
            }
            line [ length - 1 ] = 0;
            
            printf ( ">> Please enter a password: " );

#ifdef _WIN32
            HANDLE  hIn     = GetStdHandle ( STD_INPUT_HANDLE );
            DWORD   mode    = 0;

            GetConsoleMode ( hIn, &mode );
            SetConsoleMode ( hIn, mode & (~ENABLE_ECHO_INPUT) );
#else
            termios old;
            tcgetattr ( STDIN_FILENO, &old );
            
            termios term = old;
            term.c_lflag &= ~ECHO;
            
            tcsetattr ( STDIN_FILENO, TCSANOW, &term );
#endif
            
            line = fgets ( pass, sizeof(pass), stdin );
            
#ifdef _WIN32
            SetConsoleMode ( hIn, mode );
#else
            tcsetattr ( STDIN_FILENO, TCSANOW, &old );
#endif
            if ( !line || (length = strlen(line)) <= 0 ) {
                CErr ( "LoginDialog: Invalid password!" );
                printf ( ">> LoginDialog: Invalid password!\n" );
                return 0;
            }
            
            line [ length - 1 ] = 0;
            
            FAKEJNI ();
            
            environs::API::SetMediatorUserNameNM ( hInst, user );
            
            environs::API::SetMediatorPasswordNM ( hInst, pass );
            
            EnvironsCallArg ( RegisterAtMediatorsN, hInst );

#endif
            return 1;
        }
        
    }

#endif

}
