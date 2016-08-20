/**
 * Environs global state and static variables.
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
#include "Interop.h"
#include "Interfaces/Interface.Exports.h"
#include "Device/Device.Controller.h"
#include "Core/Input.Handler.h"
#include "Portal/Portal.Device.h"
#include "Environs.Sensors.h"
#include "Environs.Utils.h"
#include "Environs.h"
#include "Tracer.h"

#if ( !defined(ANDROID) )
#   include "Device.List.h"
#   include "Portal.Instance.h"
#else
#	ifndef WINDOWS_PHONE
#		include <stdlib.h>
#	endif
#endif

#include <algorithm>

using namespace std;
using namespace environs;
using namespace environs::lib;


#if (defined(__APPLE__) && defined(USE_NSLOG))
#   undef CLogN
#   define CLogN(msg)
#endif

#define	CLASS_NAME	"Instance . . . . . . . ."


namespace environs
{
    EnvironsNative          native;

    Instance            **  instances = 0;

#ifdef _WIN32
#   pragma warning( push )
#   pragma warning( disable: 4592 )
#endif

#ifdef _WIN32
#   pragma warning( pop )
#endif

	extern pthread_mutex_t	devicesAccessMutex;

    const char          *	DefAppName              = ENVIRONS_DEFAULT_APP_NAME;
    const char          *	DefAreaName             = ENVIRONS_DEFAULT_AREA_NAME;
    const char          *   DefDeviceName           = ENVIRONS_DEFAULT_DEVICE_NAME;

	const char			*	DefGCMAPIKey			= "1234567890";

	const char			*	DefMediatorIP			= "137.250.171.19";
//    const char			*	DefMediatorIP			= "127.0.0.1";

    char *                  opt_pubCert				= 0;
    char *                  opt_privKey				= 0;

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
	HCRYPTPROV				g_hPrivKeyCSP			= NULL;
	HCRYPTKEY				g_hPrivKey				= NULL;
#endif
    pthread_cond_t          captureClassTimerSignal;

	char				*	GCMAPIKey				= (char *)DefGCMAPIKey;

	int						dataRecSize				= 64000;

	const char			*	opt_pluginStore			= "./mod/";
	const char			*	opt_dataStoreDefault	= "./data/";


#ifndef NDEBUG
#	ifdef DEBUGVERBVerb
	int						g_Debug					= 10;
#	else
	int						g_Debug					= 1;
#	endif
#else
    int						g_Debug					= 0;
#endif
	char				*	g_gcmRegID				= 0;

	extern void LogBuildCommit ();


    unsigned short          Instance::tcpPortLast   = 0;
    unsigned short          Instance::udpPortLast   = 0;


    Instance::Instance ()
    {
        CLog ( "-------------------------------------------------------" );
        CLog ( "Construct" );

		disposing				= false;
		appAreaID				= -1;
        hEnvirons               = 1;
        kernel                  = 0;
		nativeObj				= &native;

        environsState           = environs::Status::Uninitialized;

        streamBufferCount       = DATA_BUFFER_COUNT;
        streamFPS               = 30;
        streamBitrateKB         = 10000;

        deviceID                = 0;
        deviceType				= DEVICE_TYPE_UNKNOWN;
        *appName                = 0;
		*areaName				= 0;

        desktopDrawRequested	= false;
        desktopDrawLeft			= 0;
        desktopDrawTop			= 0;
        desktopDrawWidth		= 0;
        desktopDrawHeight		= 0;

        appWindowHandle			= 0;
        appWindowWidth			= 1024;
        appWindowHeight			= 768;

        basePort                = NATIVE_DEFAULT_BASE_PORT;
        tcpPort					= NATIVE_DEFAULT_DEVICE_PORT;
        udpPort					= NATIVE_DEFAULT_DEVICE_PORT;
        tuioPort				= 3333;

        this->AllocBuffer       = allocBuffer;
        this->RelocateBuffer    = relocateBuffer;
        this->AllocJByteBuffer  = allocJByteBuffer;
        this->DisposeBuffer     = disposeBuffer;

        this->cOutLog           = environs::COutLog;
        this->cOutArgLog        = environs::COutArgLog;

        portalAutoAccept        = true;

		/// Modules to use
		mod_PortalEncoder		= 0;
		mod_PortalDecoder		= 0;
		mod_PortalCapturer		= 0;
		mod_PortalRenderer		= 0;

        opt_useMediatorLoginDialog                  = true;
        opt_useMediatorLoginDialogDismissDisable    = true;

        useDefaultMediator      = true;
        *DefaultMediatorUserName = 0;
        *DefaultMediatorToken   = 0;

		Zero ( DefaultMediatorIP );
        memcpy ( DefaultMediatorIP, DefMediatorIP, strlen(DefMediatorIP) );

        DefaultMediatorPort     = 3389;

        /// Environs option that determines whether the application provided Mediator shall be used or not.
        /// Detailed description.
        useCustomMediator       = false;
        *CustomMediatorUserName = 0;
        *CustomMediatorToken    = 0;

        CustomMediatorIP		= 0;
        CustomMediatorPort		= 3389;

        allowConnectDefault         = true;
        usePlatformPortalGenerator	= false;

        useTcpPortal		= true;
        useStream			= true;
        useStreamUltrafast	= true;
        useOCL				= false;
        usePNG				= false;
        useNativeResolution	= true;
        usePortalAutoStart  = false;
        streamOverCom		= true;

        visualizeTouches	= false;
        simulateMouse		= false;
        useRecognizers		= true;
        useRenderCache		= true;
        usePortalViewDimsAuto = true;

        mediatorFilterLevel	= MEDIATOR_FILTER_AREA_AND_APP;

        useCLS				= true;
        useCLSForDevices	= true;
		useCLSForDevicesEnforce = true;
        useCLSForAllTraffic	= false;
        CLSPadding			= ENVIRONS_DEFAULT_CRYPT_PAD;

        useAnonymous        = true;
        useAuth             = true;

        *UserName           = 0;
        *UserPassword		= 0;

        sensorSubscribed    = 0;

		// We reuse the second byte to mark whether we already have notified the platform layer
		// about absence of a password ( == 1 if notification has been propagated)
		UserPassword [ 1 ]	= 0;

        CVerb ( "Construct: done" );
        CVerb ( "-------------------------------------------------------" );
    }


    Instance::~Instance ()
    {
        CLog ( "-------------------------------------------------------" );
        CLog ( "Destruct" );

		disposing		= true;

        if ( kernel ) {
            delete kernel;
            kernel = 0;
        }

        Dispose ();

		DisposePortalModules ();

		if ( hEnvirons > 0 && hEnvirons < ENVIRONS_MAX_ENVIRONS_INSTANCES )
			instances [ hEnvirons ] = 0;

        CLog ( "Destruct: done" );
        CLog ( "-------------------------------------------------------" );
    }


    /**
     * First call initializer of Environs. It is designed to be executed as the first function before using environs at all.
     *		This function initializes the minimal resource configuration such as locks, critical sections, mutex, etc...
     *
     */
    bool Instance::Init ()
    {
        CVerb ( "Init" );

        Zero ( appName  );
        memcpy ( appName, DefAppName, strlen ( DefAppName ) );

        Zero ( areaName );
        memcpy ( areaName, DefAreaName, strlen ( DefAreaName ) );

        if ( !callbacks.Init ( this ) ) {
            CErr ( "Init: Failed to init asyncWorker!" );
            return false;
        }

        if ( !notificationQueue.Init ( this ) ) {
            CErr ( "Init: Failed to init notification queue!" );
            return false;
        }

        if ( !asyncWorker.Init ( this ) ) {
            CErr ( "Init: Failed to init asyncWorker!" );
            return false;
        }

#ifdef _WIN32
        if ( !Kernel::StartWinSock () )
            return false;
#endif
		if ( !Core::EstablishMediator ( this ) )
			return false;

        return true;
    }


    /**
     * Release resources allocated by initNative
     *
     */
    void Instance::Dispose ( )
    {
        CVerb ( "Dispose" );

#ifdef DISPLAYDEVICE
		InputHandler::ClearDeviceMarkers ();
#endif
        DisposeInteropThread ();

        notificationQueue.Dispose ( );

        CVerb ( "Dispose: done" );
    }


	void Instance::DisposePortalModules ()
	{
		CVerb ( "DisposePortalModules" );

        free_m ( mod_PortalEncoder );

        free_m ( mod_PortalDecoder );

        free_m ( mod_PortalCapturer );

        free_m ( mod_PortalRenderer );

		CLog ( "DisposePortalModules: done" );
    }


    HLIB Instance::LocateLoadEnvModule ( COBSTR module, int deviceIDa, Instance * obj )
    {
        return ::LocateLoadEnvModule ( (const char *) module, deviceIDa, obj );
    }


    void * Instance::CreateInstance ( const char * module, int index, int environs_InterfaceType, int deviceIDa )
    {
        return API::CreateInstance ( module, index, environs_InterfaceType, deviceIDa, this );
    }

#ifndef DISPLAYDEVICE
	const char * Instance::opt ( const char * key )
	{
		if ( !key )
			return 0;
		return environs::API::opt ( hEnvirons, BuildOptKey ( appAreaID, key ) );
	}

	bool Instance::opt ( const char * key, int minSize, char * buffer, size_t size )
	{
		if ( !key || !buffer )
			return false;

		const char * value = environs::API::opt ( hEnvirons, BuildOptKey ( appAreaID, key ) );
		if ( value && ( int ) strlen ( value ) >= minSize )
			return ( snprintf ( buffer, size, "%s", value ) > 0 );
		return false;
	}

	bool Instance::opt ( const char * key, const char * value )
	{
		if ( !key || !value )
			return false;
		return environs::API::opt ( hEnvirons, BuildOptKey ( appAreaID, key ), value );
	}

	bool Instance::optBool ( const char * key )
	{
		if ( !key  )
			return false;
		return environs::API::optBool ( hEnvirons, BuildOptKey ( appAreaID, key ) );
	}

	bool Instance::optBool ( const char * key, bool value )
	{
		if ( !key  )
			return false;
		return environs::API::opt ( hEnvirons, BuildOptKey ( appAreaID, key ), value );
	}
#endif

    void CreateRandomUUIDTemplate ()
    {
        if ( !*native.deviceUID )
			snprintf ( native.deviceUID, sizeof(native.deviceUID), "Environs-Empty-UID-Template-%i", rand ( ) );
    }


#ifndef MEDIATORDAEMON
	/**
	* Atomically set Environs state secured by a dedicated lock
	*
	* @param	state	The state to be set.
	* @return	success
	*/
	bool SetEnvironsState ( Instance * env, Status_t state )
    {
        CVerbArg ( "SetEnvironsState: [ %i ]", state );

		if ( !env ) return false;

		/// Make sure that kernelMutex is initialized
		if ( !___sync_val_compare_and_swap ( &native.environsKernelAccess, 1, 1 ) )
			return false;

		bool success = false;

		if ( !LockAcquireA ( native.kernelLock, "SetEnvironsState" ) )
			return false;

		if ( env->environsState == state ) {
			success = true;
			goto Finish;
		}

		switch ( state )
		{
		case Status::Disposing:
			if ( env->environsState > Status::Disposing && env->environsState <= Status::Stopped )
				success = true;
			break;

		case Status::Initializing:
			if ( env->environsState == Status::Uninitialized )
				success = true;
			break;

		case Status::Initialized:
			if ( env->environsState >= Status::Initializing && env->environsState <= Status::Stopped )
				success = true;
			break;

		case Status::Stopped:
			if ( env->environsState == Status::StopInProgress || env->environsState == Status::Initialized )
				success = true;
			break;

		case Status::StopInProgress:
			if ( env->environsState > Status::StopInProgress )
				success = true;
                break;

        case Status::Stopping:
            if ( env->environsState > Status::Stopping )
				success = true;
            break;

		case Status::Starting:
			if ( env->environsState == Status::Stopped )
				success = true;
			break;

		case Status::Started:
			if ( env->environsState >= Status::Starting )
				success = true;
			break;

		case Status::Connected:
			if ( env->environsState >= Status::Started )
				success = true;
			break;

            default:
                break;
		}

        if ( success ) {
            CVerbArg ( "SetEnvironsState: [ %i ] ok", state );

			env->environsState = state;

            API::onEnvironsNotifier1 ( env, 0, 0-(int) env->environsState, SOURCE_NATIVE, NOTIFY_TYPE_STATUS );
		}

	Finish:
        LockReleaseVA ( native.kernelLock, "SetEnvironsState" );

		return success;
	}


#undef CLASS_NAME
#define	CLASS_NAME	"EnvironsNative . . . . ."


    EnvironsNative::EnvironsNative ()
    {
        environsKernelAccess	= 0;

        allocated               = false;
        isAppShutdown           = false;

        *deviceUID              = 0;

        coresStarted            = 0;
        envMappingLastID        = 1;
        workDir                 = 0;
        libDir                  = 0;
        dataStore               = (char *)opt_dataStoreDefault;

        platform				= Platforms::Unknown;
        sdks                    = 0;
        OSLevel                 = 0;

		stuntMaxTry				= ENVIRONS_STUNT_MAX_TRY;
		stunMaxTry				= ENVIRONS_STUN_MAX_TRY;

        useBtObserver           = true;
		useBtInterval			= 10000;
        useWifiObserver         = true;
		useWifiInterval			= ENVIRONS_WIFI_OBSERVER_INTERVAL_MIN * 2;

        useStdout               = true;
        useLogFile              = false;
		useHeadless				= false;
        useNotifyDebugMessage   = false;
		*deviceName             = 0;

        display.dpi             = 96.0;

        display.width           = ENVIRONS_DISPLAY_UNINITIALIZED_VALUE;
        display.width_mm        = ENVIRONS_DISPLAY_UNINITIALIZED_VALUE;
        display.height          = ENVIRONS_DISPLAY_UNINITIALIZED_VALUE;
        display.height_mm       = ENVIRONS_DISPLAY_UNINITIALIZED_VALUE;
        device_left				= 0;
        device_top				= 0;

        networkStatus			= NETWORK_CONNECTION_NO_NETWORK;
		networkConnectTimeout	= 8;

        udpSignalSender         = INVALID_FD;

        Zero ( transitionLock );
        Zero ( kernelLock );
        Zero ( appEnvLock );

#ifndef ENABLE_INSTANCE_WEAK_REFERENCE
		Zero ( instancesSPLock );
#endif
		Tick32					= GetEnvironsTickCount32;
		Tick					= GetEnvironsTickCount;

		cOutLog					= environs::COutLog;
		cOutArgLog				= environs::COutArgLog;

        if ( !AllocNative () ) {
            CErr ( "Contruct: Failed to allocate native layer!" );
        }
        else
            allocated = true;
    }


    EnvironsNative::~EnvironsNative ()
    {
        CLog ( "-------------------------------------------------------" );
        CLog ( "Destruct" );

#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD

        DeviceBase::StopComDat ();
#endif
        DeallocNative ();

        if ( IsValidFD ( udpSignalSender ) ) {
			CSocketTraceRemove ( udpSignalSender, "~EnvironsNative: Closing udpSignalSender", 0 );

            closesocket ( udpSignalSender );
            udpSignalSender = INVALID_FD;
        }

        envMapping.clear();

#ifndef ENABLE_INSTANCE_WEAK_REFERENCE
		LockDisposeA ( instancesSPLock );
#endif
        LockDisposeA ( kernelLock );
        LockDisposeA ( appEnvLock );

        LockDisposeA ( transitionLock );

        if ( libDir && libDir != workDir ) {
            free ( libDir );
        }
        libDir = 0;

		SetWorkDir ( 0 );

		SetDataStore ( (char *) opt_dataStoreDefault );

#ifdef _WIN32
		Kernel::DisposeWinSock ();
#endif

        free_m ( instances );

        DisposeTracerAll ();

        CLog ( "Destruct: Done" );
        CLog ( "-------------------------------------------------------" );
    }


	void EnvironsNative::SetDataStore ( char * path )
	{
		if ( dataStore && dataStore != opt_dataStoreDefault ) {
			free ( dataStore );
		}
		dataStore = path;
	}


	void EnvironsNative::SetWorkDir ( char * path )
	{
        bool isAlsoLib = false;

        if ( libDir && libDir == workDir ) {
            isAlsoLib = true;
        }
        free_n ( workDir );

		workDir = path;

		if ( isAlsoLib )
            libDir = workDir;
    }


    bool EnvironsNative::InitSignalSender ( )
    {
        bool success = true;

#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
        LockAcquireVA ( appEnvLock, "InitSignalSender" );
#else
        LockAcquireVA ( instancesSPLock, "InitSignalSender" );
#endif
        do
        {
            if ( IsValidFD ( udpSignalSender ) )
                break;

            int sock = ( int ) socket ( PF_INET, SOCK_DGRAM, 0 );
            if ( IsInvalidFD ( sock ) ) {
                success = false; break;
            }

            CSocketTraceAdd ( sock, "EnvironsNative InitSignalSender udpSignalSender" );

            udpSignalSender = sock;

            if ( !SetNonBlockSocket( udpSignalSender, true, "InitSignalSender" ) )
            {
                CSocketTraceRemove ( udpSignalSender, "InitSignalSender: Closing udpSignalSender", 0 );
                closesocket ( udpSignalSender );

                udpSignalSender = INVALID_FD;
                success = false; break;
            }
        }
        while ( false );

#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
        LockReleaseVA ( appEnvLock, "InitSignalSender" );
#else
        LockReleaseVA ( instancesSPLock, "InitSignalSender" );
#endif
        return success;
    }


#ifdef ENABLE_EXCEPTION_REDIRECT
#	include <signal.h>
	
	void EnvSignalAbnormalHandler(int signal)
	{
        _EnvDebugBreak ( "EnvSignalAbnormalHandler" );
	}
#endif


    /**
     * First call initializer of Environs. It is designed to be executed as the first function before using environs at all.
     *		This function initializes the minimal resource configuration such as locks, critical sections, mutex, etc...
     *
     */
    bool EnvironsNative::AllocNative ()
    {
        CVerbN ( "AllocNative" );

        if ( ___sync_val_compare_and_swap ( &environsKernelAccess, 0, 1 ) == 1 )
            return false;

#ifdef ENABLE_EXCEPTION_REDIRECT
		// Exception settings of visual studio, c++ Enable Exceptions from /EHsc to .. with SEH
		typedef void (*pEnvSignalAbnormalHandler)(int);

        //pEnvSignalAbnormalHandler pHandler =
        signal ( SIGSEGV , EnvSignalAbnormalHandler );
#endif

        instances = ( Instance ** ) calloc ( 1, sizeof ( Instance * ) * ENVIRONS_MAX_ENVIRONS_INSTANCES );
        if ( !instances )
            return false;

#ifndef ENABLE_INSTANCE_WEAK_REFERENCE
		if ( !LockInitA ( instancesSPLock ) )
			return false;
#endif

        if ( !LockInitA ( transitionLock ) )
            return false;

        if ( !LockInitA ( kernelLock ) )
            return false;

        if ( !LockInitA ( appEnvLock ) )
            return false;

        if ( !gcThread.Init ( ) )
            return false;

#ifdef NATIVE_WIFI_OBSERVER
        if ( !wifiObserver.Init () )
            return false;
#endif

#ifdef NATIVE_BT_OBSERVER
		if ( !btObserver.Init () )
			return false;
#endif
		InitTracer ();

		LogBuildCommit ();

#if ( defined(LINUX) && defined(NDEBUG) )
        useStdout   = false;
#endif

#if ( !defined(CLI_CPP) && !defined(ANDROID) )
        Zero ( lib::Environs::instancesAPI );
#endif
		Zero ( deviceName );
		memcpy ( deviceName, DefDeviceName, strlen ( DefDeviceName ) );

        Zero ( Kernel::touchRecognizerNames );
        Kernel::touchRecognizerNamesCount = 0;

        Zero ( deviceUID );

        if ( !InitDevicesMap () )
            return false;

        if ( !InitPortalDevices () )
            return false;

        Zero ( captureClassTimerSignal );
        if ( pthread_cond_manual_init ( &captureClassTimerSignal, 0 ) ) {
            CErr ( "AllocNative: Failed to init captureClassTimerSignal!" );
            return false;
        }

        if ( !InitEnvironsCrypt () ) {
            CErr ( "AllocNative: Failed to init crypt layer." );
            return false;
        }

        InitInteropThread ();

        Kernel::InitStatics ();

        DetermineAndInitWorkDir ();

        if ( !Mediator::InitClass() )
            return false;

        if ( !API::EnvironsSensors_GlobalsInit () )
            return false;

#if ( !defined(ANDROID) )
        if ( !environs::lib::Environs::ObjectAPIInit () )
            return false;
#endif
		environs::DetectSDKs ();

		DetectPlatform ();

        return true;
    }


    /**
     * Detect whether minimal native layer resources have been allocated.
     *
     */
    bool EnvironsNative::IsNativeAllocated ()
    {
        return allocated;
    }


    void EnvironsNative::DisposeInstance ( int hInst )
    {
        CVerb ( "DisposeInstance" );

		if ( hInst <= 0 ) {
			CErr ( "DisposeInstance: Invalid instance handle!" );
			return;
        }

#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
        sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
		sp ( Instance ) envSP;

		LockAcquireVA ( instancesSPLock, "DisposeInstance" );

		envSP = instancesSP [ hInst ];
		if ( envSP )
			instancesSP [ hInst ].reset ();

		LockReleaseVA ( instancesSPLock, "DisposeInstance" );
#endif
		if ( !envSP )
			return;

		Instance * env = envSP.get ();

		if ( env->kernel )
			env->kernel->Stop ();

		env->asyncWorker.Dispose ();
		env->notificationQueue.Dispose ();

#ifdef DISPLAYDEVICE
		env->callbacks.Clear ();

		SaveConfig ( );
#endif
		LockAcquireVA ( transitionLock, "DisposeInstance" );

#ifdef MEDIATOR_WP
        env->mediatorSP.reset ();
#else
		env->mediator.reset ();
#endif
		env->UserPassword [ 1 ]   = 0;

        instances [ hInst ]     = 0;

		env->myself.reset ();

		LockReleaseVA ( transitionLock, "DisposeInstance" );

		/// TODO Make them instance related!!!
		for ( unsigned int i = 0; i < ENVIRONS_TOUCH_RECOGNIZER_MAX; i++ )
			if ( Kernel::touchRecognizerNames [ i ] )
				free ( Kernel::touchRecognizerNames [ i ] );

		Zero ( Kernel::touchRecognizerNames );
		Kernel::touchRecognizerNamesCount = 0;
    }


    /**
     * Release resources allocated by initNative
     *
     */
    void EnvironsNative::DeallocNative ( )
    {
        CVerb ( "DeallocNative" );

        /// Prevent multiple calls to initializer
        if ( ___sync_val_compare_and_swap ( &environsKernelAccess, 1, 0 ) == 0 )
            return;

		for ( int i = 1; i < ENVIRONS_MAX_ENVIRONS_INSTANCES; ++i )
        {
			DisposeInstance ( i );
		}

#if (!defined(ANDROID) && !defined(ENVIRONS_IOS))
        environs::lib::Environs::ObjectAPIDispose ();
#endif
        DisposeInteropThread ();

        Kernel::ReleaseStatics ();

        CondDisposeA ( captureClassTimerSignal );

        DisposeDevicesMap ();

        Mediator::DisposeClass ();

        ReleasePortalDevices ();

        DeallocNativePlatform ();

        API::EnvironsSensors_GlobalsDispose ();

		SetWorkDir ( 0 );

		SetDataStore ( 0 );

        ReleaseEnvironsCrypt ();

        allocated = false;

        CVerb ( "DeallocNative: done" );

        DisposeLog ();

#ifdef USE_CRT_MLC
		CLogN ( "NotificationQueue:_CrtDumpMemoryLeaks." );
		_CrtDumpMemoryLeaks ();
#endif
    }

#ifndef ANDROID
    lib::Environs * EnvironsNative::GetInstanceAPI ( int hInst )
    {
        return environs::lib::Environs::instancesAPI [ hInst ];
    }
#endif


    int EnvironsNative::UpdateEnvID  ( const char * app, const char * area, int eid )
    {
        string sarea = area; string sapp ( app );

        transform ( sarea.begin(), sarea.end(), sarea.begin(), ::tolower );
        transform ( sapp.begin(), sapp.end(), sapp.begin(), ::tolower );

        string key = sarea + "_" + sapp;

        return UpdateEnvID ( key.c_str(), eid );
    }


    int EnvironsNative::UpdateEnvID  ( const char * key, int eid )
    {
		if ( !LockAcquireA ( appEnvLock, "UpdateEnvID" ) )
			return 0;


        map<string, int>::iterator iter = envMapping.find ( key );

        if ( iter != envMapping.end () ) {
            eid = iter->second;
        }
		else {
			if ( eid <= 0 ) {
				eid = envMappingLastID; envMappingLastID++;
			}

			envMapping [ key ] = eid;
		}

		if ( !LockReleaseA ( appEnvLock, "UpdateEnvID" ) )
			return 0;
        return eid;
    }


    int EnvironsNative::WaitOne ( void * thread, const char * func, int ms )
    {
        if ( !thread )
            return 0;
        return ( (EnvSignal *) thread)->WaitOne ( func, ms );
    }


    bool EnvironsNative::Notify ( void * thread, const char * func )
    {
        if ( !thread )
            return 0;
        return ( (EnvSignal *) thread)->Notify ( func );
    }

#ifndef ANDROID

    bool CreateInstancePlatform ( Instance * env )
    {
        return true;
    }

#endif


	namespace API
	{

		/// This call notifies about an event by the native layer
		INLINEFUNC void onEnvironsNotifier1 ( Instance * env, int notification )
		{
			if ( env ) env->notificationQueue.Push ( 0, notification );
        }

        /// This call notifies about an event by the native or a device (deviceID)
		INLINEFUNC void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification )
        {
            if ( env ) env->notificationQueue.Push ( objID, notification );
        }

        /// This call notifies about an event of a given source
		INLINEFUNC void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification, int source, int type )
        {
            if ( env ) env->notificationQueue.Push ( objID, notification, source, type );
        }

        /// This call notifies about an event of a given source
        /*INLINEFUNC void onEnvironsNotifierContext1 ( Instance * env, OBJIDType objID, int notification, int source, int context )
        {
            if ( env ) env->notificationQueue.PushContext ( objID, notification, source, context );
        }
        */

        /// This call notifies about an event of a given source
		INLINEFUNC void onEnvironsNotifierContext1 ( Instance * env, OBJIDType objID, int notification, int source, void * context, int contextSize )
        {
            if ( env ) env->notificationQueue.PushContext ( objID, notification, source, context, contextSize );
        }

        /// This call notifies about an event of a given source
		INLINEFUNC void onEnvironsNotifierContext1 ( Instance * env, int deviceID, const char * areaName, const char * appName, int notification, void * context, int contextSize )
        {
            if ( env ) env->notificationQueue.PushContext ( deviceID, areaName, appName, notification, context, contextSize );
        }

        /// This call notifies about an event of a given source
		INLINEFUNC void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification, int sourceIdent )
        {
            if ( env ) env->notificationQueue.Push ( objID, notification, sourceIdent, NOTIFY_TYPE_NOTIFY );
        }

		INLINEFUNC void onEnvironsDataNotifier1 ( Instance * env, OBJIDType objID, int nativeID, int type, int fileID, const char * descriptor, unsigned int descLength, unsigned int dataLength )
        {
            if ( env ) env->notificationQueue.PushData ( objID, nativeID, type, fileID, descriptor, descLength, dataLength );
        }

		INLINEFUNC void onEnvironsMsgNotifier1 ( Instance * env, OBJIDType objID, int sourceIdent, const char * msg )
        {
            if ( env ) env->notificationQueue.Push ( objID, sourceIdent, msg, (unsigned int)strlen ( msg ) );
        }

		INLINEFUNC void onEnvironsMsgNotifier1 ( Instance * env, OBJIDType objID, int sourceIdent, const char * msg, int length )
        {
            if ( env ) env->notificationQueue.Push ( objID, sourceIdent, msg, length );
        }

		INLINEFUNC int onEnvironsGetResponse ( Instance * env, int objID, unsigned short optionID, unsigned int resultCapacity, void * resultBuffer )
        {
            if ( env ) return env->notificationQueue.GetResponse ( objID, optionID, resultCapacity, resultBuffer ); return 0;
        }

		INLINEFUNC void onEnvironsAsyncSend1 ( Instance * env, OBJIDType objID, int sourceIdent, bool comDat, char msgType, unsigned short payloadTyp, const void * payload, unsigned int payloadSize )
        {
            if ( env ) env->notificationQueue.Push ( objID, sourceIdent, comDat, msgType, payloadTyp, payload, payloadSize );
        }


        /// This call notifies about an event of a given source
		INLINEFUNC void onEnvironsNotifier1 ( Instance * env, int deviceID, const char * areaName, const char * appName, int notification, int sourceIdent )
        {
            if ( env ) env->notificationQueue.Push ( deviceID, areaName, appName, notification, sourceIdent );
        }


		INLINEFUNC void onEnvironsMsgNotifier ( Instance * env, int deviceID, const char * areaName, const char * appName, int sourceIdent, const char * msg, int length, const char * prefix )
        {
            if ( env ) env->notificationQueue.Push ( deviceID, areaName, appName, sourceIdent, msg, length, prefix );
        }

		INLINEFUNC void onEnvironsHandleResponse ( Instance * env, unsigned int payloadSize, char * payload )
        {
            if ( env ) env->notificationQueue.HandleResponse ( payloadSize, payload );
		}


#ifdef _WIN32
#define	PLUGIN_PREFIX		""
#define	PLUGIN_EXTENSION	".dll"

#else
#include <dirent.h>
#include <stdio.h>

#define	PLUGIN_PREFIX		"lib"
#define	PLUGIN_EXTENSION	".so"
#endif

#define	PLUGIN_PATTERN		"./mod/" PLUGIN_PREFIX "EnvPlug*" PLUGIN_EXTENSION

		bool LoadPluginModules ( )
		{
			CVerb ( "LoadPluginModules" );

			return true;
		}


		void DisposeInstance ( void * instance )
		{
			CVerb ( "DisposeInstance" );

			if ( instance ) {
				IEnvironsBase * IModule = (IEnvironsBase *)instance;

				HMODULE hModLib = (HMODULE)IModule->hModLib;

				delete IModule;

				if ( hModLib ) {
					CVerb ( "DisposeInstance: Release module." );

					dlclose ( hModLib );
				}
			}
		}


		void * CreateInstance ( const char * module, int index, int environs_InterfaceType, int deviceID, Instance * env )
		{
			CVerbID ( "CreateInstance" );

			IEnvironsIdent *    IModule             = 0;
			pCreateInstance     Create              = 0;
			//pSetEnvironsObject  SetEnvironsObject   = 0;

			HMODULE hModLib = LocateLoadModule ( module, deviceID, env );
			if ( !hModLib ) {
				CErrArgID ( "CreateInstance: [ %s ] not available.", module );
				goto Failed;
			}

			/*
			// Already done in LocateLoadModule
			SetEnvironsObject = ( pSetEnvironsObject ) dlsym ( hModLib, MODULE_EXPORT_ENVIRONSOBJ );
			if ( SetEnvironsObject ) {
				SetEnvironsObject ( env, &native );
			}
#ifndef NDEBUG
			else {
				CWarnID ( "CreateInstance: Failed to load the Environs object initializer from module." );
			}
#endif
*/
			Create = ( pCreateInstance ) dlsym ( hModLib, MODULE_EXPORT_CREATE );
			if ( !Create ) {
				CErrID ( "CreateInstance: Failed to load the instance invoker from module." );
				goto Failed;
			}

			IModule = ( IEnvironsBase * ) Create ( index, deviceID );
			if ( !IModule ) {
				CErrID ( "CreateInstance: Unable to create an interface object." );
				goto Failed;
			}

			if ( environs_InterfaceType && IModule->interfaceType && environs_InterfaceType != IModule->interfaceType ) {
				CErrArgID ( "CreateInstance: Interface of object [ %i ] and requested interface [ %i ] mismatch.", environs_InterfaceType, IModule->interfaceType );
				goto Failed;
			}

			CVerbID ( "CreateInstance: Interface object created." );

			IModule->hModLib = hModLib;
			IModule->env	 = env;
			return IModule;

		Failed:
			if ( IModule ) {
				CVerbArg ( "CreateInstance: Disposing [ %s ]", IModule->name );
				IModule = 0;
			}

			if ( hModLib ) {
				CVerbArg ( "CreateInstance: Closing [ %s ]", module );
				hModLib = 0;
			}

			return 0;
		}
	}	/// -> namespace API

#endif
}	/// -> namespace environs
