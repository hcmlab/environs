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

#ifndef CLI_CPP

#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Observer.h"
#include "Environs.Release.h"
#include "Environs.Native.h"
#include "Core/Kernel.h"

using namespace std;
using namespace environs::API;

#define CLASS_NAME	"Environs.Cpp . . . . . ."


namespace environs
{
	void EnvironsDisposer ( lib::IEnvironsDispose * obj )
	{
		if ( obj ) ((lib::IEnvironsDispose *) obj)->Release ();
	}

	namespace lib
	{
		class FactoryProxy
		{
		public:
			static Environs * Create ()
			{
				return new Environs ();
			}
			static void SetHandle ( Environs * inst, int hInst )
			{
				inst->SetInstance ( hInst );
			}
		};


		/// C function exported by the library
		ENVIRONSAPI LIBEXPORT void * CallConv ENVIRONS_CreateInstance1 ( int crt )
		{
			CVerb ( ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) );
#ifdef _WIN32
			if ( crt != ENVIRONS_BUILD_CRT ) {
				CWarnArg ( ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) ": Different versions of CRT [exe = %i] != [%i = lib]", crt, ENVIRONS_BUILD_CRT );
				//return 0;
			}
#endif
			int hEnvirons = environs::API::CreateEnvironsN ();
			if ( hEnvirons <= 0 )
				return 0;

			Environs * api = FactoryProxy::Create ();
			if ( api != nill ) {
				FactoryProxy::SetHandle ( api, hEnvirons );

                if ( api->Init () ) {
                    if ( api->Retain () )
                        return (void *) api;
                }

				delete api;
			}

			environs::API::DisposeN ( hEnvirons );
			return nill;
		}


#ifdef USE_ENVIRONS_LOG_POINTERS
        ENVIRONSAPI LIBEXPORT void ENVIRONS_GetLogMethods1 ( void ** outLog, void ** outLogArg )
        {
            if ( !outLog || !outLogArg )
                return;
            
            *outLog     = (void *) COutLog;
            *outLogArg  = (void *) COutArgLog;
        }
#endif

#ifdef _WIN32
		ENVIRONSAPI LIBEXPORT void PreDispose ( )
		{
			environs::API::PreDisposeN ();
		}
#endif
        

		bool Environs::InitPlatform ()
		{
			if ( !allocated ) 
			{
                environsObservers = make_shared < vct ( lib::IIEnvironsObserver * ) > (); // spv ( lib::IIEnvironsObserver * ) (new vector < lib::IIEnvironsObserver * >);
				if ( !environsObservers )
					return false;

				environsObserversForMessages = make_shared < vct ( lib::IIEnvironsMessageObserver * ) > (); //spv ( lib::IIEnvironsMessageObserver * ) (new vector < lib::IIEnvironsMessageObserver * >);
				if ( !environsObserversForMessages )
					return false;

                environsObserversForData = make_shared < vct ( lib::IIEnvironsDataObserver * ) > (); // spv ( lib::IIEnvironsDataObserver * ) (new vector < lib::IIEnvironsDataObserver * >);
				if ( !environsObserversForData )
					return false;

				environsObserversForSensor = make_shared < vct ( lib::IIEnvironsSensorObserver * ) > ();  //spv ( lib::IIEnvironsSensorDataObserver * ) (new vector < lib::IIEnvironsSensorDataObserver * >);
				if ( !environsObserversForSensor )
					return false;

				listAllObservers = make_shared < vector < lib::IIListObserver * > > ();
				if ( !listAllObservers )
					return false;

				listNearbyObservers = make_shared < vector < lib::IIListObserver * > > ();
				if ( !listNearbyObservers )
					return false;

				listMediatorObservers = make_shared < vector < lib::IIListObserver * > > ();
				if ( !listMediatorObservers )
					return false;
			}

			SetCallbacksN ( hEnvirons, 0, (void *) BridgeForUdpData, (void *) BridgeForMessage, (void *) BridgeForMessageExt, (void *) BridgeForNotify, (void *) BridgeForNotifyExt, (void *) BridgeForData, (void *) BridgeForStatusMessage );

			return true;
		}

	}


}

#endif



