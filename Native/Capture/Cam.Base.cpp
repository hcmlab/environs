/**
 * Base camera capture module
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
//#   define DEBUGVERBVerb
#endif

#include "Environs.Native.h"
#include "Interop.h"
#include "Interop/Threads.h"
#include "Cam.Base.h"
#include "Portal.Worker.Stages.h"

#include "Environs.Obj.h"
#include "Interfaces/Interface.Exports.h"
using namespace environs;

#define CLASS_NAME	"Cam.Base . . . . . . . ."


namespace environs 
{

	CamBase::CamBase ( )
    {
        name 				= "Camera Capture Base";
        
		CLogArg ( "Construct [ %s ]", name );

		captureType			= CaptureType::Camera;

		data				= 0;
        dataBaseAllocated   = false;
	}


	CamBase::~CamBase ( ) {
		CLogID ( "Destruct" );

		ReleaseSwapBuffers ( );
	}


	int CamBase::AllocateResources ( RenderDimensions * dims ) {
		CVerbID ( "AllocateResources" );		
		return 1;
	}


	int CamBase::ReleaseResources ( ) {
        CVerbID ( "ReleaseResources" );
        return 1;
	}


	void CamBase::ReleaseSwapBuffers ( )
	{
		CVerbID ( "ReleaseSwapBuffers" );

		if ( dataBaseAllocated && data ) {
			free ( data );
			data = 0;
		}
		dataSize = 0;
	}


	bool CamBase::AllocateSwapBuffers ()
	{
		CVerbID ( "AllocateSwapBuffers" );

		if ( dataSize <= 0 )
			return false;

		char * tmp0 = (char *) malloc ( dataSize );

		if ( tmp0 ) {
			data = tmp0;
			dataAccessed = 2;
            dataBaseAllocated = true;
			return true;
		}
		return false;
	}


	int CamBase::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		if ( ___sync_val_compare_and_swap ( &dataAccessed, 0, 1 ) != 0 ) {
			return 0;
		}

		// dataAccessed = 1
		if ( !((environs::WorkerStages *)stages)->render ) {
			context->renderedData = data;
			context->width = width;
			context->height = height;
			context->stride = dataStride;
		}
		return 1;
	}


	/*
	* PerformBase is expected to be executed by the camera thread.
	* After taken over the buffer, it triggers the portal worker thread.
	*/
	bool CamBase::PerformBase ( const char * payload, unsigned int payloadSize )
	{
		if ( !data || !dataSize ) {
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

		if ( ___sync_val_compare_and_swap ( &dataAccessed, 2, 2 ) == 2 ) {
			CVerbVerbID ( "PerformBase: Copying capture data." );
			memcpy ( data, payload, payloadSize );

			// dataAccessed = 0
			__sync_sub_and_fetch ( &dataAccessed, 1 );
			__sync_sub_and_fetch ( &dataAccessed, 1 );
		}
		else {
			CVerbID ( "PerformBase: Buffer full." );
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
    
    
    bool CamBase::PerformIOSX ( void * cvSampleBuffer )
    {
        bool success = false;
        
        if ( ___sync_val_compare_and_swap ( &dataAccessed, 2, 2 ) == 2 ) {
            CVerbVerbID ( "PerformIOSX: Taking over capture data." );

            data = (char *) cvSampleBuffer;
            
            // dataAccessed = 0
            __sync_sub_and_fetch ( &dataAccessed, 1 );
            __sync_sub_and_fetch ( &dataAccessed, 1 );
            
            success = true;
        }
        else {
            CVerbID ( "PerformIOSX: Buffer full." );
        }
        
        
        if ( portalWorkerEvent && portalWorkerEventLock ) {
            CVerbVerbID ( "PerformIOSX: Triggering..." );
            
            if ( pthread_cond_mutex_lock ( (pthread_mutex_t *) portalWorkerEventLock ) ) {
                CErr ( "PerformIOSX: Failed to lock portalWorkerEventLock." );
                return false;
            }
            
            if ( pthread_cond_signal ( (pthread_cond_t *) portalWorkerEvent ) ) {
                CErr ( "PerformIOSX: Failed to signal portalWorkerEvent." );
            }
            
            if ( pthread_cond_mutex_unlock ( (pthread_mutex_t *) portalWorkerEventLock ) ) {
                CErr ( "PerformIOSX: Failed to unlock portalWorkerEventLock." );
            }
        }
        return success;
    }
    
    

} /* namespace environs */
