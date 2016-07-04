/**
* Windows GDI screen capture class-threaded
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

#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_TGDI_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_TGDI_H

#ifdef _WIN32

#define USE_BACK_BUFFERING		2


namespace environs 
{
	/**
	*	Windows GDI screen capture class-threaded
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class CaptureWinTGDI : implements IPortalCapture
	{
	public:
		CaptureWinTGDI ();
		~CaptureWinTGDI ();
		
		/** Public Interface Methods */
		int							Init ( );
		void						Release ( );

		int                         ReleaseResources ( );
		int							AllocateResources ( RenderDimensions * dims );

		int							Perform ( RenderDimensions * dims, RenderContext * context );

	protected:
		static LONGSYNC				classCaptureAccessed;

	private:
		bool						didAddRef;

		HDC							hDC;
		HBITMAP						hBitmapCapturedOld;

		static LONGSYNC				referenceCount;
			
		static bool					ClassInit ( int deviceID );
		static void					ClassDispose ( int deviceID );

		static void *				Thread_Grab ( void * arg );

	};


} /* namespace environs */

#endif

#endif	// INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_TGDI_H