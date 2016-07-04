/**
 *	Portal Generator for iOS devices
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_IOS_H
#define INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_IOS_H

#if !defined(DISPLAYDEVICE) && defined(__APPLE__)

#include "Portal/Portal.Generator.Mobile.h"


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
	class PortalGeneratorMobile : public PortalGeneratorBase
	{
	public:
        
    private:
        IPortalCapture	* GetCameraInstance ( int cameraID );
        IPortalEncoder	* GetEncoderInstance ( int deviceID, int index );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_IOS_H

#endif