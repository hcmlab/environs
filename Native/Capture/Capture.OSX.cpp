/**
 * OSX screen capture
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

#include "Environs.Obj.h"
#include "Capture.OSX.h"
using namespace environs;

#import <Cocoa/Cocoa.h>

#include "Interfaces/IPortal.Renderer.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"Capture.OSX. . . . . . ."


static const char					*		CaptureOSX_extensionNames[]	= { "OSX Screen Capture", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	CaptureOSX_interfaceTypes[]	= { InterfaceType::Capture, InterfaceType::Unknown };


/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( CaptureOSX_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( CaptureOSX_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( CaptureOSX );


/**
 * SetEnvironsMethods
 *
 *	Injects environs runtime methods.
 *
 */
BUILD_INT_SETENVIRONSOBJECT ();

#endif


namespace environs
{
	PortalBufferType_t	CaptureOSX_outputTypeSupport [] = { PortalBufferType::ARGBHandle, PortalBufferType::ARGB };
	

	CaptureOSX::CaptureOSX ( )
	{
		CLogID ( "Construct" );

		name				= CaptureOSX_extensionNames[0];
		captureType			= CaptureType::AppWindow;
		bufferType			= CaptureBufferType::PixelBuffer;

		data				= 0;
		dataSize			= 0;

		outputTypes			= CaptureOSX_outputTypeSupport;
		outputTypesLength	= sizeof(CaptureOSX_outputTypeSupport) / sizeof(CaptureOSX_outputTypeSupport [0]);
	}


	int CaptureOSX::Init ( )
	{
		CVerbID ( "Init" );

		return MainThreadedInit ( );
	}


	bool CaptureOSX::MainThreadedInit ( )
	{
		CVerbID ( "MainThreadedInit" );

		enabled = true;

		initialized = true;
		return true;
	}


	CaptureOSX::~CaptureOSX ( )
	{
		CLogID ( "Destruct" );

		Release ( );
	}


	bool CaptureOSX::MainThreadedDispose ( )
	{
		CVerbID ( "MainThreadedDispose" );

		return true;
	}


	void CaptureOSX::ReleaseOverlayBuffers ( RenderOverlay  * overlay )
	{
		CVerbID ( "ReleaseOverlayBuffers" );
	}


	void CaptureOSX::Release ( )
	{
		CVerbID ( "Release" );

		initialized = false;

		ReleaseResources ( );

		MainThreadedDispose ( );

		if ( renderOverlays && renderOverlayMutex ) {

			pthread_mutex_lock ( (pthread_mutex_t *) renderOverlayMutex );

			for ( unsigned int i = 0; i < MAX_PORTAL_OVERLAYS; i++ ) {
				if ( !renderOverlays [i] )
					continue;

				ReleaseOverlayBuffers ( renderOverlays [i] );
			}

			pthread_mutex_unlock ( (pthread_mutex_t *) renderOverlayMutex );
		}
	}
	

	int CaptureOSX::ReleaseResources ( )
	{
		CVerbID ( "ReleaseResources" );


		if ( dataHandle ) {
			CVerbID ( "ReleaseResources: Deleting bitmap capture handle" );

			dataHandle = 0;
		}
		data		= 0;
		dataSize	= 0;
		squareLength= 0;

		buffersInitialized	= false;
        
        return 1;
	}


	int CaptureOSX::AllocateResources ( RenderDimensions * dims )
	{
		CVerbID ( "AllocateResources" );

		data = 0;

		return 1;
	}


	int CaptureOSX::AllocateOverlayBuffers ( RenderOverlay  * overlay )
	{
		CVerbID ( "AllocateOverlayBuffers" );

		if ( overlay->renderArg1 )
			ReleaseOverlayBuffers ( overlay );

		return 1;
	}


	int CaptureOSX::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		//CVerbID ( "Perform" );

		int left = 0;
		int leftc = dims->left_cap;
		int top = 0;
		int topc = dims->top_cap;
		int width = dims->square;
		int height = dims->square;

		/// grab that frame
		if ( dims->left_cap < 0 || dims->top_cap < 0 ) {
			if ( dims->left_cap < 0 ) {
				left = -dims->left_cap;
				width -= left;
				leftc = 0;
			}

			if ( dims->top_cap < 0 ) {
				top = -dims->top_cap;
				height -= top;
				topc = 0;
			}
		}
        
        CGRect rect = CGRectMake ( leftc, topc, width, height );
        
        CGImageRef rectImage = CGDisplayCreateImageForRect ( kCGDirectMainDisplay, rect );
        if ( !rectImage )
            return 0;
        
        if ( data ) {
            CGImageRelease ( (CGImageRef)data) ;
        }
        data = (void *)rectImage;
        
		return 1;
	}



} /* namespace environs */

#endif

