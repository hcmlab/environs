/**
* A portal receiver receives portal stream packages, decodes them and dispatches them to the renderer
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

#ifdef _WIN32

#include "Environs.Obj.h"
#include "Environs.Modules.h"
#include "Portal.Receiver.Windows.h"

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Receiver.Windows."


namespace environs 
{

	int PortalReceiverPlatform::CreateDecoder ()
	{
		CVerbID ( "CreateDecoder" );

		if ( !decoder )
			decoder = (IPortalDecoder *) environs::API::CreateInstance ( LIBNAME_Decoder_Windows, 0, InterfaceType::Decoder, deviceID, env );

		if ( decoder )
			return DATA_STREAM_IMAGE_JPEG | DATA_STREAM_IMAGE_PNG | DATA_STREAM_H264_NALUS | DATA_STREAM_H265_NALUS;

		CErrID ( "CreateDecoder: Failed to load/create windows libav h264 decoder." );
		return 0;
	}

	

} /// -> namespace environs

#endif