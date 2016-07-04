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

#ifdef __cplusplus
namespace environs
{
#endif

#ifdef NATIVE_WIFI_OBSERVER

	class EnvWifiItem
	{
	public:
		int                 seqNr;
		bool                isConnected;
		char	*           ssid;
		unsigned long long  bssid;
		int                 rssi;
		int					signal;
		unsigned char       channel;
		unsigned char       encrypt;

		EnvWifiItem () { };

		EnvWifiItem ( int _seqNr, char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt )
			:
			seqNr ( _seqNr ), isConnected ( false ), ssid ( _ssid ), bssid ( 0 ),
			rssi ( _rssi ), signal ( _signal ), channel ( _channel ), encrypt ( _encrypt )
		{
		}

		~EnvWifiItem ()
		{
			if ( ssid ) {
				free ( ssid );
			}
		}

		void Update ( int _seqNr, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );

		static EnvWifiItem * Create ( int _seqNr, const char * _ssid, int _rssi, int _signal, unsigned char _channel, unsigned char _encrypt );
	};

	
    class WifiObserver
    {
    public:
        bool		initialized;
        int			seqNr;
		bool		threadRun;
		bool		itemsChanged;
		ThreadSync	thread;

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

    };

#endif
    
    
#ifdef __cplusplus
}
#endif

#endif