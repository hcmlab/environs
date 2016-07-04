/**
* Implementation of encoder using java platform objects
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
//#define ENVIRONS_NATIVE_MODULE

#if defined(ANDROID)

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Android.h"
#include "Encoder/Encoder.And.h"
using namespace environs;


static const char	*		EncoderAndroid_extensionNames[]	= { "Android Java Platform Encoder", "End" };

PortalBufferType_t			EncoderAndroid_inputTypeSupport[] = { PortalBufferType::YUV420, PortalBufferType::ARGB, PortalBufferType::BGRA };


#ifdef ENVIRONS_NATIVE_MODULE

static const InterfaceType_t	EncoderAndroid_interfaceTypes[]	= { InterfaceType::Encoder, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( EncoderAndroid_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( EncoderAndroid_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( EncoderAndroid );

#endif

// The TAG for prepending in log messages
#define CLASS_NAME	"EncoderAndroid"


namespace environs 
{

	EncoderAndroid::EncoderAndroid () : objectIndex ( -1 ), usePNG ( false ) 
	{
		name				= EncoderAndroid_extensionNames [0];

		CLogArgID ( "Construct [%s]", name );
		
		inputTypes				= EncoderAndroid_inputTypeSupport;
		inputTypesLength		= sizeof ( EncoderAndroid_inputTypeSupport ) / sizeof ( EncoderAndroid_inputTypeSupport [0] );

		keyframeCounter			= 0;
		keyframeHandled			= false;
	}


	EncoderAndroid::~EncoderAndroid ()
	{
		CLogArgID ( "Destruct [%s]", name );

		// Destroy java object

		if ( objectIndex >= 0 ) {
			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
				return;

			if ( env->CallStaticIntMethod ( g_EnvironsClass, g_DestroyInstance, objectIndex ) != objectIndex ) {
				CErrArgID ( "Destruct [%s]: Failed to destroy object [%i]", name, objectIndex );
			}

			if ( rc )
				g_JavaVM->DetachCurrentThread ();
		}
	}


	bool EncoderAndroid::Init ( int deviceID, int props, int Width, int Height, int FrameRate )
	{
		this->deviceID = deviceID;

		CLogID ( "Init" );

		width		= Width;
		height		= Height;

		encodedType = props ? DATA_STREAM_IMAGE_PNG : DATA_STREAM_IMAGE_JPEG;

		if ( objectIndex >= 0 )
			return false;

		int	success = 0;
        
		// Create java object and store the id ob the object for subsequent calls
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		success = env->CallStaticIntMethod ( g_EnvironsClass, g_CreateInstance, deviceID, InterfaceType::Encoder, 0 );
		if ( success > 0 ) {
			objectIndex = success;

			success = env->CallStaticIntMethod ( g_EnvironsClass, g_Encoder_Init, deviceID, objectIndex, props, width, height, FrameRate );
		}
		else success = 0;

		if ( rc )
			g_JavaVM->DetachCurrentThread ();

		CLogID ( "Init done." );
		return (success == 1);
	}


	int EncoderAndroid::Perform ( RenderContext * context )
	{
		int					ret				= -1;

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
				//bitmap = new Bitmap ( context->width, context->height, context->stride, PixelFormat32bppPARGB, (BYTE *) (((char *) context->renderedData) + 4) );
			}
			else if ( inputType == PortalBufferType::ARGBHandle ) {
				//bitmap = new Bitmap ( (HBITMAP)context->renderedDataHandle, NULL );
			}
			else if ( inputType == PortalBufferType::GDIBitmap )
				//bitmap = (Bitmap *) context->renderedData;

			// Encoder is free for use within the next worker thread
#ifndef ENABLE_WORKER_STAGES_LOCKS
			SetEvent ( context->eventEncoded );
#endif

			//CVerbArgID ( "Size: %i!", length );
			ret = 1;
			break;
		}

		return ret;
	}


} /* namespace environs */

#endif

