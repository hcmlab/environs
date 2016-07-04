/**
* External Wifi Observer Thread
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
#ifndef INCLUDE_HCM_ENVIRONS_EXT_WIFI_OBSERVER_THREAD_H
#define INCLUDE_HCM_ENVIRONS_EXT_WIFI_OBSERVER_THREAD_H

#include "Interfaces/IExt.Thread.h"


namespace environs 
{
	/**
	*	External Wifi Observer Thread
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class WifiObsThread : implements IExtThread
	{
	public:
		WifiObsThread ( void );
		virtual ~WifiObsThread ();

	private:

	public:
		/** Interface initializer. */
		int			Init ();

		bool		ThreadFunc ( void * obj );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_EXT_WIFI_OBSERVER_THREAD_H


