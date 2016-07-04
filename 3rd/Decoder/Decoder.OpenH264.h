/**
 * H264 Decoder using openh264
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

#ifndef INCLUDE_HCM_ENVIRONS_DECODER_OPENH264
#define	INCLUDE_HCM_ENVIRONS_DECODER_OPENH264

#include "Decoder/Decoder.Base.h"
#include "DynLib/Dyn.Lib.OpenH264.h"

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

namespace environs {
	/**
	*	H264 Decoder using openh264
	*	--------------------------------------------------------------------------------------
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/30/14
	*	@version	1.0
	*	@remarks	
	* ****************************************************************************************
	*/
	class DecoderOpenH264 : public DecoderBase
    {
	public:
		DecoderOpenH264 ( );
		virtual ~DecoderOpenH264 ( );

		/** Initialize the decoder. */
		bool            InitInstance ( bool useLock = true );

		/** Start the working thread that feeds the decoder and renders to the surface. */
		bool            Start ();

		/** Stop the working thread that feeds the decoder and renders to the surface. */
		void            Stop ();

		/** Dispose all acquired resources for the decoder and prepare the decoder for deletion. */
		void            Dispose ( bool useLock = true );

        virtual int		Perform ( int type, char * payload, int payloadSize );
        
        /** Release the render surface. */
		virtual void	ReleaseRenderSurface ( bool useLock ) {};
        
        virtual bool	SetRenderSurface ( void * penv, void * newSurface, unsigned int width, unsigned int height ) {return true;};
        
        virtual ptRenderCallback GetRenderCallback ( int &callbackType ) { return 0; };
        
    protected:
        ISVCDecoder		*	decoder;
        

	private:
        
	};

} /* namespace environs */

#endif

#endif /* INCLUDE_HCM_ENVIRONS_DECODER_OPENH264 */
