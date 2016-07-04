/**
* Environs CPP ApplicationEnvironment
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

#if (defined(CLI_CPP) && defined(CLI_PS))

#include "Interop.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Device/Device.List.Cli.h"
#include "Device/Device.Instance.Cli.h"
#include "Application.Environment.h"
#include "Environs.Native.h"

using namespace System::Threading;
using namespace System::Windows::Controls;

#define	CLASS_NAME 	"Application.Environment."


namespace environs
{
	ApplicationEnvironment::ApplicationEnvironment ()
	{
		deviceID = 0;
		updating = 0;
		lastUpdateTick = 0;
		device = nullptr;
	}


	void ApplicationEnvironment::UpdateAppEnv ( int hEnvirons, DeviceInstance ^ dev )
	{
		if ( dev != nill )
		{
			Environs ^ api = Environs::instancesAPI [ hEnvirons ];
			if ( api == nill )
				return;

			ApplicationEnvironment ^ env = get ( api, dev->deviceID );
			if ( env != nill )
				env->device = dev;
		}
	}


	void ApplicationEnvironment::RemoveAppEnv ( int hEnvirons, DeviceInstance ^ dev )
	{
		if ( dev == nill )
			return;

		Environs ^ api = Environs::instancesAPI [ hEnvirons ];
		if ( api == nill )
			return;

		pop ( api, dev->deviceID );
	}


	/***
	* Search the cache map for the environment configuration corresponding to the device id
	**/
	ApplicationEnvironment ^ ApplicationEnvironment::get ( Environs ^ api, int deviceID )
	{
		if ( api == nullptr )
			return nullptr;

		Monitor::Enter ( api );

		ApplicationEnvironment ^ env = nullptr;

		if ( !api->applicationEnvironments->TryGetValue ( deviceID, env ) )
		{
			env = gcnew ApplicationEnvironment ();
			env->deviceID = deviceID;

			api->applicationEnvironments->Add ( deviceID, env );

			api->GetDeviceByDeviceID ( env->device, deviceID );
		}

		Monitor::Exit ( api );
		return env;
	}


	/***
	* Search the cache map for the environment configuration corresponding to the device id
	* However, don't block the calling thread.
	**/
	ApplicationEnvironment ^ ApplicationEnvironment::test ( Environs ^ api, int deviceID )
	{
		if ( api == nullptr )
			return nullptr;

		ApplicationEnvironment ^ env = nullptr;

		api->applicationEnvironments->TryGetValue ( deviceID, env );

		return env;
	}


	/***
	* Search the cache map for the environment configuration corresponding to the device id
	* However, don't block the calling thread.
	**/
	ApplicationEnvironment ^ ApplicationEnvironment::pop ( Environs ^ api, int deviceID )
	{
		if ( api == nullptr )
			return nullptr;

		ApplicationEnvironment ^ env = nullptr;

		api->applicationEnvironments->TryGetValue ( deviceID, env );

		if ( env != nullptr )
			api->applicationEnvironments->Remove ( deviceID );
		return env;
	}
}


#endif