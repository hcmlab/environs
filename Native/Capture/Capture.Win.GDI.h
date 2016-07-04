/**
* Windows GDI screen capture
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

#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_GDI_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_GDI_H

#ifdef _WIN32


namespace environs 
{
	/**
	*	Windows GDI screen capture
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class CaptureWinGDI : implements IPortalCapture
	{
	public:
		CaptureWinGDI ();
		~CaptureWinGDI ();

		/** Public Interface Methods */
		int					Init ( );
		void				Release ( );

		int                 ReleaseResources ( );
		int					AllocateResources ( RenderDimensions * dims );
        
		int					Perform ( RenderDimensions * dims, RenderContext * context );

		bool				MainThreadedInit ();
		bool				MainThreadedDispose ();

		int					AllocateOverlayBuffers ( RenderOverlay  * overlay );
		void                ReleaseOverlayBuffers ( RenderOverlay  * overlay );

	private:
		HDC					hDC;
		HDC					hAppWindowDC;
		HBITMAP				hBitmapCapturedOld;
	};


} /* namespace environs */

#endif

#endif	// INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_GDI_H