/**
 * Header of video/avc, h264 stream encoding using openh264
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

#ifndef INCLUDE_HCM_ENVIRONS_ENCODER_OPENH264_H
#define INCLUDE_HCM_ENVIRONS_ENCODER_OPENH264_H

#include "Encoder/Encoder.Base.h"
#include "DynLib/Dyn.Lib.OpenH264.h"

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

namespace environs 
{
	/**
	*	H264 encoder
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class EncoderOpenH264 : implements EncoderBase
	{
	public:
		EncoderOpenH264 ();
		~EncoderOpenH264 ();
	
		bool				Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate );


		virtual bool		ApplyInput ();

		int					EncodeI420 ( char * yuvdata, char * &output, RenderContext * context );

		bool				enableExtendedOption;

	private:	
		ISVCEncoder		*	encoder;
		int					idr;

		void				Dispose ();
	};


} /* namespace environs */

#endif

#endif	/// -> INCLUDE_HCM_ENVIRONS_ENCODER_OPENH264_H
