/**
* Picture encoder using GDI
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
#ifndef INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_GDI_H
#define INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_GDI_H

#ifdef _WIN32

#include "Interfaces/IPortal.Encoder.h"

#pragma warning( push )
#pragma warning( disable: 4458 )
#include <gdiplus.h>
#pragma warning( pop )

namespace environs 
{
	/**
	*	Picture encoder using GDI
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@remarks
	* ****************************************************************************************
	*/
	class EncoderGDI : implements IPortalEncoder
	{
	public:
		EncoderGDI ( );
		~EncoderGDI ( );
	
		bool						Init ( int deviceID, int usePNG, int Width, int Height, int FrameRate );

		int							Perform ( RenderContext * context );

	private:
		bool						usePNG;

		unsigned int				keyframeCounter;
		bool						keyframeHandled;

		unsigned long				uQuality;
		Gdiplus::EncoderParameters	encoderParams;
		CLSID						clsid;

		static int					GetEncoderClsid ( const WCHAR * format, CLSID * pClsid );
	};


} /* namespace environs */

#endif

#endif	// INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_GDI_H