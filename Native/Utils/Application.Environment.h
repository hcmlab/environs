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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_APPLICATION_ENVIRONMENTS_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_APPLICATION_ENVIRONMENTS_H

#if (defined(CLI_CPP) && defined(CLI_PS))


namespace environs 
{
	/**
	*	Application.Environment
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	public ref class ApplicationEnvironment
	{
	public:
		ApplicationEnvironment ();

		int deviceID;
		int portalID;
		DeviceInstance ^ device;
		int updating;
		int lastUpdateTick;

	internal:
		static ApplicationEnvironment ^ get ( Environs ^ api, int deviceID );
		static ApplicationEnvironment ^ test ( Environs ^ api, int deviceID );
		static ApplicationEnvironment ^ pop ( Environs ^ api, int deviceID );

		static void UpdateAppEnv ( int hEnvirons, DeviceInstance ^ device );
		static void RemoveAppEnv ( int hEnvirons, DeviceInstance ^ device );
	};
}


#endif

#endif