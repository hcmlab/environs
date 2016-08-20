/**
 * Environs Native Layer API exposed by the libraries
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
#define ENVIRONS_NATIVE_MODULE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifndef CLI_CPP

// Import access to the environs native object
#include "Environs.Obj.h"
using namespace environs;

// Import declarations and exports for the API
#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Crypt.h"
#include "Environs.Mobile.h"
#include "Environs.Modules.h"
#include "Environs.Release.h"
#include "Environs.Sensors.h"

#include "Tracer.h"
#include "Core/Async.Worker.h"
#include "Core/Byte.Buffer.h"
#include "Core/Callbacks.h"
#include "Core/Notifications.h"

#include "Device/Device.Controller.h"
#include "Portal/Portal.Device.h"

#ifdef DISPLAYDEVICE
#include "Renderer/Render.OpenCL.h"
#endif

#include "Environs.Build.Lnk.h"


#define CLASS_NAME	"API. . . . . . . . . . ."



namespace environs 
{
	extern void		RegisterMainThreadUtil ( );
	extern bool		InitStorageUtil ( const char * storagePath );
	

	namespace API
	{
		extern void	Environs_LoginDialog ( int hInst, CString_ptr userName );


		ENVIRONSAPI void EnvironsFunc ( SetDebugN, int mode )
		{
			g_Debug = mode;
        }
        
        
        ENVIRONSAPI int EnvironsProc ( GetDebugN )
        {
            return g_Debug;
        }


		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		ENVIRONSAPI EBOOL EnvironsProc ( GetIsReleaseBuildN )
		{
			return
#ifdef NDEBUG
				true;
#else
				false;
#endif
		}


		ENVIRONSAPI const char * EnvironsFunc ( ResolveNameN, int notification )
		{
			return resolveName ( notification );
		}


		ENVIRONSAPI void EnvironsFunc ( SetOSLevelN, int level )
		{
			native.OSLevel = level;
		}

        
        /**
         * Reset crypt layer and all created resources. Those will be recreated if necessary.
         * This method is intended to be called directly after creation of an Environs instance.
         *
         */
		ENVIRONSAPI void EnvironsProc ( ResetCryptLayerN )
		{
			CLog ( "ResetCryptLayerN" );
            
            if ( environs::opt_pubCert ) free ( environs::opt_pubCert );
			opt_pubCert = 0;

			if ( opt_privKey ) free ( opt_privKey );
			opt_privKey = 0;
            
            unlink ( BuildDataStorePath ( ENVIRONS_PRIVATE_KEYNAME ) );
            unlink ( BuildDataStorePath ( ENVIRONS_PUBLIC_CERTNAME ) );  
		}


		ENVIRONSAPI void EnvironsFunc ( RegisterMainThreadN, int hInst )
		{
			CVerb ( "RegisterMainThreadN" );

            InitSensorMainThreaded ();

			RegisterMainThreadUtil ();
		}


		/*
		* Method:    GetVersionString
		* Signature: ()Ljava/lang/String;
		*/
		ENVIRONSAPI jstring EnvironsProc ( GetVersionStringN )
		{
#ifdef ANDROID
			return jenv->NewStringUTF ( ENVIRONS_VERSION_STRING );
#else
			return ENVIRONS_VERSION_STRING;
#endif
		}

		ENVIRONSAPI jint EnvironsProc ( GetVersionMajorN ) {
			return BUILD_MAJOR_VERSION;
		}

		ENVIRONSAPI jint EnvironsProc ( GetVersionMinorN ) {
			return BUILD_MINOR_VERSION;
		}

		ENVIRONSAPI jint EnvironsProc ( GetVersionRevisionN ) {
			return BUILD_REVISION;
		}

		ENVIRONSAPI jint EnvironsProc ( GetPlatformN ) {
			return native.platform;
        }

		ENVIRONSAPI void EnvironsFunc ( SetPlatformN, jint platform ) {
			native.platform = (Platforms_t) platform;
        }
        
        ENVIRONSAPI void EnvironsFunc ( SetIsLocationNodeN, jboolean isLocationNode )
        {
            int platform = native.platform;
            
            if ( isLocationNode )
                platform |= Platforms::LocationNode_Flag;
            else
                platform &= ~Platforms::LocationNode_Flag;
            
            native.platform = (Platforms_t) platform;
        }
        
        
        ENVIRONSAPI EBOOL EnvironsFunc ( GetDisposingN, jint hInst  )
        {
            return ( !instances [ hInst ] || instances[hInst]->disposing != 0);
        }

		ENVIRONSAPI void EnvironsFunc ( SetDisposingN, jint hInst, EBOOL enable )
		{
			instances [ hInst ]->disposing = (enable == 1);
		}

		
		/**
		* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
		* Increasing this value may help to handle worse networks which suffer from large latencies.
		*
		* @param   timeout
		*/
		ENVIRONSAPI void EnvironsFunc ( SetNetworkConnectTimeoutN, int value )
		{
			if ( value < 0 )
				return;
			native.networkConnectTimeout = value;
		}

		ENVIRONSAPI void EnvironsFunc ( LogN, jstring msg, jint length )
		{
#	ifdef ANDROID
            INIT_PCHAR ( szMsg, msg );
            if ( szMsg == NULL ) {
                return;
            }
            
            environs::COutLog ( 0, szMsg, length, true );
            
            RELEASE_PCHAR ( szMsg, msg );
#	else
			environs::COutLog ( msg, length, true );
#	endif
		}
        
        
        EBOOL ApplyStringToSettingsNM ( int hInst, const char * value, char * target, unsigned int maxSize )
        {
            bool success = false;
            
            size_t len = strlen ( value );
            
            if ( len > 0 && len < maxSize )
            {
                strlcpy ( target, value, maxSize );
                
                CVerbArg ( "ApplyStringToSettingsNM: [%s]", target );
                
                if ( hInst > 0 && hInst < ENVIRONS_MAX_ENVIRONS_INSTANCES )
                {
                    Instance * env = instances [ hInst ];
                    
                    sp ( MediatorClient ) med = env->mediator MED_WP;
                    
                    if ( med && env->kernel && env->environsState >= environs::Status::Stopped )
                        med->BuildBroadcastMessage ();
                }
                success = true;
            }
            
            return success;
        }


        EBOOL EnvironsFunc ( ApplyStringToSettingsN, jint hInst, jstring value, char * target, unsigned int maxSize )
        {
            if ( !target || !value ) {
                CVerb ( "ApplyStringToSettingsN: Failed! Called with NULL argument!" );
                return false;
            }
            
            INIT_PCHAR ( szValue, value );
            
            if ( szValue == NULL ) {
                CErr ( "ApplyStringToSettingsN: Failed to allocate string or called with NULL argument!" );
                return false;
            }

            bool success = (ApplyStringToSettingsNM ( hInst, szValue, target, maxSize ) != 0);
            
            RELEASE_PCHAR ( szValue, value );
            
            return success;
        }
        
        
        /**
         * Create a native Environs object and return a handle to the object.
         * A return value of 0 means Error
         *
         * @return   An instance handle that refers to the created Environs object
         */
        ENVIRONSAPI jint EnvironsProc ( CreateEnvironsN )
        {
            CVerbN ( "CreateEnvironsN" );
            
            int hEnvirons	= 0;
            int hInst		= 1;

			sp ( Instance ) envSP;

			if ( !LockAcquireA ( native.transitionLock, "CreateEnvironsN" ) )
				return 0;
            
            for ( ; hInst < ENVIRONS_MAX_ENVIRONS_INSTANCES; ++hInst )
            {
				if ( instances [ hInst ] )
                    continue;

				envSP = std::make_shared < Instance > ();
                if ( !envSP )
                    break;
                
#ifndef ENABLE_INSTANCE_WEAK_REFERENCE
				LockAcquireVA ( native.instancesSPLock, "CreateEnvironsN" );
#endif
				native.instancesSP [ hInst ]   = envSP;
                
                Instance * inst         = envSP.get ();
                instances [ hInst ]     = inst;
                
                inst->hEnvirons         = hInst;

                inst->myself = envSP;
                
#ifndef ENABLE_INSTANCE_WEAK_REFERENCE
				LockReleaseVA ( native.instancesSPLock, "CreateEnvironsN" );
#endif
				if ( !inst->Init () )
					break;

                if ( !CreateInstancePlatform ( inst ) )
                    break;

                hEnvirons = hInst;
                break;
            }
            
			if ( !hEnvirons && hInst < ENVIRONS_MAX_ENVIRONS_INSTANCES )
            {
				instances [ hInst ] = 0;

                if ( envSP ) {
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
                    envSP->myself.reset ();
#else
					LockAcquireVA ( native.instancesSPLock, "CreateEnvironsN" );

					native.instancesSP [ hInst ].reset ();

					envSP->myself.reset ();

					LockReleaseVA ( native.instancesSPLock, "CreateEnvironsN" );
#endif
				}
			}
            
			LockReleaseVA ( native.transitionLock, "CreateEnvironsN" );

            return hEnvirons;
        }
        
        
        /**
         * Load settings for the given application environment from settings storage,
         * if any have been saved previously.
         *
         * @param	hInst		The handle to a particular native Environs instance.
         * @param 	appName		The application name for the application environment.
         * @param  	areaName	The area name for the application environment.
         *
         * @return   success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( LoadSettingsN, jint hInst, jstring app, jstring area )
        {
            CVerb ( "LoadSettingsN" );
            
            if ( hInst <= 0 || hInst >= ENVIRONS_MAX_ENVIRONS_INSTANCES ) {
                CErr ( "LoadSettingsN: Invalid arguments!" );
                return 0;
            }
            
            Instance * inst = instances[hInst];
            
            if ( app && !EnvironsCallArg ( ApplyStringToSettingsN, hInst, app, inst->appName, sizeof(inst->appName) ) )
                return false;
            
            if ( area && !EnvironsCallArg ( ApplyStringToSettingsN, hInst, area, inst->areaName, sizeof ( inst->areaName ) ) )
                return false;
            
            /// Load configuration
            return LoadConfig ( inst );
        }
        

		ENVIRONSAPI void EnvironsProc ( ClearStorageN )
        {
            CVerb ( "ClearStorageN" );
            
            ClearStorage ( native.dataStore );
        }
        
        
		sp ( Instance ) GetStartedInstanceSP ( int hInst )
        {
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "GetStartedInstanceSP" );

			sp ( Instance ) envSP = native.instancesSP [ hInst ];
			
			LockReleaseVA ( native.instancesSPLock, "GetStartedInstanceSP" );
#endif
			if ( !envSP )
				return 0;

			if ( envSP->environsState < environs::Status::Started ) {
				CVerb ( "GetStartedInstanceSP: Environs instance is not started." );
				return 0;
			}

			return envSP;
		}


		sp ( Instance ) GetInstanceSP ( int hInst )
        {
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            return native.instancesSP [ hInst ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "GetStartedInstanceSP" );

			sp ( Instance ) envSP = native.instancesSP [ hInst ];

            LockReleaseVA ( native.instancesSPLock, "GetStartedInstanceSP" );
            
            return envSP;
#endif
		}


		Instance * GetStartedInstance ( int hInst )
		{
			Instance * env = instances [ hInst ];
			if ( !env )
				return 0;

			if ( env->environsState < environs::Status::Started ) {
				CVerb ( "GetStartedInstance: Environs instance is not started." );
				return 0;
			}

			return env;
		}
		
		
        void SetDeviceID ( int hInst, int deviceID )
        {
            CVerbArg ( "SetDeviceID: %u", deviceID );
            
            Instance * env = instances[hInst];
            if ( !env )
                return;
            
            if ( !deviceID ) {
                Core::GenerateRandomDeviceID ( env );
                return;
            }

            env->deviceID = deviceID;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( mediator ) {
				mediator->BroadcastGenerateToken ();

				mediator->BuildBroadcastMessage ();
				return;
			}
            
#ifdef DISPLAYDEVICE
            SaveConfig ( );
#else
            char buffer [16];
            sprintf ( buffer, "%i", deviceID );

			env->opt ( APPENV_SETTING_DEVICE_ID, buffer );
#endif
        }
        
		ENVIRONSAPI void SetDeviceID2 ( int hInst, int deviceID )
		{
			CVerbArg ( "SetDeviceID2: %u", deviceID );
            
            SetDeviceID ( hInst, deviceID );
		}
        
        
        ENVIRONSAPI jint EnvironsFunc ( GetAppAreaIDN, jint hInst )
        {
			return instances [ hInst ]->appAreaID;
        }

		ENVIRONSAPI void EnvironsFunc ( SetDeviceIDN, jint hInst, jint deviceID )
		{
			SetDeviceID ( hInst, deviceID );
		}


		ENVIRONSAPI jint EnvironsFunc ( GetDeviceIDN, jint hInst )
		{
			return instances [ hInst ]->deviceID;
		}
        
        
		ENVIRONSAPI jint EnvironsFunc ( GetDeviceIDFromMediatorN, jint hInst )
		{
			Instance * env = instances [ hInst ];
            
            if ( env->deviceID ) {
                CWarn ( "GetDeviceIDFromMediatorN: A device ID has already been set before." );
                return env->deviceID;
            }
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( !mediator ) {
                if ( !MediatorClient::InitInstance ( ) )
                    return 0;
                
                mediator = env->mediator MED_WP;
                if ( !mediator )
                    return 0;
            }
            
            if ( env->useDefaultMediator ) {
                if ( !mediator->AddMediator ( inet_addr ( env->DefaultMediatorIP ), env->DefaultMediatorPort ) ) {
                    CWarn ( "GetDeviceIDFromMediatorN: Failed to add default Mediator!" );
                }
            }
            
            if ( env->useCustomMediator ) {
                if ( !mediator->AddMediator ( env->CustomMediatorIP, env->CustomMediatorPort ) ) {
                    CWarn ( "GetDeviceIDFromMediatorN: Failed to add custom Mediator!" );
                }
            }
            
			mediator->RegisterAtMediators ( false );
            
            int tries = 8;
            do
            {
                Sleep ( 600 );
                
                if ( env->deviceID )
                    break;
                tries--;
            }
            while ( tries > 0 );
            
			return env->deviceID;
		}


		ENVIRONSAPI jint EnvironsProc ( GetIPAddressN )
		{
            return Mediator::GetLocalIP ( );
		}


		ENVIRONSAPI jint EnvironsProc ( GetSubnetMaskN )
		{
            return Mediator::GetLocalSN ( );
		}


		ENVIRONSAPI jint EnvironsFunc ( GetMediatorFilterLevelN, jint hInst )
		{
			return instances[hInst]->mediatorFilterLevel;
		}


		ENVIRONSAPI void EnvironsFunc ( SetMediatorFilterLevelN, jint hInst, jint level )
		{
			instances[hInst]->mediatorFilterLevel = level;
		}

        
        ENVIRONSAPI EBOOL EnvironsFunc ( GetDirectContactStatusN, jint hInst, jint nativeID )
        {
            CVerbIDN ( "GetDirectContactStatusN" );
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            
            if ( !device )
                return false;
            
            bool success = device->GetDirectContactStatus ( );
            
            UnlockDevice ( device );
            return success;
        }

        
        /**
         * Enable or disable device list update notifications from Mediator layer.
         * In particular, mobile devices should disable notifications if the devicelist is not
         * visible to users or the app transitioned to background.
         * This helps recuding cpu load and network traffic when not required.
         *
         * @param enable      true = enable, false = disable
         */
        ENVIRONSAPI void EnvironsFunc ( SetMediatorNotificationSubscriptionN, jint hInst, jint enable )
        {
            sp ( MediatorClient ) mediator = instances[hInst]->mediator MED_WP;
            if ( mediator )
                mediator->SetNotificationSubscription ( enable == 1 );
        }
        
        
        /**
         * Get subscription status of device list update notifications from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        ENVIRONSAPI jint EnvironsFunc ( GetMediatorNotificationSubscriptionN, jint hInst )
        {
            sp ( MediatorClient ) mediator = instances[hInst]->mediator MED_WP;
            if ( mediator )
                return (mediator->GetNotificationSubscription () ? 1 : 0);
            return false;
        }
        
        
        
        /**
         * Enable or disable short messages from Mediator layer.
         * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
         * This helps recuding cpu load and network traffic when not necessary.
         *
         * @param enable      true = enable, false = disable
         */
        ENVIRONSAPI void EnvironsFunc ( SetMessagesSubscriptionN, jint hInst, jint enable )
        {
            sp ( MediatorClient ) mediator = instances[hInst]->mediator MED_WP;
            if ( mediator )
                mediator->SetMessagesSubscription ( enable == 1 );
        }
        
        
        /**
         * Get subscription status of short messages from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        ENVIRONSAPI jint EnvironsFunc ( GetMessagesSubscriptionN, jint hInst )
        {
            sp ( MediatorClient ) mediator = instances[hInst]->mediator MED_WP;
            if ( mediator )
                return (mediator->GetMessagesSubscription () ? 1 : 0);
            return false;
        }
        
        
        /**
         * Get the status, whether the device (id) has established an active portal
         *
         * @param 	nativeID    The device id of the target device.
         * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
         * @return	success 	true = yes, false = no
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalEnabledN, jint hInst, jint nativeID, int portalType )
        {
            CVerbIDN ( "GetPortalEnabledN" );
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            
            if ( !device )
                return false;
            
            bool success = device->GetPortalEnabled ( portalType );
            
            UnlockDevice ( device );
            return success;
        }
        
        
        /**
         * Get the portalID of the first active portal
         *
         * @param 	nativeID    The device id of the target device.
         * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
         * @return	portalID 	The portal ID.
         */
        ENVIRONSAPI int EnvironsFunc ( GetPortalIDN, jint hInst, jint nativeID, int portalType )
        {
            CVerbIDN ( "GetPortalIDN" );
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            
            if ( !device ) {
                CErrIDN ( "GetPortalIDN: Failed to lookup device." );
                return -1;
            }
            
            int success = device->GetPortalID ( portalType );
            
            UnlockDevice ( device );
            return success;
        }

        
        /**
         * Set the ports that the local instance of Environs shall use for listening on connections.
         *
         * @param	tcpPort The tcp port.
         * @param	udpPort The udp port.
         * @return success
         */
		ENVIRONSAPI EBOOL EnvironsFunc ( SetPortsN, jint hInst, int tcpPort, int udpPort )
		{
            CVerbArg ( "SetPortsN: tcp [ %d ] udp [ %d ]", tcpPort, udpPort );
            
            Instance * env = instances [ hInst ];

			if ( tcpPort < 0 || tcpPort > 65535 )
				return false;
			env->tcpPort = (unsigned short) tcpPort;

			if ( udpPort < 0 || udpPort > 65535 )
				return false;
			env->udpPort = (unsigned short) udpPort;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            
			if ( mediator && env->environsState >= environs::Status::Stopped )
				mediator->BuildBroadcastMessage ( false );

			return true;
        }


        /**
         * Set the base port that the local instance of Environs shall use for communication with other instances.
         * This option enables spanning of multiple multi surface environsments separated by the network stacks.
         *
         * @param	port The base port.
         * @return success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SetBasePortN, jint hInst, int port )
        {
            CVerbArg ( "SetBasePort: port [ %d ]", port );

            Instance * env = instances [ hInst ];

            if ( port < 0 || port > 65535 )
                return false;
            env->basePort = (unsigned short) port;

            sp ( MediatorClient ) mediator = env->mediator MED_WP;

            if ( mediator && env->environsState >= environs::Status::Stopped ) {
                mediator->BuildBroadcastMessage ( false );
                mediator->Stop ( true );
                mediator->Start ();
            }
            
            return true;
        }
        
        
        /**
         * Update device flags to native layer and populate them to the environment.
		*
		* @param	hInst    The handle to the environs instance.
        * @param	objID    The identifier for the native device object.
        * @param	flags    The internal flags to set or clear. (of type DeviceFlagsInternal::Observer*)
        * @param	set    	 true = set, false = clear.
		*/
		ENVIRONSAPI void EnvironsFunc ( SetDeviceFlagsN, jint hInst, jint async, jint objID, jint flags, jboolean set )
		{
			CVerbArg ( "SetDeviceFlagsN: objID [ %i ]\tFlags [ 0x%X ]\tSet [ %d ]", objID, flags, set );
            
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "SetDeviceFlagsN" );

			sp ( Instance ) envSP = native.instancesSP [ hInst ] ;

			LockReleaseVA ( native.instancesSPLock, "SetDeviceFlagsN" );
#endif
			if ( !envSP )
				return;
            
            if ( async )
                envSP->asyncWorker.Push ( objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC_ASYNC, flags, set, 0 );
            else
                envSP->asyncWorker.SyncDeviceFlagsAsync ( objID, flags, set );
            
		}


		ENVIRONSAPI EBOOL EnvironsFunc ( RegisterAtMediatorsN, jint hInst )
		{
			CVerb ( "RegisterAtMediatorsN" );

			Instance * env = instances [ hInst ];

            if ( env ) {
                sp ( MediatorClient ) mediator = env->mediator MED_WP;
                
				if ( mediator && env->environsState >= environs::Status::Starting )
					return mediator->RegisterAtMediators ( false );
			}
			return false;
		}


		ENVIRONSAPI EBOOL EnvironsFunc ( SetAreaNameN, jint hInst, jstring name )
		{
			CVerb ( "SetAreaNameN" );

			return EnvironsCallArg ( ApplyStringToSettingsN, hInst, name, instances[hInst]->areaName, sizeof ( instances[hInst]->areaName ) );
		}
        
        
		/*
         * Method:    GetAreaName
         * Signature: ()Ljava/lang/String;
         */
		ENVIRONSAPI jstring EnvironsFunc ( GetAreaNameN, jint hInst )
		{
#ifdef DISPLAYDEVICE
			return instances[hInst]->areaName;
#else
			jstring jvalue = 0;
            
#ifdef ANDROID
			jvalue = jenv->NewStringUTF ( instances[hInst]->areaName );
#else
			jvalue = instances[hInst]->areaName;
#endif
            
			return jvalue;
#endif
		}


		ENVIRONSAPI EBOOL EnvironsFunc ( SetApplicationNameN, jint hInst, jstring name )
		{
			CVerb ( "SetApplicationNameN" );

			return EnvironsCallArg ( ApplyStringToSettingsN, hInst, name, instances[hInst]->appName, sizeof(instances[hInst]->appName) );
		}
        
        
		/*
         * Method:    getApplicationName
         * Signature: ()Ljava/lang/String;
         */
		ENVIRONSAPI jstring EnvironsFunc ( GetApplicationNameN, jint hInst )
		{
#ifdef DISPLAYDEVICE
			return instances[hInst]->appName;
#else
            return JSTRINGNEW ( instances[hInst]->appName );
#endif
		}

		/**
		* Query whether the name of the current device has been set before.
		*
		* @return	has DeviceUID
		*/
		ENVIRONSAPI EBOOL EnvironsProc ( HasDeviceUIDN )
		{
			CVerb ( "HasDeviceUIDN" );

			return (*native.deviceUID != 0);
        }
        
		/**
		* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
		*
		* @param 	deviceUID A unique identifier to identify this device.
		* @return	success
		*/
        ENVIRONSAPI EBOOL EnvironsFunc ( SetDeviceUIDN, jstring name )
        {
            CVerb ( "SetDeviceUIDN" );
            
            if ( !name ) {
                CVerb ( "SetDeviceUIDN: Failed! Called with NULL argument!" );
                return false;
            }
            
            INIT_PCHAR ( szName, name );
            
            if ( szName == NULL ) {
                CErr ( "SetDeviceUIDN: Failed to allocate string or called with NULL argument!" );
                return false;
            }
            
#ifndef DISPLAYDEVICE
            opt ( 0, APPENV_SETTING_DEVICE_UID, szName );
#endif
            bool success = (ApplyStringToSettingsNM ( 1, szName, native.deviceUID, sizeof(native.deviceUID) ) != 0);
            
            RELEASE_PCHAR ( szName, name );
            
            return success;
        }


		/**
		* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
		*
		* @param 	deviceName  The device name.
		* @return	success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetDeviceNameN, jstring name )
		{
            CVerb ( "SetDeviceNameN" );
            
			return EnvironsCallArg ( ApplyStringToSettingsN, 0, name, native.deviceName, sizeof( native.deviceName) );
		}


		EBOOL EnvironsFunc ( ApplyModuleToSettingsN, jint hInst, jstring moduleName, char ** target, int environs_InterfaceType, bool testInstance )
		{
			if ( !target || !moduleName )
				return false;

            INIT_PCHAR ( szModule, moduleName );
            
            if ( szModule == NULL ) {
                CErr ( "ApplyModuleToSettingsN: Failed to allocate string or called with NULL argument!" );
                return false;
            }

			bool success = false;
			size_t len = strlen ( szModule );

			do
			{
				if ( len <= 0 || len > 1024 )
					break;

                if ( testInstance ) {
                    IEnvironsBase * instance = (IEnvironsBase *) environs::API::CreateInstance ( szModule, 0, environs_InterfaceType, 0, instances[hInst] );
                    if ( !instance )
                        break;
                    
                    DisposeInstance ( instance );
                }

                char * modName = strdup ( szModule );
				if ( !modName )
					break;

				if ( *target ) {
					free ( *target );
				}
				*target = modName;
				success = true;
			} 
			while ( 0 );

			RELEASE_PCHAR ( szModule, moduleName );

			return success;
		}


		/**
		* Use default encoder, decoder, capture, render modules.
		*
		* @return  success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetUsePortalDefaultModulesN, jint hInst )
		{
			CVerb ( "SetUsePortalDefaultModulesN" );

			instances[hInst]->DisposePortalModules ();

			return 1;
		}
		
		/**
		* Use encoder module with the name moduleName. (libEnv-Enc...).
		*
		* @param	moduleName	the name of the module
		* @return  success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetUseEncoderN, jint hInst, jstring moduleName )
		{
			CVerb ( "SetUseEncoderN" );

			return EnvironsCallArg ( ApplyModuleToSettingsN, hInst, moduleName, &instances[hInst]->mod_PortalEncoder, InterfaceType::Encoder, true );
		}

		/**
		* Use decoder module with the name moduleName. (libEnv-Dec...).
		*
		* @param	moduleName	the name of the module
		* @return  success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetUseDecoderN, jint hInst, jstring moduleName )
		{
			CVerb ( "SetUseDecoderN" );

			return EnvironsCallArg ( ApplyModuleToSettingsN, hInst, moduleName, &instances[hInst]->mod_PortalDecoder, InterfaceType::Decoder, true );
		}

		/**
		* Use render module with the name moduleName. (libEnv-Rend...).
		*
		* @param	moduleName	the name of the module
		* @return  success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetUseRendererN, jint hInst, jstring moduleName )
		{
			CVerb ( "SetUseRendererN" );

			return EnvironsCallArg ( ApplyModuleToSettingsN, hInst, moduleName, &instances[hInst]->mod_PortalRenderer, InterfaceType::Render, true );
		}

		/**
		* Use capture module with the name moduleName. (libEnv-Cap...).
		*
		* @param	moduleName	the name of the module
		* @return  success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetUseCapturerN, jint hInst, jstring moduleName )
		{
			CVerb ( "SetUseCapturerN" );

			return EnvironsCallArg ( ApplyModuleToSettingsN, hInst, moduleName, &instances[hInst]->mod_PortalCapturer, InterfaceType::Capture, true );
        }
        
        /*
         * Method:    SetUseNotifyDebugMessage
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseNotifyDebugMessageN, EBOOL enable )
        {
            native.useNotifyDebugMessage = (enable ? true : false);
            
#ifndef DISPLAYDEVICE
            opt ( 0, APPENV_SETTING_GL_USE_HARDWARE_DECODER, enable );
#endif
        }
        
        
        /*
         * Method:    GetUseNotifyDebugMessage
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsProc ( GetUseNotifyDebugMessageN )
        {
            return (native.useNotifyDebugMessage ? 1 : 0);
        }
        
        /*
         * Method:    v
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseLogFileN, EBOOL enable )
        {
            native.useLogFile = (enable ? true : false);
            
#ifndef DISPLAYDEVICE 			
            opt ( 0, APPENV_SETTING_GL_USE_LOG_FILE, enable );
#endif
            if ( !enable )
                CloseLog ( true );
        }
        
        
        /*
         * Method:    GetUseLogFile
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsProc ( GetUseLogFileN )
        {
            return (native.useLogFile ? 1 : 0);
        }
        
        
        /**
         * Instruct Environs to log to stdout.
         *
         * @param enable      true = enable, false = disable
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseLogToStdoutN, EBOOL enable )
        {
            native.useStdout = (enable ? true : false);            
        }
        
        /**
         * Query Environs settings whether to log to stdout.
         *
         * @return enable      true = enabled, false = disabled
         */
        ENVIRONSAPI EBOOL EnvironsProc ( GetUseLogToStdoutN )
        {
            return (native.useStdout ? 1 : 0);
        }


		/**
		* Instruct Environs to use headless mode without worrying about UI thread.
		*
		* @param enable      true = enable, false = disable
		*/
        ENVIRONSAPI void SetUseHeadlessN ( int enable )
		{
			native.useHeadless = ( enable ? true : false );
		}

		/**
		* Query Environs settings whether to use headless mode without worrying about UI thread.
		*
		* @return enable      true = enabled, false = disabled
		*/
        ENVIRONSAPI int GetUseHeadlessN ( )
		{
			return ( native.useHeadless ? 1 : 0 );
        }


        /**
         * Option for whether to observe wifi networks to help location based services.
         *
         * @param	enable  A boolean that determines the target state.
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseWifiObserverN, EBOOL enable )
        {
			native.useWifiObserver = ( enable ? true : false );

#ifndef DISPLAYDEVICE
            opt ( 0, APPENV_SETTING_GL_USE_WIFI_OBSERVER, enable );
#endif

#if !defined(ENVIRONS_IOS) && !defined(ANDROID)
            sp ( Instance ) inst = GetStartedInstanceSP ( 1 );
            if ( !inst )
                return;

            if ( enable )
                native.wifiObserver.Start ();
            else
                native.wifiObserver.Stop ();
#endif
        }

        /**
         * Query option for whether to observe wifi networks to help location based services.
         *
         * @return enabled.
         */
        ENVIRONSAPI EBOOL EnvironsProc ( GetUseWifiObserverN )
        {
			return ( native.useWifiObserver ? 1 : 0 );
        }


		/**
		* Determines the interval for scanning of wifi networks.
		*
		* @param	interval  A millisecond value for scan intervals.
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseWifiIntervalN, int interval )
		{
			if ( interval < ENVIRONS_WIFI_OBSERVER_INTERVAL_MIN )
				interval = ENVIRONS_WIFI_OBSERVER_INTERVAL_MIN;

			native.useWifiInterval = interval;

#ifndef DISPLAYDEVICE
			opt ( 0, APPENV_SETTING_GL_USE_WIFI_INTERVAL, interval );
#endif
		}

		/**
		* Query interval for scanning of wifi networks.
		*
		* @return interval in milliseconds.
		*/
		ENVIRONSAPI int EnvironsProc ( GetUseWifiIntervalN )
		{
			return native.useWifiInterval;
		}


        /**
         * Option for whether to observe blueooth to help location based services.
         *
         * @param	enable  A boolean that determines the target state.
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseBtObserverN, EBOOL enable )
        {
			native.useBtObserver = ( enable ? true : false );

#ifndef DISPLAYDEVICE
            opt ( 0, APPENV_SETTING_GL_USE_BT_OBSERVER, enable );
#endif
            
#if !defined(ANDROID)
            sp ( Instance ) inst = GetStartedInstanceSP ( 1 );
            if ( !inst )
                return;

            if ( enable )
                native.btObserver.Start ();
            else
                native.btObserver.Stop ();
#endif
        }

        /**
         * Query option for whether to observe blueooth to help location based services.
         *
         * @return enabled.
         */
        ENVIRONSAPI EBOOL EnvironsProc ( GetUseBtObserverN )
        {
			return ( native.useBtObserver ? 1 : 0 );
        }


		/**
		* Determines the interval for scanning of bluetooth devices.
		*
		* @param	interval  A millisecond value for scan intervals.
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseBtIntervalN, int interval )
		{
			if ( interval <= 0 )
				return;

			native.useBtInterval = interval;

#ifndef DISPLAYDEVICE
			opt ( 0, APPENV_SETTING_GL_USE_BT_INTERVAL, interval );
#endif
		}

		/**
		* Query interval for scanning of bluetooth devices.
		*
		* @return interval in milliseconds.
		*/
		ENVIRONSAPI int EnvironsProc ( GetUseBtIntervalN )
		{
			return native.useBtInterval;
		}
        
        
        /**
         * Check for mediator logon credentials and query on command line if necessary.
         *
         * @param success      true = successful, false = failed
         */
        ENVIRONSAPI int EnvironsFunc ( QueryMediatorLogonCommandLineN, int hInst )
        {
            if ( hInst <= 0 || hInst >= ENVIRONS_MAX_ENVIRONS_INSTANCES ) {
                CErr ( "LoadSettingsN: Invalid arguments!" );
                return 0;
            }
            
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "QueryMediatorLogonCommandLineN" );
            
            sp ( Instance ) envSP = native.instancesSP [ hInst ];

			LockReleaseVA ( native.instancesSPLock, "QueryMediatorLogonCommandLineN" );
#endif
            if ( !envSP )
                return 0;
            
            sp ( MediatorClient ) mediator = envSP->mediator MED_WP;
            if ( !mediator )
                return 0;
            
            if ( mediator->HasMediatorCredentials ( 0 ) ) {
                return true;
            }
            
            return environs::API::Environs_LoginDialogCommandLine ( hInst );
        }

        
		/*
		* Method:    SetUseCLSForMediatorN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseCLSForMediatorN, jint hInst, EBOOL enable )
		{
			Instance * env = instances [ hInst ];

			env->useCLS = (enable ? true : false);
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_CLS_MEDIATOR, enable );
#endif
		}


		/*
		* Method:    GetUseCLSForMediatorN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseCLSForMediatorN, jint hInst )
		{
			return (instances[hInst]->useCLS ? 1 : 0);
		}


		/*
		* Method:    SetUseCLSForDevicesN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseCLSForDevicesN, jint hInst, EBOOL enable )
		{
			Instance * env = instances [ hInst ];

			env->useCLSForDevices = (enable ? true : false);
            
#ifndef DISPLAYDEVICE
            env->optBool ( APPENV_SETTING_USE_CLS_DEVICE, enable );
#endif
		}


		/*
		* Method:    GetUseCLSForDevicesN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseCLSForDevicesN, jint hInst  )
		{
			return (instances[hInst]->useCLSForDevices ? 1 : 0);
		}


		/*
		* Method:    SetUseCLSForDevicesEnforce
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseCLSForDevicesEnforceN, jint hInst, EBOOL enable )
		{
			Instance * env = instances [ hInst ];

			env->useCLSForDevicesEnforce = (enable ? true : false);
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_CLS_DEV_ENFORCE, enable );
#endif
		}


		/*
		* Method:    GetUseCLSForDevicesEnforce
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseCLSForDevicesEnforceN, jint hInst  )
		{
			return (instances[hInst]->useCLSForDevicesEnforce ? 1 : 0);
		}


		/*
		* Method:    SetUseCLSForAllTraffic
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseCLSForAllTrafficN, jint hInst, EBOOL enable )
		{
			instances[hInst]->useCLSForAllTraffic = (enable ? true : false);
		}


		/*
		* Method:    GetUseCLSForAllTraffic
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseCLSForAllTrafficN, int hInst  )
		{
			return (instances[hInst]->useCLSForAllTraffic ? 1 : 0);
        }
        
        
        /**
         * Enable or disable anonymous logon to the Mediator.
         *
         * @param 	enable A boolean that determines the target state.
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseMediatorAnonymousLogonN, jint hInst, EBOOL enable )
        {
            Instance * env = instances[hInst];
            
            env->useAnonymous = enable ? true : false;
            
            if ( enable ) {
                *env->UserName = 0;
                *env->DefaultMediatorToken = 0;
                *env->CustomMediatorToken = 0;
                *env->DefaultMediatorUserName = 0;
                *env->CustomMediatorUserName = 0;
            }
            
            EnvironsCallArg ( SetUseAuthenticationN, hInst, true );
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_ANONYMOUS, enable );
#endif
        }
        
        
        /**
         * Get setting of anonymous logon to the Mediator.
         *
         * @return 	enable A boolean that determines the target state.
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetUseMediatorAnonymousLogonN, jint hInst )
        {
            return (instances[hInst]->useAnonymous ? 1 : 0);
        }


		/*
		* Option that determines whether to use username/password or anonymous login at mediator server.
         If no, then no username/password will be used. However, the mediator server may refuse if it requires authentication.
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseAuthenticationN, jint hInst, EBOOL enable )
		{
			Instance * env = instances [ hInst ];

			env->useAuth = enable ? true : false;

#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_AUTH, enable );
#endif
		}

        
        EBOOL SetMediatorUserNameNM ( int hInst, const char * name )
        {
            Instance * env = instances [ hInst ];
            
#ifndef DISPLAYDEVICE
            if ( name && strlen(name) >= 3 )
                env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_USERNAME, name );
#endif
            bool anonUser = Mediator::IsAnonymousUser ( name );
            
            FAKEJNI ();
            
            EnvironsCallArg ( SetUseMediatorAnonymousLogonN, hInst, anonUser );
            
            if ( !anonUser && name )
                anonUser = (ApplyStringToSettingsNM ( hInst, name, env->UserName, sizeof ( env->UserName ) ) != 0);
            
            return anonUser;
        }
        

		/**
		* Set the user name for authentication with a mediator service.&nbsp;Usually the user's email address is used as the user name.
		* 
		* @param 	username    The user name for authentication at the Mediator.
		* @return	success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetMediatorUserNameN, jint hInst, jstring name )
		{
			CVerb ( "SetMediatorUserNameN" );
            
            INIT_PCHAR ( szValue, name );

            bool anonUser = (SetMediatorUserNameNM ( hInst, szValue ) != 0);
            
            RELEASE_PCHAR ( szValue, name );
            
            return anonUser;
		}


		/*
		* Method:    GetMediatorUserNameN
		* Signature: ()Ljava/lang/String;
		*/
		ENVIRONSAPI jstring EnvironsFunc ( GetMediatorUserNameN, jint hInst )
		{
			Instance * env = instances [ hInst ];

#ifdef ANDROID
			const char * value = optString ( hInst, BuildOptKey ( env->appAreaID, APPENV_SETTING_TOKEN_MEDIATOR_USERNAME ) );
			if ( !value || !*value )
				value = "";

			return jenv->NewStringUTF ( value );
#else
			return env->UserName;
#endif
        }
        
        
        EBOOL SetMediatorPasswordNM ( int hInst, const char * pass )
        {
            bool success = false;
            
            char * hash = 0;
            unsigned int len = 0;
            
            success = SHAHashPassword ( pass, &hash, &len );
            
            if ( success && hash && len > 0 && len < (ENVIRONS_USER_PASSWORD_LENGTH + 2) )
            {
                memcpy ( instances[hInst]->UserPassword, hash, len );
                instances[hInst]->UserPassword [len] = 0;
                success = true;
                
#ifdef DEBUGVERB
                const char * hashStr = ConvertToHexString ( hash, len );
                if ( hashStr ) {
                    CVerbArg ( "SetMediatorPasswordNM: Password [%s]", hashStr );
                }
#endif
            }
            
            if ( hash )
                free ( hash );
            
            return success;
        }

		
		ENVIRONSAPI EBOOL EnvironsFunc ( SetMediatorPasswordN, jint hInst, jstring pass )
		{
			CVerb ( "SetMediatorPasswordN" );

			if ( !pass )
				return false;

            INIT_PCHAR ( szValue, pass );
            
            if ( szValue == NULL ) {
                CErr ( "SetMediatorPasswordN: Failed to allocate string or called with NULL argument!" );
                return false;
            }

			bool success = (SetMediatorPasswordNM ( hInst, szValue ) != 0);

			RELEASE_PCHAR ( szValue, pass );

			return success;
		}


		ENVIRONSAPI EBOOL EnvironsFunc ( SetUseTouchRecognizerN, jint hInst, jstring name, jboolean enable )
		{
			CVerb ( "SetUseTouchRecognizerN" );

			if ( !name )
				return false;

            INIT_PCHAR ( szName, name );
            
            if ( szName == NULL ) {
                CErr ( "SetUseTouchRecognizerN: Failed to allocate string or null-pointer given!" );
                return false;
            }

			bool success = false;

            success = (Kernel::SetUseTouchRecognizer ( szName, enable ) != 0);

			RELEASE_PCHAR ( szName, name );

			return success;
		}


		ENVIRONSAPI char * CloneString ( const char * source )
		{
			if ( !source )
				return 0;

			size_t length = strlen ( source );
			if ( length == 0 )
				return 0;

			char * dest = (char *)calloc ( 1, length + 4 );
			if ( !dest )
				return 0;

			strlcpy ( dest, source, length + 4 );
			return dest;
		}


		ENVIRONSAPI EBOOL SetGCMAPIKeyN ( const char * key )
		{
			CVerb ( "SetGCMAPIKeyN" );

			char * newKey = CloneString ( key );
			if ( !newKey )
				newKey = (char *)environs::DefGCMAPIKey;

			if ( environs::GCMAPIKey != 0 && environs::GCMAPIKey != DefGCMAPIKey ) {
				free ( environs::GCMAPIKey );
				environs::GCMAPIKey = 0;
			}

			environs::GCMAPIKey = newKey;
			return true;
		}

        
		ENVIRONSAPI LIBEXPORT void CallConv SetCallbacksN ( int hInst, void * HumanInputCallback, void * DataInputCallback, void * MessageCallback, void * MessageExtCallback, void * NotifyCallback, void * NotifyExtCallback, void * DataCallback, void * StatusMessageCallback )
        {
            Instance * env = instances[hInst];
            
            env->callbacks.Clear ( );
            
			if ( HumanInputCallback )
				env->callbacks.OnInputDelegate	= (InputCallbackType) HumanInputCallback;

			if ( DataInputCallback )
				env->callbacks.OnDataInput      = (InputDataCallbackType) DataInputCallback;
            
            if ( MessageCallback )
                env->callbacks.OnMessage		= (MessageCallbackType) MessageCallback;
            
            if ( MessageExtCallback )
                env->callbacks.OnMessageExt		= (MessageExtCallbackType) MessageExtCallback;
            
            if ( NotifyCallback )
                env->callbacks.OnNotify			= (NotificationCallbackType) NotifyCallback;
            
            if ( NotifyExtCallback )
                env->callbacks.OnNotifyExt		= (NotificationExtCallbackType) NotifyExtCallback;
            
            if ( DataCallback )
                env->callbacks.OnData			= (DataCallbackType) DataCallback;
            
            if ( StatusMessageCallback ) {
                env->callbacks.OnStatusMessage	= (StatusMessageCallbackType) StatusMessageCallback;
                env->callbacks.doOnStatusMessage = true;
            }
        }


		/**
		* Set host device details about the display / screen.
		*
		* @param width		width of the screen in pixel
		* @param height		height of the screen in pixel
		* @param width_mm	width of the screen in physical millimeters
		* @param height_mm	height of the screen in physical millimeters
		* @param leftpos	left position of the screen (in case of a large virtual screen, where the current screen is relatively positioned)
		* @param toppos		top position of the screen (in case of a large virtual screen, where the current screen is relatively positioned)
		*/
		ENVIRONSAPI void EnvironsFunc ( SetDeviceDimsN, int width, int height, int width_mm, int height_mm, int leftpos, int toppos )
		{
            CVerb ( "SetDeviceDimsN" );

            /// Set device dimensions only if they have not be set by DetectPlatform/DetectSDK before
            if ( native.display.width_mm == ENVIRONS_DISPLAY_UNINITIALIZED_VALUE ) {
                if ( width > 0 )
                    native.display.width		= width;
                if ( width_mm > 0 )
                    native.display.width_mm	= width_mm;
                if ( height > 0 )
                    native.display.height		= height;
                if ( height_mm > 0 )
                    native.display.height_mm	= height_mm;
                
                if ( !native.platform ) {
                    if ( native.display.width_mm > 80 || native.display.height_mm > 80 )                        
                        native.platform = Platforms::Tablet_Flag;
                    else
                        native.platform = Platforms::Smartphone_Flag;
                }
            }

			native.device_left	= leftpos;
			native.device_top	= toppos;

#ifndef DISPLAYDEVICE
			//if ( env->deviceType == DEVICE_TYPE_UNKNOWN ) {
				if ( (width_mm > 70 && height_mm > 200)
                    ||  (width_mm > 200 && height_mm > 70) )
                    native.platform = Platforms::Tablet_Flag;
                else
                    native.platform = Platforms::Smartphone_Flag;
			//}
#endif
		}


		/**
		* Initialize Environs, that is create the kernel and initialize the kernel resources.
		*
		* @return an integer which represents a boolean value
		*/
		ENVIRONSAPI int EnvironsFunc ( InitN, int hInst )
		{
            CVerb ( "InitN" );
            
            Instance * env = instances[hInst];

			if ( !SetEnvironsState ( env, environs::Status::Initializing ) ) {
				CWarn ( "InitN: Environs is already initialized." );
				return 1;
            }

			/*if ( env->kernel ) {
				env->kernel->Release ( );
				delete env->kernel;
				Kernel::ReleaseLibrary ( );
			}*/

#ifndef DISPLAYDEVICE
			g_sendSequenceNumber 	= 0;

			memset ( &g_OrientationFrame, 0, sizeof(g_OrientationFrame) );
			g_OrientationFrame.preamble [0] = 'o';
			g_OrientationFrame.preamble [1] = 'f';
			g_OrientationFrame.preamble [2] = ':';
			g_OrientationFrame.version = UDP_MSG_PROTOCOL_VERSION;

			memset ( &g_AccelFrame, 0, sizeof(g_AccelFrame) );
			g_AccelFrame.preamble [0] = 'a';
			g_AccelFrame.preamble [1] = 'f';
			g_AccelFrame.preamble [2] = ':';
			g_AccelFrame.version = UDP_MSG_PROTOCOL_VERSION;
#endif

			if ( !env->kernel ) 
			{
				env->kernel = new Kernel ();
				if ( !env->kernel ) {
					CErr ( "InitN: Failed to create kernel object!!!" );
					return 0;
				}

				if ( !env->kernel->Init ( env ) ) {
					CErr ( "InitN: Failed to initialize kernel object!!!" );
					return 0;
				}
			}

			return 1;
		}


		/**
		* Query Environs' kernel status.
		*
		* @return an integer which represents one of the states of ENVIRONS_STATUS_*
		*/
		ENVIRONSAPI int EnvironsFunc ( GetStatusN, int hInst )
        {
            
            Instance * env = instances[hInst];
            
			if ( !env || !env->kernel )
				return environs::Status::Uninitialized;

			return env->environsState;
		}


		/**
		* Start Environs
		*
		* @return an integer which represents a boolean value
		*/
		ENVIRONSAPI int EnvironsFunc ( StartN, int hInst )
		{
			CLog ( "StartN" );

			int status = EnvironsCallArg ( GetStatusN, hInst );

			if ( status < environs::Status::Initialized )
			{
				// Initialize native stack
				status = EnvironsCallArg ( InitN, hInst );
				if ( status < environs::Status::Initialized ) {
					return 0;
				}
			}

			if ( status >= environs::Status::Started )
				return 1;

			Instance * inst = instances [ hInst ];
			if ( !inst )
				return 0;

			// Start kernel
			return inst->kernel->Start ( );
		}

        
        /**
         * Stop Environs
         *
         * @return an integer which represents a boolean value
         */
		ENVIRONSAPI int EnvironsFunc ( StopN, int hInst )
		{
			CLog ( "StopN" );
			
			sp ( Instance ) envSP = GetInstanceSP ( hInst );
			if ( !envSP )
				return 0;
			 
#ifdef NDEBUG
			// Stop kernel
			return envSP->kernel->Stop ( );
#else
			int success = envSP->kernel->Stop ( );

			TraceCheckStatus ( false );

			return success;
#endif
		}


		/**
		* Stop Mediator
		*
		*/
		ENVIRONSAPI void EnvironsFunc ( StopNetLayerN, int hInst )
		{
			CLog ( "StopNetLayerN" );

			sp ( Instance ) envSP = GetInstanceSP ( hInst );
			if ( !envSP )
				return;
            
            // Stop Core / Kernel network layer
            if ( envSP->kernel )
				envSP->kernel->StopNetLayer ();
        }
        
        
        /**
         * Instructs the framework to perform a quick shutdown (with minimal wait times)
         *
         * @param enable      true / false
         */
        ENVIRONSAPI void EnvironsFunc ( SetAppShutdownN, int enable )
        {
            CLog ( "SetAppShutdownN" );
            
            native.isAppShutdown = ( enable ? true : false );
        }
        
        
        /**
         * Release resources acquired by Environs and shutdown the kernel.
         *
		*/
		ENVIRONSAPI void EnvironsFunc ( DisposeN, int hInst )
		{
            CVerb ( "DisposeN" );

			native.DisposeInstance ( hInst );            
		}


		static char * defaultDescriptor = (char *)"0";

#ifdef _WIN32
		char * TCharToChar ( const jtstring tstr )
		{
			char * str = 0;
			size_t converted = 0;

			if ( tstr ) {
				size_t len = wcslen ( tstr );
				if ( len > 0 ) {
					// Conversion
					str = (char *) malloc ( len + 1 );
					if ( str ) {
						wcstombs_s ( &converted, str, len + 1, tstr, len + 1 );
						if ( converted > 0 ) {
							str [len] = 0;
						}
						else {
							free ( str );
							str = 0;
						}
					}
				}
			}

			return str;
		}
#endif
        /**
         * Send the provided buffer to the device ...
         *
         * @param
         * @return
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SendBufferN, jint hInst, jint nativeID, int async, int fileID, jstring fileDescriptor, jbyteArray buffer, jint size )
        {
            CVerbIDN ( "SendBufferN" );
            
            if ( size <= 0 ) {
                CErrIDN ( "SendBufferN: Invalid size/capacity argument [<=0]!" );
                return false;
            }

			sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
			if ( !envSP )
				return false;
            
            bool success = false;
            
            INIT_BYTEARR ( byteBuffer, buffer );
            INIT_PCHAR ( descriptor, fileDescriptor );
            
            if ( byteBuffer == NULL) {
                CErrIDN ( "SendBufferN: Failed to allocate buffer or invalid buffer (NULL pointer) given!" );
                goto EndWithStatus;
            }
            
            if ( !descriptor ) {
                descriptor = defaultDescriptor;
            }
            
            if ( async )
                success = ( envSP->asyncWorker.PushSend ( nativeID, fileID, descriptor, byteBuffer, size ) == 1);
            else
                success = DeviceBase::SendBuffer ( nativeID, fileID, descriptor, byteBuffer, size );
            
        EndWithStatus:
            RELEASE_BYTEARR ( byteBuffer, buffer, JNI_ABORT );
            RELEASE_PCHAR ( descriptor, fileDescriptor );
            return success;
        }
        
        
        /**
         * Send file to the device ...
         *
         * @param
         * @return
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SendFileN, jint hInst, jint nativeID, int async, jint fileID, jstring fileDescriptor, jstring filePath )
        {
            CVerbIDN ( "SendFileN" );

			sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
			if ( !envSP )
				return false;
            
            bool success = false;

			INIT_PCHAR ( path, filePath );
            INIT_PCHAR ( descriptor, fileDescriptor );
            
            if ( path == NULL) {
                CErrIDN ( "SendFileN: Failed to allocate pathName or invalid file path (NULL pointer) argument!" );
                goto EndWithStatus;
            }

            if ( !descriptor ) {
                descriptor = defaultDescriptor;
            }
            
            if ( async )
                success = ( envSP->asyncWorker.PushSend ( nativeID, fileID, descriptor, (const char *)path ) == 1);
            else
                success = DeviceBase::SendFile ( nativeID, fileID, descriptor, path );
            
        EndWithStatus:
			RELEASE_PCHAR ( path, filePath );
			RELEASE_PCHAR ( descriptor, fileDescriptor );
            
            return success;
        }

		/**
		* Send text message to the device ...
		*
		* @param
		* @return
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SendMessageN, jint hInst, jint deviceID, jstring areaName, jstring appName, int async, jvoidArray msg, jint length )
		{
			CVerbID ( "SendMessageN" );

			sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
			if ( !envSP )
				return false;

			bool success = false;
            
            INIT_BYTEARR ( message, msg );
			INIT_PCHAR ( szAreaName, areaName );
            INIT_PCHAR ( szAppName, appName );

			if ( message == 0 || length <= 0 ) {
				CErr ( "SendMessageN: Failed to allocate message or invalid message (NULL pointer/zero-sized) argument!" );
				goto EndWithStatus;
			}

            if ( async )
				success = ( envSP->asyncWorker.PushSend ( deviceID, ASYNCWORK_TYPE_SEND_MESSAGE, szAreaName, szAppName, message, length ) == 1);
            else
				success = DeviceBase::SendMessage ( hInst, 0, deviceID, szAreaName, szAppName, message, length );


        EndWithStatus:
            RELEASE_BYTEARR ( message, msg, JNI_ABORT );
			RELEASE_PCHAR ( szAreaName, areaName );
			RELEASE_PCHAR ( szAppName, appName );

			return success;
        }
        
        
        /**
         * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
         *
         * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
         * @param buffer        A buffer to be send.
         * @param offset        A user-customizable id that identifies the file to be send.
         * @param bytesToSend number of bytes in the buffer to send
         * @return success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SendDataUdpN, jint hInst, jint nativeID, int async, jbyteArray buffer, jint offset, jint size )
        {
            CVerbIDN ( "SendDataUdpN" );
            
            if ( !buffer || size <= 0 ) {
                CErrIDN ( "SendDataUdpN: Invalid size/capacity argument [<=0]!" );
                return false;
            }
            
            sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
            if ( !envSP )
                return false;
            
            bool success = false;
            
            INIT_BYTEARR ( byteBuffer, buffer );
            
            if ( byteBuffer == NULL) {
                CErrIDN ( "SendDataUdpN: Failed to allocate buffer or invalid buffer (NULL pointer) given!" );
                goto EndWithStatus;
            }
            
            if ( async )
                success = envSP->asyncWorker.PushData ( ASYNCWORK_TYPE_SEND_UDP, nativeID, buffer + offset, size );
            else
                success = envSP->asyncWorker.SendDataUdpPrefix ( nativeID, buffer + offset, size );
            
        EndWithStatus:
            RELEASE_BYTEARR ( byteBuffer, buffer, JNI_ABORT );
            return success;
        }
        

		/**
		* Send push notification to the device ...
		*
		* @param
		* @return
		*/
		ENVIRONSAPI EBOOL SendPushNotificationN ( int hInst, int deviceID, const char * areaName, const char * appName, const char * message )
		{
			CVerbID ( "SendPushNotificationN" );

			// Propagate to kernel
			return DeviceController::SendPushNotification ( instances[hInst], deviceID, areaName, appName, message );
		}

        
        int GetDeviceConnectStatus ( int hInst, int nativeID )
        {
            int connectStatus = DeviceStatus::Deleteable;
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            if ( device ) {
                connectStatus = device->deviceStatus;
                UnlockDevice ( device );
            }
            
            return connectStatus;
        }
        
        
        /**
         * Query the status whether Environs is connected to the device identified by deviceID.
         *
         * @param	Destination device ID
         * @return	connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
         */
        ENVIRONSAPI int EnvironsFunc ( GetDeviceConnectStatusN, jint hInst, jint nativeID )
        {
            CVerbIDN ( "GetDeviceConnectStatusN" );
            
            if ( nativeID <= 0 ) {
                CErrIDN ( "GetDeviceConnectStatusN: Device id must not be <= 0" );
                return false;
            }       
            return GetDeviceConnectStatus ( hInst, nativeID );
        }
        
        
        /**
         * Release memory
         *
         * @param    ptr
         *
         */
        ENVIRONSAPI void EnvironsFunc ( ReleaseStringN, void * ptr )
        {
			CVerb ( "ReleaseStringN" );
            if ( ptr )
                free ( ptr );
        }
        
        
        /**
         * Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
         *  The resulting memory is managed by Environs
         *
         * @param    nativeID
         * @param    fileID
         *
         * @return
         */
		ENVIRONSAPI jstring EnvironsFunc ( GetFilePathNativeN, jint hInst, jint nativeID, jint fileID )
		{
			CVerb ( "GetFilePathNativeN" );

			DeviceBase * device = environs::GetDevice ( nativeID );
			if ( !device )
				return 0;

			const char * path = device->GetFilePath ( fileID );

			UnlockDevice ( device );
			if ( !path )
                return 0;
            
            return JSTRINGNEW ( path );
        }


		/**
		* Load the file at given path into a byte array.
		*  The memory MUST BE FREED by the callee.
		*
		* @param
		* @return
		*/
		ENVIRONSAPI char * EnvironsFunc ( LoadBinaryN, CString_ptr filePath, int * size )
		{
			CVerb ( "LoadBinaryN" );

			return environs::LoadBinary ( filePath, size );
		}

        
        /**
         * Query the absolute path name to the storage of a given device identity.
         *  The memory MUST BE FREED by the callee.
         *
         * @param    deviceID
         * @param    areaName
         * @param    appName
         *
         * @return
         */
		ENVIRONSAPI jsstring EnvironsFunc ( GetFilePathForStorageN, jint hInst, jint deviceID, jstring areaName, jstring appName )
		{
            CVerb ( "GetFilePathForStorageN" );
            
            sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
            if ( !envSP )
                return 0;

			INIT_PCHAR ( szAreaName, areaName );
			INIT_PCHAR ( szAppName, appName );

			unsigned short length = 0;

			char * path = DeviceBase::GetFilePath ( envSP.get (), deviceID, szAreaName, szAppName, length );

			RELEASE_PCHAR ( szAreaName, areaName );
			RELEASE_PCHAR ( szAppName, appName );

			if ( !path )
                return 0;
            
            JSTRINGNEW_DISPOSE ( path );
        }
        
        
        /**
         * Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
         *  The resulting memory is managed by Environs
         *
         * @param   deviceID
         * @param   areaName
         * @param   appName
         * @param   fileID
         *
         * @return
         */
		ENVIRONSAPI jstring EnvironsFunc ( GetFilePathN, jint hInst, jint deviceID, jstring areaName, jstring appName, jint fileID )
        {
			CVerb ( "GetFilePathN" );
            
			INIT_PCHAR ( szAreaName, areaName );
            INIT_PCHAR ( szAppName, appName );
            
			DeviceBase * device = environs::GetDevice ( instances[hInst], deviceID, szAreaName, szAppName );
            
			RELEASE_PCHAR ( szAreaName, areaName );
            RELEASE_PCHAR ( szAppName, appName );
            
            if ( !device )
                return 0;
            
            const char * path = device->GetFilePath ( fileID );
            
            UnlockDevice ( device );
            if ( !path )
                return 0;
            return JSTRINGNEW ( path );
        }
        
        
        /**
         * Load the file into a byte array that belonging to the fileID and received from the deviceID.
         *  The memory MUST BE FREED by the callee.
         *
         * @param   device
         * @param   fileID
         * @param   buffer
         * @param   capacity
         *
         * @return
         */
        
        jvoidArray EnvironsFunc ( GetFileN, DeviceBase * device, jint fileID, jvoidArray buffer, jintp capacity )
        {
			CVerb ( "GetFileN: with device" );
            
            jvoidArray	retBuffer = 0;
            
#ifdef ANDROID
            jbyteArray jbArray = 0;
#endif
            
#ifdef _WIN32
            if ( !capacity )
                goto Finish;
            
            if ( !device->LoadFromStorage ( fileID, ( char * ) buffer, capacity ) )
                goto Finish;
            
            retBuffer = buffer;
#else
            int capacity_ = 0;
            char * buffer_ = 0;
            
            if ( !device->LoadFromStorage ( fileID, 0, &capacity_ ) || !capacity_ )
                goto Finish;
            
			buffer_ = ( char * ) malloc ( capacity_ );
            if ( !buffer_ ) {
                CErrArg ( "GetFileN: malloc failed. [ %u bytes ]", capacity_ );
                goto Finish;
            }
            
            if ( !device->LoadFromStorage ( fileID, ( char * ) buffer_, &capacity_ ) || !capacity_ ) {
                free ( buffer_ );
                goto Finish;
            }
            
#ifdef ANDROID
            jbArray = jenv->NewByteArray ( capacity_ );
            if ( jbArray ) {
                jenv->SetByteArrayRegion ( jbArray, 0, capacity_, ( jbyte * ) buffer_ );
                retBuffer = jbArray;
            }
            else {
                CErrArg ( "GetFileN: NewByteArray failed. [ %u bytes ]", capacity_ );
            }

            free ( buffer_ );
#else
            if ( capacity )
                *capacity = capacity_;
            
            retBuffer = buffer_;
#endif
#endif
        Finish:
            return retBuffer;
        }
        
        
        /**
         * Load the file into a byte array that belonging to the fileID and received from the deviceID.
         *  The memory MUST BE FREED by the callee.
         *
         * @param
         * @return
         */
        ENVIRONSAPI jvoidArray EnvironsFunc ( GetFileNativeN, jint hInst, jint nativeID, jint fileID, jvoidArray buffer, jintp capacity )
        {
			CVerb ( "GetFileNativeN" );
            
            DeviceBase * device = environs::GetDevice ( nativeID );
            if ( !device )
                return 0;
            
            jvoidArray	retBuffer = EnvironsCallArg ( GetFileN, device, fileID, buffer, capacity );
            
            UnlockDevice ( device );
            
            return retBuffer;
        }

        
        /**
         * Load the file into a byte array that belonging to the fileID and received from the deviceID.
         *  The memory MUST BE FREED by the callee.
         *
         * @param
         * @return
         */
		ENVIRONSAPI jvoidArray EnvironsFunc ( GetFileN, jint hInst, jint deviceID, jstring areaName, jstring appName, jint fileID, jvoidArray buffer, jintp capacity )
		{
			CVerb ( "GetFileN: with deviceID" );

			INIT_PCHAR ( szAreaName, areaName );
			INIT_PCHAR ( szAppName, appName );

			CVerbArg ( "GetFileN: Device [ 0x%X : %s : %s ]", deviceID, szAreaName ? szAreaName : "EnvNULL", szAppName ? szAppName : "EnvNULL" );

			DeviceBase * device = environs::GetDevice ( instances [ hInst ], deviceID, szAreaName, szAppName );

			RELEASE_PCHAR ( szAreaName, areaName );
			RELEASE_PCHAR ( szAppName, appName );

			if ( !device ) {
				CVerbArg ( "GetFileN: Device [ 0x%X  ] not found!", deviceID );
				return 0;
			}
            
            jvoidArray	retBuffer = EnvironsCallArg ( GetFileN, device, fileID, buffer, capacity );

			UnlockDevice ( device );

			return retBuffer;
		}
        
        
        /*
         * Method:    SetUseDefaultMediatorN
         * Signature: (Z)Z
         */
		ENVIRONSAPI void EnvironsFunc ( SetUseDefaultMediatorN, jint hInst, EBOOL enable )
		{
			CVerbVerb ( "SetUseDefaultMediatorN" );

            Instance * env = instances [ hInst ];
            
            bool state = enable ? true : false;
            
            if ( env->useDefaultMediator == state )
                return;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( mediator )
                mediator->ReleaseMediators ();
            
            env->useDefaultMediator = state;

#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_DEFAULT_MEDIATOR, enable );
#endif
		}


		/*
		* Method:    GetUseDefaultMediatorN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseDefaultMediatorN, jint hInst )
		{
			CVerbVerb ( "GetUseDefaultMediatorN" );

			return instances[hInst]->useDefaultMediator;
		}


		/*
		* Method:    SetUseCustomMediatorN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseCustomMediatorN, jint hInst, EBOOL enable )
		{
			CVerbVerb ( "SetUseCustomMediatorN" );

			Instance * env = instances [ hInst ];
            
            bool state = enable ? true : false;
            
            if ( env->useCustomMediator == state )
                return;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( mediator )
                mediator->ReleaseMediators ();
            
            env->useCustomMediator = state;
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_CUSTOM_MEDIATOR, enable );
#endif
        }
        
        
        /*
         * Method:    SetUseMediatorLoginDialogN
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseMediatorLoginDialogN, jint hInst, EBOOL enable )
        {
			CVerbVerb ( "SetUseMediatorLoginDialogN" );

			Instance * env = instances [ hInst ];

			env->opt_useMediatorLoginDialog = enable ? true : false;
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_MEDIATOR_LOGIN_DLG, enable );
#endif
        }
        
        
        /*
         * Method:    GetUseMediatorLoginDialogN
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetUseMediatorLoginDialogN, jint hInst )
        {
			CVerbVerb ( "GetUseMediatorLoginDialogN" );

            return environs::instances[hInst]->opt_useMediatorLoginDialog;
        }
        
        
        /*
         * Method:    SetMediatorLoginDialogDismissDisableN
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetMediatorLoginDialogDismissDisableN, jint hInst, EBOOL enable )
        {
			CVerbVerb ( "SetMediatorLoginDialogDismissDisableN" );

			Instance * env = instances [ hInst ];

			env->opt_useMediatorLoginDialogDismissDisable = enable ? true : false;
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_MEDIATOR_LOGIN_DLG, enable );
#endif
        }
        
        
        /*
         * Method:    GetMediatorLoginDialogDismissDisableN
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetMediatorLoginDialogDismissDisableN, jint hInst )
        {
			CVerbVerb ( "GetMediatorLoginDialogDismissDisableN" );

            return environs::instances[hInst]->opt_useMediatorLoginDialogDismissDisable;
		}


		/*
		* Method:    ShowLoginDialogN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( ShowLoginDialogN, jint hInst, CString_ptr userName )
		{
			CVerbVerb ( "ShowLoginDialogN" );

			if ( !native.useHeadless )
				environs::API::Environs_LoginDialog ( hInst, userName );
		}

		
		/*
		* Class:     hcm_environs_Environs
		* Method:    setNetworkStatus
		* Signature: (I)V
		*/
		ENVIRONSAPI void EnvironsFunc ( SetNetworkStatusN, jint netStat )
		{
			CInfoArg ( "SetNetworkStatusN: [ %i ]", netStat );

            if ( native.networkStatus == netStat )
                return;
            
			int previousNetStat = native.networkStatus;

			native.networkStatus = netStat;

            Mediator::LoadNetworks ();
            
            if ( netStat == NetworkConnection::TriggerUpdate ) {
				/// -3 means trigger an UpdateNetworkStatus thread
				UpdateNetworkStatus ();
			}
            else {
				for ( int i=1; i<ENVIRONS_MAX_ENVIRONS_INSTANCES; ++i )
                {
                    Instance * env = instances[i];
                    if ( !env )
                        continue;
                    
                    sp ( MediatorClient ) mediator = env->mediator MED_WP;
                    
					if ( mediator && mediator->IsStarted () ) 
					{
						if ( previousNetStat < NetworkConnection::NoInternet )
							mediator->connectFails = 0;

						mediator->RegisterAtMediators ( false );

						mediator->SendBroadcast ();

						mediator->StartAliveThread ();
					}
                    
                    onEnvironsNotifier1 ( env, Notify::Network::Changed );
                }
            }
		}
        
        
		/*
         * Method:    setNetworkStatus
         * Signature: ()I
         */
		ENVIRONSAPI jint EnvironsProc ( GetNetworkStatusN )
		{
			CVerbVerb ( "GetNetworkStatusN" );

			return native.networkStatus;
		}

        
		/*
		* Method:    GetUseCustomMediatorN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseCustomMediatorN, jint hInst )
		{
			CVerbVerb ( "GetUseCustomMediatorN" );

			return instances[hInst]->useCustomMediator;
		}


#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
		ENVIRONSAPI EBOOL EnvironsFunc ( SetMediatorN, jint hInst, jstring ip, int port )
		{
			CVerb ( "SetMediatorN" );

			bool success = false;

			Instance * env = instances [ hInst ];
            
			if ( port > 0 && port <= 65535) {
				env->CustomMediatorPort = ( unsigned short ) port;
                
#ifndef DISPLAYDEVICE
                
#ifndef ANDROID
				JNIEnv * jenv = 0;
#endif
				char mediatorPort [ 12 ];
				sprintf ( mediatorPort, "%u", ( unsigned short ) port ); // port number must not be negative
#ifdef ANDROID
				{
					jstring jvalue = jenv->NewStringUTF ( mediatorPort );
					opt ( jenv, hInst, BuildOptKey ( env->appAreaID, APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT ), jvalue );
				}
#else
				opt ( jenv, hInst, BuildOptKey ( env->appAreaID, APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT ), mediatorPort );
#endif
#endif
                
				success = true;
			}
			else {
				CInfoArg ( "SetMediatorN: ignoring port [%d]", port );
			}

            INIT_PCHAR ( szIP, ip );
            
            if ( szIP == NULL ) {
                CErr ( "SetMediatorN: Failed to allocate local IP-Addr. or null-pointer for ip-address given!" );
                return false;
            }
            
            if ( strlen ( szIP ) >= 7 ) {
#ifndef DISPLAYDEVICE
				env->opt ( APPENV_SETTING_USE_CUSTOMMEDIATOR_IP, szIP );
#endif
				env->CustomMediatorIP = inet_addr ( szIP );
                success = true;
            }
            else {
                CInfoArg ( "SetMediatorN: ignoring [%s]", szIP );
            }
            
            
			RELEASE_PCHAR ( szIP, ip );

			return success;
		}

#ifdef _WIN32
#pragma warning( pop )
#endif

		/*
		* Method:    GetMediatorIPN
		* Signature: ()Ljava/lang/String;
		*/
		ENVIRONSAPI jstring EnvironsFunc ( GetMediatorIPN, jint hInst )
		{
			CVerbVerb ( "GetMediatorIPN" );

			Instance * env = instances [ hInst ];

#ifdef DISPLAYDEVICE
			return inet_ntoa ( *((struct in_addr *) &env->CustomMediatorIP) );
#else
			const char * mediatorIP = optString ( hInst, BuildOptKey ( env->appAreaID, APPENV_SETTING_USE_CUSTOMMEDIATOR_IP ) );
			if ( !mediatorIP )
				mediatorIP = "";

            return JSTRINGNEW ( mediatorIP );
#endif
		}

		/*
		* Method:    GetMediatorIPValueN
		* Signature: ()Ljava/lang/String;
		*/
		ENVIRONSAPI unsigned int EnvironsFunc ( GetMediatorIPValueN, jint hInst )
		{
			CVerbVerb ( "GetMediatorIPValueN" );

			Instance * env = instances [ hInst ];

			return env->CustomMediatorIP;
		}

		

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
		/*
		* Method:    GetMediatorPortN
		* Signature: ()I;
		*/
		ENVIRONSAPI int EnvironsFunc ( GetMediatorPortN, jint hInst )
		{
			CVerbVerb ( "GetMediatorPortN" );

			Instance * env = instances [ hInst ];

#ifdef DISPLAYDEVICE
			return env->CustomMediatorPort;
#else
			const char * mediatorPort = opt ( hInst, BuildOptKey ( env->appAreaID, APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT ) );
			if ( !mediatorPort )
				return 0;

			int port = 0;
			if ( sscanf ( mediatorPort, "%d", &port ) == 1 )
				return port;
			return 0;
#endif
		}

#ifdef _WIN32
#pragma warning( pop )
#endif

		/**
		* Determine whether Environs shall automatically adapt the layout dimensions of
		* the View provided for the portal with the deviceID.
		* The layout dimensions are in particular important for proper mapping of TouchDispatch contact points
		* on the remote portal.
		* If enable is set to false, then custom applications must adapt the layout parameters
		* by means of calling SetPortalViewDimsN().
		*
		* @param enable    A boolean that determines the target state.
		*/
		ENVIRONSAPI void EnvironsFunc ( SetPortalViewDimsAutoN, jint hInst, EBOOL enable )
		{
			CVerbVerb ( "SetPortalViewDimsAutoN" );

			Instance * env = instances [ hInst ];

			env->usePortalViewDimsAuto = enable ? true : false;

#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_PORTALVIEW_DIMS_AUTO, enable );
#endif
		}


		/**
		* Query the option whether Environs adapts the portal according to the size/location
		* of its view within the layout.
		*
		* @return enabled
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalViewDimsAutoN, jint hInst )
		{
			CVerbVerb ( "GetPortalViewDimsAutoN" );

			return instances[hInst]->usePortalViewDimsAuto;
		}


		/**
		* Set the location (and its size) of the portal that belongs to the nativeID.
		* Such values are usually provided within the onLayoutChangeListener of a View.
		*
		* @param portalID      The portal device id of the target device.
		* @param left          The left coordinate
		* @param top           The top coordinate
		* @param right         The right coordinate
		* @param bottom        The bottom coordinate
		*
		* @return success		This call will fail, if the touchsource (and portal resources) have not been initialized.
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetPortalViewDimsN, jint hInst, jint portalID, jint left, jint top, jint right, jint bottom )
		{
			CLogArg ( "SetPortalViewDimsN: portalID [ 0x%X ] l/t/r/b [ %i / %i / %i / %i ]", portalID, left, top, right, bottom );

			if ( left == 0 && top == 0 && right == 0 && bottom == 0 ) {
				CWarn ( "SetPortalViewDimsN: called with 0 valued arguments" );
				return 0;
			}

			if ( right <= left || bottom <= top ) {
				CWarn ( "SetPortalViewDimsN: called with invalid arguments!" );
				return 0;
			}

			EBOOL success = 0;

#ifndef DISPLAYDEVICE
			PortalDevice * portal = GetLockedPortalDevice ( portalID );
			if ( !portal ) {
				CErr ( "SetPortalViewDimsN: No portal resource found." );
				return 0;
			}

			int width	= right - left;
			int height	= bottom - top;

			if ( !portal->receiver || !portal->receiver->touchSource ) {
				CErrArg ( "SetPortalViewDimsN: No portal receiver or touchsource available for portalID [ 0x%X ].", portalID );
				goto Finish;
			}

			if ( native.display.width == width && native.display.height == height ) {
				CLogArg ( "SetPortalViewDimsN: width [ %i ] height [ %i ] are the same as the display sizes.", width, height );

				portal->receiver->touchSource->viewAdapt	= false;
				goto Finish;
			}
			CLogArg ( "SetPortalViewDimsN: width [ %i ] height [ %i ]", width, height );

			portal->receiver->touchSource->xScale = ( float ) ( ( double ) native.display.width / ( double ) width );
			portal->receiver->touchSource->yScale = ( float ) ( ( double ) native.display.height / ( double ) height );

			portal->receiver->touchSource->xOffset = -left;
			portal->receiver->touchSource->yOffset = -top;

			CLogArg ( "SetPortalViewDimsN: xScale [ %f ] yScale [ %f ]", portal->receiver->touchSource->xScale, portal->receiver->touchSource->yScale );

			portal->receiver->touchSource->viewAdapt	= true;

			success = 1;

		Finish:
			ReleasePortalDevice ( portal );
#endif
			return success;
		}


		/*
		* Method:    getDeviceInstanceSize
		* Signature: ()I;
		*/
		ENVIRONSAPI jint EnvironsProc ( GetDeviceInstanceSizeN )
		{
			//CInfoArg ( "getDeviceInstanceSize: [%d]", (int) DEVICE_PACKET_SIZE );
			return DEVICE_PACKET_SIZE;
		}


		/*
		* Method:    getDevicesHeaderSize
		* Signature: ()I;
		*/
		ENVIRONSAPI jint EnvironsProc ( GetDevicesHeaderSizeN )
		{
			//CInfoArg ( "getDevicesHeaderSize: [%d]", DEVICES_HEADER_SIZE );
			return DEVICES_HEADER_SIZE;
		}


		/*
		* Method:    getConnectedDevicesCount
		* Signature: ()I;
		*/
		ENVIRONSAPI jint EnvironsFunc ( GetConnectedDevicesCountN, jint hInst  )
		{
			return GetConnectedDevicesManagedCount ( );
		}


		/*
		* Method:    GetDevicesCountN
		* Signature: (I)I;
		*/
		ENVIRONSAPI jint EnvironsFunc ( GetDevicesCountN, jint hInst, jint fromType )
        {
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return 0;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( !mediator ) {
				CVerb ( "GetDevicesCountN: No mediator instance available!" );
				return 0;
			}

			if ( env->environsState < environs::Status::Started ) {
				CVerb ( "GetDevicesCountN: Environs is not started!" );
				return 0;
			}

			switch ( fromType ) {
			case MEDIATOR_DEVICE_CLASS_ALL:  // Available
				return mediator->GetDevicesAvailableCountCached ( );
				break;
			case MEDIATOR_DEVICE_CLASS_NEARBY: // Nearby
				return mediator->GetDevicesNearbyCount ( );
				break;
			case MEDIATOR_DEVICE_CLASS_MEDIATOR:  // Mediator
				return mediator->GetDevicesFromMediatorCountCached ( );
				break;
			}

			return 0;
		}


		/**
		* Query a DeviceInfo object that best match the deviceID only.
		* Usually the one that is in the same app environment is picked up.
		* If there is no matching in the app environment,
		* then the areas are searched for a matchint deviceID.
		* Note: IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		*
		* @param deviceID      The portalID that identifies an active portal.
		* @return DeviceInfo-ByteBuffer
		*/
		ENVIRONSAPI jobject EnvironsFunc ( GetDeviceBestMatchN, jint hInst, jint deviceID )
		{
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return 0;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( !mediator ) {
				CVerb ( "GetDeviceBestMatchN: No mediator instance available!" );
				return 0;
			}
            
#ifndef ANDROID
			void * jenv = 0;
#endif
			char * buffer = 0;
			int deviceCount = mediator->GetDevicesAvailableCountCached ();

			if ( deviceCount <= 0 )
				return 0;

			jobject byteBuffer = allocJByteBuffer ( jenv, sizeof ( lib::DevicePack ), buffer );
			if ( !buffer ) {
				CErr ( "GetDeviceBestMatchN: Failed to allocate buffer for 1 device!" );
				return 0;
			}

			CVerb ( "GetDeviceBestMatchN: Available" );
			deviceCount = mediator->GetDevicesAvailableCachedBestMatch ( &buffer, sizeof ( lib::DevicePack ), deviceID );

			if ( deviceCount <= 0 || !buffer )
				return 0;

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
			return byteBuffer;

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		}
		

		/*
		* Method:    GetDevicesN
        * IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		* Signature: (I)Ljava/nio/ByteBuffer;
		*/
		ENVIRONSAPI jobject EnvironsFunc ( GetDevicesN, jint hInst, jint fromType )
        {
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return 0;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( !mediator ) {
				CVerb ( "GetDevicesN: No mediator instance available!" );
				return 0;
			}

            DUMBJENV ();

			char * buffer = 0;
			int deviceCount;

			if ( fromType == MEDIATOR_DEVICE_CLASS_ALL ) 
			{
				CVerb ( "GetDevicesN: Available" );

				return mediator->GetDevicesAvailableCached ( jenv );

				/*if ( deviceCount <= 0 || !buffer )
					return 0;
#ifdef ANDROID
				int bufferSize = (deviceCount * DEVICE_PACKET_SIZE) + (2 * DEVICES_HEADER_SIZE);

				char * copyBuffer = 0;
				jobject byteBuffer = allocJByteBuffer ( jenv, bufferSize, copyBuffer );
				if ( !byteBuffer ) {
					CErrArg ( "GetDevicesN: Failed to allocate buffer for [%d] devices!", deviceCount );
					free ( buffer );
					return 0;
				}
				else {
					memcpy ( copyBuffer, buffer, bufferSize );
					free ( buffer );
					return byteBuffer;
				}
#endif
				return (jobject) buffer;*/
			}

			/// The other types
			deviceCount = EnvironsCallArg ( GetDevicesCountN, hInst, fromType );
			if ( deviceCount <= 0 )
				return 0;

			if ( deviceCount > 540 ) {
				CWarn ( "GetDevicesN: More than 540 devices available! Limiting request to 540." );
				deviceCount = 540;
			}

			int bufferSize = (deviceCount * DEVICE_PACKET_SIZE) + (2 * DEVICES_HEADER_SIZE);


			jobject byteBuffer = allocJByteBuffer ( jenv, bufferSize, buffer );
			if ( !buffer ) {
				CErrArg ( "GetDevicesN: Failed to allocate buffer for [%d] devices!", deviceCount );
				return 0;
			}

			switch ( fromType ) {
			case MEDIATOR_DEVICE_CLASS_NEARBY: // Nearby
				CVerb ( "GetDevicesN: Nearby" );
				deviceCount = mediator->GetDevicesNearby ( buffer, bufferSize, 0 );
				break;

			case MEDIATOR_DEVICE_CLASS_MEDIATOR:  // Mediator
				CVerb ( "GetDevicesN: Mediator" );
				deviceCount = mediator->GetDevicesFromMediatorCached ( buffer, bufferSize, 0 );
				break;
			}

			if ( deviceCount <= 0 ) {
				if ( byteBuffer )
					releaseJByteBuffer ( byteBuffer );
				return 0;
			}

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
			return byteBuffer;

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		}


		/*
		* Method:    GetWifisN
		* IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		* Signature: (I)Ljava/nio/ByteBuffer;
		*/
		ENVIRONSAPI jobject EnvironsProc ( GetWifisN )
		{
#ifndef ENVIRONS_IOS
			DUMBJENV ();

			jobject byteBuffer = native.wifiObserver.BuildNetData ( jenv );

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
			return byteBuffer;
            // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
#else
            return 0;
#endif
		}


		/*
		* Method:    GetBtN
		* IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		* Signature: (I)Ljava/nio/ByteBuffer;
		*/
		ENVIRONSAPI jobject EnvironsProc ( GetBtsN )
		{
			DUMBJENV ();

			jobject byteBuffer = native.btObserver.BuildNetData ( jenv );

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
			return byteBuffer;
			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		}

		
        /**
         * Query a DeviceInfo object for the active portal identified by the portalID.
         *
         * @param portalID      The portalID that identifies an active portal.
         * @return DeviceInfo-object    IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
         */
        ENVIRONSAPI jobject EnvironsFunc ( GetDeviceForPortalN, jint hInst, jint portalID )
        {
            CVerb ( "GetDeviceForPortalN" );
            
            DeviceBase * device = GetDeviceIncLock ( portalID );
            if ( device )
            {
				jobject deviceInfo = EnvironsCallArg ( GetDeviceN, hInst, device->deviceID, (jstring) device->deviceAreaName, (jstring) device->deviceAppName, MEDIATOR_DEVICE_CLASS_ALL );
                
                UnlockDevice ( device );
                
                // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
                
                return deviceInfo;
                
                // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
            }
            
            return 0;
        }

        
		/*
		* Method:    GetDeviceN
        * IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
		* Signature: (II)Ljava/nio/ByteBuffer;
		*/
		ENVIRONSAPI jobject EnvironsFunc ( GetDeviceN, jint hInst, jint deviceID, jstring areaName, jstring appName, jint fromType )
        {
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return 0;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( !mediator ) {
				CVerbID ( "GetDeviceN: No mediator instance available!" );
				return 0;
			}

			INIT_PCHAR ( szAreaName, areaName );
			INIT_PCHAR ( szAppName, appName );

			DUMBJENV ();

			char *	buffer		= 0;
			int		deviceCount = 0;
			jobject byteBuffer	= 0;

			int		bufferSize	= DEVICE_PACKET_SIZE + (2 * DEVICES_HEADER_SIZE);

			byteBuffer = allocJByteBuffer ( jenv, bufferSize, buffer );
			if ( !buffer ) {
				CErrID ( "GetDeviceN: Failed to allocate buffer for [1] device!" );
				goto Finish;
            }
            
            if ( fromType == MEDIATOR_DEVICE_CLASS_NEARBY ) {
                goto getDeviceNearby;
            }
            
            CVerbID ( "GetDeviceN: Mediator" );
			deviceCount = mediator->GetDeviceFromMediatorCached ( buffer, bufferSize, deviceID, szAreaName, szAppName );
            
            if ( deviceCount <= 0 && fromType == MEDIATOR_DEVICE_CLASS_ALL )
                goto getDeviceNearby;
            goto Finish;


        getDeviceNearby:
            CVerbID ( "GetDeviceN: Nearby" );
			deviceCount = mediator->GetDevicesNearby ( buffer, bufferSize, 0, deviceID, szAreaName, szAppName );

		Finish:
			CVerbID ( "GetDeviceN: Done" );
			RELEASE_PCHAR ( szAreaName, areaName );
			RELEASE_PCHAR ( szAppName, appName );

			if ( deviceCount <= 0 ) {
				releaseJByteBuffer ( byteBuffer );
				return 0;
			}

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
			return byteBuffer;

			// IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
        }
        
        
        /*
         * Method:    GetDeviceN
         * IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
         * Signature: (II)Ljava/nio/ByteBuffer;
         */
        ENVIRONSAPI jobject EnvironsFunc ( GetDeviceByObjIDN, jint hInst, OBJIDType nativeID )
        {
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return 0;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( !mediator ) {
                CVerbIDN ( "GetDeviceByObjIDN: No mediator instance available!" );
                return 0;
            }
            
            DUMBJENV ();
            
            char *	buffer		= 0;
            int		deviceCount = 0;
            jobject byteBuffer	= 0;
            
            int		bufferSize	= DEVICE_PACKET_SIZE + (2 * DEVICES_HEADER_SIZE);
            
            byteBuffer = allocJByteBuffer ( jenv, bufferSize, buffer );
            if ( !buffer ) {
                CErrIDN ( "GetDeviceByObjIDN: Failed to allocate buffer for [1] device!" );
                goto Finish;
            }
            
            CVerbIDN ( "GetDeviceByObjIDN: Mediator" );
            deviceCount = mediator->GetDevicesAvailableCached ( &buffer, bufferSize, nativeID );
            
            
        Finish:
            CVerbIDN ( "GetDeviceByObjIDN: Done" );
            
            if ( deviceCount <= 0 ) {
                releaseJByteBuffer ( byteBuffer );
                return 0;
            }
            
            // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
            return byteBuffer;
            
            // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
        }
        

		/*
		* Method:    GetDeviceDisplayPropsN
		* Signature: (I)Ljava/nio/ByteBuffer;
         */
        ENVIRONSAPI jobject EnvironsFunc ( GetDeviceDisplayPropsN, jint hInst, jint nativeID )
        {
			DeviceBase * device = 0;

			if ( nativeID > 0 )
			{
				if ( instances [ hInst ]->environsState < environs::Status::Starting ) {
					CWarnIDN ( "GetDeviceDisplayPropsN: Environs is not started!" );
					return 0;
				}

				device = environs::GetDevice ( nativeID );
				if ( !device ) {
					CWarnIDN ( "GetDeviceDisplayPropsN: Device is not connected!" );
					return 0;
				}
			}
#ifndef ANDROID
            void * jenv = 0;
#endif
            char * buffer = 0;
            
            jobject byteBuffer = allocJByteBuffer ( jenv, sizeof(DeviceDisplay), buffer );
            if ( !buffer ) {
                CErrArgIDN ( "GetDeviceDisplayPropsN: Failed to allocate buffer [%u bytes] device screen sizes!", (unsigned int)sizeof ( DeviceDisplay ) );
                byteBuffer = 0;
            }
            else {
				if ( nativeID > 0 )
					*( ( DeviceDisplay * ) buffer ) = device->display;
				else
					*( ( DeviceDisplay * ) buffer ) = native.display;
            }

			if ( device )
				UnlockDevice ( device );
            
            // IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
            return byteBuffer;
        }


		/**
		* Method:    FreeNativeMemory
		* Signature: ()V
		*/
		ENVIRONSAPI void FreeNativeMemoryN ( void * obj )
		{
			CVerb ( "FreeNativeMemoryN" );

			free_n ( obj );
		}


		/**
		* Method:    SetGCM
		* Signature: (Ljava/lang/String;)V
		*/
		ENVIRONSAPI void EnvironsFunc ( SetGCMN, jstring gcm )
		{
			CVerb ( "SetGCMN" );

			INIT_PCHAR ( szGCM, gcm );

            if ( szGCM == NULL ) {
                CErr ( "SetGCMN: Failed to allocate local gcm registration id string!" );
                return;
            }
            
			size_t len = strlen ( szGCM );
			if ( len > 0 ) {
				len += 5;
				char * newRegID = (char *)malloc ( len );
				if ( newRegID ) {
					if ( g_gcmRegID ) {
						free ( g_gcmRegID );
					}
					g_gcmRegID = newRegID;
					snprintf ( newRegID, len, "gcm%s", szGCM );
                    
                    //Instance * env = instances[hInst];

					/*if ( mediator ) {
						mediator->SetParam ( env->deviceID, env->areaName, env->appName, "pn", g_gcmRegID );
					}
                    */
				}
				else {
					CErr ( "SetGCMN: memory allocation failed!" );
				}
			}
			else {
				CErr ( "SetGCMN: gcm reg id too short (0)!" );
			}

			RELEASE_PCHAR ( szGCM, gcm );
		}


		/**
		* Method:    InitStorage
		* Signature: (Ljava/lang/String;)V
		*/
		ENVIRONSAPI void EnvironsFunc ( InitStorageN, jstring path )
        {
            CVerbVerb ( "InitStorageN" );
            
			INIT_PCHAR ( szPath, path );

            ANDROID_ASSERT_ErRrtv ( szPath == NULL, "InitStorageN: Failed to allocate local pathname orcalled with null-pointer argument!" );

			InitStorageUtil ( szPath );

			RELEASE_PCHAR ( szPath, path );
		}


		/**
		* Method:    InitWorkDir
		* Signature: (Ljava/lang/String;)V
		*/
		ENVIRONSAPI void EnvironsFunc ( InitWorkDirN, jstring path )
        {
            CVerbVerb ( "InitWorkDirN" );
            
			INIT_PCHAR ( szPath, path );

			if ( szPath == NULL ) {
				CErr ( "InitWorkDirN: Failed to allocate local pathname or called with null-pointer argument!" );
				return;
			}
            
            char * workPath = strdup ( szPath );
            if ( !workPath ) {
                CErr ( "InitWorkDirN: Failed to allocate memory for new storagePath!" );
            }
            else {
                native.SetWorkDir ( workPath );
                
                CVerbArg ( "InitWorkDirN: Working directory [%s]", workPath );
            }
            
			RELEASE_PCHAR ( szPath, path );
        }
        
        
        /**
         * Method:    InitLibDir
         * Signature: (Ljava/lang/String;)V
         */
        ENVIRONSAPI void EnvironsFunc ( InitLibDirN, jstring path )
        {
            CVerbVerb ( "InitLibDirN" );
            
            if ( !path ) {
                native.libDir = native.workDir;
                return;
            }
            
            INIT_PCHAR ( szPath, path );
            
            if ( szPath == NULL ) {
                CErr ( "InitLibDirN: Failed to allocate local pathname or called with null-pointer argument!" );
                return;
            }
            
            char * workPath = strdup ( szPath );
            if ( !workPath ) {
                CErr ( "InitLibDirN: Failed to allocate memory for new libPath!" );
            }
            else {
                if ( native.libDir && native.libDir != native.workDir ) {
                    free ( native.libDir );
                }
                native.libDir = workPath;
                
                CVerbArg ( "InitLibDirN: Library directory [%s]", workPath );
            }
            
            RELEASE_PCHAR ( szPath, path );
        }


        /*
         * Method:    GetDeviceWidth
         * Signature: (I)I
         */
        ENVIRONSAPI jint EnvironsFunc ( GetDeviceWidthN, jint nativeID )
        {
            DeviceController * device = (DeviceController *)environs::GetDevice ( nativeID );
            
            if ( !device )
                return 0;
            
            int width = device->display.width;
            
            UnlockDevice ( device );
            return width;
        }
        
        
        /**
         * Method:    GetDeviceHeight
         * Signature: (I)I
         */
        ENVIRONSAPI jint EnvironsFunc ( GetDeviceHeightN, jint nativeID )
        {
            DeviceController * device = (DeviceController *)environs::GetDevice ( nativeID );
            
            if ( !device )
                return 0;
            
            int height = device->display.height;
            
            UnlockDevice ( device );
            return height;
        }

        
        /*
         * Method:    DeviceDisconnectN
         *
         * @param	nativeID    The native identifier that targets the device.
         * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * Signature: (I)Z
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( DeviceDisconnectN, int hInst, jint nativeID, int async )
        {
            CVerbIDN ( "DeviceDisconnectN" );
            
            if ( nativeID <= 0 ) {
                CErrIDN ( "DeviceDisconnectN: Can not dispose a native id <= 0" );
                return false;
            }
            
            EBOOL success = false;
            
            Instance * env = GetStartedInstance ( hInst );
			if ( !env )
				return false;
            
            if ( async )
                success = (env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_DEVICE_DISCONNECT, 0, 1, 1, 0 ) != 0);
            else {
                success = RemoveDevice ( nativeID );
            
				if ( env->environsState >= environs::Status::Connected && GetConnectedDevicesManagedCount ( ) == 0 )
				{
					SetEnvironsState ( env, environs::Status::Started );
				}
			}
            return success;
        }
        
        
        /**
         * Connect to device with the given ID and a particular application environment. Return value is of type enum Types.DeviceStatus
         *
         * @param	deviceID	Destination device ID
         * @param	areaName	Area name of the application environment
         * @param	appName		Application name of the application environment
         * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param 	x
         * @param 	y
         * @param 	angle
         * @return	0	Connection can't be established (maybe environs is stopped or the device ID is invalid)
         * @return	1	A new connection has been triggered and is in progress
         * @return	2	A connection to the device already exists or a connection task is already in progress)
         */
		ENVIRONSAPI int DeviceDetectedN ( int hInst, int deviceID, const char * areaName, const char * appName, int Environs_CALL_, int x, int y, float angle )
        {
            CVerbID ( "DeviceDetectedN" );
            
			sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
			if ( !envSP )
				return 0;

            if ( Environs_CALL_ )
				return envSP->asyncWorker.Push ( deviceID, ASYNCWORK_TYPE_DEVICE_CONNECT, areaName, appName, 0, x, y, angle );
            else {
				return DeviceController::DeviceDetected ( hInst, CALL_WAIT, deviceID, areaName, appName, x, y, angle );
            }
        }
        
        
        /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
         Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
        ENVIRONSAPI int AllowConnectN ( int hInst, int objID, int value )
        {
            sp ( DeviceInstanceNode ) device = MediatorClient::GetDeviceSP ( hInst, objID );
            if ( device )
            {
                if ( value < 0 ) {
                    return (device->allowConnect ? 1 : 0);
                }
                
                // Set the value
                device->allowConnect = (value != 0);
            }
            return 1;
        }
        
        
        /** Default value for each DeviceInstance after object creation. */
        ENVIRONSAPI int AllowConnectDefaultN ( int hInst, int value )
        {
            sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
            if ( envSP ) {
                if ( value < 0 )
                    return ( envSP->allowConnectDefault ? 1 : 0);
                
                envSP->allowConnectDefault = (value != 0);
            }
            return 1;
        }
        
        
        /**
         * Connect to device with the given ID and a particular application environment. Return value is of type enum Types.DeviceStatus
         *
         * @param	deviceID	Destination device ID
         * @param	areaName	Area name of the application environment
         * @param	appName		Application name of the application environment
         * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @return	0	Connection can't be established (maybe environs is stopped or the device ID is invalid)
         * @return	1	A new connection has been triggered and is in progress
         * @return	2	A connection to the device already exists or a connection task is already in progress)
         */
		ENVIRONSAPI int EnvironsFunc ( DeviceConnectN, int hInst, int deviceID, jstring areaName, jstring appName, int async )
        {
            CInfosID ( 2, "DeviceConnectN" );

			sp ( Instance ) envSP = GetStartedInstanceSP ( hInst );
			if ( !envSP )
				return 0;

			INIT_PCHAR ( szAreaName, areaName );
            INIT_PCHAR ( szAppName, appName );
            
            int success = 0;
            
            if ( async )
				success = envSP->asyncWorker.Push ( deviceID, ASYNCWORK_TYPE_DEVICE_CONNECT, szAreaName, szAppName, 0, 1, 1, -1 );
            else
				success = DeviceController::DeviceDetected ( hInst, CALL_WAIT, deviceID, szAreaName, szAppName, 1, 1, -1 );
            
			RELEASE_PCHAR ( szAreaName, areaName );
            RELEASE_PCHAR ( szAppName, appName );
            return success;
        }
        
        
		/*
         * Method:    GetUseStream
         * Signature: ()Z
         */
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseStreamN, int hInst )
		{
			return (instances[hInst]->useStream ? 1 : 0);
		}
        
        
		/*
         * Method:    SetUseStream
         * Signature: (Z)Z
         */
		ENVIRONSAPI void EnvironsFunc ( SetUseStreamN,  jint hInst, EBOOL enable )
		{
			CVerb ( "SetUseStreamN" );
            
            Instance * env = instances[hInst];

#ifdef DISPLAYDEVICE
            
			bool status = enable ? true : false;
            
			env->useStream = status;
            
            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, status ? "Enabled Stream" : "Disabled Stream" );
#else
			env->useStream = (enable ? true : false);

			env->optBool ( APPENV_SETTING_USE_STREAM, enable );
#endif
		}


		ENVIRONSAPI void EnvironsFunc ( SetAppStatusN, jint hInst, jint status )
		{
			CVerb ( "SetAppStatusN" );

			Instance * env = instances [ hInst ];

			if ( env->kernel )
				env->kernel->SetAppStatus ( status );
		}


		/**
		* Instruct Environs native layer to prepare required portal resources to base on generation within the platform layer.
		*
		* @param enable      true = enable, false = disable
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUsePlatformPortalGeneratorN, jint hInst, EBOOL enable )
		{
			CVerbArg ( "SetUsePlatformPortalGeneratorN: %i", enable );

			instances[hInst]->usePlatformPortalGenerator = (enable ? true : false);
		}
        
        
        
        ENVIRONSAPI EBOOL GetTrackerEnabledN ( jint hInst, jint index )
        {
            CVerb ( "GetTrackerEnabledN" );
            
#ifdef DISPLAYDEVICE
            if ( instances[hInst]->kernel )
                return (instances[hInst]->kernel->GetTracker ( index ) != 0);
#endif
            return false;
        }
        
        
        int EnvironsFunc ( ApplyTrackerN, int hInst, int async, jstring moduleName, int action )
        {
            int success = 0;
            
            INIT_PCHAR ( szModuleName, moduleName );
            
            Instance * env = instances[hInst];
            
            if ( async && env->asyncWorker.GetIsActive () )
                success = env->asyncWorker.Push ( 0, ASYNCWORK_TYPE_TRACKER_USAGE, szModuleName, 0, action, 0, 0, 0 );
            else
                success = env->asyncWorker.TrackerUsage ( szModuleName, action );
            
            RELEASE_PCHAR ( szModuleName, moduleName );
            
            return success;
        }
        
        
        ENVIRONSAPI int EnvironsFunc ( SetUseTrackerN, jint hInst, jint async, jstring moduleName )
        {
            CVerb ( "SetUseTrackerN" );
            
            return EnvironsCallArg ( ApplyTrackerN, hInst, async, moduleName, 1 );
        }
        
        
        ENVIRONSAPI int EnvironsFunc ( GetUseTrackerN, jint hInst, jstring moduleName )
        {
            CVerb ( "GetUseTrackerN" );
            
            return EnvironsCallArg ( ApplyTrackerN, hInst, CALL_WAIT, moduleName, 2 );
        }
        
        
        ENVIRONSAPI EBOOL EnvironsFunc ( DisposeTrackerN, jint hInst, jint async, jstring moduleName )
        {
            CVerb ( "DisposeTrackerN" );
            
            return (EnvironsCallArg ( ApplyTrackerN, hInst, async, moduleName, 0 ) != 0);
        }
        
        
        ENVIRONSAPI EBOOL EnvironsFunc ( PushTrackerCommandN, jint hInst, jint async, jint index, jint command )
        {
            CVerb ( "PushTrackerCommandN" );
            
            EBOOL success = 0;
            
            Instance * env = instances[hInst];
			if ( !env || env->environsState < environs::Status::Starting )
				return 0;

            if ( async )
                success = (env->asyncWorker.Push ( 0, ASYNCWORK_TYPE_TRACKER_COMMAND, index, command, 0 ) != 0);
            else
                success = env->asyncWorker.TrackerCommand ( index, command );
            
            return success;
        }
        
        
        
        /*
         * Method:    SetPortalNativeResolutionN
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetPortalNativeResolutionN, jint hInst, EBOOL enable )
        {
			Instance * env = instances [ hInst ];

			env->useNativeResolution = (enable != 0);
            
#ifndef DISPLAYDEVICE
			env->optBool ( APPENV_SETTING_USE_NATIVE_RESOLUTION, enable );
#endif
        }
        
        /*
         * Method:    GetPortalNativeResolutionN
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalNativeResolutionN, jint hInst )
        {
            return instances[hInst]->useNativeResolution;
        }
        
        
        /*
         * Method:    SetPortalAutoStartN
         * Signature: (Z)Z
         */
        ENVIRONSAPI void EnvironsFunc ( SetPortalAutoStartN, jint hInst, EBOOL enable )
        {
			Instance * env = instances [ hInst ];

			env->usePortalAutoStart = (enable != 0);
            
#ifndef DISPLAYDEVICE	
			env->optBool ( APPENV_SETTING_USE_PORTAL_AUTOSTART, enable );
#endif
        }
        
        
        /*
         * Method:    GetPortalAutoStartN
         * Signature: ()Z
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalAutoStartN, jint hInst )
        {
            return instances[hInst]->usePortalAutoStart;
        }
	}
}

#endif


