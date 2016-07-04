/**
* Portal Generator for the OSX platform
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

#ifdef ENVIRONS_OSX

#include "Environs.Obj.h"
#include "Environs.Modules.h"
#include "Portal.Generator.OSX.h"
#include "Device/Device.Controller.h"

#include "Capture/Capture.OSX.h"

#include "Renderer/Render.OpenCL.h"
#include "Renderer/Render.OSX.h"

#include "Encoder/Encoder.iOSX.H264.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator.OSX . ."


namespace environs 
{

	PortalGeneratorOSX::~PortalGeneratorOSX ()
	{
		CVerbID ( "Destruct" );
	}


	bool PortalGeneratorOSX::CreateWorkerStages ( WorkerStages * stages, int index )
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


	bool PortalGeneratorOSX::CreatePictureStages ( WorkerStages * stages )
	{
		CVerbID ( "CreatePictureStages" );

		stages->capture = CreateCapture ( );
		if ( !stages->capture )
			return false;

		stages->render = (IPortalRenderer *) new RenderOSX ();
		if ( !stages->render || (stages->render->env = env ) == 0 ) {
			CErrID ( "CreatePictureStages: Failed to create render stage." );
			return false;
		}

		stages->encoder = (IPortalEncoder *) new EncoderIOSH264 ( );
		if ( !stages->encoder || (stages->encoder->env = env ) == 0 ) {
			CVerbID ( "CreatePictureStages: Failed to create prefered encoder stage." );
		}
        else {
            CLogArg ( "CreatePictureStages: Encoder stage using [%s]", stages->encoder->name );
        }

		streamOptions->useStream = false;
		return true;
	}
    
    
	bool PortalGeneratorOSX::CreateStreamPreferredStages ( WorkerStages * stages )
	{
		CVerbID ( "CreateStreamPreferredStages" );

		if ( !stages || !streamOptions || !streamOptions->useStream )
			return false;
		
		stages->capture = CreateCapture ( );
		if ( !stages->capture || (stages->capture->env = env ) == 0 )
            return false;
        
        stages->render = CreateCustomRenderer ();
        if ( !stages->render ) {
            if ( streamOptions->useOpenCL ) {
                stages->render = (IPortalRenderer *) new RenderOpenCL ();
            }
            else {
                stages->render = (IPortalRenderer *) new RenderOSX ();
            }
        }
        
        if ( !stages->render || (stages->render->env = env ) == 0 ) {
            CErrID ( "CreateStreamPreferredStages: Failed to create render stage." );
            return false;
        }
		
		if ( env->mod_PortalEncoder ) {
			stages->encoder = (IPortalEncoder *) environs::API::CreateInstance ( env->mod_PortalEncoder, 0, InterfaceType::Encoder, deviceID, env );
			if ( !stages->encoder ) {
				CErr ( "CreateStreamPreferredStages: Failed to create user specified encoder. Fallback to default encoder." );
			}
			else {
				CLogArg ( "CreateStreamPreferredStages: Encoder stage using [%s]", stages->encoder->name );
                return true;
			}
        }
        
        stages->encoder = (IPortalEncoder *) new EncoderIOSH264Env ();
        if ( !stages->encoder || (stages->encoder->env = env ) == 0 ) {
            CErrID ( "CreateStreamPreferredStages: Failed to create hw stream encoder stage." );
        }
        else {
            CLogArg ( "CreateStreamPreferredStages: Encoder stage using [%s]", stages->encoder->name );
            return true;
        }

		return false;
    }
    
    
    IPortalRenderer	* PortalGeneratorOSX::CreateCustomRenderer ()
    {
        IPortalRenderer * rend = 0;
        
        if ( env->mod_PortalRenderer ) {
            rend = (IPortalRenderer *) environs::API::CreateInstance ( env->mod_PortalRenderer, 0, InterfaceType::Render, deviceID, env );
            if ( !rend || (rend->env = env ) == 0 ) {
                CErr ( "CreateCustomRenderer: Failed to create user specified renderer. Fallback to default." );
            }
            else {
                CLogArg ( "CreateCustomRenderer: Renderer stage using [%s]", rend->name );
            }
        }
        
        return rend;
    }
    
    
	bool PortalGeneratorOSX::CreateStreamFallbackStages ( WorkerStages * stages )
	{
		CVerbID ( "CreateStreamFallbackStages" );
        
		return CreatePictureStages ( stages );
	}


	IPortalCapture * PortalGeneratorOSX::CreateCapture ()
	{
		CVerbID ( "CreateCapture" );
        
        /// Create capture stage
        IPortalCapture * cap = 0;
        
        do {
            if ( env->mod_PortalCapturer ) {
                cap = (IPortalCapture *) environs::API::CreateInstance ( env->mod_PortalCapturer, 0, InterfaceType::Capture, deviceID, env );
                if ( !cap || (cap->env = env ) == 0 ) {
                    CErrID ( "CreateCapture: Failed to create user specified capture." );
                    return 0;
                }
                CLogArgID ( "CreateCapture: Capture stage using [%s]", cap->name );
                break;
            }
            
            cap = new CaptureOSX ();
            if ( !cap || (cap->env = env ) == 0 ) {
                CErrID ( "CreateCapture: No capture plugins enabled/available." );
                return 0;
            }
        } 
        while ( false );
        
		return cap;

	Failed:
		if ( cap )
			delete cap;
		return 0;
	}
    
    

} /// -> namespace environs

#endif

