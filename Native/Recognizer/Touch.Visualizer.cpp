/**
* Environs Touch Visualizer for Windows
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

#ifndef DISPLAYDEVICE
#   define DISPLAYDEVICE
#endif

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Touch.Visualizer.h"
#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"
#include "Environs.Build.Lnk.h"

using namespace environs;
using namespace environs::lib;

#include <cmath>

#define GESTURE_ITEM_HALFSIZE	15

#define CLASS_NAME	"Touch.Visualizer . . . ."

static const char		*		TouchVisualizer_extensionNames []	= { "Touch Visualizer", "End" };
static const InterfaceType_t	TouchVisualizer_interfaceTypes[]	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };

#ifndef ENVIRONS_CORE_LIB

/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( TouchVisualizer_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( TouchVisualizer_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( TouchVisualizer );


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
	bool TouchVisualizer::enabled = true;

// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
TouchVisualizer::TouchVisualizer ()
{
	CLogID ( "Construct" );

	name				= TouchVisualizer_extensionNames[0];

	hAppWnd				= 0;
	hDCTouchDisplay		= 0;
	hTouchBrush			= 0;
}


TouchVisualizer::~TouchVisualizer ()
{
	CLogID ( "Destructor" );
	
	if ( hDCTouchDisplay ) {
		// Issue: this must be called by the same thread that called GetDC
		if ( hAppWnd )
			ReleaseDC ( hAppWnd, hDCTouchDisplay );
		else
			DeleteDC ( hDCTouchDisplay );
		hDCTouchDisplay = 0;
	}

	if ( hTouchBrush ) {
		DeleteObject ( hTouchBrush );
		hTouchBrush = 0;
	}

	hAppWnd = 0;
}


bool TouchVisualizer::Init ()
{
	//if ( !parent )
	//	return false;

	//deviceID = ((DeviceBase *)parent)->deviceID;

	CVerbID ( "Init" );

	if ( !hDCTouchDisplay ) {
		// Issue: this same thread must call ReleaseDC
		if ( hAppWnd )
			hDCTouchDisplay = GetDC ( hAppWnd );
		else
			hDCTouchDisplay = CreateDCA ( "DISPLAY", NULL, NULL, NULL );

		if ( !hDCTouchDisplay ) {
			return false;
		}
	}	

	if ( !hTouchBrush ) {
		hTouchBrush = CreateSolidBrush ( RGB(0, 255, 0) );

		if ( !hTouchBrush )
			return false;
	}

	return true;
}


bool TouchVisualizer::Init ( HWND hWnd )
{
	CVerbID ( "Init" );

	hAppWnd = hWnd;

	return true;
}


int TouchVisualizer::Trigger ( InputPackRec **	inputs, int inputCount )
{
	return 2;
}


void TouchVisualizer::Finish ( InputPackRec **	inputs, int inputCount )
{
}


int TouchVisualizer::Perform ( InputPackRec **	inputs, int inputCount )
{
	if ( !enabled )
		return false;

	Input * inputsContainer = (Input *)inputs;

	HBRUSH hOldBrush = (HBRUSH) SelectObject ( hDCTouchDisplay, hTouchBrush );
			
	// Update UI touch circles
	for ( int i = 0; i < inputCount; i++ ) {
		Input * in = inputsContainer + i;

		if ( in->pack.raw.x != in->x_old || in->pack.raw.y != in->y_old || in->pack.raw.state == INPUT_STATE_DROP ) {

			RECT r = { in->x_old - GESTURE_ITEM_HALFSIZE,
				in->y_old - GESTURE_ITEM_HALFSIZE,
				in->x_old + GESTURE_ITEM_HALFSIZE,
				in->y_old + GESTURE_ITEM_HALFSIZE };

			InvalidateRect ( NULL, &r, false );
		}

		if ( in->pack.raw.state != INPUT_STATE_DROP ) {
			Ellipse ( hDCTouchDisplay, in->pack.raw.x - GESTURE_ITEM_HALFSIZE, in->pack.raw.y - GESTURE_ITEM_HALFSIZE,
				in->pack.raw.x + GESTURE_ITEM_HALFSIZE, in->pack.raw.y + GESTURE_ITEM_HALFSIZE );
		}

		//TextOutA ( hDC, 200, 600, "hello world", 12 );
	}

	SelectObject ( hDCTouchDisplay, hOldBrush );

	return false;
}


} /* namespace environs */

#endif

