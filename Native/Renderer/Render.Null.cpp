/**
* Null renderer
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


#include "Render.Null.h"
#include "Environs.Obj.h"
#include "Environs.Build.Lnk.h"
using namespace environs;

#define CLASS_NAME	"Render.Null. . . . . . ."

static const char		*		RenderNull_extensionNames[]	= { "Null Renderer", "End" };

static const InterfaceType_t	RenderNull_interfaceTypes[]	= { InterfaceType::Unknown, InterfaceType::Unknown };

#ifndef ENVIRONS_CORE_LIB


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( RenderNull_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( RenderNull_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( RenderNull );


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
	PortalBufferType_t	RenderNull_inputTypeSupport[] = { PortalBufferType::Unknown };
	PortalBufferType_t	RenderNull_outputTypeSupport[] = { PortalBufferType::Unknown };


	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	RenderNull::RenderNull ()
	{
		CLog ( "Construct" );

		interfaceType		= RenderNull_interfaceTypes [0];
		name				= RenderNull_extensionNames [0];

		inputTypes			= RenderNull_inputTypeSupport;
		inputTypesLength	= sizeof ( RenderNull_inputTypeSupport ) / sizeof ( RenderNull_inputTypeSupport [0] );

		outputTypes			= RenderNull_outputTypeSupport;
		outputTypesLength	= sizeof ( RenderNull_outputTypeSupport ) / sizeof ( RenderNull_outputTypeSupport [0] );
	}


	RenderNull::~RenderNull ()
	{
		CLog ( "Destruct" );
	}


	bool RenderNull::Init ()
	{
		CVerb ( "Init" );

		return true;
	}


	void RenderNull::Dispose ()
	{
		CVerb ( "Dispose" );
	}


	char * RenderNull::GetBuffer ( RenderContext * context )
	{
		return 0;
	}


	int RenderNull::ReleaseResources ( RenderContext * context )
	{
		return 1;
	}


	int RenderNull::AllocateResources ( RenderDimensions * dims )
	{
		return 1;
	}


	int RenderNull::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
	{
		return 1;
	}


	bool RenderNull::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		return 1;
	}


	bool RenderNull::Compare ( unsigned int &equal )
	{
		return false;
	}


} /* namespace environs */



