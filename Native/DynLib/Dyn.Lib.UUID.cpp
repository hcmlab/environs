/**
 * Dynamically accessing uuid
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

#if defined(LINUX)

#include "Environs.Native.h"
#include "Interop.h"
#include "Dyn.Lib.UUID.h"
#include "Environs.Utils.h"
using namespace environs;

#include <errno.h>

// Disable this flag to use library as statically linked library again
#ifndef ENVIRONS_IOS
#   define USE_DYNAMIC_LIB
#endif

#define CLASS_NAME	"Dyn.Lib.uuid . . . . . ."

#define	MODLIBNAME	"libuuid.so"

namespace environs
{
	static bool						uuidlib_LibInitialized		= false;
	HMODULE							hLibUUID					= 0;

	extern void ReleaseLibUUID ();

	extern bool InitLibUUID ( int deviceID );

	puuid_generate					duuid_generate				= 0;
	puuid_is_null					duuid_is_null				= 0;
	puuid_unparse_upper				duuid_unparse_upper			= 0;

	bool VerifyLibUUID ()
	{
		if ( !duuid_generate || !duuid_is_null || !duuid_unparse_upper ) {
			CWarn ( "VerifyLibUUID: One of the " MODLIBNAME " functions could not be loaded!" );
			return false;
		}

		return true;
	}


	void ReleaseLibUUID ()
	{
		CVerb ( "ReleaseLibUUID" );

		uuidlib_LibInitialized		= false;

		duuid_generate              = 0;
		duuid_is_null				= 0;
		duuid_unparse_upper			= 0;

		if ( hLibUUID ) {
			dlclose ( hLibUUID );
			hLibUUID = 0;
		}
	}


#ifdef USE_DYNAMIC_LIB


    void euuid_generate ( uuid_t uuid )
    {
        srand ( GetEnvironsTickCount32 () );

        for ( int i = 0; i < 16; ++i )
            uuid [ i ] = (unsigned char) rand();
    }

    int euuid_is_null ( const uuid_t uuid )
    {
        int v = 0;

        for ( int i = 0; i < 16; ++i )
            v += uuid [ i ];
        return ( v == 0 ? 1 : v );
    }


    void euuid_unparse_upper ( const uuid_t uuid, char * buffer )
    {
        int i = 4; int p = 0;

        while ( i > 0 ) {
            sprintf ( buffer, "%2X", uuid [ p ] ); --i; ++p; buffer++;
        }

        *buffer = '-'; buffer++;

        i = 2; while ( i > 0 ) {
            sprintf ( buffer, "%2X", uuid [ p ] ); --i; ++p; buffer++;
        }

        *buffer = '-'; buffer++;

        i = 2; while ( i > 0 ) {
            sprintf ( buffer, "%2X", uuid [ p ] ); --i; ++p; buffer++;
        }

        *buffer = '-'; buffer++;

        i = 2; while ( i > 0 ) {
            sprintf ( buffer, "%2X", uuid [ p ] ); --i; ++p; buffer++;
        }

        *buffer = '-'; buffer++;

        i = 6; while ( i > 0 ) {
            sprintf ( buffer, "%2X", uuid [ p ] ); --i; ++p; buffer++;
        }
    }


	bool InitLibUUID ( int deviceID )
	{
		CVerb ( "InitLibIW" );

		if ( uuidlib_LibInitialized ) {
			CVerbID ( "InitLibUUID: already initialized." );
			return true;
		}

		HMODULE				hLib	= 0;
        bool				ret = false;

		hLib = dlopen ( MODLIBNAME, RTLD_LAZY );
		if ( !hLib ) {
#ifdef _WIN32
			CWarnArgID ( "InitLibUUID: Loading of " MODLIBNAME " FAILED with error [ 0x%.8x ]", GetLastError () );
#else
			CWarnArgID ( "InitLibUUID: Loading of " MODLIBNAME " FAILED with error [ 0x%.8x ]", errno );
#endif
            duuid_generate			= ( puuid_generate ) euuid_generate;
            duuid_is_null			= ( puuid_is_null ) euuid_is_null;
            duuid_unparse_upper		= ( puuid_unparse_upper ) euuid_unparse_upper;

            CVerbID ( "InitLibUUID: Using internal implementation." );

            uuidlib_LibInitialized = true;
            return true;
		}

		duuid_generate			= ( puuid_generate ) dlsym ( hLib, "uuid_generate" );
		duuid_is_null			= ( puuid_is_null ) dlsym ( hLib, "uuid_is_null" );
		duuid_unparse_upper		= ( puuid_unparse_upper ) dlsym ( hLib, "uuid_unparse_upper" );

		if ( !VerifyLibUUID () ) {
			goto Finish;
		}

		ret = true;

	Finish:
		if ( ret ) {
			hLibUUID = hLib;
			uuidlib_LibInitialized = true;
			CVerbID ( "InitLibUUID: successfully initialized access to " MODLIBNAME );
		}
        else {
            CErrID ( "InitLibUUID: Failed to initialize " MODLIBNAME );
			ReleaseLibUUID ();
		}

		return ret;
	}


#else

	bool InitLibUUID ( int deviceID )
	{
		CVerbID ( "InitLibUUID" );

		if ( uuidlib_LibInitialized ) {
			CVerbID ( "InitLibUUID: already initialized." );
			return true;
		}

		duuid_generate			= ( puuid_generate ) uuid_generate;
		duuid_is_null			= ( puuid_is_null ) uuid_is_null;
		duuid_unparse_upper		= ( puuid_unparse_upper ) uuid_unparse_upper;

        if ( !VerifyLibUUID ( ) ) {
            CErrID ( "InitLibUUID: Failed to initialize " MODLIBNAME );
			goto Failed;
		}

		uuidlib_LibInitialized = true;

		CVerbID ( "InitLibUUID: successfully initialized access to " MODLIBNAME );
		return true;

	Failed:
		ReleaseLibUUID ();
		return false;
	}
#endif



} // -> namespace environs

#endif



