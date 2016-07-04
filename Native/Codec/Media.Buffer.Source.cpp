/**
 *	RenderBuffer MediaSource source
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

#ifdef ANDROID

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Media.Buffer.Source.h"
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>

#include "Environs.lib.h"
#include "Environs.Obj.h"
#include "Environs.native.h"
using namespace environs::API;

#include "Portal/Portal.Stream.h"
#include "Interfaces/IPortal.Capture.h"
#include "Portal.Worker.Stages.h"

#define	CLASS_NAME 	"MediaBufferSource"


namespace environs
{

MediaBufferSource::MediaBufferSource ()
{
	CLog ( "Construct" );

	deviceID 		= 0;
	width 			= 0;
	height 			= 0;
	format 			= 0;
	encoder			= 0;
}


MediaBufferSource::~MediaBufferSource ()
{
	CLogID ( "Destruct" );

	format 			= 0;
	width 			= 0;
	height 			= 0;
	encoder			= 0;
}


bool MediaBufferSource::initSource ( IPortalEncoder	* encoder )
{
	if ( !encoder ) {
		CErr ( "initSource: called with NULL encoder argument!" );
		return false;
	}
	this->encoder = encoder;
	deviceID = encoder->deviceID;

	CVerbID ( "initSource" );

	if ( format == 0 ) {
		format = new MetaData;
		if ( format == 0 ) {
			CErrID ( "initSource: Failed to create new MetaData for MediaSource." );
			return false;
		}
	}

	format->setCString ( kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC );
	//format->setCString ( kKeyAVCC, MEDIA_MIMETYPE_VIDEO_AVC );

	if ( ((environs::WorkerStages *)encoder->stages)->render ) {
		//payload = 0; //((IPortalRenderer *) encoder->renderer)->
	}
	else {
		width = encoder->width;
		height = encoder->height;
	}

	if ( !setStreamResolution (width, height) )
		return false;

	return true;
}


status_t MediaBufferSource::read ( MediaBuffer **buffer, const MediaSource::ReadOptions *options )
{
	CVerbID ( "read" );

	status_t ret = 0;
	bool found = false;
	static long long pts = 1;
	pts++;

	char * payload = 0;
	unsigned int payloadSize = 0;
	IPortalCapture * capture = 0;

	CVerbID ( "read: ..." );

	while ( encoder )
	{
		if ( ((environs::WorkerStages *)encoder->stages)->render )
			payload = 0; //((IPortalRenderer *) encoder->renderer)->
		else if ( ((environs::WorkerStages *)encoder->stages)->capture )
        {
			capture = ((environs::WorkerStages *)encoder->stages)->capture;
	        /*if ( ___sync_val_compare_and_swap ( &capture->dataAccessed, 1, 1 ) == 1 ) {
			//if ( capture->dataAccessed ) {
				CVerbID ( "read: Data buffer has not been refilled..." );

		        if ( capture->portalWorkerEvent && capture->portalWorkerEventLock ) {
		        	pthread_mutex_lock ( (pthread_mutex_t *)capture->portalWorkerEventLock );
		        	pthread_cond_wait ( (pthread_cond_t *)capture->portalWorkerEvent, (pthread_mutex_t *)capture->portalWorkerEventLock );
		        	pthread_mutex_unlock ( (pthread_mutex_t *)capture->portalWorkerEventLock );
		        	continue;
		        }
				//break;
			}
	        */

			payload = (char *) capture->data;
			payloadSize = capture->dataSize;
		}

		if ( !payload ) {
			CVerbID ( "read: No data available." );
			return ERROR_END_OF_STREAM;
		}

		CVerbArgID ( "read: Copy [%i] bytes", payloadSize );

		ret = buffers.acquire_buffer ( buffer );

		if ( ret == OK ) {
			memcpy ( (*buffer)->data(), payload, payloadSize );
			(*buffer)->set_range ( 0, payloadSize );

			(*buffer)->meta_data()->clear ();
			//(*buffer)->meta_data()->setInt32(kKeyIsSyncFrame, packet.flags & AV_PKT_FLAG_KEY);
			(*buffer)->meta_data()->setInt64( kKeyTime, pts );

			if (capture) {
				capture->dataAccessed = 1;
			}
		}
		break;
	}

	return OK;
}


bool MediaBufferSource::setStreamResolution ( unsigned int width, unsigned int height )
{
	bool ret = false;

	CVerbID ( "setStreamResolution" );

	if ( this->width != width || this->height != height ) {
		if ( width && height ) {
			this->width		= width;
			this->height	= height;

			format->setInt32 ( kKeyWidth, width );
			format->setInt32 ( kKeyHeight, height );
		}
	}

	size_t bufferSize = ( width * height * 3); // / 2;
	if ( width && height ) {
		buffers.add_buffer ( new MediaBuffer(bufferSize) );
	}

	return true;
}


} /* namespace environs */

#endif

