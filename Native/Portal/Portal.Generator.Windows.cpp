/**
* Portal Generator for the Windows platform
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
#   define DEBUGVERBVerb
#endif

#ifdef _WIN32

#include "Environs.Obj.h"
#include "Environs.Modules.h"
#include "Portal.Generator.Windows.h"
#include "Device/Device.Controller.h"

#include "Capture/Capture.Win.GDI.h"
#include "Capture/Capture.Win.TGDI.h"
#include "Capture/Capture.Win.D3D.h"

#include "Renderer/Render.GDI.h"
#include "Renderer/Render.OpenCL.h"

#include "Encoder/Encoder.GDI.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator.Windows"


namespace environs 
{

	PortalGeneratorWindows::~PortalGeneratorWindows ()
	{
		CVerbID ( "Destruct" );
	}


#ifndef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
	void PortalGeneratorWindows::InitRecognizers ()
	{
		CVerbID ( "InitRecognizers" );

		if ( false && !recognizers )
		{
			recognizedGesture = false;

			recognizers = (IInputRecognizer **)malloc ( sizeof ( IInputRecognizer * ) * ENVIRONS_TOUCH_RECOGNIZER_MAX );
			if ( recognizers ) {
				memset ( recognizers, 0, sizeof ( IInputRecognizer * ) * 4 );

				recognizersCount = 0;

				for ( unsigned int i = 0; i < Kernel::touchRecognizerNamesCount; i++ )
				{
					if ( Kernel::touchRecognizerNames [i] )
					{
						IInputRecognizer * reco = (IInputRecognizer *) environs::API::CreateInstance ( Kernel::touchRecognizerNames [i], 0, InterfaceType::InputRecognizer, deviceID, env );

						if ( reco ) {
							if ( !reco->Init ( parentDevice, &parentDevice->display ) ) {
								CErrID ( "InitRecognizers: Failed to initiailze recognizer." );
								delete reco;
							}
							else {
								recognizers [recognizersCount] = reco;
								recognizersCount++;
							}
						}
					}
				}
			}
		}
	}
#endif


	bool PortalGeneratorWindows::CreateWorkerStages ( WorkerStages * stages, int index )
	{
        CVerbArgID ( "CreateWorkerStages: Requested index [%u]!", index );
        
        if ( index == 0 )
			return CreatePictureStages ( stages );
        
        if ( index == 1 )
			return CreateStreamPreferredStages ( stages );
        
		if ( index == 2 )
            return CreateStreamFallbackStages ( stages );
        
		return false;
	}


	bool PortalGeneratorWindows::CreatePictureStages ( WorkerStages * stages )
	{
		CVerbID ( "CreatePictureStages" );

		stages->capture = CreateCapture ( );
		if ( !stages->capture ) {
			return false;
		}
		stages->capture->env = env;

		stages->render = (IPortalRenderer *) new RenderGDI ();
		if ( !stages->render ) {
			CErrID ( "CreatePictureStages: Failed to create render stage." );
			return false;
		}

		stages->encoder = (IPortalEncoder *) new EncoderGDI ( );
		if ( !stages->encoder ) {
			CVerbID ( "CreatePictureStages: Failed to create prefered encoder stage." );

			stages->encoder = (IPortalEncoder *) environs::API::CreateInstance ( LIBNAME_Encoder_LibOpenH264, 0, InterfaceType::Encoder, deviceID, env );
			if ( !stages->encoder ) {
				CErrID ( "CreatePictureStages: Failed to create encoder stage." );
				return false;
            }
            CLogArg ( "CreatePictureStages: Encoder stage using [%s]", stages->encoder->name );
		}

		streamOptions->useStream = false;
		return true;
	}
    
    
	bool PortalGeneratorWindows::CreateStreamPreferredStages ( WorkerStages * stages )
	{
		CVerbID ( "CreateStreamPreferredStages" );

		if ( !stages || !streamOptions || !streamOptions->useStream )
			return false;
		
		stages->capture = CreateCapture ( );
		if ( !stages->capture ) {
			return false;
		}
		stages->capture->env = env;

		stages->render = CreateCustomRenderer ();
		if ( !stages->render ) {
			if ( streamOptions->useOpenCL ) {
				stages->render = (IPortalRenderer *) new RenderOpenCL ();
			}
			else {
				stages->render = (IPortalRenderer *) new RenderGDI ();
			}
		}

		if ( !stages->render ) {
			CErrID ( "CreateStreamPreferredStages: Failed to create render stage." );
			return false;
		}
		stages->render->env = env;

		if ( env->mod_PortalEncoder ) {
			stages->encoder = (IPortalEncoder *) environs::API::CreateInstance ( env->mod_PortalEncoder, 0, InterfaceType::Encoder, deviceID, env );
			if ( !stages->encoder ) {
				CErr ( "CreateStreamPreferredStages: Failed to create user specified encoder. Fallback to default encoder." );
			}
			else {
				CLogArg ( "CreateStreamPreferredStages: Encoder stage using [%s]", stages->encoder->name );
			}
		}
		else {
			stages->encoder = (IPortalEncoder *) environs::API::CreateInstance ( LIBNAME_Encoder_LibOpenH264, 0, InterfaceType::Encoder, deviceID, env );
			if ( !stages->encoder ) {
				CErrID ( "CreateStreamPreferredStages: Failed to create stream encoder stage." );
			}
			else {
				CLogArg ( "CreateStreamPreferredStages: Encoder stage using [%s]", stages->encoder->name );
			}
		}

		if ( !stages->encoder ) {
			stages->encoder = (IPortalEncoder *) new EncoderGDI ( );
		}

		if ( !stages->encoder ) {
			CErrID ( "CreateStreamPreferredStages: Failed to create render stage." );
			return false;
		}

		return true;
	}
    
    
	bool PortalGeneratorWindows::CreateStreamFallbackStages ( WorkerStages * stages )
	{
		CVerbID ( "CreateStreamFallbackStages" );
        
		return CreatePictureStages ( stages );
	}

	
	IPortalRenderer	* PortalGeneratorWindows::CreateCustomRenderer ()
	{
		IPortalRenderer * rend = 0;

		if ( env->mod_PortalRenderer ) {
			rend = (IPortalRenderer *) environs::API::CreateInstance ( env->mod_PortalRenderer, 0, InterfaceType::Render, deviceID, env );
			if ( !rend ) {
				CErr ( "CreateCustomRenderer: Failed to create user specified renderer. Fallback to default." );
                return 0;
            }
            CLogArg ( "CreateCustomRenderer: Renderer stage using [%s]", rend->name );
		}

		return rend;
	}


	IPortalCapture * PortalGeneratorWindows::CreateCapture ()
	{
		CVerbID ( "CreateCapture" );

		/// Create capture stage
		IPortalCapture * cap = 0;

		do {
			if ( env->mod_PortalCapturer ) {
				cap = (IPortalCapture *) environs::API::CreateInstance ( env->mod_PortalCapturer, 0, InterfaceType::Capture, deviceID, env );
				if ( !cap ) {
					CErrID ( "CreateCapture: Failed to create user specified capture." );
                    return 0;
                }
                CLogArgID ( "CreateCapture: Capture stage using [%s]", cap->name );
                break;
			}

#ifdef _WIN32
#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
			WARNING ( "CreateCapture: WinD3D is alpha." )

			if ( opt_useWinD3D && CaptureWinD3D::d3dDevice ) {
				cap = new CaptureWinD3D ( );	
			}		
			else
#endif
				if ( opt_useWinTGDI )
					cap = new CaptureWinTGDI ();
				else
					cap = new CaptureWinGDI ();

			if ( !cap ) {
				CErrID ( "CreateCapture: No capture plugins enabled/available." );
			}
		} 
		while ( false );
#endif
		return cap;
	}
    
    

} /// -> namespace environs

#endif

