/**
* Null renderer
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_NULL_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_NULL_H

#include "Interfaces/IPortal.Renderer.h"


namespace environs 
{
	/**
	*	Null renderer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class RenderNull : implements IPortalRenderer
	{
	public:
		RenderNull ( void );
		virtual ~RenderNull ();

	private:

	public:
		bool						Init ();
		void						Dispose ();

        int                         ReleaseResources ( RenderContext * context );
        int							AllocateResources ( RenderDimensions * dims );
        
		int							UpdateBuffers ( RenderDimensions * dims, RenderContext * context );
		char					*	GetBuffer ( RenderContext * context );

		bool						Compare ( unsigned int &equal );
		bool						Perform ( RenderDimensions * dims, RenderContext * context );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTAL_RENDER_NULL_H


