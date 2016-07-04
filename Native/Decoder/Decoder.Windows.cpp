/**
 * Base decoder for windows
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


#ifndef ENVIRONS_NATIVE_MODULE
#	define ENVIRONS_NATIVE_MODULE
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Decoder.Windows.h"
#include <Shlwapi.h>

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Environs.Modules.h"
#include "Environs.Av.Context.h"
#include "Core/Byte.Buffer.h"
#include "Environs.Build.Lnk.h"
using namespace environs;

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Shlwapi.lib")


#define	CLASS_NAME 	"DecoderWindows"


static const char		*		DecoderWindows_extensionNames[]	= { "Windows GDI/module Decoder", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	DecoderWindows_interfaceTypes[]	= { InterfaceType::Decoder, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( DecoderWindows_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( DecoderWindows_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( DecoderWindows );


/**
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();

#endif

namespace environs
{

	DecoderWindows::DecoderWindows ()
	{
		name = DecoderWindows_extensionNames[0];

		CLogArg ( "Construct [%s]", name );

        idecoder    = 0;

        renderSurface   = 0;
        renderWidth     = 0;
        renderHeight    = 0;
		bitmap			= 0;
		bitmapData		= 0;

        decodeImage = true;
        outputRGBA  = true;
	}


	DecoderWindows::~DecoderWindows ()
	{
		CLogID ( "Destruct" );

		Release ( );

    }


	int DecoderWindows::Init ()
    {
        CVerbID ( "Init" );

        initialized = true;

        return true;
    }


	bool DecoderWindows::InitType ( int type )
    {
        CVerbArgID ( "InitType [%i]", type );

        bool success = false;

        // This initializer is called when the receiver actually received a stream init packet
        // which determines the stream type
        // If we support the stream type, then we return true. Otherwise we return false.

        if ( (type & DATA_STREAM_VIDEO ) == DATA_STREAM_VIDEO )
        {
            // We support h264 streams.
            CVerb ( "InitType: Initialize video decoder." );

            // Try loading an appropriate decoder here.
			if ( env->mod_PortalDecoder )
				idecoder = (IPortalDecoder *) CreateEnvInstance ( env->mod_PortalDecoder, 0, InterfaceType::Decoder, deviceID );

			if ( !idecoder )
				idecoder = (IPortalDecoder *) CreateEnvInstance ( LIBNAME_Decoder_LibOpenH264, 0, InterfaceType::Decoder, deviceID );

			while ( idecoder )
			{
				idecoder->avContextSubType = ENVIRONS_AVCONTEXT_SUBTYPE_BGR;

				idecoder->SetWidthHeight ( width, stride, height );

				if ( !idecoder->Init ( deviceID ) )
					break;

				if ( !idecoder->Start () )
					break;

				if ( renderSurface )
					success = idecoder->SetRenderSurface ( 0, renderSurface, renderWidth, renderHeight );
				else
					success = true;

				if ( success ) {
					avContextType = idecoder->avContextType;
					avContextSubType = idecoder->avContextSubType;
				}
				break;
			}

            if ( success )
                return true;

            if ( idecoder ) {
                delete idecoder;
                idecoder = 0;
            }
        }
        else if ( (type & DATA_STREAM_IMAGE) == DATA_STREAM_IMAGE ) {

			// We using GDIPlus to support image streams.
			ReleaseAVContext ();

			EnvironsAVContext * ctx = (EnvironsAVContext *) calloc ( 1, sizeof (EnvironsAVContext) );
			if ( ctx )
			{
				ctx->width = width;
				ctx->height = height;

				avContext = (void *) ctx;
				avContextType = RENDER_CALLBACK_TYPE_AVCONTEXT;
				return true;
			}
        }

        return success;
    }


	void DecoderWindows::Release ()
	{
        CVerbID ( "Release");

		enabled = false;

        if ( idecoder ) {
            delete idecoder;
            idecoder = 0;

			// Clear
			avContext = NULL;
        }

		if ( bitmap ) {
			if ( bitmapData ) {
				bitmap->UnlockBits ( bitmapData );
				delete bitmapData;

				bitmapData = 0;
			}
			delete bitmap;
			bitmap = 0;
		}

		ReleaseAVContext ();

		initialized = false;
	}


	bool DecoderWindows::Start ()
	{
		CVerbID ( "Start" );

        bool ret = false;

        enabled = true;

        if ( idecoder )
            idecoder->Start ();

		return ret;
	}


	void DecoderWindows::Stop ()
	{
		CVerbID ( "Stop" );

		enabled = false;

        if ( idecoder )
            idecoder->Stop ();
	}


	bool DecoderWindows::SetRenderResolution ( int width, int height )
    {
        renderWidth     = width;
        renderHeight    = height;

        return true;
    }


	bool DecoderWindows::SetRenderSurface ( void * penv, void * newSurface, int width, int height )
    {
        bool ret = false;

        if ( idecoder )
            ret = idecoder->SetRenderSurface ( penv, newSurface, width, height );
        else {
            renderSurface   = newSurface;
            renderWidth     = width;
            renderHeight    = height;
            CVerbID ( "SetRenderSurface: No actual decoder available to set the render surface! We're caching them for later access." );

            ret = true;
        }

        return ret;
    }


	/**
	* Decode the buffer, that is acquire the decoder resources such as frames, context, etc.
	*
	*	@return	success
	* */
	int DecoderWindows::Perform ( int type, char * payload, int payloadSize )
	{
		CVerbVerbID ( "Perform" );

		int					ret = 0;
		Gdiplus::Status		gdiStatus;

		if ( (type & DATA_STREAM_VIDEO) == DATA_STREAM_VIDEO )
		{
			if ( idecoder ) {
				ret = idecoder->Perform ( type, payload, payloadSize );
				if ( ret == 1 ) {
					avContext = idecoder->avContext;
					avContextSize = idecoder->avContextSize;
				}
			}
		}
		else if ( (type & DATA_STREAM_IMAGE_DATA) == DATA_STREAM_IMAGE_DATA )
		{
			IStream * pStream = ::SHCreateMemStream ( (const BYTE *) payload, payloadSize );
			if ( pStream )
			{
				if ( bitmap )  {
					if ( bitmapData ) {
						bitmap->UnlockBits ( bitmapData );
					}
					delete bitmap;
				}

				bitmap = Gdiplus::Bitmap::FromStream ( pStream );
				pStream->Release ();

				if ( bitmap )
				{
					if ( bitmap->GetLastStatus () != Gdiplus::Ok ) {
						delete bitmap;
						bitmap = 0;
					}
					else {
						if ( !bitmapData )
							bitmapData = new BitmapData;

						if ( bitmapData ) {
							Rect rect ( 0, 0, width, height );

							gdiStatus = bitmap->LockBits ( &rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData );

							if ( gdiStatus == Gdiplus::Ok ) {
								EnvironsAVContext * ctx = (EnvironsAVContext *) avContext;

								ctx->data = (char *)bitmapData->Scan0;
								ctx->stride = bitmapData->Stride;
								ret = 1;
							}
							else {
								delete bitmapData;
								bitmapData = 0;
							}
						}
					}
				}
			}
		}

		CVerbVerbID ( "Perform: done" );

		return ret;
	}


	int DecoderWindows::AllocateResources ()
    {
        CVerbID ( "AllocateResources" );

        int success     = 1;

		if ( idecoder ) {
			success = idecoder->AllocateResources ();

			stride = idecoder->stride;
		}

        CVerbID ( "AllocateResources succeeded" );

        return success;
    }


	int DecoderWindows::ReleaseResources ()
    {
        CVerbID ( "ReleaseResources" );

        int success     = 1;

		if ( idecoder )
			success = idecoder->ReleaseResources ();

        CVerbID ( "ReleaseResources succeeded" );

        return success;
    }


} /* namespace environs */

#endif

