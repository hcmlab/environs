/**
 * Bluetooth Observer
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
#include "Bt.Observer.h"
#include <stdlib.h>

#ifdef VS2010
#	define strtoull(a,b,c)	_strtoui64 ( a, b, c )
#endif

#define CLASS_NAME	"Bt.Observer. . . . . . ."

//#define BREAK_INTO_LEAKS

namespace environs
{

#ifdef NATIVE_BT_OBSERVER
	extern unsigned long long GetBSSIDFromColonMac ( const char * _bssid );

    /*
     * Bluetooth observer object
     *
     */
    pthread_mutex_t btObserverLock;

    void        *   Thread_BtObserverStarter ( void * object );
    void        *   Thread_BtObserver ();

    std::map<unsigned long long, EnvBtItem *> * envBluetoothItems = 0;



    EnvBtItem::EnvBtItem () : seqNr ( 0 ), ssid ( 0 )
    {
        data.bssid			= 0;
        data.rssi			= 0;
        data.isConnected	= false;
        data.sizeOfssid		= 0;
    }
    
    
    EnvBtItem::~EnvBtItem ()
    {
        if ( ssid ) {
            free ( ssid );
        }
    }
    
    
	EnvBtItem * EnvBtItem::Create ( int _seqNr, unsigned long long  bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2  )
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


		EnvBtItem * item = new EnvBtItem ( _seqNr, bssid, ssid, _rssi, cod, uuid1, uuid2 );

		if ( ssid ) {
			if ( !item )
				free ( ssid );
			else
				item->data.sizeOfssid = ( unsigned char ) ssidLen;
		}

		CVerbVerbArg ( "Create: [ %i dBm \t: %s ]", _rssi, ssid );
        
#if defined(DEBUGVERB) && defined(LINUX)
		printf ( "Create: [ %i dBm \t: %s ]\n", _rssi, ssid );
#endif
        return item;
    }


    void EnvBtItem::Update ( int _seqNr, const char * _ssid, int _rssi )
    {
		CVerbVerb ( "Update" );

        seqNr = _seqNr;

        if ( !_ssid || !*_ssid ) {
            if ( ssid ) {
                free ( ssid );
                ssid = 0;
				data.sizeOfssid = 0;
                native.btObserver.itemsChanged = true;
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

					native.btObserver.itemsChanged = true;
				}
            }
        }

        if ( _rssi < 42 && _rssi != ( int ) data.rssi ) {
			CVerbVerbArg ( "Update: rssi [ %i to %i ]", ( int ) data.rssi, _rssi );
            
#if defined(DEBUGVERB) && defined(LINUX) && !defined(ANDROID)
            printf ( "Update: rssi [ %i to %i ]\n", ( int ) data.rssi, _rssi );
#endif
			data.rssi = ( short ) _rssi;
			native.btObserver.itemsChanged = true;
        }
    }


    bool BtObserver::Init ()
    {
		CVerb ( "Init" );

        if ( !envBluetoothItems ) {
            envBluetoothItems = new std::map<unsigned long long, EnvBtItem *> ();
            
            if ( !envBluetoothItems ) {
                CErr ( "Init: Failed to initialize envBluetoothItems!" );
                return false;
            }
        }
        
        if ( !thread.Init () ) {
            CErr ( "Init: Failed to initialize btThread!" );
            return false;
        }
		thread.autoreset = true;

        if ( !LockInitA ( btObserverLock ) ) {
			CErr ( "Init: Failed to initialize lock!" );
			return false;
		}
        initialized = true;
        return true;
    }


#ifdef NATIVE_BT_OBSERVER_THREAD

    bool BtObserver::Start ()
    {
        CVerb ( "Start" );
        
        if ( !envBluetoothItems )
            return false;

        threadRun = true;

        if ( !thread.Run ( pthread_make_routine ( Thread_BtObserverStarter ), 0, "BtObserver.Start" ) )
        {
            CErr ( "Start: Failed to create thread for observing bluetooth!" );
            return false;
        }

        return true;
    }


    void BtObserver::Stop ()
    {
        CVerb ( "Stop" );

		threadRun = false;

		thread.Notify ( "BtObserver.Stop" );
		thread.Join ( "BtObserver.Stop" );
    }


	void * Thread_BtObserverStarter ( void * object )
	{
		if ( native.useBtObserver )
			Thread_BtObserver ();

		native.btObserver.thread.Detach ( "Thread_BtObserverStarter" );
		return 0;
	}
#endif


    void BtObserver::Begin ()
    {
		CVerbVerb ( "Begin" );

        seqNr++;
    }


    void BtObserver::Finish ()
    {
		CVerbVerb ( "Finish" );

        LockAcquireA ( btObserverLock, "Finish" );

		std::map<unsigned long long, EnvBtItem *>::iterator it = envBluetoothItems->begin ();

		int minSeq = seqNr - 256;

		while ( it != envBluetoothItems->end () )
		{
			EnvBtItem * item = it->second;

			if ( item->seqNr < minSeq )
			{
				CVerbVerbArg ( "Finish: Removing [ %s : %i ]", item->ssid ? item->ssid : "???", item->data.rssi  );

				delete item;
				envBluetoothItems->erase ( it++ );

				itemsChanged = true;
			}
			else ++it;
		}

        LockReleaseA ( btObserverLock, "Finish" );
    }


	BtObserver::~BtObserver ()
    {
        CLog ( "Destruct" );

		if ( initialized ) {
			initialized = false;

			LockDisposeA ( btObserverLock );

            if ( envBluetoothItems )
            {
                std::map<unsigned long long, EnvBtItem *>::iterator it = envBluetoothItems->begin ();
                
                while ( it != envBluetoothItems->end () )
                {
                    if ( it->second )
                        delete it->second;
                    ++it;
                }
                
                envBluetoothItems->clear ();
                
                delete envBluetoothItems; envBluetoothItems = 0;
            }
		}
    }


    void BtObserver::UpdateWithColonMac ( const char * _bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 )
    {
		CVerbVerb ( "UpdateWithColonMac" );

		UpdateWithMac ( GetBSSIDFromColonMac ( _bssid ), _ssid, _rssi, cod, uuid1, uuid2 );
    }


    void BtObserver::UpdateWithMac ( unsigned long long bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 )
    {
		CVerbVerb ( "UpdateWithMac" );

		if ( !bssid )
			return;

		EnvBtItem * item = 0;

		LockAcquireA ( btObserverLock, "UpdateWithMac" );

        if ( _rssi == 42 ) {
            we0.Update ( seqNr, _ssid, 42 ); we0.data.cod = cod; we0.data.luuid.data[0] = uuid1; we0.data.luuid.data[1] = uuid2;
        }
        else {
            std::map<unsigned long long, EnvBtItem *>::iterator it = envBluetoothItems->find ( bssid ? bssid : uuid1 );

            if ( it == envBluetoothItems->end () )
            {
                item = EnvBtItem::Create ( seqNr, bssid, _ssid, _rssi, cod, uuid1, uuid2 );
                if ( item )
                {
                    item->data.bssid = bssid;
                    (*envBluetoothItems) [ bssid ? bssid : uuid1 ] = item;
                    
                    item = 0;
                }
            }
            else {
                it->second->Update ( seqNr, _ssid, _rssi );
            }
        }
        
        LockReleaseA ( btObserverLock, "UpdateWithMac" );

		if ( item )
			delete item;
    }


	jobject BtObserver::BuildNetData ( JNIEnv * jenv )
	{
		unsigned int	bufSize, items, itemCount = 0;
		char		  * data	= 0;

		// Calculate size

		if ( !LockAcquireA ( btObserverLock, "BuildNetData" ) )
			return 0;

		items = ( unsigned int ) envBluetoothItems->size ();
		if ( items > 0 )
		{
			if ( items > 256 )
				items = 256;

			const std::map<unsigned long long, EnvBtItem *>::iterator end = envBluetoothItems->end ();
			std::map<unsigned long long, EnvBtItem *>::iterator it	= envBluetoothItems->begin ();

			bufSize = ( unsigned int ) ( items * sizeof ( BtItem ) );

			while ( it != end && itemCount < items )
			{
				bufSize += ( unsigned int ) it->second->data.sizeOfssid;
				++itemCount;
				++it;
			}

			bufSize += sizeof ( BtItem ) + 40;

			jobject byteBuffer = allocJByteBuffer ( jenv, bufSize, data );
			if ( !data ) {
				CErrArg ( "BuildNetData: Failed to allocate buffer of size [ %d ]!", bufSize );
			}
			else {
				unsigned int remainSize = bufSize; itemCount = 0;

				it	= envBluetoothItems->begin ();

				char * cur = data + 8;

				while ( it != end && itemCount < items )
				{
					EnvBtItem * item = it->second;

					unsigned int ssidSize = ( unsigned int ) item->data.sizeOfssid;

					unsigned int min = ( unsigned int ) ( sizeof ( BtItem ) + ssidSize );

					if ( min > remainSize )
						break;
					BtItem * decls = ( BtItem * ) cur;

					memcpy ( decls, &item->data, sizeof ( BtItem ) );

					cur += sizeof ( BtItem );

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

				LockReleaseA ( btObserverLock, "BuildNetData" );

				unsigned int * sizes = reinterpret_cast< unsigned int * > ( data );

				*sizes = itemCount;  *( sizes + 1 ) = ( bufSize - remainSize );

				return byteBuffer;
			}
		}

		LockReleaseA ( btObserverLock, "BuildNetData" );
		
		return 0;
	}

#endif
}

#endif
