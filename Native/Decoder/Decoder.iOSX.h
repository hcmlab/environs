/**
 * Base codec using native iOSX
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

#ifndef INCLUDE_HCM_ENVIRONS_DECODER_IOSX
#define	INCLUDE_HCM_ENVIRONS_DECODER_IOSX

#import "Decoder.iOSX.H264.h"

#include "Decoder/Decoder.Base.h"


namespace environs {
	/**
	*	Base codec using native iOSX
	*	--------------------------------------------------------------------------------------
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/30/14
	*	@version	1.0
	*	@remarks	
	* ****************************************************************************************
	*/
	class DecoderIOSX : public IPortalDecoder
    {
	public:
		DecoderIOSX ();
		virtual ~DecoderIOSX ();
        
        int                 Init ( );        
        
        bool                InitType ( int type );

		/** Start the working thread that feeds the decoder and renders to the surface. */
		bool                Start ();

		/** Stop the working thread that feeds the decoder and renders to the surface. */
		void                Stop ();

		/** Dispose all acquired resources for the decoder and prepare the decoder for deletion. */
		void                Release ( );
        
        int                 AllocateResources ( );
        int                 ReleaseResources ( );
        
        int                 Perform ( int type, char * payload, int payloadSize );
        
        bool                SetRenderResolution ( int width, int height );
        
        /** Release the render surface. */
		virtual void        ReleaseRenderSurface ( bool useLock ) {};
        
        bool                SetRenderSurface ( void * penv, void * newSurface, int width, int height );
        
        virtual ptRenderCallback GetRenderCallback ( int &callbackType ) { return 0; };
        
    protected:

	private:
        bool                allocated;
        
        void            *   renderSurface;
        int                 renderWidth;
        int                 renderHeight;
        
        DecoderIOSXH264 *    decoder;
        IPortalDecoder *    idecoder;
        
	};

} /* namespace environs */


#endif /* INCLUDE_HCM_ENVIRONS_DECODER_IOS */
