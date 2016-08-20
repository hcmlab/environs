/**
* External Wifi Observer Thread
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

#ifdef _WIN32

#include "Ext.Wifi.Observer.h"
#include "Environs.Obj.h"
#include "Environs.Build.Lnk.h"
#include <wlanapi.h>
using namespace environs;

#pragma comment ( lib, "wlanapi.lib" )

#define CLASS_NAME	"Wifi.Obs.Thread  . . . ."

static const char		*		WifiObsThread_extensionNames[]	= { "Wifi.Obs.Thread", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	WifiObsThread_interfaceTypes[]	= { InterfaceType::ExtThread };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( WifiObsThread_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( WifiObsThread_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( WifiObsThread );


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
	unsigned int		lastScan	= 0;
	EnvironsNative	*	nativo		= 0;
	WifiObserver	*	wifi		= 0;

	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	WifiObsThread::WifiObsThread ()
	{
		CLog ( "Construct" );

		interfaceType		= WifiObsThread_interfaceTypes [0];
		name				= WifiObsThread_extensionNames [0];

		nativo				= ( EnvironsNative * ) GetEnvironsNative ();

		if ( nativo )
			wifi			= &nativo->wifiObserver;
	}


	WifiObsThread::~WifiObsThread ()
	{
		CLog ( "Destruct" );
	}


	int WifiObsThread::Init ()
	{
		CVerb ( "Init" );

		return true;
	}


	void WifiObserverNotification ( WLAN_NOTIFICATION_DATA * data, VOID * context )
	{
		if ( data->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM ) {
			// wlan_notification_acm_scan_list_refresh == 26 (vs2010 is missing this)
			if ( data->NotificationCode == 26 || data->NotificationCode == wlan_notification_acm_scan_complete )
				lastScan = nativo->Tick32 ();
		}
		/*bool doNotify = false;

		if ( data->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM ) {
		if ( data->NotificationCode == wlan_notification_acm_scan_list_refresh || data->NotificationCode == wlan_notification_acm_scan_complete
		|| data->NotificationCode == wlan_notification_acm_network_available || data->NotificationCode == wlan_notification_acm_network_not_available )
		doNotify = true;
		else
		CLogArg ( "WifiObserverNotification: ACM [ %i ]", data->NotificationCode );
		}

		else if ( data->NotificationSource == WLAN_NOTIFICATION_SOURCE_MSM ) {
		if ( data->NotificationCode == wlan_notification_msm_radio_state_change || data->NotificationCode == wlan_notification_msm_signal_quality_change )
		doNotify = true;
		else
		CLogArg ( "WifiObserverNotification: MSM [ %i ]", data->NotificationCode );
		}

		if ( doNotify )*/
		nativo->Notify ( &wifi->thread, "WifiObserverNotification" );
	}


	unsigned char GetWiFiChannel ( unsigned long centerFreq )
	{
		unsigned long f1 = ( centerFreq % 2412000 ) / 1000;
		return ( unsigned char ) ( ( f1 / 5 ) + 1 );
	}


	bool WifiObsThread::ThreadFunc ( void * obj )
	{
		CLog ( "WifiObserver: Created ..." );

		if ( !nativo )
			return false;

		HANDLE	hClient			= NULL;
		DWORD	maxClients		= 2;
		DWORD	curVersion		= 0;
		DWORD	result			= 0;

		char		 ssidName [ 256 ];
		bool		 doScan		= true;
		unsigned int lastCheck	= 0;
		unsigned int i;

		WLAN_INTERFACE_INFO_LIST	* interfaceList = 0;
		WLAN_INTERFACE_INFO			* interfaceInfo = 0;

		result = WlanOpenHandle ( maxClients, NULL, &curVersion, &hClient );
		if ( result != ERROR_SUCCESS ) {
			CErrArg ( "WifiObserver: OpenHandle failed [ %u ]", result );
			return 0;
		}

		result = WlanEnumInterfaces ( hClient, NULL, &interfaceList );
		if ( result != ERROR_SUCCESS ) {
			CErrArg ( "WifiObserver: EnumInterfaces failed [ %u ]", result );
			return 0;
		}
		//  | WLAN_NOTIFICATION_SOURCE_ONEX
		result = WlanRegisterNotification ( hClient, WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM, TRUE, ( WLAN_NOTIFICATION_CALLBACK ) WifiObserverNotification, 0, 0, 0 );
		if ( result != ERROR_SUCCESS ) {
			CErrArg ( "WifiObserver: RegisterNotification failed [ %u ]", result );
		}

		CLogArg ( "WifiObserver: Interfaces [ %u ]", interfaceList->dwNumberOfItems );

		while ( wifi->threadRun )
		{
			for ( i = 0; i < ( int ) interfaceList->dwNumberOfItems; i++ )
			{
				interfaceInfo = ( WLAN_INTERFACE_INFO * ) & interfaceList->InterfaceInfo [ i ];

				CLogArg ( "WifiObserver [ %u ]: Desc  [ %ws ] - State [ %d ]", i, interfaceInfo->strInterfaceDescription, interfaceInfo->isState );

				if ( doScan )
				{
					doScan	 = false;
					lastScan = nativo->Tick32 ();

					result = WlanScan ( hClient, &interfaceInfo->InterfaceGuid, 0, 0, 0 );

					if ( result != ERROR_SUCCESS ) {
						CLogArg ( "WifiObserver [ %u ]: Scan failed [ %u ]", i, result );
					}
				}

				// Get available Networks
				/*WLAN_AVAILABLE_NETWORK_LIST * networkList = 0;

				if ( WlanGetAvailableNetworkList ( hClient, &interfaceInfo->InterfaceGuid, 0x00000002|0x00000001, 0, &networkList ) == ERROR_SUCCESS )
				{
				for ( DWORD nw = 0; nw < networkList->dwNumberOfItems; nw++ )
				{
				char szName [ 256 ];
				int len = networkList->Network[nw].dot11Ssid.uSSIDLength;

				memcpy ( szName, networkList->Network[nw].dot11Ssid.ucSSID, len );
				szName [ len ] = 0;
				CLogArg ( "WifiObserver:   Network %s (%ld)", szName, networkList->Network[nw].wlanSignalQuality );
				}
				}

				if ( networkList )
				WlanFreeMemory ( networkList );*/

				// Get available BSSIDs
				WLAN_BSS_LIST * bssList = 0;

				if ( WlanGetNetworkBssList ( hClient, &interfaceInfo->InterfaceGuid, NULL, dot11_BSS_type_any, 0, NULL, &bssList ) == ERROR_SUCCESS )
				{
					if ( bssList->dwNumberOfItems > 0 )
					{
						wifi->Begin ();

						for ( DWORD k = 0; k < bssList->dwNumberOfItems; k++ )
						{
							PWLAN_BSS_ENTRY pEntry = bssList->wlanBssEntries + k;

							int len = pEntry->dot11Ssid.uSSIDLength;
							*ssidName = 0;

							if ( ( len + 1 ) < sizeof ( ssidName ) ) {
								memcpy ( ssidName, pEntry->dot11Ssid.ucSSID, len );
								ssidName [ len ] = 0;
							}

							unsigned long long bssid = 0;

							for ( int j = 0; j < 6; ++j )
							{
								bssid <<= 8;
								bssid |= pEntry->dot11Bssid [ j ];
							}

							unsigned char channel = GetWiFiChannel ( pEntry->ulChCenterFrequency );

							CLogArg ( "WifiObserver: MAC [ %12llX ]\t: C%i\t: S%i\t: %i dBm\t: Network %s", bssid, ( int ) channel, pEntry->uLinkQuality, pEntry->lRssi, ssidName );

							wifi->UpdateWithMac ( bssid, ssidName, pEntry->lRssi, pEntry->uLinkQuality, channel, 0 );
						}

						wifi->Finish ();
					}
				}

				if ( bssList )
					WlanFreeMemory ( bssList );
			}

			lastCheck = nativo->Tick32 ();

			unsigned int waitTime = nativo->useWifiInterval;

		WaitLoop:
			if ( wifi->threadRun ) {
				nativo->WaitOne ( &wifi->thread, "WifiObserver", waitTime );

				unsigned int now = nativo->Tick32 ();
				unsigned int diff = now - lastCheck;

				if ( diff < ENVIRONS_WIFI_OBSERVER_INTERVAL_CHECK_MIN ) {
					waitTime = ( ENVIRONS_WIFI_OBSERVER_INTERVAL_CHECK_MIN + 30 ) - diff;
					goto WaitLoop;
				}

				//interfaceInfo->isState == wlan_interface_state_connected
				if ( ( now - lastScan ) > ( unsigned ) nativo->useWifiInterval )
					doScan = true;
			}
		}

		WlanRegisterNotification ( hClient, WLAN_NOTIFICATION_SOURCE_NONE, TRUE, 0, 0, 0, 0 );

		if ( interfaceList )
			WlanFreeMemory ( interfaceList );

		if ( hClient )
			WlanCloseHandle ( hClient, 0 );

		CLog ( "WifiObserver: bye bye ..." );

		return true;
	}
	

} /* namespace environs */


#endif

