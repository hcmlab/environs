/**
 * Decoder Base access methods and states
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
#include "Interfaces/IPortal.Decoder.h"

#ifndef INCLUDE_HCM_ENVIRONS_DECODERBASE_H
#define	INCLUDE_HCM_ENVIRONS_DECODERBASE_H


namespace environs {
	/**
	*	Decoder Base access methods and states
	*	--------------------------------------------------------------------------------------
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/30/14
	*	@version	1.0
	*	@remarks	
	* ****************************************************************************************
	*/
	class DecoderBase : public IPortalDecoder {
	public:
		DecoderBase ( );
		virtual ~DecoderBase ( );
        
        virtual bool            InitType ( int type );
		virtual void			Release ( );
        
		virtual bool            Start ();
		virtual void            Stop ();

		/** Change target resolution of the decoder (for scaling). */
		virtual bool            SetStreamResolution ( int width, int stride, int height );

		virtual int				AllocateResources ( );
        virtual int             ReleaseResources ( );

		virtual void			ReleaseRenderSurface ( bool useLock ) {};
		virtual bool			SetRenderSurface ( void * penv, void * newSurface, int widtha, int heighta ) { return true; };


        bool                    SetRenderResolution ( int width, int height );
        
        virtual ptRenderCallback GetRenderCallback ( int &callbackType ) { return 0; };

		int                     Perform ( int type, char * payload, int payloadSize );
        
	protected:
        bool                    allocated;
        
		pthread_mutex_t         stateMutex;
		pthread_cond_t			stateSignal;
        
		bool					enabled;
		bool                    initialized;

        int                     renderWidth;
        int                     renderHeight;

		int						Init ( );
        virtual bool			InitInstance ( bool useLock = true );
        
        int                     PrepareRGBAResources ( );
        int                     ConvertI420ToSubRGB ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
        
        int                     ConvertI420ToABGR ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
		int                     ConvertI420ToBGRA ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
		int                     ConvertI420ToBGR ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
        int                     ConvertI420ToRGBA ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
        int                     ConvertI420ToRGB ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride );
        
	};

} /* namespace environs */


#endif /* INCLUDE_HCM_ENVIRONS_DECODERBASE */
