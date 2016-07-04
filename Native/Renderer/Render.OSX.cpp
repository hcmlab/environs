/**
* GDI portal implementation
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

#include "Render.OSX.h"
#include "Environs.Obj.h"
#include "Core/Performance.Count.h"
#include "Device/Device.Controller.h"

#include "Math.h"
#define PI 3.14159265

#define CLASS_NAME	"Render.OSX . . . . . . ."



namespace environs 
{
//	
// Initialization of static values
    PortalBufferType_t	RenderGDI_inputTypeSupport [] = { PortalBufferType::CVPixelBufferIOSX };
    PortalBufferType_t	RenderGDI_outputTypeSupport [] = { PortalBufferType::YUV420 };


// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
RenderOSX::RenderOSX ()
{
	CLog ( "Construct" );

	name					= "OSX Renderer";

	inputTypes				= RenderGDI_inputTypeSupport;
	inputTypesLength		= sizeof(RenderGDI_inputTypeSupport) / sizeof(RenderGDI_inputTypeSupport [0]);

	outputTypes				= RenderGDI_outputTypeSupport;
	outputTypesLength		= sizeof(RenderGDI_outputTypeSupport) / sizeof(RenderGDI_outputTypeSupport [0]);
}


RenderOSX::~RenderOSX ()
{
	CLog ( "Destructor" );

	Dispose ();
}


bool RenderOSX::Init ()
{
	CLog ( "Init" );

	outputType = PortalBufferType::ARGB;

	return true;
}


void RenderOSX::Dispose ()
{
	CLog ( "Dispose" );

}


char * RenderOSX::GetBuffer ( RenderContext * context )
{

	return 0;
}


void RenderOSX::ReleaseBuffer ( RenderContext * context )
{
}


int RenderOSX::ReleaseResources ( RenderContext * context )
{
	CLog ( "ReleaseResources" );

	buffersInitialized = false;
    return 1;
}


int RenderOSX::AllocateResources ( RenderDimensions * dims )
{
	CLog ( "AllocateResources" );
    
    buffersInitialized = true;
	return 1;
}


int RenderOSX::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
{
	//CLog ( "Perform" );
	return 1;
}


bool RenderOSX::Perform ( RenderDimensions * dims, RenderContext * context ) // doPortalUpdateGDIDoubleBuffer
{
	//CVerbID ( "Perform" );

	rendered = false;

	bool		ret				= false;
	
    // Rotate image, scale image and convert to yuv
	
	rendered = ret;
	return ret;
}
    

bool RenderOSX::Compare ( unsigned int &equal )
{
	//
	// Memory compare of pixel values
	//
	//IPortalCapture * cap = ((WorkerStages *) stages)->capture;

	return false;
}


} /* namespace environs */

#endif


