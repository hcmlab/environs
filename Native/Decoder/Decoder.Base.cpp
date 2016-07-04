/**
 * Decoder Base access methods and states
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

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Decoder.Base.h"

#define	CLASS_NAME 	"Decoder.Base . . . . . ."


namespace environs
{

	DecoderBase::DecoderBase ( )
	{
		CVerb ( "Construct" );

        allocated           = false;
        
		enabled				= false;

		initialized         = false;

		width				= 0;
		height				= 0;
		renderWidth         = 0;
		renderHeight        = 0;
	}


	DecoderBase::~DecoderBase ( )
	{
		CVerbID ( "Destruct" );

		Release ( );
        
        
        if ( allocated )
        {
            allocated = false;
            
            LockDisposeA ( stateMutex );
            
            CondDisposeA ( stateSignal );
        }
	}

    
    bool DecoderBase::InitType ( int type )
    {
        // This initializer is called when the receiver actually received a stream init packet
        // which determines the stream type
        // If we support the stream type, then we return true. Otherwise we return false.
        
        if ( (type & DATA_STREAM_VIDEO) == DATA_STREAM_VIDEO ) {
            // We support h264 streams.
            CVerb ( "InitType: Initialize h264/h265 decoder." );
            
            // Try loading an appropriate decoder here.
            //return true;
        }
        
        else if ( (type & DATA_STREAM_IMAGE) == DATA_STREAM_IMAGE ) {
            // We support image streams.
            
            if ( (type & DATA_STREAM_IMAGE_JPEG) == DATA_STREAM_IMAGE_JPEG ) {
                // jpeg
                CVerb ( "InitType: Initialize jpeg decoder." );
            }
            
            if ( (type & DATA_STREAM_IMAGE_PNG) == DATA_STREAM_IMAGE_PNG ) {
                // png
                CVerb ( "InitType: Initialize png decoder." );
            }
            // Try loading an appropriate decoder here.
            //return true;
        }
        
        return false;
    }
    
    
	void DecoderBase::Release ( )
	{
		CVerbID ( "Release" );
	}


	int DecoderBase::Init ( )
	{
        CVerbID ( "Init" );
        
        if ( !allocated )
		{
			Zero ( stateSignal );
            if ( pthread_cond_manual_init	( &stateSignal, NULL ) ) {
                CErr ( "Init: Failed to init stateSignal!" );
                return false;
            }
            
            if ( !LockInit ( &stateMutex ) )
                return false;
            
            allocated = true;
        }

		return InitInstance ();
	}
    
    
	bool DecoderBase::InitInstance ( bool useLock )
	{
		CVerbID ( "InitInstance" );
        
        initialized = true;
        return 1;
	}
    
    
	bool DecoderBase::Start ( )
	{
		CVerbID ( "Start" );
        
		return true;
	}
    

	void DecoderBase::Stop ( )
	{
		CVerbID ( "Stop" );
	}    
    

	int DecoderBase::Perform ( int type, char * payload, int payloadSize )
    {
        CVerbVerbID ( "Perform: not implemented." );
        
		return 0;
	}


	bool DecoderBase::SetRenderResolution ( int width, int height )
	{
		CVerbID ( "SetRenderResolution" );
        
		if ( !LockAcquire ( &stateMutex, "SetRenderResolution" ) )
			return false;
        
		renderWidth = width;
		renderHeight = height;
        
		if ( !LockRelease ( &stateMutex, "SetRenderResolution" ) )
			return false;
        
		CVerbID ( "SetRenderResolution succeeded" );
        
		return true;
	}
    
    
	bool DecoderBase::SetStreamResolution ( int width, int stride, int height )
	{
		CVerbID ( "SetStreamResolution" );
        
		if ( !LockAcquire ( &stateMutex, "SetStreamResolution" ) )
			return false;

		this->width		= width;
        this->stride    = stride;
		this->height	= height;
        
		if ( !LockRelease ( &stateMutex, "SetStreamResolution" ) )
			return false;

		CVerbID ( "SetStreamResolution succeeded" );

		return true;
    }
    
    
    int DecoderBase::ConvertI420ToSubRGB ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
    {
        int ret = 0;
        
		if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_BGRA )
            ret = ConvertI420ToBGRA ( yData, yStride, uData, vData, uvStride );
		else if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_ABGR )
			ret = ConvertI420ToABGR ( yData, yStride, uData, vData, uvStride );
		else if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_BGR )
			ret = ConvertI420ToBGR ( yData, yStride, uData, vData, uvStride );
		else if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_RGBA )
            ret = ConvertI420ToRGBA ( yData, yStride, uData, vData, uvStride );
		else if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_RGB )
            ret = ConvertI420ToRGB ( yData, yStride, uData, vData, uvStride );
        
        return ret;
    }
    
    
#define fitUCHAR(v)     (v > 0xFF ? 0xFF : (v < 0 ? 0 : v))
    
#define YUV_DEF_Y(Value)        int yuvY = (Value - 16) * 298
#define YUV_DEF_U(Value)        int yuvU = Value - 128
#define YUV_DEF_V(Value)        int yuvV = Value - 128
    
#define YUV2R(Res)              Res = ((yuvY + 409 * yuvV + 128) >> 8); Res = fitUCHAR ( Res )
#define YUV2G(Res)              Res = ((yuvY - 100 * yuvU - 208 * yuvV + 128) >> 8); Res = fitUCHAR ( Res )
#define YUV2B(Res)              Res = ((yuvY + 516 * yuvU + 128) >> 8); Res = fitUCHAR ( Res )
    
    
    int DecoderBase::ConvertI420ToABGR ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
    {
        CVerbVerbID ( "ConvertI420ToABGR" );
        
        if ( !avContext && PrepareRGBAResources () != 1 )
            return 0;
        
        unsigned int * data = (unsigned int *)avContext;
        
        int uvRunner = 0;
        int uvPixel = 0;
        int uvWidth = uvStride;
        
        for ( int row = 0; row < (int) height; row++ )
        {
            uvPixel = (row >> 1) * uvWidth;
            uvRunner = 0;
            
            int U = uData [ uvPixel ];
            int V = vData [ uvPixel ];
            
            for ( int col = 0; col < (int) width; col++ )
            {
                int pixel = row * width + col;
                int ypixel = row * yStride + col;
                
                int Y = yData [ ypixel ];
                
                int r, g, b;
                
                YUV_DEF_Y ( Y );
                YUV_DEF_U ( U );
                YUV_DEF_V ( V );
                
                YUV2R ( r );
                YUV2G ( g );
                YUV2B ( b );
                
                unsigned int value = b << 16 | g << 8 | r;
                data [ pixel ] = value;
                
                uvRunner++;
                if ( uvRunner >= 2 ) {
                    uvRunner = 0;
                    uvPixel++;
                    
                    U = uData [uvPixel];
                    V = vData [uvPixel];
                }
            }
        }
        
        return 1;
    }
    
    
    int DecoderBase::ConvertI420ToRGBA ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
    {
        CVerbVerbID ( "ConvertI420ToRGBA" );
        
        if ( !avContext && PrepareRGBAResources () != 1 )
            return 0;
        
        unsigned int * data = (unsigned int *)avContext;
        
        int uvRunner = 0;
        int uvPixel = 0;
        int uvWidth = uvStride;
        
        for ( int row = 0; row < (int) height; row++ )
		{
			uvPixel = (row >> 1) * uvWidth;
			uvRunner = 0;

			int U = uData [uvPixel];
			int V = vData [uvPixel];
            
            for ( int col = 0; col < (int) width; col++ )
            {
                int pixel = row * width + col;
                int ypixel = row * yStride + col;
                
                int Y = yData [ ypixel ];
                
                int r, g, b;
                
                YUV_DEF_Y ( Y );
                YUV_DEF_U ( U );
                YUV_DEF_V ( V );
                
                YUV2R ( r );
                YUV2G ( g );
                YUV2B ( b );
                
                unsigned int value = r << 24 | g << 16 | b << 8;

                data [ pixel ] = value;
                
                uvRunner++;
                if ( uvRunner >= 2 ) {
                    uvRunner = 0;
                    uvPixel++;
                    
                    U = uData [uvPixel];
                    V = vData [uvPixel];
                }
            }
        }
        
        return 1;
    }
    
    
    int DecoderBase::ConvertI420ToBGRA ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
    {
        CVerbVerbID ( "ConvertI420ToBGRA" );
        
        if ( !avContext && PrepareRGBAResources () != 1 )
            return 0;
        
        unsigned int * data = (unsigned int *)avContext;        
        
        int uvRunner = 0;
        int uvPixel = 0;
        int uvWidth = uvStride;
        
        for ( int row = 0; row < (int) height; row++ )
		{
			uvPixel = (row >> 1) * uvWidth;
			uvRunner = 0;

			int U = uData [uvPixel];
			int V = vData [uvPixel];
            
            for ( int col = 0; col < (int) width; col++ )
            {
                int pixel = row * width + col;
                int ypixel = row * yStride + col;
                
                int Y = yData [ ypixel ];
                
                int r, g, b;
                
                YUV_DEF_Y ( Y );
                YUV_DEF_U ( U );
                YUV_DEF_V ( V );
                
                YUV2R ( r );
                YUV2G ( g );
                YUV2B ( b );
                
                unsigned int value = b << 24 | g << 16 | r << 8;
                data [ pixel ] = value;
                
                uvRunner++;
                if ( uvRunner >= 2 ) {
                    uvRunner = 0;
                    uvPixel++;
                    
                    U = uData [uvPixel];
                    V = vData [uvPixel];
                }
            }
        }
        
        return 1;
    }
    
    
    int DecoderBase::ConvertI420ToRGB ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
    {
        CVerbVerbID ( "ConvertI420ToRGB" );
        
        if ( !avContext && PrepareRGBAResources () != 1 )
            return 0;
        
        unsigned char * data = (unsigned char *)avContext;
        
        int uvRunner = 0;
        int uvPixel = 0;
        int uvWidth = uvStride;
        
		for ( int row = 0; row < (int) height; row++ )
		{
			uvPixel = (row >> 1) * uvWidth;
			uvRunner = 0;

			int U = uData [uvPixel];
			int V = vData [uvPixel];
            
			for ( int col = 0; col < (int) width; col++ )
            {
                int pixel = row * width + col;
                int ypixel = row * yStride + col;
                
                int Y = yData [ ypixel ];
                
                int r, g, b;
                
                YUV_DEF_Y ( Y );
                YUV_DEF_U ( U );
                YUV_DEF_V ( V );
                
                YUV2R ( r );
                YUV2G ( g );
                YUV2B ( b );
                
                data [ ( pixel * 3 ) ] = (unsigned char) r;
				data [(pixel * 3) + 1] = (unsigned char) g;
				data [(pixel * 3) + 2] = (unsigned char) b;
                
				uvRunner++;
				if ( uvRunner >= 2 ) {
					uvRunner = 0;
                    uvPixel++;
                    
                    U = uData [uvPixel];
                    V = vData [uvPixel];
				}
            }
         }
        
        return 1;
	}


	int DecoderBase::ConvertI420ToBGR ( unsigned char * yData, int yStride, unsigned char * uData, unsigned char * vData, int uvStride )
	{
		CVerbVerbID ( "ConvertI420ToBGR" );

		if ( !avContext && PrepareRGBAResources () != 1 )
			return 0;

		unsigned char * data = (unsigned char *) avContext;

		int uvRunner = 0;
		int uvPixel = 0;
		int uvWidth = uvStride;

		for ( int row = 0; row < (int) height; row++ )
		{
			uvPixel = (row >> 1) * uvWidth;
			uvRunner = 0;

			int U = uData [uvPixel];
			int V = vData [uvPixel];

			for ( int col = 0; col < (int) width; col++ )
			{
				int pixel = row * width + col;
				int ypixel = row * yStride + col;

				int Y = yData [ypixel];

				int r, g, b;

				YUV_DEF_Y ( Y );
				YUV_DEF_U ( U );
				YUV_DEF_V ( V );

				YUV2R ( r );
				YUV2G ( g );
				YUV2B ( b );

				data [(pixel * 3)] = (unsigned char) b;
				data [(pixel * 3) + 1] = (unsigned char) g;
				data [(pixel * 3) + 2] = (unsigned char) r;

				uvRunner++;
				if ( uvRunner >= 2 ) {
					uvRunner = 0;
					uvPixel++;

					U = uData [uvPixel];
					V = vData [uvPixel];
				}
			}
		}

		return 1;
	}


    int DecoderBase::PrepareRGBAResources ( )
    {
        CVerbID ( "PrepareRGBAResources" );
        
        if ( avContextType == DECODER_AVCONTEXT_TYPE_PIXELS ) {

			if ( avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_BGR || avContextSubType == ENVIRONS_AVCONTEXT_SUBTYPE_RGB ) {
				stride = width * 3;
			}
			else
				stride = width * 4;

            int reqSize = stride * height;
            
            if ( reqSize > avContextSize ) {
                ReleaseResources ();
                
                avContext = calloc ( 1, reqSize );
                if ( !avContext )
                    return 0;
                
                avContextSize = reqSize;
            }
        }
        
        CVerbID ( "PrepareRGBAResources succeeded" );
        
        return 1;
    }
    
    
	int DecoderBase::AllocateResources ( )
	{
		CVerbID ( "AllocateResources" );
        
		if ( !LockAcquire ( &stateMutex, "AllocateResources" ) )
			return false;

        int success   = PrepareRGBAResources ();
        
		if ( !LockRelease ( &stateMutex, "AllocateResources" ) )
			return false;

		CVerbID ( "AllocateResources succeeded" );

		return success;
    }
    
    
    int DecoderBase::ReleaseResources ()
    {
        CVerbID ( "ReleaseResources" );
        
        if ( avContextType == DECODER_AVCONTEXT_TYPE_PIXELS ) {
            if ( avContext ) {
                free ( avContext );
                avContext = 0;
            }
        }
        
        return 1;
    
    }


} /* namespace environs */
