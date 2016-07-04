/**
 * Base camera capture module
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
#include "Interfaces/IPortal.Capture.h"
#include "Core/Byte.Buffer.h"

#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_CAM_BASE_MODULE_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_CAM_BASE_MODULE_H

namespace environs 
{
	/**
	*	Base camera capture module
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class CamBase : implements IPortalCapture
	{
	public:
		CamBase ( );
		~CamBase ( );

		virtual int			ReleaseResources ( );
		virtual int			AllocateResources ( RenderDimensions * dims );

		int					Perform ( RenderDimensions * dims, RenderContext * context );
		
        bool				PerformBase ( const char * payload, unsigned int payloadSize );

        bool                PerformIOSX ( void * cvSampleBuffer );

		bool				AllocateSwapBuffers ();
		void				ReleaseSwapBuffers ();

	private:
        bool                dataBaseAllocated;


	protected:
	};


} /* namespace environs */


#endif	// INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_CAM_BASE_MODULE_H


