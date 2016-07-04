/**
 * Base implementation for a portal encoder/compressor
   (create transport packages)
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
#include "Interfaces/IPortal.Encoder.h"

#ifndef INCLUDE_HCM_ENVIRONS_ENCODER_BASE_IMPLEMENTATION_H
#define INCLUDE_HCM_ENVIRONS_ENCODER_BASE_IMPLEMENTATION_H


namespace environs 
{
	class EncoderBase;

	typedef int (EncoderBase::*ProcessorHandler)(char * source, char * &output, RenderContext * context);

	/**
	*	Base video implementation for a portal encoder/compressor
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class EncoderBase : public IPortalEncoder
	{

	public:
		/**
		* Default constructor
		*	@comment:
		*/
		EncoderBase ();

		virtual ~EncoderBase ();

		virtual bool		ApplyInput ();


		unsigned int		cacheCount;
        char			*	yuvBuffer;
        unsigned int		yuvBufferSize;
        char			*	uCache;
        char			*	vCache;


		// 0 means encoded, but do not send the data; 1 means encoded and new data to transmit, -1 means failed to encode.
		virtual int			Perform ( RenderContext * context );

		virtual int			EncodeARGB ( char * source, char * &output, RenderContext * context );
		virtual int			EncodeBGRA ( char * source, char * &output, RenderContext * context );
		virtual int			EncodeYV12 ( char * source, char * &output, RenderContext * context );
		virtual int			EncodeYUY2 ( char * source, char * &output, RenderContext * context );
		virtual int			EncodeGDIBitmap ( char * source, char * &output, RenderContext * context );
		virtual int			EncodeI420 ( char * yuvdata, char * &output, RenderContext * context ) { return 0; };

	protected:
		ProcessorHandler 	processor;
        
        void                DisposeCaches ();
	};
		

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_ENCODER_BASE_IMPLEMENTATION_H

