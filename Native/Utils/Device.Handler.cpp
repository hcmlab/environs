/**
* Environs CPP DeviceHandler
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
#include "Environs.h"
#include "Environs.Lib.h"
#include "Device/Device.List.Cli.h"
#include "Device/Device.Instance.Cli.h"
#include "Device.Handler.h"
#include "Environs.Native.h"

using namespace System::Threading;
using namespace System::Windows::Controls;

#define CLASS_NAME	"Device.Handler . . . . ."


namespace environs
{
	DeviceHandler::DeviceHandler ( Environs ^ a )
	{
		api = a;
		hEnvirons = api->hEnvirons;
	}


	/// <summary>
	/// Create a thread to handle the detected device id. Must be threaded since subsequent network communication with Mediator may happen.
	/// </summary>
	void DeviceHandler::detected ( int deviceID, int x, int y, float orientation )
	{
		CVerb ( "detected: ID [" + deviceID + "] at [" + x + "/" + y + "], orientation [" + orientation + "]" );

		ApplicationEnvironment ^ env = ApplicationEnvironment::get ( api, deviceID );
		if ( env != nill && env->device != nill )
			environs::API::DeviceDetectedN ( hEnvirons, deviceID, env->device->areaName, env->device->appName, CALL_NOWAIT, x, y, orientation );
	}


	/// <summary>
	/// A device has moved
	/// </summary>
	void DeviceHandler::moved ( int deviceID, int x, int y, float orientation )
	{
		//CVerb( "moved: ID [" + deviceID + "] at [" + x + "/" + y + "], orientation [" + orientation + "]");

		ApplicationEnvironment ^ env = ApplicationEnvironment::test ( api, deviceID );
		if ( env == nullptr || env->device == nullptr )
			return;

		int curTick = Environment::TickCount;
		if ( env->lastUpdateTick > 0 )
		{
			if ( (curTick - env->lastUpdateTick) < 66 )
				return;
		}
		env->lastUpdateTick = curTick;

		environs::API::DeviceUpdatedN ( hEnvirons, env->device->nativeID, env->portalID, CALL_NOWAIT, x, y, orientation );
	}


	void DeviceHandler::vanished ( int deviceID, int x, int y, double orientation )
	{
		CVerb ( "vanished: ID [" + deviceID + "] at [" + x + "/" + y + "], orientation [" + orientation + "]" );

		ApplicationEnvironment ^ env = ApplicationEnvironment::get ( api, deviceID );

		if ( env != nullptr && env->device != nullptr )
			environs::API::DeviceRemovedN ( hEnvirons, env->device->nativeID, env->portalID, CALL_NOWAIT, x, y, (float) orientation );
	}




	/// <summary>
	/// Device vanished..
	/// </summary>
	void DeviceHandler::remove ( int deviceID )
	{
		CVerb ( "remove: ID [" + deviceID + "]" );

		ApplicationEnvironment ^ env = ApplicationEnvironment::pop ( api, deviceID );

		if ( env != nullptr && env->device != nullptr )
			environs::API::DeviceDisconnectN ( hEnvirons, env->device->nativeID, CALL_NOWAIT );
	}
}


#endif