/**
 * Environs Linux platform specific
 * ------------------------------------------------------------------
 *
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

#ifdef LINUX


#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Sensors.h"
#include "Message.Instance.h"
#include "File.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Portal.Instance.h"
#include "Interfaces/IExt.Thread.h"

#include <string>
#include <limits.h>
#include <unistd.h>
#include <time.h>

#include "DynLib/Dyn.Lib.UUID.h"

using namespace environs;
using namespace environs::API;

// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.Linux . . . . ."


namespace environs
{
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
        uuid_t newUID;
        char uuidStr [ 40 ];

        if ( InitLibUUID ( 0 ) )
        {
            duuid_generate ( newUID );
            if ( duuid_is_null ( newUID ) == 1 )
                return false;

            duuid_unparse_upper ( newUID, uuidStr );

            ReleaseLibUUID ();
        }

        if ( !*uuidStr ){
            CErr ( "CreateAppID: Failed to init uuid!" );
            return false;
        }

        size_t len = strlen ( uuidStr );
        if ( len >= bufSize )
            return false;

        memcpy ( buffer, uuidStr, len );
        buffer [ len ] = 0;
		return true;
	}


    bool DetermineAndInitWorkDir ()
    {
        CVerb ( "DetermineAndInitWorkDir" );

        if ( !native.workDir )
        {
            char workingDirectory [ PATH_MAX ];
            if ( !getcwd ( workingDirectory, sizeof(workingDirectory) ) ) {
                CErr ( "DetermineAndInitWorkDir: Failed to get working directory." );
                return false;
            }

            CVerbVerbArg ( "DetermineAndInitWorkDir: [ %s ]", workingDirectory );

            char * check = ( workingDirectory + strlen (workingDirectory) );

            if ( *check != '/' ) {
                *check = '/'; check++;
                *check = 0;
            }

            CVerbArg ( "DetermineAndInitWorkDir: [ %s ] adapted.", workingDirectory );

            InitWorkDirN ( workingDirectory );

            InitLibDirN ( 0 );

            InitStorageN ( workingDirectory );
        }
        return true;
	}


    bool AllocNativePlatform ()
	{
		CInfo ( "AllocNativePlatform" );

        char buffer [ 128 ];

        if ( !gethostname ( buffer, sizeof(buffer) ) )
        {
            strlcpy ( native.deviceName, buffer, sizeof ( native.deviceName ) );
        }
        else
            snprintf ( native.deviceName, sizeof ( native.deviceName ), "Unknown-Computer-Name-Nr-%i", rand ( ) );

        CVerbArg ( "AllocNativePlatform: Hostname [ %s ]", native.deviceName );

		return true;
	}



	/**
	* Perform SDK checks to detect Samsung SUR40 SDK, MultiTaction SDK, etc.
	*
	*/
	void DetectSDKs ( )
	{
	}


	/**
	* Perform platform checks to detect Samsung SUR40, MultiTaction Cells, etc.
	*
	*/
	void DetectPlatform ( )
    {
        if ( native.platform != Platforms::Unknown )
            return;

        int platform = Platforms::Display_Flag;

        native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;

        platform |= Platforms::Linux_Flag;

#ifdef RASPBERRY
        platform |= Platforms::Raspberry_Flag;
#endif

        native.platform = (environs::Platforms_t) platform;
    }


#ifdef NATIVE_WIFI_OBSERVER_THREAD
    unsigned int		lastScan	= 0;
    unsigned int        doScan      = true;



    unsigned char GetWiFiChannel ( unsigned long centerFreq )
    {
        unsigned long f1 = ( centerFreq % 2412000 ) / 1000;
        return ( unsigned char ) ( ( f1 / 5 ) + 1 );
    }


    char * FindSep ( char * line )
    {
        char c = *line;

        while ( c != '\t' && c != ' ' && c )
        {
            line++;
            c = *line;

            if ( c == '\n' )
                return 0;
        }
        if ( c == '\n' )
            return 0;
        return line;
    }


    char * FindValue ( char * line )
    {
        char c = *line;

        while ( c == '\t' || c == ' ' )
        {
            line++;
            c = *line;

            if ( c == '\n' )
                return 0;
        }

        if ( c == '\n' )
            return 0;
        return line;
    }


    char * FindEnd ( char * line )
    {
        char c = *line;

        while ( c != '\n' && c )
        {
            line++;
            c = *line;
        }
        if ( !c )
            return 0;
        return line;
    }


    void Parse ( char * line )
    {
        char * bssid = line;

        line    = FindSep ( line );
        if ( !line )
            return;
        *line = 0;

        char * sfreq = FindValue ( line + 1 );
        if ( !sfreq )
            return;

        line    = FindSep ( sfreq );
        if ( !line )
            return;
        *line = 0;

        char * srssi = FindValue ( line + 1 );
        if ( !srssi )
            return;

        line    = FindSep ( srssi );
        if ( !line )
            return;
        *line = 0;

        char * flags = FindValue ( line + 1 );
        if ( !flags )
            return;

        line    = FindSep ( flags );
        if ( !line )
            return;
        *line = 0;

        char * ssid = FindValue ( line + 1 );
        if ( !ssid )
            return;

        line = FindEnd ( ssid );
        if ( !line )
            return;
        *line = 0;

        // Parse rssi
        int rssi = 0;
        if ( sscanf_s ( srssi, "%i", &rssi ) != 1 )
            return;

        unsigned int freq = 0;
        if ( sscanf_s ( sfreq, "%u", &freq ) != 1 )
            return;
        unsigned char channel = GetWiFiChannel ( freq * 10 );

        char encrypt = 0;
        if ( strstr ( flags, "WPA" ) )
            encrypt = 2;
        else if ( strstr ( flags, "WEP" ) )
            encrypt = 1;

        native.wifiObserver.UpdateWithColonMac( bssid, ssid, rssi, 0, channel, encrypt );
    }


    int Scan ()
    {

        FILE * fp;
        char line [ 1024 ];

        if ( doScan ) {
            doScan = false;
            int rc = system ( "sudo -n /sbin/wpa_cli -iwlan0 scan" );
            if ( rc < 0 ) {
                CWarn ( "Scan: wpa_client failed!" );
                return -1;
            }
        }

        fp = popen ( "sudo -n /sbin/wpa_cli -iwlan0 scan_results", "r");
        if ( !fp ) {
            CWarn ( "Scan: wpa_client failed!" );
            return -1;
        }

        while ( fgets(line, sizeof ( line )-1, fp ) != NULL ) {
            CVerbArg ( "Scan: %s", line );

            //printf ( "Scan: %s", line );

            Parse ( line );
        }

        pclose ( fp );

        return 0;
    }


    void WifiObserverNotification ( )
    {
        native.wifiObserver.thread.Notify ( "WifiObserverNotification" );
    }


    bool Is_WPA_Cli_Available ()
    {
        FILE * file = fopen ( "/sbin/wpa_cli", "r" );
        if ( !file )
            return false;

        fclose ( file );
        return true;
    }


    void * Thread_WifiObserver ()
    {
        if ( !Is_WPA_Cli_Available () ) return 0;

        unsigned int        lastCheck   = 0;
        WifiObserver	*	wifi        = &native.wifiObserver;
        doScan  = true;

        while ( wifi->threadRun )
        {
            if ( Scan () < 0 )
                break;

            lastCheck = GetEnvironsTickCount32 ();

            unsigned int waitTime = native.useWifiInterval;

        WaitLoop:

            if ( wifi->threadRun ) {
                wifi->thread.WaitOne ( "WifiObserver", waitTime );

                unsigned int now = GetEnvironsTickCount32 ();
                unsigned int diff = now - lastCheck;

                if ( diff < ENVIRONS_WIFI_OBSERVER_INTERVAL_CHECK_MIN ) {
                    waitTime = ( ENVIRONS_WIFI_OBSERVER_INTERVAL_CHECK_MIN + 30 ) - diff;
                    goto WaitLoop;
                }

                if ( ( now - lastScan ) > ( unsigned ) native.useWifiInterval )
                    doScan = true;
            }
        }

        CLog ( "WifiObserver: bye bye ..." );
        return 0;
    }
#endif



#ifdef NATIVE_BT_OBSERVER

	void * Thread_BtObserver ()
	{
		CLog ( "BtObserver: Created ..." );

		BtObserver *	bt			= &native.btObserver;
		bool			doScan		= true;
		unsigned int	lastCheck	= 0;


		while ( bt->threadRun )
		{

			lastCheck = GetEnvironsTickCount32 ();

			unsigned int waitTime = native.useBtInterval;

		WaitLoop:
			if ( bt->threadRun ) {
				bt->thread.WaitOne ( "BtObserver", waitTime );

				unsigned int now = GetEnvironsTickCount32 ();
				unsigned int diff = now - lastCheck;

				if ( diff < ENVIRONS_BT_OBSERVER_INTERVAL_CHECK_MIN ) {
					waitTime = ( ENVIRONS_BT_OBSERVER_INTERVAL_CHECK_MIN + 30 ) - diff;
					goto WaitLoop;
				}

				if ( ( now - lastScan ) > ( unsigned ) native.useBtInterval )
					doScan = true;
			}
		}


		CLog ( "BtObserver: bye bye ..." );
		return 0;
	}


#endif

    namespace lib
    {
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

		bool PortalInstance::ShowDialogOutgoingPortal ()
        {
			return false;
        }

        void DeviceList::PlatformDispose ()
        {
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
            /*
             NSString * uname = userName ? [[NSString alloc ] initWithUTF8String: userName] : @"User Name";

             LoginDialog * dlg = [LoginDialog SingletonInstance:@"Please enter login credentials:" withTitle:@"Mediator Login" withUserName:uname];
             if ( dlg ) {
             [dlg ShowResult];
             }
             */
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

				char * buffer = (BYTEBUFFER_DATA_POINTER_START ( decoderOrByteBuffer )) + 4;

				int width = *((int *)buffer);
				int height = *((int *)(((char *)buffer) + 4));

				CLogArg ( "renderSurfaceCallback: got [%s]stream init with width [%i] and height [%i]",
					(((ByteBuffer *)decoderOrByteBuffer)->type & DATA_STREAM_IMAGE) == DATA_STREAM_IMAGE ? "image" : "h264", width, height );
			}

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
		bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
		{
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
		void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            if ( sensorType == -1 ) {
                sensorRegistered = 0;
                return;
            }
		}
	}
} /* namespace environs */

#endif

