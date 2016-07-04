/**
 * A portal receiver receives portal stream packages, decodes them
   and dispatches them to the renderer
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_ANDROID_H
#define INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_ANDROID_H

#ifdef ANDROID

#include "Portal.Receiver.h"


namespace environs 
{
	/**
	*	A portal receiver receives portal stream packages, decodes them and dispatches them to the renderer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class PortalReceiverAndroid : public PortalReceiver
	{
	public:

	private:
		int	CreateDecoder ( );
	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_ANDROID_H
