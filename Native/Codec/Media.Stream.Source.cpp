/**
 * PortalStream MediaSource source
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

#include "Media.Stream.Source.h"
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>

#include "Environs.Lib.h"
#include "Environs.Obj.h"
#include "Environs.Native.h"
using namespace environs::API;

#include "Portal/Portal.Stream.h"

#define	CLASS_NAME 	"MediaStreamSource"


namespace environs
{

MediaStreamSource::MediaStreamSource ()
{
	CLog ( "Construct" );

	width 			= 0;
	height 			= 0;
	portalStream	= 0;
	format 			= 0;
}


MediaStreamSource::~MediaStreamSource ()
{
	CLog ( "Destruct" );

	portalStream 	= 0;
	format 			= 0;

	width 			= 0;
	height 			= 0;
}


bool MediaStreamSource::initFormat ( PortalStream * streamSource )
{
	CLog ( "initFormat" );

	portalStream = streamSource;

	if ( format == 0 ) {
		format = new MetaData;
		if ( format == 0 ) {
			CErr ( "initFormat: Failed to create new MetaData for AVSource." );
			return false;
		}
	}

	format->setCString ( kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC );
	//format->setCString ( kKeyAVCC, MEDIA_MIMETYPE_VIDEO_AVC );

	return true;
}


status_t MediaStreamSource::read ( MediaBuffer **buffer, const MediaSource::ReadOptions *options )
{
	//CLog ( "read" );

	status_t ret = 0;
	bool found = false;
	static long long pts = 1;
	pts++;

	while ( portalStream ) {
		//CLog ( "read: ..." );
		ByteBuffer * byteBuffer = (ByteBuffer *) portalStream->ReceiveStreamPack ();
		if ( !byteBuffer ) {
			return ERROR_END_OF_STREAM;
		}

		CLogArg ( "read: copy [%i] bytes", byteBuffer->payloadSize );

		ret = buffers.acquire_buffer ( buffer );

		if ( ret == OK ) {
			memcpy ( (*buffer)->data(), BYTEBUFFER_DATA_POINTER ( byteBuffer ), byteBuffer->payloadSize );
			(*buffer)->set_range ( 0, byteBuffer->payloadSize );
			(*buffer)->meta_data()->clear ();
			//(*buffer)->meta_data()->setInt32(kKeyIsSyncFrame, packet.flags & AV_PKT_FLAG_KEY);
			(*buffer)->meta_data()->setInt64( kKeyTime, pts );

		}
		break;
	}

	return OK;
}

bool MediaStreamSource::setStreamResolution ( unsigned int width, unsigned int height )
{
	bool ret = false;

	CLog ( "setStreamResolution" );

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

	ret = true;

	return true;
}

} /* namespace environs */

#endif

