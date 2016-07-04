/**
* Dynamically accessing Wlan API
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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_WLAN_API_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_WLAN_API_H

#ifdef _WIN32

#include <wlanapi.h>

// Disable this flag to use library as statically linked library again
#define USE_DYNAMIC_LIB_WLAN_API

namespace environs
{
#ifdef VS2010
#	define _Reserved_
#	define _Outptr_
#endif
	typedef DWORD ( WINAPI * pWlanOpenHandle ) ( _In_ DWORD version, _Reserved_ PVOID res,
		_Out_ PDWORD outVersion, _Out_ PHANDLE client );

	typedef DWORD ( WINAPI * pWlanCloseHandle )( _In_ HANDLE client, _Reserved_ PVOID res );

	typedef DWORD ( WINAPI * pWlanEnumInterfaces )( _In_ HANDLE client, _Reserved_ PVOID res,
		_Outptr_ PWLAN_INTERFACE_INFO_LIST *intfList );
	
	typedef DWORD ( WINAPI * pWlanRegisterNotification )( _In_ HANDLE client, _In_ DWORD source,
		_In_ BOOL ignoreDups, _In_opt_ WLAN_NOTIFICATION_CALLBACK pCallback, _In_opt_ PVOID ctx,
		_Reserved_ PVOID res, _Out_opt_ PDWORD prevSource );

	typedef DWORD ( WINAPI * pWlanScan )( _In_ HANDLE client, _In_ CONST GUID *guid,
		_In_opt_ CONST PDOT11_SSID ssid, _In_opt_ CONST PWLAN_RAW_DATA dataIE, _Reserved_ PVOID res );

	typedef DWORD ( WINAPI * pWlanGetNetworkBssList )( _In_ HANDLE client, _In_ CONST GUID *guid,
		_In_opt_ CONST PDOT11_SSID ssid, _In_ DOT11_BSS_TYPE type, _In_ BOOL security,
		_Reserved_ PVOID res, _Outptr_ PWLAN_BSS_LIST *bssList );

	typedef DWORD ( WINAPI * pWlanGetAvailableNetworkList )( _In_ HANDLE client, _In_ CONST GUID *guid,
			_In_ DWORD flags, _Reserved_ PVOID res, _Outptr_ PWLAN_AVAILABLE_NETWORK_LIST *networkList );

	typedef VOID ( WINAPI * pWlanFreeMemory )( _In_ PVOID pMemory );

extern void ReleaseWlanAPI( );
extern bool InitLibWlanAPI ( );

extern bool							wlanAPI_LibInitialized;

#ifdef USE_DYNAMIC_LIB_WLAN_API

extern pWlanOpenHandle				dWlanOpenHandle;
extern pWlanCloseHandle				dWlanCloseHandle;
extern pWlanEnumInterfaces			dWlanEnumInterfaces;
extern pWlanRegisterNotification	dWlanRegisterNotification;
extern pWlanScan					dWlanScan;
extern pWlanGetNetworkBssList		dWlanGetNetworkBssList;
extern pWlanGetAvailableNetworkList	dWlanGetAvailableNetworkList;
extern pWlanFreeMemory				dWlanFreeMemory;

#else

#	define dWlanOpenHandle(...)					WlanOpenHandle ( __VA_ARGS__ )
#	define dWlanCloseHandle(...)				WlanCloseHandle ( __VA_ARGS__ )
#	define dWlanEnumInterfaces(...)				WlanEnumInterfaces ( __VA_ARGS__ )
#	define dWlanRegisterNotification(...)		WlanRegisterNotification ( __VA_ARGS__ )
#	define dWlanScan(...)						WlanScan ( __VA_ARGS__ )
#	define dWlanGetNetworkBssList(...)			WlanGetNetworkBssList ( __VA_ARGS__ )
#	define dWlanGetAvailableNetworkList(...)	WlanGetAvailableNetworkList ( __VA_ARGS__ )
#	define dWlanFreeMemory(...)					WlanFreeMemory ( __VA_ARGS__ )

#endif

} // -> namespace environs

#else

#	define ReleaseWlanAPI()
#	define InitLibWlanAPI()	true

#endif

#endif // INCLUDE_HCM_ENVIRONS_DYNAMIC_WLAN_API_H