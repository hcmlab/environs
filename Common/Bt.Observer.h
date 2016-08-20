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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_BT_OBSERVER_H
#define INCLUDE_HCM_ENVIRONS_BT_OBSERVER_H

#include "Environs.Build.Opts.h"
#include "Interop/jni.h"

#ifdef __cplusplus
namespace environs
{
#endif
    NET_PACK_PUSH1

    typedef struct BtKey
    {
        unsigned long long  data [ 2 ];
    }
    BtKey;

    typedef struct BtItem
    {
		// MAC, physical address
        unsigned long long  bssid;			// 8
		// Signal strength in dB
        short               rssi;			// 2
		// Determines whether we are connected or not
        bool                isConnected;	// 1
        unsigned char		sizeOfssid;		// 1
		// class of device
        int                 cod;            // 4
        union
        {
			// uuid of an iBeacon
            char                uuid [ 16 ];    // 16
            BtKey               luuid;
        };
    }
    NET_PACK_ALIGN BtItem;
    
    NET_PACK_POP


#ifdef NATIVE_BT_OBSERVER

	class EnvBtItem
	{
	public:
		int                 seqNr;
		BtItem				data;
		char	*           ssid;

        EnvBtItem ();

		EnvBtItem ( int _seqNr, unsigned long long  bssid, char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 )
			:
			seqNr ( _seqNr ), ssid ( _ssid )
		{
			data.bssid              = bssid;
			data.rssi               = ( short ) _rssi;
			data.isConnected        = false;
            data.sizeOfssid         = 0;
            data.cod                = cod;
            data.luuid.data [ 0 ]   = uuid1 ? uuid1 : bssid;
            data.luuid.data [ 1 ]   = uuid2;
		}

        ~EnvBtItem ();

		void Update ( int _seqNr, const char * _ssid, int _rssi );

		static EnvBtItem * Create ( int _seqNr, unsigned long long  bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 );
	};

	
    class BtObserver
    {
    public:
        bool		initialized;
        int			seqNr;
		bool		threadRun;
		bool		itemsChanged;
		ThreadSync	thread;

        EnvBtItem      we0;
        EnvBtItem      we1;

		BtObserver () : initialized ( false ), seqNr ( 0 ), threadRun ( false ), itemsChanged ( true )
        {
        }

        ~BtObserver ();

        bool Init ();

#ifdef NATIVE_BT_OBSERVER_THREAD
        bool Start ();
        void Stop ();
#endif
        virtual void Begin ();
		virtual void Finish ();

        void UpdateWithColonMac ( const char * _bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 );

		virtual void UpdateWithMac ( unsigned long long _bssid, const char * _ssid, int _rssi, int cod, unsigned long long uuid1, unsigned long long uuid2 );

		jobject BuildNetData ( JNIEnv * jenv );
    };

#endif
    
    
#ifdef __cplusplus
}
#endif

#endif