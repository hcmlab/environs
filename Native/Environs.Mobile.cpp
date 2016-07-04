/**
 * Environs mobile specific
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

#ifndef DISPLAYDEVICE 

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Sensors.h"
#include <string>
#include <sstream>

using namespace std;
using namespace environs::API;

// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.Mobile. . . . ."

#ifdef ANDROID
#   define ENVIRONS_OPT_INITIAL_SETTINGS_MISSING   1
#else
#   define ENVIRONS_OPT_INITIAL_SETTINGS_MISSING   0
#endif

namespace environs 
{
	lib::SensorFrame 		g_OrientationFrame;
	lib::SensorFrame 		g_AccelFrame;
	unsigned int			g_sendSequenceNumber	= 0;

	bool					opt_useSensors			= true;
	bool					opt_usePushNotifications= false;
	bool					opt_useNativeDecoder	= false;
	bool					opt_useHardwareEncoder	= false;

    
    /*
     * Method:    AllocNativePlatformMobile
     * Signature: ()V
     */
    bool AllocNativePlatformMobile ( )
    {
        CInfo ( "AllocNativePlatformMobile" );
        
        // Determine device name
        /*
         #ifdef ANDROID
         char model_id [ PROP_VALUE_MAX ];
         int len = __system_property_get ( ANDROID_OS_BUILD_MODEL, model_id );
         if ( len > 0 )
         strlcpy ( Environs::DeviceName, model_id, MAX_NAMEPROPERTY );
         #endif
         */
		if ( !environs::dataRecSize )
			environs::dataRecSize    	= 66000;
        
        //*native.deviceUID        = 0;
        
        
        return true;
    }


	bool SaveMappings ()
    {
        CVerbVerb ( "SaveMappings" );
        
		stringstream  maps;

		// Save app/area mappings
		for ( map<string, int>::iterator it = native.envMapping.begin (); it != native.envMapping.end (); ++it )
		{
			maps << it->first << " " << it->second << ";";
		}

		opt ( 0, APPENV_MAPPINGS, maps.str ().c_str () );
        
        return true;
	}


	const char * BuildOptKey ( int eid, const char * key )
    {
        CVerbVerbArg ( "BuildOptKey [%i]: [%s]", eid, key );
        
		static char buffer [ 256 ];
		if ( snprintf ( buffer, 256, "%i_%s", eid, key ) < 0 )
			*buffer = 0;

		return buffer;
	}


	bool getAreaAppMapping ( const string &line )
    {
        CVerbVerb ( "getAreaAppMapping" );
        
		std::istringstream iss ( line );

		string key; int eid = -1;

		if ( !( iss >> key >> eid ) )
			return false;

		return native.UpdateEnvID ( key.c_str (), eid ) > 0;
	}


    /*
     * Method:    LoadConfig
     * Signature: ()V
     */
    bool LoadConfig ( Instance * env )
    {
        CInfo ( "LoadConfig" );
        
		int hInst			= env->hEnvirons;

        env->deviceID       = ENVIRONS_DEBUG_TAGID;
        env->tcpPort		= NATIVE_DEFAULT_DEVICE_PORT;
        env->udpPort		= NATIVE_DEFAULT_DEVICE_PORT;
        
        // Determine device name
        /*
         #ifdef ANDROID
         char model_id [ PROP_VALUE_MAX ];
         int len = __system_property_get ( ANDROID_OS_BUILD_MODEL, model_id );
         if ( len > 0 )
         strlcpy ( Environs::DeviceName, model_id, MAX_NAMEPROPERTY );
         #endif
         */
        
        if ( env->deviceType == DEVICE_TYPE_UNKNOWN )
            env->deviceType	= DEVICE_TYPE_TABLET;

		int eid = env->appAreaID;

		/// Get mappings
		if ( eid < 0 )
		{
			const char * maps = opt ( 0, APPENV_MAPPINGS );
			if ( maps && strlen ( maps ) > 4 )
			{
				istringstream smaps ( maps );
				string map;

				while ( getline ( smaps, map, ';' ) )
				{
					getAreaAppMapping ( map );
				}
			}

            if ( !*env->appName || !*env->areaName )
            {
                // Copy default app environment
                
				strlcpy ( env->areaName, DefAreaName, MAX_LENGTH_AREA_NAME );
				strlcpy ( env->appName, DefAppName, MAX_LENGTH_AREA_NAME );
            }
            
			eid = env->appAreaID = native.UpdateEnvID ( env->appName, env->areaName, -1 );
		}

		SaveMappings ();

        // Load options
		const char * initialKey = BuildOptKey ( eid, APPENV_SETTING_INITIALS );

        if ( optBool ( hInst, initialKey ) == ENVIRONS_OPT_INITIAL_SETTINGS_MISSING ) {
            opt ( hInst, initialKey, !((bool)ENVIRONS_OPT_INITIAL_SETTINGS_MISSING) );
            
            CVerb ( "LoadConfig: RESET to initial settings" );
            
            // Save default options
            opt ( hInst, APPENV_SETTING_GL_USE_NATIVE_DECODER,          opt_useNativeDecoder );
			opt ( hInst, APPENV_SETTING_GL_USE_HARDWARE_DECODER,        opt_useHardwareEncoder );

			opt ( hInst, APPENV_SETTING_GL_USE_SHOW_DEBUG_LOGS,         native.useNotifyDebugMessage );
            opt ( hInst, APPENV_SETTING_GL_USE_LOG_FILE,                native.useLogFile );
            opt ( hInst, APPENV_SETTING_GL_USE_BT_OBSERVER,             native.useBtObserver );
			opt ( hInst, APPENV_SETTING_GL_USE_BT_INTERVAL,				native.useBtInterval );
            opt ( hInst, APPENV_SETTING_GL_USE_WIFI_OBSERVER,           native.useWifiObserver );
			opt ( hInst, APPENV_SETTING_GL_USE_WIFI_INTERVAL,			native.useWifiInterval );
			opt ( hInst, APPENV_SETTING_GL_USE_PUSH_NOTIFS,             opt_usePushNotifications );
			opt ( hInst, APPENV_SETTING_GL_USE_SENSORS,                 opt_useSensors );

			env->optBool ( APPENV_SETTING_USE_PORTAL_AUTOSTART,			env->usePortalAutoStart );
			env->optBool ( APPENV_SETTING_USE_DEFAULT_MEDIATOR,			env->useDefaultMediator );
			env->optBool ( APPENV_SETTING_USE_CUSTOM_MEDIATOR,			env->useCustomMediator );
			env->optBool ( APPENV_SETTING_USE_PORTAL_TCP,				env->useTcpPortal );
			env->optBool ( APPENV_SETTING_USE_NATIVE_RESOLUTION,		env->useNativeResolution );
			env->optBool ( APPENV_SETTING_USE_STREAM,					env->useStream );
			env->optBool ( APPENV_SETTING_USE_CLS_MEDIATOR,				env->useCLS );
			env->optBool ( APPENV_SETTING_USE_CLS_DEVICE,				env->useCLSForDevices );
			env->optBool ( APPENV_SETTING_USE_CLS_DEV_ENFORCE,			env->useCLSForDevicesEnforce );
			env->optBool ( APPENV_SETTING_USE_AUTH,						env->useAuth );
			env->optBool ( APPENV_SETTING_USE_ANONYMOUS,				env->useAnonymous );
			env->optBool ( APPENV_SETTING_USE_PORTAL_AUTOACCEPT,		env->portalAutoAccept );
			env->optBool ( APPENV_SETTING_USE_MEDIATOR_LOGIN_DLG,		env->opt_useMediatorLoginDialog );
			env->optBool ( APPENV_SETTING_USE_PORTALVIEW_DIMS_AUTO,		env->usePortalViewDimsAuto );

			env->optBool ( APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT, env->CustomMediatorPort );

			//opt ( hInst, BuildOptKey ( eid, APPENV_SETTING_USE_CUSTOMMEDIATOR_IP ),		env->CustomMediatorIP );
            //opt ( "streamBitrateKB",            env->streamBitrateKB );
        }
        else {
            const char * value = env->opt ( APPENV_SETTING_DEVICE_ID );
            if ( value )
                sscanf_s ( value, "%i", &env->deviceID );
            
            CVerb ( "LoadConfig: LOAD settings" );
            
            opt_useSensors 						= optBool ( hInst, APPENV_SETTING_GL_USE_SENSORS  );
            opt_usePushNotifications 			= optBool ( hInst, APPENV_SETTING_GL_USE_PUSH_NOTIFS  );
            opt_useNativeDecoder 				= optBool ( hInst, APPENV_SETTING_GL_USE_NATIVE_DECODER  );
            opt_useHardwareEncoder 				= optBool ( hInst, APPENV_SETTING_GL_USE_HARDWARE_DECODER  );
            native.useNotifyDebugMessage		= optBool ( hInst, APPENV_SETTING_GL_USE_SHOW_DEBUG_LOGS  );
            native.useLogFile					= optBool ( hInst, APPENV_SETTING_GL_USE_LOG_FILE  );
            native.useBtObserver                = optBool ( hInst, APPENV_SETTING_GL_USE_BT_OBSERVER  );

			value = opt ( hInst, APPENV_SETTING_GL_USE_BT_INTERVAL );
			if ( value )
				sscanf_s ( value, "%i", &native.useBtInterval );

            native.useWifiObserver				= optBool ( hInst, APPENV_SETTING_GL_USE_WIFI_OBSERVER  );

			value = opt ( hInst, APPENV_SETTING_GL_USE_WIFI_INTERVAL );
			if ( value )
				sscanf_s ( value, "%i", &native.useWifiInterval );

            value = opt ( hInst, APPENV_SETTING_DEVICE_UID );
            if ( value && strlen ( value ) > 10 )
				snprintf ( native.deviceUID, sizeof ( native.deviceUID ), "%s", value );

            env->useStream 						= env->optBool ( APPENV_SETTING_USE_STREAM );
            env->useNativeResolution 			= env->optBool ( APPENV_SETTING_USE_NATIVE_RESOLUTION );
            env->useTcpPortal 					= env->optBool ( APPENV_SETTING_USE_PORTAL_TCP );
            env->useCustomMediator 				= env->optBool ( APPENV_SETTING_USE_CUSTOM_MEDIATOR );
            env->useDefaultMediator				= env->optBool ( APPENV_SETTING_USE_DEFAULT_MEDIATOR );
            env->usePortalAutoStart 			= env->optBool ( APPENV_SETTING_USE_PORTAL_AUTOSTART );
            env->useCLSForDevices 				= env->optBool ( APPENV_SETTING_USE_CLS_DEVICE );
            env->useCLSForDevicesEnforce 		= env->optBool ( APPENV_SETTING_USE_CLS_DEV_ENFORCE );
            env->useAuth                        = env->optBool ( APPENV_SETTING_USE_AUTH );
            env->useAnonymous                   = env->optBool ( APPENV_SETTING_USE_ANONYMOUS );
            env->portalAutoAccept				= env->optBool ( APPENV_SETTING_USE_PORTAL_AUTOACCEPT );

			env->opt_useMediatorLoginDialog		= env->optBool ( APPENV_SETTING_USE_MEDIATOR_LOGIN_DLG );
			env->CustomMediatorPort				= env->optBool ( APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT );
			env->usePortalViewDimsAuto			= env->optBool ( APPENV_SETTING_USE_PORTALVIEW_DIMS_AUTO );
            
            //            opt_useCLS                          = optBool ( "useCLSMediator" );

			env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT, 11, env->DefaultMediatorToken, sizeof ( env->DefaultMediatorToken ) );
            
			env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT_N, 4, env->DefaultMediatorUserName, sizeof ( env->DefaultMediatorUserName ) );

			env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM, 11, env->CustomMediatorToken, sizeof ( env->CustomMediatorToken ) );

			env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM_N, 4, env->CustomMediatorUserName, sizeof ( env->CustomMediatorUserName ) );

			env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_USERNAME, 4, env->UserName, sizeof ( env->UserName ) );

			value = env->opt ( APPENV_SETTING_USE_CUSTOMMEDIATOR_IP );
			if ( value && strlen ( value ) >= 7 )
				env->CustomMediatorIP = inet_addr ( value );
        }
        
        return true;
    }
    
    
    void DeallocNativePlatform ()
    {
        CVerbVerb ( "DeallocNativePlatform" );
        
        EnvironsSensors_GlobalsDispose();
    }
    
    
    namespace API
    {
        
        
        bool opt ( int hInst, const char * key, bool value )
        {
            CVerbVerbArg ( "opt [%i]: Saving [%s] [%d]", hInst, key ? key : "NULL", value );
            
            return opt ( hInst, key, value ? "1" : "0" );
        }
        
        
        bool optBool ( int hInst, const char * key )
        {
            //CVerbVerbArg ( "optBool [%i]: Loading [%s]", hInst, key ? key : "NULL" );
            
            const char * value = opt ( hInst, key );
            
            CVerbArg ( "optBool [%i]: key [%s] - value [%s]", hInst, key ? key : "NULL", value ? value : "NULL" );
            
            if ( value && value [0] == '0' ) {
                return false;
            }
            return true;
        }
        
#ifndef ANDROID
        bool opt ( JNIEnv * jenv, int hInst, const char * key, jstring value )
        {
            return opt ( hInst, key, value );
        }
#endif
    }
    
} /* namespace environs */

#endif

