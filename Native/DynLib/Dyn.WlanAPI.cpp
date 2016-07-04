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
#include "Environs.native.h"
#include "Interop.h"
#include "Dyn.WlanAPI.h"
using namespace environs;

#define	LIBNAME1	"wlanapi.dll"
#define CLASS_NAME	"DynWlanAPI"


namespace environs
{
	bool							wlanAPI_LibInitialized			= false;
	HMODULE							hLibWlanAPI						= 0;

	pWlanOpenHandle					dWlanOpenHandle					= 0;
	pWlanCloseHandle				dWlanCloseHandle				= 0;
	pWlanEnumInterfaces				dWlanEnumInterfaces				= 0;
	pWlanRegisterNotification		dWlanRegisterNotification		= 0;
	pWlanScan						dWlanScan						= 0;
	pWlanGetNetworkBssList			dWlanGetNetworkBssList			= 0;
	pWlanGetAvailableNetworkList	dWlanGetAvailableNetworkList	= 0;
	pWlanFreeMemory					dWlanFreeMemory					= 0;


	bool VerifyLibWlanAPIs ()
	{
		if ( !dWlanOpenHandle || !dWlanCloseHandle || !dWlanEnumInterfaces || !dWlanRegisterNotification
			|| !dWlanScan || !dWlanGetNetworkBssList || !dWlanGetAvailableNetworkList || !dWlanFreeMemory
			) {
			CWarn ( "VerifyLibWlanAPIs: One of the functions could not be loaded!" );
			return false;
		}
		return true;
	}


void ReleaseWlanAPI( )
{
	CLog ( "ReleaseDirect3D" );

	wlanAPI_LibInitialized = false;

	dWlanOpenHandle				= 0;
	dWlanCloseHandle			= 0;
	dWlanEnumInterfaces			= 0;
	dWlanRegisterNotification	= 0;
	dWlanScan					= 0;
	dWlanGetNetworkBssList		= 0;
	dWlanGetAvailableNetworkList= 0;
	dWlanFreeMemory				= 0;

	if ( hLibWlanAPI ) {
		dlclose ( hLibWlanAPI );
		hLibWlanAPI = 0;
	}
}


#ifdef USE_DYNAMIC_LIB_WLAN_API

bool InitLibWlanAPI()
{
	CLog ( "InitLibWlanAPI" );

	if ( wlanAPI_LibInitialized ) {
		CLog ( "InitLibWlanAPI: already initialized." );
		return true;
	}

	HMODULE				hDLL	= 0;
	bool				ret		= false;

	hDLL = dlopen ( LIBNAME1, RTLD_LAZY );

	if ( !hDLL ) {
		CWarnArg ( "InitLibWlanAPI: Loading of " LIBNAME1 " FAILED with error [0x%.8x]", GetLastError () );
		goto Finish;
	}

	dWlanOpenHandle					= ( pWlanOpenHandle )			dlsym ( hDLL, "WlanOpenHandle" );
	dWlanCloseHandle				= ( pWlanCloseHandle )			dlsym ( hDLL, "WlanCloseHandle" );
	dWlanEnumInterfaces				= ( pWlanEnumInterfaces )		dlsym ( hDLL, "WlanEnumInterfaces" );
	dWlanRegisterNotification		= ( pWlanRegisterNotification ) dlsym ( hDLL, "WlanRegisterNotification" );
	dWlanScan						= ( pWlanScan )					dlsym ( hDLL, "WlanScan" );
	dWlanGetNetworkBssList			= ( pWlanGetNetworkBssList )	dlsym ( hDLL, "WlanGetNetworkBssList" );
	dWlanGetAvailableNetworkList	= ( pWlanGetAvailableNetworkList )	dlsym ( hDLL, "WlanGetAvailableNetworkList" );
	dWlanFreeMemory					= ( pWlanFreeMemory )			dlsym ( hDLL, "WlanFreeMemory" );

	if ( !VerifyLibWlanAPIs () ) {
		goto Finish;
	}

	ret = true;

Finish:
	if ( ret ) {
		hLibWlanAPI				= hDLL;
		wlanAPI_LibInitialized	= true;
		CLog ( "InitLibWlanAPI: successfully initialized access to " LIBNAME1 "." );
	}
	else {
		ReleaseWlanAPI ();
	}

	return ret;
}


#else
#pragma comment ( lib, "wlanapi.lib" )

bool InitLibWlanAPI()
{
	CLog ( "InitLibDirect3D" );

	if ( wlanAPI_LibInitialized ) {
		CLog ( "InitLibWlanAPI: static access already initialized." );
		return true;
	}

	dWlanOpenHandle					= ( pWlanOpenHandle )			WlanOpenHandle;
	dWlanCloseHandle				= ( pWlanCloseHandle )			WlanCloseHandle;
	dWlanEnumInterfaces				= ( pWlanEnumInterfaces )		WlanEnumInterfaces;
	dWlanRegisterNotification		= ( pWlanRegisterNotification ) WlanRegisterNotification;
	dWlanScan						= ( pWlanScan )					WlanScan;
	dWlanGetNetworkBssList			= ( pWlanGetNetworkBssList )	WlanGetNetworkBssList;
	dWlanGetAvailableNetworkList	= ( pWlanGetAvailableNetworkList ) WlanGetAvailableNetworkList;
	dWlanFreeMemory					= ( pWlanFreeMemory )			WlanFreeMemory;

	if ( !VerifyLibWlanAPIs () ) {
		goto Failed;
	}

	wlanAPI_LibInitialized = true;

	CLog ( "InitLibWlanAPI: successfully initialized static access to " LIBNAME1 "." );
	return true;

Failed:
	ReleaseWlanAPI ();
	return false;
}
#endif



} // -> namespace environs



