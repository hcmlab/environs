/**
* Dynamic usage of Direct3D, Implementation file for dynamically accessing DirectX3D
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
#include "Environs.Native.h"

#ifdef _WIN32

#include "Interop.h"
#include "Dyn.Direct3D.11.h"
using namespace environs;

// Disable this flag to use library as statically linked library again
//#define USE_DYNAMIC_LIB

#define	LIBNAME1	"d3d11.dll"
#define	LIBNAME2	"dxgi.dll"
#define CLASS_NAME	"dynDirect3D"


namespace environs
{
bool							direct3D_LibInitialized		= false;
HMODULE							hLibD3D						= 0;

pD3D11CreateDevice				dD3D11CreateDevice			= 0;




bool VerifyLibDirect3DAccess ( )
{
	if ( !dD3D11CreateDevice 
		//|| !dx264_encoder_headers || !dx264_encoder_encode || !dx264_picture_alloc
		) {
		CWarn ( "VerifyLibDirect3DAccess: One of the functions could not be loaded!" );
		return false;
	}
	return true;
}


void ReleaseDirect3D ( )
{
	CLog ( "ReleaseDirect3D" );

	direct3D_LibInitialized		= false;

	dD3D11CreateDevice			= 0;

	if ( hLibD3D ) {
		dlclose ( hLibD3D );
		hLibD3D = 0;
	}

}


#ifdef USE_DYNAMIC_LIB




bool InitLibDirect3D ()
{
	CLog ( "InitLibDirect3D" );

	if ( direct3D_LibInitialized ) {
		CLog ( "InitLibDirect3D: already initialized." );
		return true;
	}
	
	HMODULE				hDLL	= 0;
	bool				ret = false;

	hDLL = dlopen ( LIBNAME1, RTLD_LAZY );

	if ( !hDLL ) {
		CWarnArg ( "InitLibx264: Loading of " LIBNAME1 " FAILED with error [0x%.8x]", GetLastError ( ) );
		goto Finish;
	}

	 
	dD3D11CreateDevice			= (pD3D11CreateDevice)			dlsym ( hDLL, "D3D11CreateDevice" );

	if ( !VerifyLibDirect3DAccess ( ) ) {
		goto Finish;
	}

	ret = true;

Finish:
	if ( ret ) {
		hLibD3D = hDLL;
		direct3D_LibInitialized = true;
		CLog ( "InitLibx264: successfully initialized access to " LIBNAME1 "." );
	}
	else {
		ReleaseDirect3D ( );
	}

	return ret;
}


#else
#pragma comment ( lib, "d3d11.lib" )

#if defined(WINDOWS_8)
#pragma comment ( lib, "dxgi.lib" )
#pragma comment ( lib, "dxguid.lib" )
#endif

bool InitLibDirect3D ()
{
	CLog ( "InitLibDirect3D" );

	if ( direct3D_LibInitialized ) {
		CLog ( "InitLibDirect3D: static access already initialized." );
		return true;
	}

	dD3D11CreateDevice			= (pD3D11CreateDevice)			D3D11CreateDevice;



	if ( !VerifyLibDirect3DAccess ( ) ) {
		goto Failed;
	}

	direct3D_LibInitialized = true;

	CLog ( "InitLibDirect3D: successfully initialized static access to " LIBNAME1 "." );
	return true;

Failed:
	ReleaseDirect3D ();
	return false;
}
#endif



} // -> namespace environs

#endif

