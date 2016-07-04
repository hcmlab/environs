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

#include "Render.GDI.h"
#include "Environs.Obj.h"
#include "Core/Performance.Count.h"
#include "Device/Device.Controller.h"

#include "Math.h"
#define PI 3.14159265

using namespace Gdiplus;

#define CLASS_NAME	"RenderGDI"



namespace environs 
{
//	
// Initialization of static values
	PortalBufferType_t	RenderGDI_inputTypeSupport[] = { PortalBufferType::ARGBHandle };
	PortalBufferType_t	RenderGDI_outputTypeSupport[] = { PortalBufferType::GDIBitmap };


// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
RenderGDI::RenderGDI ()
{
	CLog ( "Construct" );

	lpPixelsCompareCache	= 0;
	sizePixelsCompareCache	= 0;

	name					= "GDI Renderer";

	Zero ( rect );

	inputTypes				= RenderGDI_inputTypeSupport;
	inputTypesLength		= sizeof(RenderGDI_inputTypeSupport) / sizeof(RenderGDI_inputTypeSupport [0]);

	outputTypes				= RenderGDI_outputTypeSupport;
	outputTypesLength		= sizeof(RenderGDI_outputTypeSupport) / sizeof(RenderGDI_outputTypeSupport [0]);
}


RenderGDI::~RenderGDI ()
{
	CLog ( "Destructor" );

	Dispose ();
}


bool RenderGDI::Init ()
{
	CLog ( "Init" );

	//outputType = PortalBufferType::ARGB;

	return true;
}


void RenderGDI::Dispose ()
{
	CLog ( "Dispose" );

	if ( lpPixelsCompareCache ) {
		CLog ( "Dispose: Deleting pixel compare cache." );
		delete lpPixelsCompareCache;
		lpPixelsCompareCache = NULL;
	}
	sizePixelsCompareCache = 0;
}


char * RenderGDI::GetBuffer ( RenderContext * context )
{
	Bitmap * bitmap = (Bitmap *) context->renderedData;
	if ( !bitmap )
		return 0;

	Gdiplus::Status gdiStatus = bitmap->LockBits ( &rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData );

	if ( gdiStatus != S_OK )
		return 0;
			
	if ( bitmapData.PixelFormat == PixelFormat32bppARGB )
	{
		char * inOutBuffer = (char *)bitmapData.Scan0;
		if ( inOutBuffer ) {
			context->width = bitmapData.Width;
			context->height = bitmapData.Height;
			context->stride = bitmapData.Stride;
			return inOutBuffer;
		}
	}
	else
		bitmap->UnlockBits ( &bitmapData );

	return 0;
}


void RenderGDI::ReleaseBuffer ( RenderContext * context )
{
	Bitmap * bitmap = (Bitmap *)context->renderedData;
	if ( !bitmap )
		return;

	bitmap->UnlockBits ( &bitmapData );
}


int RenderGDI::ReleaseResources ( RenderContext * context )
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

	buffersInitialized = false;
    return 1;
}


int RenderGDI::AllocateResources ( RenderDimensions * dims )
{
	CLog ( "AllocateResources" );
	
	return 1;
}


int RenderGDI::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
{
	//CLog ( "Perform" );
	CVerbArgID ( "UpdateBuffers: Updating buffers for context [%u]", context->id );

	Bitmap * bitmap = (Bitmap *) context->renderedData;
	if ( !bitmap ) {
		Gdiplus::Status status = Gdiplus::NotImplemented;

		rect.Width = dims->streamWidth;
		rect.Height = dims->streamHeight;

		bitmap = new Bitmap ( dims->streamWidth, dims->streamHeight, PixelFormat32bppPARGB );
		if ( bitmap )
			status = bitmap->GetLastStatus ( );

		if ( status != Gdiplus::Ok ) {
			CErrArgID ( "UpdateBuffers: Failed to create bitmap for scaling in gdi+. Status [%u]", status );
			if ( bitmap )
				delete bitmap;

			return 0;
		}

		Graphics * graphics = new Graphics ( bitmap );
		if ( !graphics ) {
			CErrID ( "UpdateBuffers: Failed to create graphics for scaling in gdi+!" );
			delete bitmap;
			return 0;
		}

		context->renderedData = bitmap;
		context->renderedDataHandle = graphics;
	}
	return 1;
}


bool RenderGDI::Perform ( RenderDimensions * dims, RenderContext * context ) // doPortalUpdateGDIDoubleBuffer
{
	//CVerbID ( "Perform" );

	rendered = false;

	bool		ret				= false;
	
	while ( true )
	{
#ifndef ENABLE_WORKER_STAGES_COMPARE
		//
		// Memory compare of pixel values
		//
		COMPARE_CACHE ( equal );
#endif
		Bitmap * bitmap = (Bitmap *) context->renderedData;
		if ( !bitmap )
			break;

		Graphics * graphics = (Graphics *) context->renderedDataHandle;
		if ( !graphics )
			break;

		IPortalCapture * cap = ((WorkerStages *) stages)->capture;
		
		Bitmap bitmapCapture ( (HBITMAP) cap->dataHandle, NULL );
		if ( bitmapCapture.GetLastStatus() != S_OK ) {
			CErrID ( "Perform: Failed to load captured bitmap into gdi+!" );
			break;
		}
		
		float theta = 0 - (dims->orientation - 90);
		/*if (!inContact) {
			theta += (deviceAzimut - deviceAzimutLast);
		}*/
		
		Graphics captureGraphics ( &bitmapCapture );
	
		Matrix transformMatrix;
		transformMatrix.RotateAt ( theta, Gdiplus::PointF ( (REAL)(dims->square >> 1), (REAL)(dims->square >> 1) ), MatrixOrderAppend );
		captureGraphics.SetTransform ( &transformMatrix );
		captureGraphics.DrawImage ( &bitmapCapture, 0, 0, 0, 0, dims->square, dims->square, UnitPixel );

		if ( graphics->DrawImage ( &bitmapCapture, rect, dims->xOffset, dims->yOffset, dims->width_cap, dims->height_cap, UnitPixel ) != Gdiplus::Ok )
			break;

		ret = true;
		break;
	}
	
	rendered = ret;
	return ret;
}

/*
bool _memcmp ( void * src, void * dst, int size )
{
	int ints = size >> 2;
	int rest = 4 - (size % 4);

	int * psrc = (int *) src;
	int * pdst = (int *) dst;

	for ( int i = 0; i < ints; i++ ) {
		if ( *psrc != *pdst ) {
			return false;
		}
		psrc++;
		pdst++;
	}
	return true;
}
*/

bool RenderGDI::Compare ( unsigned int &equal )
{
	//
	// Memory compare of pixel values
	//
	IPortalCapture * cap = ((WorkerStages *) stages)->capture;

	while ( cap && cap->data ) {
		if ( sizePixelsCompareCache != cap->dataSize ) {
			if ( lpPixelsCompareCache ) {
				delete lpPixelsCompareCache; 
				lpPixelsCompareCache = 0;
			}
			sizePixelsCompareCache = 0;
			if ( !cap->dataSize )
				break;

			lpPixelsCompareCache = new BYTE [cap->dataSize];
			if ( !lpPixelsCompareCache ) {
				CErrID ( "Compare: Failed to allocate memory for bitmap pixels comparer!" );
				break;
			}
			sizePixelsCompareCache = cap->dataSize;

			memcpy ( lpPixelsCompareCache, cap->data, sizePixelsCompareCache );
			equal = false;
			return true;
		}
		
		if ( memcmp ( lpPixelsCompareCache, cap->data, sizePixelsCompareCache ) != 0 ) {

			memcpy ( lpPixelsCompareCache, cap->data, sizePixelsCompareCache );
			equal = false;
		}
		else
			equal = true;
		
		return true;
	}

	return false;
}


} /* namespace environs */

#endif


