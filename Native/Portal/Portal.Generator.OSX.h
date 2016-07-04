/**
* Portal Generator for the OSX platform
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_OSX_H
#define INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_OSX_H

#ifdef ENVIRONS_OSX

#include "Portal/Portal.Generator.h"


namespace environs 
{
	class DeviceController;

	/**
	*	A portal generator renders, encodes and sends a portal to a device
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class PortalGeneratorOSX : public PortalGenerator
	{
	public:
		virtual ~PortalGeneratorOSX ();

	private:
		bool                CreateWorkerStages (  WorkerStages * stages, int index );
		bool                CreatePictureStages ( WorkerStages * stages );
		bool                CreateStreamPreferredStages (  WorkerStages * stages );
		bool                CreateStreamFallbackStages ( WorkerStages * stages );

        IPortalCapture	*	CreateCapture ();
        IPortalRenderer	*	CreateCustomRenderer ();
	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_OSX_H
