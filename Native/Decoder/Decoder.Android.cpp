/**
 * Base codec using native iOS 
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#ifdef ANDROID

#include "Decoder.Android.h"

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Environs.Modules.h"
#include "Core/Byte.Buffer.h"
using namespace environs;

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>

#include <stdio.h>


#define	CLASS_NAME 	"DecoderAndroid"

static const char		*		DecoderAndroid_extensionNames[]	= { "Android Native Decoder", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	DecoderAndroid_interfaceTypes[]	= { InterfaceType::Decoder, InterfaceType::Unknown };


/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( DecoderAndroid_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( DecoderAndroid_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( DecoderAndroid );


/**
 * SetEnvironsMethods
 *
 *	Injects environs runtime methods.
 *
 */
BUILD_INT_SETENVIRONSOBJECT ();

#endif


typedef struct _ANativeAVPack
{
    ANativeWindow	* 		renderWindow;
    ANativeWindow_Buffer 	windowBuffer;
    int                     width;
    int                     height;
    //jobject					surfaceBitmap;
    //void			*		surfaceBitmapBuffer;
}
ANativeAVPack;


namespace environs
{
    
	DecoderAndroid::DecoderAndroid ()
	{
		CLog ( "Construct" );
        
        idecoder        = 0;
        instanceAVPack  = 0;
        
        renderSurface   = 0;
        renderWidth     = 0;
        renderHeight    = 0;
        
        decodeImage = false;
        outputRGBA  = true;
        allocated   = false;
	}


	DecoderAndroid::~DecoderAndroid ()
	{
		CLogID ( "Destruct" );

        Release ( );
        
        if ( allocated )
        {
            if ( pthread_mutex_destroy	( &stateMutex ) ) {
                CVerb ( "Init: Failed to destroy stateMutex!" );
            }
        }
    }
    
    
	ptRenderCallback DecoderAndroid::GetRenderCallback ( int &callbackType )
    {
        CVerbID  ( "GetRenderCallback" );
        
		callbackType = RENDER_CALLBACK_TYPE_DECODER;

        return (ptRenderCallback) renderCallback;
    }
    
    
    int DecoderAndroid::Init ( )
    {
        CVerbID ( "Init" );
        
        
        if ( !allocated )
        {
            Zero ( stateMutex );
            if ( pthread_mutex_init	( &stateMutex, NULL ) ) {
                CErr ( "Init: Failed to init stateMutex!" );
                return false;
            }
            
            allocated = true;
        }
        
        initialized = true;
        
        return true;
    }  
    
    
    bool DecoderAndroid::InitType ( int type )
    {
        CVerbArgID ( "InitType [%i]", type );
        
        bool success = false;
        
        // This initializer is called when the receiver actually received a stream init packet
        // which determines the stream type
        // If we support the stream type, then we return true. Otherwise we return false.

		if ( ( type & DATA_STREAM_VIDEO ) == DATA_STREAM_VIDEO )
        {
            // We support h264 streams.
            CVerb ( "InitType: Initialize video decoder." );
            
            // Try loading an appropriate decoder here.
            if ( ((environs::Instance *)pEnvirons)->mod_PortalDecoder )
                idecoder = (IPortalDecoder *) CreateEnvInstance ( ((environs::Instance *)pEnvirons)->mod_PortalDecoder, 0, InterfaceType::Decoder, deviceID );
            
            if ( !idecoder )
                idecoder = (IPortalDecoder *) CreateEnvInstance ( LIBNAME_Decoder_LibOpenH264, 0, InterfaceType::Decoder, deviceID );
            
            while ( idecoder )
            {
                idecoder->SetWidthHeight ( width, stride, height );
                idecoder->avContextSubType = ENVIRONS_AVCONTEXT_SUBTYPE_ABGR;
                
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
            // We support image streams.
            
            if ( (type & DATA_STREAM_IMAGE_JPEG) == DATA_STREAM_IMAGE_JPEG ) {
                
                // jpeg
                avContextType = DECODER_AVCONTEXT_TYPE_JPG;
                CVerb ( "InitType: Initialize jpeg decoder." );
                return true;
            }
            
            if ( (type & DATA_STREAM_IMAGE_PNG) == DATA_STREAM_IMAGE_PNG ) {
                // png
                avContextType = DECODER_AVCONTEXT_TYPE_PNG;
                CVerb ( "InitType: Initialize png decoder." );
                return true;
            }
            // Try loading an appropriate decoder here.
            //return true;
        }
        
        return success;
    }
    

	void DecoderAndroid::Release ( )
	{
        CVerbID ( "Release");

		enabled = false;
        
        if ( idecoder ) {
            delete idecoder;
            idecoder = 0;
        }
        
		initialized = false;
	}


	bool DecoderAndroid::Start ()
	{
		CVerbID ( "Start" );

        bool ret = false;

        enabled = true;
        
        if ( idecoder )
            idecoder->Start ();
        
		return ret;
	}


	void DecoderAndroid::Stop ()
	{
		CVerbID ( "Stop" );

		enabled = false;
        
        if ( idecoder )
            idecoder->Stop ();
    }
    
    
    bool DecoderAndroid::renderCallback ( int type, void * surface, void * _decoderOrByteBuffer )
    {
        CVerbVerb  ( "renderCallback" );
        
		if ( type != RENDER_CALLBACK_TYPE_DECODER )
        {
            CVerb ( "renderCallback: not a decoder packet." );
            return true;
        }
        
        DecoderAndroid * decoder = (DecoderAndroid *) _decoderOrByteBuffer;
        if ( !decoder || !decoder->avContext ) {
            CErr ( "renderCallback: invalid decoder" );
            return false;
        }
        
        ANativeAVPack * aNativepack = (ANativeAVPack *) decoder->instanceAVPack;
        if ( !aNativepack ) {
            CErr ( "renderCallback: invalid aNativepack" );
            return false;
        }
        
        if ( aNativepack->width != decoder->width || aNativepack->height != decoder->height )
        {
            CVerbArg ( "renderCallback: changing buffers geometry width[%i] height [%i]", decoder->width, decoder->height );
            
            aNativepack->width = decoder->width;
            aNativepack->height = decoder->height;
            
            if ( ANativeWindow_setBuffersGeometry ( aNativepack->renderWindow, aNativepack->width, aNativepack->height, WINDOW_FORMAT_RGBA_8888 ) < 0 ) {
                CErr ( "renderCallback: Failed to set buffers geometry!" );
            }
        }
        
        
        if ( ANativeWindow_lock ( aNativepack->renderWindow, &aNativepack->windowBuffer, NULL ) < 0 ) {
            CWarn ( "renderCallback: cannot lock window" );
        }
        else {
            //CLogArg ( "copy buffer %d:%d:%d", width, height, width*height*4 );
            //CLogArg ( "window buffer: %d:%d:%d", windowBuffer.width, windowBuffer.height, windowBuffer.stride );
            
            CVerbArg ( "renderCallback: copying stride[%i] height [%i]", decoder->stride, decoder->height );
            
            memcpy ( aNativepack->windowBuffer.bits, decoder->avContext, decoder->avContextSize );
            
            ANativeWindow_unlockAndPost ( aNativepack->renderWindow );
        }
        
        return false;
    }

    
    bool DecoderAndroid::SetRenderResolution ( int width, int height )
    {
        CVerbArg ( "SetRenderResolution: width[%i] height [%i]", width, height );
        
        renderWidth     = width;
        renderHeight    = height;
        
        return true;
    }
    
    
    bool DecoderAndroid::SetRenderSurface ( void * penv, void * newSurface, int surfaceWidth, int surfaceHeight )
    {
        CVerbID ( "SetRenderSurface" );
        
        if ( !newSurface || !penv )
            return false;

		if ( !LockAcquire ( &stateMutex, "SetRenderSurface" ) )
			return false;
        
        ReleaseRenderSurface ( false );
        
        ANativeAVPack   *   aNativepack;
        bool                ret             = false;
        JNIEnv          *   jenv;
        jobject             jSurface;
        void            *   bitmapBuffer    = 0;
        
        instanceAVPack = calloc ( 1, sizeof(ANativeAVPack) );
        if ( !instanceAVPack ) {
            CErrID ( "SetRenderSurface: Failed to allocate memory for android native AV pack!" );
            goto Finish;
        }
        
        aNativepack = (ANativeAVPack *)	instanceAVPack;
        if ( !aNativepack ) {
            CErrID ( "SetRenderSurface: No avContext!" );
            goto Finish;
        }
        
        jenv        = (JNIEnv *) penv;
        jSurface    = (jobject) newSurface;
        
        aNativepack->renderWindow = ANativeWindow_fromSurface ( jenv, jSurface );
        if ( !aNativepack->renderWindow ) {
            CErrID ( "SetRenderSurface: Failed to query native window from render surface!" );
            goto Finish;
        }
        
        aNativepack->width = width;
        aNativepack->height = height;
        
        CVerbArg ( "SetRenderSurface: changing buffers geometry width[%i] height [%i]", width, height );
        
        if ( ANativeWindow_setBuffersGeometry ( aNativepack->renderWindow, aNativepack->width, aNativepack->height, WINDOW_FORMAT_RGBA_8888 ) < 0 ) {
            CErr ( "SetRenderSurface: Failed to set buffers geometry!" );
        }
        
	Finish:
		if ( !LockRelease ( &stateMutex, "SetRenderSurface" ) )
			return false;
        return ret;
    }
    
    
    /**
     * Release the render surface.
     * */
    void DecoderAndroid::ReleaseRenderSurface ( bool useLock )
    {
        CVerbID ( "ReleaseRenderSurface" );

		if ( useLock )
			LockAcquireV ( &stateMutex, "ReleaseRenderSurface" );
        
        if ( instanceAVPack )
        {
            ANativeAVPack * avpack = (ANativeAVPack *)instanceAVPack;
            if ( avpack ) {
                if ( avpack->renderWindow ) {
                    CVerbID ( "ReleaseRenderSurface: Releasing renderSurface" );
                    
                    ANativeWindow_release ( avpack->renderWindow );
                }
            }
            
            free ( instanceAVPack );
            instanceAVPack = 0;
        }

		if ( useLock )
			LockReleaseV ( &stateMutex, "ReleaseRenderSurface" );
    }
    

	/**
	* Decode the buffer, that is acquire the decoder resources such as frames, context, etc.
	*
	*	@return	success
	* */
	int DecoderAndroid::Perform ( int type, char * payload, int payloadSize )
	{
		CVerbVerbID ( "Perform" );

        int ret = 0;
        
        if ( idecoder ) {
            ret = idecoder->Perform ( type, payload, payloadSize );
            if ( ret == 1 ) {
                avContext = idecoder->avContext;
                avContextSize = idecoder->avContextSize;
            }
        }

		CVerbVerbID ( "Perform: done" );

		return ret;
	}

    
    
    
    int DecoderAndroid::AllocateResources ( )
    {
        CVerbID ( "AllocateResources" );
        
        int success     = 1;
        
        if ( idecoder )
            success = idecoder->AllocateResources ( );
        
        CVerbID ( "AllocateResources succeeded" );
        
        return success;
    }
    
    
    int DecoderAndroid::ReleaseResources ()
    {
        CVerbID ( "ReleaseResources" );
        
        int success     = 1;
        
        if ( idecoder )
            success = idecoder->ReleaseResources ( );
        
        CVerbID ( "ReleaseResources succeeded" );
        
        return success;
    }


} /* namespace environs */

#endif

