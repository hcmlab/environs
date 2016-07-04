/**
* GDI portal implementation
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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Render.CaptureGDI.h"
#include "Environs.Obj.h"
#include "Core/Performance.Count.h"
#include "Device/Device.Controller.h"

#include "Math.h"
#define PI 3.14159265

using namespace Gdiplus;

#define CLASS_NAME	"RenderCaptureGDI"



namespace environs 
{
//	
// Initialization of static values
	PortalBufferType_t	RenderCaptureGDI_inputTypeSupport[] = { PortalBufferType::GDIBitmap };
	PortalBufferType_t	RenderCaptureGDI_outputTypeSupport[] = { PortalBufferType::GDIBitmap };


	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	RenderCaptureGDI::RenderCaptureGDI ( )
	{
		CLog ( "Construct" );

		hAppWindowDC			= 0;
		hDC						= 0;
		lpPixelsCache			= 0;
		sizePixelsCache			= 0;
		lpPixelsCompareCache	= 0;
		sizePixelsCompareCache	= 0;
		hBitmapCaptured			= NULL;
		hBitmapCapturedOld		= NULL;

		name					= "GDI";

		Zero ( rect );

		inputTypes				= RenderCaptureGDI_inputTypeSupport;
		inputTypesLength		= sizeof(RenderCaptureGDI_inputTypeSupport) / sizeof(RenderCaptureGDI_inputTypeSupport [0]);

		outputTypes				= RenderCaptureGDI_outputTypeSupport;
		outputTypesLength		= sizeof(RenderCaptureGDI_outputTypeSupport) / sizeof(RenderCaptureGDI_outputTypeSupport [0]);
	}


	RenderCaptureGDI::~RenderCaptureGDI ( )
	{
		CLog ( "Destructor" );

		Dispose ( );
	}


	bool RenderCaptureGDI::Init ( )
	{
		CLog ( "Init" );

		//outputType = PortalBufferType::ARGB;
		if ( env ) {
			useRenderCache = env->useRenderCache;
		}

		// We know that we're called by the tcp-listener thread (which is the main or responsible thread)
		return MainThreadedInit ( );
	}


	bool RenderCaptureGDI::MainThreadedInit ( )
	{
		CLogArg ( "MainThreadedInit device [%d]", deviceID );

		if ( !hDC ) {
			//if ( !hAppWindow )
			//	return false;

			hAppWindowDC = GetDC ( (HWND) hAppWindow );
			if ( !hAppWindowDC )
				return false;

			hDC = CreateCompatibleDC ( hAppWindowDC );
			if ( !hDC )
				return false;
		}

		return true;
	}


	bool RenderCaptureGDI::MainThreadedDispose ( )
	{
		CLogArg ( "MainThreadedDispose device [%d]", deviceID );

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


	void RenderCaptureGDI::Dispose ( )
	{
		CLog ( "Dispose" );

		if ( lpPixelsCache ) {
			CLog ( "Dispose: Deleting pixel cache." );
			delete lpPixelsCache;
			lpPixelsCache = NULL;
		}
		sizePixelsCache = 0;

		if ( lpPixelsCompareCache ) {
			CLog ( "Dispose: Deleting pixel compare cache." );
			delete lpPixelsCompareCache;
			lpPixelsCompareCache = NULL;
		}
		sizePixelsCompareCache = 0;
	}

	/*
	bool RenderCaptureGDI::ConnectOutput ( IEnvironsBase * dest )
	{
		CLogID ( "ConnectOutput" );

		if ( dest->interfaceType != InterfaceType::Encoder ) {
			return false;
		}

		IPortalEncoder * encoder = (IPortalEncoder *) dest;

		for ( unsigned int i = 0; i < encoder->inputTypesLength; i++ ) {
			if ( encoder->inputTypes [i] == PortalBufferType::ARGB ) {
				encoder->inputType = PortalBufferType::ARGB;
				return true;
			}
		}
		return false;
	}
	*/



	char * RenderCaptureGDI::GetBuffer ( RenderContext * context )
	{
		Bitmap * bitmap = (Bitmap *)context->renderedData;
		if ( !bitmap )
			return 0;

		bitmap->LockBits ( &rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData );

		if ( bitmapData.PixelFormat == PixelFormat32bppARGB )
		{
			char * inOutBuffer = (char *) bitmapData.Scan0;
			if ( inOutBuffer ) {
				context->width = bitmapData.Width;
				context->height = bitmapData.Height;
				context->stride = bitmapData.Stride;
				return inOutBuffer;
			}
		}

		return 0;
	}



	void RenderCaptureGDI::ReleaseBuffer ( RenderContext * context )
	{
		Bitmap * bitmap = (Bitmap *)context->renderedData;
		if ( !bitmap )
			return;

		bitmap->UnlockBits ( &bitmapData );
	}


	int RenderCaptureGDI::ReleaseResources ( RenderContext * context )
	{
		CLog ( "ReleaseResources" );

		if ( context->renderedDataHandle ) {
			delete ((Graphics *)context->renderedDataHandle);
			context->renderedDataHandle = 0;
		}

		if ( context->renderedData ) {
			delete ((Bitmap *)context->renderedData);
			context->renderedData = 0;
		}

		if ( hBitmapCaptured ) {
			DeleteObject ( hBitmapCaptured );
			hBitmapCaptured = 0;
		}

		buffersInitialized = false;
        return 1;
	}


	int RenderCaptureGDI::AllocateResources ( RenderDimensions * dims )
	{
		CLog ( "AllocateResources" );

		if ( !hBitmapCaptured ) {
			hBitmapCaptured = CreateCompatibleBitmap ( hAppWindowDC, dims->square, dims->square );
			if ( !hBitmapCaptured )
				return 0;
		}

		return 1;
	}


	int RenderCaptureGDI::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
	{
		//CLog ( "Perform" );
		CVerbArgID ( "UpdateBuffers: Updating buffers for context [%u]", context->id );
		ReleaseResources ( context );

		return AllocateResources ( dims );
	}


	bool RenderCaptureGDI::Perform ( RenderDimensions * dims, RenderContext * context ) // doPortalUpdateGDIDoubleBuffer
	{
		//CLog ( "Perform" );

		rendered = false;

		unsigned int portal_width = dims->streamWidth, portal_heigth = dims->streamHeight;
		bool		ret				= false;
		HBITMAP		hOldBitmap		= NULL;

		while ( true )
		{
			hOldBitmap = (HBITMAP) SelectObject ( hDC, hBitmapCaptured );

			BitBlt ( hDC, 0, 0, dims->square, dims->square, hAppWindowDC, dims->left_cap, dims->top_cap, SRCCOPY );
			pc_MeasureCapture ( context );

			hBitmapCaptured = (HBITMAP) SelectObject ( hDC, hOldBitmap );


			//
			// Memory compare of pixel values
			//
			if ( CompareBitmaps ( hDC, hBitmapCaptured ) )
			{
				if ( useRenderCache ) {
					filledContexts++;

					if ( filledContexts > MAX_PORTAL_CONTEXT_WORKERS ) {
						context->hasChanged = false;

						ret = true;
						break;
					}
				}
			}
			else {
				filledContexts = 0;
				context->hasChanged = true;
			}

			Bitmap bitmapCapture ( hBitmapCaptured, NULL );
			if ( bitmapCapture.GetLastStatus ( ) != S_OK ) {
				CLog ( "Perform: Failed to load captured bitmap into gdi+!" );
				break;
			}

			float theta = 0 - (dims->orientation - 90);
			/*if (!inContact) {
			theta += (deviceAzimut - deviceAzimutLast);
			}*/

			Graphics captureGraphics ( &bitmapCapture );

			Matrix transformMatrix;
			transformMatrix.RotateAt ( theta, PointF ( (REAL) (dims->square >> 1), (REAL) (dims->square >> 1) ), MatrixOrderAppend );
			captureGraphics.SetTransform ( &transformMatrix );
			captureGraphics.DrawImage ( &bitmapCapture, 0, 0, 0, 0, dims->square, dims->square, UnitPixel );


			Bitmap * bitmap = (Bitmap *)context->renderedData;
			if ( !bitmap ) {
				context->width = portal_width;
				context->height = portal_heigth;
				rect.Width = portal_width;
				rect.Height = portal_heigth;

				bitmap = new Bitmap ( portal_width, portal_heigth, PixelFormat32bppPARGB );
				if ( !bitmap || bitmap->GetLastStatus ( ) != S_OK ) {
					CLog ( "Perform: Failed to create bitmap for scaling in gdi+!" );
					break;
				}

				Graphics * graphics = new Graphics ( bitmap );
				if ( !graphics ) {
					CLog ( "Perform: Failed to create graphics for scaling in gdi+!" );
					delete bitmap;
					break;
				}

				context->renderedData = bitmap;
				context->renderedDataHandle = graphics;
			}
			else {
				portal_width = context->width;
				portal_heigth = context->height;
			}

			Graphics * graphics = (Graphics *)context->renderedDataHandle;
			if ( !graphics )
				break;

			if ( graphics->DrawImage ( &bitmapCapture, rect, dims->xOffset, dims->yOffset, dims->width_cap, dims->height_cap, UnitPixel ) != Ok )
				break;

			ret = true;
			break;
		}

		/*if ( hBitmapCaptured ) {
		DeleteObject ( hBitmapCaptured );
		hBitmapCaptured = 0;
		}*/

		rendered = ret;
		return ret;
	}





	bool RenderCaptureGDI::CompareBitmaps ( HDC hCompareDC, HBITMAP hBitmap )
	{
		//
		// Memory compare of pixel values
		//
		while ( true ) {
			BITMAPINFO bmInfo;
			Zero ( bmInfo );
			bmInfo.bmiHeader.biSize = sizeof(bmInfo);

			// Get the BITMAPINFO structure from the bitmap
			if ( 0 == GetDIBits ( hCompareDC, hBitmap, 0, 0, NULL, &bmInfo, DIB_RGB_COLORS ) ) {
				// error handling
				break;
			}

			if ( sizePixelsCompareCache != bmInfo.bmiHeader.biSizeImage ) {
				if ( lpPixelsCompareCache ) {
					delete lpPixelsCompareCache;
					lpPixelsCompareCache = 0;
				}
				sizePixelsCompareCache = 0;
				if ( bmInfo.bmiHeader.biSizeImage <= 0 )
					break;

				lpPixelsCompareCache = new BYTE [bmInfo.bmiHeader.biSizeImage];
				if ( !lpPixelsCompareCache ) {
					CErr ( "compareBitmaps: Failed to allocate memory for bitmap pixels comparer!" );
					break;
				}
				sizePixelsCompareCache = bmInfo.bmiHeader.biSizeImage;
			}

			bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
			bmInfo.bmiHeader.biBitCount = 32;
			bmInfo.bmiHeader.biCompression = BI_RGB;
			bmInfo.bmiHeader.biHeight = (bmInfo.bmiHeader.biHeight < 0) ? (-bmInfo.bmiHeader.biHeight) : (bmInfo.bmiHeader.biHeight);

			// get the actual bitmap buffer
			if ( 0 == GetDIBits ( hCompareDC, hBitmap, 0, bmInfo.bmiHeader.biHeight, (LPVOID) lpPixelsCompareCache, &bmInfo, DIB_RGB_COLORS ) ) {
				// error handling
				break;
			}

			if ( !lpPixelsCache ) {
				lpPixelsCache = lpPixelsCompareCache;
				lpPixelsCompareCache = 0;
				sizePixelsCache = sizePixelsCompareCache;
				sizePixelsCompareCache = 0;
				return false;
			}

			if ( sizePixelsCache != sizePixelsCompareCache ) {
				delete lpPixelsCache;

				lpPixelsCache = lpPixelsCompareCache;
				lpPixelsCompareCache = 0;
				sizePixelsCache = sizePixelsCompareCache;
				sizePixelsCompareCache = 0;
				return false;
			}

			if ( memcmp ( lpPixelsCompareCache, lpPixelsCache, sizePixelsCache ) != 0 ) {
				delete lpPixelsCache;

				lpPixelsCache = lpPixelsCompareCache;
				lpPixelsCompareCache = 0;
				sizePixelsCache = sizePixelsCompareCache;
				sizePixelsCompareCache = 0;
				return false;
			}

			return true;
		}

		return false;
	}


} /* namespace environs */

#endif

