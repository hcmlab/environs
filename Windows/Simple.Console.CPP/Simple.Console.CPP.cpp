/**
* Simple console app main
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
#include "Simple.Console.CPP.h"
#include "Environs.Native.h"

using namespace environs;

#define CLASS_NAME	"SimpleConsole. . . . . ."


int main ()
{

	SimpleConsole app;

	if ( !app.Init () ) {
		CErr ( "main: Failed to initialize app." );
		return false;
	}

    app.Run ();
    
    printf ( "-------------------------------------------------------\n" );
    printf ( "  Bye bye.\n" );
    
    return 0;
}


SimpleConsole::SimpleConsole ()
{
    isRunning       = true;
    bufferEnd       = 0;
    listIDs         = 1;
    currentListID   = -2;
    
    appNameCur      = "ChatApp";
    areaNameCur     = "Environs";
}


SimpleConsole::~SimpleConsole ()
{    
    if ( env )
        env->SetUseLogToStdout ( true );
    
	Dispose ();
}


bool SimpleConsole::Init ()
{
#if defined(VS2010)
	if ( !listAccess.Init () )
		return false;
#endif

	if ( !env ) {
        listIDs = 1;
        
		env = environs::Loader::CreateInstance ( appNameCur, areaNameCur );
        if ( !env ) {
            CErr ( "Init: Failed to create an instance." );
            return false;
        }
        env->SetUseLogToStdout ( false );

		env->AddObserver ( this );
		env->Start ();
	}

	if ( !deviceList ) {
		deviceList = env->CreateDeviceList ( DeviceClass::All );
		if ( !deviceList )
			return false;

		deviceList->AddObserver ( this );
	}

	return true;
}


void SimpleConsole::Dispose ()
{
	if ( deviceList ) {
		deviceList->RemoveObserver ( this );
		deviceList = 0;
	}

	if ( env ) {
		env->async = Call::Wait;
		env->Stop ();

		env->RemoveObserver ( this );
		env = 0;
	}
}

void PrintSmallHelp ()
{
	printf ( "  Press ESC to quit; h for help.\n" );
}

void PrintHelp ()
{
	printf ( "-------------------------------------------------------\n" );
	printf ( "  h[elp]               - print help\n" );
	printf ( "  l[log]               - toggle logging to file\n" );
	printf ( "  o[utput]             - toggle logging to output (std)\n" );
    printf ( "  p[rint] [page numer] - print available devices\n" );
    printf ( "  q[uit]               - quit application\n\n" );
    
    printf ( "  c[onnect] id\n" );
    printf ( "  d[isconnect] id\n" );
    printf ( "  sendm id message\n" );
    printf ( "  set env appName areaName\n" );
	printf ( "-------------------------------------------------------\n" );
}



void SimpleConsole::Run ()
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
			
        else if ( c == 'c' ) {
            Connect ( true, line );
        }
        else if ( c == 'd' ) {
            Connect ( false, line );
        }
        else if ( c == 'h' ) {
			PrintHelp ();
		}
		else if ( c == 'l' ) {
			env->SetUseLogFile ( !env->GetUseLogFile () );
        }
        else if ( c == 'o' ) {
            env->SetUseLogToStdout ( !env->GetUseLogToStdout () );
        }
		else if ( c == 'p' ) {
            PrintDevices ( line );
		}
	}
	while ( isRunning );
}


/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param status      A status constant of environs::Status
 */
void SimpleConsole::OnStatus ( Status_t status )
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
void SimpleConsole::OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t changedFlags )
{
    if ( !msg )
        return;

    const char * text = msg->text();
    if ( !text || *text == '$' )
        return;
    
    const sp ( DeviceInstance ) &device = msg->device();
    if ( !device )
        return;
    
    printf ( "Message [ %4i ]: %s\n> ", device->appContext0, text );
}


void SimpleConsole::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
    CLog ( "OnListChanged" );
    
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
                device->appContext0 = listIDs++;

                device->async = Call::Wait;
                device->ClearStorage ();
                device->ClearMessages ();
                
                device->AddObserverForMessages ( this );
                device->AddObserver ( this );
            }
        }
    }
}


void SimpleConsole::OnDeviceChanged ( const sp ( DeviceInstance ) &device, environs::DeviceInfoFlag_t changedFlags )
{
	CLogArg ( "OnDeviceChanged: flags [ %i ]", changedFlags );

    if ( !device )
        return;
    
    if ( currentListID != -1 && currentListID != device->appContext0 )
        return;
    
    if ( changedFlags == DeviceInfoFlag::ConnectProgress ) {
        printf ( ". " ); fflush ( stdout );
    }
    else if ( (changedFlags & DeviceInfoFlag::IsConnected) == DeviceInfoFlag::IsConnected )
    {
        if ( device->isConnected () ) {
            printf ( "\n  Device [ %i ] Connected.\n> ", device->appContext0 ); fflush ( stdout );
        }
        else  {
            printf ( "\n  Device [ %i ] Disconnected.\n> ", device->appContext0 ); fflush ( stdout );
        }
    }
}


sp ( environs::DeviceInstance ) SimpleConsole::GetDevice ( int listID )
{
    sp ( DeviceInstanceList ) list = deviceList->GetDevices ();
    
    for ( int i=0; i< ( int ) list->size(); ++i )
    {
        sp ( DeviceInstance ) device = list->at (i);
        
        if ( device && device->appContext0 == listID )
        {
            return device;
        }
    }
    return 0;
}


void SimpleConsole::PrintDevices ( char * line )
{
    char * arg = line;
    
    int pageID = 0;
    
    if ( arg && *arg )
        pageID = GetFirstIntArg ( arg );
    
    sp ( DeviceInstanceList ) devices = deviceList->GetDevices ();
    if ( devices ) {
        printf ( "  Devices [ %i ]:\n", ( int ) devices->size () );
        printf ( "----------------------------------------------------------------\n" );
        int i = 0;
        if ( pageID > 0 )
            i = (pageID - 1) * 10;
        
        listAccess.lock ();
        
        for ( ; i < ( int ) devices->size (); ++i )
        {
            sp ( DeviceInstance ) device = devices->at ( i );
            if ( device ) {
                printf ( "  [ %4i ] %s\n", device->appContext0, device->toString ().c_str () );
            }
        }
        
        listAccess.unlock ();
        
        printf ( "----------------------------------------------------------------\n" );
    }
    else
        printf ( "  No Devicelist!\n" );
}


char * SimpleConsole::GetStartOfArg ( char * line )
{
    char * arg = line;
    
    while ( *arg && *arg == ' ' && *arg != '\n' && line < bufferEnd )
        ++arg;
    
    if ( line >= bufferEnd )
        return 0;
    return arg;
}


char * SimpleConsole::GetEndOfArg ( char * line )
{
    char * arg = line;
    
    while ( *arg && *arg != ' ' && *arg != '\n' && line < bufferEnd )
        arg++;
    
    if ( line >= bufferEnd )
        return 0;
    return arg;
}


char * SimpleConsole::GetNextArg ( char * line )
{
    char * argEnd = GetEndOfArg ( line );
    
    if ( argEnd ) {
        return GetStartOfArg ( argEnd );
    }
    return 0;
}


int SimpleConsole::GetIntArg ( char * line )
{
    char * argEnd = GetEndOfArg ( line );
    
    if ( !argEnd )
        return 0;
    
    *argEnd = 0;
    
    int arg = 0;
    sscanf ( line, "%i", &arg );
    
    *argEnd = ' ';
    return arg;
}


void SimpleConsole::HandleLine ( char * line )
{
    if ( !line || *line == '\n' ) {
        printf ( "\n" );
        PrintSmallHelp ();
    }
    
    else if ( strstr ( line, "connect" ) == line ) {
        Connect ( true, line );
    }
    else if ( strstr ( line, "disconnect" ) == line ) {
        Connect ( false, line );
    }
    else if ( strstr ( line, "help" ) == line ) {
        PrintHelp ();
    }
    else if ( strstr ( line, "log" ) == line ) {
        env->SetUseLogFile ( !env->GetUseLogFile () );
    }
    else if ( strstr ( line, "output" ) == line ) {
        env->SetUseLogToStdout ( !env->GetUseLogToStdout () );
    }
    else if ( strstr ( line, "print" ) == line ) {
        PrintDevices ( line );
    }
    else if ( strstr ( line, "quit" ) == line ) {
        isRunning = false;
    }
    else if ( strstr ( line, "sendm" ) == line ) {
        SendMessage ( line );
    }
    else if ( strstr ( line, "set" ) == line ) {
        HandleSet ( GetNextArg ( line ) );
    }
}


void SimpleConsole::HandleSet ( char * line )
{
    if ( !line )
        return;
    
    if ( strstr ( line, "env" ) == line ) {
        HandleSetEnv ( GetNextArg ( line ) );
    }
}


void SimpleConsole::HandleSetEnv ( char * line )
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
    
    Init ();
}


int SimpleConsole::GetFirstIntArg ( char * &line )
{
    char * argLine = GetNextArg ( line );
    if ( !argLine ) {
        printf ( "  Invalid arguments: %s\n", line );
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


void SimpleConsole::SendMessage ( char * line )
{
    char * arg = line;
    
    int listID = GetFirstIntArg ( arg );
    if ( !listID ) {
        printf ( "  Invalid list id: %s\n", arg );
        return;
    }
    
    char * msg = GetNextArg ( arg );
    if ( !msg ) {
        printf ( "  Invalid msg: %s\n", arg );
        return;
    }
    
    sp ( DeviceInstance ) device = GetDevice ( listID );
    if ( !device ) {
        printf ( "  List id not found: %i\n", listID );
        return;
    }
    device->SendMessage ( msg );
}


void SimpleConsole::Connect ( bool connect, char * line )
{
    char * arg = line;
    
    int listID = GetFirstIntArg ( arg );
    if ( !listID ) {
		if ( !connect ) {
            arg = GetEndOfArg ( line );
            if ( arg ) {
                arg = GetStartOfArg ( arg );
                if ( arg && *arg == 'a' )
                {
                    currentListID = -1;
                    
                    sp ( DeviceInstanceList ) list = deviceList->GetDevices ();
                    
                    for ( int i=0; i< ( int ) list->size(); ++i )
                    {
                        sp ( DeviceInstance ) device = list->at (i);
                        
                        if ( device && device->isConnected () )
                        {
                            device->Disconnect ( Call::Wait );
                        }
                    }
                    
                    currentListID = -2;
                    return;
                }
            }

		}
        printf ( "  Invalid list id: %s\n", arg );
        return;
    }
    
    sp ( DeviceInstance ) device = GetDevice ( listID );
    if ( !device ) {
        printf ( "  List id not found: %i\n", listID );
        return;
    }
    
    currentListID = listID;
    
    if ( connect )
        device->Connect ();
    else
        device->Disconnect ();
}























