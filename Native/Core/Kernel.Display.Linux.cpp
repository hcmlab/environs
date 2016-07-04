/**
 * Kernel for Mobile Devices (Unspecified Platform)
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

#include "Environs.Platforms.h"

#if (!defined(_WIN32) && !defined(ENVIRONS_IOS) && !defined(ENVIRONS_OSX))

#include "Kernel.Display.OSX.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Kernel.Display.Linux . ."


namespace environs 
{
	//
	// Static class members

    namespace API
    {
		void UpdateNetworkStatus ( )
		{
			CVerb ( "UpdateNetworkStatus" );
            
			/// Determine network connect status
            native.networkStatus = NETWORK_CONNECTION_LAN;
            
			//int netStat = NETWORK_CONNECTION_NO_NETWORK;            
		}
    }
    
    
    
    KernelPlatform::KernelPlatform()
    {
        CLog ( "Construct..." );
    }
    
    
    KernelPlatform::~KernelPlatform()
    {
        CLog ( "Destruct..." );
    }
    
    /*
     void KernelPlatform::DetermineAndInitWorkDir ()
     {
     CVerb ( "DetermineAndInitWorkDir" );
     
     WARNING ( "Kernel.Display.Linux::DetermineAndInitWorkDir: Implementation needs to be tested." )
     }
     */
    
    int KernelPlatform::onInitialized ()
    {
        CVerb ( "onInitialized" );
        
        return true;
    }
    
    
    int KernelPlatform::onPreStop ()
    {
        CVerb ( "onPreStop" );
        
        return true;
    }
    
    
    int KernelPlatform::onStopped ()
    {
        CVerb ( "onStopped" );
        
        return true;
    }
    
    
    void KernelPlatform::ReleaseLibrary ()
    {
        CVerb ( "ReleaseLibrary" );
    }
    
    
    int KernelPlatform::onPreInit ()
    {
        CVerb ( "onPreInit" );
        
        return true;
    }
    
    
    int KernelPlatform::onPreStart ()
    {
        CVerb ( "onPreStart" );
        
        return true;
    }
    
    
    int KernelPlatform::onStarted ()
    {
        CVerb ( "onStarted" );

		environs::API::SetNetworkStatusN ( NetworkConnection::LAN );
        
        return true;
    }
    
    
    
    void KernelPlatform::UpdateAppWindowSize ()
    {
        CVerb ( "UpdateAppWindowSize" );
        
    }
    
    
    // return values:
    // -1 means failed to set main application window handle, because of active portals at the moment
    // 0 means failed due to unknow reason
    // 1 means sucessfully set window handle
    int KernelPlatform::SetMainAppWindow ( WNDHANDLE appWnd )
    {
        return 1;
    }
    

} /* namespace environs */

#endif
