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
#include "stdafx.h"

#ifndef NDEBUG
#   define TEST1
//#   define TESTCONNECT
#endif

#include "Echo.Bot.CPP.h"
#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME	"EchoBot. . . . . ."



#ifdef LINUX
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>

void crashHandler ( int signum )
{
#define TRACEMAX	200
	void * traces [ TRACEMAX ];
	char **symbols;

	int traceCount = backtrace ( traces, TRACEMAX );

	symbols = backtrace_symbols ( traces, traceCount );
	if ( symbols == NULL ) {
		CErr ( "crashHandler: backtrace_symbols failed." );
	}
	else {
		for ( int i = 0; i < traceCount; i++ ) {
			printf ( "%s\n", symbols [ i ] );
			CErrArg ( "crashHandler: %s", symbols [ i ] );
		}

		free ( symbols );
	}

	signal ( signum, SIG_DFL );
	kill ( getpid (), signum );
}
#endif

#ifdef TEST1

bool			testListerRun   = true;
EchoBot *		g_echo          = 0;

size_t			testListenerCur = 0;
const size_t	testListenerCount = 20;
ThreadSync		testListerThreads [ testListenerCount ];

void * EchoBot::TestListerThread ( void * arg )
{
    ThreadSync * thread = (ThreadSync *) arg;
    if ( !thread || !g_echo )
        return 0;

    try
    {
        while (testListerRun)
        {
            g_echo->PrintDevices ( 0 );

            thread->WaitOne( "" );
            thread->ResetSync("");
        }
    }
    catch (...)
    {
    }
    
    return 0;
}
#endif

#ifdef TESTCONNECT

bool        testRun     = true;
ThreadSync  testThread;

void * EchoBot::TestThread ( void * arg )
{
    EchoBot * echo = (EchoBot *) arg;
    if ( !echo )
        return 0;

    int min = 500;
    int max = 10000;

    try
    {
        while (testRun)
        {
            if (echo->envSP->GetStatus () >= Status::Started)
            {
                echo->envSP->Stop();
                min = 500;
                max = 2000;
            }
            else if (echo->envSP->GetStatus () == Status::Stopped)
            {
                echo->envSP->ClearStorage();

                echo->envSP->Start();
                min = 20000;
                max = 30000;
                //max = 60000;
            }

            testThread.WaitOne( "", 500 + min + (rand() % max));
        }
    }
    catch (...)
    {
    }

    return 0;
}

#endif


int main ()
{
#ifdef LINUX
	signal ( SIGSEGV, crashHandler );
#endif

	EchoBot app;

	if ( !app.Init () ) {
		CErr ( "main: Failed to initialize app." );
		return false;
	}

    app.Run ();

    printf ( "-------------------------------------------------------\n" );
    printf ( "  Bye bye.\n" );

    return 0;
}


EchoBot::EchoBot ()
{
    isRunning       = true;
    resetAuth       = false;
    bufferEnd       = 0;
    listIDs         = 1;
	deviceList		= 0;

    appNameCur      = "ChatApp";
    areaNameCur     = "Environs";
}


EchoBot::~EchoBot ()
{
	if ( envSP ) {
		envSP->SetAppShutdown ( true );
		envSP->SetUseLogToStdout ( true );
	}

	Dispose ();
}


bool EchoBot::Init ()
{
#if defined(VS2010)
	if ( !listAccess.Init () )
		return false;
#endif

	if ( !envSP ) {
		// Initialize the id for our own list to 1 (increasing with each appeared device)
        listIDs = 1;

		// Create an Environs instance
		envSP = environs::Loader::CreateInstance ( appNameCur, areaNameCur );
        if ( !envSP ) {
            CErr ( "Init: Failed to create an instance." );
            return false;
        }

		// Do some housekeeping and remove all old files and messages
		envSP->ClearStorage ();
        
        // Let Environs know that we do not want to see any log messages (but errors) on stdout
		envSP->SetUseLogToStdout ( false );
        
		// Allow unencrypted connections from other devices
		envSP->SetUseCLSForDevicesEnforce ( false );

		// resetAuth is true, if we changed the appArea for which we need a new auth token from Mediator servers
		if ( resetAuth ) {
			// Enable/Disable anonymous authentication to clear auth tokens
			envSP->SetUseMediatorAnonymousLogon ( true );
			envSP->SetUseMediatorAnonymousLogon ( false );
			resetAuth = false;
		}

		// Let's observer the status of Environs
		envSP->AddObserver ( this );

		// Enable command line mode without app window
		envSP->SetUseHeadless ( true );

        // Check and query for logon credentials if necessary
		envSP->QueryMediatorLogonCommandLine ();

        // Dont apply a filter to the list. We want to see them all.
		envSP->SetMediatorFilterLevel ( MediatorFilter::None );

		// Alright, let's go ...
		envSP->Start ();
	}
	
    listAccess.lock ();

	if ( !deviceListSP ) {
		// Create a new device list with all devices that we can see
		deviceListSP = envSP->CreateDeviceList ( DeviceClass::All );
		if ( !deviceListSP )
			return false;

		// Let's observer changes of the device list (new and vanished devices)
		deviceListSP->AddObserver ( this );
		deviceList = deviceListSP.get ();
	}

    listAccess.unlock ();

#ifdef TEST1
    g_echo = this;
	for ( size_t i = 0; i < testListenerCount; ++i ) {
		testListerThreads [ i ].Init ();
		testListerThreads [ i ].Run ( pthread_make_routine ( &TestListerThread ), ( void * ) ( testListerThreads  + i), "Init", false );
	}
#endif

#ifdef TESTCONNECT
    testThread.Init ();
    testThread.Run ( pthread_make_routine ( &TestThread ), (void * ) this, "Init", false );
#endif
	return true;
}


void EchoBot::Dispose ()
{
    listAccess.lock ();

	if ( deviceListSP ) {
		deviceListSP->RemoveObserver ( this );
		deviceListSP = 0; deviceList = 0;
	}

    listAccess.unlock ();

	if ( envSP ) {
        //env->SetUseLogToStdout ( true );

		envSP->async = Call::Wait;
		envSP->Stop ();

		envSP->RemoveObserver ( this );
		envSP = 0;
	}
    
    environs::Loader::DisposeEnvironsLib ();
}


void PrintSmallHelp ()
{
	printf ( "  Press ESC to quit; h for help.\n" );
}

void PrintHelp ()
{
	printf ( "-------------------------------------------------------\n" );
	printf ( "  h[elp]               - print help\n" );
    printf ( "  p[rint] [page numer] - print available devices\n" );
    printf ( "  s[top]               - stop Environs\n\n" );
    printf ( "  d[ispose]            - dispose Environs\n\n" );
    printf ( "  q[uit]               - quit application\n\n" );

    printf ( "  set env appName areaName\n" );
	printf ( "-------------------------------------------------------\n" );
}



void EchoBot::Run ()
{
	PrintSmallHelp ();
	printf ( "-------------------------------------------------------\n" );

    bufferEnd = buffer + ( sizeof(buffer) - 2 );
    bool doPropmt = true;

	do
	{
        if ( doPropmt )
            printf ( "> " );
        doPropmt = true;

        char * line = fgets ( buffer, sizeof(buffer), stdin );
        if ( !line ) {
            doPropmt = false;
            continue;
        }

        // Remove prepending spaces
        while ( *line == ' ' && line < bufferEnd )
            line++;

        int c = 0;

        char t = *(line + 1);
        if ( t == ' ' || t == '\n' ) {
            // One character command detected
            c = *line;
        }

        if ( !c ) {
			HandleLine ( line );
        }

		else if ( c == 27 || c == 'q' ) {
			break;
		}

        else if ( c == 'h' ) {
			PrintHelp ();
        }
        else if ( c == 's' ) {
            Stop ();
        }
        else if ( c == 'd' ) {
            Dispose ();
        }
		else if ( c == 'p' ) {
            PrintDevices ( line );
		}
	}
    while ( isRunning );

#ifdef TEST1
    testListerRun = false;

	for ( size_t i = 0; i < testListenerCount; ++i ) {
		testListerThreads [ i ].Notify ( "" );
		testListerThreads [ i ].WaitOne ( "" );
	}
#endif

#ifdef TESTCONNECT
    testRun = false;

    testThread.Notify("");
    testThread.WaitOne("");
#endif
}


void EchoBot::Stop ()
{
#ifdef TESTCONNECT
	testRun = false;

	testThread.Notify ( "" );
	testThread.WaitOne ( "" );
#endif

    if ( deviceListSP ) {
		deviceListSP->RemoveObserver ( this );
		deviceListSP = 0; deviceList = 0;
    }
    
    if ( envSP ) {
        //env->SetUseLogToStdout ( true );
        
		envSP->async = Call::Wait;
		envSP->Stop ();
    }
}


/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param status      A status constant of environs::Status
 */
void EchoBot::OnStatus ( Status_t status )
{
	CLogArg ( "OnStatus: [ %i ]", status );

	const char * str;

	switch ( status )
	{
	case Status::Disposed:
		str = "Disposed";
		break;
	case Status::Uninitialized:
		str = "Uninitialized";
		break;
	case Status::Disposing:
		str = "Disposing";
		break;
	case Status::Initializing:
		str = "Initializing";
		break;
	case Status::Initialized:
		str = "Initialized";
		break;
	case Status::Stopped:
		str = "Stopped";
		break;
	case Status::StopInProgress:
		str = "StopInProgress";
		break;
	case Status::Stopping:
		str = "Stopping";
		break;
	case Status::Starting:
		str = "Starting";
		break;
	case Status::Started:
		str = "Started";
		break;
	case Status::Connected:
		str = "Connected";
		break;
	default:
		str = "Unknown";
		break;
	}

	printf ( "Environs: %s\n> ", str );
}


/**
 * OnMessage is called whenever a text message has been received from a device.
 *
 * @param msg           The corresponding message object of type MessageInstance
 * @param changedFlags  Flags that indicate the object change.
 */
void EchoBot::OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t changedFlags )
{
    if ( !msg || msg->sent() )
        return;

    try
    {
        const char * text = msg->text();
        if ( !text )
            return;

        const sp ( DeviceInstance ) &device = msg->device ();
        if ( !device )
            return;

        if ( *text == '$' ) {
            if ( strstr ( text, "$ca$0" ) ) {
                device->SendMessage ( Call::NoWait, "$ca$1$Echo" );
            }
            else if ( strstr ( text, "$ca$8" ) ) {
                device->SendMessage ( Call::NoWait, text );
            }
            else if ( strstr ( text, "$ca$4" ) ) {
                device->SendMessage ( Call::NoWait, "$ca$5$Hi i am your echo!" );
            }
            return;
        }

        printf ( "Message [ %4i ]: %s\n> ", device->appContext0, text );
        
        device->SendMessage ( Call::NoWait, text );
    }
    catch (...)
    {
    }
}


void EchoBot::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
    CLog ( "OnListChanged" );

    try
    {
        if ( vanished )
        {
            for ( int i=0; i< ( int ) vanished->size (); ++i )
            {
                sp ( DeviceInstance ) device = vanished->at (i);
                if ( device ) {
                    device->RemoveObserver ( this );

                    device->RemoveObserverForMessages ( this );

                    device->ClearStorage ();
                    device->ClearMessages ();
                }
            }
        }

        if ( appeared )
        {
            for ( int i=0; i< ( int ) appeared->size (); ++i )
            {
                sp ( DeviceInstance ) device = appeared->at (i);
                if ( !device )
                    continue;

                if ( !device->appContext0 ) {
                    device->async = Call::Wait;
                    device->ClearStorage ();
                    device->ClearMessages ();

                    device->appContext0 = listIDs++;

                    device->AddObserver ( this );

                    device->AddObserverForMessages ( this );
                }
            }
        }

#ifdef TEST1
        size_t i = ++testListenerCur;
        
        if ( i >= testListenerCount )
            i = testListenerCur = 0;
        
        testListerThreads [ i ].Notify ( "" );
#endif
    }
    catch (...)
    {
    }
}


void EchoBot::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags )
{
#ifdef TEST1
    CVerbN ( "OnDeviceChanged" );

    try
    {
        if ( !device )
            return;

        if ( (flags & DeviceInfoFlag::IsConnected) == DeviceInfoFlag::IsConnected )
        {
            size_t i = ++testListenerCur;

            if ( i >= testListenerCount )
                i = testListenerCur = 0;
            
            testListerThreads [ i ].Notify ( "" );
        }
    }
    catch (...)
    {
    }
#endif
}


void EchoBot::PrintDevices ( char * line )
{
    char * arg = line;

    int pageID = 0;
    
    if ( arg && *arg )
        pageID = GetFirstIntArg ( arg );
    
    //listAccess.lock ();

    //sp ( environs::DeviceList )	listSP = deviceList;
    
    //listAccess.unlock ();
    
    if ( !deviceList )
        return;
    //environs::DeviceList * list = listSP.get ();

    sp ( DeviceInstanceList ) devices;

    listAccess.lock ();
    
	if ( deviceList )
	{
		devices = deviceList->GetDevices ();
    }
    else
        printf ( "  No Devicelist!\n" );
    
    listAccess.unlock ();

    if ( devices ) {
        printf ( "  Devices [ %i ]:\n", ( int ) devices->size () );
        printf ( "----------------------------------------------------------------\n" );
        int i = 0;
        if ( pageID > 0 )
            i = (pageID - 1) * 10;

        for ( ; i < ( int ) devices->size (); ++i )
        {
            sp ( DeviceInstance ) device = devices->at ( i );
            if ( device ) {
                printf ( "  [ %4i ] %s\n", device->appContext0, device->toString ().c_str () );
            }
        }

        printf ( "----------------------------------------------------------------\n" );
    }
    else
        printf ( "  Devicelist is empty!\n" );
}


char * EchoBot::GetStartOfArg ( char * line )
{
    char * arg = line;

    while ( *arg && *arg == ' ' && *arg != '\n' && line < bufferEnd )
        ++arg;

    if ( line >= bufferEnd )
        return 0;
    return arg;
}


char * EchoBot::GetEndOfArg ( char * line )
{
    char * arg = line;

    while ( *arg && *arg != ' ' && *arg != '\n' && line < bufferEnd )
        arg++;

    if ( line >= bufferEnd )
        return 0;
    return arg;
}


char * EchoBot::GetNextArg ( char * line )
{
    char * argEnd = GetEndOfArg ( line );

    if ( argEnd ) {
        return GetStartOfArg ( argEnd );
    }
    return 0;
}


void EchoBot::HandleLine ( char * line )
{
    if ( !line || *line == '\n' ) {
        printf ( "\n" );
        PrintSmallHelp ();
    }

    else if ( strstr ( line, "help" ) == line ) {
        PrintHelp ();
    }
    else if ( strstr ( line, "print" ) == line ) {
        PrintDevices ( line );
    }
    else if ( strstr ( line, "quit" ) == line ) {
        isRunning = false;
    }
    else if ( strstr ( line, "stop" ) == line ) {
        Stop ();
    }
    else if ( strstr ( line, "dispose" ) == line ) {
        Dispose ();
    }
    else if ( strstr ( line, "set" ) == line ) {
        char * arg = GetNextArg ( line );

        if ( strstr ( arg, "env" ) == arg ) {
            HandleSetEnv ( GetNextArg ( arg ) );
        }
    }
}


int EchoBot::GetFirstIntArg ( char * &line )
{
    char * argLine = GetNextArg ( line );
    if ( !argLine ) {
        //printf ( "  Invalid arguments: %s\n", line );
        return 0;
    }

    char * argEnd = GetEndOfArg ( argLine );

    if ( !argEnd )
        return 0;

    *argEnd = 0;

    int arg = 0;
    sscanf ( argLine, "%i", &arg );

    *argEnd = ' ';

    line = argEnd + 1;
    return arg;
}


void EchoBot::HandleSetEnv ( char * line )
{
    if ( !line )
        return;

    char * appName = GetStartOfArg ( line );
    if ( !appName )
        return;

    char * areaName = GetNextArg ( appName );
    if ( !areaName )
        return;

    char * appCheck = areaName - 1;

    while ( *appCheck == ' ' && appCheck > appName )
        appCheck--;
    *(appCheck + 1) = 0;

    size_t areaLength = strlen ( areaName );
    if ( areaLength <= 0 )
        return;

    char * areaCheck = areaName + (areaLength - 1);

    while ( (*areaCheck == ' ' || *areaCheck == '\n' ) && areaCheck > areaName )
        areaCheck--;
    *(areaCheck + 1) = 0;

    Dispose ();

    appNameCur = appName;
    areaNameCur = areaName;

	resetAuth = true;

    Init ();
}























