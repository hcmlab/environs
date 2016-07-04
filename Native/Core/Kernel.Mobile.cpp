/**
 * Kernel for Mobile Devices
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

#ifndef DISPLAYDEVICE

#include "Kernel.Mobile.h"
#include "Environs.Obj.h"
#include "Portal/Portal.Stream.h"
#include "Core/Callbacks.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"KernelMobile"


namespace environs 
{
	//
	// Static class members

    
    int Kernel::SetUseTouchRecognizer ( const char * moduleName, bool enable )
    {
        CVerbVerb ( "SetUseTouchRecognizer" );
        
        if ( enable ) {
            if ( !Kernel::AddTouchRecognizer ( moduleName ) )
                return 0;
        }
        else {
            int index = Kernel::RemoveTouchRecognizer ( moduleName );
            if ( index < 0 )
                return 0;
        }
        
        return 1;
    }
    
    
    void KernelDevice::InitStatics ()
    {
        CVerb ( "InitStatics" );
    }
    
    
    void KernelDevice::ReleaseStatics ()
    {
        CVerb ( "ReleaseStatics" );
    }
    
    
    KernelDevice::KernelDevice()
    {
        CVerb ( "Construct..." );
    }
    
    
    KernelDevice::~KernelDevice()
    {
        CVerb ( "Destruct..." );
    }
    
    
    int KernelDevice::onInitialized ()
    {
        CVerb ( "onInitialized" );
        
        return true;
    }
    
    
    int KernelDevice::onPreStop ()
    {
        CVerb ( "onPreStop" );
        
        return true;
    }
    
    
    int KernelDevice::onStopped ()
    {
        CVerb ( "onStopped" );
        
        return true;
    }
    
    
    void KernelDevice::ReleaseLibrary ()
    {
        CVerb ( "ReleaseLibrary" );
    }


} /* namespace environs */

#endif
