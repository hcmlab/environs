/**
 * Environs large display platform commons
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

#ifdef DISPLAYDEVICE 

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Utils.h"
using namespace environs;

#include "Renderer/Render.OpenCL.h"
#include "Recognizer/Mouse.Simulator.Win.h"
#include "Recognizer/Touch.Visualizer.h"

#include <fstream>
#include <sstream>
#include <string>
using namespace std;

// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.Display . . . ."


namespace environs 
{
    extern void LogBuildCommit ();
    
	bool					opt_useDeviceMarkerHandler = false;
	bool					opt_useDeviceMarkerReducedPrecision = false;

    
#ifndef ENVIRONS_OSX
    void DeallocNativePlatform ()
    {
        
    }
#endif


#define ENVIRONS_CONF_FILE  "env.conf"


	char * GetConfPath ()
	{
		const char * workDir = native.workDir;

		if ( !workDir )
			workDir = "./";

		size_t length = strlen ( workDir );

		char * path = ( char * ) malloc ( length + sizeof ( ENVIRONS_CONF_FILE ) + 4 );
		if ( path ) {
			sprintf ( path, "%s%s", workDir, ENVIRONS_CONF_FILE );
		}
		return path;
	}


	bool SaveConfig ( ofstream &conffile, Instance * env )
	{
		CVerb ( "SaveConfig" );

		if ( !env || !conffile.good () )
			return false;

		conffile << ":: " << env->appName << " " << env->areaName << endl;

		// DeviceID
		conffile << "id: " << env->deviceID << endl;

		// useStream
		conffile << "useStream: " << env->useStream << endl;

		// useStreamUltrafast
		conffile << "useStreamUltrafast: " << env->useStreamUltrafast << endl;

		// useOCL
		conffile << "useOCL: " << env->useOCL << endl;

		// usePNG
		conffile << "usePNG: " << env->usePNG << endl;

		// useAnonymousLogon
		conffile << "useAnonymousLogon: " << env->useAnonymous << endl;

		// useNativeResolution
		conffile << "useNativeResolution: " << env->useNativeResolution << endl;

		// useTCPPortal
		conffile << "useTCPPortal: " << env->streamOverCom << endl;

		// visualizeTouches
		conffile << "visualizeTouches: " << env->visualizeTouches << endl;

		// simulateMouse
		conffile << "simulateMouse: " << env->simulateMouse << endl;

		// streamBitrateKB
		conffile << "streamBitrateKB: " << env->streamBitrateKB << endl;

		// useRecognizer
		conffile << "useRecognizer: " << env->useRecognizers << endl;

		// useRenderCache
		conffile << "useRenderCache: " << env->useRenderCache << endl;

		// portalAutoAccept
		conffile << "portalAutoAccept: " << env->portalAutoAccept << endl;

		if ( *env->CustomMediatorToken )
			conffile << "optCustomMedToken: " << env->CustomMediatorToken << endl;

		if ( *env->CustomMediatorUserName )
			conffile << "optCNustomMedToken: " << env->CustomMediatorUserName << endl;

		if ( *env->DefaultMediatorToken )
			conffile << "optDefaultMedToken: " << env->DefaultMediatorToken << endl;

		if ( *env->DefaultMediatorUserName )
			conffile << "optDNefaultMedToken: " << env->DefaultMediatorUserName << endl;

		if ( *env->UserName )
			conffile << "optUserName: " << env->UserName << endl;

		if ( env->mod_PortalEncoder && *env->mod_PortalEncoder )
			conffile << "modPortalEncoder: " << env->mod_PortalEncoder << endl;

		if ( env->mod_PortalDecoder && *env->mod_PortalDecoder )
			conffile << "modPortalDecoder: " << env->mod_PortalDecoder << endl;

		if ( env->mod_PortalCapturer && *env->mod_PortalCapturer )
			conffile << "modPortalCapturer: " << env->mod_PortalCapturer << endl;

		if ( env->mod_PortalRenderer && *env->mod_PortalRenderer )
			conffile << "modPortalRenderer: " << env->mod_PortalRenderer << endl;

		conffile << "tuioPort: " << env->tuioPort << endl;

#ifdef _WIN32
		// useWinD3D
		conffile << "useWinD3D: " << opt_useWinD3D << endl;

		// useWinTGDI
		conffile << "useWinTGDI: " << opt_useWinTGDI << endl;
#endif

		return true;
	}


	bool SaveConfig ()
	{
		CLog ( "SaveConfig" );

		bool success = false;

		if ( !LockAcquireA ( native.kernelLock, "SaveConfig" ) )
			return false;

		ofstream conffile;
		ifstream conffileOld;

		string conffileBak;

		char * path = GetConfPath ();

		do
		{
			conffileBak = path;
			conffileBak += ".bak";

			conffile.open ( conffileBak );
			if ( !conffile.good () )
				goto Finish;

			conffile << "0:";

			// Save app/area mappings
			for ( map<string, int>::iterator it = native.envMapping.begin (); it != native.envMapping.end (); ++it )
			{
				conffile << it->first << " " << it->second << ";";
			}
			conffile << endl << endl;

			// DeviceUID
			conffile << "uid: " << native.deviceUID << endl;

			conffile << "useNotifyDebug: " << native.useNotifyDebugMessage << endl;

            conffile << "useLogFile: " << native.useLogFile << endl;

            conffile << "useWifiObserver: " << native.useWifiObserver << endl;

			conffile << "useWifiInterval: " << native.useWifiInterval << endl;

            conffile << "useBtObserver: " << native.useBtObserver << endl;

			conffile << "useBtInterval: " << native.useBtInterval << endl;

#if defined(_WIN32) && !defined(NDEBUG)
			conffile << "useDebugHeap: " << native.useDebugHeap << endl;
#endif
			conffileOld.open ( path );
			if ( conffileOld.good () )
			{
				/// Search through to the first app/area

				bool push = false;
				bool done [ ENVIRONS_MAX_ENVIRONS_INSTANCES ] = { 0 };

				string line;
				while ( getline ( conffileOld, line ) )
				{
					const char * str = line.c_str ();
					size_t len = strlen ( str );
					if ( len < 4 )
						continue;

					if ( str [ 0 ] == ':' && str [ 1 ] == ':' ) {
						string prefix, area, app;

						push = true;

						std::istringstream iss ( line );

						if ( !( iss >> prefix >> app >> area ) )
							continue;

						for ( int i=1; i < ENVIRONS_MAX_ENVIRONS_INSTANCES; i++ )
						{
							Instance * env = instances [ i ];
							if ( !env || done [i])
								continue;

							if ( !area.compare ( env->areaName ) && !app.compare ( env->appName ) ) {
								SaveConfig ( conffile, env );
								done [ i ] = true;
								push = false;
								break;
							}
						}
					}

					if ( push )
						conffile << line << endl;
				}

				for ( int i=1; i < ENVIRONS_MAX_ENVIRONS_INSTANCES; i++ )
				{
					Instance * env = instances [ i ];
					if ( !env || done [ i ] )
						continue;

					SaveConfig ( conffile, env );
				}
			}
			else 
			{
				for ( int i=0; i < ENVIRONS_MAX_ENVIRONS_INSTANCES; i++ )
				{
					Instance * env = instances [ i ];
					if ( !env )
						continue;

					SaveConfig ( conffile, env );
				}
			}

			success = true;
		}
		while ( 0 );

Finish:
		LockReleaseA ( native.kernelLock, "SaveConfig" );

		if ( conffileOld.is_open () )
			conffileOld.close ();
		if ( conffile.is_open () )
			conffile.close ();

		if ( success ) {
			string bak = path;
			bak += ".1.bak";
			std::remove ( bak.c_str () );

			std::rename ( path, bak.c_str () );

			if ( std::rename ( conffileBak.c_str (), path ) ) {
				CErr ( "SaveConfig: Failed to store configuration." );
				success = false;
			}
		}
		
		if ( path )
			free ( path );

		return success;
	}


	bool getUInt ( unsigned int &value, const string &line )
	{
		std::istringstream iss ( line );

		unsigned int parsed = 0;
		string prefix;

		if ( !( iss >> prefix >> parsed ) )
			return false;

		value = parsed;
		return true;
	}


	bool getInt ( int &value, const string &line )
	{
		std::istringstream iss ( line );

		int parsed = 0;
		string prefix;

		if ( !( iss >> prefix >> parsed ) )
			return false;

		value = parsed;
		return true;
	}


	bool getBool ( bool &value, const string &line )
	{
		std::istringstream iss ( line );

		bool parsed = 0;
		string prefix;

		if ( !( iss >> prefix >> parsed ) )
			return false;

		value = parsed;
		return true;
	}


	bool getCString ( char * &value, unsigned int bufSize, const string &line )
	{
		std::istringstream iss ( line );

		string parsed;
		string prefix;

		if ( !( iss >> prefix >> parsed ) )
			return false;

		if ( !bufSize ) {
			if ( value ) {
				free ( value );
				value = 0;
			}

			bufSize = ( unsigned int ) parsed.length () + 2;

			value = ( char * ) calloc ( 1, bufSize );
			if ( !value )
				return false;
		}

		strlcpy ( value, parsed.c_str (), bufSize - 1 );
		return true;
	}


	bool getAreaAppMapping ( const string &line )
	{
		std::istringstream iss ( line );

		string key; int eid = -1;

		if ( !( iss >> key >> eid ) )
			return false;

		return native.UpdateEnvID ( key.c_str (), eid ) > 0;
	}


	bool LoadConfig ( Instance * env )
	{
		CVerb ( "LoadConfig" );

		bool success = false;
		bool isGlobal = true;

		if ( !env || !*env->appName || !*env->areaName )
			return false;

		char *	tmpStr	= 0;

		char * path = GetConfPath ();

		ifstream conffile;
		conffile.open ( path );

		if ( !conffile.good () ) {
			free ( path );
			SaveConfig ();
			return false;
		}

		// Adapt default configuration

		string line;
		while ( getline ( conffile, line ) )
		{
			const char * str = line.c_str ();
			size_t len = strlen ( str );
			if ( len < 4 )
				continue;

			bool valid = false;

			if ( str [ 0 ] == ':' && str [ 1 ] == ':' ) {
				isGlobal = false;

				if ( success ) {
					/// Found a new app/area and that one we was looking for also. So.. we're done..
					break;
				}

				string prefix, area, app;

				std::istringstream iss ( line );

				if ( !( iss >> prefix >> app >> area ) )
					continue;

				if ( !area.compare ( env->areaName ) && !app.compare ( env->appName ) ) {
					success = true;
					continue;
				}
			}

			if ( isGlobal )
			{
				switch ( str [ 0 ] ) {
				case '0':
					if ( str [ 1 ] == ':' ) 
					{
						istringstream smaps ( line.c_str () + 2 );
						string map;

						while ( getline ( smaps, map, ';' ) ) 
						{
							//valid =
                            getAreaAppMapping ( map );
						}
					}
					break;

				case 'u':
					if ( str [ 1 ] == 'i' && str [ 2 ] == 'd' )
					{
						tmpStr = ( char * ) native.deviceUID;
						//valid =
                        getCString ( tmpStr, sizeof ( native.deviceUID ), line );
					}
					else if ( str [ 1 ] == 's' && str [ 2 ] == 'e' )
					{
						switch ( str [ 3 ] )
						{
						case 'B': // useBtInterval
							if ( str [ 5 ] == 'I' )
								getInt ( native.useBtInterval, line );
							else // useBtObserver
								 //valid =
								getBool ( native.useBtObserver, line );
							break;

#if defined(_WIN32) && !defined(NDEBUG)
						case 'D': // useDebugHeap
							if ( str [ 5 ] == 'b' )
								getInt ( native.useDebugHeap, line );
							break;
#endif
						case 'L': // useLogFile
								  //valid =
							getBool ( native.useLogFile, line );

							if ( native.useLogFile ) {
								LogBuildCommit ();
							}
							break;

						case 'N':
							switch ( str [ 4 ] )
							{
							case 'o':
								//valid =
								getBool ( native.useNotifyDebugMessage, line );
								break;
							}
							break;

						case 'W': // useWifiInterval
							if ( str [ 7 ] == 'I' )
								getInt ( native.useWifiInterval, line );
							else // useWifiObserver
								 //valid =
								getBool ( native.useWifiObserver, line );
							break;
						}
					}
				}
			}
			else if ( success )
			{
				switch ( str [ 0 ] ) {
				case 'i':
					if ( str [ 1 ] == 'd' ) {
						if ( !env->deviceID )
							valid = getInt ( env->deviceID, line );
						else
							valid = true;
					}
					break;

				case 'm':
					if ( line.length () > 11 && str [ 3 ] == 'P' ) {
						if ( str [ 9 ] == 'C' )
							valid = getCString ( env->mod_PortalCapturer, 0, line );
						else if ( str [ 9 ] == 'D' )
							valid = getCString ( env->mod_PortalDecoder, 0, line );
						else if ( str [ 9 ] == 'E' )
							valid = getCString ( env->mod_PortalEncoder, 0, line );
						else if ( str [ 9 ] == 'R' )
							valid = getCString ( env->mod_PortalRenderer, 0, line );
					}
					break;

				case 's':
					if ( str [ 1 ] == 'i' ) // simulateMouse
						valid = getBool ( env->simulateMouse, line );

					else if ( str [ 1 ] == 't' ) // streamBitrateKB
						valid = getInt ( env->streamBitrateKB, line );
					break;

				case 't': // tuioPort
					if ( str [ 1 ] == 'u' ) {
						unsigned int tmp = 0;

						valid = getUInt ( tmp, line );
						if ( valid )
							env->tuioPort = ( unsigned short ) tmp;
					}
					break;

				case 'p':
					if ( str [ 6 ] == 'A' ) // portalAutoAccept
						valid = getBool ( env->portalAutoAccept, line );
					break;

				case 'u':
					if ( str [ 1 ] == 's' && str [ 2 ] == 'e' )
					{
						switch ( str [ 3 ] )
						{
						case 'A': // useAnonymousLogon
							valid = getBool ( env->useAnonymous, line );
							if ( valid && env->useAnonymous )
								*env->UserName = 0;
							break;

						case 'N':
							switch ( str [ 4 ] ) {
							case 'a':
								valid = getBool ( env->useNativeResolution, line );
								break;
							}
							break;
						case 'O':
							valid = getBool ( env->useOCL, line );
							break;
						case 'P':
							valid = getBool ( env->usePNG, line );
							break;
						case 'R':
							if ( len < 8 )
								break;
							if ( str [ 5 ] == 'c' ) // useRecognizer
								valid = getBool ( env->useRecognizers, line );
							else // useRenderCache
								valid = getBool ( env->useRenderCache, line );
							break;
						case 'S':
							if ( len < 8 )
								break;
							if ( str [ 9 ] == 'U' )
								valid = getBool ( env->useStreamUltrafast, line );
							else
								valid = getBool ( env->useStream, line );
							break;
						case 'T':
							valid = getBool ( env->streamOverCom, line );
							break;
#ifdef _WIN32
						case 'W':
							if ( len < 8 )
								break;
							if ( str [ 6 ] == 'D' )
								valid = getBool ( opt_useWinD3D, line );
							else
								valid = getBool ( opt_useWinTGDI, line );
							break;
#endif
						}
					}
					break;
				case 'v':
					if ( str [ 1 ] == 'i' )
						valid = getBool ( env->visualizeTouches, line );
					break;
				case 'o':
					if ( str [ 1 ] == 'p' && str [ 2 ] == 't' ) {
						if ( str [ 3 ] == 'C' && str [ 4 ] == 'u' ) {
							tmpStr = ( char * ) env->CustomMediatorToken;
							valid = getCString ( tmpStr, sizeof ( env->CustomMediatorToken ), line );
						}
						else if ( str [ 3 ] == 'C' && str [ 4 ] == 'N' ) {
							tmpStr = ( char * ) env->CustomMediatorUserName;
							valid = getCString ( tmpStr, sizeof ( env->CustomMediatorUserName ), line );
						}
						else if ( str [ 3 ] == 'D' && str [ 4 ] == 'e' ) {
							tmpStr = ( char * ) env->DefaultMediatorToken;
							valid = getCString ( tmpStr, sizeof ( env->DefaultMediatorToken ), line );
						}
						else if ( str [ 3 ] == 'D' && str [ 4 ] == 'N' ) {
							tmpStr = ( char * ) env->DefaultMediatorUserName;
							valid = getCString ( tmpStr, sizeof ( env->DefaultMediatorUserName ), line );
						}
						else if ( str [ 3 ] == 'U' ) {
							tmpStr = ( char * ) env->UserName;
							valid = getCString ( tmpStr, sizeof ( env->UserName ), line );
						}
					}
					break;
				}

				if ( !valid ) {
					CWarnArg ( "LoadConfig: Invalid config line [%s]", str );
				}
            }
		}

		conffile.close ();
		free ( path );

		if ( env->appAreaID < 0 )
			env->appAreaID = native.UpdateEnvID ( env->appName, env->areaName, -1 );

#ifdef DEBUG_EXT_HEAP_CHECK
		if ( native.useDebugHeap > 0 )
		{
			int flags = _CrtSetDbgFlag ( _CRTDBG_REPORT_FLAG );

			//flags |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
			// _CRTDBG_DELAY_FREE_MEM_DF lags heavily under vs2010

			//flags |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_CHECK_EVERY_16_DF;
			flags |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF;

			flags |= native.useDebugHeap << 16; // 0x00010000; // check heap after every call to heap functions

			_CrtSetDbgFlag ( flags );
		}
#endif
		return success;
	}

} /* namespace environs */

#endif

