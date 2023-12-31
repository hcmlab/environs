/**
 * Exported functions that are required for environs core interplay 
   with extension plugins.
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
#include "Interop/Export.h"
#include "Log.h"

#ifndef INCLUDE_HCM_ENVIRONS_EXT_INTERFACE_EXPORTS_H
#define	INCLUDE_HCM_ENVIRONS_EXT_INTERFACE_EXPORTS_H

/**
* Exported functions that are required for environs core interplay.
*
*	These entry functions to query interface types and create interface objects must be exported by each module.
*
*/
#ifdef __cplusplus
extern "C"
{
#endif
	
	/**
	* GetINames
	*
	*	@param	size	on success, this argument is filled with the count of names available in the returned array.
	*
	*	@return returns an array of user readable friendly names in ASCII encoding.
	*
	*/
	#define	MODULE_EXPORT_GETINAMES		"GetINames"
	typedef const char ** ( CallConv *pGetINames )( int * size );

	LIBEXPORT const char **	CallConv	GetINames ( int * size );

#define BUILD_INT_GETINAMES(arr)	\
	extern "C" const char ** CallConv GetINames ( int * size ) { \
		if ( size ) *size = sizeof(arr) / sizeof(arr [0]); \
			return (const char **)arr; \
		}

	/**
	* GetITypes
	*
	*	@param	size	on success, this argument is filled with the count of types available in the returned array.
	*
	*	@return returns an array with values of type PluginType::PluginType that describes the type of the plugin names returned by getINames();
	*
	*/
	#define	MODULE_EXPORT_GETITYPES				"GetITypes"
	typedef const unsigned int * ( CallConv *pGetITypes )( int * size );

	LIBEXPORT const	unsigned int * CallConv		GetITypes ( int * size );

#define BUILD_INT_GETITYPES(arr)	\
	extern "C" const unsigned int * CallConv GetITypes ( int * size ) { \
		if ( size ) *size = sizeof(arr) / sizeof(arr [0]); \
			return (unsigned int *)arr; \
		}

	/**
	* CreateInstance
	*
	*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
	*	@param	deviceID	the deviceID that the created interface object should use.
	*
	*	@return An object that supports the requested interface determined through the index argument.
	*
	*/
	#define	MODULE_EXPORT_CREATE	"CreateInstance"
	typedef void * ( CallConv *pCreateInstance )( int index, int deviceID );

#ifndef ENVIRONS_CORE_LIB
	LIBEXPORT void *	CallConv	CreateInstance ( int index, int deviceID );

#define BUILD_INT_CREATEOBJ(objType)	\
	extern "C" void * CallConv CreateInstance ( int index, int deviceID ) { \
		if ( index == 0 ) { \
			objType * obj = new objType ( ); \
			if ( obj ) { \
				obj->deviceID = deviceID; \
				return obj; \
			} \
			else { \
				CErrID ( "CreateInstance: Failed to create an interface object." ); \
			} \
		} \
		return 0; \
    }
#endif
    
    
    /**
     * SetEnvironsObject
     *
     *	Injects environs runtime object.
     *
     */
#define	MODULE_EXPORT_ENVIRONSOBJ	"SetEnvironsObject"
    typedef void ( CallConv *pSetEnvironsObject )( void * envObj, void * natObj );
    
	LIBEXPORT void 	CallConv	SetEnvironsObject ( void * envObj, void * natObj );


#define BUILD_INT_SETENVIRONSOBJECT()                  \
namespace environs {                                    \
    void	*   pEnvirons			= 0;				\
	void	*	pNative				= 0;				\
}                                                       \
    extern "C" void CallConv SetEnvironsObject ( void * envObj, void * natObj ) { \
        environs::pEnvirons             = envObj;	\
        environs::pNative				= natObj;	\
    }                                               \
	void * GetEnvironsNative () { return environs::pNative; }	\
	void * GetEnvironsInstance () { return environs::pEnvirons; }

    //CVerbVerb ( "SetEnvironsObject: Set ByteBuffer methods" );		

#ifdef __cplusplus
}
#endif


#endif	/// -> INCLUDE_HCM_ENVIRONS_EXT_INTERFACE_EXPORTS_H








