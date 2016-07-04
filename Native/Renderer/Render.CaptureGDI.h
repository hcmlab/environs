/**
* GDI portal implementation (a combined capture / render)
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_CAPTURE_GDI_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_CAPTURE_GDI_H

#include "Interfaces/IPortal.Renderer.h"

#pragma warning( push )
#pragma warning( disable: 4458 )
#include <gdiplus.h>
#pragma warning( pop )
using namespace Gdiplus;


namespace environs 
{
	/**
	*	GDI portal implementation
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class RenderCaptureGDI : implements IPortalRenderer
	{
	public:
		RenderCaptureGDI ( void );
		virtual ~RenderCaptureGDI ( );

	private:
		HDC							hDC;
		HDC							hAppWindowDC;
		HBITMAP						hBitmapCaptured;
		HBITMAP						hBitmapCapturedOld;
		Gdiplus::BitmapData			bitmapData;
		Gdiplus::Rect				rect;

		BYTE					*	lpPixelsCache;
		unsigned int				sizePixelsCache;
		BYTE					*	lpPixelsCompareCache;
		unsigned int				sizePixelsCompareCache;
		
		bool						MainThreadedInit ();
		bool						MainThreadedDispose ();

		bool						CompareBitmaps ( HDC hCompareDC, HBITMAP hBitmap );

		//bool						ConnectOutput ( IEnvironsBase * dest );

	public:
		bool						Init ();
		void						Dispose ();

		int                         ReleaseResources ( RenderContext * context );
        int							AllocateResources ( RenderDimensions * dims );
        
        int							UpdateBuffers ( RenderDimensions * dims, RenderContext * context );

		bool						Perform ( RenderDimensions * dims, RenderContext * context );

		char					*	GetBuffer ( RenderContext * context );
		void						ReleaseBuffer ( RenderContext * context );
	};

} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_CAPTURE_GDI_H
