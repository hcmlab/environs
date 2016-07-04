/**
 * Environs Mouse simulator example
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
#include "Mouse.Simulator.h"
#include <cmath>

#define CLASS_NAME	"Mouse.Simulator. . . . ."

#ifndef DISPLAYDEVICE
#   define DISPLAYDEVICE
#endif

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"
using namespace environs;

#include <cmath>

// keybd_event ()

#define GESTURE_ITEM_HALFSIZE	15


static const char		*		WindowsMouseSimulator_extensionNames []	= { "Mouse Simulator for Touch Input", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	WindowsMouseSimulator_interfaceTypes[]	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };

#define ENVIRONS_NATIVE_MODULE

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
BUILD_INT_SETENVIRONSMETHODS ();

#endif


namespace environs 
{
	bool MouseSimulator::enabled = true;


// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
MouseSimulator::MouseSimulator ()
{
	CLog ( "Construct" );

	name				= WindowsMouseSimulator_extensionNames[0];
}


MouseSimulator::~MouseSimulator ()
{
	CLog ( "Destructor" );
}


bool MouseSimulator::Init ()
{
	CLog ( "Init" );

	return true;
}
    int MouseSimulator::Trigger ( lib::InputPackRec **	inputs, int inputCount )
    {
        return 2;
    }
    
    
int MouseSimulator::Perform ( lib::InputPackRec **	inputs, int inputCount )
{
        lib::InputPackRec * inputsContainer = (lib::InputPackRec *)inputs;
        
	if ( enabled )
	{
		for ( int i = 0; i < inputCount; i++ ) {
			lib::InputPackRec * touch = inputsContainer + i;

			switch ( touch->raw.state )
			{
			case INPUT_STATE_ADD:
				break;
			case INPUT_STATE_CHANGE:
				break;
			case INPUT_STATE_DROP:
				break;
			}
		}
	}

	return false;
}


} /* namespace environs */
