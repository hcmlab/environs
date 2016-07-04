/**
 * Base camera capture module with double buffering
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
#define DEBUGVERB
#define DEBUGVERBVerb
#endif

#include "Environs.Native.h"
#include "Interop.h"
#include "Interop/Threads.h"
#include "Cam.Base.2B.h"
#include "Portal.Worker.Stages.h"

#include "Environs.Obj.h"
#include "Interfaces/Interface.Exports.h"
using namespace environs;

#define CLASS_NAME	"CamBase2B"


namespace environs
{

	CamBase2B::CamBase2B ()
	{
		CLog ( "Construct" );

		name 				= "Camera Capture Base";
		captureType			= CaptureType::Camera;

		nextBuffer			= 0;
		data0               = 0;
		data1               = 0;
		tempFilled          = false;
	}


	CamBase2B::~CamBase2B () {
		CLogID ( "Destruct" );

		ReleaseSwapBuffers ( );
	}


	int CamBase2B::AllocateResources ( RenderDimensions * dims ) {
		CVerbID ( "AllocateResources" );
		return 1;
	}


	int CamBase2B::ReleaseResources () {
        CVerbID ( "ReleaseResources" );
        return 1;
	}


	void CamBase2B::ReleaseSwapBuffers ()
	{
		CVerbID ( "ReleaseSwapBuffers" );

		delete[] data0;
		data0 = 0;

		delete[] data1;
		data1 = 0;

		dataSize = 0;
	}


	bool CamBase2B::AllocateSwapBuffers ()
	{
		CVerbID ( "AllocateSwapBuffers" );

		if ( dataSize <= 0 )
			return false;

		char * tmp0 = new char[ dataSize ];
		char * tmp1 = new char[ dataSize ];

		if ( tmp0 && tmp1 ) {

			delete[] data0;
			delete[] data1;

			data0 = tmp0; data1 = tmp1;
			data = data0;

			dataAccessed = 0;
			return true;
		}

		delete[] tmp0;
		delete[] tmp1;
		return false;
	}


	int CamBase2B::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		if ( !nextBuffer )
			return 0;

		if ( !((environs::WorkerStages *)stages)->render ) {
			context->renderedData = nextBuffer;
		}
		nextBuffer = 0;
		return 1;
	}


	/*
	* PerformBase is expected to be executed by the camera thread.
	* After taken over the buffer, it triggers the portal worker thread.
	*/
	bool CamBase2B::PerformBase ( const char * payload, unsigned int payloadSize )
	{
		if ( !data0 || !dataSize ) {
			CLogArgID ( "PerformBase: Allocating buffers of size [%u].", payloadSize );

			ReleaseSwapBuffers ();

			dataSize = payloadSize;

			if ( !AllocateSwapBuffers () ) {
				CErrArgID ( "PerformBase: Failed to allocate [%u] bytes for capture buffers.", payloadSize );
				return false;
			}
		}
		if ( dataSize != payloadSize )
			return false;

		char * dest;
		if ( data == data0 )
			dest = data1;
		else
			dest = data0;

		if ( !tempFilled ) {
			CVerbVerbID ( "PerformBase: Filling temporary buffer." );
			memcpy ( BYTEBUFFER_DATA_POINTER_START ( dest ), payload, payloadSize );
			tempFilled = true;
		}

		//ReCheck:
		if ( ___sync_val_compare_and_swap ( &dataAccessed, 1, 1 ) == 1 ) {
			//if ( capture->dataAccessed ) {
			CVerbVerbID ( "PerformBase: Swapping temporary buffer." );
			data = dest;
			dataAccessed = 0;
			tempFilled = false;
		}


		if ( portalWorkerEvent && portalWorkerEventLock ) {
			CVerbVerbID ( "PerformBase: Triggering..." );

			if ( pthread_cond_mutex_lock ( (pthread_mutex_t *) portalWorkerEventLock ) ) {
				CErr ( "PerformBase: Failed to lock portalWorkerEventLock." );
				return false;
			}

			if ( pthread_cond_signal ( (pthread_cond_t *) portalWorkerEvent ) ) {
				CErr ( "PerformBase: Failed to signal portalWorkerEvent." );
			}

			if ( pthread_cond_mutex_unlock ( (pthread_mutex_t *) portalWorkerEventLock ) ) {
				CErr ( "PerformBase: Failed to unlock portalWorkerEventLock." );
			}
		}
		return true;
	}

} /* namespace environs */
