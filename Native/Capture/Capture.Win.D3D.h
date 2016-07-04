/**
* Windows D3D screen capture
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
#include "Environs.Types.h"
#include "Interfaces/IPortal.Capture.h"

#ifdef _WIN32


#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_D3D_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_D3D_H


namespace environs 
{
	/**
	*	Windows D3D screen capture
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/20/14
	*	@version	1.0
	*	@remarks	This functionality requires Windows 8 DXGI 1.2 Desktop Duplication
	* ****************************************************************************************
	*/
	class CaptureWinD3D : implements IPortalCapture
	{
	public:
		static void *				d3dDevice;

#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
		CaptureWinD3D ();
		~CaptureWinD3D ();

		/** Public Interface Methods */
		int							Init ( );
		void						Release ( );

		int                         ReleaseResources ( );
		int							AllocateResources ( RenderDimensions * dims );
        
		int							Perform ( RenderDimensions * dims, RenderContext * context );


		static bool					InitD3D ( Instance * env );
		static bool					DisposeD3D ( );

		//bool						ConnectOutput ( IEnvironsBase * dest );

	private:
		bool						didAddRef;

		void *						d3dTexture;

		bool						GetInvalidRegion ( RECT * rc );


		static LONGSYNC				referenceCount;
		static HANDLE				capturedEvent;
		static pthread_t			captureThreadID;
		static HANDLE				captureThreadEvent;
		static unsigned int			framesAvail;
		static CRITICAL_SECTION		capturedFrameCS;
	
		static bool					Duplicate ();

		static bool					CInit ( void * arg, int deviceID );
		static void					CDispose ();

		static bool					AcquireNextFrame ( UINT timeout );
		static void					ReleaseFrame ();
		static void *				Thread_Capture ( void * arg );

	protected:
		static LONGSYNC				clientAccessed;
#endif
	};


} /* namespace environs */

#endif

#endif	// INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_D3D_H