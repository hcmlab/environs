/**
 * Decoder using openh264
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

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Core/Byte.Buffer.h"
#include "Decoder.OpenH264.h"
#include "Environs.Build.Lnk.h"
using namespace environs;


#define	CLASS_NAME 	"Decoder.OpenH264 . . . ."

static const char					*		DecoderOpenH264_extensionNames[]	= { "Openh264 Decoder", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	DecoderOpenH264_interfaceTypes[]	= { InterfaceType::Decoder, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( DecoderOpenH264_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( DecoderOpenH264_interfaceTypes );


/**
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();


#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( DecoderOpenH264 );

#endif
#endif

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

namespace environs
{
    
	DecoderOpenH264::DecoderOpenH264 ( )
	{
        name                = DecoderOpenH264_extensionNames [ 0 ];        
        
        CLogArg ( "Construct: [ %s ]", name );
        
        decoder             = 0;
        
        avContextType       = DECODER_AVCONTEXT_TYPE_PIXELS;

		avContextSubType    = ENVIRONS_AVCONTEXT_SUBTYPE_RGB;
	}


	DecoderOpenH264::~DecoderOpenH264 ( )
	{
		CVerbID ( "Destruct" );

		Dispose ( );
	}
    

	/**
	* Initialize the decoder, that is acquire the decoder resources such as frames, context, etc.
	*
	*	@return	success
	* */
	bool DecoderOpenH264::InitInstance ( bool useLock )
	{
		CVerbID ( "InitInstance" );

        if ( initialized )
            return true;
        
		if ( !InitLibOpenH264 ( env, deviceID ) ) {
			CErrID ( "InitInstance: Failed to initialize access to libopenh264!" );
			return false;
		}
        
		if ( useLock && !LockAcquire ( &stateMutex, "InitInstance" ) ) {
			return false;
		}
        
        SDecodingParam params;
        Zero ( params );
        
        long success = dWelsCreateDecoder ( &decoder );
        if ( success != 0 ) {
            CErr ( "InitInstance: Failed to create decoder." );
            goto Failed;
        }
        
        params.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
        //for Parsing only, the assignment is mandatory
        params.bParseOnly = false;
        //params.eOutputColorFormat = videoFormatBGRA;
        params.eOutputColorFormat = videoFormatI420;
        
        success = decoder->Initialize ( &params );
        if ( success != 0 ) {
            CErr ( "InitInstance: Failed to initialize decoder." );
            goto Failed;
        }
        
		initialized = true;

		if ( useLock && !LockRelease ( &stateMutex, "InitInstance" ) )
			return false;

		CVerbID ( "InitInstance succeeded" );

		return true;

	Failed:

		if ( useLock && pthread_mutex_unlock ( &stateMutex ) ) {
			CErrID ( "InitInstance: Failed to release mutex on decoder state!" );
		}

		return false;
	}


	void DecoderOpenH264::Dispose ( bool useLock )
	{
        CVerbArgID ( "Dispose [%s]", useLock ? "lock" : "no lock" );

		enabled = false;
        
        if ( !allocated )
            useLock = false;
        
		if ( useLock && !LockAcquire ( &stateMutex, "Dispose" ) ) {
			return;
		}

        if ( decoder ) {
            decoder->Uninitialize ();
            
            dWelsDestroyDecoder ( decoder );
            
            decoder = 0;
        }
        ReleaseLibOpenH264 ();
        
        initialized = false;
        
		if ( useLock ) {
            LockReleaseV ( &stateMutex, "Dispose" );
		}
	}


	bool DecoderOpenH264::Start ( )
	{
		CVerbID ( "Start" );

		if ( pthread_cond_mutex_lock ( &stateMutex ) ) {
			CErrID ( "Start: Failed to acquire mutex!" );
			return false;
		}

		enabled = true;

		if ( pthread_cond_mutex_unlock ( &stateMutex ) ) {
			CErrID ( "Start: Failed to release mutex!" );
			return false;
		}
		return true;
	}


	void DecoderOpenH264::Stop ( )
	{
		CVerbID ( "Stop" );

		enabled = false;

		DecoderBase::Stop ();
	}


	/**
	* Decode the buffer, that is acquire the decoder resources such as frames, context, etc.
	*
	*	@return	success
	* */
	int DecoderOpenH264::Perform ( int type, char * payload, int payloadSize )
	{
		CVerbVerbID ( "Perform" );

		int ret = 0;
        
        unsigned char * outputYUV [3];

        outputYUV [ 0 ] = 0; outputYUV [ 1 ] = 0; outputYUV [ 2 ] = 0;
        
        SBufferInfo         bufferInfo;
        Zero ( bufferInfo );
        
		if ( pthread_mutex_lock ( &stateMutex ) ) {
			CErrID ( "Perform: Failed to aquire mutex on decoder state!" );
			return false;
		}
        
        ret = decoder->DecodeFrameNoDelay ( (const unsigned char *)payload, payloadSize, (unsigned char **) &outputYUV, &bufferInfo );
		if ( ret != dsErrorFree ) {
			CVerbArgID ( "Perform: Failed decode [%i]", ret );
            ret = 0;
            goto Finish;
        }
        
		if ( bufferInfo.iBufferStatus && outputYUV [0] && outputYUV [1] && outputYUV [2] ) {
			/*if ( bufferInfo.UsrData.sSystemBuffer.iHeight != height || bufferInfo.UsrData.sSystemBuffer.iWidth != width ) {
				CWarnID ( "Perform: width/height [%i/%i] changed to [%i/%i]", width, height, bufferInfo.UsrData.sSystemBuffer.iWidth, bufferInfo.UsrData.sSystemBuffer.iHeight );
				width = bufferInfo.UsrData.sSystemBuffer.iWidth;
				height = bufferInfo.UsrData.sSystemBuffer.iHeight;
			}
			*/
			ret = ConvertI420ToSubRGB ( outputYUV [0], bufferInfo.UsrData.sSystemBuffer.iStride [0], outputYUV [1], outputYUV [2], bufferInfo.UsrData.sSystemBuffer.iStride [1] );
		}

		CVerbVerbID ( "Perform: done" );

	Finish:
		if ( pthread_mutex_unlock ( &stateMutex ) ) {
			CErrID ( "Perform: Failed to release mutex on decoder state!" );
		}

		return ret;
	}



} /* namespace environs */

#endif



