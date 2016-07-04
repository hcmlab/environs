/**
* Environs Loader Exports
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
#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_LOADER_EXPORT_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_LOADER_EXPORT_H

#include "Interop/Export.h"
#include "Interop/Smart.Pointer.h"
#include "Interfaces/IEnvirons.Dispose.h"


/* DLL exports of Environs to create instance objects */
#define	ENVIRONS_CreateInstance			CreateInstance
#define	ENVIRONS_CreateInstance1		CreateInstance1
#define	ENVIRONS_CreateInstanceObject	CreateInstanceObject
#define	ENVIRONS_CreateInstanceObject1	CreateInstanceObject1

#define	ENVIRONS_GetLogMethods          GetLogMethods
#define	ENVIRONS_GetLogMethods1         GetLogMethods1

#define ENVIRONS_PP(a)				pp##a

namespace environs
{
	class Environs;


	namespace Loader
	{
		/**
		* Create an Environs object.
		*
		* @return   An Environs object wrapped into a smart pointer.
		*/
		sp ( environs::Environs )	ENVIRONS_CreateInstance ();


		/**
		* Create an Environs object.
		*
		* @return   An Environs object interface.
		*/
		environs::Environs *        ENVIRONS_CreateInstanceObject ();


		/**
		* Create an Environs object.
		*
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object wrapped into a smart pointer.
		*/
		sp ( environs::Environs )	ENVIRONS_CreateInstance ( const char * appName, const char * areaName );
        
        /**
         * Create an Environs object.
         *
         * @param 	appName		The application name for the application environment.
         * @param  	areaName	The area name for the application environment.
         *
         * @return   An Environs object interface.
         */
        ENVIRONSAPI ENVIRONS_LIB_API void * CallConv	ENVIRONS_CreateInstance1 ( int crt );
        
        void                        DisposeEnvironsLib ();
        
        
        /**
         * Get log methods from Environs object.
         *
         */
        void                        ENVIRONS_GetLogMethods ( void ** outLog, void ** outLogArg );
	}

	typedef void *		( *ENVIRONS_PP ( ENVIRONS_CreateInstance1 ) )( int crt );
    
    
    typedef void *		( *ENVIRONS_PP ( ENVIRONS_GetLogMethods1 ) )( void ** outLog, void ** outLogArg );

	typedef void 		(*pPreDispose)();
	/**
	* Create an Environs object.
	*
	* C++ macro to create an instance of Environs using the static linked library.
	* Note: You MUST link to Environs.lib in the libs/vXXX folders according to
	*		the platform toolset that your binary is build with.
	*
	* Alternatively, you may dynamically load the library using environs:lib:Loader:CreateInstance ()
	*
	* @return   An Environs object wrapped into a smart pointer.
	*/
#define Environs_CreateInstanceStaticLinked()	sp ( environs::Environs ) ( (environs::Environs *) ENVIRONS_CreateInstance1 ( ENVIRONS_BUILD_CRT ), ::environs::EnvironsDisposer )


	/// C++ function implemented in the loader (statically build into each app)
	//sp ( IEnvirons )								ENVIRONS_CreateInstance ();


	/// C function implemented in the loader (statically build into each app)
	//  Note: The returned instance must be destroyed using "delete" in order to
	//        clean up the allocated resources
	//void * CallConv									ENVIRONS_CreateInstanceObject ();


	/// C function exported by the library
	ENVIRONSAPI ENVIRONS_LIB_API void * CallConv	ENVIRONS_CreateInstance1 ( int crt );
    
    
}



#endif



