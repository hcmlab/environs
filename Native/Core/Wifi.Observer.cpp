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
//#   define DEBUGVERB
//#   define DEBUGVERBVerb

#include "Environs.Native.h"
#include "Interop/Threads.h"
#include <map>
#include "Environs.Obj.h"
#include "Wifi.Observer.h"
#include "DynLib/Dyn.WlanAPI.h"

#if !defined(WINDOWS_PHONE) && !defined(_WIN32)
#   include <stdlib.h>
#endif

#ifdef VS2010
#	define strtoull(a,b,c)	_strtoui64 ( a, b, c )
#endif

#define CLASS_NAME	"Wifi.Observer. . . . . ."

//#define BREAK_INTO_LEAKS

namespace environs
{
    unsigned long long GetBSSIDFromColonMac ( const char * _bssid )
    {
        if ( !_bssid || !*_bssid )
            return 0;

        int len = ( int ) strnlen ( _bssid, 17 );
        if ( len < 5 )
            return 0;

        char hex [ 13 ];
        int  pos    = 0;
        int  count  = 0;
        int  js     = 0;
        const char * cur = _bssid;

        for ( int i = 0; i < len; ++i )
        {
            char c = *cur; cur++;
            if ( !c )
                break;

            if ( c == ':' ) {
                if ( js == 1 ) {
                    hex [ pos ] = hex [ pos - 1 ]; hex [ pos - 1 ] = '0';
                }
                else if ( js == 0 ) {
                    hex [ pos ] = hex [ pos + 1 ] = '0'; pos += 2;
                }
                js = 0; count++; if ( count > 5 ) break;
                continue;
            }

            hex [ pos++ ] = c; js++;
        }

        hex [ pos ] = 0;
        
        return strtoull ( hex, NULL, 16 );
    }

#ifdef NATIVE_WIFI_OBSERVER


    /*
     * Wifi observer object
     *
     */

    pthread_mutex_t wifiObserverLock;

    void        *   Thread_WifiObserverStarter ( void * object );
    void        *   Thread_WifiObserver ();

    std::map<unsigned long long, EnvWifiItem *> * envWifiItems = 0;


    EnvWifiItem::EnvWifiItem () : seqNr ( 0 ), ssid ( 0 )
    {
        data.bssid			= 0;
        data.rssi			= 0;
        data.signal			= 0;
        data.channel		= 0;
        data.encrypt		= 0;
        data.isConnected	= 0;
        data.sizeOfssid		= 0;
    }
    
    
    EnvWifiItem::~EnvWifiItem ()
    {
        if ( ssid ) {
            free ( ssid );
        }
    }
    
    
    EnvWifiItem * EnvWifiItem::Create ( int _seqNr, unsigned long long  bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "Create" );

		char * ssid = 0; size_t ssidLen = 0;

		if ( _ssid && *_ssid )
		{
			size_t len = strnlen ( _ssid, 64 );
			if ( len ) {
				ssidLen = len + 1;

				unsigned int rest = ssidLen % 4;
				if ( rest ) {
					ssidLen += 4 - rest;
				}

				ssid = ( char * ) malloc ( ssidLen );
				if ( !ssid ) {
					CErrArg ( "Create: Failed to allocate memory [ %u bytes ]", ssidLen );
					return 0;
				}

				memcpy ( ssid, _ssid, len );
				ssid [ len ] = 0;
			}
		}


        EnvWifiItem * item = new EnvWifiItem ( _seqNr, bssid, ssid, _rssi, _signal, _channel, _encrypt );

		if ( ssid ) {
			if ( !item )
				free ( ssid );
			else
				item->data.sizeOfssid = ( unsigned char ) ssidLen;
		}


		CVerbVerbArg ( "Create: [ %i dBm \t: C%i \t: E%i \t: %s ]", _rssi, ( int ) _channel, ( int ) _encrypt, ssid );
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
				data.sizeOfssid = 0;
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
					data.sizeOfssid = 0;
                }
            }

            if ( rebuild ) {
                size_t len = strlen ( _ssid );

				if ( len > 0 ) {
					size_t ssidLen = len + 1;

					unsigned int rest = ssidLen % 4;
					if ( rest ) {
						ssidLen += 4 - rest;
					}

					ssid = ( char * ) malloc ( ssidLen );
					if ( !ssid ) {
						CErrArg ( "Update: Failed to allocate memory [ %u bytes ]", ssidLen );
						return;
					}

					memcpy ( ssid, _ssid, len );
					ssid [ len ] = 0;
					data.sizeOfssid = ( unsigned char ) ssidLen;

					native.wifiObserver.itemsChanged = true;
				}
            }
        }

		if ( _signal != ( int ) data.signal ) {
			CVerbVerbArg ( "Update: signal [ %i to %i ]", ( int ) data.signal, _signal );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: signal [ %i to %i ]\n", ( int ) data.signal, _signal );
#endif

			data.signal = ( short ) _signal;
			native.wifiObserver.itemsChanged = true;
		}

        if ( _rssi < 42 && _rssi != ( int ) data.rssi ) {
			CVerbVerbArg ( "Update: rssi [ %i to %i ]", ( int ) data.rssi, _rssi );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: rssi [ %i to %i ]\n", ( int ) data.rssi, _rssi );
#endif

			data.rssi = ( short ) _rssi;
			native.wifiObserver.itemsChanged = true;
        }

        if ( _channel != data.channel ) {
			CVerbVerbArg ( "Update: channel [ %i to %i ]", ( int ) data.channel, ( int ) _channel );
#if defined(DEBUGVERB) && defined(LINUX)
            printf ( "Update: channel [ %i to %i ]\n", ( int ) data.channel, ( int ) _channel );
#endif
			data.channel = _channel;
			native.wifiObserver.itemsChanged = true;
        }
    }


    bool WifiObserver::Init ()
    {
        CVerb ( "Init" );
        
        if ( !envWifiItems ) {
            envWifiItems = new std::map<unsigned long long, EnvWifiItem *> ();
            
            if ( !envWifiItems ) {
                CErr ( "Init: Failed to initialize envWifiItems!" );
                return false;
            }
        }

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
        
        if ( !envWifiItems )
            return false;

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

		std::map<unsigned long long, EnvWifiItem *>::iterator it = envWifiItems->begin ();

		int minSeq = seqNr - 4;

		while ( it != envWifiItems->end () )
		{
			EnvWifiItem * item = it->second;

			if ( item->seqNr < minSeq )
			{
				CVerbVerbArg ( "Finish: Removing [ %s : %i : %i ]", item->ssid ? item->ssid : "???", item->data.rssi, ( int ) item->data.channel );

				delete item;
				envWifiItems->erase ( it++ );

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

            if ( envWifiItems )
            {
                std::map<unsigned long long, EnvWifiItem *>::iterator it = envWifiItems->begin ();
                
                while ( it != envWifiItems->end () )
                {
                    if ( it->second )
                        delete it->second;
                    ++it;
                }
                
                envWifiItems->clear ();
                
                delete envWifiItems; envWifiItems = 0;
            }

			ReleaseWlanAPI ();
		}
    }


    void WifiObserver::UpdateWithColonMac ( const char * _bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "UpdateWithColonMac" );

		UpdateWithMac ( GetBSSIDFromColonMac ( _bssid ), _ssid, _rssi, _signal, _channel, _encrypt );
    }


    void WifiObserver::UpdateWithMac ( unsigned long long bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt  )
    {
		CVerbVerb ( "UpdateWithMac" );

		if ( !bssid )
			return;

		EnvWifiItem * item = 0;

		LockAcquireA ( wifiObserverLock, "UpdateWithMac" );

        if ( _rssi == 42 ) {
            we0.Update ( seqNr, _ssid, 42, 0, 0, 0 );
            we0.data.bssid = bssid;
        }
        else {
            std::map<unsigned long long, EnvWifiItem *>::iterator it = envWifiItems->find ( bssid );

            if ( it == envWifiItems->end () )
            {
                item = EnvWifiItem::Create ( seqNr, bssid, _ssid, _rssi, _signal, _channel, _encrypt );
                if ( item )
                {
                    (*envWifiItems) [ bssid ] = item;

                    item = 0;
                }
            }
            else {
                it->second->Update ( seqNr, _ssid, _rssi, _signal, _channel, _encrypt );
            }
        }
        
        LockReleaseA ( wifiObserverLock, "UpdateWithMac" );

		if ( item )
			delete item;
    }


	jobject  WifiObserver::BuildNetData ( JNIEnv * jenv )
	{
		unsigned int	bufSize, items, itemCount = 0;
		char		  * data	= 0;

		// Calculate size

		if ( !LockAcquireA ( wifiObserverLock, "BuildNetData" ) )
			return 0;

		items = ( unsigned int ) envWifiItems->size ();
		if ( items > 0 )
		{
			if ( items > 256 )
				items = 256;

			const std::map<unsigned long long, EnvWifiItem *>::iterator end = envWifiItems->end ();
			std::map<unsigned long long, EnvWifiItem *>::iterator it	= envWifiItems->begin ();

			bufSize = ( unsigned int ) ( items * sizeof ( WifiItem ) );

			while ( it != end && itemCount < items )
			{
				bufSize += ( unsigned int ) it->second->data.sizeOfssid;
				++itemCount;
				++it;
			}

			bufSize += sizeof ( WifiItem ) + 40;

			jobject byteBuffer = allocJByteBuffer ( jenv, bufSize, data );
			if ( !data ) {
				CErrArg ( "BuildNetData: Failed to allocate buffer of size [ %d ]!", bufSize );
			}
			else {
				unsigned int remainSize = bufSize; itemCount = 0;

				it	= envWifiItems->begin ();

				char * cur = data + 8;

				while ( it != end && itemCount < items )
				{
					EnvWifiItem * item = it->second;

					unsigned int ssidSize = ( unsigned int ) item->data.sizeOfssid;

					unsigned int min = ( unsigned int ) ( sizeof ( WifiItem ) + ssidSize );

					if ( min > remainSize )
						break;
					WifiItem * decls = ( WifiItem * ) cur;

					memcpy ( decls, &item->data, sizeof ( WifiItem ) );

					cur += sizeof ( WifiItem );

					if ( ssidSize ) {
						memcpy ( cur, item->ssid, ssidSize );

						cur [ ssidSize - 1 ] = 0;
						cur += ssidSize;
					}
					else
						*cur = 0;

					remainSize -= min;

					++itemCount;
					++it;
				}

				LockReleaseA ( wifiObserverLock, "BuildNetData" );

				unsigned int * sizes = reinterpret_cast< unsigned int * > ( data );

				*sizes = itemCount;  *( sizes + 1 ) = (  bufSize - remainSize );

				return byteBuffer;
			}
		}

		LockReleaseA ( wifiObserverLock, "BuildNetData" );

		return 0;
	}

#endif
}

#endif
