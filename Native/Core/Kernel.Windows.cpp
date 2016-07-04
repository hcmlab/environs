/**
* Environs Windows Kernel
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

#if (defined(_WIN32))

#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Interfaces/IExt.Thread.h"

#include "Kernel.Windows.h"
#include "Device/Device.Controller.h"
#include "Callbacks.h"
#include "Renderer/Render.OpenCL.h"
#include "DynLib/Dyn.OpenCL.h"
#include "Environs.Utils.h"

#include <string>
#include <map>
using namespace std;


#include "Capture/Capture.Win.D3D.h"

//#include "WinNetEventSink.h"

// Link to Gdiplus library
#pragma comment ( lib, "Gdiplus.lib" )

// Link to winsock library
#pragma comment ( lib, "Ws2_32.lib" )

#ifndef WINDOWS_PHONE
#   define USE_GDIPLUS

#	include <iphlpapi.h>
#pragma comment ( lib, "iphlpapi.lib" )

#   include <wininet.h>
#   pragma comment ( lib, "wininet.lib" )

#else
#endif

#define USE_NETWORK_STATUS_UPDATES

#ifdef NATIVE_WIFI_OBSERVER	
#	include "DynLib/Dyn.WlanAPI.h"

//#   include <wlanapi.h>
//#   pragma comment ( lib, "wlanapi.lib" )

#   include <Windot11.h>
#   include <objbase.h>
#   include <wtypes.h>

#   pragma comment ( lib, "ole32.lib" )
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Kernel.Windows . . . . ."


namespace environs
{
	//
	// Static class members	
	bool		KernelPlatform::winSockStarted = false;

	ULONG_PTR	KernelPlatform::gdiplusToken	= 0;

	HANDLE		addrChangeHandle		= 0;
	HANDLE		interfaceChangeHandle	= 0;
	HANDLE		routeChangeHandle		= 0;

	//
	// Externals
	extern const char * pref_dataStoreDefault;
	extern bool			InitStorageUtil ( const char * storagePath );


	namespace API
	{
		LONGSYNC	updateNetworkStatusLock	= 0;

#ifndef WINDOWS_PHONE
		void UpdateNetworkStatus ()
		{
			CVerb ( "UpdateNetworkStatus" );

			/// Determine network connect status
			int netStat = NETWORK_CONNECTION_NO_NETWORK;

			DWORD status = 0, res = 0;
			if ( InternetGetConnectedState ( &status, res ) ) {
				if ( status & INTERNET_CONNECTION_MODEM )
					netStat = NETWORK_CONNECTION_MOBILE_DATA;
				else if ( status & INTERNET_CONNECTION_LAN )
					netStat = NETWORK_CONNECTION_LAN;
				else if ( status & INTERNET_CONNECTION_OFFLINE )
					netStat = NETWORK_CONNECTION_NO_INTERNET;
				else
					netStat = NETWORK_CONNECTION_MOBILE_DATA; // By default we assume 
			}

			if ( netStat != native.networkStatus )
				SetNetworkStatusN ( netStat );
		}


		void * UpdateNetworkStatusInvoker ( void * arg )
		{
			CLog ( "UpdateNetworkStatusInvoker: thread started..." );

			int oldStatus = native.networkStatus;
			/// Wait some ms for the remaining "notification fire" abate (and a likely network connection to get available)
			Sleep ( 300 );

			/// Let's try it 6 times and wait 200ms inbetween
			unsigned int tries = 80;

			while ( tries > 0 ) {
				UpdateNetworkStatus ();
				if ( !( tries % 4 ) ) {
					CVerbArg ( "UpdateNetworkStatusInvoker: old status [%i], new status [%i], tries left [%u]", oldStatus, native.networkStatus, tries ); true;
				}

				if ( oldStatus < NETWORK_CONNECTION_NO_INTERNET ) {
					if ( native.networkStatus >= NETWORK_CONNECTION_NO_INTERNET )
						break;
				}
				else if ( native.networkStatus != oldStatus )
					break;

				Sleep ( 500 );
				tries--;
			}
			CVerbArg ( "UpdateNetworkStatusInvoker: old status [%i], new status [%i]", oldStatus, native.networkStatus );

			long val = ___sync_val_compare_and_swap ( &updateNetworkStatusLock, 1, 0 );
			if ( val != 1 ) {
				CWarnArg ( "UpdateNetworkStatusInvoker: Something went wrong with modifying the updater access lock [%u]", ( unsigned int ) val );
			}
			return 0;
		}


		void InvokeUpdateNetworkStatus ()
		{
			CVerb ( "InvokeUpdateNetworkStatus" );

			if ( ___sync_val_compare_and_swap ( &native.environsKernelAccess, 1, 1 ) < 1 )
				return;
			/*if ( native.environsState < environs::Status::Started ) {
			CLog ( "InvokeUpdateNetworkStatus: No need to update network status. Environs is about to shut down." );
			return;
			}*/

			// Check whether Environs is initialized and no other cleaner is currently active
			if ( ___sync_val_compare_and_swap ( &updateNetworkStatusLock, 0, 1 ) != 0 ) {
				CVerb ( "InvokeUpdateNetworkStatus: An updater seems to be running already.." );
				return;
			}

			pthread_t threadID;
			int ret = pthread_create ( &threadID, NULL, &UpdateNetworkStatusInvoker, 0 );
			if ( ret != 0 ) {
				CErr ( "InvokeUpdateNetworkStatus: Failed to create updater thread!" );
				return;
			}
			pthread_detach_handle ( threadID );
		}


		void __stdcall NetAddressChanged ( void * CallerContext, PMIB_UNICASTIPADDRESS_ROW Address, MIB_NOTIFICATION_TYPE NotificationType )
		{
			CVerbArg ( "NetAddressChanged: Notification type is %d.", NotificationType );

			/*if ( Address != NULL ) {
			CLogArg ( "NetAddressChanged: Interface Index %d.", Address->InterfaceIndex );
			CLog ( "NetAddressChanged: Address:  " );
			CLogArg (
			"NetAddressChanged: %d.%d.%d.%d",
			Address->Address.Ipv4.sin_addr.s_net,
			Address->Address.Ipv4.sin_addr.s_host,
			Address->Address.Ipv4.sin_addr.s_lh,
			Address->Address.Ipv4.sin_addr.s_impno );
			}*/

			if ( NotificationType == MibAddInstance || NotificationType == MibDeleteInstance )
				InvokeUpdateNetworkStatus ();
		}


		void __stdcall NetRouteChanged ( void * CallerContext, PMIB_IPFORWARD_ROW2 Route, MIB_NOTIFICATION_TYPE NotificationType )
		{
			CVerbArg ( "NetRouteChanged: Notification type is %d.", NotificationType );

			/*if ( Route != NULL ) {
			CLogArg ( "NetRouteChanged: Interface Index %d.", Route->InterfaceIndex );
			}*/

			if ( NotificationType == MibAddInstance || NotificationType == MibDeleteInstance )
				InvokeUpdateNetworkStatus ();
		}


		void __stdcall NetInterfaceChanged ( void * CallerContext, PMIB_IPINTERFACE_ROW InterfaceRow, MIB_NOTIFICATION_TYPE NotificationType )
		{
			CVerbArg ( "NetInterfaceChanged: Notification type is %d.", NotificationType );

			/*if ( InterfaceRow != NULL ) {
			CLogArg ( "NetInterfaceChanged: Interface Index: %d.", InterfaceRow->InterfaceIndex );
			}*/

			if ( NotificationType == MibAddInstance || NotificationType == MibDeleteInstance )
				InvokeUpdateNetworkStatus ();
		}
#endif
	}


	KernelPlatform::KernelPlatform ()
	{
		CLog ( "Construct..." );

		env					= 0;
		
#ifdef USE_GDIPLUS
		if ( !gdiplusToken ) {
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			Gdiplus::GdiplusStartup ( &gdiplusToken, &gdiplusStartupInput, NULL );
		}
#endif
	}


	KernelPlatform::~KernelPlatform ()
	{
		CLog ( "Destruct..." );

#ifndef WINDOWS_PHONE
		if ( addrChangeHandle ) {
			//CancelMibChangeNotify2 ( addrChangeHandle );
			addrChangeHandle = 0;
		}

		if ( interfaceChangeHandle ) {
			//CancelMibChangeNotify2 ( interfaceChangeHandle );
			interfaceChangeHandle = 0;
		}

		if ( routeChangeHandle ) {
			//CancelMibChangeNotify2 ( routeChangeHandle );
			routeChangeHandle = 0;
		}
#endif

		env->callbacks.Clear ();

		CVerb ( "Destruct done." );
	}


	bool KernelPlatform::StartWinSock ()
	{
		if ( !winSockStarted ) {
			CVerb ( "StartWinSock" );

			WSADATA w;
			Zero ( w );

			int error = WSAStartup ( MAKEWORD ( 2, 2 ), &w );
			if ( error )
			{
				CErrArg ( "StartWinSock: Failed to initialize WinSock API [%i]", error );
				return false;
			}

			winSockStarted = true;
			CVerb ( "StartWinSock: WinSock API initialized." );
		}

		return true;
	}


	bool KernelPlatform::DisposeWinSock ()
	{
		if ( winSockStarted ) {
			CVerb ( "DisposeWinSock" );
            
            if ( native.udpSignalSender != -1 ) {
				//CSocketTraceRemove ( native.udpSignalSender, "DisposeWinSock: Closing native.udpSignalSender.", 0 );
                closesocket ( native.udpSignalSender );
                native.udpSignalSender = -1;
            }
            
			int error = WSACleanup ();
			if ( error )
			{
				CErrArg ( "DisposeWinSock: Failed to cleanup WinSock API [%i]", error );
				return false;
			}

			winSockStarted = false;
			CVerb ( "DisposeWinSock: WinSock API cleaned up." );
		}
		return true;
	}

	
	void KernelPlatform::ReleaseLibrary ()
	{
		CVerb ( "ReleaseLibrary" );

		if ( winSockStarted ) {
			CVerb ( "ReleaseLibrary: Cleanup WinSock ..." );
			WSACleanup ();
			winSockStarted = false;
		}

#ifdef USE_GDIPLUS
		if ( gdiplusToken ) {
			CVerb ( "ReleaseLibrary: Closing GdiPlus ..." );
			Gdiplus::GdiplusShutdown ( gdiplusToken );
			gdiplusToken = NULL;
		}
#endif
	}


	int KernelPlatform::onPreInit ()
	{
		CVerb ( "onPreInit" );

		if ( !StartWinSock () )
			return false;

#if (!defined(WINDOWS_PHONE	) && defined(USE_NETWORK_STATUS_UPDATES))
		DWORD status;

		if ( !addrChangeHandle ) {
			status =
				NotifyUnicastIpAddressChange (
					AF_UNSPEC,
					&API::NetAddressChanged,
					this,
					TRUE,
					&addrChangeHandle );

			if ( status != NO_ERROR ) {
				CWarnArg ( "onPreInit: Register address change failed. Error %d.", status );
			}
			else {
				CVerb ( "onPreInit: Register address change succeeded." );
			}
		}

		/*if ( !interfaceChangeHandle ) {
		status = NotifyIpInterfaceChange (
		AF_UNSPEC,
		&API::NetInterfaceChanged,
		NULL,
		FALSE,
		&interfaceChangeHandle );

		if ( status != NO_ERROR ) {
		CLogArg ( "onPreInit: Register interface change failed. Error %d.", status );
		}
		else {
		CLog ( "onPreInit: Register interface change succeeded." );
		}
		}*/

		if ( !routeChangeHandle ) {
			status = NotifyRouteChange2 (
				AF_UNSPEC,
				&API::NetRouteChanged,
				this,
				TRUE,
				&routeChangeHandle );

			if ( status != NO_ERROR ) {
				CWarnArg ( "onPreInit: Register route change failed. Error %d.", status );
			}
			else {
				CVerb ( "onPreInit: Register route change succeeded." );
			}
		}
#endif
		return true;
	}


	int KernelPlatform::onPreStart ()
	{
		CVerb ( "onPreStart" );

		if ( env->environsState == environs::Status::Uninitialized  && !StartWinSock () ) {
			CErr ( "onPreStart: Failed to start WinSock" );
			return false;
		}

		return true;
	}


	int KernelPlatform::onStarted ()
	{
		CVerb ( "onStarted" );

		return true;
	}


	int KernelPlatform::onPreStop ()
	{
		CVerb ( "onPreStop" );

		return true;
	}


	int KernelPlatform::onStopped ()
	{
		CVerb ( "onStopped" );

#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
		if ( CaptureWinD3D::d3dDevice ) {
			CaptureWinD3D::DisposeD3D ();
		}
#endif

		return true;
	}


	void KernelPlatform::UpdateAppWindowSize ()
	{
		CVerb ( "UpdateAppWindowSize" );

#ifndef WINDOWS_PHONE
		if ( env->appWindowHandle ) {
			WINDOWINFO info;
			Zero ( info );

			if ( GetWindowInfo ( env->appWindowHandle, &info ) ) {
				env->appWindowWidth = info.rcWindow.right - info.rcWindow.left;
				env->appWindowHeight = info.rcWindow.bottom - info.rcWindow.top;
			}
		}
		else {
			env->appWindowWidth = GetSystemMetrics ( SM_CXSCREEN );
			env->appWindowHeight = GetSystemMetrics ( SM_CYSCREEN );
		}
#endif
	}


	// return values:
	// -1 means failed to set main application window handle, because of active portals at the moment
	// 0 means failed due to unknow reason
	// 1 means sucessfully set window handle
	int KernelPlatform::SetMainAppWindow ( WNDHANDLE appWnd )
	{
#ifndef WINDOWS_PHONE
		DWORD pid = 0;
		GetWindowThreadProcessId ( appWnd, &pid );

		DeviceController::app_pid	= pid;
#endif

#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
		// Close opencl resources at first if required
		if ( opt_useWinD3D ) {
			if ( RenderOpenCL::ocl_initialized )
				RenderOpenCL::DisposeOpenCL ();

			CaptureWinD3D::DisposeD3D ();
			if ( !CaptureWinD3D::InitD3D (env) || !CaptureWinD3D::d3dDevice )
				opt_useWinD3D = false;
		}
#endif

		return 1;
	}



#ifdef NATIVE_WIFI_OBSERVER

	unsigned int		lastScan = 0;


	void WifiObserverNotification ( WLAN_NOTIFICATION_DATA * data, VOID * context )
	{
		if ( data->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM ) {
			// wlan_notification_acm_scan_list_refresh == 26 (vs2010 is missing this)
			if ( data->NotificationCode == 26 || data->NotificationCode == wlan_notification_acm_scan_complete )
				lastScan = GetEnvironsTickCount32 ();
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
		native.wifiObserver.thread.Notify ( "WifiObserverNotification" );
	}


	unsigned char GetWiFiChannel ( unsigned long centerFreq )
	{
		unsigned long f1 = ( centerFreq % 2412000 ) / 1000;
		return ( unsigned char ) ( ( f1 / 5 ) + 1 );
	}


	void * Thread_WifiObserver ()
	{
		CLog ( "WifiObserver: Created ..." );

		HANDLE	hClient		= 0;
		DWORD	maxClients	= 2;
		DWORD	curVersion	= 0;
		DWORD	result		= 0;
		WifiObserver * wifi = &native.wifiObserver;

		char		 ssidName [ 256 ];
		bool		 doScan = true;
		unsigned int lastCheck = 0;
		unsigned int i;

		WLAN_INTERFACE_INFO_LIST	* interfaceList = 0;
		WLAN_INTERFACE_INFO			* interfaceInfo = 0;

		result = dWlanOpenHandle ( maxClients, NULL, &curVersion, &hClient );
		if ( result != ERROR_SUCCESS ) {
			CErrArg ( "WifiObserver: OpenHandle failed [ %u ]", result );
			return 0;
		}

		result = dWlanEnumInterfaces ( hClient, NULL, &interfaceList );
		if ( result != ERROR_SUCCESS ) {
			CErrArg ( "WifiObserver: EnumInterfaces failed [ %u ]", result );
			return 0;
		}
		//  | WLAN_NOTIFICATION_SOURCE_ONEX
		result = dWlanRegisterNotification ( hClient, WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM, TRUE, ( WLAN_NOTIFICATION_CALLBACK ) WifiObserverNotification, 0, 0, 0 );
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
					doScan = false;
					lastScan = GetEnvironsTickCount32 ();

					result = dWlanScan ( hClient, &interfaceInfo->InterfaceGuid, 0, 0, 0 );

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

				if ( dWlanGetNetworkBssList ( hClient, &interfaceInfo->InterfaceGuid, NULL, dot11_BSS_type_any, 0, NULL, &bssList ) == ERROR_SUCCESS )
				{
					if ( bssList->dwNumberOfItems > 0 )
					{
						native.wifiObserver.Begin ();

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

							native.wifiObserver.UpdateWithMac ( bssid, ssidName, pEntry->lRssi, pEntry->uLinkQuality, channel, 0 );
						}

						native.wifiObserver.Finish ();
					}
				}

				if ( bssList )
					dWlanFreeMemory ( bssList );
			}

			lastCheck = GetEnvironsTickCount32 ();

			unsigned int waitTime = native.useWifiInterval;

		WaitLoop:
			if ( wifi->threadRun ) {
				wifi->thread.WaitOne ( "WifiObserver", waitTime );

				unsigned int now = GetEnvironsTickCount32 ();
				unsigned int diff = now - lastCheck;

				if ( diff < NATIVE_WIFI_OBSERVER_INTERVAL_CHECK_MIN ) {
					waitTime = ( NATIVE_WIFI_OBSERVER_INTERVAL_CHECK_MIN + 30 ) - diff;
					goto WaitLoop;
				}

				//interfaceInfo->isState == wlan_interface_state_connected
				if ( ( now - lastScan ) > ( unsigned ) native.useWifiInterval )
					doScan = true;
			}
		}

		dWlanRegisterNotification ( hClient, WLAN_NOTIFICATION_SOURCE_NONE, TRUE, 0, 0, 0, 0 );

		if ( interfaceList )
			dWlanFreeMemory ( interfaceList );

		if ( hClient )
			dWlanCloseHandle ( hClient, 0 );

		CLog ( "WifiObserver: bye bye ..." );
		return 0;
	}


#endif



} /* namespace environs */

#endif

