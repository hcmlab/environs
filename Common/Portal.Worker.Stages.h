/**
 * Environs Portal WorkerStages
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_WORKERSTAGES
#define	INCLUDE_HCM_ENVIRONS_PORTAL_WORKERSTAGES

#include "Interfaces/IPortal.Encoder.h"
#include "Interfaces/IPortal.Renderer.h"

namespace environs {
	/**
	*	Environs Portal WorkerStages
	*	--------------------------------------------------------------------------------------
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	
	* ****************************************************************************************
     */
    typedef struct _WorkerStages
    {
        IPortalCapture	*	capture;
        IPortalRenderer *	render;
        IPortalEncoder	*	encoder;
    }
    WorkerStages;

} /* namespace environs */


#endif /* INCLUDE_HCM_ENVIRONS_PORTAL_WORKERSTAGES */
