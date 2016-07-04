/**
 * DirectShow Camera Capture
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_CAMERA_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_CAMERA_H

#ifdef _WIN32

#include "Interfaces/IPortal.Capture.h"
#ifdef GetStatus
#undef GetStatus
#endif

#include <windows.h>
#include <dshow.h>
#include "streams.h"
#include "Cam.Base.h"

#if !defined(ENVIRONS_MISSING_DIRECTSHOW_SDK)

#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.

#define	IDENTIFIER_LENGTH	523

#ifndef CLSID_NullRenderer
DEFINE_GUID ( CLSID_NullRenderer, 0xC1F400A4, 0x3F08, 0x11d3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 );
#endif

// {C1EDCA91-7CCC-433E-B2E0-16F20C0BBE68}
DEFINE_GUID ( CLSID_CEnvironsGrabber, 0xc1edca91, 0x7ccc, 0x433e, 0xb2, 0xe0, 0x16, 0xf2, 0xc, 0xb, 0xbe, 0x68 );

// {FAD1CD2E-9C6B-4A78-A4D9-9C27C1AE1596}
DEFINE_GUID ( CLSID_CEnvironsGrabberPropertyPage, 0xfad1cd2e, 0x9c6b, 0x4a78, 0xa4, 0xd9, 0x9c, 0x27, 0xc1, 0xae, 0x15, 0x96 );

// {2B222B69-0F47-434A-8403-0C7BA97F7BB8}
DEFINE_GUID ( IID_IEnvironsGrabber, 0x2b222b69, 0xf47, 0x434a, 0x84, 0x3, 0xc, 0x7b, 0xa9, 0x7f, 0x7b, 0xb8 );


namespace environs
{
	class CaptureCamWin;

	DECLARE_INTERFACE_ ( IEnvironsGrabber, IUnknown )
	{
		STDMETHOD ( ToggleGrabbing ) (THIS_
			bool activateFrameBufferForGrabbing) PURE;

		STDMETHOD ( SetCaptureInterface ) (THIS_
			CaptureCamWin * capture) PURE;
	};


	/**
	*	DirectShow Camera Capture
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class CaptureCamWin : implements CamBase
	{
	public:
		CaptureCamWin ();
		~CaptureCamWin ();

		bool				initialized;
		int					coInitialized;

		/** Public Interface Methods */
		int					PreInit ( );
		int					Init ( );
		void				Release ( );

		int					Start ();
		int					Stop ();

	private:
		int							countOfCameras;
		wchar_t						camIdentifier [IDENTIFIER_LENGTH];

		GUID						mediaSubType;

		IGraphBuilder			*	graphBuilder;
		ICaptureGraphBuilder2	*	captureGraphBuilder;

		IMediaControl			*	mediaControl;
		IEnvironsGrabber		*	grabberInterface;
		IBaseFilter				*	captureDevice;
		IBaseFilter				*	grabberFilter;
		IAMVideoProcAmp			*	camQualInterface;
		IAMCameraControl		*	camControlInterface;

		HRESULT						InitCaptureGraphBuilder ( IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppCapBuild );
		HRESULT						DetectCameras ( );
		HRESULT						SelectCamera ( );
		HRESULT						SelectMediaTypeOfCam ( int fps, GUID desiredMediaSubType, bool useFirst = FALSE );
	};


	/**
	*	Windows Camera Grabber
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class CEnvironsGrabber : public CTransInPlaceFilter, public IEnvironsGrabber, public ISpecifyPropertyPages
	{
	public:

		static CUnknown *WINAPI CreateInstance ( LPUNKNOWN punk, HRESULT *phr );

		DECLARE_IUNKNOWN;

		CEnvironsGrabber ( TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr );

		~CEnvironsGrabber ();

		// Overrriden from CTransformFilter base class
		HRESULT			Transform ( IMediaSample *pSample );
		HRESULT			CheckInputType ( const CMediaType* mtIn );

		// Reveals ITransformTemplate and ISpecifyPropertyPages
		STDMETHODIMP	NonDelegatingQueryInterface ( REFIID riid, void ** ppv );

		STDMETHODIMP	ToggleGrabbing ( bool activate );

		// ISpecifyPropertyPages interface
		STDMETHODIMP	GetPages ( CAUUID *pPages );

		STDMETHODIMP	SetCaptureInterface ( CaptureCamWin * capture );


	private:
		BOOL CanPerformTransform ( const CMediaType *pMediaType ) const;

		CaptureCamWin * capture;

		bool			enabled;

	};

} /* namespace environs */

#endif

#endif // _WIN32

#endif	// INCLUDE_HCM_ENVIRONS_PORTAL_CAPTURE_WIN_CAMERA_H
