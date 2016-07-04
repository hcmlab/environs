/**
 * Environs Windows platform specific
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

#ifdef _WIN32

#define USE_LOCATION_SERVICE

#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Sensors.h"
#include "Message.Instance.h"
#include "File.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Portal.Instance.h"
#include "Utils/Login.Dialog.Win32.h"

#ifdef USE_LOCATION_SERVICE
#	include <atlbase.h>
#	include <atlcom.h>
#	include <LocationApi.h>

#	pragma comment(lib, "LocationAPI.lib")
#endif

using namespace environs;
using namespace environs::API;

#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")


// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.Windows . . . ."


namespace environs 
{
	bool					opt_useWinD3D			= false;
	bool					opt_useWinTGDI			= false;

#ifdef USE_LOCATION_SERVICE
	namespace API
	{
		bool CreateLocationInterface ();
	}

	class InitializeATL : public CAtlDllModuleT<InitializeATL> { };

	InitializeATL ATLInit;


	class LocationListener : public CComObjectRoot, public ILocationEvents
	{
	public:
		LocationListener () { }
		virtual ~LocationListener () { }

		DECLARE_NOT_AGGREGATABLE ( LocationListener )

		BEGIN_COM_MAP ( LocationListener )
			COM_INTERFACE_ENTRY ( ILocationEvents )
		END_COM_MAP ()

		STDMETHOD ( OnLocationChanged )( __RPC__in REFIID type, __RPC__in_opt ILocationReport* report );

		STDMETHOD ( OnStatusChanged )( __RPC__in REFIID type, LOCATION_REPORT_STATUS status );
	};


	CComPtr<ILocation>				locationIntf;
	CComObject<LocationListener>*	locationEvents		= NULL;

	IID locationTypes [ ]	= { IID_ILatLongReport };
#endif
    
    void EnvironsPlatformInit ( int hInst )
    {
        CVerb ( "EnvironsPlatformInit" );

    }
    
    
    void InvokeNetworkNotifier ( int hInst, bool enable )
    {
        CVerb ( "InvokeNetworkNotifier" );
    }
    
    
	/**
	* Creates an application identifier by means of a UUID
	*
	* @param	buffer	The UUID will be stored in this buffer.
	* @param	bufSize	The size of the buffer. Must be at least 180 bytes.
	* @return	success
	*/
	bool CreateAppID ( char * buffer, unsigned int bufSize )
	{
#ifndef WINDOWS_PHONE
		UUID uuid;
		Zero ( uuid );

		if ( ::UuidCreate ( &uuid ) != RPC_S_OK ) {
			CWarn ( "CreateAppID: UuidCreate was not successfull." );
		}

		char * pUuid = 0;
		if ( ::UuidToStringA ( &uuid, (RPC_CSTR *) &pUuid ) != RPC_S_OK || !pUuid ) {
			CWarn ( "CreateAppID: UuidToStringA was not successfull." );
			return false;
		}

		bool success = true;

		if ( snprintf ( buffer, bufSize, "%s", pUuid ) < 0 )
			success = false;

		::RpcStringFreeA ( (RPC_CSTR *) &pUuid );
#endif
		return success;
	}


	bool DetermineAndInitWorkDir ()
	{
#ifndef WINDOWS_PHONE
		if ( !native.workDir )
		{
			char * workingDirectory = 0;

			DWORD length = GetCurrentDirectory ( 0, 0 );
			if ( length > 0 ) {
				length += 2;

				workingDirectory = ( char * ) malloc ( length + 2 );
				if ( workingDirectory ) {
					length = GetCurrentDirectoryA ( length, workingDirectory );
					if ( length > 0 ) {
						workingDirectory [ length ] = '/';
						workingDirectory [ length + 1 ] = 0;

						InitWorkDirN ( workingDirectory );

						InitStorageN ( workingDirectory );
					}
					free ( workingDirectory );
				}
			}
		}
#endif
        return true;
	}

    
	bool AllocNativePlatform ( )
	{
		CInfo ( "AllocNativePlatform" );

		bool changed = false;

		if ( native.deviceName [ 0 ] == ENVIRONS_DEFAULT_DEVICE_NAME [ 0 ] && native.deviceName [ 7 ] == ENVIRONS_DEFAULT_DEVICE_NAME [ 7 ] )
		{
			char deviceName [ MAX_COMPUTERNAME_LENGTH + 1 ];
			DWORD len = MAX_COMPUTERNAME_LENGTH;

			if ( GetComputerNameA ( deviceName, &len ) ) {
				strlcpy ( native.deviceName, deviceName, sizeof ( native.deviceName ) );
				changed = true;
			}
		}

		if ( !*native.deviceUID ) {
			CreateAppID ( native.deviceUID, sizeof ( native.deviceUID ) );
			changed = true;
		}

		if ( changed )
			environs::SaveConfig ();
		return true;
	}



	/**
	* Perform SDK checks to detect Samsung SUR40 SDK, MultiTaction SDK, etc.
	*
	*/
	void DetectSDKs ( )
    {
        CVerb ( "DetectSDKs" );

		HKEY hKey = NULL;
		LONG ret = 0;

		// Detect Surface 1 SDK
		ret = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Surface\\v1.0", 0, KEY_READ, &hKey );
		if ( ret == ERROR_SUCCESS ) {
			CLog ( "DetectSDKs: Surface SDK1 installed." );
			if ( hKey ) {
				RegCloseKey ( hKey );
				hKey = NULL;
			}
		}

		// Detect Samsung SUR40 SDK
		ret = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Surface\\v2.0", 0, KEY_READ, &hKey );
		if ( ret == ERROR_SUCCESS ) {
			CLog ( "DetectSDKs: Surface SDK2 installed." );
			if ( hKey ) {
				RegCloseKey ( hKey );
				hKey = NULL;
			}
		}

		// Detect MultiTaction Cell SDK
	}


	/**
	* Perform platform checks to detect Samsung SUR40, MultiTaction Cells, etc.
	*
	*/
	void DetectPlatform ( )
    {
        CVerb ( "DetectPlatform" );
        
		if ( native.platform != Platforms::Unknown )
			return;

		HKEY hKey = NULL;
		LONG ret = 0;

		unsigned int tplatform = Platforms::Windows_Flag;

		// Detect MultiTaction Cell
		ret = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\MultiTouch\\MTSvc", 0, KEY_READ | KEY_WOW64_64KEY, &hKey );
		if ( ret == ERROR_SUCCESS && hKey ) {
			CLog ( "DetectPlatform: MultiTaction detected." );

			tplatform |= Platforms::MultiTaction_Flag;
			tplatform |= Platforms::Tabletop_Flag;

			WCHAR buffer [ 64 ];
			DWORD bufSize ( sizeof ( buffer ) );

			ret = ::RegQueryValueExW ( hKey, L"Model", 0, NULL, ( LPBYTE ) buffer, &bufSize );

			if ( ret == ERROR_SUCCESS && bufSize ) {
				if ( wcsstr ( buffer, L"55" ) ) {
					tplatform |= Platforms::MultiTaction55;
					CLog ( "DetectPlatform: MultiTaction 55 detected." );

					native.display.dpi = ( float )39.26;
					native.display.width = 1920;
					native.display.height = 1080;

					native.display.width_mm = 1208;
					native.display.height_mm = 681;
				}
				else if ( wcsstr ( buffer, L"42" ) ) {
					tplatform |= Platforms::MultiTaction55;
					CLog ( "DetectPlatform: MultiTaction 42 detected." );

					native.display.dpi = ( float )39.26;
					native.display.width = 1920;
					native.display.height = 1080;

					// This needs to be updated for the real device
					native.display.width_mm = 1208;
					native.display.height_mm = 681;
				}
			}
			native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;

			native.platform = ( Platforms_t ) tplatform;
			RegCloseKey ( hKey );
			hKey = NULL;
			return;
		}

		// Detect Samsung SUR40
		ret = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Surface\\v2.0\\Shell", 0, KEY_READ, &hKey );
		if ( ret == ERROR_SUCCESS && hKey ) {
			CLog ( "DetectPlatform: Samsung SUR40 aka Pixelsense detected." );

			native.platform = ( Platforms_t ) ( tplatform | Platforms::SAMSUR40 | Platforms::MSSurface_Flag | Platforms::Tabletop_Flag );
			RegCloseKey ( hKey );
			hKey = NULL;

			native.display.width = 1920;
			native.display.height = 1080;

			native.display.width_mm = 885;
			native.display.height_mm = 498;

			native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;
			return;
		}

		// Detect Surface 1 tabletop
		ret = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Surface\\v1.0", 0, KEY_READ, &hKey );
		if ( ret == ERROR_SUCCESS && hKey ) {
			DWORD bufSize ( sizeof ( DWORD ) );
			DWORD val ( 0 );
			ret = ::RegQueryValueExW ( hKey, L"IsNonSurfaceUnit", 0, NULL, reinterpret_cast<LPBYTE>( &val ), &bufSize );

			if ( ret == ERROR_SUCCESS && !val ) {
				CLog ( "DetectPlatform: Surface 1 tabletop detected." );

				native.platform = ( Platforms_t ) ( tplatform | Platforms::MSSUR01 | Platforms::MSSurface_Flag | Platforms::Tabletop_Flag );

				native.display.width = 1024;
				native.display.height = 768;

				native.display.width_mm = 608;
				native.display.height_mm = 456;

				native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;
			}
			RegCloseKey ( hKey );
			hKey = NULL;

			if ( native.platform )
				return;
		}

		tplatform |= Platforms::Display_Flag;

		native.platform = ( Platforms_t ) tplatform;

		native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;

		native.display.width = GetSystemMetrics ( SM_CXSCREEN );
		native.display.height = GetSystemMetrics ( SM_CYSCREEN );

		// Assume 96 dpi
		native.display.dpi = 96.0f;

		native.display.width_mm = ( int ) ( ( native.display.width / 96 ) * 25.4f );
		native.display.height_mm = ( int ) ( ( native.display.height / 96 ) * 25.4f );
	}

    
    namespace lib
    {
        /*void DeviceList::PlatformDispose ()
        {
        }*/
        
        void MessageInstance::PlatformDispose ()
        {
        }
        
        void FileInstance::PlatformDispose ()
        {
        }
        
        void DeviceInstance::PlatformDispose ()
        {
        }
        
        void PortalInstance::PlatformDispose ()
        {
        }
        
        void DeviceList::PlatformDispose ()
        {
        }
        

		bool PortalInstance::ShowDialogOutgoingPortal ()
        {
			/*std::string message = "Establish outgoing portal to " + std::string ( device_->info_.deviceID ) + " " + device_->info_.appName + "/" + device_->info_.areaName;

			int msgboxID = MessageBoxA (
				NULL,
				message.c_str (),
				"Outgoing portal",
				MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2
				);

			switch ( msgboxID )
			{
			case IDYES:
				break;
			}*/
			return false;
        }


		bool DeviceList::DeviceListUpdateDispatchSync ( const sp ( DeviceListUpdatePack ) &updatePacks )
		{
			return DeviceList::DeviceListUpdateDataSourceSync ( updatePacks );
		}
    }
    
    
	namespace API
	{
		const char * GetSSID ( bool desc )
		{
			return "Not implemented";
		}
        
        
        void Environs_LoginDialog ( int hInst, const char * userName )
        {
			environs::LoginDialog * dlg = environs::LoginDialog::SingletonInstance ( hInst, "Enter user/pass", "Mediator Logon", userName );
			if ( !dlg )
				return;
			dlg->ShowResult ();
        }
        
        
        bool RenderSurfaceCallback ( int type, void * surface, void * decoderOrByteBuffer )
        {
			WARNING ( TODO RenderSurfaceCallback )
				if ( type == RENDER_CALLBACK_TYPE_DECODER )
				{
					if ( !surface || !decoderOrByteBuffer )
						return false;

					//RenderDecoderToSurface ( surface, decoderOrByteBuffer );
				}
				else if ( type == RENDER_CALLBACK_TYPE_IMAGE )
				{
					if ( !surface || !decoderOrByteBuffer )
						return false;

					//RenderImageToSurface ( surface, decoderOrByteBuffer );
				}
				else if ( type == RENDER_CALLBACK_TYPE_INIT )
				{
					if ( !decoderOrByteBuffer ) {
						CErr ( "renderSurfaceCallback: invalid buffer!" );
						return false;
					}

					char * buffer = ( BYTEBUFFER_DATA_POINTER_START ( decoderOrByteBuffer ) ) + 4;

					int width = *( ( int * ) buffer );
					int height = *( ( int * ) ( ( ( char * ) buffer ) + 4 ) );

					CLogArg ( "renderSurfaceCallback: got [%s]stream init with width [%i] and height [%i]",
						( ( ( ByteBuffer * ) decoderOrByteBuffer )->type & DATA_STREAM_IMAGE ) == DATA_STREAM_IMAGE ? "image" : "h264", width, height );
				}
            
            return true;
        }
        

		bool CreateLocationInterface ()
		{
#ifdef USE_LOCATION_SERVICE
			HRESULT hr = S_OK;

			if ( locationIntf == NULL ) {
				hr = locationIntf.CoCreateInstance ( CLSID_Location );
				if ( FAILED ( hr ) ) {
					CErr ( "CreateLocationInterface: Creating location interface failed." );
					return false;
				}
			}

			if ( locationEvents == NULL )
			{
				hr = CComObject<LocationListener>::CreateInstance ( &locationEvents ); 
				if ( FAILED ( hr ) || !locationEvents ) {
					CErr ( "CreateLocationInterface: Creating location listener failed." );
					return false;
				}
				locationEvents->AddRef ();

				hr = locationIntf->RequestPermissions ( NULL, locationTypes, ARRAYSIZE ( locationTypes ), FALSE );
				if ( FAILED ( hr ) )
				{
					CErr ( "CreateLocationInterface: No permission to access location." );
					locationEvents->Release ();
					locationEvents = 0;
					return false;
				}

				for ( int index = 0; index < ARRAYSIZE ( locationTypes ); index++ )
				{
					hr = locationIntf->RegisterForReport ( locationEvents, locationTypes [ index ], 0 );

					if ( FAILED ( hr ) )
					{
						CErr ( "CreateLocationInterface: Failed to register for location reports." );
						locationEvents->Release ();
						locationEvents = 0;
						return false;
					}
				}
			}
#endif
			return true;
		}


		bool DisposeLocationInterface ()
		{
#ifdef USE_LOCATION_SERVICE

			if ( locationEvents != NULL )
			{
				locationEvents->Release ();
				locationEvents = 0;
			}

			if ( locationIntf != NULL ) {
				locationIntf = 0;
			}
#endif
			return true;
		}

        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailableImpl ( int hInst, environs::SensorType_t sensorType )
        {
			switch ( sensorType )
			{
			case environs::SensorType::Location:
				if ( CreateLocationInterface () )
					return true;
				break;
			}
            return false;
        }

        
        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         */
		bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType )
		{
			switch ( sensorType )
            {
			case environs::SensorType::Location:
				if ( CreateLocationInterface () )
					return true;
				break;
            }
            return false;
        }


		/**
		* Deregister to sensor events and stop listen to sensor data events.
		* This implementation is platform specific and needs to be implemented
		* in the particular platform layer.
		*
		* @param sensorType A value of type environs::SensorType.
		*
         */
        void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType )
		{
			if ( sensorType == -1 ) {
				sensorRegistered = 0;

				DisposeLocationInterface ();
				return;
			}

			switch ( sensorType )
            {
                case environs::SensorType::Location:
					DisposeLocationInterface ();                    
                    break;
			}
		}
	}


#ifdef USE_LOCATION_SERVICE
	STDMETHODIMP LocationListener::OnLocationChanged ( __RPC__in REFIID type, __RPC__in_opt ILocationReport* report )
	{
		if ( IID_ILatLongReport == type )
		{
			CComPtr<ILatLongReport> ireport;

			if ( ( SUCCEEDED ( report->QueryInterface ( IID_PPV_ARGS ( &ireport ) ) ) ) && ( ireport.p ) )
			{
				DOUBLE value = 0;

				if ( SUCCEEDED ( ireport->GetLatitude ( &value ) ) )
				{
					CLogArg ( "OnLocationChanged: Latitude [ %f ]", value );
				}

				if ( SUCCEEDED ( ireport->GetLongitude ( &value ) ) )
				{
					CLogArg ( "OnLocationChanged: Longitude [ %f ]", value );
				}

				if ( SUCCEEDED ( ireport->GetAltitude ( &value ) ) )
				{
					CLogArg ( "OnLocationChanged: Altitude [ %f ]", value );
				}

				if ( SUCCEEDED ( ireport->GetErrorRadius ( &value ) ) )
				{
					CLogArg ( "OnLocationChanged: Error Radius [ %f ]", value );
				}

				if ( SUCCEEDED ( ireport->GetAltitudeError ( &value ) ) )
				{
					CLogArg ( "OnLocationChanged: Error Altitude [ %f ]", value );
				}
			}
		}

		return S_OK;
	}


	STDMETHODIMP LocationListener::OnStatusChanged ( __RPC__in REFIID type, LOCATION_REPORT_STATUS status )
	{
		if ( IID_ILatLongReport == type )
		{
			switch ( status )
			{
			case REPORT_NOT_SUPPORTED:
				CLog ( "OnStatusChanged: REPORT_NOT_SUPPORTED" );
				break;
			case REPORT_ERROR:
				CLog ( "OnStatusChanged: REPORT_ERROR" );
				break;
			case REPORT_ACCESS_DENIED:
				CLog ( "OnStatusChanged: REPORT_ACCESS_DENIED" );
				break;
			case REPORT_INITIALIZING:
				CLog ( "OnStatusChanged: REPORT_INITIALIZING" );
				break;
			case REPORT_RUNNING:
				CLog ( "OnStatusChanged: REPORT_RUNNING" );
				break;
			}
		}

		return S_OK;
	}
#endif

} /* namespace environs */

#endif

