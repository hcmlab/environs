/**
* Echo bot app main
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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_ECHOBOT_CPP_EXAMPLE_H
#define INCLUDE_HCM_ENVIRONS_ECHOBOT_CPP_EXAMPLE_H

#include "Environs.h"

#if defined(_WIN32) && _MSC_VER <= 1600
#   define VS2010
#endif

#if defined(VS2010)
#	include "Interop/Threads.h"
#else
#	include <mutex>
#endif

/**
*	Echo bot functionality
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
*	@remarks
* ****************************************************************************************
*/
class EchoBot : environs::EnvironsObserver, environs::ListObserver, environs::MessageObserver, environs::DeviceObserver
{
public:
	EchoBot ();
	~EchoBot ();

	bool Init ();
	void Dispose ();
	void Run ();

private:
	bool	isRunning;
    
    int     listIDs;
    char    buffer [ 4096 ];
    char *  bufferEnd;
    
	bool		 resetAuth;
    const char * appNameCur;
    const char * areaNameCur;

	sp ( environs::Environs )	envSP;

	sp ( environs::DeviceList )	deviceListSP;
	environs::DeviceList *		deviceList;
    
#if defined(VS2010)
	environs::EnvLock listAccess;
#else
    std::mutex listAccess;
#endif
    
    /**
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param status      A status constant of environs::Status
     */
	void OnStatus ( environs::Status_t status );

    
	void OnListChanged ( const sp ( environs::DeviceInstanceList ) &vanished, const sp ( environs::DeviceInstanceList ) &appeared );
    
    void OnDeviceChanged ( const sp ( environs::DeviceInstance ) &device, environs::DeviceInfoFlag_t flags );
    
    /**
     * OnMessage is called whenever a text message has been received from a device.
     *
     * @param msg   The corresponding message object of type MessageInstance
     * @param flags Flags that indicate the object change.
     */
    void    OnMessage ( const sp ( environs::MessageInstance ) &msg, environs::MessageInfoFlag_t flags );
        
    void HandleLine ( char * line );
    
    void HandleSetEnv ( char * line );
    
    void Stop ();
    
    void PrintDevices ( char * line );    
    
    char * GetStartOfArg ( char * line );
    char * GetEndOfArg ( char * line );
    char * GetNextArg ( char * line );
    
    int GetFirstIntArg ( char * &line );

#ifdef TEST1
    static void * TestListerThread ( void * arg );
#endif

#ifdef TESTCONNECT
    static void * TestThread ( void * arg );
#endif
};

#endif