/**
 * Dummy Touch Gestures Recognizer
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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#define ENVIRONS_NATIVE_MODULE

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Gesture.Dummy.h"
#include "Device/Device.Base.h"
using namespace environs;

#define CLASS_NAME			"GestureDummy"


static const char					*		DummyTouchRecognizer_extensionNames []	= { "Dummy Touch Recognizer", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType::InterfaceType	DummyTouchRecognizer_interfaceTypes []	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };



/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( DummyTouchRecognizer_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( DummyTouchRecognizer_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( GestureDummy );

#endif


namespace environs 
{
	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	GestureDummy::GestureDummy ( )
	{
		CLogID ( "Construct" );

		name				= DummyTouchRecognizer_extensionNames[0];

		triggerTouchCount	= 3; /// makes sense...
	}


	GestureDummy::~GestureDummy ( )
	{
		CLogID ( "Destructor" );
	}
	

	bool GestureDummy::Init ( )
	{
		CVerbID ( "Init" );

		if ( device_width == 0 || device_height == 0 )
			return false;

		return true;
	}
	

	int GestureDummy::Trigger ( InputPackRec ** inputs, int inputCount )
	{
		CVerbID ( "Trigger" );

		/// If we return 1, then we signal that we want to take over gesture recognizing and don't allow subsequent recognizers to do their job.
		/// If we return 2, then we signal that we do gesture recognizing, but allow other recognizers (the next within the chain) may aplly as well.
		/// It would make sense to analyse the touchpacks for a initial triggering condition to match.
        
        if ( !deviceBase )
            return 0;
        
        deviceID = ((DeviceBase *)deviceBase)->deviceID;
        
		return 2;
	}


	int GestureDummy::Perform ( InputPackRec ** inputs, int inputCount )
	{
		//CVerbID ( "Perform" );

		/// If we return 1, then we have taken over recognizing and are still recognizing the gesture and don't allow subsequent recognizers to do their job.
		/// If we return 2, then we signal that we do gesture recognizing, but allow other recognizers (the next within the chain) may aplly as well.
        
        PortalInfo info;
        info.deviceID = deviceID;
        info.portalID = PORTAL_DIR_INCOMING;
        
        if ( !deviceBase || !((DeviceBase *)deviceBase)->GetPortalInfo ( &info ) ) {
            CVerbArgID ( "Perform: GetPortalInfo deviceID [%u] failed.", info.deviceID );
            return 0;
        }
        
		/// We return 0 to signal that we are done with gesture recognizing
		return 0;
    }
    
    
    void GestureDummy::Flush ( )
    {
        //CVerbID ( "Flush" );
    }


} /* namespace environs */
