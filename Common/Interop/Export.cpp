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
#endif

#include "Interop/Export.h"
#include "Environs.Native.h"
#include "Environs.Release.h"
#include "Interfaces/Interface.Exports.h"

#if (!defined(MEDIATORDAEMON))
#	include "Environs.Obj.h"
#endif
using namespace environs;

#ifdef WINDOWS_PHONE
#	include "winbase.h"
#else
#	include <stdio.h>
#endif

#define CLASS_NAME	"Export . . . . . . . . ."


HLIB LocateLoadEnvModule ( COBSTR module, int deviceID, Instance * obj )
{
	CVerbID ( "LocateLoadModule" );

#ifdef WINDOWS_PHONE
	HMODULE hModLib = LoadPackagedLibrary ( module, 0 );
	if ( !hModLib ) {
		CErrArgID ( "LocateLoadModule: [%ws] not available in search path. No working directory available.", module );
	}
#else

	char absPath [1024];

	sprintf ( absPath, "%s" LIBEXTENSION, module );

	HLIB hModLib = dlopen ( module, RTLD_LAZY );
	if ( !hModLib )
	{
		CVerbVerbArgID ( "LocateLoadModule:  [ %s ] not found in system search path.", absPath );

		do
		{
			bool hasPrefix = ( strstr ( module, ENVMODPREFIX ) == module );

			/**
			* Try adding libEnv- prefix and let plattform search for the usual suspects ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, ENVMODPREFIX "%s" LIBEXTENSION, module );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
				CVerbVerbArgID ( "LocateLoadModule: libEnv- [ %s ] not found in system search path.", absPath );
			}

			/**
			* Let plattform search for the usual suspects ...
			*/
			sprintf ( absPath, "%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule:  [ %s ] not found in system search path.", absPath );

#ifdef _WIN32
			/**
			* Try adding libEnv- prefix and search in libs folder with absolute path ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/" ENVMODPREFIX "%s" LIBEXTENSION, module );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
				CVerbVerbArgID ( "LocateLoadModule: libEnv- [ %s ] not found in toolset search path.", absPath );
			}

			/**
			* Search in libs folder with absolute path ...
			*/
			sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule:  [ %s ] not found in toolset search path.", absPath );
#endif
			/**
			* Try adding libEnv- prefix and search in libs folder without absolute path ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVMODPREFIX "%s" LIBEXTENSION, module );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
			}

			/**
			* Search in libs folder without absolute path ...
			*/
			sprintf ( absPath, LIBNAME_EXT_DIR "/%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;

#ifndef MEDIATORDAEMON
            const char * libDir = environs::native.libDir;
            if ( !libDir )
                libDir = environs::native.workDir;

            if ( !libDir ) {
				CVerbArgID ( "LocateLoadModule: [%s" LIBEXTENSION "] not available in search path. No working directory available.", module );
				return 0;
			}
			CVerbArgID ( "LocateLoadModule:  [ %s ] not found in search path.", absPath );

#ifndef _WIN32
			CVerbArgID ( "LocateLoadModule:  [ %s ]", dlerror ( ) );
#endif

#ifdef _WIN32
			/**
			* Try adding libEnv- prefix and search in libs folder and platform toolset with absolute path ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/" ENVMODPREFIX "%s" LIBEXTENSION, environs::native.workDir, module );

				CVerbArgID ( "LocateLoadModule: Trying  [ %s ].", absPath );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
			}

			/**
			* Search in libs folder and platform toolset with absolute path ...
			*/
			sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, environs::native.workDir, module );

			CVerbArgID ( "LocateLoadModule: Trying  [ %s ].", absPath );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule:  [ %s ] not found in toolset working directory path.", absPath );
#endif
			/**
			* Try adding libEnv- prefix and search in libs folder without absolute path ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, "%s" ENVMODPREFIX "%s" LIBEXTENSION, libDir, module );

				CVerbArgID ( "LocateLoadModule: Trying  [ %s ].", absPath );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
			}

			/**
			* Search in libs folder without absolute path ...
			*/
			sprintf ( absPath, "%s%s" LIBEXTENSION, libDir, module );

			CVerbArgID ( "LocateLoadModule: Trying [ %s ].", absPath );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;

#ifdef _WIN32
			CVerbArgID ( "LocateLoadModule:  [ %s ] not found.", absPath );
#else
			CVerbArgID ( "LocateLoadModule:  [ %s ]", dlerror ( ) );
#endif
			/**
			* Try adding libEnv- prefix and search in libs folder with absolute path ...
			*/
			if ( !hasPrefix )
			{
				sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/" ENVMODPREFIX "%s" LIBEXTENSION, libDir, module );

				CVerbArgID ( "LocateLoadModule: Trying  [ %s ].", absPath );

				hModLib = dlopen ( absPath, RTLD_LAZY );
				if ( hModLib )
					break;
			}

			/**
			* Search in libs folder with absolute path ...
			*/
			sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/%s" LIBEXTENSION, libDir, module );

			CVerbArgID ( "LocateLoadModule: Trying  [ %s ].", absPath );

            hModLib = dlopen ( absPath, RTLD_LAZY );
#endif
			if ( !hModLib ) {
#ifdef _WIN32
				CLogArgID ( "LocateLoadModule:  [ %s ] not found.", absPath );
#else
				CLogArgID ( "LocateLoadModule:  [ %s ]", dlerror ( ) );
#endif
			}
		}
		while ( 0 );
	}
#endif

#ifndef MEDIATORDAEMON
	if ( hModLib ) {
		pSetEnvironsObject SetEnvironsObject = 0;

		SetEnvironsObject = ( pSetEnvironsObject ) dlsym ( hModLib, MODULE_EXPORT_ENVIRONSOBJ );
		if ( SetEnvironsObject ) {
			CVerbID ( "LocateLoadModule: injecting Environs methods." );

			SetEnvironsObject ( ( void * ) obj, ( void * ) &native );
		}
		else {
			CVerbID ( "LocateLoadModule: Cannot find SetEnvironsObject in module. Assuming a 3rd party module." );
		}
	}
#endif

	return hModLib;
}


#ifdef _WIN32
#pragma warning( pop )
#endif
