/**
 * Environs object
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
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#ifndef ENVIRONS_NATIVE_MODULE
#	define ENVIRONS_NATIVE_MODULE
#endif

#include "Environs.Cli.Forwards.h"
#include "Environs.h"
#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Interop/Threads.h"
#include "Environs.Types.h"

#ifndef CLI_CPP
#	include "Environs.Obj.h"
#	include "Environs.Observer.h"
#	include "Device.List.h"

#	include <vector>
	using namespace std;

#	ifdef _WIN32
#		include <OleCtl.h>
#	endif
#else

#	include "Device/Device.List.Cli.h"
#	include "Device/Device.Instance.Cli.h"
#	include "Core/File.Instance.Cli.h"
#	include "Core/Message.Instance.Cli.h"
#endif

#include "Environs.Native.h"
#include "Portal.Instance.h"
#include "Environs.Sensors.h"
#include "Tracer.h"
#include "Core/Wifi.List.h"
#include "Core/Bt.List.h"


using namespace environs;
using namespace environs::API;

#define CLASS_NAME	"Environs . . . . . . . ."


namespace environs
{
#if !defined(CLI_CPP) && !defined(USE_INSTANCE_OBJ_LOCK)
    pthread_mutex_t objLock;
#endif
  
    extern void InvokeNetworkNotifier ( int hInst, bool enable );
    extern void EnvironsPlatformInit ( int hInst );
    
#ifdef ENVIRONS_OSX
    extern void AddKeyMonitor ();
#endif
    
    namespace API
    {        
        extern bool RenderSurfaceCallback ( int type, void * surface, void * decoderOrByteBuffer );
    }


    namespace lib
    {
        /** An identifier that is unique for every created object within the Environs native layer. */        
		LONGSYNC  objectIdentifiers       = 1;

#ifndef CLI_CPP
        Environs * Environs::instancesAPI [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];

        bool Environs::objetAPIInit = false;
#endif


		bool Environs::ObjectAPIInit ()
		{
			CVerb ( "ObjectAPIInit" );

			if ( objetAPIInit )
				return true;

			if ( !DeviceInstance::GlobalsInit () )
				return false;

			if ( !PortalInstance::GlobalsInit () )
                return false;

#if !defined(CLI_CPP) && !defined(USE_INSTANCE_OBJ_LOCK)
            if ( !LockInitA ( environs::objLock ) )
                return false;
#endif
			objetAPIInit = true;

			return true;
		}


		void Environs::ObjectAPIDispose ()
		{
			CVerb ( "ObjectAPIDispose" );

			if ( !objetAPIInit )
                return;
            
#if !defined(CLI_CPP) && !defined(USE_INSTANCE_OBJ_LOCK)
            LockDisposeA ( environs::objLock );
#endif
			PortalInstance::GlobalsDispose ();

			DeviceInstance::GlobalsDispose ();

			objetAPIInit = false;
		}
        
        
        /**
         * Release memory allocated by Environs to be temporarily used by client code.
         *
         * @param		mem
         */
        void Environs::ReleaseEnvMemory ( Addr_obj mem )
        {
            environs::API::ReleaseStringN ( Addr_pvalued ( mem ) );
        }
        

#ifdef ENVIRONS_IOS

		class PortalRequestContext
		{
		public:
			sp ( environs::lib::DeviceInstance ) device;
		};

		void * CreatePortalRequestThread ( void * pack )
		{
			CVerbVerb ( "CreatePortalRequestThread" );

			pthread_setname_current_envthread ( "Environs.CreatePortalRequestThread" );

			PortalRequestContext * ctx = ( PortalRequestContext * ) pack;

			//[Environs requestPortalStream:deviceID Area:areaName App:appName doAsync:CALL_ASYNC withType:environs::PortalType::Any];

            if ( !ctx->device->info ()->isConnected && !ctx->device->Connect ( Call::Wait ) ) {
				CErr ( "CreatePortalRequestThread: Failed to connect to device." );
			}
			else {
				sp ( environs::PortalInstance ) portal = ctx->device->PortalRequest ( PortalType::Any );
				if ( portal != nill ) {
					portal->Establish ( true );
				}
			}

			delete ctx;
			return 0;
		}


		void CreatePortalRequest ( const DeviceInstanceSP &device )
		{
			PortalRequestContext OBJ_ptr ctx = new PortalRequestContext;
			if ( ctx == nill ) {
				return;
			}

			ctx->device = device;

			pthread_t threadID;
			int ret = pthread_create ( &threadID, NULL, &CreatePortalRequestThread, ( void * ) ctx );
			if ( ret != 0 ) {
				CErr ( "CreatePortalRequest: Failed to create handler thread." );
				delete ctx;
                return;
			}
            
            pthread_detach_handle ( threadID );
		}
#endif

        /**
         * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
         * 		such as when a connection has been established or closed.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param notification  The received notification.
         * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
         * @param context       A value that provides additional context information (if available).
         */
		void Environs::BridgeForNotify ( int hInst, OBJIDType objID, int notification, int sourceIdent, Addr_obj contextPtr, int context )
		{
			EnvironsOBJ api = EnvironsOBJInst [hInst];

			if ( api == nill )
				return;

#ifndef CLI_CPP
            vct ( lib::IIEnvironsObserver OBJ_ptr ) OBJ_ptr observers = sp_get ( api->environsObservers );
            
            vct ( lib::IIEnvironsObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForNotify" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForNotify" );
            
            size = obss.size ();
#endif
            
			if ( IsStatus ( notification ) ) {
#ifdef CLI_CPP
				if ( api->OnStatus != nill ) {
					int status = GetStatusValue ( notification );
                    
                    if ( status == STATUS_STARTED ) {
                        api->ReloadLists ();
                    }
                    
					api->OnStatus ( ( Status ) status );
				}
#else
                int status = GetStatusValue ( notification );
                
                if ( status == STATUS_STARTED ) {
                    api->ReloadLists ();
                }
                
				if ( size > 0 )
                {
					for ( size_t i=0; i < size; i++ )
					{
						lib::IIEnvironsObserver * obs =  obss.at ( i );
                        if ( obs ) {
							try {
								if ( obs->OnEnvironsStatus_ ) obs->OnStatus ( (Status_t) status );
							}
                            catch ( ... ) {
                                CErr ( "BridgeForNotify: Exception!" );
                            }
						}
                    }
                }
#endif
				return;
			}

			environs::ObserverNotifyContext ctx;

			ctx.hEnvirons		= hInst;
			ctx.destID	        = objID;
			ctx.notification    = notification;
			ctx.sourceIdent     = sourceIdent;
            ctx.contextPtr      = contextPtr;
            ctx.context         = context;
            ctx.destID	        = objID;
            ctx.appName         = nill;
            ctx.areaName        = nill;
            
			CVerbVerbArg2 ( "Notify", "nativeID", "i", objID, "", "s", CPP_CLI ( resolveName ( notification ), CCharToString ( environs::API::ResolveNameN ( notification ) ) ) );

            int notifType = notification & MSG_NOTIFY_CLASS;
            
            if ( notifType == NOTIFY_TYPE_CONNECTION )
            {
                if ( notification == NOTIFY_CONNECTION_PROGRESS ) {
					api->UpdateConnectProgress ( objID, ( int ) sourceIdent );
                }
                
#ifdef ENVIRONS_IOS
                else if ( notification == NOTIFY_CONNECTION_ESTABLISHED_ACK ) {
                    if ( observers->size () && environs::API::GetPortalAutoStartN ( hInst ) ) {
                        
                        DeviceInstanceSP device = nill;

                        api -> GetDeviceAll ( device, objID );

                        if ( device == nill )
                            return;
                        CreatePortalRequest ( device );
                    }
                }
#endif
            }
            else if ( notifType == NOTIFY_TYPE_PORTAL )
            {
#ifdef __APPLE__
                if ( notification == NOTIFY_PORTAL_STREAM_INCOMING || notification == NOTIFY_PORTAL_IMAGES_INCOMING )
                {
					environs::API::SetRenderCallbackN ( hInst, CALL_NOWAIT, sourceIdent, ( void * ) RenderSurfaceCallback, RENDER_CALLBACK_TYPE_ALL );
                }
#endif
				PortalInstance::Update ( hInst, Addr_of ( ctx ) );
            }
            else if ( notifType == NOTIFY_TYPE_FILE )
            {
                api->OnDeviceListNotification ( OBJ_ref ctx );
            }
            else if ( notifType == NOTIFY_TYPE_ENVIRONS )
            {
                if ((notification & NOTIFY_MEDIATOR) == NOTIFY_MEDIATOR)
                {
                    switch(notification) {
                        case NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE:
                            api ->OnDeviceListUpdate ();
                            break;
                            
                        case NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL:
                        case NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING:
							
                            if ( api->GetUseMediatorLoginDialog () && !api->GetUseMediatorAnonymousLogon() && api->GetStatus () >= Status::Started )
                            {
                                const char * t = environs::API::GetMediatorUserNameN ( hInst );

                                CString_ptr userName = ( t != nill ? CCharToString ( t ) : "" );
                                
                                environs::API::ShowLoginDialogN ( hInst, userName );
                            }
                            break;
                            
                        default:
                            DeviceList::OnDeviceListNotification ( hInst, Addr_of ( ctx ) );
                    }
                }
                else if ( notification == NOTIFY_DEVICE_ON_SURFACE || notification == NOTIFY_DEVICE_NOT_ON_SURFACE ) {
					PortalInstance::UpdateOptions ( Addr_of ( ctx ) );
                }
				else if ( notification == NOTIFY_DEVICE_FLAGS_UPDATE ) 
				{
					/**
					* Update flags from mediator daemon by AsyncWorker which changed them to CPTypes of DeviceFlagsInternal
					*
					*/
					DeviceInstanceESP device = nill; api->GetDeviceByObjectID ( device, objID );
					if ( device ) {
						if ( context )
							device->info_->flags |= ( unsigned short ) sourceIdent;
						else
							device->info_->flags &= ~( ( unsigned short ) sourceIdent );

						device->NotifyObservers ( ( int ) DeviceInfoFlag::Flags, true );
					}
#if !defined(NDEBUG) && !defined(CLI_CPP)
                    else {
                        CVerbArg ( "BridgeForNotify: Device for flags update not found. objID [ %i ]", objID  );
                    }
#endif
				}
            }
            else if ( notifType == NOTIFY_TYPE_OPTIONS )
            {
                if ( notification == NOTIFY_CONTACT_DIRECT_CHANGED )
                    api -> OnDeviceListNotification ( Addr_of ( ctx ) );
				else
					PortalInstance::UpdateOptions ( Addr_of ( ctx ) );
            }

#ifdef CLI_CPP  
			if ( api->OnNotify == nill )
				return;
			api->OnNotify ( Addr_of ( ctx ) );
#else
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsObserver * obs =  obss.at ( i );
                if ( obs )
                {
                    try {
						if ( obs->OnEnvironsNotify_ ) obs->OnNotify ( Addr_of ( ctx ) );
                    }
                    catch(...) {
                        CErr ( "BridgeForNotify: Exception!" );
                    }
                }
            }
#endif
		}
        
        /**
         * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
         * 		such as when a connection has been established or closed.
         *
         * @param hInst			A handle to the Environs instance
         * @param deviceID      The device id of the sender device.
         * @param areaName		Area name of the application environment.
         * @param appName		Application name of the application environment.
         * @param notification  The received notification.
         * @param sourceIdent   A value of the enumeration type Types.Source
         * @param context       A value that provides additional context information (if available).
         */
        void Environs::BridgeForNotifyExt ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int notification, int sourceIdent, Addr_obj contextPtr )
        {
            CVerbArg2 ( "NotifyExt:", "deviceID", "X", deviceID, "", "s", CPP_CLI ( resolveName ( notification ), CCharToString ( environs::API::ResolveNameN ( notification ) ) ) );


			environs::ObserverNotifyContext ctx;

			ctx.destID			= deviceID;
			ctx.areaName		= areaName;
			ctx.appName			= appName;
			ctx.notification    = notification;
			ctx.sourceIdent     = sourceIdent;
			ctx.contextPtr      = contextPtr;

			if ( (notification & NOTIFY_MEDIATOR) == NOTIFY_MEDIATOR || notification == MSG_HANDSHAKE_MAIN_FAIL )
			{
				DeviceList::OnDeviceListNotification ( hInst, Addr_of ( ctx ) );
			}
            
            EnvironsOBJ api = EnvironsOBJInst [hInst];
            
#ifdef CLI_CPP
			if ( api == nill || api->OnNotifyExt == nill )
				return;
			api->OnNotifyExt ( Addr_of ( ctx ) );
#else

            std::vector < lib::IIEnvironsObserver * > * observers = instancesAPI [ hInst ]->environsObservers.get ();
            
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForNotifyExt" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForNotifyExt" );
            
            size = obss.size ();
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
						if ( obs->OnEnvironsNotifyExt_ ) obs->OnNotifyExt ( Addr_of ( ctx ) );
                    }
                    catch(...) {
                        CErr ( "BridgeForNotifyExt: Exception!" );
                    }
                }
            }
#endif
        }
        
        
        /**
         * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param deviceID      The device id of the sender device.
         * @param areaName		Area name of the application environment
         * @param appName		Application name of the application environment
         * @param sourceType    The type of this message.
         * @param msg           The message.
         * @param length        The length of the message.
         */
		void Environs::BridgeForMessageExt ( int hInst, int deviceID, CString_ptr areaName, CString_ptr appName, int sourceType, CVString_ptr message, int length )
		{
#ifdef ENABLE_MESSAGE_EXT_HANDLER
			EnvironsOBJ api = EnvironsOBJInst [hInst];
			if ( api == nill )
				return;

			environs::ObserverMessageContext ctx;
			ctx.destID      = deviceID;
			ctx.areaName    = areaName;
			ctx.appName     = appName;
			ctx.sourceType  = sourceType;
			ctx.message     = (CString_ptr) message;
			ctx.length      = length;
			ctx.connection  = 'u';

			api->UpdateMessage ( Addr_of ( ctx ) );

#   ifdef CLI_CPP
			if ( api->OnMessageExt != nill )
				api->OnMessageExt ( Addr_of ( ctx ) );
#   else
#       ifndef NDEBUG
			int c = -1;
			char * m = ( ( char * ) message );

			if ( length > 64 ) {
				c = m [ 64 ]; m [ 64 ] = 0;
			}
            CVerbVerbArg ( "onMessage:   deviceID [%i]\t[%s]", deviceID, message );
			if ( c != -1 ) {
				m [ 64 ] = (char) c;
			}
#       endif
            std::vector < lib::IIEnvironsMessageObserver * > * observers = api->environsObserversForMessages.get ();
            
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsMessageObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForMessageExt" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsMessageObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForMessageExt" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsMessageObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
						if ( obs->OnEnvironsMessageExt_ ) obs->OnMessageExt ( Addr_of ( ctx ) );
                    }
                    catch(...) {
                        CErr ( "BridgeForMessageExt: Exception!" );
                    }
                }
            }
#   endif
#endif
        }
        
        
        /**
         * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param sourceType    The type of this message.
         * @param msg           The message.
         * @param length        The length of the message.
         */
		void Environs::BridgeForMessage ( int hInst, OBJIDType objID, int sourceType, CVString_ptr message, int length )
		{
			EnvironsOBJ api = EnvironsOBJInst [hInst];
			if ( api == nill )
				return;
            
			environs::ObserverMessageContext ctx;
			ctx.destID      = objID;
			ctx.message     = (CString_ptr) message;
			ctx.length      = length;
			ctx.sourceType	= sourceType;
			ctx.connection  = 'c';

			if ( objID != 0 && sourceType == ( int ) Source::Device )
				api->UpdateMessage ( Addr_of ( ctx ) );

#ifdef CLI_CPP
			if ( api -> OnMessage != nill )
				api->OnMessage ( Addr_of ( ctx ) );
#else
#ifndef NDEBUG
			int c = -1;
			char * m = ( ( char * ) message );

			if ( length > 64 ) {
				c = m [ 64 ]; m [ 64 ] = 0;
			}
			CVerbArg ( "BridgeForMessage:   objID [ %i ]\t[ %s ]", objID, message );
			if ( c != -1 ) {
				m [ 64 ] = ( char ) c;
			}
#endif
            std::vector < lib::IIEnvironsMessageObserver * > * observers = api->environsObserversForMessages.get ();
            
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsMessageObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForMessage" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsMessageObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForMessage" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsMessageObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
						if ( obs->OnEnvironsMessage_ ) obs->OnMessage ( Addr_of ( ctx ) );
                    }
                    catch(...) {
                        CErr ( "BridgeForMessage: Exception!" );
                    }
                }
            }
#endif
        }
        
        
        /**
         * BridgeForStatusMessage static method to be called by native layer in order to drop a status messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param msg           A status message of Environs.
         */
        void Environs::BridgeForStatusMessage ( int hInst, CString_ptr message )
        {
            CVerbVerb ( "BridgeForStatusMessage" );
            
            EnvironsOBJ api = EnvironsOBJInst [hInst];
			if ( api == nill )
				return;
#ifdef CLI_CPP
			if ( api -> OnStatusMessage != nill )
				api->OnStatusMessage ( message );
#else
            std::vector < lib::IIEnvironsMessageObserver * > * observers = instancesAPI [ hInst ]->environsObserversForMessages.get ();
            
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsMessageObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForStatusMessage" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsMessageObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForStatusMessage" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsMessageObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
                        if ( obs->OnEnvironsStatusMessage_ ) obs->OnStatusMessage ( message );
                    }
                    catch(...) {
                        CErr ( "BridgeForStatusMessage: Exception!" );
                    }
                }
            }
#endif
        }
        
        
        /**
         * BridgeForData static method to be called by native layer in order to notify about data received from a device.
         *
		 * @param hInst			A handle to the Environs instance
		 * @param objID			The object id of the sender device.
		 * @param nativeID		The native device id of the sender device.
         * @param type          The type of this message.
         * @param fileID        A fileID that was attached to the buffer.
         * @param descriptor    A descriptor that was attached to the buffer.
         * @param size          The size of the data buffer.
         */
        void Environs::BridgeForData ( int hInst, OBJIDType objID, int nativeID, int type, int fileID, CString_ptr descriptor, int size )
        {
			CVerbVerb ( "BridgeForData" );

			EnvironsOBJ api = EnvironsOBJInst [ hInst ];
			if ( api == nill )
				return;

            environs::ObserverDataContext ctx;
            ctx.objID       = objID;
			ctx.nativeID    = nativeID;
            ctx.type		= type;
            ctx.fileID		= fileID;
            ctx.descriptor	= descriptor;
            ctx.size		= size;

			api->UpdateData ( Addr_of ( ctx ) );

#ifdef CLI_CPP
			if ( api->OnData != nill )
				api->OnData ( Addr_of ( ctx ) );
#else            
            std::vector < lib::IIEnvironsDataObserver * > * observers = api->environsObserversForData.get ();
                        
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsDataObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForData" );
            
            size_t count = observers->size ();
            
            for ( size_t i = 0; i < count; ++i )
            {
                lib::IIEnvironsDataObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForData" );
            
            count = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < count; i++ )
            {
				lib::IIEnvironsDataObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
						obs->OnData ( Addr_of ( ctx ) );
                    }
                    catch(...) {
                        CErr ( "BridgeForData: Exception!" );
                    }
                }
            }
#endif
		}

        
        bool GetUdpDataContext ( Addr_obj pack, UdpDataContext OBJ_ref ctx )
        {
            if ( ctx.size <= 0 )
                return false;

			char * data = ( char * ) Addr_pvalued ( pack );
			if ( data == nill )
				return false;
            
            if ( data [ 0 ] == 's' && data [ 1 ] == 'f' && data [ 2 ] == ':' )
            {
                ctx.sensorFrame = GetSensorInputPack ( pack );
#ifdef CLI_CPP
                if ( ctx.sensorFrame == nill )
                    return false;
#endif
                ctx.dataPtr     = nill;
            }
            else {
                ctx.sensorFrame = nill;
                ctx.dataPtr     = pack;
            }
            
            return true;
        }


        /**
         * BridgeForUdpData static method to be called by native layer in order to notify about udp data received from a device.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param pack          A udp data structure containing the received udp or sensor data.
         * @param packSize      The size of the data buffer in number of bytes.
         */
        void Environs::BridgeForUdpData ( int hInst, OBJIDType objID, Addr_obj pack, int packSize )
		{
			EnvironsOBJ api = EnvironsOBJInst [ hInst ];
			if ( api == nill )
				return;

            UdpDataContext ctx;
            
            ctx.size = packSize;
            
            if ( !GetUdpDataContext ( pack, ctx ) )
                return;
            
            ctx.objID = objID;
            
            environs::SensorFrame OBJ_ptr frame = nill;
            
#ifndef CLI_CPP
            environs::DeviceInstance    * device = nill;
            void                        * devicePlatform = nill;
            
            if ( ctx.sensorFrame )
            {
                frame = ctx.sensorFrame;
                
                // Backup the first values of the next frame (Do we overflow the buffer here???)
                device            = frame->device;
                devicePlatform    = frame->devicePlatform;

                frame->frame.type &= ~ENVIRONS_SENSOR_PACK_TYPE_EXT;
            }
#endif
            if ( ctx.objID )
                api->UpdateUdpData ( Addr_of ( ctx ) );

#ifdef CLI_CPP
			frame = ctx.sensorFrame;
#endif
            if ( !frame )
                return;
#ifdef CLI_CPP
			if ( api -> OnSensorInput != nill )
				api -> OnSensorInput ( objID, frame );
#else            
            std::vector < lib::IIEnvironsSensorObserver * > * observers = api -> environsObserversForSensor.get ();
            
            if ( observers->size () <= 0 )
                return;
            
            vct ( lib::IIEnvironsSensorObserver * ) obss;
            
            LockAcquireVA ( api->queryLock, "BridgeForUdpData" );
            
            size_t size = observers->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIEnvironsSensorObserver OBJ_ptr obs = observers->at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( api->queryLock, "BridgeForUdpData" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
				lib::IIEnvironsSensorObserver * obs =  obss.at ( i );
                if ( obs ) {
                    try {
						obs->OnSensorData ( objID, frame );
                    }
                    catch(...) {
                        CErr ( "BridgeForUdpData: Exception!" );
                    }
                }
            }
			 
			// Restore the first values of the next frame (Do we overflow the buffer here???)
            frame->device           = device;
            frame->devicePlatform   = devicePlatform;
#endif
        }


		ENVIRONS_OUTPUT_ALLOC ( Environs );
      
        
        /**
         * Constructor.
         *
         */
        Environs::Environs ()
        {
			EnvironsPlatformInit ( hEnvirons );
            
            hEnvirons   = 0;
            allocated   = false;
            isUIAdapter = false;
            
            listCommandThreadRun    = false;
            deviceNotifierThreadRun = false;
            allowConnectDefault     = true;
            
            async       = Call::NoWait;

			pthread_reset ( startStopThread );

			ENVIRONS_OUTPUT_ALLOC_INIT ();
            
            CVerb ( "Construct" );
        }
        
        
        /**
         * Destructor.
         *
         */
        Environs::~Environs ()
        {
            CVerb ( "Destruct" );
            
            allocated = false;

			DisposeInstance ();
            
            listCommandThreadRun = false;
            if ( listCommandThread.isRunning () )
            {
                listCommandThread.Notify ( "Destruct", true );
                listCommandThread.Join ( "Destruct: Environs.listCommandThread" );
            }
            
            // We need to make sure, that each DeviceInstance has notified the client code about its disposal
            // Otherwise client code may produce memory leaks
            deviceNotifierThreadRun = false;
            if ( deviceNotifierThread.isRunning () )
            {
                deviceNotifierThread.Notify ( "Destruct", true );
                deviceNotifierThread.Join ( "Destruct: Environs.listCommandThread" );
            }
            
			ListCommandQueueClear ();

            // There should not be any device notifications in the queue anymore.
            // Anyway, let's check and make sure that we don't leak memory.
			DeviceNotifierQueueClear ();
            
            LockDisposeA ( listLock );
            LockDisposeA ( queryLock );
            LockDisposeA ( listAllLock );
            LockDisposeA ( listNearbyLock );
            LockDisposeA ( listMediatorLock );
            LockDisposeA ( deviceNotifierLock );
            LockDisposeA ( listCommandThreadLock );
            
            LockDisposeA ( contextAll.lock );
            LockDisposeA ( contextNearby.lock );
            LockDisposeA ( contextMediator.lock );

            ENVIRONS_OUTPUT_DISPOSE_OBJLOCK ();
        }
        
        
        void Environs::DisposeDevice ( DeviceInstanceReferenceSP device )
        {
            device->DisposeInstance ();
            
            // Make sure that client code receives a dispose notification.
            device->NotifyObservers ( ENVIRONS_OBJECT_DISPOSED, false );
            
            device->PlatformDispose ();
        }
        
        
        void Environs::DisposeListContainer ( NLayerVecType ( DeviceInstanceEP ) c_ref srcSP )
        {
            NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr list = sp_get ( srcSP );
            
            for ( size_t i=0; i < vp_size ( list ); ++i )
            {
				DeviceInstanceReferenceSP device = vp_at ( list, i );
                
                if ( device == nill || device->disposed_ )
                    continue;
                
                DisposeDevice ( device );
            }
            ContainerClear ( list );
        }
        
        
        void Environs::DisposeListCommandContext ( ListContext OBJ_ptr ctx )
        {
            if ( LockAcquire ( c_Addr_of ( ctx->lock ), "DisposeListCommandContext" ) )
            {
                if ( ctx->vanished != nill && vp_size ( ctx->vanished ) > 0 )
                {
                    DisposeListContainer ( ctx->vanished );
                }

                if ( ctx->appeared != nill && vp_size ( ctx->appeared ) > 0 )
                {
                    DisposeListContainer ( ctx->appeared );
                }
                
                LockReleaseV ( c_Addr_of ( ctx->lock ), "DisposeListCommandContext" );
            }
        }
        

		void Environs::ListCommandQueueClear ()
		{
			CVerb ( "ListCommandQueueClear" );

			LockAcquireVA ( listCommandThreadLock, "ListCommandQueueClear" );

			while ( envQueue_nempty ( listCommandQueue ) )
			{
				ListCommandContextPtr ctx = C_Only ( ( ListCommandContextPtr ) ) envQueue_front ( listCommandQueue );
				envQueue_pop ( listCommandQueue );
                
                if ( ctx != nill )
                {
                    ctx->device = nill;

                    CheckListCommandContexts ( 0, ctx, true );
                    
					delete__obj_n ( ctx );
                }
			}

			LockReleaseVA ( listCommandThreadLock, "ListCommandQueueClear" );
            
            DisposeListCommandContext ( Addr_of ( contextAll ) );
            DisposeListCommandContext ( Addr_of ( contextMediator ) );
            DisposeListCommandContext ( Addr_of ( contextNearby ) );
        }
        

        void Environs::DisposeNotifierContext ( DeviceNotifierContextPtr c_ref ctx )
        {
            if ( ctx->message != nill && !ctx->message->disposed_ )
            {
                ctx->message->DisposeInstance ();
            }
            
            if ( ctx->fileData != nill && !ctx->fileData->disposed_ )
            {
                ctx->fileData->DisposeInstance ();
            }
            
            if ( ctx->device != nill && !ctx->device->disposed_ )
            {
                DisposeDevice ( ctx->device );
            }
        }
        
        
        void Environs::DeviceNotifierQueueClear ()
        {
            CVerb ( "DeviceNotifierQueueClear" );
            
            LockAcquireVA ( deviceNotifierLock, "DeviceNotifierQueueClear" );
            
            while ( !envQueue_empty ( deviceNotifierQueue ) )
            {
				DeviceNotifierContextPtr ctx = C_Only ( ( DeviceNotifierContextPtr ) ) envQueue_front ( deviceNotifierQueue );
				envQueue_pop ( deviceNotifierQueue );
                
                if ( ctx != nill )
                {
                    DisposeNotifierContext ( ctx );

                    CheckNotifierContextContexts ( ctx, true );

                    delete__obj_n ( ctx );
                }
            }
            
            LockReleaseVA ( deviceNotifierLock, "DeviceNotifierQueueClear" );
        }

        
        void Environs::AddToListContainer ( DeviceClass_t listType, NLayerVecType ( DeviceInstanceEP ) c_ref vanished, NLayerVecType ( DeviceInstanceEP ) c_ref appeared )
        {
            bool success = false;
            
            ListContext OBJ_ptr ctx = GetListContext ( listType );
            
            if ( LockAcquire ( c_Addr_of ( ctx->lock ), "AddToListContainer" ) )
            {
                if ( vanished != nill ) {
                    if ( ctx->vanished == nill ) {
                        ctx->vanished = vanished;
                    }
                    else {
                        size_t size = vp_size ( vanished );
                        
                        for ( size_t i = 0; i < size; ++i )
                        {
							DeviceInstanceReferenceESP device = vp_at ( vanished, i );
                            if ( device == nill )
                                continue;
                            
                            ContainerAppend ( ctx->vanished, device );
                        }
                        ContainerClear ( vanished );
                    }
                }
                
                if ( appeared != nill ) {
                    if ( ctx->appeared == nill ) {
                        ctx->appeared = appeared;
                    }
                    else {
                        size_t size = vp_size ( appeared );
                        
                        for ( size_t i = 0; i < size; ++i )
                        {
							DeviceInstanceReferenceESP device = vp_at ( appeared, i );
                            if ( device == nill )
                                continue;
                            
                            ContainerAppend ( ctx->appeared, device );
                        }
                        ContainerClear ( appeared );
                    }
                }
                
                success = true;

				LockRelease ( c_Addr_of ( ctx->lock ), "AddToListContainer" );
            }
            
            if ( !success ) {
                if ( vanished != nill )
                    DisposeListContainer ( vanished );
                if ( appeared != nill )
                    DisposeListContainer ( appeared );
            }
        }
        

		bool Environs::IsDisposalContextSafe ()
		{
			if ( deviceNotifierThread.areWeTheThread () || listCommandThread.areWeTheThread () )
				return true;
			
			if ( areWeTheThreadID ( startStopThread ) )
				return true;

			return false;
        }
        
        
        /**
         * Instructs the framework to perform a quick shutdown (with minimal wait times)
         *
         * @param enable      true / false
         */
        void Environs::SetAppShutdown ( bool enable )
        {
            environs::API::SetAppShutdownN ( enable ? 1 : 0 );
        }
		

        /**
         * Release ownership on this interface and mark it disposable.
         * Release must be called once for each Interface that the Environs framework returns to client code.
         * Environs will dispose the underlying object if no more ownership is hold by anyone.
         *
         */
        void Environs::Release ()
        {
            ENVIRONS_OUTPUT_RELEASE ();

#ifndef CLI_CPP
			if ( localRefCount == 0 ) { // It was -1 before (as we have not retained before)
				CLog ( "Release: Delete" );
				delete this;
			}
#endif
        }


        void Environs::SetInstance ( int hInst  )
        {
			hEnvirons = hInst;
			
#ifndef CLI_CPP
			env = instances [ hEnvirons ];

            hEnvirons = env->hEnvirons;

			instancesAPI [ hInst ] = this;
#endif
        }

        
        /**
         * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
         * Tasks:
         *  - Request display always on state, hence disable standby and power management functions.
         *  - Load sensor managers and sensor services
         *  - Initialize google cloud messaging
         *
         @returns success
         */
        bool Environs::Init ()
        {
            CVerb ( "Init" );
            
            if ( !allocated ) {
				if ( !LockInitA ( listLock ) )
					return false;
				if ( !LockInitA ( queryLock ) )
					return false;
				if ( !LockInitA ( listAllLock ) )
					return false;
				if ( !LockInitA ( listNearbyLock ) )
					return false;
				if ( !LockInitA ( listMediatorLock ) )
					return false;
                if ( !LockInitA ( deviceNotifierLock ) )
                    return false;
                if ( !LockInitA ( listCommandThreadLock ) )
                    return false;

                ENVIRONS_OUTPUT_INIT_OBJLOCK ();

                if ( !LockInitA ( contextAll.lock ) )
                    return false;
                contextAll.type = DeviceClass::All;
                
                if ( !LockInitA ( contextNearby.lock ) )
                    return false;
                contextNearby.type = DeviceClass::Nearby;
                
                if ( !LockInitA ( contextMediator.lock ) )
                    return false;
                contextMediator.type = DeviceClass::Mediator;
                
                if ( !listCommandThread.Init () )
                    return false;
                listCommandThread.autoreset = true;

                if ( !deviceNotifierThread.Init () )
                    return false;
                deviceNotifierThread.autoreset = true;

                if ( !deviceListUpdateSignal.Init () )
                    return false;
                
				listAll				= nill;
				listNearby			= nill;
				listMediator		= nill;

				listAllUpdate       = 0;
				listNearbyUpdate    = 0;
				listMediatorUpdate  = 0;
            }
            
			if ( !InitPlatform () )
				return false;

			allocated = true;

            return environs::API::InitN ( hEnvirons ) != 0;
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
		bool Environs::LoadSettings ( CString_ptr appName, CString_ptr areaName )
		{
			CVerbArg2 ( "LoadSettings", "appName", "s", appName ? appName : "NULL", "areaName", "s", areaName ? areaName : "NULL" );
#ifdef CLI_CPP
			envAppName = appName;
			envAreaName = areaName;
#endif
			return environs::API::LoadSettingsN ( hEnvirons, appName, areaName ) != 0;
        }
        
        
        /**
         * Load settings. Prior to this call, an application environment MUST be given
         * using SetApplicationName and SetAreaName.
         *
         * @return   success
         */
        bool Environs::LoadSettings ()
        {
            CVerb ( "LoadSettings" );
            
            return environs::API::LoadSettingsN ( hEnvirons, nill, nill ) != 0;
        }
        
        
        /**
         * Dispose the storage, that is remove all data and messages received in the data store.
         *
         */
        void Environs::ClearStorage ()
        {
            CVerb ( "LoadSettings" );
            
            environs::API::ClearStorageN ();
        }
        
        
        /**
         * Instruct Environs to output verbose debug logging.
         *
         * @param level      debug level 0 ... 16
         */
        void Environs::SetDebug ( int level )
        {
			CVerbArg1 ( "SetDebug", "enable", "i", level );
            
			Cli_Only ( Utils::logLevel = level; )
            
            environs::API::SetDebugN ( level );
        }
        
        
        /**
         * Get output debug level.
         *
         * @return level      debug level 0 ... 16
         */
        int Environs::GetDebug ()
        {
            CVerb ( "GetDebug" );
            
            return environs::API::GetDebugN ();
        }
        

		/**
		* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
		* Increasing this value may help to handle worse networks which suffer from large latencies.
		*
		* @param   timeout
		*/
        void Environs::SetNetworkConnectTimeout ( int timeout )
		{
			CVerbArg1 ( "SetNetworkConnectTimeout", "timeout", "i", timeout );

			environs::API::SetNetworkConnectTimeoutN ( timeout );
		}
        
        
        /**
         * Get platform that the app is running on.
         *
         * @return 	enum of type Environs.Platforms
         */
        int Environs::GetPlatform ()
        {
            return environs::API::GetPlatformN ();
        }
        
        
        /**
         * Set the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
         * Valid type are enumerated in Environs.Platforms.*
         *
         * @param	platform	Environs.Platforms.*
         */
        void Environs::SetPlatform ( int platform )
        {
            environs::API::SetPlatformN ( platform );
        }
        
        
        /**
         * Set/Remove the location-node flag to the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
         * Flag: Environs.Platforms.LocationNode_Flag
         *
         * @param	isLocationNode	true or false
         */
        void Environs::SetIsLocationNode ( bool isLocationNode )
        {
            environs::API::SetIsLocationNodeN ( isLocationNode );
        }
        
        
        Location cli_OBJ_ptr Environs::GetLocation ()
        {
            Location cli_OBJ_ptr loc Cli_Only ( = nill );
#ifdef CLI_CPP
			loc = gcnew ( Location );
#else
            Zero ( loc );
#endif
            
            // Platform specific location detection here...
            // TODO
            
            return loc;
        }
        
        
        /**
         * Enable or disable device list update notifications from Mediator layer.
         * In particular, mobile devices should disable notifications if the devicelist is not
         * visible to users or the app transitioned to background.
         * This helps recuding cpu load and network traffic when not required.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetMediatorNotificationSubscription ( bool enable )
        {
            bool stateBefore = GetMediatorNotificationSubscription ();
            if ( stateBefore == enable )
                return;
            
            environs::API::SetMediatorNotificationSubscriptionN ( hEnvirons, enable );
            
            if ( !stateBefore )
                ReloadLists ();
        }
        
        
        /**
         * Get subscription status of device list update notifications from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        bool Environs::GetMediatorNotificationSubscription ( )
        {
            return (environs::API::GetMediatorNotificationSubscriptionN ( hEnvirons ) ? 1 : 0);
        }
        
        
        /**
         * Enable or disable short messages from Mediator layer.
         * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
         * This helps recuding cpu load and network traffic when not necessary.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetMessagesSubscription ( bool enable )
        {
            bool stateBefore = GetMessagesSubscription ();
            if ( stateBefore == enable )
                return;
            
            environs::API::SetMessagesSubscriptionN ( hEnvirons, enable );
        }
        
        
        /**
         * Get subscription status of short messages from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        bool Environs::GetMessagesSubscription ( )
        {
            return (environs::API::GetMessagesSubscriptionN ( hEnvirons ) ? 1 : 0);
        }


		/**
		* Ignore autodetection of the actual runtime platform and enforce the given platform.
		*
		* @param		platform of type Environs.platform
		*/
		void Environs::SetPlatform ( environs::Platforms_t platform )
		{
			CVerbArg1 ( "SetPlatform", "platform", "i", (int)platform );

			environs::API::SetPlatformN ( (int) platform );
		}
        
        
        /**
         * Instruct Environs to show log messages in the status log.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetUseNotifyDebugMessage ( int enable )
        {
			CVerbArg1 ( "SetUseNotifyDebugMessage", "enable", "i", enable );
            
            environs::API::SetUseNotifyDebugMessageN ( enable != 0 );
        }
        
        
        /**
         * Query Environs settings that show log messages in the status log.
         *
         * @return enable      true = enabled, false = disabled
         */
        bool Environs::GetUseNotifyDebugMessage ()
        {
            CVerb ( "GetUseNotifyDebugMessage" );
            
            return environs::API::GetUseNotifyDebugMessageN () != 0;
        }
        
        
        /**
         * Instruct Environs to create and write a log file in the working directory.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetUseLogFile ( bool enable )
        {
			CVerbArg1 ( "SetUseLogFile", "enable", "d", enable );
            
            environs::API::SetUseLogFileN ( enable );
        }
        
        
        /**
         * Query Environs settings that create and write a log file in the working directory.
         *
         * @return enable      true = enabled, false = disabled
         */
        bool Environs::GetUseLogFile ()
        {
            CVerb ( "GetUseLogFile" );
            
            return environs::API::GetUseLogFileN () != 0;
        }
        
        
        /**
         * Instruct Environs to log to stdout.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetUseLogToStdout ( bool enable )
        {
            CVerb ( "SetUseLogToStdout" );
            
            environs::API::SetUseLogToStdoutN ( enable );
        }
        
        
        /**
         * Query Environs settings whether to log to stdout.
         *
         * @return enable      true = enabled, false = disabled
         */
        bool Environs::GetUseLogToStdout ()
        {
            CVerb ( "GetUseLogToStdout" );
            
            return environs::API::GetUseLogToStdoutN () != 0;
        }


		/**
		* Instruct Environs to use headless mode without worrying about UI thread.
		*
		* @param enable      true = enable, false = disable
		*/
        void Environs::SetUseHeadless ( bool enable )
		{
			CVerb ( "SetUseHeadless" );

			environs::API::SetUseHeadlessN ( enable );
		}


		/**
		* Query Environs settings whether to use headless mode without worrying about UI thread.
		*
		* @return enable      true = enabled, false = disabled
		*/
        bool Environs::GetUseHeadless ()
		{
			CVerb ( "GetUseHeadless" );

			return environs::API::GetUseHeadlessN () != 0;
		}
        
        
        /**
         * Check for mediator logon credentials and query on command line if necessary.
         *
         * @param success      true = successful, false = failed
         */
        bool Environs::QueryMediatorLogonCommandLine ()
        {
            CVerb ( "QueryMediatorLogonCommandLine" );
            
            return environs::API::QueryMediatorLogonCommandLineN ( hEnvirons ) != 0;
        }


		/**
		* Instruct Environs to create DeviceLists that are used as UIAdapter by client code.
		* Any changes of those lists are made within the applications main / UI thread context.
		* Changing this option is allowed only before the Start of the very first Environs instance
		* (or after stopping of all Environs instances).
		*
		* @param enable      true = enable, false = disable
		*/
		void Environs::SetUseDeviceListAsUIAdapter ( bool enable )
		{
			if ( GetStatus () > Status::Stopped ) {
				CErr ( "SetUseDeviceListAsUIAdapter: Option cannot be changed when at least on instance has started." );
				return;
			}
			isUIAdapter = enable;
		}


		/**
		* Query Environs settings whether to create DeviceLists that are used as UIAdapter by client code.
		* Any changes of those lists are made within the applications main / UI thread context.
		*
		* @return enable      true = enabled, false = disabled
		*/
		bool Environs::GetUseDeviceListAsUIAdapter ()
		{
			return isUIAdapter;
		}
        
        
        /**
         * Reset crypt layer and all created resources. Those will be recreated if necessary.
         * This method is intended to be called directly after creation of an Environs instance.
         *
         */
        void Environs::ResetCryptLayer ()
        {
            CVerb ( "ResetCryptLayer" );
            
            environs::API::ResetCryptLayerN ();
        }
        
        /**
         * Get the native version of Environs.
         *
         * @return		version string
         */
        CString_ptr Environs::GetVersionString ()
        {
			return CCharToString ( environs::API::GetVersionStringN () );
        }
        
        
        /**
         * Get the native major version of Environs.
         *
         * @return		major version
         */
        int Environs::GetVersionMajor ()
        {
            return environs::API::GetVersionMajorN ();
        }
        
        /**
         * Get the native minor version of Environs.
         *
         * @return		minor version
         */
        int Environs::GetVersionMinor ()
        {
            return environs::API::GetVersionMinorN ();
        }
        
        /**
         * Get the native revision of Environs.
         *
         * @return		revision
         */
        int Environs::GetVersionRevision ()
        {
            return environs::API::GetVersionRevisionN ();
        }
        
        
        /**
         * Query whether the native layer was build for release (or debug).
         *
         * @return	true = Release build, false = Debug build.
         */
        bool Environs::GetIsReleaseBuild ()
        {
            return environs::API::GetIsReleaseBuildN () != 0;
        }
        

		/**
		* Update device flags to native layer and populate them to the environment.
		*
        * @param	objID    The identifier for the native device object.
        * @param	flags    The internal flags to set or clear.
        * @param	set    	 true = set, false = clear.
		*/
		void Environs::SetDeviceFlags ( int objID, int flags, bool set )
		{
			CVerbArg1 ( "SetDeviceFlags", "flags", "i", flags );

			environs::API::SetDeviceFlagsN ( hEnvirons, (int) environs::Call::NoWait, objID, flags, set );
		}
        
        
        /**
         * Set the ports that the local instance of Environs shall use for listening on connections.
         *
         * @param	tcpPort communication channel
         * @param	udpPort data channel
         *
         * @return  success
         */
        bool Environs::SetPorts ( int tcpPort, int udpPort )
        {
			CVerbArg2 ( "SetPorts", "tcpPort", "i", tcpPort, "udpPort", "i", udpPort );
            
            return environs::API::SetPortsN ( hEnvirons, tcpPort, udpPort ) != 0;
        }


        /**
         * Set the base port that the local instance of Environs shall use for communication with other instances.
         * This option enables spanning of multiple multi surface environsments separated by the network stacks.
         *
         * @param	port The base port.
         * @return success
         */
        bool Environs::SetBasePort ( int port )
        {
            CVerbArg1 ( "SetBasePort", "port", "i", port );

            return environs::API::SetBasePortN ( hEnvirons, port ) != 0;
        }

        
        unsigned int Environs::GetIPAddress ()
        {
            CVerb ( "GetIPAddress" );
            
            return environs::API::GetIPAddressN ();
        }
        
        
        unsigned int Environs::GetSubnetMask ()
        {
            CVerbVerb ( "GetSubnetMask" );
            
            return environs::API::GetSubnetMaskN ();
        }
        
        
        CString_ptr Environs::GetSSID ()
        {
            CVerb ( "GetSSID" );

#ifndef CLI_CPP
            return environs::API::GetSSID ( false );
#else
			return "";
#endif
        }
        
        
        CString_ptr Environs::GetSSIDDesc ()
        {
            CVerbVerb ( "GetSSIDDesc" );
            
#ifndef CLI_CPP
            return environs::API::GetSSID ( true );
#else
			return "";
#endif
        }
        
        
        /**
         * Set the device id that is assigned to the instance of Environs.
         *
         * @param   deviceID
         */
        void Environs::SetDeviceID ( int deviceID )
        {
			CVerbArg1 ( "SetDeviceID", "deviceID", "i", deviceID );
            
            environs::API::SetDeviceIDN ( hEnvirons, deviceID );
        }
        
        
        /**
         * Get the device id that the application has assigned to the instance of Environs.
         *
         * @return	deviceID
         */
        int Environs::GetDeviceID ()
        {
            CVerbVerb ( "GetDeviceID" );
            
            return environs::API::GetDeviceIDN ( hEnvirons );
        }
        
        
        /**
         * Request a device id from mediator server instances that have been provided before this call.
         * Prior to this call, the area and application name must be provided as well,
         * in order to get an available device id for the specified application environment.
         * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
         *
         * @return	deviceID
         */
        int Environs::GetDeviceIDFromMediator ()
        {
            CVerb ( "GetDeviceIDFromMediator" );
            
            return environs::API::GetDeviceIDFromMediatorN ( hEnvirons );
        }
        
        
        /**
         * Query whether the name of the current device has been set before.
         *
         * @return	has DeviceUID
         */
        bool Environs::HasDeviceUID ()
        {
            CVerb ( "HasDeviceUID" );
            
            return environs::API::HasDeviceUIDN () != 0;
        }
        
        
        /**
         * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
         *
         * @param 	name    A unique device identifier.
         *
         * @return	success
         */
        bool Environs::SetDeviceUID ( CString_ptr name )
        {
            CVerb ( "SetDeviceUID" );
            
            return environs::API::SetDeviceUIDN ( name ) != 0;
        }
        
        
        /**
         * Query ip of custom Mediator.
         *
         * @return ip
         */
        unsigned int Environs::GetMediatorIPValue ()
        {
            CVerb ( "GetMediatorIPValue" );
            
			return environs::API::GetMediatorIPValueN ( hEnvirons );
        }
        
        
        /**
         * Query ip of custom Mediator.
         *
         * @return ip
         */
        CString_ptr Environs::GetMediatorIP ()
        {
            CVerb ( "GetMediatorIP" );

            const char * t = environs::API::GetMediatorIPN ( hEnvirons );

            return ( t != nill ? CCharToString ( t ) : "" );
        }
        
        
        /**
         * Query port of custom Mediator.
         *
         * @return port
         */
        int Environs::GetMediatorPort ()
        {
            CVerb ( "GetMediatorPort" );
            
            return environs::API::GetMediatorPortN ( hEnvirons );
        }
        
        
        /**
         * Determines whether to use Crypto Layer Security for Mediator connections.
         * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
         *
         * @param	enable
         */
        void Environs::SetUseCLSForMediator ( bool enable )
        {
			CVerbArg1 ( "SetUseCLSForMediator", "enable", "i", enable );
            
            environs::API::SetUseCLSForMediatorN (  hEnvirons, enable );
        }
        
        
        /**
         * Query whether to use Crypto Layer Security for Mediator connections.
         *
         * @return	enabled
         */
        bool Environs::GetUseCLSForMediator ()
        {
            CVerb ( "GetUseCLSForMediator" );
            
            return environs::API::GetUseCLSForMediatorN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Determines whether to use Crypto Layer Security for device-to-device connections.
         *
         * @param	enable
         */
        void Environs::SetUseCLSForDevices ( bool enable )
        {
			CVerbArg1 ( "SetUseCLSForDevices", "enable", "i", enable );
            
            environs::API::SetUseCLSForDevicesN (  hEnvirons, enable );
        }
        
        
        /**
         * Query whether to use Crypto Layer Security for device-to-device connections.
         *
         * @return	enabled
         */
        bool Environs::GetUseCLSForDevices ()
        {
            CVerb ( "GetUseCLSForDevices" );
            
            return environs::API::GetUseCLSForDevicesN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Determines whether to enforce Crypto Layer Security for device-to-device connections.
         *
         * @param	enable
         */
        void Environs::SetUseCLSForDevicesEnforce ( bool enable )
        {
			CVerbArg1 ( "SetUseCLSForDevicesEnforce", "enable", "i", enable );
            
            environs::API::SetUseCLSForDevicesEnforceN ( hEnvirons, enable );
        }
        
        
        /**
         * Query whether to enforce Crypto Layer Security for device-to-device connections.
         *
         * @return	enabled
         */
        bool Environs::GetUseCLSForDevicesEnforce ()
        {
            CVerb ( "GetUseCLSForDevicesEnforce" );
            
            return environs::API::GetUseCLSForDevicesEnforceN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
         *
         * @param	enable
         */
        void Environs::SetUseCLSForAllTraffic ( bool enable )
        {
			CVerbArg1 ( "SetUseCLSForAllTraffic", "enable", "i", enable );
            
            environs::API::SetUseCLSForAllTrafficN ( hEnvirons, enable );
        }
        
        
        /**
         * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
         *
         * @return	enabled
         */
        bool Environs::GetUseCLSForAllTraffic ()
        {
            CVerb ( "GetUseCLSForAllTraffic" );
            
            return environs::API::GetUseCLSForAllTrafficN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Determines whether to use environs default Mediator predefined by framework developers or not.
         *
         * @param enable 	true = use the default Mediator
         */
        void Environs::SetUseDefaultMediator ( bool enable )
        {
			CVerbArg1 ( "SetUseDefaultMediator", "enable", "i", enable );
            
            environs::API::SetUseDefaultMediatorN ( hEnvirons, enable );
        }
        
        
        /**
         * Query whether to use given Mediator by setMediator()
         *
         * @return enabled
         */
        bool Environs::GetUseDefaultMediator ()
        {
            CVerb ( "GetUseDefaultMediator" );
            
            return environs::API::GetUseDefaultMediatorN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Determines whether to use given Mediator by setMediator()
         *
         * @param enable 	true = enable, false = disable
         */
        void Environs::SetUseCustomMediator ( bool enable )
        {
			CVerbArg1 ( "SetUseCustomMediator", "enable", "i", enable );
            
            environs::API::SetUseCustomMediatorN ( hEnvirons, enable );
        }
        
        
        /**
         * Query whether to use given Mediator by setMediator()
         *
         * @return enabled
         */
        bool Environs::GetUseCustomMediator ()
        {
            CVerb ( "GetUseCustomMediator" );
            
            return environs::API::GetUseCustomMediatorN ( hEnvirons ) == 1;
        }
        
        
        /**
         * Set custom Mediator to use.
         *
         * @param ip
         * @param port
         */
        bool Environs::SetMediator ( CString_ptr ip, unsigned short port )
        {
            CVerb ( "SetMediator" );
            
            return environs::API::SetMediatorN ( hEnvirons, ip ? ip : nill, port ) != 0;
        }
        
        
        /**
         * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
         *
         * @param 	name
         * @return	success
         */
        bool Environs::SetMediatorUserName ( CString_ptr name )
        {
            CVerb ( "SetMediatorUserName" );
            
            return environs::API::SetMediatorUserNameN ( hEnvirons, name ) != 0;
        }
        
        bool Environs::SetUserName ( CString_ptr name )
        {
            CVerb ( "SetUserName" );
            
            return environs::API::SetMediatorUserNameN ( hEnvirons, name ) != 0;
        }
        
        
        /**
         * Query UserName used to authenticate with a Mediator.
         *
         * @return UserName
         */
        CString_ptr Environs::GetMediatorUserName ()
        {
            CVerb ( "GetMediatorUserName" );

            const char * t = environs::API::GetMediatorUserNameN ( hEnvirons );

            return ( t != nill ? CCharToString ( t ) : "" );
        }
        
        
        /**
         * Enable or disable anonymous logon to the Mediator.
         *
         * @param 	enable A boolean that determines the target state.
         */
        void Environs::SetUseMediatorAnonymousLogon ( bool enable )
        {
            CVerb ( "SetUseMediatorAnonymousLogon" );
            
            environs::API::SetUseMediatorAnonymousLogonN ( hEnvirons, enable );
        }
        
        
        /**
         * Get setting of anonymous logon to the Mediator.
         *
         * @return 	enabled A boolean that determines the target state.
         */
        bool Environs::GetUseMediatorAnonymousLogon ()
        {
            return environs::API::GetUseMediatorAnonymousLogonN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
         *
         * @param 	password
         * @return	success
         */
        bool Environs::SetMediatorPassword ( CString_ptr password )
        {
            CVerb ( "SetMediatorPassword" );
            
            return environs::API::SetMediatorPasswordN ( hEnvirons, password ) != 0;
        }
        
        bool Environs::SetUserPassword ( CString_ptr password )
        {
            CVerb ( "SetUserPassword" );
            
            return environs::API::SetMediatorPasswordN ( hEnvirons, password ) != 0;
        }
        
        
        /**
         * Enable or disable authentication with the Mediator using username/password.
         *
         * @param 	enable
         */
        void Environs::SetUseAuthentication ( bool enable )
        {
			CVerbArg1 ( "SetUseAuthentication", "enable", "d", enable );
            
            environs::API::SetUseAuthenticationN ( hEnvirons, enable );
        }
        
        
        /**
         * Query the filter level for device management within Environs.
         *
         * return level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
         */
        environs::MediatorFilter_t Environs::GetMediatorFilterLevel ()
        {
            CVerb ( "GetMediatorFilterLevel" );
            
            return (environs::MediatorFilter_t) environs::API::GetMediatorFilterLevelN ( hEnvirons );
        }
        
        /**
         * Set the filter level for device management within Environs.
         *
		 * @param   level	can be one of the values of MediatorFilter
         */
        void Environs::SetMediatorFilterLevel ( MediatorFilter_t level )
        {
			CVerbArg1 ( "SetMediatorFilterLevel", "level", "i", level );

			environs::API::SetMediatorFilterLevelN ( hEnvirons, ( int ) level );
        }
        
        
        /**
         * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
         *
         * @return		true = yes, false = no
         */
        bool Environs::GetUseMediatorLoginDialog ()
        {
            CVerb ( "GetUseMediatorLoginDialog" );
            
            return environs::API::GetUseMediatorLoginDialogN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetUseMediatorLoginDialog ( bool enable )
        {
			CVerbArg1 ( "SetUseMediatorLoginDialog", "enable", "d", enable );
            
            environs::API::SetUseMediatorLoginDialogN ( hEnvirons, enable );
        }
        
        
        /**
         * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
         *
         * @return		true = yes, false = no
         */
        bool Environs::GetMediatorLoginDialogDismissDisable ()
        {
            CVerb ( "GetMediatorFilterLevel" );
            
            return environs::API::GetMediatorLoginDialogDismissDisableN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Instruct Environs disable Mediator settings on dismiss of the login dialog.
         *
         * @param enable      true = enable, false = disable
         */
        void Environs::SetMediatorLoginDialogDismissDisable ( bool enable )
        {
			CVerbArg1 ( "SetMediatorLoginDialogDismissDisable", "enable", "d", enable );
            
            environs::API::SetMediatorLoginDialogDismissDisableN ( hEnvirons, enable );
        }


        /**
         * Determines whether touch events should be translated to mouse events. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
         *
         * @param   enable      true = enable, false = disable
         */
		void Environs::SetUseMouseEmulation ( bool enable )
		{
#ifdef DISPLAYDEVICE
			CVerbArg1 ( "SetUseMouseEmulation", "enable", "d", enable );

			environs::API::SetUseMouseEmulationN ( hEnvirons, enable );
#endif
		}


        /**
         * Determines whether touch events should be visualized as rounded circles on the desktop Window. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
         *
         * @param   enable      true = enable, false = disable
         */
		void Environs::SetUseTouchVisualization ( bool enable )
		{
#ifdef DISPLAYDEVICE
			CVerbArg1 ( "SetUseTouchVisualization", "enable", "d", enable );

			environs::API::SetUseTouchVisualizationN ( hEnvirons, enable );
#endif
        }


		/** Default value for each DeviceInstance after object creation. */
		bool Environs::GetAllowConnectDefault ()
		{
			return allowConnectDefault;
		}

		/** Default value for each DeviceInstance after object creation. */
		void Environs::SetAllowConnectDefault ( bool value )
		{
			allowConnectDefault = value;

			environs::API::AllowConnectDefaultN ( hEnvirons, value );
		}
        

        /**
         * Register at known Mediator server instances.
         *
         * @return	success
         */
        bool Environs::RegisterAtMediators ()
        {
            CVerb ( "RegisterAtMediators" );
            
            return environs::API::RegisterAtMediatorsN ( hEnvirons ) != 0;
        }
        
        
        void c_OBJ_ptr Environs::EnvironsStart ( pthread_param_t arg )
        {
			EnvironsPtr envObj = ( EnvironsPtr ) arg;

			pthread_setname_current_envthread ( "Environs.EnvironsStart" );

            int hEnvirons = envObj->GetHandle ();

            envObj->startStopThread = getSelfThreadID ();
            
            CVerb ( "EnvironsStart" );
            CLog ( "Start ---------------------------------------------------------" );
            
#ifdef ENVIRONS_OSX
            if ( !native.useHeadless )
                environs::AddKeyMonitor ();
#endif
            int status = environs::API::GetStatusN ( hEnvirons );

			if ( status >= ( int ) Status::Stopping ) {

                InvokeNetworkNotifier ( hEnvirons, false );
                
                environs::API::StopN ( hEnvirons );
                CLog ( "---------------------------------------------------------------" );
				goto Finish;
            }
            
            if ( status < ( int ) Status::Initialized )
            {
                // Initialize native stack
                status = InitN ( hEnvirons );
                if ( status == 0 ) {
					goto Finish;
                }
            }

            envObj->listCommandThreadRun = true;
            envObj->listCommandThread.Run ( pthread_make_routine ( &DeviceList::CommandThread ), ( pthread_param_t ) envObj, "Start", false );

            envObj->deviceNotifierThreadRun = true;
            envObj->deviceNotifierThread.Run ( pthread_make_routine ( &DeviceInstance::NotifierThread ), ( pthread_param_t ) envObj, "Start", false );
            
            if ( environs::API::GetUseCustomMediatorN ( hEnvirons ) )
            {
                const char * t = GetMediatorIPN ( hEnvirons );

                if ( t != nill )
                    SetMediatorN ( hEnvirons, CCharToString ( t ), GetMediatorPortN ( hEnvirons ) );
            }

            // Start native stack
            if ( !StartN ( hEnvirons ) ) {
				goto Finish;
            }
            
            status = GetStatusN ( hEnvirons );
			if ( status < ( int ) Status::Started ) {
                goto Finish;
            }

            InvokeNetworkNotifier ( hEnvirons, true );

            environs::API::StartSensorListeningAllN ( hEnvirons );
            
        Finish:
			pthread_reset ( envObj->startStopThread );

			return C_Only ( 0 );
        }
        

        /**
         * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
         * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
         * 		this call executes the start tasks within a thread.&nbsp;
         * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
         *
         */
        void Environs::Start ()
        {
            // Create a thread that retrieves the recent parameters from parameter server and connect to the surface app
            CVerb ( "Start" );

			startStopThread = getSelfThreadID ();

			ListCommandQueueClear ();

			DeviceNotifierQueueClear ();
            
            if ( async == Call::NoWait )
            {
                pthread_t thread;
                
                int ret = pthread_create ( c_Addr_of ( thread ), NULL, &EnvironsStart, ( pthread_param_t ) this );
                if ( ret != 0 ) {
                    CErr ( "Start: Failed to create start thread." );
                    return;
                }
#ifndef CLI_CPP
                DetachThread ( nill, nill, thread, "Environs::Start" );
#endif
            }
            else
                EnvironsStart ( ( pthread_param_t ) this );
        }
        
        
        /**
         * Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
         *
         * @return NATIVE_STATUS_*
         */
		environs::Status_t Environs::GetStatus ()
        {
            CVerbVerb ( "GetStatus" );

			return ( environs::Status_t ) environs::API::GetStatusN ( hEnvirons );
        }
        
        
        void c_OBJ_ptr Environs::EnvironsStop ( pthread_param_t arg )
        {            
            CVerb ( "EnvironsStop" );
            CLog ( "Stop ---------------------------------------------------------" );

			pthread_setname_current_envthread ( "Environs.EnvironsStop" );
            
			EnvironsPtr envObj = ( EnvironsPtr ) arg;
            
			envObj->DoStop ();
            
            CLog ( "---------------------------------------------------------------" );
            return C_Only ( 0 );
        }
        
        
        /**
         * Stop Environs and release all acquired resources.
         */
        void Environs::DoStop ()
        {
            CVerb ( "DoStop" );

            startStopThread = getSelfThreadID ();

            environs::API::StopSensorListeningAllN ( hEnvirons );

			if ( hEnvirons )
				environs::API::StopNetLayerN ( hEnvirons );

			deviceNotifierThreadRun = false;
            
            listCommandThread.Notify ( "Environs.Stop", false );
            deviceNotifierThread.Notify ( "Environs.Stop", false );
            
            listCommandThread.Notify ( "Environs.Stop", true );
            deviceNotifierThread.Notify ( "Environs.Stop", true );
            
            ListCommandQueueClear ();
            
            bool isUIThreadedLists = isUIAdapter && IsUIThread ();
            
            if ( isUIThreadedLists ) {
				// If we are the UI thread and are using UIAdapter device lists, then make us not legible to deliver notifications
				pthread_reset ( startStopThread );

                listCommandThread.Detach ( "Environs.listCommandThread" );
                deviceNotifierThread.Detach ( "Environs.deviceNotifierThread" );
            }
            else {
                listCommandThread.Join ( "Environs.listCommandThread" );
                deviceNotifierThread.Join ( "Environs.deviceNotifierThread" );
            }
            
            
            // Clear device lists if there are some and they contain device instances
            DisposeLists ( false );
            
            if ( !isUIThreadedLists )
                DeviceNotifierQueueClear ();
 
			// Make sure that the platform layer has shut down before shutting down native layer
			if ( hEnvirons )
				environs::API::StopN ( hEnvirons );
            
            if ( !isUIThreadedLists ) {
                // Make sure that no more device list updaters are running
                while ( true )
                {
                    deviceListUpdateSignal.ResetSync ( "DeviceListUpdateThread", true, false );

                    if ( listAllUpdate == 0 && listNearbyUpdate == 0 && listMediatorUpdate == 0 )
                        break;

                    deviceListUpdateSignal.WaitOne ( "DeviceListUpdateThread", 2000, true, false );

                    // Cli_Only (System::Threading::Thread::) Sleep ( 250 );
                }
            }

			pthread_reset ( startStopThread );
        }
        
        
        /**
         * Stop Environs and release all acquired resources.
         */
        void Environs::Stop ()
        {
            CVerb ( "Stop" );
            
            listCommandThreadRun	= false;
            
            if ( async == Call::NoWait )
            {
                pthread_t thread;
                
                int ret = pthread_create ( c_Addr_of ( thread ), NULL, &EnvironsStop, ( pthread_param_t ) this );
                if ( ret != 0 ) {
                    CErr ( "Stop: Failed to create stop thread." );
                    return;
                }
#ifndef CLI_CPP
                DetachThread ( nill, nill, thread, "Environs::Stop" );
#endif
            }
            else
                EnvironsStop ( ( pthread_param_t ) this );
        }
        
        
        /**
         * Stop Environs and release all acquired resources.
         */
        void Environs::DisposeInstance ()
        {
            CVerb ( "DisposeInstance" );

			int hInst = hEnvirons;
			if ( !hInst )
				return;

			if ( environs::API::GetDisposingN ( hInst ) )
				return;

			environs::API::SetDisposingN ( hInst, 1 );

			DisposeLists ( true );

			//hEnvirons = 0;

            environs::API::DisposeN ( hInst );
        }
        
        
        /**
         * Set the area name that the local instance of Environs shall use for identification within the environment.
         * It must be set before creating the Environs instance.
         *
         * @param	name
         * @return	success
         */
        bool Environs::SetAreaName ( CString_ptr name )
        {
            CVerb ( "SetAreaName" );
            
			envAreaName = name;

            return environs::API::SetAreaNameN ( hEnvirons, name ) != 0;
        }
        
        /**
         * Get the area name that the local instance of Environs use for identification within the environment.
         * It must be set before creating the Environs instance.
         *
         * @return	areaName
         */
        CString_ptr Environs::GetAreaName ()
        {
            CVerb ( "GetAreaName" );
            
			return STRING_get ( envAreaName );

		//return CCharToString ( environs::API::GetAreaNameN ( hEnvirons ) );
        }
        
        
        /**
         * Set the application name of that the local instance of Environs shall use for identification within the environment.
         * It must be set before creating the Environs instance.
         *
         * @param	name
         * @return	success
         */
        void Environs::SetApplication ( CString_ptr name )
        {
			SetApplicationName ( name );
        }
        
        void Environs::SetApplicationName ( CString_ptr name )
        {
			CVerbArg1 ( "SetApplicationName", "name", "s", name );
            
			envAppName = name;

            environs::API::SetApplicationNameN ( hEnvirons, name );
        }
        
        
        /**
         * Get the application name that the local instance of Environs use for identification within the environment.
         * It must be set before creating the Environs instance.
         *
         * @return	appName
         */
        CString_ptr Environs::GetApplicationName ()
        {
            CVerb ( "GetApplicationName" );
            
			return STRING_get ( envAppName );
			//return CCharToString ( environs::API::GetApplicationNameN ( hEnvirons ) );
        }
        
        
        /**
         * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
         *
         * @param 	deviceName
         * @return	success
         */
        bool Environs::SetDeviceName ( CString_ptr name )
        {
			CVerbArg1 ( "SetDeviceName", "name", "s", name );
            
            return environs::API::SetDeviceNameN ( name ) != 0;
        }
        
        
        /**
         * Use default encoder, decoder, capture, render modules.
         *
         * @return  success
         */
        bool Environs::SetUsePortalDefaultModules ()
        {
            CVerb ( "SetUsePortalDefaultModules" );
            
            return environs::API::SetUsePortalDefaultModulesN ( hEnvirons ) != 0;
        }
        
        
        void Environs::SetUseH264 ( bool enable )
        {
			CVerbArg1 ( "SetUseH264", "enable", "d", enable );
            
            environs::API::SetUseStreamN ( hEnvirons, enable );
        }
        
        bool Environs::GetUseH264 ()
        {
            CVerb ( "GetUseH264" );
            
            return environs::API::GetUseStreamN ( hEnvirons ) != 0;
        }
        
        
        /**
         * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
         *
         * @param   enable
         *
         * @return	success
         */
        void Environs::SetUsePortalTCP ( bool enable )
        {
			CVerbArg1 ( "SetUsePortalTCP", "enable", "d", enable );
            
            //environs::API::SetPortalTCP ( hEnvirons, enable );
        }
        
        
        /**
         * Query whether to use TCP for portal streaming (UDP otherwise)
         *
         * @return enabled
         */
        bool Environs::GetUsePortalTCP ()
        {
            CVerb ( "GetUsePortalTCP" );
            
            return false;
        }
        
        
        /**
         * Use encoder module with the name moduleName. (libEnv-Enc...).
         *
         * @param	name  The module name
         *
         * @return  success
         */
        bool Environs::SetUseEncoder ( CString_ptr name )
        {
			CVerbArg1 ( "SetUseEncoder", "name", "s", name );
            
            return environs::API::SetUseEncoderN ( hEnvirons, name ) != 0;
        }
        
        
        /**
         * Use decoder module with the name moduleName. (libEnv-Dec...).
         *
         * @param	name  The module name
         *
         * @return  success
         */
        bool Environs::SetUseDecoder ( CString_ptr name )
        {
			CVerbArg1 ( "SetUseDecoder", "name", "s", name );
            
            return environs::API::SetUseDecoderN ( hEnvirons, name ) != 0;
        }
        
        
        /**
         * Use capture module with the name moduleName. (libEnv-Cap...).
         *
         * @param	name	the name of the module
         *
         * @return  success
         */
        bool Environs::SetUseCapturer ( CString_ptr name )
        {
			CVerbArg1 ( "SetUseCapturer", "name", "s", name );
            
            return environs::API::SetUseCapturerN ( hEnvirons, name ) != 0;
        }
        
        
        /**
         * Use render module with the name moduleName. (libEnv-Rend...).
         *
         * @param	name	the name of the module
         *
         * @return  success
         */
        bool Environs::SetUseRenderer ( CString_ptr name )
        {
			CVerbArg1 ( "SetUseRenderer", "name", "s", name );
            
            return environs::API::SetUseRendererN ( hEnvirons, name ) != 0;
        }
        
        /**
         * Enable or disable a touch recognizer module by name (libEnv-Rec...).
         *
         * @param	moduleName  The module name
         * @param	enable      Enable or disable
         *
         * @return  success
         */
        bool Environs::SetUseTouchRecognizer ( CString_ptr name, bool enable )
        {
			CVerbArg1 ( "SetUseTouchRecognizer", "name", "s", name );
            
            return environs::API::SetUseTouchRecognizerN ( hEnvirons, name, enable) != 0;
        }
        
        
        int Environs::SetUseTracker ( Call_t asyncl, CString_ptr name )
        {
			CVerbArg1 ( "SetUseTracker", "name", "s", name );

			return environs::API::SetUseTrackerN ( hEnvirons, ( int ) asyncl, name ) != 0;
        }
        
        
        int Environs::GetUseTracker ( CString_ptr name )
        {
			CVerbArg1 ( "GetUseTracker", "name", "s", name );
            
            return environs::API::GetUseTrackerN ( hEnvirons, name );
        }
        
        
        bool Environs::DisposeTracker ( Call_t asyncl, CString_ptr name )
        {
			CVerbArg1 ( "DisposeTracker", "name", "s", name );
            
            return environs::API::DisposeTrackerN ( hEnvirons, ( int ) asyncl, name ) != 0;
        }
        
        
        bool Environs::PushTrackerCommand ( Call_t asyncl, int moduleIndex, int command )
        {
            CVerb ( "PushTrackerCommand" );
            
            return environs::API::PushTrackerCommandN ( hEnvirons, ( int ) asyncl, moduleIndex, command ) != 0;
        }
        

#ifndef CLI_CPP
        /**
         * Add an observer for communication with Environs and devices within the environment.
         *
         * @param   observer Your implementation of EnvironsObserver.
         *
         * @return	success
         */
        bool Environs::AddObserver ( environs::EnvironsObserver OBJ_ptr observer )
        {
			CVerb ( "AddObserver" );

			lib::IIEnvironsObserver OBJ_ptr obsc = ( lib::IIEnvironsObserver OBJ_ptr ) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "AddObserver" );
                        
            if ( observer != 0 && environsObservers != 0 ) {
                size_t i=0;
                size_t size = environsObservers->size ();
                
                for ( ; i<size; i++ ) {
					lib::IIEnvironsObserver OBJ_ptr obs = environsObservers->at ( i );
                    
					if ( obsc == obs )
                        break;
                }
                
                if ( i >= size ) {
					environsObservers->push_back ( obsc );
                    success = true;
                }
            }
            
            LockReleaseVA ( queryLock, "AddObserver" );
            
            return success;
        }
        
        
        /**
         * Remove an observer for communication with Environs and devices within the environment.
         *
         * @param   observer Your implementation of EnvironsObserver.
         *
         * @return	success
         */
        bool Environs::RemoveObserver ( environs::EnvironsObserver OBJ_ptr  observer )
        {
			CVerb ( "RemoveObserver" );

            lib::IIEnvironsObserver OBJ_ptr obsc = ( lib::IIEnvironsObserver OBJ_ptr ) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "RemoveObserver" );

            if ( observer != 0 && environsObservers != 0 )
            {
                size_t size = environsObservers->size ();
                
                for ( size_t i=0; i<size; i++ ) {
					lib::IIEnvironsObserver OBJ_ptr obs = environsObservers->at ( i );
                    
					if ( obsc == obs ) {
                        environsObservers->erase ( environsObservers->begin () + i );
                        success = true;
                        break;
                    }
                }
            }
            
            LockReleaseVA ( queryLock, "RemoveObserver" );
            
            return success;
        }
        

        /**
         * Add an observer for receiving messages.
         *
         * @param   observer Your implementation of EnvironsMessageObserver.
         *
         * @return	success
         */
        bool Environs::AddObserverForMessages ( environs::EnvironsMessageObserver OBJ_ptr observer )
        {
			CVerb ( "AddObserverForMessages" );

            lib::IIEnvironsMessageObserver * obsc = ( lib::IIEnvironsMessageObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "AddObserverForMessages" );
            
            if ( observer != 0 && environsObserversForMessages != 0 ) {
                size_t i=0;
                size_t size = environsObserversForMessages->size ();
                
                for ( ; i<size; i++ ) {
					lib::IIEnvironsMessageObserver * obs = environsObserversForMessages->at ( i );
                    
					if ( obsc == obs )
                        break;
                }
                
                if ( i >= size ) {
                    environsObserversForMessages->push_back ( obsc );
                    success = true;
                }
            }
            
            LockReleaseVA ( queryLock, "AddObserverForMessages" );
            
            return success;
        }
        
        
        /**
         * Remove an observer for receiving messages.
         *
         * @param   observer Your implementation of EnvironsMessageObserver.
         *
         * @return	success
         */
        bool Environs::RemoveObserverForMessages ( environs::EnvironsMessageObserver * observer )
        {
			CVerb ( "RemoveObserverForMessages" );

            lib::IIEnvironsMessageObserver * obsc = ( lib::IIEnvironsMessageObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "RemoveObserverForMessages" );
            
            if ( observer != 0 && environsObserversForMessages != 0 )
            {
                size_t size = environsObserversForMessages->size ();
                
                for ( size_t i=0; i<size; i++ ) {
					lib::IIEnvironsMessageObserver * obs = environsObserversForMessages->at ( i );
                    
					if ( obsc == obs ) {
                        environsObserversForMessages->erase ( environsObserversForMessages->begin () + i );
                        success = true;
                        break;
                    }
                }
            }
            
            LockReleaseVA ( queryLock, "RemoveObserverForMessages" );
            
            return success;
        }

        
        /**
         * Add an observer for receiving data buffers and files.
         *
         * @param   observer Your implementation of EnvironsDataObserver.
         *
         * @return	success
         */
        bool Environs::AddObserverForData ( environs::EnvironsDataObserver * observer )
        {
            CVerb ( "AddObserverForData" );

            lib::IIEnvironsDataObserver * obsc = ( lib::IIEnvironsDataObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "AddObserverForData" );

            if ( observer != 0 && environsObserversForData != 0 ) {
                size_t i=0;
                size_t size = environsObserversForData->size ();
                
                for ( ; i<size; i++ ) {
					lib::IIEnvironsDataObserver * obs = environsObserversForData->at ( i );
                    
					if ( obsc == obs )
                        break;
                }
                
                if ( i >= size ) {
                    environsObserversForData->push_back ( obsc );
                    success = true;
                }
            }
            
            LockReleaseVA ( queryLock, "AddObserverForData" );
            
            return success;
        }
        
        
        /**
         * Remove an observer for receiving data buffers and files.
         *
         * @param   observer Your implementation of EnvironsDataObserver.
         *
         * @return	success
         */
        bool Environs::RemoveObserverForData ( environs::EnvironsDataObserver * observer )
        {
			CVerb ( "RemoveObserverForData" );

            lib::IIEnvironsDataObserver * obsc = ( lib::IIEnvironsDataObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "RemoveObserverForData" );
            
            if ( observer != 0 && environsObserversForData != 0 )
            {
                size_t size = environsObserversForData->size ();
                
                for ( size_t i=0; i<size; i++ ) {
					lib::IIEnvironsDataObserver * obs = environsObserversForData->at ( i );
                    
					if ( obsc == obs ) {
                        environsObserversForData->erase ( environsObserversForData->begin () + i );
                        success = true;
                        break;
                    }
                }
            }
            
            LockReleaseVA ( queryLock, "RemoveObserverForData" );
            
            return success;
        }
        
        
        /**
         * Add an observer for receiving sensor data of all devices.
         * Please note: This observer reports sensor data of all devices that are connected and send to us.
         * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
         *
         * @param   observer Your implementation of EnvironsSensorObserver.
         *
         * @return	success
         */
        bool Environs::AddObserverForSensorData ( environs::EnvironsSensorObserver * observer )
        {
            CVerb ( "AddObserverForSensorData" );
            
            lib::IIEnvironsSensorObserver * obsc = ( lib::IIEnvironsSensorObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "AddObserverForSensorData" );

            if ( observer != 0 && environsObserversForSensor != 0 ) {
                size_t i=0;
                size_t size = environsObserversForSensor->size ();
                
                for ( ; i<size; i++ ) {
					lib::IIEnvironsSensorObserver * obs = environsObserversForSensor->at ( i );
                    
					if ( obsc == obs )
                        break;
                }
                
                if ( i >= size ) {
					environsObserversForSensor->push_back ( obsc );
                    success = true;
                }
            }
            
            LockReleaseVA ( queryLock, "AddObserverForSensorData" );
            
            return success;
        }
        
        
        /**
         * Remove an observer for receiving data buffers and files.
         * Please note: This observer reports sensor data of all devices that are connected and send to us.
         * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
         *
         * @param   observer Your implementation of EnvironsSensorObserver.
         *
         * @return	success
         */
        bool Environs::RemoveObserverForSensorData ( environs::EnvironsSensorObserver * observer )
        {
            CVerb ( "RemoveObserverForSensorData" );
            
            lib::IIEnvironsSensorObserver * obsc = ( lib::IIEnvironsSensorObserver *) observer;
            
            bool success = false;
            
            LockAcquireVA ( queryLock, "RemoveObserverForSensorData" );

            if ( observer != 0 && environsObserversForSensor != 0 )
            {
                size_t size = environsObserversForSensor->size ();
                
                for ( size_t i=0; i<size; i++ ) {
					lib::IIEnvironsSensorObserver * obs = environsObserversForSensor->at ( i );
                    
					if ( obsc == obs ) {
                        environsObserversForSensor->erase ( environsObserversForSensor->begin () + i );
                        success = true;
                        break;
                    }
                }
            }
            
            LockReleaseVA ( queryLock, "RemoveObserverForSensorData" );
            
            return success;
        }
#endif
        
        
        bool Environs::GetPortalNativeResolution ()
        {
            return environs::API::GetPortalNativeResolutionN ( hEnvirons ) != 0;
        }
        
        void Environs::SetPortalNativeResolution ( bool enable )
        {
            environs::API::SetPortalNativeResolutionN ( hEnvirons, enable );
        }
        
        bool Environs::GetPortalAutoStart ()
        {
            return environs::API::GetPortalAutoStartN ( hEnvirons ) != 0;
        }
        
        void Environs::SetPortalAutoStart ( bool enable )
        {
            environs::API::SetPortalNativeResolutionN ( hEnvirons, enable );
        }


		DeviceDisplay OBJ_ptr Environs::GetDeviceDisplayProps ( int nativeID )
		{
			return ( DeviceDisplay OBJ_ptr ) BuildDeviceDisplayProps ( environs::API::GetDeviceDisplayPropsN ( hEnvirons, nativeID ), nativeID );
		}


        /**
         * Create a new collection that holds all devices of given list type. This list ist updated dynamically by Environs.
         * After client code is done with the list, the list->Release () method MUST be called by the client code,
         * in order to release the resource (give ownership) back to Environs.
         *
         * @return Collection of IDeviceInstance objects
         */
		EPSPACE DeviceList OBJ_ptr Environs::CreateDeviceList ( environs::DeviceClass_t listType )
        {
			CVerbArg1 ( "CreateDeviceList", "listType", "i", listType );
            
			EPSPACE DeviceList OBJ_ptr list = new__obj ( EPSPACE DeviceList );
            if ( list ) {
                list->hEnvirons     = hEnvirons;
                list->isUIAdapter   = isUIAdapter;
                list->envObj        = this;
                
                C_Only ( list->env  = env );
                
                list->SetListType ( listType );
            }
            return list;
        }
        

#ifndef CLI_CPP
        /**
         * Create a new collection that holds all devices of given list type. This list ist updated dynamically by Environs.
         * After client code is done with the list, the list->Release () method MUST be called by the client code,
         * in order to release the resource (give ownership) back to Environs.
         *
         * @return Collection of IDeviceInstance objects
         */
		DeviceList OBJ_ptr Environs::CreateDeviceListRetained ( environs::DeviceClass_t listType )
        {
            CVerb ( "CreateDeviceListRetained" );
            
			DeviceList OBJ_ptr list = CreateDeviceList ( listType );
			if ( list )
                list->Retain ();
            return list;
        }
#endif
        
        ListContext OBJ_ptr Environs::GetListContext ( environs::DeviceClass_t listType )
        {
            
            switch ( listType ) {
                case environs::DeviceClass::All :
                    return Addr_of ( contextAll );
                    
                case environs::DeviceClass::Nearby :
                    return Addr_of ( contextNearby );
                    
                default:
                    break;
            }
            return Addr_of ( contextMediator );
        }
        
        
        /**
         * Dispose all device lists.
         * This method is intended to be used by the platform layer when the framework shuts down.
         */
        void Environs::DisposeLists ( bool releaseList )
        {
            CVerbArg1 ( "DisposeLists", "releaseList", "d", releaseList );
            
            if ( listAll != nill ) 
			{
				DeviceList::DisposeList ( isUIAdapter, listAll, OBJ_ref listAllLock );
				if ( releaseList )
					listAll = nill;
                
                if ( listNearby != nill ) {
                    DeviceList::DisposeList ( isUIAdapter, listNearby, OBJ_ref listNearbyLock );
                    if ( releaseList )
                        listNearby = nill;
                }
                
                if ( listMediator != nill ) {
                    DeviceList::DisposeList ( isUIAdapter, listMediator, OBJ_ref listMediatorLock );
                    if ( releaseList )
                        listMediator = nill;
                }
            }
            else {
				DeviceList::DisposeList ( isUIAdapter, listNearby, OBJ_ref listNearbyLock );
				if ( releaseList )
					listNearby = nill;

				DeviceList::DisposeList ( isUIAdapter, listMediator, OBJ_ref listMediatorLock );
				if ( releaseList )
					listMediator = nill;
            }
        }
        
        
        /**
         * Dispose device list.
         * This method is intended to be used by the platform layer to enforce disposal of a certain list.
         */
        void Environs::DisposeList ( int listType )
        {
            CVerbArg1 ( "DisposeList", "listType", "d", listType );
            
            switch ( listType ) {
                case MEDIATOR_DEVICE_CLASS_ALL:
                    if ( listAll != nill )
                    {
                        DeviceList::DisposeList ( isUIAdapter, listAll, OBJ_ref listAllLock );
                    }
                    break;
                    
                case MEDIATOR_DEVICE_CLASS_NEARBY:
                    if ( listNearby != nill )
                    {
                        DeviceList::DisposeList ( isUIAdapter, listNearby, OBJ_ref listNearbyLock );
                    }
                    break;
                    
                case MEDIATOR_DEVICE_CLASS_MEDIATOR:
                    if ( listMediator != nill )
                    {
                        DeviceList::DisposeList ( isUIAdapter, listMediator, OBJ_ref listMediatorLock );
                    }
                    break;
            }
        }
        
        
        /**
         * Reload all device lists.
         */
        void Environs::ReloadLists ()
        {
            CVerb ( "ReloadLists" );
            
			if ( GetStatus () < Status::Starting )
				return;

            if ( listAll != nill ) {
                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_ALL );
                return;
            }
            
            if ( listNearby != nill )
                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_NEARBY );
            
            if ( listMediator != nill )
                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_MEDIATOR );
        }
        

		devList ( DeviceInstanceEP ) c_ref Environs::GetDevices ( int type )
        {
            CVerb ( "GetDevices" );
                        
            if ( LockAcquireA ( listLock, "GetDevices" ) )
            {
                switch ( type ) {
                    case MEDIATOR_DEVICE_CLASS_ALL:
                        if ( listAll == nill ) {
							listAll = devListNew ( isUIAdapter, DeviceInstanceEP );
                            
                            // if the other lists exist, then clear them and rebuild
                            if ( listNearby != nill ) {
								DeviceList::DisposeList ( isUIAdapter, listNearby, OBJ_ref listNearbyLock );
                            }
                            
                            if ( listMediator != nill ) {
								DeviceList::DisposeList ( isUIAdapter, listMediator, OBJ_ref listMediatorLock );
                            }
                            DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_ALL );
                        }
                        
                        LockReleaseVA ( listLock, "GetDevices" );
                        return listAll;
                        
                    case MEDIATOR_DEVICE_CLASS_NEARBY:
                        if ( listNearby == nill ) {
							listNearby = devListNew ( isUIAdapter, DeviceInstanceEP );
                            if ( listAll )
                                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_ALL );
                            else
                                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_NEARBY );
                        }
                        
                        LockReleaseVA ( listLock, "GetDevices" );
                        return listNearby;
                        
                    case MEDIATOR_DEVICE_CLASS_MEDIATOR:
                        if ( listMediator == nill ) {
							listMediator = devListNew ( isUIAdapter, DeviceInstanceEP );
                            if ( listAll )
                                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_ALL );
                            else
                                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_MEDIATOR );
                        }
                        
                        LockReleaseVA ( listLock, "GetDevices" );
                        return listMediator;
                }
            }
            
            return listAll;
        }
        
        
		devList ( DeviceInstanceEP ) c_ref Environs::GetDevicesBest ( pthread_mutex_t OBJ_ptr OBJ_ref lock )
        {
            CVerb ( "GetDevices" );
            
            if ( LockAcquireA ( listLock, "GetDevicesBest" ) )
            {
                if ( listAll != nill ) {
                    LockReleaseVA ( listLock, "GetDevicesBest" );
					lock = OBJ_ref listAllLock;
                    return listAll;
                }
                else if ( listNearby != nill ) {
                    LockReleaseVA ( listLock, "GetDevicesBest" );
					lock = OBJ_ref listNearbyLock;
                    return listNearby;
                }
                else if ( listMediator != nill ) {
                    LockReleaseVA ( listLock, "GetDevicesBest" );
					lock = OBJ_ref listMediatorLock;
                    return listMediator;
                }
                
                LockReleaseVA ( listLock, "GetDevicesBest" );
            }
            
            return GetDevices ( MEDIATOR_DEVICE_CLASS_ALL );
        }


		void Environs::GetDeviceByObjectID ( DeviceInstanceESP OBJ_ref device, OBJIDType objID )
		{
			pthread_mutex_t OBJ_ptr lock	= nill;

			devList ( DeviceInstanceEP ) list = GetDevicesBest ( lock );
			
			if ( list != nill )
				DeviceList::GetDevice ( list, lock, device, objID, nill );
		}


		void Environs::GetDeviceByDeviceID ( DeviceInstanceESP OBJ_ref device, int deviceID )
		{
			pthread_mutex_t OBJ_ptr lock = nill;

			devList ( DeviceInstanceEP ) list = GetDevicesBest ( lock );
			
			if ( list != nill )
				DeviceList::GetDeviceSeeker ( list, lock, device, deviceID, false );
		}


		void Environs::GetDeviceByNativeID ( DeviceInstanceESP OBJ_ref device, int nativeID )
		{
			pthread_mutex_t OBJ_ptr lock = nill;

			devList ( DeviceInstanceEP ) list = GetDevicesBest ( lock );

			if ( list != nill ) {
				DeviceList::GetDeviceByNativeID ( list, lock, device, nativeID );
			}
		}


        /**
         * Query a DeviceInstance object that first match the deviceID only.
         * Usually the one that is in the same app environment is picked up.
         * If there is no matching in the app environment,
         * then the areas are searched for a matchint deviceID.
         *
         * @param objID      The objID that identifies the device.
         * @return DeviceInstance-object
         */
		void Environs::GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceAll" );
            
            return GetDeviceAll ( device, objID, true );
        }
        
        
		void Environs::GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isNativeID )
        {
            CVerbVerb ( "GetDeviceAll" );
            
			devList ( DeviceInstanceEP ) deviceList;
            pthread_mutex_t          OBJ_ptr lock = nill;
            
            if ( listAll != nill ) {
                deviceList = listAll;
                lock = Addr_of ( listAllLock );
            }
            else if ( listNearby != nill ) {
                deviceList = listNearby;
                lock = Addr_of ( listNearbyLock );
            }
            else if ( listMediator != nill ) {
                deviceList = listMediator;
                lock = Addr_of ( listMediatorLock );
            }
            
            if ( deviceList == nill ) {
                deviceList = GetDevices ( MEDIATOR_DEVICE_CLASS_ALL );
                lock = Addr_of ( listAllLock );
            }

			DeviceList::GetDeviceSeeker ( deviceList, lock, device, objOrDeviceID, isNativeID );
        }
        
        
        /**
         * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
         *
         * @param nativeID      The native id of the target device.
         * @return DeviceInstance-object
         */
		void Environs::GetDeviceNearby ( DeviceInstanceESP OBJ_ref device, OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceNearby" );
			
			DeviceList::GetDevice ( GetDevicesNearby (), OBJ_ref listNearbyLock, device, objID, nill );
        }

        
        /**
         * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
         *
         * @return ArrayList with DeviceInstance objects
         */
		c_const devList ( DeviceInstanceEP ) c_ref Environs::GetDevicesNearby ()
        {
            CVerb ( "GetDevicesNearby" );
            
            return GetDevices ( MEDIATOR_DEVICE_CLASS_NEARBY );
        }
        

		/**
		* Query a DeviceInstance object of Mediator managed devices within the environment.
		*
		* @param nativeID      The native id of the target device.
		* @return DeviceInstance-object
		*/
		void Environs::GetDeviceFromMediator ( DeviceInstanceESP OBJ_ref device, OBJIDType objID )
		{
			CVerbVerb ( "GetDeviceFromMediator" );

			DeviceList::GetDevice ( GetDevicesFromMediator (), OBJ_ref listMediatorLock, device, objID, nill );
		}
        
        
        /**
         * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
         *
         * @return ArrayList with DeviceInstance objects
         */
		c_const devList ( DeviceInstanceEP ) c_ref Environs::GetDevicesFromMediator ()
        {
            CVerb ( "GetDevicesFromMediator" );
            
            return GetDevices ( MEDIATOR_DEVICE_CLASS_MEDIATOR );
        }
        
        
        void Environs::OnDeviceListNotification ( environs::ObserverNotifyContext OBJ_ptr ctx )
        {
            CVerbVerb ( "OnDeviceListNotification" );

			if ( listAll != nill ) {
				DeviceInstanceESP device = nill; GetDeviceAll ( device, ctx->destID );
				if ( device != nill )
					OnDeviceListNotification1 ( device, ctx );
			}
			else
			{
				if ( listNearby != nill ) {
					DeviceInstanceESP device = nill; GetDeviceNearby ( device, ctx->destID );
					if ( device != nill )
						OnDeviceListNotification1 ( device, ctx );
				}

				if ( listMediator != nill ) {
					DeviceInstanceESP device = nill; GetDeviceFromMediator ( device, ctx->destID );
					if ( device != nill )
						OnDeviceListNotification1 ( device, ctx );
				}
			}
        }


		void Environs::OnDeviceListNotification1 ( c_const DeviceInstanceESP c_ref device, environs::ObserverNotifyContext OBJ_ptr ctx )
        {
            CVerbVerb ( "OnDeviceListNotification1" );

			if ( ctx->notification == NOTIFY_CONTACT_DIRECT_CHANGED ) {
				device->SetDirectContact ( ctx->sourceIdent );
		}
			else if ( ctx->notification == NOTIFY_FILE_SEND_PROGRESS )
			{
				CVerbArg2 ( "OnDeviceListNotification1", "send fileID", "i", ctx->sourceIdent, "progress", "i", ctx->contextPtr );
				device->SetFileProgress ( ctx->sourceIdent, ctx->context, true );
			}
			else if ( ctx->notification == NOTIFY_FILE_RECEIVE_PROGRESS )
			{
				CVerbArg2 ( "OnDeviceListNotification1", "receive fileID", "i", ctx->sourceIdent, "progress", "i", ctx->contextPtr );
				device->SetFileProgress ( ctx->sourceIdent, ctx->context, false );
			}
        }
    
        
        CLASS ThreadPackListUpdate
        {
        public:
            int				listType;
            long OBJ_ptr	listRequests;
            EnvironsPtr     api;
        };

        
        void c_OBJ_ptr Environs::DeviceListUpdateThreadStarter ( pthread_param_t pack )
        {
            CVerbVerb ( "DeviceListUpdateThreadStarter" );

			pthread_setname_current_envthread ( "Environs.DeviceListUpdateThreadStarter" );
            
			sp_assign ( ThreadPackListUpdate, thread, pack );

            thread->api->DeviceListUpdateThread ( pack );

			C_Only ( return 0 );
        }
        
        
        void * Environs::DeviceListUpdateThread ( pthread_param_t pack )
        {
            CVerbVerb ( "DeviceListUpdateThread" );

			pthread_setname_current_envthread ( "Environs.DeviceListUpdateThread" );

            ThreadPackListUpdate OBJ_ptr thread = (ThreadPackListUpdate OBJ_ptr ) pack;
            
        QueryAgain:
            DeviceList::DeviceListUpdater ( this, thread->listType );
            
            if ( !LockAcquireA ( queryLock, "DeviceListUpdateThread" ) )
                return 0;

            long cur = *thread->listRequests;
            cur--;
            *(thread->listRequests) = cur;
            
            if ( cur > 0 ) {
                if ( !listCommandThreadRun ) {
                    *(thread->listRequests) = 0;
                    
                    LockReleaseVA ( queryLock, "DeviceListUpdateThread" );
                    return 0;
                }
                
                LockReleaseVA ( queryLock, "DeviceListUpdateThread" );
                goto QueryAgain;
            }
            
            LockReleaseVA ( queryLock, "DeviceListUpdateThread" );

            deviceListUpdateSignal.Notify ( "DeviceListUpdateThread", true );

            return 0;
        }
        
        
        void Environs::OnDeviceListUpdate ()
        {
            CVerbVerb ( "OnDeviceListUpdate" );

            if ( listAll != nill )
                DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_ALL );
            else
            {
                if ( listNearby != nill )
                    DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_NEARBY );
                
                if ( listMediator != nill )
                    DeviceListUpdate ( MEDIATOR_DEVICE_CLASS_MEDIATOR );
            }
        }
        
        
        /**
         * Updated the devicelist within a thread.
         */
        void Environs::DeviceListUpdate ( int listType )
        {
			CVerbVerbArg1 ( "DeviceListUpdate", "listType", "i", listType );

            if ( !listCommandThreadRun )
                return;
            
            long OBJ_ptr listRequests = nill;
            
            if ( listType == MEDIATOR_DEVICE_CLASS_ALL )
                listRequests = c_Addr_of ( listAllUpdate );
            
            else if ( listType == MEDIATOR_DEVICE_CLASS_NEARBY )
                listRequests = c_Addr_of ( listNearbyUpdate );
            
            else if ( listType == MEDIATOR_DEVICE_CLASS_MEDIATOR )
                listRequests = c_Addr_of ( listMediatorUpdate );
            
            else return;
            
            if ( !LockAcquireA ( queryLock, "DeviceListUpdate" ) )
                return;
            
            if ( *listRequests == 0 ) {
                *listRequests = 1;
            }
            else {
                if (*listRequests == 1)
                    *listRequests = 2;
                
                LockReleaseVA ( queryLock, "DeviceListUpdate" );
                return;
            }
            
            LockReleaseVA ( queryLock, "DeviceListUpdate" );

            ThreadPackListUpdate OBJ_ptr threadPack = new__obj ( ThreadPackListUpdate );
            
            threadPack->listType     = listType;
            threadPack->listRequests = listRequests;
            threadPack->api          = this;
            
            pthread_t thread;

			int ret = pthread_create ( c_Addr_of ( thread ), NULL, &Environs::DeviceListUpdateThreadStarter, ( pthread_param_t ) threadPack );
			if ( ret != 0 ) {
				CErr ( "DeviceListUpdate: Failed to create handler thread." );
				delete__obj ( threadPack );
                return;
			}            
#ifndef CLI_CPP
            DetachThread ( nill, nill, thread, "Environs::DeviceListUpdate" );
#endif
        }
        
        
        void Environs::UpdateConnectProgress ( OBJIDType objID, int progress )
        {
			CVerbVerbArg2 ( "UpdateConnectProgress", "objID", "i", objID, "progress", "i", progress );

            // Keep an SP to the list on the stack and call by reference
            devList ( DeviceInstanceEP ) list = listAll;
			if ( list != nill ) {
				DeviceList::UpdateConnectProgress ( Addr_of ( listAllLock), list, objID, progress );
			}
			else {
                list = listNearby;
				if ( list != nill )
					DeviceList::UpdateConnectProgress ( Addr_of ( listNearbyLock ), list, objID, progress );
                
                list = listMediator;
				if ( list != nill )
					DeviceList::UpdateConnectProgress ( Addr_of ( listMediatorLock ), list, objID, progress );
			}
        }


		void Environs::UpdateMessage ( environs::ObserverMessageContext OBJ_ptr ctx )
		{
			CVerbVerb ( "UpdateMessage" );
            
            // Keep an SP to the list on the stack and call by reference
            devList ( DeviceInstanceEP ) list = listAll;
			if ( list != nill ) {
				DeviceList::UpdateMessage ( Addr_of ( listAllLock ), list, ctx );
			}
            else {
                list = listNearby;
				if ( list != nill )
					DeviceList::UpdateMessage ( Addr_of ( listNearbyLock ), list, ctx );
                
                list = listMediator;
				if ( list != nill )
					DeviceList::UpdateMessage ( Addr_of ( listMediatorLock ), list, ctx );
			}
		}


		void Environs::UpdateData ( environs::ObserverDataContext OBJ_ptr ctx )
		{
			CVerbVerb ( "UpdateData" );
            
            devList ( DeviceInstanceEP ) list = listAll;
			if ( listAll != nill ) {
				DeviceList::UpdateData ( Addr_of ( listAllLock ), listAll, ctx );
			}
            else {
                list = listNearby;
				if ( list != nill )
					DeviceList::UpdateData ( Addr_of ( listNearbyLock ), list, ctx );
                
                list = listMediator;
				if ( list != nill )
					DeviceList::UpdateData ( Addr_of ( listMediatorLock ), list, ctx );
			}
		}


		void Environs::UpdateUdpData ( UdpDataContext OBJ_ptr udpData )
		{
			//CVerbVerb ( "UpdateSensorData" );
            
            devList ( DeviceInstanceEP ) list = listAll;
			if ( listAll != nill ) {
				DeviceList::UpdateUdpData ( Addr_of ( listAllLock ), listAll, udpData );
			}
            else {
                list = listNearby;
				if ( list != nill )
					DeviceList::UpdateUdpData ( Addr_of ( listNearbyLock ), list, udpData );
                
                list = listMediator;
				if ( list != nill )
					DeviceList::UpdateUdpData ( Addr_of ( listMediatorLock ), list, udpData );
			}
		}
        
        
		/**
		* Connect to device with the given ID and a particular application environment.
		*
		* @param deviceID	Destination device ID
		* @param areaName	Project name of the application environment
		* @param appName	Application name of the application environment
		* @param async		(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
		* @return status	0: Connection can't be conducted (maybe environs is stopped or the device ID is invalid) &nbsp;
		* 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
		* 					2: A new connection has been triggered and is in progress
		*/
		int Environs::DeviceConnect ( int deviceID, CString_ptr areaName, CString_ptr appName, Call_t asyncl )
		{
			return environs::API::DeviceConnectN ( hEnvirons, deviceID, areaName, appName, ( int ) asyncl );
		}


		/**
		* Set render callback.
		*
		* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
		* @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
		* @param callback		The pointer to the callback.
		* @param callbackType	A value of type Environs.RenderCallbackType_t that tells the portal receiver what we actually can render..
		* @return				true = success, false = failed.
		*/
#ifdef CLI_CPP
		bool Environs::SetRenderCallback ( Call_t asyncl, int portalID, PortalSinkSource ^ callback, RenderCallbackType_t callbackType )
		{
			return environs::API::SetRenderCallbackN ( hEnvirons, ( int ) asyncl, portalID, Marshal::GetFunctionPointerForDelegate ( callback ).ToPointer (), ( int ) callbackType );
		}
#else
		bool Environs::SetRenderCallback ( Call_t asyncl, int portalID, void * callback, RenderCallbackType_t callbackType )
		{
			return environs::API::SetRenderCallbackN ( hEnvirons, asyncl, portalID, callback, ( int ) callbackType ) != 0;
		}
#endif


		/**
		* Release render callback delegate or pointer
		*
		* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
		* @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
		* @param callback		A delegate that manages the callback.
		* @return				true = success, false = failed.
		*/
		bool Environs::ReleaseRenderCallback ( Call_t asyncl, int portalID )
		{
			return environs::API::ReleaseRenderSurfaceN ( hEnvirons, ( int ) asyncl, portalID ) != 0;
		}



		/**
		* Start streaming of portal to or from the portal identifier (received in notification).
		*
		* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
		* @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
		*
		* @return success
		*/
		bool Environs::StartPortalStream ( Call_t asyncl, int portalID )
		{
			return environs::API::StartPortalStreamN ( hEnvirons, ( int ) asyncl, portalID ) != 0;
		}


		/**
		* Stop streaming of portal to or from the portal identifier (received in notification).
		*
		* @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
		* @param 	nativeID    The native device id of the target device.
		* @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 					    Applications should store them in order to address the correct portal within Environs.
		* @return success
		*/
		bool Environs::StopPortalStream ( Call_t asyncl, int nativeID, int portalID )
		{
			return environs::API::StopPortalStreamN ( hEnvirons, ( int ) asyncl, nativeID, portalID ) != 0;
		}


		/**
		* Get the status, whether the device (id) has established an active portal
		*
		* @param 	nativeID    The device id of the target device.
		* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
		* @return	success 	true = yes, false = no
		*/
		bool Environs::GetPortalEnabled ( int nativeID, int portalType )
		{
			return environs::API::GetPortalEnabledN ( hEnvirons, nativeID, portalType ) != 0;
		}


		/// <summary>
		/// Get the number of devices that are currently connected to our device.
		/// <returns>Count of connected devices</param>
		/// </summary>
		int Environs::GetConnectedDevicesCount ()
		{
			return environs::API::GetConnectedDevicesCountN ( hEnvirons );
		}

		/// <summary>
		/// Get enabled status for stream encoding.
		/// </summary>
		bool Environs::GetUseStream ()
		{
			return environs::API::GetUseStreamN ( hEnvirons ) != 0;
		}
        
        
		/// <summary>
		/// Get platform_ support for OpenCL.
		/// </summary>
		bool Environs::GetUseOpenCL ()
		{
#ifdef DISPLAYDEVICE
			return environs::API::GetUseOpenCLN ( hEnvirons ) != 0;
#else
            return false;
#endif
		}

		/// <summary>
		/// Switch platform_ support for OpenCL rendering.
		/// </summary>
		void Environs::SetUseOpenCL ( bool enable )
		{
#ifdef DISPLAYDEVICE
			return environs::API::SetUseOpenCLN ( hEnvirons, enable );
#endif
        }


        /**
         * Option for whether to observe wifi networks to help location based services.
         *
         * @param	enable  A boolean that determines the target state.
         */
        void Environs::SetUseWifiObserver ( bool enable )
        {
            return environs::API::SetUseWifiObserverN ( enable );
        }


        /**
         * Query option for whether to observe wifi networks to help location based services.
         *
         * @return enabled.
         */
        bool Environs::GetUseWifiObserver ()
        {
            return environs::API::GetUseWifiObserverN () != 0;
        }

		/**
		* Determines the interval for scanning of wifi networks.
		*
		* @param	interval  A millisecond value for scan intervals.
		*/
		void Environs::SetUseWifiInterval ( int interval )
		{
			environs::API::SetUseWifiIntervalN ( interval );
        }

        /**
         * Get the interval for scanning of wifi networks.
         *
         * @return	interval  A millisecond value for scan intervals.
         */
        int Environs::GetUseWifiInterval ()
        {
            return environs::API::GetUseWifiIntervalN ();
        }


#ifndef CLI_CPP
        /**
         * Get a collection that holds all available wifi APs. This list is NOT updated dynamically.
         *
         * @return WifiList with WifiItem objects
         */
        WifiList * Environs::GetWifisRetained ()
        {
            void * data = environs::API::GetWifisN ();

            if ( data == nill )
                return nill;

			WifiListInstance * wifis = WifiListInstance::CreateWithWifisRetained ( ( char * ) data );
            if ( wifis == nill )
                free ( data );

            return wifis;
        }


		/**
		* Get a collection that holds all available Bluetooth devices. This list is NOT updated dynamically.
		*
		* @return BtList with BtItem objects
		*/
        BtList * Environs::GetBtsRetained ()
        {
            void * data = environs::API::GetBtsN ();

            if ( data == nill )
                return nill;

			BtListInstance * bts = BtListInstance::CreateWithBtsRetained ( ( char * ) data );
            if ( bts == nill )
                free ( data );

            return bts;
        }
#endif


        /**
         * Option for whether to observe blueooth to help location based services.
         *
         * @param	enable  A boolean that determines the target state.
         */
        void Environs::SetUseBtObserver ( bool enable )
        {
            return environs::API::SetUseBtObserverN ( enable );
        }


        /**
         * Query option for whether to observe blueooth to help location based services.
         *
         * @return enabled.
         */
        bool Environs::GetUseBtObserver ()
        {
            return environs::API::GetUseBtObserverN () != 0;
        }

		/**
		* Determines the interval for scanning of bluetooth devices.
		*
		* @param	interval  A millisecond value for scan intervals.
		*/
		void Environs::SetUseBtInterval ( int interval )
		{
			environs::API::SetUseBtIntervalN ( interval );
        }

        /**
         * Determines the interval for scanning of bluetooth devices.
         *
         * @param	interval  A millisecond value for scan intervals.
         */
        int Environs::GetUseBtInterval ()
        {
            return environs::API::GetUseBtIntervalN ();
        }
        
        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType_t.
         *
         * @return success true = available, false = not available.
         */
        bool Environs::IsSensorAvailable ( environs::SensorType_t sensorType )
        {
			return ( environs::API::IsSensorAvailableN ( hEnvirons, ( int ) sensorType ) != 0 );
        }


        /**
         * Set use of Tcp transport channel of the given sensorType.
         *
         * @param sensorType    A value of type environs::SensorType_t.
         * @param enable        true = TCP, false = UDP.
         *
         */
        void Environs::SetUseSensorChannelTcp ( environs::SensorType_t sensorType, bool enable )
        {
            environs::API::SetUseSensorChannelTcpN ( hEnvirons, ( int ) sensorType, enable );
        }


        /**
         * Get use of Tcp transport channel of the given sensorType.
         *
         * @param sensorType    A value of type environs::SensorType_t.
         * @return success      1 = TCP, 0 = UDP, -1 = error.
         *
         */
        int Environs::GetUseSensorChannelTcp ( environs::SensorType_t sensorType )
        {
            return environs::API::GetUseSensorChannelTcpN ( hEnvirons, ( int ) sensorType );
        }


        /**
         * Set sample rate of the given sensorType in microseconds.
         *
         * @param sensorType        A value of type environs::SensorType_t.
         * @param microseconds      The sensor sample rate in microseconds.
         *
         */
        void Environs::SetUseSensorRate ( environs::SensorType_t sensorType, int microseconds )
        {
            environs::API::SetUseSensorRateN ( hEnvirons, ( int ) sensorType, microseconds );
        }

        
        /**
         * Get sample rate of the given sensorType in microseconds.
         *
         * @param sensorType        A value of type environs::SensorType_t.
         *
         * @return microseconds     The sensor sample rate in microseconds. -1 means error.
         */
        int Environs::GetUseSensorRate ( environs::SensorType_t sensorType )
        {
            return environs::API::GetUseSensorRateN ( hEnvirons, ( int ) sensorType );
        }


        /**
         * Enable dispatching of sensor events from ourself.
         * Events are send if Environs instance is started stopped if the Environs instance has stopped.
         *
         * @param sensorType            A value of type environs.SensorType.
         * @param enable 				true = enable, false = disable.
         *
         * @return success true = enabled, false = failed.
         */
        bool Environs::SetSensorEvent ( environs::SensorType_t sensorType, bool enable )
        {
            CVerbArg2 ( "SetSensorEvent", "enable", "i", enable, "type", "i", sensorType );

			bool success = ( environs::API::SetSensorEventSenderN ( hEnvirons, 0, 0, ( int ) sensorType, enable ) ) != 0;

            if ( enable ) {
                StartSensorListeningN ( hEnvirons, ( int ) sensorType );
            }
            else {
                StopSensorListeningN ( hEnvirons, ( int ) sensorType );
            }

            return success;
        }


        /**
         * Enable sending of sensor events to this DeviceInstance.
         * Events are send if the device is connected and stopped if the device is disconnected.
         *
         * @param nativeID 				Destination native device id
         * @param objID 				Destination object device id
         * @param sensorType            A value of type environs.SensorType.
         * @param enable 				true = enable, false = disable.
         *
         * @return success true = enabled, false = failed.
        bool Environs::SetSensorEventSender ( int nativeID, int objID, environs::SensorType_t sensorType, bool enable )
        {
            CVerbArg2 ( "SetSensorEventSender", "nativeID", "i", nativeID, "type", "i", sensorType );
            
            bool success = (environs::API::SetSensorEventSenderN ( hEnvirons, nativeID, objID, ( int ) sensorType, enable ) ) != 0;
            
            if ( enable ) {
                StartSensorListeningN ( hEnvirons, ( int ) sensorType );
            }
            else {
                StopSensorListeningN ( hEnvirons, ( int ) sensorType );
            }
            
            return success;
        }
        
        
        void Environs::SetSensorEventSenderFlags ( int nativeID, int objID, int flags, bool enable )
        {
            CVerbArg2 ( "SetSensorEventSenderFlags", "nativeID", "i", nativeID, "flags", "i", flags );
            
            for ( int i=0; i<ENVIRONS_SENSOR_TYPE_MAX; ++i )
            {
                if ( ( flags & sensorFlags [ i ] ) != 0 ) {
                    SetSensorEventSender ( nativeID, objID, ( environs::SensorType_t ) i, enable );
                }
            }
         }
         */
        

		CString_ptr Environs::GetFilePathNative ( int nativeID, int fileID )
		{
            const char * t = environs::API::GetFilePathNativeN ( hEnvirons, nativeID, fileID );

            return ( t != nill ? CCharToString ( t ) : "" );
		}

		String_ptr Environs::GetFilePath ( int nativeID, int fileID )
		{
			//return environs::API::GetFilePathN ( hEnvirons, nativeID, fileID );
			return nill;
		}


		/**
		* Load the file that is assigned to the fileID received by deviceID into an byte array.
		*
		* @param fileID        The id of the file to load (given in the onData receiver).
		* @param size        An int pointer, that receives the size of the returned buffer.
		* @return byte-array
		*/
		UCharArray_ptr Environs::GetFile ( bool nativeIDOK, int nativeID, int deviceID, CString_ptr areaName, CString_ptr appName, int fileID, int OBJ_ref size )
		{
			CVerbVerb ( "GetFile" );

#ifdef CLI_CPP
			array<unsigned char> ^ buffer = nill;
			int capacity_ = 0;

			if ( nativeIDOK )
				environs::API::GetFileNativeN ( hEnvirons, nativeID, fileID, 0, &capacity_ );
			else
				environs::API::GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, 0, &capacity_ );

			if ( !capacity_ )
				return nill;

			buffer = gcnew array<unsigned char> ( capacity_ + 2 );
			if ( !buffer )
				return nill;

			pin_ptr<unsigned char> pBuffer = &buffer [ 0 ];
			size = capacity_;

			if ( nativeIDOK )
				environs::API::GetFileNativeN ( hEnvirons, nativeID, fileID, ( void * ) pBuffer, &capacity_ );
			else
				environs::API::GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, ( void * ) pBuffer, &capacity_ );

			size = capacity_;
			return buffer;
#else
			if ( nativeIDOK )
				return ( unsigned char * ) environs::API::GetFileNativeN ( hEnvirons, nativeID, fileID, 0, &size );

			return ( unsigned char * ) environs::API::GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, 0, &size );
#endif
		}


#ifndef CLI_CPP
        
		void * Environs::LoadPicture ( const char * filePath )
		{
#ifdef _WIN32
			HANDLE hFile = CreateFileA ( filePath,
				GENERIC_READ, 0, NULL,
				OPEN_EXISTING, 0, NULL );

			if ( hFile == INVALID_HANDLE_VALUE ) {
				CErrArg ( "LoadPicture: File [%s] not found.", filePath );
				return 0;
			}

			HGLOBAL			hMem		= nill;
			LPSTREAM		stream		= nill;
			LPPICTURE		picture		= nill;
			HBITMAP			hBmp		= nill;
			OLE_HANDLE		hOleJpg		= nill;
			DWORD			bytesRead	= 0;
			HRESULT			hr			= S_OK;

			do
			{
				DWORD fileSize = GetFileSize ( hFile, nill );
				if ( fileSize <= 0 )
				{
					CloseHandle ( hFile );
					CErrArg ( "LoadPicture: File size is invalid [%i].", fileSize );
					break;
				}

				LPVOID data = nill;

				// alloc memory based on file size
				hMem = GlobalAlloc ( GMEM_MOVEABLE, fileSize );
				if ( hMem == nill ) {
					CErrArg ( "LoadPicture: Failed to allocate memory of size [%i].", fileSize );
					break;
				}

				data = GlobalLock ( hMem );
				if ( data == NULL ) {
					CErr ( "LoadPicture: Failed to lock memory." );
					break;
				}

				BOOL read = ReadFile ( hFile, data, fileSize, &bytesRead, NULL );

				GlobalUnlock ( hMem );

				if ( !read || bytesRead != fileSize ) {
					CErrArg ( "LoadPicture: Failed to read file [ %u != &u ] .", bytesRead, fileSize );
					break;
				}
				 
				hr = CreateStreamOnHGlobal ( hMem, TRUE, &stream );
				if ( FAILED ( hr ) || stream == nill ) {
					CErr ( "LoadPicture: Failed to create stream." );
					break;
				}

				hr = ::OleLoadPicture ( stream, fileSize, FALSE, IID_IPicture, ( LPVOID * ) &picture );
				if ( FAILED ( hr )  || picture == nill ) {
					CErr ( "LoadPicture: Failed to Load picture from stream." );
					break;
				}

				hr = picture->get_Handle ( &hOleJpg );
				if ( FAILED ( hr ) )
				{
					CErr ( "LoadPicture: Failed to get bitmap handle." );
					break;
				}
				 
				hBmp = ( HBITMAP ) CopyImage ( ( HANDLE ) ( size_t ) hOleJpg, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG );
			}
			while ( false );

			if ( picture != nill )
				picture->Release ();

			if ( stream != nill )
				stream->Release ();

			if ( hMem != nill )
				GlobalFree ( hMem );

			CloseHandle ( hFile );

			return (void *) hBmp;
#else
            return nill;
#endif
		}
        
#endif
	}
}



