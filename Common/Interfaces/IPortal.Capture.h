/**
 * Portal capture interfaces
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
#include "Render.Dimensions.h"
#include "Render.Overlay.h"

#ifndef INCLUDE_HCM_ENVIRONS_IPORTALCAPTURE_H
#define INCLUDE_HCM_ENVIRONS_IPORTALCAPTURE_H



namespace environs 
{
	/**
	*	Interface for a capture / grabber extension
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( IPortalCapture )

	public:
		/** Default constructor */
		IPortalCapture () :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Capture ),

			/** Default initialization */
			captureType ( CaptureType::Unknown ),
            portalWorkerEvent ( 0 ), portalCaptureEvent ( 0 ), portalWorkerEventLock ( 0 ),
			bufferType ( CaptureBufferType::Unknown ), width ( 0 ), height ( 0 ),
			squareLength ( 0 ), buffersInitialized ( false ), dataAccessed ( 1 ), data ( 0 ), dataHandle ( 0 ),
            dataSize ( 0 ), dataStride ( 0 ),
			renderOverlayMutex ( 0 ), renderOverlays ( 0 ),
			osLevel ( 0 ), hAppWindow ( 0 ), stages ( 0 )
		{};

		virtual ~IPortalCapture () {};

		/** Interface initializer. Do not override this method. Init () is called at the end of the Interface initializer */
		int										Init ( int _deviceID, void * appWindow ) {
													deviceID	= _deviceID;
													hAppWindow  = appWindow;
													return Init ( );
												}

		virtual int								Init ( ) = 0;
		virtual void							Release ( ) = 0;
		virtual int								Start ( ) { return 0; };
		virtual int								Stop ( ) { return 0; };

		virtual int                             ReleaseResources ( ) = 0;
		virtual int								AllocateResources ( RenderDimensions * dims ) = 0;

		/**
		* Grab a frame.
		*
		* @param	RenderDimensions
		*
		* @return	status	1 = success; 0 = failed this time, error status is recoverable, so try again with next round; -1 = failed, skip this plugin for further procesing
		*/
		virtual int								Perform ( RenderDimensions * dims ) = 0;
		
		CaptureType::CaptureType				captureType;

		/// If the extension is a camera capture, then the following trigger sink may be used with the camera as a trigger source.
		/// If they are not used, then the Init() method must clear them (set to 0), in order to instruct the PortalSource to create its own frame trigger source
		void								*	portalWorkerEvent;
		void								*	portalCaptureEvent;
		void								*	portalWorkerEventLock;


		CaptureBufferType::CaptureBufferType	bufferType;

		int										width;
		int										height;
		int										squareLength;
		bool									buffersInitialized;

		long									dataAccessed;
		void								*	data;
		void								*	dataHandle;

		unsigned int 							dataSize;
		unsigned int 							dataStride;

		void								*	renderOverlayMutex;
		RenderOverlay						**	renderOverlays;
		virtual int								AllocateOverlayBuffers ( RenderOverlay  * overlay ) { return 0; };
		virtual void							ReleaseOverlayBuffers ( RenderOverlay  * overlay ) {};
    
        void								*	stages;
		int										osLevel;

	protected:
		void *									hAppWindow;

		
	EndInterface


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_IPORTALCAPTURE_H
