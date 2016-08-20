/**
* GDI portal implementation
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

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif


#include "Base.Null.h"
#include "Environs.Obj.h"
#include "Environs.Build.Lnk.h"
using namespace environs;

#define CLASS_NAME	"Base.Null. . . . . . . ."

static const char		*		BaseNULL_extensionNames[]	= { "NULL Base", "End" };

static const InterfaceType_t	BaseNULL_interfaceTypes[]	= { InterfaceType::Unknown, InterfaceType::Unknown };

#ifndef ENVIRONS_CORE_LIB

/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( BaseNULL_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( BaseNULL_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( BaseNULL );


/**
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();


#ifdef _WIN32
BOOL APIENTRY DllMain ( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif


#endif


namespace environs 
{
	//	
	// Initialization of static values
	PortalBufferType_t	BaseNULL_inputTypeSupport[] = { PortalBufferType::Unknown };
	PortalBufferType_t	BaseNULL_outputTypeSupport[] = { PortalBufferType::Unknown };


	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	BaseNULL::BaseNULL ()
	{
		CLog ( "Construct" );

		interfaceType		= BaseNULL_interfaceTypes [0];
		name				= BaseNULL_extensionNames [0];

		inputTypes			= BaseNULL_inputTypeSupport;
		inputTypesLength	= sizeof ( BaseNULL_inputTypeSupport ) / sizeof ( BaseNULL_inputTypeSupport [0] );

		outputTypes			= BaseNULL_outputTypeSupport;
		outputTypesLength	= sizeof ( BaseNULL_outputTypeSupport ) / sizeof ( BaseNULL_outputTypeSupport [0] );
	}


	BaseNULL::~BaseNULL ()
	{
		CLog ( "Destruct" );
	}


	bool BaseNULL::Init ()
	{
		CVerb ( "Init" );

		return true;
	}


	void BaseNULL::Dispose ()
	{
		CVerb ( "Dispose" );
	}


	char * BaseNULL::GetBuffer ( RenderContext * context )
	{
		return 0;
	}


	int BaseNULL::ReleaseResources ( RenderContext * context )
	{
		return 1;
	}


	int BaseNULL::AllocateResources ( RenderDimensions * dims )
	{
		return 1;
	}


	int BaseNULL::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
	{
		return 1;
	}


	bool BaseNULL::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		return 1;
	}


	bool BaseNULL::Compare ( unsigned int &equal )
	{
		return false;
	}


} /* namespace environs */



