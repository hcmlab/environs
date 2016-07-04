/**
 * Header of jpeg/png encoding
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
#ifndef INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_H
#define INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_H

#include "Interfaces/IPortal.Encoder.h"

namespace environs 
{
	/**
	*	Picture encoder
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@remarks
	* ****************************************************************************************
	*/
	class EncoderPictures : implements IPortalEncoder
	{
	public:
		EncoderPictures ();
		~EncoderPictures ();
	
		bool Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate );

		int Perform ( RenderContext * context );


	private:
	};


} /* namespace environs */


#endif	// INCLUDE_HCM_ENVIRONS_ENVIRONS_PICTURES_H