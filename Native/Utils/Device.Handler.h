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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_DEVICE_HANDLER_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_DEVICE_HANDLER_H

#if (defined(CLI_CPP) && defined(CLI_PS))

#include "Utils/Application.Environment.h"

namespace environs 
{
	/**
	*	Device.Handler
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	public ref class DeviceHandler
	{
	public:
		DeviceHandler ( Environs ^ api );

	internal:
		Environs ^ api;
		int hEnvirons;

		/***
		* A lock object to safely access applicationEnvironments
		**/
		static Object ^ classLock = gcnew Object ();

		/***
		* A dictionary to map device ids to their environment configuration
		**/
		static System::Collections::Generic::Dictionary<int, ApplicationEnvironment ^ > ^ applicationEnvironments = gcnew System::Collections::Generic::Dictionary<int, ApplicationEnvironment ^ > ();

		/// <summary>
		/// Create a thread to handle the detected device id. Must be threaded since subsequent network communication with Mediator may happen.
		/// </summary>
		void detected ( int deviceID, int x, int y, float orientation );

		/// <summary>
		/// A device has moved
		/// </summary>
		void moved ( int deviceID, int x, int y, float orientation );


		void vanished ( int deviceID, int x, int y, double orientation );


		/// <summary>
		/// Device vanished..
		/// </summary>
		void remove ( int deviceID );

	};
}


#endif

#endif