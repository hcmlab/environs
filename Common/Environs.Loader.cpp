/**
 * Environs Loader
 * Loads the Environs library and creates an instance of Environs.
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

#include "Environs.h"
#include "Environs.Release.h"
#include "Environs.Native.h"

#ifdef __APPLE__
#include <dlfcn.h>
#endif

#include <string>

#ifdef _WIN32
# include <direct.h>
#endif

using namespace std;
using namespace environs::lib;


#define CLASS_NAME	"Environs.Loader. . . . ."


void ShowLibMissingDialog ( const char * libName );


namespace environs
{
#ifdef USE_ENVIRONS_LOG_POINTERS
    pCOutLog    COutLog     = 0;
    pCOutArgLog COutArgLog  = 0;
#endif
    
	void EnvironsDisposer ( IEnvironsDispose * obj )
	{
		if ( obj ) ( ( IEnvironsDispose * ) obj )->Release ();
	}

	namespace lib
	{        
		/// C function implemented in the loader (statically build into each app)
		/*sp ( IEnvirons ) ENVIRONS_CreateInstance ()
		{
			sp ( IEnvirons ) obj ( (IEnvirons *) ENVIRONS_CreateInstance1 ( ENVIRONS_BUILD_CRT ) );
			return obj;
		}
		*/

        
		/// C function implemented in the loader (statically build into each app)
		/*void * CallConv ENVIRONS_CreateInstanceObject ()
		{
			return ENVIRONS_CreateInstance1 ( ENVIRONS_BUILD_CRT );
		}
		*/
	}


	namespace Loader
    {
        HMODULE		g_EnvironsModuleHandle	= 0;
        
		/// Forward declarations
		environs::Environs * LocateLoadEnvirons ( COBSTR module, int crt );
        
        void DisposeEnvironsLib ();
        
        
        class StaticDisposer
        {
        public:
            ~StaticDisposer () {
                DisposeEnvironsLib ();
            }
        };
        
        StaticDisposer staticDisposer;
        
        
#ifdef USE_ENVIRONS_LOG_POINTERS
        void LocateLoadLogMethods ( COBSTR module, int crt, void ** outLog, void ** outLogArg );
#endif


		/**
		* Create an Environs object.
		*
		* @return   An Environs object wrapped into a smart pointer.
		*/
		sp ( environs::Environs ) ENVIRONS_CreateInstance ()
		{
			sp ( environs::Environs ) obj ( LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT ), EnvironsDisposer );
			return obj;
		}


		/**
		* Create an Environs object.
		*
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object wrapped into a smart pointer.
		*/
		sp ( environs::Environs ) ENVIRONS_CreateInstance ( const char * appName, const char * areaName )
		{
			sp ( environs::Environs ) obj ( LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT ), EnvironsDisposer );
			if ( obj )
				obj->LoadSettings ( appName, areaName );
			return obj;
		}


		/**
		* Create an Environs object.
		*
		* @return   An Environs object interface.
		*/
		environs::Environs * ENVIRONS_CreateInstanceObject ()
		{
			/// Load Environs.dll and create an instance
			return LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT );
		}


		/**
		* Create an Environs object.
		*
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object interface.
		*/
		environs::Environs * ENVIRONS_CreateInstanceObject ( const char * appName, const char * areaName )
		{
			/// Load Environs.dll and create an instance
			environs::Environs * env = LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT );
			if ( env )
				env->LoadSettings ( appName, areaName );
			return env;
        }
        

#ifdef USE_ENVIRONS_LOG_POINTERS
        /**
         * Get log methods from Environs object.
         *
         */
        void ENVIRONS_GetLogMethods ( void ** outLog, void ** outLogArg )
        {
            LocateLoadLogMethods ( "Environs", ENVIRONS_BUILD_CRT, outLog, outLogArg );
        }
#endif
        
        bool LocateLoadLib ( COBSTR module, int crt )
        {
            CVerbN ( "LocateLoadLib" );
            
            char absPath [ 1024 ];
            
            HMODULE hModLib = g_EnvironsModuleHandle;
            if ( !hModLib )
            {
                sprintf ( absPath, ENVLIBPREFIX "%s" LIBEXTENSION, module );
                
                hModLib = dlopen ( module, RTLD_LAZY );
                if ( !hModLib )
                {
                    CVerbVerbN ( "LocateLoadLib: Not found in system search path." );
                    
                    do
                    {
                        sprintf ( absPath, "./" ENVLIBPREFIX "%s" LIBEXTENSION, module );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
                        CVerbVerbN ( "LocateLoadLib: Not found in system search path [./]." );
                        
                        sprintf ( absPath, "./" LIBNAME_EXT_DIR "/" ENVLIBPREFIX "%s" LIBEXTENSION, module );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
#ifdef __APPLE__
                        Dl_info info;
                        if ( dladdr ( &g_EnvironsModuleHandle, &info ) ) {
                            if ( info.dli_fname )
                            {
                                int length =  ( int ) strlen ( info.dli_fname );
                                length -= 2;
                                
                                while ( length > 0 ) {
                                    if ( info.dli_fname [ length ] == '/' )
                                        break;
                                    length--;
                                }
                                
                                if ( length > 0 ) {
                                    sprintf ( absPath, "%s", info.dli_fname );
                                    
                                    sprintf ( absPath + length + 1, "../../../" ENVLIBPREFIX "%s" LIBEXTENSION, module );
                                    
                                    CLogArgN ( "LocateLoadLib: [%s]", absPath );
                                    
                                    hModLib = dlopen ( absPath, RTLD_LAZY );
                                    if ( hModLib )
                                        break;
                                }
                            }
                        }
                        
                        CVerbVerbN ( "LocateLoadLib: Not found in root folder of app [./../]" );
                        
                        sprintf ( absPath, "./../../../" ENVLIBPREFIX "%s" LIBEXTENSION, module );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
                        CVerbVerbN ( "LocateLoadLib: Not found in root folder of app [./../../../.]" );
#endif
                        
#ifdef _WIN32
                        
#   ifdef TEST_DIFFERENT_CRT
#   undef ENVIRONS_TSDIR
#   define ENVIRONS_TSDIR TEST_DIFFERENT_CRT
#   endif
                        
                        sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
                        CVerbVerbN ( "LocateLoadLib: Not found in toolset search path [" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/]" );
                        
                        if ( !_getcwd ( absPath, 1024 ) )
                            break;
                        
                        int pos = ( int ) strlen ( absPath );
                        if ( pos <= 0 )
                            break;
                        
                        sprintf ( absPath + pos, "/" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );
                        
                        CVerbN ( "LocateLoadLib: Trying [/" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/]" );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
                        CVerbVerbN ( "LocateLoadLib: Not found in toolset working directory path." );
#else
                        sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVLIBPREFIX "%s" LIBEXTENSION, module );
                        
                        hModLib = dlopen ( absPath, RTLD_LAZY );
                        if ( hModLib )
                            break;
#endif
                        if ( !hModLib ) {
#ifdef _WIN32
                            CLogN ( "LocateLoadLib: Not found." );
#else
                            CLogArgN ( "LocateLoadLib: [%s]", dlerror () );
#endif
                        }
                    }
                    while ( 0 );
                }
            }
            
            if ( !hModLib ) {
                CErrN ( "LocateLoadLib: Cannot find Environs library." );
                
                ShowLibMissingDialog ( module );
                return false;
            }
            
            g_EnvironsModuleHandle = hModLib;
            return true;
        }


		environs::Environs * LocateLoadEnvirons ( COBSTR module, int crt )
		{
			CVerbN ( "LocateLoadEnvirons" );

            if ( !LocateLoadLib ( module , crt ) || !g_EnvironsModuleHandle )
            {
                return 0;
            }

			ENVIRONS_PP ( ENVIRONS_CreateInstance1 ) pCreate = 0;

			pCreate = ( ENVIRONS_PP ( ENVIRONS_CreateInstance1 ) )
				dlsym ( g_EnvironsModuleHandle, ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) );

			if ( pCreate ) {
				CVerbN ( "LocateLoadEnvirons: Creating Instance." );

				return ( environs::Environs * ) pCreate ( ENVIRONS_BUILD_CRT );
			}

			CErrN ( "LocateLoadEnvirons: Cannot find " ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) " in Environs library." );
            return 0;
        }


#ifdef _WIN32
		void CommitPreDispose ( )
		{
			CVerbN ( "CommitPreDispose" );

			if ( !g_EnvironsModuleHandle )
				return;

			pPreDispose PreDispose = 0;

			PreDispose = ( pPreDispose )
				dlsym ( g_EnvironsModuleHandle, "PreDispose" );

			if ( PreDispose ) {
				CVerbN ( "CommitPreDispose: Pre disposing." );

				PreDispose ();
			}
		}
#endif
        

#ifdef USE_ENVIRONS_LOG_POINTERS
        void LocateLoadLogMethods ( COBSTR module, int crt, void ** outLog, void ** outLogArg )
        {
            CVerbN ( "LocateLoadLogMethods" );
            
            if ( !LocateLoadLib ( module , crt ) || !g_EnvironsModuleHandle )
            {
                return;
            }
            
            ENVIRONS_PP ( ENVIRONS_GetLogMethods1 ) pGet = 0;
            
            pGet = ( ENVIRONS_PP ( ENVIRONS_GetLogMethods1 ) )
                dlsym ( g_EnvironsModuleHandle, ENVIRONS_TOSTRING ( ENVIRONS_GetLogMethods1 ) );
            
            if ( pGet ) {
                CVerbN ( "LocateLoadLogMethods: Updating log methods." );
                
                pGet ( outLog, outLogArg );
                return;
            }
            
            CErrN ( "LocateLoadLogMethods: Cannot find " ENVIRONS_TOSTRING ( ENVIRONS_GetLogMethods1 ) " in Environs library." );
        }
#endif
        
        
        void DisposeEnvironsLib ()
        {
            if ( !g_EnvironsModuleHandle )
                return;

#ifdef _WIN32
			CommitPreDispose ();
#endif

            dlclose ( g_EnvironsModuleHandle );
            
            g_EnvironsModuleHandle = 0;
        }
	}
}


#ifdef __OBJC__

#import <Foundation/Foundation.h>


void ShowLibMissingDialog ( const char * libName )
{
    NSString * msg = [[NSString alloc ] initWithFormat:@ENVLIBPREFIX "%s" LIBEXTENSION " is missing!", libName];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:msg];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert runModal];
}

#else

#   ifdef _WIN32

void ShowLibMissingDialog ( const char * libName )
{
	MessageBoxA ( NULL, ENVLIBPREFIX "Environs" LIBEXTENSION " is missing!", NULL, NULL );
}

#   else

void ShowLibMissingDialog ( const char * libName )
{
}

#   endif

#endif

























