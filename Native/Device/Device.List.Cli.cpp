/**
 * DeviceList CLI Object
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Device.List.Cli.h"
#include "Environs.Native.h"

#define CLASS_NAME	"Device.List.Cli. . . . ."


namespace environs
{
	DeviceList::DeviceList ()
	{
		CVerb ( "Construct" );
	}


	DeviceList::~DeviceList ()
	{
		CVerbArg1 ( "Destruct", "", "i", objID_ );
	}

	namespace lib
	{
		ref class DeviceListSyncClass
		{
			sp ( DeviceListUpdatePack ) updatePacks;

		public:
			bool updated;

			DeviceListSyncClass ( sp ( DeviceListUpdatePack ) p ) : updated ( false ), updatePacks ( p ) { }

			void Run () {
				CVerbVerb ( "Run" );

				updated = DeviceList::DeviceListUpdateDataSourceSync ( updatePacks );
			}
		};


		bool DeviceList::DeviceListUpdateDispatchSync ( sp ( DeviceListUpdatePack ) updatePacks )
		{
			CVerbVerb ( "DeviceListUpdateDispatchSync" );

			DeviceListSyncClass ^ act = gcnew DeviceListSyncClass ( updatePacks );

			Action ^ action = gcnew Action ( act, &DeviceListSyncClass::Run );

			environs::Environs::dispatchSync ( action );

			return act->updated;
		}
	}

}


#endif // CLI_CPP







