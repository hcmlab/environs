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
#ifdef ANDROID

#ifndef INCLUDE_HCM_ENVIRONS_PORTALSTREAM_MEDIASOURCE_SOURCE
#define	INCLUDE_HCM_ENVIRONS_PORTALSTREAM_MEDIASOURCE_SOURCE

#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include "Decoder/Decoder.Base.h"

using namespace android;

namespace environs
{

/**
*	PortalStream MediaSource source
*	--------------------------------------------------------------------------------------
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@remarks
* ****************************************************************************************
*/
class MediaStreamSource : public MediaSource
{
public:
	MediaStreamSource ();
	virtual ~MediaStreamSource ();

	virtual status_t 	read(MediaBuffer **buffer, const MediaSource::ReadOptions *options);
	sp<MetaData> 		getFormat() { return format; }
	virtual status_t 	start(MetaData *params) { return OK; }
	virtual status_t 	stop() { return OK; }

	bool				initFormat ( PortalStream * streamSource );
	bool    			setStreamResolution ( unsigned int width, unsigned int height );

protected:
    unsigned int 		width;
    unsigned int 		height;

    sp<MetaData> 		format;
	PortalStream	*	portalStream;
	MediaBufferGroup 	buffers;

private:

};


}

#endif	/// -> ANDROID

#endif	/// -> INCLUDE_HCM_ENVIRONS_PORTALSTREAM_MEDIASOURCE_SOURCE
