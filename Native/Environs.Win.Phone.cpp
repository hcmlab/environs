/**
 * Android JNI platform specific
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

#ifdef WINDOWS_PHONE



namespace environs 
{
    
    bool AllocNativePlatform () {
        
        return true;
    }
    
    
    /**
     * Creates an application identifier by means of a UUID
     *
     * @param	buffer	The UUID will be stored in this buffer.
     * @param	bufSize	The size of the buffer. Must be at least 180 bytes.
     * @return	success
     */
    bool CreateAppID ( char * buffer, unsigned int bufSize )
    {
        return false;
    }
    
    
	/**
	* Perform SDK checks to detect ...
	*
	*/
	void DetectSDKs ( )
	{
	}


	/**
	* Perform platform checks to detect ...
	*
	*/
	void DetectPlatform ( )
    {
	}

    /**
     * This is done by the android platform layer ...
     *
     */
    bool DetermineAndInitWorkDir ()
    {
        return true;
    }
    
    
    bool CreateInstancePlatform ( Instance * env )
    {
        //SetCallbacksN ( env->hEnvirons, 0, ( void * ) BridgeForUdpData, (void *)BridgeForMessage, (void *)BridgeForMessageExt, (void *)BridgeForNotify, (void *)BridgeForNotifyExt, (void *)BridgeForData, (void *)BridgeForStatusMessage );
        return true;
    }
    

	namespace API
    {
                
        void Environs_LoginDialog ( int hInst, const char * userName )
        {
        }
        

	} /* namespace API */


} /* namespace environs */



#endif



