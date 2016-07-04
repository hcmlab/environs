/**
 * Wifi Observer
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
#if ( !defined(CLI_CPP) )
#   define DEBUGVERB
//#   define DEBUGVERBVerb

#include "Environs.Native.h"
#include "Interop/Threads.h"
#include <map>
#include "Environs.Obj.h"
#include "Wifi.Observer.h"
#include "DynLib/Dyn.WlanAPI.h"
#include <stdlib.h>

#ifdef VS2010
#	define strtoull(a,b,c)	_strtoui64 ( a, b, c )
#endif

#define CLASS_NAME	"Wifi.Observer. . . . . ."

//#define BREAK_INTO_LEAKS

namespace environs
{
#ifdef NATIVE_WIFI_OBSERVER


    /*
     * Wifi observer object
     *
     */

    pthread_mutex_t wifiObserverLock;

    void        *   Thread_WifiObserverStarter ( void * object );
    void        *   Thread_WifiObserver ();

    std::map<unsigned long long, EnvWifiItem *> wifiItems;


    EnvWifiItem * EnvWifiItem::Create ( int _seqNr, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "Create" );

        char * ssid = 0;

		if ( _ssid && *_ssid )
		{
			size_t len = strnlen ( _ssid, 64 );
			if ( len ) {
				ssid = ( char * ) malloc ( len + 1 );
				if ( !ssid ) {
					CErrArg ( "Create: Failed to allocate memory [ %u bytes ]", len );
					return 0;
				}

				memcpy ( ssid, _ssid, len );
				ssid [ len ] = 0;
			}
		}


        EnvWifiItem * item = new EnvWifiItem ( _seqNr, ssid, _rssi, _signal, _channel, _encrypt );

        if ( ssid && !item )
            free ( ssid );

		CVerbArg ( "Create: [ %i dBm \t: C%i \t: E%i \t: %s ]", _rssi, ( int ) _channel, ( int ) _encrypt, ssid );
#if defined(DEBUGVERB) && defined(LINUX)
		printf ( "Create: [ %i dBm \t: C%i \t: E%i \t: %s ]\n", _rssi, ( int ) _channel, ( int ) _encrypt, ssid );
#endif
        return item;
    }


    void EnvWifiItem::Update ( int _seqNr, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "Update" );

        seqNr = _seqNr;

        if ( !_ssid || !*_ssid ) {
            if ( ssid ) {
                free ( ssid );
                ssid = 0;

                native.wifiObserver.itemsChanged = true;
            }
        }
        else {
            bool rebuild = false;

            if ( !ssid || !*ssid )
                rebuild = true;
            else {
                size_t len = strlen ( ssid );

                if ( strncmp ( ssid, _ssid, len ) != 0 ) {
                    rebuild = true;
                    free ( ssid );
                    ssid = 0;
                }
            }

            if ( rebuild ) {
                size_t len = strlen ( _ssid );

                ssid = ( char * ) malloc ( len + 1 );
                if ( !ssid ) {
                    CErrArg ( "Update: Failed to allocate memory [ %u bytes ]", len );
                    return;
                }

                memcpy ( ssid, _ssid, len );
                ssid [ len ] = 0;

				native.wifiObserver.itemsChanged = true;
            }
        }

		if ( _signal != signal ) {
			CVerbArg ( "Update: signal [ %i to %i ]", signal, _signal );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: signal [ %i to %i ]\n", signal, _signal );
#endif

			signal = _signal;
			native.wifiObserver.itemsChanged = true;
		}

        if ( _rssi != rssi ) {
			CVerbArg ( "Update: rssi [ %i to %i ]", rssi, _rssi );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: rssi [ %i to %i ]\n", rssi, _rssi );
#endif

            rssi = _rssi;
			native.wifiObserver.itemsChanged = true;
        }

        if ( _channel != channel ) {
			CVerbArg ( "Update: channel [ %i to %i ]", ( int ) channel, ( int ) _channel );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: channel [ %i to %i ]\n", ( int ) channel, ( int ) _channel );
#endif
			channel = _channel;
			native.wifiObserver.itemsChanged = true;
        }
    }


    bool WifiObserver::Init ()
    {
		CVerb ( "Init" );

#ifdef _WIN32
		if ( !InitLibWlanAPI () ) {
			native.useWifiObserver = false;
		}
#endif
        if ( !thread.Init () ) {
            CErr ( "Init: Failed to initialize wifiThread!" );
            return false;
        }
		thread.autoreset = true;

        if ( !LockInitA ( wifiObserverLock ) ) {
			CErr ( "Init: Failed to initialize lock!" );
			return false;
		}
        initialized = true;
        return true;
    }


#ifdef NATIVE_WIFI_OBSERVER_THREAD

    bool WifiObserver::Start ()
    {
		CVerb ( "Start" );

#if defined(_WIN32) && defined(USE_DYNAMIC_LIB_WLAN_API)
		if (!dWlanOpenHandle) {
			native.useWifiObserver = false;
			return true;
		}
#endif
        threadRun = true;

        if ( !thread.Run ( pthread_make_routine ( Thread_WifiObserverStarter ), 0, "WifiObserver.Start" ) )
        {
            CErr ( "Start: Failed to create thread for observing wifi!" );
            return false;
        }

        return true;
    }


    void WifiObserver::Stop ()
    {
        CVerb ( "Stop" );

		threadRun = false;

		thread.Notify ( "WifiObserver.Stop" );
		thread.Join ( "WifiObserver.Stop" );
    }


	void * Thread_WifiObserverStarter ( void * object )
	{
		if ( native.useWifiObserver )
			Thread_WifiObserver ();

		native.wifiObserver.thread.Detach ( "Thread_WifiObserverStarter" );
		return 0;
	}
#endif


    void WifiObserver::Begin ()
    {
		CVerbVerb ( "Begin" );

        seqNr++;
    }


    void WifiObserver::Finish ()
    {
		CVerbVerb ( "Finish" );

        LockAcquireA ( wifiObserverLock, "Finish" );

		std::map<unsigned long long, EnvWifiItem *>::iterator it = wifiItems.begin ();

		int minSeq = seqNr - 4;

		while ( it != wifiItems.end () )
		{
			EnvWifiItem * item = it->second;

			if ( item->seqNr < minSeq )
			{
				CVerbArg ( "Finish: Removing [ %s : %i : %i ]", item->ssid ? item->ssid : "???", item->rssi, ( int ) item->channel );

				delete item;
				wifiItems.erase ( it++ );

				itemsChanged = true;
			}
			else ++it;
		}

        LockReleaseA ( wifiObserverLock, "Finish" );
    }


    WifiObserver::~WifiObserver ()
    {
        CLog ( "Destruct" );

		if ( initialized ) {
			initialized = false;

			LockDisposeA ( wifiObserverLock );

			std::map<unsigned long long, EnvWifiItem *>::iterator it = wifiItems.begin ();

			while ( it != wifiItems.end () )
			{
				if ( it->second )
					delete it->second;
				++it;
			}

			wifiItems.clear ();

			ReleaseWlanAPI ();
		}
    }


    void WifiObserver::UpdateWithColonMac ( const char * _bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "UpdateWithColonMac" );

		if ( !_bssid || !*_bssid || strnlen ( _bssid, 17 ) < 17 )
			return;

		char hex [ 13 ];
		int  skip = 0;
		int  pos  = 0;

		for ( int i = 0; i < 17; ++i )
		{
			if ( skip == 2 ) {
				skip = 0; continue;
			}
			skip++;

			hex [ pos++ ] = _bssid [ i ];
		}

		hex [ 12 ] = 0;

		unsigned long long bssid = strtoull ( hex, NULL, 16 );

		WifiObserver::UpdateWithMac ( bssid, _ssid, _rssi, _signal, _channel, _encrypt );
    }


    void WifiObserver::UpdateWithMac ( unsigned long long bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "UpdateWithMac" );

		if ( !bssid )
			return;

		EnvWifiItem * item = 0;

		LockAcquireA ( wifiObserverLock, "UpdateWithMac" );

		std::map<unsigned long long, EnvWifiItem *>::iterator it = wifiItems.find ( bssid );

		if ( it == wifiItems.end () )
		{
			item = EnvWifiItem::Create ( seqNr, _ssid, _rssi, _signal, _channel, _encrypt );
			if ( item )
			{
				item->bssid = bssid;
				wifiItems [ bssid ] = item;

				item = 0;
			}
		}
		else {
			it->second->Update ( seqNr, _ssid, _rssi, _signal, _channel, _encrypt );
		}

		LockReleaseA ( wifiObserverLock, "UpdateWithMac" );

		if ( item )
			delete item;
    }

#endif
}

#endif
