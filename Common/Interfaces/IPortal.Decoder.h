/**
 * Portal decoder interface
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
#include "Portal/Portal.Stream.h"

#ifndef INCLUDE_HCM_ENVIRONS_IPORTALDECODER_H
#define INCLUDE_HCM_ENVIRONS_IPORTALDECODER_H


namespace environs 
{
#ifdef _WIN32
	typedef void ( __cdecl * ptRenderCallback ) ( int type, void * surface, void * decoderOrByteBuffer );
#else
	typedef bool ( * ptRenderCallback ) ( int type, void * surface, void * decoderOrByteBuffer  );
#endif
	/**
	*	Interface for a portal decoder
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( IPortalDecoder )

	public:
		/** Default constructor */
		IPortalDecoder ( ) :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Decoder ),

			/** Default initialization */
			avContext ( 0 ), decodeImage ( true ), outputRGBA ( true), width ( 0 ), height ( 0 ), stride ( 0),
			avContextType ( DECODER_AVCONTEXT_TYPE_PIXELS ), avContextSubType ( ENVIRONS_AVCONTEXT_SUBTYPE_RGB ), avContextSize ( 0 )
		{};

		virtual ~IPortalDecoder () { ReleaseAVContext (); };

		/** Interface initializer. Do not override this method. Init () is called at the end of the Interface initializer */
		int										Init ( unsigned int deviceID ) {
													this->deviceID	= deviceID;
													return Init ( );
        }

        virtual void                            SetWidthHeight (int width, int stride, int height ) { this->width = width; this->height = height; this->stride = stride; };
    
        virtual bool                            InitType ( int type ) = 0;
    
		virtual bool							Start ( ) = 0;
		virtual void							Stop ( ) = 0;
    
        virtual void							SetStreamSource ( PortalStream * streamSource ) {};
    
		virtual bool							SetRenderResolution ( int width, int height ) = 0;
		virtual bool							SetRenderSurface ( void * penv, void * newSurface, int width, int height ) = 0;

        virtual void							ReleaseRenderSurface ( bool useLock ) = 0;
		virtual ptRenderCallback				GetRenderCallback ( int &callbackType ) { return 0; };

		/**
		* Decode a frame.
		*
		* @param	payload
		* @param	payloadSize
		*
		* @return	status	1 = success; 0 = failed this time, error status is recoverable, so try again with next round; -1 = failed, skip this plugin for further procesing
		*/
		virtual int								Perform ( int type, char * payload, int payloadSize ) = 0;
    
        virtual int								AllocateResources ( ) = 0;
		virtual int								ReleaseResources () = 0;

		virtual int								ReleaseAVContext ()  { return 1; };
    
    
        int                                     width;
        int                                     stride;
        int                                     height;
    
        void								*	avContext;
        int                                     avContextType;
        int                                     avContextSubType;
        int                                     avContextSize;
        bool                                    decodeImage;
        int                                     frameType;
    
        bool                                    outputRGBA;

	protected:
		virtual int								Init ( ) = 0;
		virtual void							Release ( ) = 0;
		
	EndInterface


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_IPORTALDECODER_H
