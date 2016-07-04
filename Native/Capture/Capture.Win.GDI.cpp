/**
* Windows GDI screen capture
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
#include "stdafx.h"

#ifdef _WIN32

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Capture.Win.GDI.h"
#include "Interfaces/IPortal.Renderer.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"Capture.Win.GDI. . . . ."

#pragma comment ( lib, "msimg32.lib" )

namespace environs 
{
	PortalBufferType_t	CaptureWinGDI_outputTypeSupport[] = { PortalBufferType::ARGBHandle, PortalBufferType::ARGB };
	

	CaptureWinGDI::CaptureWinGDI ( )
	{
		CLogID ( "Construct" );

		name				= "Windows GDI Grabber";
		captureType			= CaptureType::AppWindow;
		bufferType			= CaptureBufferType::PixelBuffer;

		hDC					= 0;
		hAppWindowDC		= 0;
		data				= 0;
		dataSize			= 0;
		hBitmapCapturedOld	= 0;

		outputTypes			= CaptureWinGDI_outputTypeSupport;
		outputTypesLength	= sizeof(CaptureWinGDI_outputTypeSupport) / sizeof(CaptureWinGDI_outputTypeSupport [0]);
	}


	int CaptureWinGDI::Init ( )
	{
		CVerbID ( "Init" );

		return MainThreadedInit ( );
	}


	bool CaptureWinGDI::MainThreadedInit ( )
	{
		CVerbID ( "MainThreadedInit" );

		if ( !hDC ) {
			hAppWindowDC = GetDC ( (HWND) hAppWindow );
			if ( !hAppWindowDC )
				return false;

			hDC = CreateCompatibleDC ( hAppWindowDC );
			if ( !hDC )
				return false;
		}

		enabled = true;

		initialized = true;
		return true;
	}


	CaptureWinGDI::~CaptureWinGDI ( )
	{
		CLogID ( "Destruct" );

		Release ( );
	}


	bool CaptureWinGDI::MainThreadedDispose ( )
	{
		CVerbID ( "MainThreadedDispose" );

		if ( hDC ) {
			DeleteDC ( hDC );
			hDC = 0;
		}

		if ( hAppWindowDC ) {
			ReleaseDC ( (HWND) hAppWindow, hAppWindowDC );
			hAppWindowDC = 0;
		}

		return true;
	}


	void CaptureWinGDI::ReleaseOverlayBuffers ( RenderOverlay  * overlay )
	{
		CVerbID ( "ReleaseOverlayBuffers" );

		if ( overlay->renderArg1 ) {
			if ( overlay->renderArg3 ) {
				SelectObject ( (HDC) overlay->renderArg1, (HBITMAP) overlay->renderArg3 );
				overlay->renderArg3 = 0;
			}

			DeleteDC ( (HDC) overlay->renderArg1 );
			overlay->renderArg1 = 0;
		}

		if ( overlay->renderArg2 ) {
			DeleteObject ( (HBITMAP) overlay->renderArg2 );
			overlay->renderArg2 = 0;
		}
	}


	void CaptureWinGDI::Release ( )
	{
		CVerbID ( "Release" );

		initialized = false;

		ReleaseResources ( );

		MainThreadedDispose ( );

		if ( renderOverlays && renderOverlayMutex ) {

			LockAcquireV ( (pthread_mutex_t *) renderOverlayMutex, "Release" );

			for ( unsigned int i = 0; i < MAX_PORTAL_OVERLAYS; i++ ) {
				if ( !renderOverlays [i] )
					continue;

				ReleaseOverlayBuffers ( renderOverlays [i] );
			}

			LockReleaseV ( (pthread_mutex_t *) renderOverlayMutex, "Release" );
		}
	}
	

	int CaptureWinGDI::ReleaseResources ( )
	{
		CVerbID ( "ReleaseResources" );

		if ( hBitmapCapturedOld ) {
			if ( hDC ) {
				CVerbID ( "ReleaseResources: selecting old bitmap for capture into portal DC" );

				SelectObject ( hDC, hBitmapCapturedOld );
			}
			else {
				CErrID ( "ReleaseResources: INVALID GDI STATE. An old DC bitmap cannot be released due to a missing PortalDC" );
			}
			hBitmapCapturedOld = 0;
		}

		if ( dataHandle ) {
			CVerbID ( "ReleaseResources: Deleting bitmap capture handle" );

			DeleteObject ( dataHandle );
			dataHandle = 0;
		}
		data		= 0;
		dataSize	= 0;
		squareLength= 0;

		buffersInitialized	= false;
        
        return 1;
	}


	int CaptureWinGDI::AllocateResources ( RenderDimensions * dims )
	{
		CVerbID ( "AllocateResources" );

		BITMAPINFO info;
		Zero ( info );

		info.bmiHeader.biBitCount		= 32;
		info.bmiHeader.biCompression	= BI_RGB;
		info.bmiHeader.biPlanes			= 1;
		info.bmiHeader.biSize			= sizeof(info.bmiHeader);
		info.bmiHeader.biWidth			= dims->square;
		info.bmiHeader.biHeight			= - dims->square;
		//info.bmiHeader.biPlanes		= 4;
		info.bmiHeader.biSizeImage		= dims->square * dims->square * 4;

		data = 0;
		dataHandle = CreateDIBSection ( hDC, &info, DIB_RGB_COLORS, (VOID**) &data, 0, 0 );
		if ( !data ) {
			CErrArgID ( "AllocateResources: Failed to retrieve memory pointer (CreateDIBSection) for bitmap of size [%u], Error [%i]", (dims->square * dims->square * 4), GetLastError ( ) );
			return -1;
		}

		if ( !dataHandle ){
			CErrArgID ( "AllocateResources: Failed to create hBitmap for capturing of [%u] bytes", (dims->square * dims->square * 4) );
			return -1;
		}
		dataSize = info.bmiHeader.biSizeImage;

		hBitmapCapturedOld = (HBITMAP) SelectObject ( hDC, (HBITMAP)dataHandle );

		return 1;
	}


	int CaptureWinGDI::AllocateOverlayBuffers ( RenderOverlay  * overlay )
	{
		CVerbID ( "AllocateOverlayBuffers" );

		int ret = 0;

		if ( overlay->renderArg1 )
			ReleaseOverlayBuffers ( overlay );

		unsigned int widtha = 0;
		unsigned int heighta = 0;
		HBITMAP oldTmpBitmap = 0;
		HBITMAP oldBitmap = 0;

		if ( hAppWindow ) {
			WINDOWINFO info;
			Zero ( info );

			if ( GetWindowInfo ( (HWND)hAppWindow, &info ) ) {
				widtha = info.rcWindow.right - info.rcWindow.left;
				heighta = info.rcWindow.bottom - info.rcWindow.top;
			}
		}
		else {
			widtha = GetSystemMetrics ( SM_CXSCREEN );
			heighta = GetSystemMetrics ( SM_CYSCREEN );
		}

		if ( !widtha || !heighta )
			return 0;

		HDC tempDC = CreateCompatibleDC ( 0 );
		HDC overlayDC = CreateCompatibleDC ( 0 );
		if ( !overlayDC || !tempDC ) {
			CErrID ( "AllocateOverlayBuffers: Failed to create a compatible DC for overlay drawing." );
			return 0;
		}

		HBITMAP tempBitmap = CreateBitmap ( overlay->width, overlay->height, 1, 32, overlay->data );
		HBITMAP overlayBitmap = CreateBitmap ( widtha, heighta, 1, 32, 0 );
		if ( !overlayBitmap || !tempBitmap ) {
			DeleteDC ( overlayDC );
			CErrID ( "AllocateOverlayBuffers: Failed to create a bitmap for overlay drawing." );
			goto Failed;
		}
		
		oldBitmap = (HBITMAP) SelectObject ( overlayDC, overlayBitmap );

		oldTmpBitmap = (HBITMAP) SelectObject ( tempDC, tempBitmap );

		ret = (int)BitBlt ( overlayDC, overlay->left, overlay->top, overlay->width, overlay->height, tempDC, 0, 0, SRCCOPY );
		if ( !ret ) {
			CErrID ( "AllocateOverlayBuffers: Failed to render overlay bitmap for overlay drawing." );
			goto Failed;
		}

		overlay->renderArg3 = oldBitmap;
		overlay->renderArg1 = overlayDC;
		overlayDC = 0;
		overlay->renderArg2 = overlayBitmap;
		overlayBitmap = 0;

		ret = 1;

	Failed:
		if ( tempDC ) {
			if ( oldTmpBitmap )
				SelectObject ( tempDC, oldTmpBitmap );
			DeleteDC ( tempDC );
		}
		if ( tempBitmap )
			DeleteObject ( tempBitmap );

		if ( overlayDC ) {
			if ( oldBitmap )
				SelectObject ( overlayDC, oldBitmap );
			DeleteDC ( overlayDC );
		}
		if ( overlayBitmap )
			DeleteObject ( overlayBitmap );
		return ret;
	}


	int CaptureWinGDI::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		//CVerbID ( "Perform" );

		int left = 0;
		int leftc = dims->left_cap;
		int top = 0;
		int topc = dims->top_cap;
		int widtha = dims->square;
		int heighta = dims->square;

		/// grab that frame
		if ( dims->left_cap < 0 || dims->top_cap < 0 ) {
			if ( dims->left_cap < 0 ) {
				left = -dims->left_cap;
				widtha -= left;
				leftc = 0;
			}

			if ( dims->top_cap < 0 ) {
				top = -dims->top_cap;
				heighta -= top;
				topc = 0;
			}
		}

		//			GdiFlush();
		BOOL ret = BitBlt ( hDC, left, top, widtha, heighta, hAppWindowDC, leftc, topc, SRCCOPY );
		if ( !ret )
			return 0;

		if ( renderOverlays [0] ) {

			pthread_mutex_lock ( (pthread_mutex_t *) renderOverlayMutex );

			if ( leftc + widtha > (int) env->appWindowWidth )
				widtha = env->appWindowWidth - leftc;
			if ( topc + heighta > (int) env->appWindowHeight )
				heighta = env->appWindowHeight - topc;

			for ( unsigned int i = 0; i < MAX_PORTAL_OVERLAYS; i++ ) {
				if ( !renderOverlays [i] )
					break;

				if ( renderOverlays [i]->errorCount > 20 )
					continue;

				BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)renderOverlays [i]->alpha, AC_SRC_ALPHA };

				if ( renderOverlays [i]->positionDevice ) {
					leftc = 0;
					topc = 0;
				}

				ret = AlphaBlend ( hDC, left, top, widtha, heighta, (HDC) renderOverlays [i]->renderArg1, leftc, topc, widtha, heighta, bf );
				//ret = BitBlt ( hDC, 0, 0, squareLength, squareLength, (HDC) renderOverlays [i]->renderArg1, dims->left_cap, dims->top_cap, SRCCOPY );
				if ( !ret ) {
					CErrArgID ( "Perform: AlphaBlend failed drawing the overlay with error [%u].", GetLastError () );
					//renderOverlays [i]->errorCount++;
					//break;
				}
				renderOverlays [i]->errorCount = 0;
			}

			pthread_mutex_unlock ( (pthread_mutex_t *) renderOverlayMutex );
		}

		return 1;
	}



} /* namespace environs */

#endif

