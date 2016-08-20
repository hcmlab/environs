/**
 * Environs CLI implementation common
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

#ifdef CLI_CPP

#include "Environs.Cli.Forwards.h"
#include "Environs.Types.h.cli.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Cli.Base.h"
#include "Environs.Cli.h"

#include "Message.Instance.h"
#include "File.Instance.h"
#include "Portal.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Environs.Cli.Base.h"
#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Cli.TouchDevice.h"
#include "Environs.Build.Lnk.h"
#include "Environs.Native.h"

#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>


#define CLASS_NAME	"Environs.Cli . . . . . ."


namespace environs
{
	String ^ getEnvironsMissingMsg ( String ^ reason );
	
	namespace lib
	{
		enum CompositionAction
		{
			/// <summary>
			/// To enable DWM composition
			/// </summary>
			DWM_EC_DISABLECOMPOSITION = 0,
			/// <summary>
			/// To disable composition.
			/// </summary>
			DWM_EC_ENABLECOMPOSITION = 1
		};

		[DllImport ( "dwmapi.dll", PreserveSig = false )]
		extern void DwmEnableComposition ( CompositionAction uCompositionAction );
	}


	Environs::Environs ()
	{
		initialized			= false;

#ifndef CLI_NOUI
		appWindow			= nullptr;
		windowStartStopEvent = gcnew System::Threading::ManualResetEvent ( false );

		Initialized			= nullptr;

		onCloseAttached		= false;
#endif		
		disposeOnClose		= true;

		onNotify			= gcnew NotificationSourceInt ( this, &Environs::BridgeForNotify );

		onNotifyExt			= gcnew NotificationSourceExtInt ( this, &Environs::BridgeForNotifyExt );

		onMessage			= gcnew MessageSourceInt ( this, &Environs::BridgeForMessage );

		onMessageExt		= gcnew MessageSourceExtInt ( this, &Environs::BridgeForMessageExt );

		onStatusMessage		= gcnew StatusMessageSourceInt ( this, &Environs::BridgeForStatusMessage );

		onData				= gcnew DataSourceInt ( this, &Environs::BridgeForData );

		onHumanInput		= gcnew EnvironsHumanInputHandlerRaw ( this, &Environs::BridgeForInputData );

		onUdpData			= gcnew EnvironsUdpDataHandlerRaw ( this, &Environs::BridgeForUdpData );

		onNotify_ptr		= IntPtr::Zero;
		onNotifyExt_ptr		= IntPtr::Zero;
		onMessage_ptr		= IntPtr::Zero;
		onMessageExt_ptr	= IntPtr::Zero;
		onStatusMessage_ptr = IntPtr::Zero;
		onData_ptr			= IntPtr::Zero;
		onHumanInput_ptr	= IntPtr::Zero;
		onUdpData_ptr		= IntPtr::Zero;

#if (defined(CLI_PS) || defined(CLI_STT))
		isSurface			= false;
		latencyIndicator	= nullptr;
		updateDeviceTagPosition = true;
#endif

#ifndef CLI_NOUI
		isUIAdapter			= true;
#else
		isUIAdapter			= false;
#endif
	}


	Environs::~Environs ()
	{
#ifndef CLI_NOUI
		if ( windowStartStopEvent ) {
			delete windowStartStopEvent;
			windowStartStopEvent = nullptr;
		}
#endif
	}


	/**
	* Add an observer for status changes.
	*
	* @param   observer Your implementation of EnvironsStatusObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForStatus ( EnvironsStatusObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnStatus += observer;
		return true;
	}

	/**
	* Remove an observer for status changes.
	*
	* @param   observer Your implementation of EnvironsStatusObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForStatus ( EnvironsStatusObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnStatus -= observer;
		return true;
	}

	/**
	* Add an observer for notifications.
	*
	* @param   observer Your implementation of EnvironsNotificationObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForNotify ( EnvironsNotificationObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnNotify += observer;
		return true;
	}

	/**
	* Remove an observer for notifications.
	*
	* @param   observer Your implementation of EnvironsNotificationObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForNotify ( EnvironsNotificationObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnNotify -= observer;
		return true;
	}

	/**
	* Add an observer for extended notifications.
	*
	* @param   observer Your implementation of EnvironsNotificationObserverExt.
	*
	* @return	success
	*/
	bool Environs::AddObserverForNotifyExt ( EnvironsNotificationObserverExt ^ observer )
	{
		if ( observer == nill )
			return false;
		OnNotifyExt += observer;
		return true;
	}

	/**
	* Remove an observer for extended notifications.
	*
	* @param   observer Your implementation of EnvironsNotificationObserverExt.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForNotifyExt ( EnvironsNotificationObserverExt ^ observer )
	{
		if ( observer == nill )
			return false;
		OnNotifyExt -= observer;
		return true;
	}

	/**
	* Add an observer for messages.
	*
	* @param   observer Your implementation of EnvironsMessageObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForMessages ( EnvironsMessageObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnMessage += observer;
		return true;
	}
	
	/**
	* Remove an observer for messages.
	*
	* @param   observer Your implementation of EnvironsMessageObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForMessages ( EnvironsMessageObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnMessage -= observer;
		return true;
	}

	/**
	* Add an observer for extended messages.
	*
	* @param   observer Your implementation of EnvironsMessageObserverExt.
	*
	* @return	success
	*/
	bool Environs::AddObserverForMessagesExt ( EnvironsMessageObserverExt ^ observer )
	{
		if ( observer == nill )
			return false;
		OnMessageExt += observer;
		return true;
	}

	/**
	* Remove an observer for extended messages.
	*
	* @param   observer Your implementation of EnvironsMessageObserverExt.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForMessagesExt ( EnvironsMessageObserverExt ^ observer )
	{
		if ( observer == nill )
			return false;
		OnMessageExt -= observer;
		return true;
	}

	/**
	* Add an observer for status messages.
	*
	* @param   observer Your implementation of EnvironsStatusMessageObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForStatusMessages ( EnvironsStatusMessageObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnStatusMessage += observer;
		return true;
	}

	/**
	* Remove an observer for status messages.
	*
	* @param   observer Your implementation of EnvironsStatusMessageObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForStatusMessages ( EnvironsStatusMessageObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnStatusMessage -= observer;
		return true;
	}

	/**
	* Add an observer for data.
	*
	* @param   observer Your implementation of EnvironsDataObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForData ( EnvironsDataObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnData += observer;
		return true;
	}

	/**
	* Remove an observer for data.
	*
	* @param   observer Your implementation of EnvironsDataObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForData ( EnvironsDataObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnData -= observer;
		return true;
	}

	/**
	* Add an observer for human input, such as touch.
	*
	* @param   observer Your implementation of EnvironsHumanInputObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForHumanInput ( EnvironsHumanInputObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnHumanInput += observer;
		return true;
	}

	/**
	* Remove an observer for human input, such as touch.
	*
	* @param   observer Your implementation of EnvironsHumanInputObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForHumanInput ( EnvironsHumanInputObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnHumanInput -= observer;
		return true;
	}

	/**
	* Add an observer for sensor data.
	*
	* @param   observer Your implementation of EnvironsSensorDataObserver.
	*
	* @return	success
	*/
	bool Environs::AddObserverForSensorData ( EnvironsSensorDataObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnSensorInput += observer;
		return true;
	}

	/**
	* Remove an observer for sensor data.
	*
	* @param   observer Your implementation of EnvironsSensorDataObserver.
	*
	* @return	success
	*/
	bool Environs::RemoveObserverForSensorData ( EnvironsSensorDataObserver ^ observer )
	{
		if ( observer == nill )
			return false;
		OnSensorInput -= observer;
		return true;
	}


	/**
	* BridgeForInputData is called (by the native layer) when an input event has been received from a connected device.
	*
	* @param hInst			A handle to the Environs instance
	* @param nativeID       The native device id of the sender device.
	* @param pack			The input data frame.
	*/
	void Environs::BridgeForInputData ( int hInst, int nativeID, Addr_obj data )
	{
		InputPack ^ pack = lib::GetInputPack ( data );
		if ( pack == nullptr )
			return;

		InjectTouch ( nativeID, pack );

		if ( OnHumanInput != nullptr )
			OnHumanInput ( nativeID, pack );
	}


	/**
	* Create an Environs object.
	*
	* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
	* @param 	appName		The application name for the application environment.
	* @param  	areaName	The area name for the application environment.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::CreateInstance ( String ^appName, String ^areaName )
	{
		Environs ^ env = gcnew Environs ();
		if ( env != nullptr )
		{
			if ( !env->InitInstance ( nullptr ) )
				return nullptr;

			env->LoadSettings ( appName, areaName );
		}
		return env;
	}


	DeviceList ^ Environs::CreateDeviceList ( environs::DeviceClass MEDIATOR_DEVICE_CLASS_ )
	{
		return lib::Environs::CreateDeviceList ( MEDIATOR_DEVICE_CLASS_ );
	}


	bool Environs::InitPlatform ()
	{
		if ( onUdpData == nill || onNotify == nill || onNotifyExt == nill || onMessage == nill ||
			onMessageExt == nill || onData == nill || onStatusMessage == nill )
			return false;

		onHumanInput_ptr = Marshal::GetFunctionPointerForDelegate ( onHumanInput );
		onUdpData_ptr = Marshal::GetFunctionPointerForDelegate ( onUdpData );

		onNotify_ptr = Marshal::GetFunctionPointerForDelegate ( onNotify );
		onNotifyExt_ptr = Marshal::GetFunctionPointerForDelegate ( onNotifyExt );

		onMessage_ptr = Marshal::GetFunctionPointerForDelegate ( onMessage );
		onMessageExt_ptr = Marshal::GetFunctionPointerForDelegate ( onMessageExt );

		onData_ptr = Marshal::GetFunctionPointerForDelegate ( onData );
		onStatusMessage_ptr = Marshal::GetFunctionPointerForDelegate ( onStatusMessage );

		if ( onHumanInput_ptr == IntPtr::Zero || onUdpData_ptr == IntPtr::Zero || onNotify_ptr == IntPtr::Zero ||
			onNotifyExt_ptr == IntPtr::Zero || onMessage_ptr == IntPtr::Zero ||
			onMessageExt_ptr == IntPtr::Zero || onData_ptr == IntPtr::Zero || onStatusMessage_ptr == IntPtr::Zero )
			return false;


		environs::API::SetCallbacksN ( hEnvirons, (void *) static_cast<void *>(onHumanInput_ptr.ToPointer ()), (void *) static_cast<void *>( onUdpData_ptr.ToPointer ()),
			(void *) static_cast<void *>(onMessage_ptr.ToPointer ()), (void *) static_cast<void *>(onMessageExt_ptr.ToPointer () ),
			(void *) static_cast<void *>(onNotify_ptr.ToPointer () ), (void *) static_cast<void *>(onNotifyExt_ptr.ToPointer () ),
			(void *) static_cast<void *>(onData_ptr.ToPointer ()), (void *) static_cast<void *>(onStatusMessage_ptr.ToPointer ()) );

#ifndef CLI_NOUI
		do_FullScreen = true;
		do_PortalFromAppWindow = true;
#endif
		keyInput = "";
		debug_device_tagID = 0xDD;

		return lib::Environs::InitPlatform ();
	}


	bool Environs::InitInstance ( EnvironsInitializedHandler ^ handler )
	{
		CVerb ( "InitInstance" );

		if ( nativeStatus < 2 ) {
			System::Windows::MessageBox::Show ( getEnvironsMissingMsg ( "Environs.dll" ) );
			return false;
		}

#ifndef CLI_NOUI
		if ( handler != nullptr )
			Initialized += handler;
#endif

		int hInst = environs::API::CreateEnvironsN ();
		if ( hInst <= 0 || hInst >= ENVIRONS_MAX_ENVIRONS_INSTANCES )
			return false;

		bool success = false;

		do
		{
			hEnvirons = hInst;

			LockAcquireA ( platformLock, "InitInstance" );

			instancesAPI [ hInst ] = this;

			LockReleaseA ( platformLock, "InitInstance" );

			if ( !lib::Environs::Init () )
				break;

			if ( !InitPlatformLayer () )
				break;
#ifdef CLI_PS
			lib::DwmEnableComposition ( lib::DWM_EC_DISABLECOMPOSITION );
#endif
			success = true;
		}
		while ( false );

		if ( !success ) {
			environs::API::DisposeN ( hInst );

			LockAcquireA ( platformLock, "InitInstance" );

			instancesAPI [ hInst ] = nullptr;

			LockReleaseA ( platformLock, "InitInstance" );

		}
		return success;
	}

	
	/**
	* Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
	* 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
	* 		this call executes the start tasks within a thread.&nbsp;
	* 		In order to get the status, catch the onNotify handler of your EnvironsListener.
	*
	*/
	void Environs::Start ()
	{
		if ( environs::API::GetStatusN ( hEnvirons ) > ( int ) Status::Stopped )
		{
			CWarn ( "Environs has already started." );
			return;
		}

		StartPlatformHandlers ();

		System::Threading::Thread ^ init = gcnew Thread ( gcnew ThreadStart ( this, &Environs::InitThread ) );
		if ( init == nullptr )
			CErr ( "Failed to create initialization thread!!!" );
		else
			init->Start ();
	}
	

	void Environs::Stop ()
	{
#ifndef CLI_NOUI
		DetachEventHandlers ( true );
#endif

		lib::Environs::Stop ();

#ifndef CLI_NOUI
		if ( async == Call::Wait )
			WaitUIFinished ( windowStartStopEvent );
#endif
	}


	/**
	* Initialization thread 
	*
	*/
	void Environs::InitDispatched ()
	{
		try
		{
			environs::API::RegisterMainThreadN ( hEnvirons );

#ifndef CLI_NOUI
			// Enable fullscreen
			if ( do_FullScreen && appWindow != nullptr )
			{
				SwitchToFullScreen ();
#if defined(ENABLE_NETWORKLIST)
				if ( networkStatus != nullptr )
					networkStatus->Start ();
#endif
			}

			if ( Initialized != nullptr ) {
				Initialized ();
				Initialized = nullptr;
			}
#endif

			CVerb ( "Initialization thread done." );
		}
		catch ( Exception ^ ex )
		{
            CErr ( "Environs: " + ( ex != nill ? ex->Message : "" ) );
		}
	}


	/**
	* Initialization thread for establishing and initializing the device environment
	*
	*/
	void Environs::InitThread ()
	{
		CVerb ( "Threaded initialization started ..." );

		try
		{
#ifndef CLI_NOUI
			if ( appWindow != nullptr )
			{
#if defined(XNA)
				//if (!appWindow.IsAccessible)
				windowLoadedEvent->WaitOne ();
#else
				CLog ( "InitThread waiting for loaded event ..." );
				if ( !windowStartStopEvent->WaitOne ( 60000 ) )
					return;
#endif
			}
#endif
			environs::lib::Environs::Start ();

#if defined(ENABLE_NETWORKLIST)
			if ( networkStatus == nullptr )
				networkStatus = gcnew NetworkStatus ();
#endif
			dispatch ( gcnew Action ( this, &Environs::InitDispatched ) );
		}
		catch ( Exception ^ ex )
		{
			CErr ( "Environs: " + ( ex != nill ? ex->Message : "" ) );
		}
	}




	/**
	* Try enabling stream encoder or disable stream encoding.
	*
	*/
	void Environs::SetUseStream ( bool enable )
	{
		SetUseH264 ( enable );
	}


	cli::array < environs::WifiEntry ^ > ^ BuildWifiList ( IntPtr ^ pData )
	{
		if ( pData == nill || pData == IntPtr::Zero )
			return nill;

		cli::array < environs::WifiEntry ^ > ^ data = nill;

		const unsigned int wifiItemSize = 16;

		unsigned char * pBytes = ( unsigned char * ) pData->ToPointer ();

		unsigned int count = *( ( unsigned int * ) pBytes );
		if ( count <= 0 )
			return nill;
		if ( count > 256 )
			count = 256;
		pBytes += 4;

		unsigned int remainSize = *( ( unsigned int * ) pBytes );
		if ( remainSize < wifiItemSize )
			return nill;
		pBytes += 4;

		data = gcnew cli::array < environs::WifiEntry ^ > ( count );
		if ( data == nill )
			return nill;

		unsigned int i = 0;

		for ( ; i < count; i++ )
		{
			if ( remainSize < wifiItemSize )
				break;

			environs::WifiEntry ^ entry = gcnew environs::WifiEntry ();
			if ( entry == nill )
				break;
			
			entry->bssid		= *( ( unsigned long long * ) pBytes ); pBytes += 8;
			entry->rssi			= *( ( short * ) pBytes ); pBytes += 2;
			entry->signal		= *( ( short * ) pBytes ); pBytes += 2;
			entry->channel		= *( ( char * ) pBytes ); pBytes++;
			entry->encrypt		= *( ( char * ) pBytes ); pBytes++;
			entry->isConnected	= (*( ( char * ) pBytes ) != 0); pBytes++;
			entry->sizeOfssid	= ( short ) *( ( char * ) pBytes ); pBytes++;

			remainSize -= wifiItemSize;

			unsigned int ssidSize = ( unsigned int ) entry->sizeOfssid;

			if ( ssidSize > 0 ) {
				if ( ssidSize > 32 )
					break;				

				if ( remainSize < ssidSize )
					break;

				pBytes [ ssidSize - 1 ] = 0;

				entry->ssid = CCharToString ( pBytes );
				pBytes += ssidSize;

				remainSize -= ssidSize;
			}

			data [ i ] = entry;
		}

		for ( ; i < count; i++ )
		{
			data [ i ] = nill;
		}

		return data;
	}


	/**
	* Get a collection that holds all available wifi APs. This list is NOT updated dynamically.
	*
	* @return WifiList with WifiItem objects
	*/
	cli::array<WifiEntry ^> ^ Environs::GetWifis ()
	{
		void * pData = environs::API::GetWifisN ();
		if ( pData == nill )
			return nill;

		array<WifiEntry ^> ^ wifis = BuildWifiList ( IntPtr ( pData ) );

		free_plt ( pData );

		return wifis;
	}


	cli::array < environs::BtEntry ^ > ^ BuildBtList ( IntPtr ^ pData )
	{
		if ( pData == nill || pData == IntPtr::Zero )
			return nill;

		cli::array < environs::BtEntry ^ > ^ data = nill;

		const unsigned int btItemSize = 32;

		unsigned char * pBytes = ( unsigned char * ) pData->ToPointer ();

		unsigned int count = *( ( unsigned int * ) pBytes );
		if ( count <= 0 )
			return nill;
		if ( count > 256 )
			count = 256;
		pBytes += 4;

		unsigned int remainSize = *( ( unsigned int * ) pBytes );
		if ( remainSize < btItemSize )
			return nill;
		pBytes += 4;

		data = gcnew cli::array < environs::BtEntry ^ > ( count );
		if ( data == nill )
			return nill;

		unsigned int i = 0;

		for ( ; i < count; i++ )
		{
			if ( remainSize < btItemSize )
				break;

			environs::BtEntry ^ entry = gcnew environs::BtEntry ();
			if ( entry == nill )
				break;

			entry->bssid		= *( ( unsigned long long * ) pBytes ); pBytes += 8;
			entry->rssi			= *( ( short * ) pBytes ); pBytes += 2;
			entry->isConnected	= ( *( ( char * ) pBytes ) != 0 ); pBytes++;
			entry->sizeOfssid	= ( short ) *( ( char * ) pBytes ); pBytes++;
			entry->uuid1		= *( ( unsigned long long * ) pBytes ); pBytes += 8;
			entry->uuid2		= *( ( unsigned long long * ) pBytes ); pBytes += 8;

			remainSize -= btItemSize;

			unsigned int ssidSize = ( unsigned int ) entry->sizeOfssid;

			if ( ssidSize > 0 ) {
				if ( ssidSize > 32 )
					break;

				if ( remainSize < ssidSize )
					break;

				pBytes [ ssidSize - 1 ] = 0;

				entry->ssid = CCharToString ( pBytes );
				pBytes += ssidSize;

				remainSize -= ssidSize;
			}

			data [ i ] = entry;
		}

		for ( ; i < count; i++ )
		{
			data [ i ] = nill;
		}

		return data;
	}


	/**
	* Get a collection that holds all available Bluetooth devices. This list is NOT updated dynamically.
	*
	* @return BtList with BtItem objects
	*/
	cli::array<BtEntry ^> ^ Environs::GetBts ()
	{
		void * pData = environs::API::GetBtsN ();
		if ( pData == nill )
			return nill;

		array<BtEntry ^> ^ bts = BuildBtList ( IntPtr ( pData ) );

		free_plt ( pData );

		return bts;
	}


	namespace lib
	{
	}
}

#endif



