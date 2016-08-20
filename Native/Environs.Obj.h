/**
 * Environs instance state, declarations and attributes
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
#ifndef INCLUDE_HCM_ENVIRONS_GLOBAL_OBJECTS_H
#define INCLUDE_HCM_ENVIRONS_GLOBAL_OBJECTS_H

#include "Environs.Native.h"
#include "Device.Display.Decl.h"

#include "Core/Mediator.Client.h"
#include "Core/Kernel.h"
#include "Core/Performance.Count.h"
#include "Core/Notifications.h"
#include "Core/Async.Worker.h"
#include "Core/Byte.Buffer.h"
#include "Core/Callbacks.h"
#include "Log.h"
#include "Wifi.Observer.h"
#include "Bt.Observer.h"

#include "Environs.Display.h"
#include "Environs.Mobile.h"
#include "Environs.Linux.h"


#define LIBNAME		"Environs.Native"

#define	NOTIFY_TYPE_NOTIFY		0
#define	NOTIFY_TYPE_STATUS		1
#define	NOTIFY_TYPE_MSG			2
#define	NOTIFY_TYPE_DATA		3
#define	NOTIFY_TYPE_SEND		4

#define	DEFAULT_NO_PORTAL_POSITION_LEFT		60
#define	DEFAULT_NO_PORTAL_POSITION_TOP		60
#define	DEFAULT_NO_PORTAL_POSITION_WIDTH	20
#define	DEFAULT_NO_PORTAL_POSITION_HEIGHT	20


#define MEDIATOR_SP
#define MEDIATOR_WP

#ifdef MEDIATOR_WP
#   define MED_WP  .lock()
#else
#   define MED_WP
#endif

typedef unsigned int ( CallConv * pEnvironsTickCount32 ) ( void );
typedef unsigned long long ( CallConv * pEnvironsTickCount ) ( void );

namespace environs
{
    namespace lib
    {
        class Environs;
    }


    class EnvironsNative
    {
    public:
        EnvironsNative ();
        ~EnvironsNative ();

        bool                        allocated;
        bool                        isAppShutdown;
        LONGSYNC                    environsKernelAccess;
        char                        deviceUID				[ MAX_NAMEPROPERTY * 6 ];
		char						deviceName				[ MAX_NAMEPROPERTY + 1 ];

        char                    *   workDir;
        char                    *   libDir;
        char                    *	dataStore;


        DeviceDisplay               display;

        int                         device_left;
        int                         device_top;

        Platforms_t                 platform;
        int                         sdks;
        int                         OSLevel;
		int							stuntMaxTry;
		int							stunMaxTry;

        ThreadSync                  gcThread;

		pthread_mutex_t             appEnvLock;
        pthread_mutex_t             kernelLock;
        pthread_mutex_t             transitionLock;

        LONGSYNC                    coresStarted;
        int                         envMappingLastID;
        std::map<std::string, int>  envMapping;

        // -1 means no network interfaces connected
        // 0 means connected to a network, but no internet access
        // 1 means internet access through mobile data
        // 2 means internet access through WiFi
        // 3 means internet access through LAN
        int                         networkStatus;
		int							networkConnectTimeout;

        int                         udpSignalSender;
        bool                        InitSignalSender ();

        bool                        useBtObserver;
		int							useBtInterval;

        bool                        useWifiObserver;
		int							useWifiInterval;

#ifdef NATIVE_WIFI_OBSERVER
        WifiObserver                wifiObserver;
#endif

#ifdef NATIVE_BT_OBSERVER
		BtObserver					btObserver;
#endif
        bool                        useStdout;
        bool                        useLogFile;
        bool                        useHeadless;
        bool                        useNotifyDebugMessage;

#ifdef __cplusplus
#   ifdef ENABLE_INSTANCE_WEAK_REFERENCE
        wp ( Instance )				instancesSP [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];
#   else
        sp ( Instance )				instancesSP [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];

        pthread_mutex_t				instancesSPLock;
#   endif
#endif
        bool                        IsNativeAllocated ();
		void						DisposeInstance ( int hInst );
		void						SetDataStore ( char * path );

		void						SetWorkDir ( char * path );

#ifndef ANDROID
        lib::Environs           *   GetInstanceAPI ( int hInst );
#endif
        int                         UpdateEnvID  ( const char * app, const char * area, int eid );
        int                         UpdateEnvID  ( const char * key, int eid );

        // Helper functions for EnvSignal
        virtual int                         WaitOne ( void * thread, const char * func, int ms );
        virtual bool                        Notify ( void * thread, const char * func );

		pEnvironsTickCount32		Tick32;
		pEnvironsTickCount			Tick;

		pCOutLog					cOutLog;
		pCOutArgLog					cOutArgLog;

    private:
        bool                        AllocNative ();
        void                        DeallocNative ();
    };

#ifdef ENVIRONS_CORE_LIB
    extern EnvironsNative   native;
#endif

    class Instance
    {
    public:
        Instance ();
        ~Instance ();

        bool                Init ();
        void                Dispose ();

		bool				disposing;

        NotificationQueue   notificationQueue;
		AsyncWorker         asyncWorker;
        ICallbacks          callbacks;

        Kernel			*	kernel;
		EnvironsNative	*	nativeObj;

        environs::Status_t  environsState;

        unsigned short          basePort;
        unsigned short          tcpPort;
        unsigned short          udpPort;
        unsigned short          tuioPort;
        static unsigned short   tcpPortLast;
        static unsigned short   udpPortLast;

        int                 streamBufferCount;
        int                 streamFPS;
        int                 streamBitrateKB;

        bool                desktopDrawRequested;
        int					desktopDrawLeft;
        int					desktopDrawTop;
        int					desktopDrawWidth;
        int					desktopDrawHeight;

        int					deviceID;
        char				deviceType;
		int					appAreaID;
        char				appName					[ MAX_NAMEPROPERTY + 1 ];
        char				areaName				[ MAX_NAMEPROPERTY + 1 ];

        WNDHANDLE			appWindowHandle;
        unsigned int		appWindowWidth;
        unsigned int		appWindowHeight;

        pCOutLog            cOutLog;
        pCOutArgLog         cOutArgLog;

        pallocBuffer        AllocBuffer;
        prelocateBuffer     RelocateBuffer;
        pallocJByteBuffer   AllocJByteBuffer;
        pdisposeBuffer      DisposeBuffer;

        bool                portalAutoAccept;

		/// Modules to use
		char			*	mod_PortalEncoder;
		char			*	mod_PortalDecoder;
		char			*	mod_PortalCapturer;
		char			*	mod_PortalRenderer;

        /** If true, then a login dialog requesting Mediator credentials will pop up. 	*/
        bool                opt_useMediatorLoginDialog;

        /** If true, then a cancel the Mediator login dialog will disable Mediator settings. 	*/
        bool                opt_useMediatorLoginDialogDismissDisable;

        virtual HLIB		LocateLoadEnvModule ( COBSTR module, int deviceID, Instance * obj );
        virtual void *      CreateInstance ( const char * module, int index, int environs_InterfaceType, int deviceID );

        void				DisposePortalModules ();

        int                 hEnvirons;

        bool				useDefaultMediator;
        char				DefaultMediatorUserName [ MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1 ];
        char                DefaultMediatorToken [ ENVIRONS_USER_PASSWORD_LENGTH + 2 ];
        char                DefaultMediatorIP [ 20 ];
        unsigned short      DefaultMediatorPort;

        /// Environs option that determines whether the Mediator details provided by user application shall be used.
        /// Detailed description.
        bool				useCustomMediator;
        char				CustomMediatorUserName [ MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1 ];
        char                CustomMediatorToken [ ENVIRONS_USER_PASSWORD_LENGTH + 2 ];

        unsigned int		CustomMediatorIP;
        unsigned short		CustomMediatorPort;

        bool				allowConnectDefault;

        bool				usePlatformPortalGenerator;
        bool				useTcpPortal;
        bool				useStream;
        bool				useStreamUltrafast;
        bool				useOCL;
        bool				usePNG;
        bool				useNativeResolution;
        bool                usePortalAutoStart;
        bool				streamOverCom;
        bool				visualizeTouches;
        bool				simulateMouse;
        bool				useRecognizers;
        bool				useRenderCache;
        bool				usePortalViewDimsAuto;

        int					mediatorFilterLevel;

        bool                useCLS;
        bool                useCLSForDevices;
        bool                useCLSForDevicesEnforce;
        bool                useCLSForAllTraffic;
        unsigned int        CLSPadding;

        bool                useAnonymous;
        bool                useAuth;
        char				UserName [MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1];
        char				UserPassword [ENVIRONS_USER_PASSWORD_LENGTH + 2];

        unsigned int        sensorSubscribed;

#ifndef DISPLAYDEVICE
        const char *		opt ( const char * key );
        bool				opt ( const char * key, int minSize, char * buffer, size_t size );
        bool				opt ( const char * key, const char * value );

        bool				optBool ( const char * key );
        bool				optBool ( const char * key, bool value );
#endif

#ifdef __cplusplus
#   ifdef MEDIATOR_WP
        wp ( MediatorClient )	mediator;
        sp ( MediatorClient )	mediatorSP;
#   else
        sp ( MediatorClient )	mediator;
#   endif

		sp ( Instance )		myself;
#endif

    };


    extern Instance     **  instances;

	extern pthread_cond_t   captureClassTimerSignal;

    extern const char   *	DefAppName;
    extern const char   *	DefAreaName;
    extern const char   *   DefDeviceName;


	extern char			*	GCMAPIKey;
    extern const char   *	DefGCMAPIKey;

    extern char			*	opt_pubCert;
    extern char			*	opt_privKey;

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
	extern HCRYPTPROV		g_hPrivKeyCSP;
	extern HCRYPTKEY		g_hPrivKey;
#endif
	extern int				dataRecSize;

	extern const char	*	opt_dataStoreDefault;
	extern const char	*	opt_pluginStore;

	extern int				g_Debug;
	extern char			*	g_gcmRegID;

#ifdef __cplusplus
    bool                    SetEnvironsState ( Instance * env, Status_t state );

    bool                    CreateInstancePlatform ( Instance * env );
#endif

	const char *			BuildOptKey ( int eid, const char * key );

    void                    CreateRandomUUIDTemplate ();


#ifndef MEDIATORDAEMON
	extern char * BuildDataStorePath ( const char * fileName );
    extern bool DirectoryPathExist ( char * dir );

	namespace API {
		extern void onEnvironsNotifier1 ( Instance * env, int notification );

        extern void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification );
        extern void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification, int sourceIdent );
		extern void onEnvironsNotifier1 ( Instance * env, OBJIDType objID, int notification, int sourceIdent, int type );

		//extern void onEnvironsNotifierContext ( Instance * env, OBJIDType objID, int notification, int sourceIdent, int context );
		extern void onEnvironsNotifierContext1 ( Instance * env, OBJIDType objID, int notification, int sourceIdent, void * context, int contextSize );
        extern void onEnvironsNotifierContext1 ( Instance * env, int deviceID, const char * areaName, const char * appName, int notification, void * context, int contextSize );

        extern void onEnvironsDataNotifier1 ( Instance * env, OBJIDType objID, int nativeID, int type, int fileID, const char * descriptor, unsigned int descLength, unsigned int dataLength );

        extern int onEnvironsGetResponse ( Instance * env, int nativeID, unsigned short optionID, unsigned int resultCapacity, void * resultBuffer );

        extern void onEnvironsAsyncSend1 ( Instance * env, OBJIDType objID, int sourceIdent, bool comDat, char msgType, unsigned short payloadTyp, const void * payload, unsigned int payloadSize );

        extern void onEnvironsMsgNotifier1 ( Instance * env, OBJIDType objID, int sourceType, const char * msg );
        extern void onEnvironsMsgNotifier1 ( Instance * env, OBJIDType objID, int sourceType, const char * msg, int length );


        extern void onEnvironsNotifier1 ( Instance * env, int deviceID, const char * areaName, const char * appName, int notification, int sourceIdent );
        extern void onEnvironsMsgNotifier ( Instance * env, int deviceID, const char * areaName, const char * appName, int sourceIdent, const char * msg, int length, const char * prefix );


        extern void onEnvironsHandleResponse ( Instance * env, unsigned int payloadSize, char * payload );

#ifdef __cplusplus
        extern void * CreateInstance ( const char * module, int index, int environs_InterfaceType, int deviceID, Instance * env );
#endif

		extern void DisposeInstance ( void * instance );

    }	/// -> namespace API
#endif

}	/// -> namespace environs

#endif	/// end-INCLUDE_HCM_ENVIRONS_GLOBAL_STATE_H
