/**
 * Callback handlers
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

#include "Callbacks.h"
#include "Interop/Threads.h"
#include "Environs.Native.h"
#include "Kernel.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Device/Devices.h"
#include "Core/Input.Handler.h"
using namespace environs;

#include "Core/Byte.Buffer.h"

#include <map>
using namespace std;

// The TAG for prepending in log messages
#define CLASS_NAME	"Callbacks. . . . . . . ."


namespace environs
{    
    

	// dummy methods

	void CallBackConv dummyStatusMessage ( int hInst, const char * msg ) {
	}
    
	void CallBackConv dummyData ( int hInst, OBJIDType objID, int nativeID, int type, int fileID, const char * fileDescriptor, int size ) {
    }
    
    void CallBackConv dummyNotification ( int hInst, OBJIDType objID, int notification, int source, void * contextPtr, int context ) {
    }
    
    void CallBackConv dummyMessage ( int hInst, OBJIDType objID, int type, const void * msg, int length ) {
    }
    
    void CallBackConv dummyNotificationExt ( int hInst, int deviceID, const char * areaName, const char * appName, int notification, int source, void * context ) {
    }
    
    void CallBackConv dummyMessageExt ( int hInst, int deviceID, const char * areaName, const char * appName, int type, const void * msg, int length ) {
    }
    
	void CallBackConv dummyInput ( int hInst, int nativeID, Input * input ) {
    }
    
	void CallBackConv dummyData ( int hInst, OBJIDType objID, environs::lib::SensorFrame * input, int packSize ) {
    }


	ICallbacks::ICallbacks () : OnHumanInput( nill ), OnInputDelegate ( nill ), OnDataInput( nill ), doOnStatusMessage( false ),
		OnStatusMessage( nill ), OnNotify( nill ), OnMessage( nill ), OnNotifyExt( nill ), OnMessageExt( nill ), OnData( nill )
	{
		CVerbN ( "Construct" );

		env = 0;
    }

    
    ICallbacks::~ICallbacks ()
	{
		CVerb ( "Destruct" );

		Clear ();
        
        OnHumanInput = &dummyInput;

#ifdef DISPLAYDEVICE
		InputHandler::Release ();
#endif
    }


	bool ICallbacks::Init ( Instance * obj )
	{
		CVerb ( "Init" );

		env = obj;
        
		return Clear ();
    }


	bool ICallbacks::Clear ()
	{
		CVerb ( "Clear" );

#ifdef DISPLAYDEVICE
		OnHumanInput	= &InputHandler::FilterInput;
#else
		OnHumanInput	= &dummyInput;
#endif
		OnInputDelegate = 0;

		OnDataInput     = &dummyData;

		doOnStatusMessage = false;
		OnStatusMessage = &dummyStatusMessage;

		OnNotifyExt     = &dummyNotificationExt;
		OnMessageExt    = &dummyMessageExt;

		OnMessage		= &dummyMessage;
		OnNotify		= &dummyNotification;
		OnData			= &dummyData;

#ifdef DISPLAYDEVICE
		return InputHandler::Init ();
#else
		return true;
#endif
	}
    
    
}