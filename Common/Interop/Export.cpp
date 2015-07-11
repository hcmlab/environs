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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Interop/Export.h"
#include <stdio.h>
#include "Environs.Native.h"
#include "Interop/Stat.h"
#include "Interfaces/Interface.Exports.h"
#include "Environs.h"
using namespace environs;

#ifdef WINDOWS_PHONE
#include "winbase.h"
#endif

#define CLASS_NAME	"Export"


HMODULE LocateLoadEnvModule ( COBSTR module, unsigned int deviceID )
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
	
	HMODULE hModLib = dlopen ( module, RTLD_LAZY );
	if ( !hModLib )
	{
		CVerbVerbArgID ( "LocateLoadModule: [%s] not found in system search path.", absPath );

		do
		{
			sprintf ( absPath, "%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule: [%s] not found in system search path.", absPath );

#ifdef _WIN32
			sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule: [%s] not found in toolset search path.", absPath );
#endif
			sprintf ( absPath, LIBNAME_EXT_DIR "/%s" LIBEXTENSION, module );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;

            if ( !environs::environs.workDir ) {
				CVerbArgID ( "LocateLoadModule: [%s" LIBEXTENSION "] not available in search path. No working directory available.", module );
				return 0;
			}
			CVerbArgID ( "LocateLoadModule: [%s] not found in search path.", absPath );

#ifndef _WIN32
			CVerbArgID ( "LocateLoadModule: [%s]", dlerror ( ) );
#endif

#ifdef _WIN32
			sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, environs::environs.workDir, module );

			CVerbArgID ( "LocateLoadModule: Trying [%s].", absPath );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;
			CVerbVerbArgID ( "LocateLoadModule: [%s] not found in toolset working directory path.", absPath );
#endif

			sprintf ( absPath, "%s%s" LIBEXTENSION, environs::environs.workDir, module );

			CVerbArgID ( "LocateLoadModule: Trying [%s].", absPath );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( hModLib )
				break;

#ifdef _WIN32
			CVerbArgID ( "LocateLoadModule: [%s] not found.", absPath );
#else
			CVerbArgID ( "LocateLoadModule: [%s]", dlerror ( ) );
#endif

			sprintf ( absPath, "%s" LIBNAME_EXT_DIR "/%s" LIBEXTENSION, environs::environs.workDir, module );

			CVerbArgID ( "LocateLoadModule: Trying [%s].", absPath );

			hModLib = dlopen ( absPath, RTLD_LAZY );
			if ( !hModLib ) {
#ifdef _WIN32
				CLogArgID ( "LocateLoadModule: [%s] not found.", absPath );
#else
				CLogArgID ( "LocateLoadModule: [%s]", dlerror ( ) );
#endif
			}
		}
		while ( 0 );
	}
#endif

    if ( hModLib ) {
        pSetEnvironsObject SetEnvironsObject = 0;
        
        SetEnvironsObject = (pSetEnvironsObject) dlsym ( hModLib, MODULE_EXPORT_ENVIRONSOBJ );
        if ( SetEnvironsObject ) {
            CVerbID ( "LocateLoadModule: injecting Environs methods." );
            
            SetEnvironsObject ( (void *)&environs::environs );
        }
		else {
			CVerbID ( "LocateLoadModule: Cannot find SetEnvironsObject in module. Assuming a 3rd party module." );
        }
    }
    
	return hModLib;
}


#ifdef _WIN32
#pragma warning( pop )
#endif
