/**
* GDI portal implementation
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_GDI_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_GDI_H

#ifdef _WIN32

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
	class RenderGDI : implements IPortalRenderer
	{
	public:
		RenderGDI ( void );
		virtual ~RenderGDI ( );

	private:
		Gdiplus::BitmapData			bitmapData;
		Gdiplus::Rect				rect;

		BYTE					*	lpPixelsCompareCache;
		unsigned int				sizePixelsCompareCache;


		//bool						ConnectOutput ( IEnvironsBase * dest );

	public:
		bool						Init ();
		void						Dispose ();

        int                         ReleaseResources ( RenderContext * context );
        int							AllocateResources ( RenderDimensions * dims );
        
        int							UpdateBuffers ( RenderDimensions * dims, RenderContext * context );

		bool						Compare ( unsigned int &equal );
		bool						Perform ( RenderDimensions * dims, RenderContext * context );

		char					*	GetBuffer ( RenderContext * context );
		void						ReleaseBuffer ( RenderContext * context );
	};

} /* namespace environs */

#endif


#endif // INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_GDI_H
