/**
 * Implementation of jpeg/png encoding
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

#include "Environs.Obj.h"
#include "Encoder/Encoder.Pictures.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"Encoder.Pictures . . . ."


namespace environs 
{


EncoderPictures::EncoderPictures ()
{
	this->deviceID = deviceID;

	name				= "Picture Stream Encoder";

    encodedType         = DATA_STREAM_IMAGE_DATA;
    
	CLogID ( "Construct" );
}


EncoderPictures::~EncoderPictures ()
{
	CLogID ( "Destruct" );
}

//
// Note: Init must be prepared for for a ReInit!
//
bool EncoderPictures::Init ( int deviceID, int BitRate, int Width, int Height, int FrameRate )
{
	this->deviceID = deviceID;

	CLogID ( "Init" );
	
	width = Width;
	height = Height;
	
	
	CLogID ( "Init successful." );
	return true;
}


int EncoderPictures::Perform ( RenderContext * context )
{

    if ( context )
        context->isIFrame = true;
    
	return 0;
}



} /* namespace environs */