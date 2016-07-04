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
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import "Decoder.iOSX.h"

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Decoder/Decoder.Openh264.h"
using namespace environs;

#include "Core/Byte.Buffer.h"


#define	CLASS_NAME 	"DecoderIOSX"



namespace environs
{
    
	DecoderIOSX::DecoderIOSX ()
	{
		CLog ( "Construct" );
        
        decoder     = 0;
        idecoder    = 0;
        
        renderSurface   = 0;
        renderWidth     = 0;
        renderHeight    = 0;
        
        decodeImage = false;
#ifdef ENVIRONS_IOS
        outputRGBA  = false;
#else
        outputRGBA  = true;
#endif
        allocated   = false;
	}


	DecoderIOSX::~DecoderIOSX ()
	{
		CLogID ( "Destruct" );

		Release ( );
        
    }
    
    
    int DecoderIOSX::Init ( )
    {
        CVerbID ( "Init" );
        
        initialized = true;
        
        return true;
    }  
    
    
    bool DecoderIOSX::InitType ( int type )
    {
        CVerbArgID ( "InitType [%i]", type );
        
        bool success = false;
        
        // This initializer is called when the receiver actually received a stream init packet
        // which determines the stream type
        // If we support the stream type, then we return true. Otherwise we return false.
        
        if ( (type & DATA_STREAM_VIDEO) == DATA_STREAM_VIDEO )
        {
            // We support h264 streams.
            CVerb ( "InitType: Initialize h264 decoder." );
            
            // Try loading an appropriate decoder here.
#ifdef ENVIRONS_OSX
            if ( env->mod_PortalDecoder )
            {
                idecoder = (IPortalDecoder *) environs::API::CreateInstance ( env->mod_PortalDecoder, 0, InterfaceType::Decoder, deviceID, env );
                if ( !idecoder ) {
                    CErr ( "InitType: Failed to create user specified decoder. Fallback to default encoder." );
                }
                else {
                    while ( idecoder )
                    {
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
                            idecoder->avContextType = ENVIRONS_AVCONTEXT_SUBTYPE_RGB;
                            avContextType = idecoder->avContextType;
                            avContextSubType = idecoder->avContextSubType;
                        }
                        break;
                    }
                }
            }
            
            if ( success ) {
                CLogArg ( "InitType: Decoder stage using [%s]", idecoder->name );
                return true;
            }
            
#else
#ifndef ENABLE_IOS_NATIVE_H264_ONLY
            if ( environs::opt_useNativeDecoder || (environs.mod_PortalDecoder && strstr ( environs.mod_PortalDecoder, "openh264" ) ) )
            {
                idecoder = new DecoderOpenH264 ();
                while ( idecoder )
                {
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
            }
            
            if ( success )
                return true;
            
            if ( idecoder ) {
                delete idecoder;
                idecoder = 0;
            }
#endif
#endif
            decoder = [[DecoderIOSXH264 alloc] init];
            if ( decoder && [decoder Init] ) {
                decoder->outputRGBA = outputRGBA;
                decoder->parentDecoder = this;
                if ( [decoder Start] ) {
                    if ( renderSurface )
                        success = [decoder SetRenderSurface:renderSurface];
                    else
                        success = true;                    
                }
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
            return true;
        }
        
        return success;
    }
    

	void DecoderIOSX::Release ( )
	{
        CVerbID ( "Release");

		enabled = false;
        
        if ( decoder ) {
            decoder = 0;
            
            if ( outputRGBA ) {
                void * tmp = avContext;
                avContext = 0;
                if ( tmp ) {
                    free ( tmp );
                }                
            }
        }
        
        if ( idecoder ) {
            delete idecoder;
            idecoder = 0;
        }
        
		initialized = false;
	}


	bool DecoderIOSX::Start ()
	{
		CVerbID ( "Start" );

        bool ret = false;

        enabled = true;
        
        if ( decoder )
            ret = [decoder Start];
        
        else if ( idecoder )
            idecoder->Start ();
        
		return ret;
	}


	void DecoderIOSX::Stop ()
	{
		CVerbID ( "Stop" );

		enabled = false;
        
        if ( decoder )
            [decoder Stop];
        
        else if ( idecoder )
            idecoder->Stop ();
	}

    
    bool DecoderIOSX::SetRenderResolution ( int width, int height )
    {
        renderWidth     = width;
        renderHeight    = height;
        
        return true;
    }
    
    
    bool DecoderIOSX::SetRenderSurface ( void * penv, void * newSurface, int width, int height )
    {
        bool ret = false;
        
        renderSurface   = newSurface;
        renderWidth     = width;
        renderHeight    = height;
        
        if ( decoder )
            ret = [decoder SetRenderSurface:newSurface];
        else if ( idecoder )
            ret = idecoder->SetRenderSurface ( penv, newSurface, width, height );
        else {
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
	int DecoderIOSX::Perform ( int type, char * payload, int payloadSize )
	{
		CVerbVerbID ( "Perform" );

		int ret = 0;

        if ( decoder ) {
            ret = [decoder Perform:type withData:payload withSize:payloadSize];
        }
        else
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

    
    
    
    int DecoderIOSX::AllocateResources ( )
    {
        CVerbID ( "AllocateResources" );
        
        int success     = 1;
        
        if ( idecoder )
            success = idecoder->AllocateResources ( );
        
        CVerbID ( "AllocateResources succeeded" );
        
        return success;
    }
    
    
    int DecoderIOSX::ReleaseResources ()
    {
        CVerbID ( "ReleaseResources" );
        
        int success     = 1;
        
        if ( idecoder )
            success = idecoder->ReleaseResources ( );
        
        CVerbID ( "ReleaseResources succeeded" );
        
        return success;
    }


} /* namespace environs */
