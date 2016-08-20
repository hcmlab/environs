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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_WIFI_OBSERVER_H
#define INCLUDE_HCM_ENVIRONS_WIFI_OBSERVER_H

#include "Environs.Build.Opts.h"
#include "Interop/jni.h"

#ifdef __cplusplus
namespace environs
{
#endif
    extern unsigned long long GetBSSIDFromColonMac ( const char * _bssid );

    NET_PACK_PUSH1

    typedef struct WifiItem
    {
        unsigned long long  bssid;			// 8
        short               rssi;			// 2
        short				signal;			// 2
        unsigned char       channel;		// 1
        unsigned char       encrypt;		// 1
        bool                isConnected;	// 1
        unsigned char		sizeOfssid;		// 1
    }
    NET_PACK_ALIGN WifiItem;
    
    NET_PACK_POP


#ifdef NATIVE_WIFI_OBSERVER

	class EnvWifiItem
	{
	public:
		int                 seqNr;
		WifiItem			data;
		char	*           ssid;

        EnvWifiItem ();

		EnvWifiItem ( int _seqNr, unsigned long long  bssid, char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt )
			:
			seqNr ( _seqNr ), ssid ( _ssid )
		{
			data.bssid			= bssid;
			data.rssi			= ( short ) _rssi;
			data.signal			= ( short ) _signal;
			data.channel		= _channel;
			data.encrypt		= _encrypt;
			data.isConnected	= false;
			data.sizeOfssid		= 0;
		}

        ~EnvWifiItem ();

		void Update ( int _seqNr, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );

		static EnvWifiItem * Create ( int _seqNr, unsigned long long  bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );
	};

	
    class WifiObserver
    {
    public:
        bool		initialized;
        int			seqNr;
		bool		threadRun;
		bool		itemsChanged;
		ThreadSync	thread;
        EnvWifiItem we0;

        WifiObserver () : initialized ( false ), seqNr ( 0 ), threadRun ( false ), itemsChanged ( true )
        {
        }

        ~WifiObserver ();

        bool Init ();

#ifdef NATIVE_WIFI_OBSERVER_THREAD
        bool Start ();
        void Stop ();
#endif
        virtual void Begin ();
		virtual void Finish ();

        void UpdateWithColonMac ( const char * _bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );

		virtual void UpdateWithMac ( unsigned long long _bssid, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );

		jobject BuildNetData ( JNIEnv * jenv );
    };

#endif
    
    
#ifdef __cplusplus
}
#endif

#endif