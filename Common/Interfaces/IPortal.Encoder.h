/**
 * Interface for a portal encoder/compressor
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
#include "IEnvirons.Base.h"
#include "Interface.Exports.h"
#include "Render.Context.h"

#ifndef INCLUDE_HCM_ENVIRONS_IPORTALENCODER_H
#define INCLUDE_HCM_ENVIRONS_IPORTALENCODER_H



namespace environs
{
	/**
	*	Interface for a portal encoder/compressor (create transport packages)
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( IPortalEncoder )

	public:
		/**
		* Default constructor
		*	@comment:
		*/
		IPortalEncoder () :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Encoder ),

			/** Default initialization */
			inBufferType ( EncoderBufferType::ARGB ), stages ( 0 ), iFrameFPSMode ( false ), iFrameRequest ( false ),
            width ( 0 ), height ( 0 ), frameCounter ( 0 ), avContext ( 0 ), encodedType ( 0 ),
            requireSendID ( false ), sendID ( -1 )
            { };

		virtual ~IPortalEncoder () {};

		virtual bool							Init ( int deviceID, int EncoderProps, int Width, int Height, int FrameRate ) = 0;

		// 0 means encoded, but do not send the data; 1 means encoded and new data to transmit, -1 means failed to encode.
        virtual int								Perform ( RenderContext * context ) = 0;
        virtual int								Perform () { return 0; };

		EncoderBufferType_t						inBufferType;

        void								*	stages;

        bool                                    iFrameFPSMode;
        bool                                    iFrameRequest;
	//protected:
		unsigned int							width;
		unsigned int							height;
        unsigned int                            frameCounter;

        void								*	avContext;
		int                                     encodedType;

        bool                                    requireSendID;
        int                                     sendID;

		virtual int								EncodeARGB ( char * source, char * &output, RenderContext * context ) { return 0; };
		virtual int								EncodeBGRA ( char * source, char * &output, RenderContext * context ) { return 0; };
		virtual int								EncodeI420 ( char * yuvdata, char * &output, RenderContext * context ) { return 0; };

	EndInterface

		extern bool								ResizeBuffer ( RenderContext * context, unsigned int payloadSize );

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_IPORTALENCODER_H
