/**
 * Interface for a portal encoder/compressor
   (create transport packages)
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
#   define DEBUGVERB
#   define ENCODERDEBUGVERB
#endif

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Encoder/Encoder.Base.h"
#include "Interfaces/IPortal.Renderer.h"
#include "Portal.Worker.Stages.h"
#include "stdint.h"

#define CLASS_NAME	"Encoder.Base . . . . . ."

namespace environs
{
	PortalBufferType_t	EncoderBase_inputTypeSupport[] = { PortalBufferType::YUV420, PortalBufferType::YV12, PortalBufferType::YUY2, PortalBufferType::ARGB, PortalBufferType::BGRA, PortalBufferType::GDIBitmap };


	bool EncoderBase::ApplyInput ()
	{
		CLogID ( "ApplyInput" );

		switch ( inputType )
		{
		case environs::PortalBufferType::ARGB:
			processor = &EncoderBase::EncodeARGB;
			break;
		case environs::PortalBufferType::BGRA:
			processor = &EncoderBase::EncodeBGRA;
			break;
		case environs::PortalBufferType::YV12:
			processor = &EncoderBase::EncodeYV12;
			break;
		case environs::PortalBufferType::YUY2:
			processor = &EncoderBase::EncodeYUY2;
			break;
		case environs::PortalBufferType::GDIBitmap:
			processor = &EncoderBase::EncodeGDIBitmap;
			break;
		default:
			break;
		}

		return (processor != 0);
	}


	EncoderBase::EncoderBase () :
		/** Default initialization */
		cacheCount ( 0 ), yuvBuffer ( 0 ), yuvBufferSize ( 0 ), uCache ( 0 ), vCache ( 0 ), processor ( 0 )
	{
		CVerbArgID ( "Construct: [%s]", name );

		inputTypes			= EncoderBase_inputTypeSupport;
		inputTypesLength	= sizeof ( EncoderBase_inputTypeSupport ) / sizeof ( EncoderBase_inputTypeSupport [0] );
	}


	EncoderBase::~EncoderBase ()
	{
		CLogID ( "Destruct..." );

        DisposeCaches ();

		CVerbID ( "Destruct destroyed." );
	}


    void EncoderBase::DisposeCaches ()
    {
        if ( yuvBuffer ) {
            CVerbID ( "DisposeCaches: releasing yuv buffer..." );
            delete[] yuvBuffer;
            yuvBuffer = 0;
        }
        yuvBufferSize = 0;

        if ( uCache ) {
            CVerbID ( "DisposeCaches: releasing u cache..." );
            delete[] uCache;
            uCache = 0;
        }

        if ( vCache ) {
            CVerbID ( "DisposeCaches: releasing v cache..." );
			delete[]  vCache;
            vCache = 0;
        }
    }


	int EncoderBase::Perform ( RenderContext * context )
	{
		if ( !processor )
			return -1;

		char			*	output		= 0;
		unsigned int		payloadSize = 0;

		while ( context )
		{
			if ( context->hasChanged )
				cacheCount = 0;
			else {
				cacheCount++;
				if ( cacheCount > 20 ) {
					CVerbVerbID ( "Perform: Not encoding. Not sending. Compare cached." );
					break;
				}
			}

			payloadSize = ((*this.*processor) ((char *) context->renderedData, output, context));

			if ( payloadSize > 0 && output ) {
				if ( !ResizeBuffer ( context, payloadSize + 4 ) ) {
					CErrID ( "Perform: Failed to resize output buffer." );
					break;
				}

				frameCounter++;
				context->frameCounter = frameCounter;

				memcpy ( BYTEBUFFER_DATA_POINTER_START ( context->outputBuffer ), output, payloadSize );
			}
			else if ( payloadSize == 0 ) {
				CVerbID ( "Perform: Encoded buffer is 0." );
			}
			else
				return 0;
			return 1;
		}

		return 0;
	}


	int EncoderBase::EncodeGDIBitmap ( char * source, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeGDIBitmap" );

		source = (((environs::WorkerStages *)stages)->render)->GetBuffer ( context );
		if ( !source ) {
			CErrID ( "EncodeGDIBitmap: Failed to get buffer from renderer." );
			return 0;
		}

		int payloadSize = EncodeARGB ( source, output, context );
		(((environs::WorkerStages *)stages)->render)->ReleaseBuffer ( context );

		return payloadSize;
	}


	int EncoderBase::EncodeYV12 ( char * source, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeYV12" );

		unsigned int size = width * height * 2;
		if ( yuvBufferSize != size ) {
            DisposeCaches ();
		}

		if ( !yuvBuffer ) {
			yuvBuffer = new char[ size ];
			if ( !yuvBuffer ) {
				return 0;
			}
			yuvBufferSize = size;
		}

		int planeSize = width * height;
		int planeSizeCx = planeSize >> 2;

		memcpy ( yuvBuffer, source, planeSize );
		memcpy ( yuvBuffer + planeSize + planeSizeCx, source + planeSize, planeSizeCx );
		memcpy ( yuvBuffer + planeSize, source + planeSize + planeSizeCx, planeSizeCx );

		return EncodeI420 ( yuvBuffer, output, context );
	}


	int EncoderBase::EncodeYUY2 ( char * source, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeYUY2" );

        int planeSize = width * height;
        int planeSizeCx = planeSize >> 2;

		unsigned int size = planeSize + (planeSize >> 1);

		if ( yuvBufferSize != size ) {
            DisposeCaches ();
		}

		if ( !yuvBuffer ) {
			yuvBuffer = new char[ size ];
			if ( !yuvBuffer ) {
				return 0;
			}
			memset ( yuvBuffer, 0, size );

            uCache = new char[ planeSizeCx ];
            if ( !uCache ) {
                return 0;
            }
			memset ( uCache, 0, planeSizeCx );

            vCache = new char[ planeSizeCx ];
            if ( !vCache ) {
                return 0;
            }
			memset ( vCache, 0, planeSizeCx );
			yuvBufferSize = size;
		}

        char * curPack = source;
        char * curY = yuvBuffer;
		char * curULine = uCache;
		char * curU;
		char * curVLine = vCache;
		char * curV;
		unsigned int curStride = width >> 1;

		for ( unsigned int i=0; i < height; i++ ) {
			curU = curULine;
			curV = curVLine;
			if ( i % 2 == 0 ) {
				for ( unsigned int j=0; j < curStride; j++ ) {
					*curY++ = *curPack++;
					*curU++ = *curPack++;
					*curY++ = *curPack++;
					*curV++ = *curPack++;
				}
			}
			else {
				for ( unsigned int j=0; j < curStride; j++ ) {
					*curY++ = *curPack;
					//int val = ((int) *curU + (int) *(curPack + 1)) / 2;
					//*curU++ = (char) val;
					*curY++ = *(curPack + 2);
					//val = ((int) *curV + (int) *(curPack + 3)) / 2;
					//*curV++ = (char) val;
					curPack += 4;
				}
				curULine += curStride;
				curVLine += curStride;
			}
		}

        memcpy ( yuvBuffer + planeSize, uCache, planeSizeCx );
        memcpy ( yuvBuffer + planeSize + planeSizeCx, vCache, planeSizeCx );

		return EncodeI420 ( yuvBuffer, output, context );
	}


	int EncoderBase::EncodeARGB ( char * source, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeARGB" );

		unsigned int size = width * height * 3;
		if ( yuvBufferSize != size ) {
            DisposeCaches ();
		}

		// Convert the image to yuv, maybe change to an external library later some time
		if ( !yuvBuffer ) {
			yuvBuffer = new char[ size ];
			if ( !yuvBuffer ) {
				return 0;
			}
			yuvBufferSize = size;
		}

		if ( !context->hasChanged )
			return EncodeI420 ( yuvBuffer, output, context );

		const size_t image_size = width * height;
		uint8_t *dst_y = (uint8_t *) yuvBuffer;
		uint8_t *dst_u = dst_y + image_size;
		uint8_t *dst_v = dst_y + image_size + image_size / 4;

		uint8_t * rgb = (uint8_t *) source;
		// Y plane
		for ( size_t i = 0; i < image_size; ++i ) {
			*dst_y++ = ((66 * rgb [4 * i + 2] + 129 * rgb [4 * i + 1] + 25 * rgb [4 * i]) >> 8) + 16;
		}
		// U+V planes
		for ( size_t y=0; y<height; y+=2 ) {
			for ( size_t x=0; x<width; x+=2 ) {
				const size_t i = y*width + x;
				*dst_u++ = ((-38 * rgb [4 * i + 2] + -74 * rgb [4 * i + 1] + 112 * rgb [4 * i]) >> 8) + 128;
				*dst_v++ = ((112 * rgb [4 * i + 2] + -94 * rgb [4 * i + 1] + -18 * rgb [4 * i]) >> 8) + 128;
			}
		}

		return EncodeI420 ( yuvBuffer, output, context );
	}


	int EncoderBase::EncodeBGRA ( char * source, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeBGRA" );

		unsigned int size = width * height * 3;
        if ( yuvBufferSize != size ) {
            DisposeCaches ();
		}

		// Convert the image to yuv, maybe change to an external library later some time
        if ( !yuvBuffer ) {
            yuvBuffer = new char[ size ];
            if ( !yuvBuffer ) {
                return 0;
            }
            yuvBufferSize = size;
        }

		if ( !context->hasChanged )
			return EncodeI420 ( yuvBuffer, output, context );

		const size_t image_size = width * height;
		uint8_t *dst_y = (uint8_t *) yuvBuffer;
		uint8_t *dst_u = dst_y + image_size;
		uint8_t *dst_v = dst_y + image_size + image_size / 4;

		uint8_t * rgb = (uint8_t *) source;
		// Y plane
		for ( size_t i = 0; i < image_size; ++i ) {
			*dst_y++ = ((66 * rgb [4 * i] + 129 * rgb [4 * i + 1] + 25 * rgb [4 * i + 2]) >> 8) + 16;
		}
		// U+V planes
		for ( size_t y=0; y<height; y+=2 ) {
			for ( size_t x=0; x<width; x+=2 ) {
				const size_t i = y*width + x;
				*dst_u++ = ((-38 * rgb [4 * i] + -74 * rgb [4 * i + 1] + 112 * rgb [4 * i + 2]) >> 8) + 128;
				*dst_v++ = ((112 * rgb [4 * i] + -94 * rgb [4 * i + 1] + -18 * rgb [4 * i + 2]) >> 8) + 128;
			}
		}

		return EncodeI420 ( yuvBuffer, output, context );
	}
}
