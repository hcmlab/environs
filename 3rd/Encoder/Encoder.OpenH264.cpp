/**
 * Implementation of video/avc, h264 stream encoding using libx264
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
#   define DEBUGVERB
#   define ENCODERDEBUGVERB
#endif

#define	INIT_EXT_PARAMS

#include <stdio.h>
#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Encoder/Encoder.OpenH264.h"
#include "Environs.Types.h"
#include "DynLib/Dyn.Lib.OpenH264.h"
#include "Interfaces/Interface.Exports.h"
#include "Interfaces/IPortal.Renderer.h"
#include "Interop/Export.h"
#include "Environs.Build.Lnk.h"

using namespace environs;


// The TAG for prepending in log messages
#define CLASS_NAME	"Encoder.OpenH264 . . . ."


static const char		*		EncoderOpenH264_extensionNames[]	= { "Openh264 Encoder", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	EncoderOpenH264_interfaceTypes[]	= { InterfaceType::Encoder, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( EncoderOpenH264_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( EncoderOpenH264_interfaceTypes );


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
BUILD_INT_CREATEOBJ ( EncoderOpenH264 );
#endif
#endif

#if !defined(ENVIRONS_MISSING_OPENH264_HEADERS)

#define	ENCODER_INTRA_FRAME_INTERVALL	5

namespace environs 
{
	extern PortalBufferType_t	EncoderBase_inputTypeSupport[];

	EncoderOpenH264::EncoderOpenH264 ()
	{
		name				= EncoderOpenH264_extensionNames [0];

		CVerbArgID ( "Construct: [%s]", name );


		inBufferType		= EncoderBufferType::YUV420;
		cacheCount			= 0;
        encodedType         = DATA_STREAM_H264_NALUS;
		encoder				= 0;
		idr					= 0;
		enableExtendedOption = true;

		inputTypes			= EncoderBase_inputTypeSupport;
	}


	bool EncoderOpenH264::ApplyInput ()
	{
		CLogID ( "ApplyInput" );

		if ( inputType == environs::PortalBufferType::YUV420 ) {
			processor = (ProcessorHandler) &EncoderOpenH264::EncodeI420;
			return true;
		}
		return EncoderBase::ApplyInput ();
	}


	EncoderOpenH264::~EncoderOpenH264 ()
	{
		CLogID ( "Destruct..." );

		Dispose ( );

		CVerbID ( "Destruct destroyed." );
	}


	void EncoderOpenH264::Dispose ()
	{
		if ( encoder ) {
			encoder->Uninitialize ();
			dWelsDestroySVCEncoder ( encoder );
			encoder = 0;
		}
	}


	//#define OLD_PARAMS
	//#define ULTRAFAST

	bool EncoderOpenH264::Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate )
	{
		this->deviceID = deviceID;

		CVerbID ( "Init" );

		if ( !Width || !Height )
			return false;

		width = Width; height = Height;
        
        if ( !InitLibOpenH264 ( env, deviceID ) ) {
			CErrID ( "Init: Failed to initialize access to libopenh264!" );
            return false;
        }
        
		int success = cmResultSuccess;
		
		if ( !encoder ) {
			success = dWelsCreateSVCEncoder ( &encoder );
			if ( success != 0 || !encoder )
				return false;
		}

		if ( enableExtendedOption ) {
			SEncParamExt epars;
			Zero ( epars );
			success = encoder->GetDefaultParams ( &epars );

			if ( success != cmResultSuccess )
				return false;

			//pars.iUsageType = SCREEN_CONTENT_REAL_TIME;
			epars.iUsageType = CAMERA_VIDEO_REAL_TIME; // 
			epars.fMaxFrameRate = (float) ((!FrameRate || FrameRate > 30) ? 30 : FrameRate);
			epars.iPicWidth = Width;
			epars.iPicHeight = Height;
			
			epars.iTargetBitrate = BitRate ? BitRate : 5000000;
			//epars.iTargetBitrate >>= 2;

			epars.eSpsPpsIdStrategy				= CONSTANT_ID;

			epars.iMaxBitrate					= epars.iTargetBitrate;
			epars.iRCMode						= RC_QUALITY_MODE;
			epars.iTemporalLayerNum				= 1;
			//epars.iTemporalLayerNum				= 2;
			epars.iSpatialLayerNum				= 1;
			epars.bIsLosslessLink				= 0;
			epars.bEnableDenoise				= 1;
			epars.bEnableBackgroundDetection	= 1;
			epars.bEnableAdaptiveQuant			= 1;
			epars.bEnableFrameSkip				= 1;
			epars.bEnableLongTermReference		= 0;
			epars.iLtrMarkPeriod				= 30;
			epars.uiIntraPeriod					= 1;
			//epars.uiIntraPeriod					= 10; // reduced required bitrate ftom 5 to 1.4
			epars.eSpsPpsIdStrategy				= CONSTANT_ID;
			epars.bPrefixNalAddingCtrl			= 1;
			epars.iLoopFilterDisableIdc			= 0;
			epars.iEntropyCodingModeFlag		= 0;
			//epars.iEntropyCodingModeFlag		= 1;
			epars.iMultipleThreadIdc			= 1;
			epars.iEntropyCodingModeFlag		= 1;

			epars.sSpatialLayers [0].iVideoWidth         = epars.iPicWidth;
			epars.sSpatialLayers [0].iVideoHeight        = epars.iPicHeight;
			epars.sSpatialLayers [0].fFrameRate          = epars.fMaxFrameRate;
			epars.sSpatialLayers [0].iSpatialBitrate     = epars.iTargetBitrate;
			epars.sSpatialLayers [0].iMaxSpatialBitrate  = epars.iMaxBitrate;

			epars.sSpatialLayers [0].sSliceCfg.uiSliceMode               = SM_SINGLE_SLICE ;
			epars.sSpatialLayers [0].sSliceCfg.sSliceArgument.uiSliceNum = 1;

			success = encoder->InitializeExt ( &epars );	

		}	
		else {
			SEncParamBase pars;
			Zero ( pars );

			//pars.iUsageType = SCREEN_CONTENT_REAL_TIME;
			pars.iUsageType = CAMERA_VIDEO_REAL_TIME; // 
			pars.fMaxFrameRate = (float) ((!FrameRate || FrameRate > 30) ? 30 : FrameRate);
			pars.iPicWidth = Width;
			pars.iPicHeight = Height;

			pars.iTargetBitrate = BitRate ? BitRate : 5000000;
			//pars.iTargetBitrate >>= 2;

			success = encoder->Initialize ( &pars );

		}

		if ( success != cmResultSuccess  )
			return false;

		//int g_LevelSetting = 1;
		//encoder->SetOption ( ENCODER_OPTION_TRACE_LEVEL, &g_LevelSetting );
		//int option = videoFormatI420;
		//encoder->SetOption ( ENCODER_OPTION_DATAFORMAT, &option );

		/*option = 1;			
		if ( encoder->SetOption ( ENCODER_OPTION_IDR_INTERVAL, &option ) != 0 ) {
			CErrID ( "Init: Failed to set idr interval option." );
		}
		*/
		/*
		option = 1;
		if ( encoder->SetOption ( ENCODER_OPTION_RC_MODE, &option ) != cmResultSuccess ) {
			CErrID ( "Init: Failed to set rc mode." );
		}

		option = 0;
		if ( encoder->SetOption ( ENCODER_OPTION_IS_LOSSLESS_LINK, &option ) != cmResultSuccess ) {
			CErrID ( "Init: Failed to set lossless option." );
		}

		option = 0;
		if ( encoder->SetOption ( ENCODER_OPTION_ENABLE_SPS_PPS_ID_ADDITION, &option ) != cmResultSuccess ) {
			CErrID ( "Init: Failed to set sps/pps option." );
		}
		
		option = 1;
		if ( encoder->SetOption ( ENCODER_OPTION_ENABLE_PREFIX_NAL_ADDING, &option ) != cmResultSuccess ) {
			CErrID ( "Init: Failed to set sps/pps option." );
		}
		*/

		CVerbID ( "Init successful." );
		return true;
	}

    
	int EncoderOpenH264::EncodeI420 ( char * yuvdata, char * &output, RenderContext * context )
	{
		CVerbVerbID ( "EncodeI420" );

		int success = 0;

		SFrameBSInfo frame;
		Zero ( frame );

		SSourcePicture picture;
		Zero ( picture );

		picture.iPicWidth = width;
		picture.iPicHeight = height;
		picture.iColorFormat = videoFormatI420;
		picture.iStride [0] = picture.iPicWidth;
		picture.iStride [1] = picture.iStride [2] = picture.iPicWidth >> 1;
		picture.pData [0] = (unsigned char *) yuvdata;
		picture.pData [1] = picture.pData [0] + width * height;
		picture.pData [2] = picture.pData [1] + (width * height >> 2);

		//context->isIFrame = true;
		//encoder->ForceIntraFrame ( true );

        context->isIFrame = false;
        
        if ( iFrameFPSMode || iFrameRequest ) {
			encoder->ForceIntraFrame ( true );
        }
        else {
            /*if ( idr >= ENCODER_INTRA_FRAME_INTERVALL ) {
                encoder->ForceIntraFrame ( true );
                context->isIFrame = true;
                idr = 0;
            }
            else
                idr++;*/
        }
		

		success = encoder->EncodeFrame ( &picture, &frame );
		if ( success != cmResultSuccess )
			return 0;

		if ( frame.eFrameType == videoFrameTypeSkip ) {
			CVerbVerb ( "EncodeYUV: skipping frame." );
			return 1;
		}

		int payloadSize = 0;

		if ( frame.iLayerNum && frame.iFrameSizeInBytes > 0 )
		{
			if ( frame.eFrameType == videoFrameTypeIDR ) {
				context->isIFrame = true;

				if ( iFrameRequest )
					iFrameRequest = false;
			}

			output = (char *)frame.sLayerInfo->pBsBuf;
			return frame.iFrameSizeInBytes;
			
		}
		if ( context->outputBuffer )
			context->outputBuffer->payloadSize = payloadSize;
		return 0;
		
	}



} /* namespace environs */

#endif


