/**
* Mouse simulator for Windows
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
#include "Mouse.Simulator.Win.h"
#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"
#include "Environs.Build.Lnk.h"

using namespace environs;
using namespace environs::lib;

#include <cmath>

// keybd_event ()

#define GESTURE_ITEM_HALFSIZE	15

#define CLASS_NAME	"Mouse.Simulator. . . . ."


static const char		*		WindowsMouseSimulator_extensionNames []	= { "Windows Mouse Simulator for Touch", "End" };
static const InterfaceType_t	WindowsMouseSimulator_interfaceTypes[]	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };

#ifndef ENVIRONS_CORE_LIB

#ifndef ENVIRONS_NATIVE_MODULE
#	define ENVIRONS_NATIVE_MODULE
#endif

/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( WindowsMouseSimulator_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( WindowsMouseSimulator_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( MouseSimulator );


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
	bool	MouseSimulator::enabled = true;

	int		MouseSimulator::desktopWidth;
	int		MouseSimulator::desktopHeight;


	void SendMouse ( int x, int y, unsigned int Flags )
	{
		INPUT inp;
		Zero ( inp );

		inp.type = INPUT_MOUSE;
		inp.mi.dx = (x * 65535) / MouseSimulator::desktopWidth;
		inp.mi.dy = (y * 65535) / MouseSimulator::desktopHeight;
		inp.mi.dwFlags = Flags | MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | 0x2000 | MOUSEEVENTF_VIRTUALDESK;

		SendInput ( 1, &inp, sizeof(inp) );
	}


// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
MouseSimulator::MouseSimulator ()
{
	CLog ( "Construct" );

	name				= WindowsMouseSimulator_extensionNames[0];

	desktopWidth = GetSystemMetrics ( SM_CXMAXTRACK );
	desktopHeight = GetSystemMetrics ( SM_CYMAXTRACK );
}


MouseSimulator::~MouseSimulator ()
{
	CLogID ( "Destructor" );
}


bool MouseSimulator::Init ()
{
	/*if ( !parent )
		return false;

	deviceID = ((DeviceBase *)parent)->deviceID;*/

	CVerbID ( "Init" );


	return true;
}


bool MouseSimulator::Init ( HWND hWnd )
{
	CVerbID ( "Init" );

	return true;
}


int MouseSimulator::Trigger ( InputPackRec **	inputs, int inputCount )
{
	return 2;
}


void MouseSimulator::Finish ( InputPackRec **	inputs, int inputCount )
{
}


int MouseSimulator::Perform ( InputPackRec **	inputs, int inputCount )
{
	Input * inputsContainer = (Input *)inputs;

	if ( enabled )
	{
		for ( int i = 0; i < inputCount; i++ ) {
			Input * touch = inputsContainer + i;

			switch ( touch->pack.raw.state ) 
			{
			case INPUT_STATE_ADD:
				//CLog ( "INPUT_STATE_ADD" );
				SendMouse ( touch->pack.raw.x, touch->pack.raw.y, MOUSEEVENTF_LEFTDOWN );
				break;
			case INPUT_STATE_CHANGE:
				//CLog ( "INPUT_STATE_CHANGE" );
				SendMouse ( touch->pack.raw.x, touch->pack.raw.y, MOUSEEVENTF_MOVE );
				break;
			case INPUT_STATE_DROP:
				//CLog ( "INPUT_STATE_DROP" );
				SendMouse ( touch->pack.raw.x, touch->pack.raw.y, MOUSEEVENTF_LEFTUP );
				break;
			}
		}
	}

	return false;
}


} /* namespace environs */

#endif


