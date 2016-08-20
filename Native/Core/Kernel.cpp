/**
* Environs Windows Kernel
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
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Lib.h"

#include "Kernel.h"

#ifdef DISPLAYDEVICE
#include "Renderer/Render.OpenCL.h"
#endif

#include <string>
#include <map>
using namespace std;



// The TAG for prepending to log messages
#define CLASS_NAME	"Kernel"


namespace environs 
{
	//
	// Static class members
	char       *	Kernel::touchRecognizerNames [ ENVIRONS_TOUCH_RECOGNIZER_MAX ];
	unsigned int    Kernel::touchRecognizerNamesCount = 0;

    LONGSYNC        environsKernelUniqueIDCounter = 128;
    
    //
	// Externals

	namespace API
	{
	}


	Kernel::Kernel ()
	{
		CVerb ( "Construct..." );

	}


	Kernel::~Kernel ()
	{
		CVerb ( "Destruct..." );

		Core::env->callbacks.Clear ();

		CVerb ( "Destruct done." );
	}


	void Kernel::ReleaseLibrary ()
	{
		CVerb ( "ReleaseLibrary" );

		KernelPlatform::ReleaseLibrary ();

		KernelDevice::ReleaseLibrary ();
	}


	int Kernel::onPreInit ()
	{
		CVerb ( "onPreInit" );

		KernelPlatform::env = Core::env;

		if ( KernelPlatform::onPreInit () )
            return KernelDevice::onPreInit ();

		return false;
	}


	int Kernel::onPreStart ()
	{
		CVerb ( "onPreStart" );

		if ( KernelPlatform::onPreStart () )
			return KernelDevice::onPreStart ();

		return false;
    }


	int Kernel::onStarted ()
	{
        CVerb ( "onStarted" );

#ifdef NATIVE_WIFI_OBSERVER_THREAD
        if ( native.useWifiObserver ) {
            if ( !native.wifiObserver.Start () )
                return false;
        }
#endif

#ifdef NATIVE_BT_OBSERVER_THREAD
		if ( native.useBtObserver ) {
			if ( !native.btObserver.Start () )
				return false;
		}
#endif

		if ( KernelPlatform::onStarted () )
            return KernelDevice::onStarted ();

		return false;
	}


	int Kernel::onPreStop ()
	{
		CVerb ( "onPreStop" );

#ifdef NATIVE_WIFI_OBSERVER_THREAD
		if ( native.useBtObserver )
			native.btObserver.Stop ();
#endif

#ifdef NATIVE_WIFI_OBSERVER_THREAD
		if ( native.useWifiObserver )
			native.wifiObserver.Stop ();
#endif
		if ( KernelPlatform::onPreStop () )
            return KernelDevice::onPreStop ();
        return false;
	}


	int Kernel::onStopped ()
	{
		CVerb ( "onStopped" );

		if ( KernelPlatform::onStopped () )
			return KernelDevice::onStopped ();

		return false;
	}


	int Kernel::SetMainAppWindow ( WNDHANDLE appWnd )
	{
#ifdef DISPLAYDEVICE
		if ( KernelDevice::SetMainAppWindow ( appWnd ) )
			if ( !KernelPlatform::SetMainAppWindow ( appWnd ) )
				return false;

		UpdateAppWindowSize ();

		if ( Core::env->useOCL ) {
			RenderOpenCL::InitOpenCL ();

			if ( !RenderOpenCL::ocl_initialized )
				Core::env->useOCL = false;
		}
#endif

		return 1;
	}


	void Kernel::UpdateAppWindowSize ()
	{
		CVerb ( "UpdateAppWindowSize" );

		KernelPlatform::UpdateAppWindowSize ();
	}

    
    int Kernel::AddTouchRecognizer ( const char * moduleName )
    {
		CVerb ( "AddTouchRecognizer" );

		if ( touchRecognizerNamesCount >= ENVIRONS_TOUCH_RECOGNIZER_MAX - 1 )
			return 0;

		/// Add to tracker names if neccessary
		char * trackerName = 0;

		for ( unsigned int i = 0; i < touchRecognizerNamesCount; i++ ) {
			char * name = touchRecognizerNames [ i ];

			if ( name && !strcmp ( name, moduleName ) ) {
				return 0;
			}
		}

		size_t len = strlen ( moduleName );
		if ( !len )
			return 0;

		trackerName = ( char * ) calloc ( 1, len + 2 );
		if ( !trackerName )
			return 0;

		memcpy ( trackerName, moduleName, len );

		touchRecognizerNames [ touchRecognizerNamesCount ] = trackerName;
		touchRecognizerNamesCount++;
        
        CVerbArg ( "AddTouchRecognizer: successfully added [ %s ]", moduleName );
        return 1;
    }
    
    
    int Kernel::RemoveTouchRecognizer ( const char * moduleName )
    {
		CVerb ( "RemoveTouchRecognizer" );

		for ( unsigned int i = 0; i < touchRecognizerNamesCount; i++ ) {
			char * name = touchRecognizerNames [ i ];

			if ( name && !strcmp ( name, moduleName ) ) 
			{
				for ( unsigned int j = i; j < touchRecognizerNamesCount - 1; j++ ) {
					touchRecognizerNames [ j ] = touchRecognizerNames [ j + 1 ];
				}

				touchRecognizerNamesCount--;

				free ( name );
				CVerbVerbArg ( "RemoveTouchRecognizer: successfully removed [ %s ]", moduleName );
				return i;
			}
		}
        
        CVerbArg ( "RemoveTouchRecognizer: [ %s ] not found.", moduleName );
        return -1;
    }
    
    
    int Kernel::GetUniqueInputID ()
    {
        
        long lCounter = __sync_add_and_fetch ( &environsKernelUniqueIDCounter, 1 );
        
		if ( lCounter & 0x80000000 ) {
			___sync_test_and_set ( &environsKernelUniqueIDCounter, 128 );
		}
        
        return (int)lCounter;
    }
    

} /* namespace environs */
