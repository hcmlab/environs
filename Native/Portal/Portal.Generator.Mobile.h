/**
 *	Platform Portal Generator
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_PLATFORM_H
#define INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_PLATFORM_H

#ifndef DISPLAYDEVICE

#include "Portal/Portal.Generator.h"


namespace environs 
{
	/**
	*	A portal generator renders, encodes and sends a portal to a device
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	* ****************************************************************************************
	*/
	class PortalGeneratorBase : public PortalGenerator
	{
	public:
        
    private:
        bool                CreateWorkerStages (  WorkerStages * stages, int index );
        bool                CreateWorkerStagesPicture (  WorkerStages * stages );
        bool                CreateCameraStages (  WorkerStages * stages, int cameraID );

        virtual IPortalCapture	* GetCameraInstance ( int cameraID ) = 0;
        virtual IPortalEncoder	* GetEncoderInstance ( int deviceID, int index ) = 0;
        
        IPortalCapture	*	CreateCameraCapture ( int cameraID );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_BASE_H

#endif