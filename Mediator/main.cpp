/**
 * Mediator main
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
#include "Environs.Release.h"
#include "Mediator.Daemon.h"
#include "Environs.Native.h"
#include "Tracer.h"

#ifndef _WIN32
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>

#	ifndef _XOPEN_SOURCE
#		define _XOPEN_SOURCE
#	endif

#	include <execinfo.h>
#	include <signal.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include <unistd.h>
#	include <ucontext.h>
#endif

#ifndef CLASS_NAME
#	define CLASS_NAME	"Main . . . . . . . . . ."
#endif


int main(int argc, char* argv[])
{
	WSAData wsaData;
	if ( WSAStartup ( MAKEWORD ( 2, 2 ), &wsaData ) ) {
		printf ( "Init: Failed to initialize WinSock API!!!" );
		return false;
	}

	printf ( "Environs Mediator v. %s\n", ENVIRONS_VERSION_STRING );
    printf ( "------------------------\n" );

	InitTracer ();

    if ( !Mediator::InitClass() ) {
        printf ( "ERROR: Failed to initialize class Mediator!\n" );
        return 0;
    }
    
	MediatorDaemon * mediator = new MediatorDaemon();
	if ( !mediator ) {
		printf ( "ERROR: Failed to create Mediator. Possible reason: Low memory?\n" );
		return 0;
	}
	
	if ( !mediator->InitMediator () )
		return 0;

	mediator->OpenLog ();

	mediator->InitDefaultConfig ();
	
	if ( !mediator->Init () )
		return 0;
	
	// Load private key / public certificate
	mediator->LoadKeys ();

	// Read configuration mediator.conf, if available
	mediator->LoadConfig ();
	
	// Load mediator cache
	mediator->LoadProjectValues ();
	
	// Load device mappings and auth tokens
	mediator->LoadDeviceMappings ();
	
	// Load users db
	mediator->LoadUserDB ();

	// Create a working thread for each port
	mediator->CreateThreads ();

	// Wait for keys
	mediator->Run ();

	// Wait for each thread to terminate
	mediator->ReleaseThreads ();

	if ( !mediator->SaveUserDB () ) {
		printf ( "WARNING: Failed to save users db!\n" );
	}

	if ( !mediator->SaveDeviceMappings () ) {
		printf ( "WARNING: Failed to save device mappings!\n" );
	}

	if ( !mediator->SaveConfig () ) {
		printf ( "WARNING: Failed to save configuration!\n" );
	}

	if ( !mediator->SaveProjectValues () ) {
		printf ( "WARNING: Failed to save project value data!\n" );
	}
    
	// Release mediator keys
	mediator->ReleaseKeys ();
    
	delete mediator;

    Mediator::DisposeClass ();
    
    DisposeTracerAll ();
    
#ifdef _WIN32
	WSACancelBlockingCall ();
#endif
    
    WSACleanup ( );
    
    MediatorDaemon::DisposeClass ();

	return 0;
}
