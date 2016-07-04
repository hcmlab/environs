/**
* Implementation of jpeg/png encoding using GDI
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
#include "Encoder/Encoder.GDI.h"

using namespace Gdiplus;

// The TAG for prepending in log messages
#define CLASS_NAME	"Encoder.GDI. . . . . . ."


namespace environs 
{
	PortalBufferType_t	EncoderGDI_inputTypeSupport[] = { PortalBufferType::GDIBitmap, PortalBufferType::ARGBHandle, PortalBufferType::ARGB };

	EncoderGDI::EncoderGDI ( )
	{
		name = "Picture Stream Encoder GDI";

		CLogArgID ( "Construct [%s]", name );

		usePNG				= true;
		uQuality			= 80;
		frameCounter		= 0;

		keyframeCounter		= 0;
		keyframeHandled		= false;

		encoderParams.Count = 1;

		encoderParams.Parameter [0].NumberOfValues = 1;
		encoderParams.Parameter [0].Guid  = EncoderQuality;
		encoderParams.Parameter [0].Type  = EncoderParameterValueTypeLong;
		encoderParams.Parameter [0].Value = &uQuality;

		inputTypes				= EncoderGDI_inputTypeSupport;
		inputTypesLength		= sizeof ( EncoderGDI_inputTypeSupport ) / sizeof ( PortalBufferType_t );
	}


	EncoderGDI::~EncoderGDI ( )
	{
		CLogArgID ( "Destruct [%s]", name );
	}


	bool EncoderGDI::Init ( int deviceIDa, int usePNGa, int widtha, int heighta, int frameRate )
	{
		deviceID = deviceIDa;

		CLogArgID ( "Init: [%s]", usePNGa ? "PNG" : "JPG" );

		usePNG = (usePNGa != 0);

		width		= widtha;
		height		= heighta;

		GetEncoderClsid ( usePNGa ? L"image/png" : L"image/jpeg", &clsid );
        
        encodedType = usePNGa ? DATA_STREAM_IMAGE_PNG : DATA_STREAM_IMAGE_JPEG;

		CLogID ( "Init successful." );
		return true;
	}


	int EncoderGDI::Perform ( RenderContext * context )
	{
		int					ret				= -1;
		IStream			*	pIStream		= NULL;
		Bitmap			*	bitmap			= 0;
		Gdiplus::Status		gdiStatus;

		while ( context )
		{
			//
			// Image stream creation
			// -------------------------------
			keyframeCounter++;

			if ( context->hasChanged ) {
				keyframeHandled = false;
			}
			else {
				// Use tcp and png support for progressive images
				if ( keyframeHandled ) // Already sent a key frame with the previous round
				{
					if ( (keyframeCounter % 30) != 0 ) {
						ret = 0;
						break;
					}
				}
				keyframeHandled = true;
				keyframeCounter = 1;
			}

			if ( inputType == PortalBufferType::ARGB ) {
				bitmap = new Bitmap ( context->width, context->height, context->stride, PixelFormat32bppPARGB, (BYTE *) context->renderedData );
			}
			else if ( inputType == PortalBufferType::ARGBHandle ) {
				bitmap = new Bitmap ( (HBITMAP)context->renderedDataHandle, NULL );
			}
			else if ( inputType == PortalBufferType::GDIBitmap )
				bitmap = (Bitmap *) context->renderedData;

			if ( !bitmap || bitmap->GetLastStatus ( ) != S_OK ){
				CErrID ( "Perform: Failed to load/create bitmap for picture encoding in gdi+!" );
				break;
			}

			if ( CreateStreamOnHGlobal ( NULL, TRUE, (LPSTREAM*)&pIStream ) != S_OK || !pIStream ) {
				CLogID ( "Perform: Failed to create stream on global memory!" );
				break;
			}

			if ( usePNG )
				gdiStatus = bitmap->Save ( pIStream, &clsid, NULL );
			else
				gdiStatus = bitmap->Save ( pIStream, &clsid, &encoderParams );

			if ( gdiStatus != S_OK ) {
				CLogID ( "Perform: Failed to save to stream!" );
				break;
			}

			// Encoder is free for use within the next worker thread
#ifndef ENABLE_WORKER_STAGES_LOCKS
			SetEvent ( context->eventEncoded );
#endif
			ULARGE_INTEGER ulnSize;
			LARGE_INTEGER lnOffset;
			lnOffset.QuadPart = 0;
			if ( pIStream->Seek ( lnOffset, STREAM_SEEK_END, &ulnSize ) != S_OK ) {
				CErrID ( "Perform: Failed to get the size of the stream!" );
				break;
			}

			// now move the pointer to the beginning of the file
			if ( pIStream->Seek ( lnOffset, STREAM_SEEK_SET, NULL ) != S_OK ) {
				CErrID ( "Perform: Failed to move the file pointer to the beginning of the stream!" );
				break;
			}

			ULONG payloadSize = (ULONG)ulnSize.QuadPart;

			if ( !ResizeBuffer ( context, payloadSize + 4 ) )
				break;

			// Read the stream directly into the buffer
			if ( pIStream->Read ( BYTEBUFFER_DATA_POINTER_START ( context->outputBuffer ), (ULONG) ulnSize.QuadPart, &payloadSize ) != S_OK ) {
				CErrArgID ( "Perform: Failed to read stream into buffer [%u]!", payloadSize );
				break;
			}

			context->frameCounter = frameCounter;
			frameCounter++;
            context->isIFrame = true;
            
			//CVerbArgID ( "Size: %i!", length );
			ret = 1;
			break;
		}

		if ( pIStream )
			pIStream->Release ( );

		if ( bitmap && inputType != PortalBufferType::GDIBitmap )
			delete bitmap;

		return ret;
	}


	int EncoderGDI::GetEncoderClsid ( const WCHAR* format, CLSID* pClsid )
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		ImageCodecInfo* pImageCodecInfo = NULL;

		Gdiplus::Status status = GetImageEncodersSize ( &num, &size );
		if ( status != S_OK || size == 0 )
			return -1;

		pImageCodecInfo = (ImageCodecInfo *)(malloc ( size ));
		if ( pImageCodecInfo == NULL )
			return -1;

		status = GetImageEncoders ( num, size, pImageCodecInfo );
		if ( status != S_OK )
			return -1;

		int ret = -1;
		if ( num > size ) num = size;

		for ( unsigned int j = 0; j < num; j++ )
		{
			if ( wcscmp ( pImageCodecInfo [j].MimeType, format ) == 0 )
			{
				*pClsid = pImageCodecInfo [j].Clsid;
				ret = j;  // Success
				break;
			}
		}

		free ( pImageCodecInfo );
		return ret;
	}


} /* namespace environs */

#endif

