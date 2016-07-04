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
#ifdef ANDROID

#include "Environs.types.h"
#include "Interfaces/IPortal.Encoder.h"

#ifndef INCLUDE_HCM_ENVIRONS_RENDERBUFFER_MEDIASOURCE_SOURCE
#define	INCLUDE_HCM_ENVIRONS_RENDERBUFFER_MEDIASOURCE_SOURCE

#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MediaBufferGroup.h>

using namespace android;

namespace environs
{

/**
*	RenderBuffer MediaSource source
*	--------------------------------------------------------------------------------------
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@remarks
* ****************************************************************************************
*/
class MediaBufferSource : public MediaSource
{
public:
	MediaBufferSource ();
	virtual ~MediaBufferSource ();

	virtual status_t 	read ( MediaBuffer **buffer, const MediaSource::ReadOptions *options );
	sp<MetaData> 		getFormat () { return format; }
	virtual status_t 	start ( MetaData *params ) { return OK; }
	virtual status_t 	stop () { return OK; }

	bool				initSource ( IPortalEncoder	* encoder );
	bool    			setStreamResolution ( unsigned int width, unsigned int height );

protected:
	unsigned int		deviceID;
    unsigned int 		width;
    unsigned int 		height;
    IPortalEncoder	*	encoder;

    sp<MetaData> 		format;
	MediaBufferGroup 	buffers;

private:

};


}

#endif	/// -> ANDROID

#endif	/// -> INCLUDE_HCM_ENVIRONS_PORTALSTREAM_MEDIASOURCE_SOURCE
