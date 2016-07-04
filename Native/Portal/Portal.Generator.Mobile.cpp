/**
 *	Platform Portal Generator
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

#ifndef DISPLAYDEVICE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Portal.Generator.Mobile.h"
#include "Device/Device.Controller.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator.Base. ."


namespace environs 
{
	bool PortalGeneratorBase::CreateWorkerStages ( WorkerStages * stages, int index )
	{
        CVerbArgID ( "CreateWorkerStages: Requested index [%u]!", index );
        
        /// 0 = Preferred stages
        if ( index == 0 )
            return CreateCameraStages ( stages, 1 );
        
        /// 1 = Secondary stages
        if ( index == 1 )
            return CreateCameraStages ( stages, index );

        /// 2 = ??
        if ( index == 2 )
            return CreateCameraStages ( stages, index );
        
        /// Fallback stages - no fallback yet
		return CreateWorkerStagesPicture ( stages );
	}


	bool PortalGeneratorBase::CreateWorkerStagesPicture ( WorkerStages * stages )
	{
        CVerbID ( "CreateWorkerStagesPicture" );

		return false;
	}
    
    
	bool PortalGeneratorBase::CreateCameraStages ( WorkerStages * stages, int cameraID )
	{
		CVerbID ( "CreateBackCameraStages" );

		/*if ( !streamOptions->useStream )
			return false;*/
		
		stages->capture = CreateCameraCapture ( cameraID );
		if ( !stages->capture )
			return false;
        
        stages->encoder = GetEncoderInstance ( deviceID, cameraID );
        if ( stages->encoder ) {
            stages->encoder->deviceID = deviceID;
            //stages->encoder->encodedType = streamOptions->useStream;
        }
        
		if ( !stages->encoder ) {
			CErrID ( "CreateBackCameraStages: Failed to create encoder stage." );
			return false;
		}

		return true;
	}


	IPortalCapture * PortalGeneratorBase::CreateCameraCapture ( int cameraID )
	{
		CVerbID ( "CreateCameraCapture" );

		/// Create camera capture
        IPortalCapture * cap =  GetCameraInstance ( cameraID );        
		if ( !cap ) {
			CErrArgID ( "CreateCameraCapture: Failed to create android camera portalID [0x%X].", portalID );
			goto Failed;
		}

		/*cap->osLevel = environs.OSLevel;
		cap->portalWorkerEvent = (void *) &PortalGenerator::portalWorkerEvent;
		cap->portalWorkerEventLock = (void *) &PortalGenerator::portalWorkerEventLock;

		if ( !cap->Init ( deviceID, (void *)(long)portalID, &parentDevice->streamOptions ) ) {
			CErrArgID ( "CreateCameraCapture: Failed to create android camera portalID [0x%X].", portalID );
			goto Failed;
		}

		renderDimensions.width_cap	= cap->width;
        renderDimensions.height_cap	= cap->height;
        renderDimensions.streamWidth	= cap->width;
        renderDimensions.streamHeight	= cap->height;
         renderDimensions.orientation = 90;
         */
        centerX = cap->width >> 1;
        centerY = cap->height >> 1;

		CVerbArgID ( "CreateCameraCapture: Successfully created/initialized camera for portalID [0x%X].", portalID );
		return cap;

	Failed:
		if ( cap )
			delete cap;
		return 0;
	}
    
    

} /// -> namespace environs

#endif
