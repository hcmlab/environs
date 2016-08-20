/**
 * Notification Queue
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
//#   define DEBUGTYPES
#endif

#ifndef WINDOWS_PHONE
#	include <string.h>
#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>
#endif

#include "Notifications.h"
#include "Async.Worker.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Callbacks.h"
#include "Device/Devices.h"
#include "Portal/Portal.Device.h"

#define	CLASS_NAME 	"Notification.Queue . . ."


#ifdef DISPLAYDEVICE
#   define NOTIF_QUEUE_MAX_SIZE     8888
#else
#   define NOTIF_QUEUE_MAX_SIZE     4444
#endif


/* Namespace: environs -> */
namespace environs
{
    extern bool IsUIThread ();
    
    //NotificationQueue * notificationQueue = 0;

	NotificationQueue::NotificationQueue ()
	{
		CVerbN ( "Construct..." );
        
        isRunning           = false;
        hEnvirons           = 0;
		env					= 0;
        allocated			= false;
        
		active				= false;
        
        responseCounter		= 0;
        responseQueue		= 0;
        responseQueueLast	= 0;

		//locationUpdCache	= 0;

        Zero ( responseLock );
        
        CVerbN ( "Construct: done" );
	}


	NotificationQueue::~NotificationQueue ()
	{
		CLog ( "Destruct..." );
		
		Dispose ( );

        if ( allocated ) {
            allocated = false;
            
            LockDispose ( &responseLock );
        }

		CVerb ( "Destructed." );
    }
    
    
    bool NotificationQueue::Start ()
    {
        if ( thread.isRunning () ) {
            
            if ( isRunning ) {
                CWarn ( "Start: NotificationQueueWorker already running!" );
                return true;
            }
            
            thread.Join ( "NotificationQueue.Start" );
        }

		bool success = false;
        
		if ( !thread.Lock ( "Notification.Start" ) )
			return false;

		if ( !workerAliveSP )
		{
			workerAliveSP = env->myself;
			if ( workerAliveSP )
			{
				active = true;

				if ( !thread.isRunning () && !thread.Run ( pthread_make_routine ( &StartQueueWorker ), this, "NotificationQueue.Start" ) )
				{
					CErr ( "Start: Failed to create worker thread!" );

					active = false;
					workerAliveSP.reset ();
				}
				else 
					success = true;
			}
		}

		if ( !thread.Unlock ( "Notification.Start" ) )
			success = false;

		return success;
    }


	void NotificationQueue::Stop ()
	{
		active = false;
		
        thread.Notify ( "NotificationQueue::Stop", true );
        
        if ( !IsUIThread () )
            thread.Join ( "Notification Queue" );
        
        CVerbArg ( "Dispose: Elements in the queue %i. Disposing them now.", queue.size_ );
        
        thread.Lock ( "NotificationQueue::Dispose" );
        
        while ( queue.size_ > 0 ) {
            EnvironsNotify * cur = ( EnvironsNotify * ) queue.pop ();
            
            if ( cur )
                DisposeNotif ( cur );
        }
      
        thread.Unlock ( "NotificationQueue::Dispose" );
	}

    
    void NotificationQueue::DisposeNotif ( EnvironsNotify * notif )
    {
        CVerbVerb ( "DisposeNotif" );
        
		if ( notif->type == NOTIFY_TYPE_DATA || notif->type == NOTIFY_TYPE_MSG || notif->type == NOTIFY_TYPE_SEND )
        {
            EnvironsNotifyExt * dataNotif = (EnvironsNotifyExt *) notif;
            if ( dataNotif->payload )
                free ( dataNotif->payload );
        }

		if ( notif->contextSize && notif->contextPtr )
            free ( notif->contextPtr );
        
        free_n ( notif->areaName );

        if ( notif->appName != env->appName ) {
            free_n ( notif->appName );
        }
        
        free ( notif );
        
    }
    

	void NotificationQueue::Dispose ()
	{
		CVerb ( "Dispose" );

		DisposeResonses ( );

		Stop ();
	}


	bool NotificationQueue::Init ( Instance * obj )
	{		
		CVerb ( "Init" );
		
		if ( !obj )
			return false;
		env = obj;
		hEnvirons = env->hEnvirons;

        if ( !allocated ) {
            if ( !thread.Init () )
                return false;
            thread.autoreset = true;
            
            if ( !LockInit ( &responseLock ) )
                return false;
            
            allocated = true;
        }
        
		return true;
	}
    
    
	void NotificationQueue::AddToQueue ( EnvironsNotify * notif, bool useLock )
    {
		CVerbVerb ( "AddToQueue" );
        
		if ( active ) 
		{
			if ( useLock )
				thread.Lock ( "NotificationQueue::AddToQueue" );

			CVerbArg ( "AddToQueue: Elements in the queue [ %i ].", queue.size_ );

			// Append the notif
			if ( queue.push ( notif ) )
				notif = 0;

			thread.Notify ( "NotificationQueue::AddToQueue", false );

			if ( useLock )
				thread.Unlock ( "NotificationQueue::AddToQueue" );
		}

		if ( notif ) {
#ifndef NDEBUG
			CErrArg ( "AddToQueue: Dismissing notification queue packet. Queue size [ %i ]", queue.size_ );
#endif
			DisposeNotif ( notif );
		}
    }
    
    
    void BuildAppEnvirons ( char * &destAreaName, char * &destAppName, const char * areaName, const char * appName )
    {
		CVerbVerb ( "BuildAppEnvirons" );

		if ( areaName && *areaName ) {
			destAreaName = ( char * ) malloc ( MAX_LENGTH_AREA_NAME );
		}
		else destAreaName = 0;

		if ( appName && *appName ) {
			destAppName = ( char * ) malloc ( MAX_LENGTH_APP_NAME );
		}
		else destAppName = 0;
        
		CopyAppEnvirons ( destAreaName, destAppName, areaName, appName );
    }
    
    
	int CopyAppEnvirons ( char * destAreaName, char * destAppName, const char * areaName, const char * appName )
    {
		CVerbVerb ( "CopyAppEnvirons" );
        
		if ( destAreaName ) {
			if ( areaName && *areaName )
				strlcpy ( destAreaName, areaName, MAX_LENGTH_AREA_NAME );
			else
				*destAreaName = 0;
        }
        
        if ( destAppName ) {
			if ( appName && *appName )
				strlcpy ( destAppName, appName, MAX_LENGTH_APP_NAME );
			else
				*destAppName = 0;
        }
        return 1;
    }
    
    
    void NotificationQueue::Push ( OBJIDType objID, int notification )
    {
        Push ( objID, notification, objID > 0 ? SOURCE_DEVICE : SOURCE_NATIVE, NOTIFY_TYPE_NOTIFY );
    }
    
    
    void NotificationQueue::Push ( OBJIDType objID, int notification, int sourceIdent, int type )
    {
		CVerbVerb ( "Push" );

		if ( !active || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
            return;
        
        // Create a notif
        EnvironsNotify * notif = ( EnvironsNotify * ) calloc ( 1, sizeof ( EnvironsNotify ) );
        if ( !notif )
            return;
        
        notif->sourceID		= objID;
        notif->type			= type;
        notif->notification = notification;
        notif->sourceIdent	= sourceIdent;
        
        AddToQueue ( notif );
    }
    
    
    void NotificationQueue::PushContext ( OBJIDType objID, int notification, int sourceIdent, void * context, int contextSize, bool useLock )
    {
		CVerbVerb ( "PushContext" );

		if ( !active || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
            return;
        
        // Create a notif
        EnvironsNotify * notif = ( EnvironsNotify * ) calloc ( 1, sizeof ( EnvironsNotify ) );
        if ( !notif )
            return;
        
        notif->sourceID = objID;
        notif->type		= NOTIFY_TYPE_NOTIFY;
        
        if ( contextSize && context )
        {
            void * tmp = malloc ( contextSize + 1 );
            if ( !tmp ) {
                CErrArg ( "PushContext: Failed to allocate context of size [%i]", contextSize );
                free ( notif );
                return;
            }
            memcpy ( tmp, (void *) context, contextSize );

			/// Add a 0-terminator for possible cstring contexts
			( ( char * ) tmp ) [ contextSize ] = 0;
            
            notif->contextPtr = tmp;
        }
        
        notif->contextSize = contextSize;
        
        notif->notification = notification;
        notif->sourceIdent = sourceIdent;
        
        AddToQueue ( notif, useLock );
    }
    
    
    void NotificationQueue::PushData ( OBJIDType objID, int nativeID, int sourceIdent, int fileID, const char * descriptor, unsigned int descLen, unsigned int fileSize )
    {
        CVerbVerbIDN ( "PushData" );

		if ( !active || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
			return;
        
        // Create a notif
        EnvironsNotifyExt * notif = ( EnvironsNotifyExt * ) calloc ( 1, sizeof ( EnvironsNotifyExt ) );
        if ( !notif )
            return;
        
        char * desc = 0;
        if ( descriptor ) {
            size_t len = (size_t) descLen;
            if ( len > 0 ) {
                desc = (char *) malloc ( len + 1 );
                if ( desc ) {
                    memcpy ( desc, descriptor, len );
                    desc [ len ] = 0;
                }
            }
        }
        
        notif->sourceID		= objID;
        notif->type			= NOTIFY_TYPE_DATA;
        notif->sourceIdent	= sourceIdent;
        notif->fileID		= fileID;
		notif->notification	= nativeID;
        
        notif->payload		= desc;
        notif->context		= fileSize;
        
        AddToQueue ( (EnvironsNotify *) notif );
    }
    
    
    void NotificationQueue::PushContext ( OBJIDType deviceID, const char * areaName, const char * appName, int notification, void * context, int contextSize )
    {
        CVerbVerbID ( "PushContext" );

		if ( !active || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
            return;
        
        // Create a notif
        EnvironsNotify * notif = ( EnvironsNotify * ) calloc ( 1, sizeof ( EnvironsNotify ) );
        if ( !notif )
            return;
        
        notif->sourceID	= deviceID;
        notif->type		= NOTIFY_TYPE_NOTIFY;
        
        BuildAppEnvirons ( notif->areaName, notif->appName, areaName, appName );
        
        if ( !notif->appName )
            notif->appName = env->appName;
        
        if ( !contextSize )
            notif->contextPtr = context;
        else {
            void * tmp = malloc ( contextSize );
            if ( !tmp ) {
                CErrArgID ( "PushContext: Failed to allocate context of size [%i]", contextSize );
                free ( notif );
                return;
            }
            memcpy ( tmp, (void *) context, contextSize );
            
            notif->contextSize = contextSize;
            notif->contextPtr = tmp;
        }
        notif->notification = notification;
        
        AddToQueue ( notif );
    }
    
    
    void NotificationQueue::Push ( OBJIDType nativeID, int sourceIdent, const void * buffer, int length )
    {
        CVerbVerbIDN ( "Push" );

		if ( !active || !buffer || queue.size_ > NOTIF_QUEUE_MAX_SIZE || length <= 0 )
			return;

		char * payload = ( char * ) malloc ( length + 2 );
		if ( !payload )
			return;

		memcpy ( payload, buffer, length );

		// Create a notif
		EnvironsNotifyExt * notif = ( EnvironsNotifyExt * ) calloc ( 1, sizeof ( EnvironsNotifyExt ) );
		if ( !notif ) {
			free ( payload );
			return;
		}

		notif->sourceID		= nativeID;
		notif->type			= NOTIFY_TYPE_MSG;
		notif->sourceIdent	= sourceIdent;
		notif->context		= length;
		notif->payload		= payload;

		// Append 0 zero bytes (make sure that something trying to read the buffer as a string does find the zeros
		payload += length; *payload++ = 0; *payload++ = 0;
        
        AddToQueue ( (EnvironsNotify *) notif );
    }
    
    
    void NotificationQueue::Push ( OBJIDType nativeID, int sourceIdent, bool comDat, char msgType, unsigned short payloadTyp, const void * payload, unsigned int payloadSize )
    {
        CVerbVerbArgIDN ( "Push: send payload of size [ %u ] type [ %c ] over [ %s ]", payloadSize, msgType, comDat ? "ComDat" : "Interact" );

        if ( !active || !payload || queue.size_ > NOTIF_QUEUE_MAX_SIZE || !payloadSize ) {
            CErr ( "Push: Invalid arguments." );
            return;
        }

		char * sendBuffer = (char *) malloc ( payloadSize + 1 );
        if ( !sendBuffer )
			return;

		memcpy ( sendBuffer, payload, payloadSize );
		sendBuffer [ payloadSize ] = 0;

		// Create a notif
		EnvironsNotifyExt * notif = (EnvironsNotifyExt *) calloc ( 1, sizeof ( EnvironsNotifyExt ) );
		if ( !notif ) {
			free ( sendBuffer );
			return;
		}
        
        notif->sourceID		= nativeID;
        notif->type			= NOTIFY_TYPE_SEND;
        notif->sourceIdent	= sourceIdent;
        notif->context		= payloadSize;
        notif->notification = msgType | (comDat << 8) | ( payloadTyp << 16 );
        notif->payload		= sendBuffer;
        
        AddToQueue ( (EnvironsNotify *) notif );
    }
    
    
    
    void NotificationQueue::Push ( int deviceID, const char * areaName, const char * appName, int notification, int sourceIdent )
    {
        CVerbVerbID ( "Push" );

		if ( !active || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
            return;
        
        // Create a notif
        EnvironsNotify * notif = ( EnvironsNotify * ) calloc ( 1, sizeof ( EnvironsNotify ) );
        if ( !notif )
            return;
        
        BuildAppEnvirons ( notif->areaName, notif->appName, areaName, appName );
        
        if ( !notif->appName )
            notif->appName = env->appName;
        
        notif->sourceID		= deviceID;
        notif->notification = notification;
        notif->sourceIdent	= sourceIdent;
        
        AddToQueue ( notif );
    }
    
    
    void NotificationQueue::Push ( int deviceID, const char * areaName, const char * appName, int sourceIdent, const void * buffer, int length, const char * prefix )
    {
        CVerbVerbID ( "Push" );

		if ( !active || !buffer || queue.size_ > NOTIF_QUEUE_MAX_SIZE )
            return;
        
        if ( length <= 0 )
            return;

		char * payload = ( char * ) malloc ( length + 2 );
		if ( !payload )
			return;
        
        void * contextPtr = calloc ( 1, 4 );
        if ( !contextPtr ) {
            free ( payload );
            return;
        }
        memcpy ( contextPtr, prefix, 2 );

		memcpy ( payload, buffer, length );

		// Create a notif
		EnvironsNotifyExt * notif = ( EnvironsNotifyExt * ) calloc ( 1, sizeof ( EnvironsNotifyExt ) );
        if ( !notif ) {
            free ( contextPtr );
			free ( payload );
			return;
		}

        //if ( IsSameAppEnv ( env, appName, areaName ) ) {
        //    notif->appName = env->appName;
        //}
        //else
            BuildAppEnvirons ( notif->areaName, notif->appName, areaName, appName );

        if ( !notif->appName )
            notif->appName = env->appName;
        
		notif->sourceID     = deviceID;
		notif->type         = NOTIFY_TYPE_MSG;
		notif->context      = length;
        notif->sourceIdent  = sourceIdent;
        
        // If we add a prefix, then the message will be stored into the messagestore
        notif->contextPtr   = contextPtr;
        notif->contextSize  = 2;

		notif->payload      = payload;

		// Append 0 zero bytes (make sure that something trying to read the buffer as a string does find the zeros
		payload += length; *payload++ = 0; *payload++ = 0;
        
        AddToQueue ( (EnvironsNotify *) notif );
    }
    
	
	EnvironsNotify * NotificationQueue::Pop ()
	{
		CVerbVerb ( "Pop" );

		if ( !active )
			return 0;

		EnvironsNotify * notif = 0;

        thread.Lock ( "NotificationQueue.Pop" );
		
		//CLogArg ( "Push: Elements in the queue %i.", size );

		// Remove a notif
		notif = ( EnvironsNotify * ) queue.pop ();

        thread.Unlock ( "NotificationQueue.Pop" );

		return notif;
	}

	
	void * NotificationQueue::StartQueueWorker ( void * arg )
	{
        NotificationQueue * worker = (NotificationQueue *) arg;
        
		worker->Worker ();
		
		worker->thread.Lock ( "Notification.StartQueueWorker" );

        worker->workerAliveSP.reset ();

		worker->thread.Unlock ( "Notification.StartQueueWorker" );
        
        return 0;
	}
	

	char * buildMessage ( const char * format, ... )
	{
		static char buffer[1024];
		va_list marker;

		va_start ( marker, format );
		vsnprintf_s ( buffer, 1024, _TRUNCATE, format, marker );
		va_end ( marker );
		return buffer;
	}


	void NotificationQueue::WorkTextNotify ( EnvironsNotify * notif, int deviceID, int notifyID )
	{
		unsigned int notifType = (notifyID & 0x00FF0000) >> 16;
		const char * msg = 0;

		switch ( notifType )
		{
			case MSG_TYPE_HELO:
				switch ( notifyID )
				{
				case NOTIFY_CONNECTION_ESTABLISHED:
					msg = buildMessage ( "[%i] Connection established.", deviceID );
                        
                        // Send ack to connected device
					break;
				case NOTIFY_CONNECTION_MAIN_FAILED:
					msg = buildMessage ( "[%i] Failed to connect.", deviceID );
					break;
				case NOTIFY_CONNECTION_MAIN_ACK:
					msg = buildMessage ( "[%i] Interact channel ready.", deviceID );
					break;
				case NOTIFY_CONNECTION_DATA_ACK:
					msg = buildMessage ( "[%i] Data channel ready.", deviceID );
					break;
				case NOTIFY_CONNECTION_COMDAT_ACK:
					msg = buildMessage ( "[%i] ComDat channel  ready.", deviceID );
					break;
				case NOTIFY_CONNECTION_COMDAT_FAILED:
					msg = buildMessage ( "[%i] Connecting ComDat channel failed.", deviceID );
					break;
				case NOTIFY_CONNECTION_COMDAT_CLOSED:
					msg = buildMessage ( "[%i] ComDat channel closed.", deviceID );
					break;
				case NOTIFY_CONNECTION_CLOSED:
					msg = buildMessage ( "[%i] Com channel closed.", deviceID );
					break;
				}
				break;
					
			case MSG_TYPE_FILE:
				break;
					
			case MSG_TYPE_PORTAL:
				switch ( notifyID )
				{
				case NOTIFY_PORTAL_STREAM_STARTED:
					msg = buildMessage ( "[%i] %s portal started.", deviceID, ( notif->sourceIdent & PORTAL_DIR_INCOMING ) ? "Incoming" : "Outgoing" );
					break;
				case NOTIFY_PORTAL_PROVIDE_STREAM_ACK:
					msg = buildMessage ( "[%i] Outgoing Portal initiated.", deviceID );
					break;
				case NOTIFY_PORTAL_PROVIDE_IMAGES_ACK:
					msg = buildMessage ( "[%i] Outgoing portal using images.", deviceID );
					break;
				case NOTIFY_PORTAL_STREAM_PAUSED:
					msg = buildMessage ( "[%i] %s portal paused.", deviceID, ( notif->sourceIdent & PORTAL_DIR_INCOMING ) ? "Incoming" : "Outgoing" );
					break;
				case NOTIFY_PORTAL_STREAM_STOPPED:
					msg = buildMessage ( "[%i] %s portal stopped.", deviceID, ( notif->sourceIdent & PORTAL_DIR_INCOMING ) ? "Incoming" : "Outgoing" );
					break;
				}
				break;
					
			case MSG_TYPE_ENVIRONS:
				{
					notifType = notifyID & 0xFFFF00;
					switch ( notifType ) {
						case NOTIFY_START:
							switch ( notifyID )
							{
							case NOTIFY_START_IN_PROGRESS:
								msg = "Starting Environs.";
								break;
							case NOTIFY_START_ENABLING_WIFI:
								msg = "Enabling WiFi.";
								break;
							case NOTIFY_START_STREAM_DECODER:
								msg = "Initializing Stream decoder.";
								break;
							case NOTIFY_START_INIT:
								msg = "Initializing Environs.";
								break;
							case NOTIFY_START_INIT_FAILED:
								msg = "Initializing Environs failed.";
								break;
							case NOTIFY_START_METHOD_FAILED:
								msg = "Starting Environs failed.";
								break;
							case NOTIFY_START_DECODER_FAILED:
								msg = "Initializing Stream decoder failed.";
								break;
							case NOTIFY_START_WIFI_FAILED:
								msg = "Enabling WiFi failed.";
								break;
							case NOTIFY_START_FAILED:
								msg = "Initializing Environs failed.";
								break;
							case NOTIFY_START_INIT_SUCCESS:
								msg = "Environs initialized.";
								break;
                            case NOTIFY_START_SUCCESS:
                                msg = "Environs started.";
                                break;
							case NOTIFY_START_LISTEN_SUCCESS:
								msg = "Environs communication online.";
								break;
							case NOTIFY_START_LISTENDA_SUCCESS:
								msg = "Data listener online.";
								break;
							}
							break;

						case NOTIFY_STOP:
							switch ( notifyID )
							{
							case NOTIFY_STOP_IN_PROGRESS:
								msg = "Stopping Environs.";
								break;
							case NOTIFY_STOP_FAILED:
								msg = "Environs failed to stop.";
								break;
							case NOTIFY_STOP_SUCCESS:
								msg = "Environs stopped.";
								break;
							case NOTIFY_STOP_RELEASED:
								msg = "Environs resources disposed.";
								break;
							}
							break;

						case NOTIFY_SOCKET:
							switch ( notifyID )
							{
							case NOTIFY_SOCKET_BIND_FAILED:
								msg = buildMessage ( "Socket bind on port %i failed.", deviceID );
								break;
							case NOTIFY_SOCKET_LISTEN_FAILED:
								msg = buildMessage ( "Socket listen on port %i failed.", deviceID );
								break;
							case NOTIFY_SOCKET_FAILED:
								msg = "Environs stopped.";
								break;
							}
							break;
					}
				}
				break;
		}

		if ( msg )
			env->callbacks.OnStatusMessage ( hEnvirons, msg );
	}

    
    void NotificationQueue::WorkNotify ( EnvironsNotify * notif, int deviceID, int notifyID )
    {
        unsigned int notifType = (notifyID & 0x00FF0000) >> 16;
        const char * msg = 0;
        
        switch ( notifType )
        {                
            case MSG_TYPE_ENVIRONS:
            {
                notifType = notifyID & 0xFFFF00;
                switch ( notifType ) {
                    case NOTIFY_MEDIATOR:
                    case NOTIFY_MEDIATOR_SERVER:
                        switch ( notifyID )
                    {
                        case NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ:
                            if ( ( env->useCustomMediator || env->useDefaultMediator) )
								env->asyncWorker.Push ( environs::Source::Native, ASYNCWORK_TYPE_RENEW_STUNT_SOCKETS );
                            break;
                            
                        case NOTIFY_MEDIATOR_SERVER_CONNECTED:
						{
							if ( ( env->useCustomMediator || env->useDefaultMediator ) )
                            {
                                sp ( MediatorClient ) mediator = env->mediator MED_WP;
								if ( mediator ) {
									//env->asyncWorker.Push ( environs::Source::Native, ASYNCWORK_TYPE_DEVICE_FLAGS_PUSH );

									mediator->RegisterAtMediators ( false ); /// register stunt socket / try register at mediator again
								}
							}
							msg = "Mediator connected.";
							break;
						}
                            
                        case NOTIFY_MEDIATOR_SERVER_DISCONNECTED:
                        {
                            //	//CLog ( "WorkTextNotify: NOTIFY_MEDIATOR_SERVER_*" );
                            sp ( MediatorClient ) mediator = env->mediator MED_WP;
                            
                            if ( mediator && env->environsState >= environs::Status::Starting )
                            {
                                if ( env->deviceID == 0 )
                                {
                                    if ( mediator->connectFails > 0 )
                                    {
                                        // We seem to have problems connecting to a Mediator and the deviceID has not been set by the application.
                                        // Let's randomly pick one
                                        Core::GenerateRandomDeviceID ( env );
                                        
                                        mediator->SendBroadcast ();
                                    }
                                }
                                
                                mediator->RegisterAtMediators ( false ); /// register spare socket / try register at mediator again
                            }
                            
                            msg = "Mediator disconnected.";
                            break;
                        }
                            
                            
                        case NOTIFY_MEDIATOR_DEVICE_REMOVED:
                        case NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED:
                        {
                            // Cancel any ongoing stunt connects by this device
                            DeviceBase * device = GetDevice ( env, (int) notif->sourceID, notif->areaName, notif->appName );
                            
                            if ( device ) {
                                CVerbArg ( "WorkNotify: Device [ 0x%X / %s / %s ] needs to be disposed ...", notif->sourceID, notif->areaName ? notif->areaName : "",
                                         notif->appName ? notif->appName : "" );
                                
                                device->deviceStatus = DeviceStatus::Deleteable;
                                
                                UnlockDevice ( device );
                                
                                TriggerCleanUpDevices ();
                            }
                            
                            break;
                        }
                            
                    }
                        break;
                }
                break;
            }
                
            case MSG_TYPE_OPTIONS:
            {
				switch ( notifyID ) {
					case NOTIFY_PORTAL_LOCATION_CHANGED:
					case NOTIFY_PORTAL_SIZE_CHANGED:
					{
						/// Only notify the destination (if we are the source)
						unsigned int portalID = notif->sourceIdent;

						if ( IsPortalGenerator () )
                        {
                            DeviceBase * device = GetDeviceIncLock ( portalID );
                            if ( device )
                            {
                                device->SyncPortalInfo ( portalID );
                                
                                UnlockDevice ( device );
                            }
						}
					}
					break;
                }
                break;
            }
        }
        
		if ( msg && env->callbacks.doOnStatusMessage )
			env->callbacks.OnStatusMessage ( hEnvirons, msg );
    }
    

	void * NotificationQueue::Worker ()
	{
        CVerb ( "Worker thread started..." );
        
        isRunning = true;

        pthread_setname_current_envthread ( "NotificationQueue::Worker" );
        
		while ( active )
		{
			// pop a notification
			EnvironsNotify * notif = Pop ();
			if ( !notif ) {
                if ( !thread.LockCond ( "NotificationQueue::Worker" ) )
                    break;
                
                if ( active )
                    thread.WaitLocked ( "NotificationQueue::Worker" );
                
                if ( !thread.UnlockCond ( "NotificationQueue::Worker" ) )
                    break;
				continue;
			}

			// process the notification
			int     notifyID   = notif->notification;
			int     deviceID   = (int) notif->sourceID;

#ifdef DEBUGTYPES
			CLogArgID ( "Notify: [%s]", resolveName(notifyID) );
#endif

            if ( notif->type == NOTIFY_TYPE_NOTIFY )
            {
                WorkNotify ( notif, deviceID, notifyID );
                
				if ( env->callbacks.doOnStatusMessage )
                    WorkTextNotify ( notif, deviceID, notifyID );
                
                //CLogArgID ( "Notify: [%s] [%s/%s]", resolveName(notifyID), notif->areaName ? notif->areaName : "-", notif->appName ? notif->appName : "-"  );
                
                if ( active )
                {
                    if ( notif->appName ) {
                        if ( notif->appName == env->appName )
                            notif->appName = 0;
                        
                        env->callbacks.OnNotifyExt ( hEnvirons, deviceID, notif->areaName, notif->appName, notifyID, notif->sourceIdent, notif->contextPtr );
                    }
                    else
						env->callbacks.OnNotify ( hEnvirons, deviceID, notifyID, notif->sourceIdent, notif->contextPtr, notif->contextSize );
                }
			}
			else if ( notif->type == NOTIFY_TYPE_STATUS )
			{
                if ( active ) {
					env->callbacks.OnNotify ( hEnvirons, deviceID, notifyID, notif->sourceIdent, notif->contextPtr, notif->contextSize );
                }
			}
			else if ( notif->type == NOTIFY_TYPE_DATA )
			{
				EnvironsNotifyExt * dataNotif = (EnvironsNotifyExt *) notif;
				if ( active )
					env->callbacks.OnData ( hEnvirons, dataNotif->sourceID, dataNotif->notification, dataNotif->sourceIdent, dataNotif->fileID, (const char *) dataNotif->payload, dataNotif->context );
			}
			else if ( notif->type == NOTIFY_TYPE_MSG )
			{
				EnvironsNotifyExt * dataNotif = (EnvironsNotifyExt *) notif;
                if ( active )
                {
                    if ( dataNotif->contextSize == 2 && dataNotif->contextPtr )
                    {
                        DeviceBase::SaveToStorageMessages ( env, (char *) dataNotif->contextPtr, deviceID, notif->areaName, dataNotif->appName == env->appName ? 0 : dataNotif->appName, (const char *) dataNotif->payload, dataNotif->context );
                    }
                    
					if ( env->environsState >= environs::Status::Started )
					{
						if ( notif->appName )
						{
                            if ( notif->appName == env->appName )
                                notif->appName = 0;
                            
                            // Let's try looking for the DeviceInstanceNode
                            sp ( MediatorClient ) mediator = env->mediator MED_WP;
							if ( mediator )
							{
								sp ( DeviceInstanceNode ) device = mediator->GetDeviceSP ( deviceID, notif->areaName, notif->appName );
								if ( device )
                                {
                                    if (device->info.flags & (unsigned short) DeviceFlagsInternal::PlatformReady)
                                        env->callbacks.OnMessage ( hEnvirons, device->info.objID, dataNotif->sourceIdent, dataNotif->payload, dataNotif->context );
									else {
                                        CVerb ( "Worker: Skipping delivery of message due to PLATFORM NOT READY." );
                                    }
#ifdef ENABLE_MESSAGE_EXT_DISPATCH
									sendExt = false;
#endif
                                }
                                else { CVerb ( "Worker: Skipping delivery of message due to MISSING DEVICE." ); }
                            }
                            else { CVerb ( "Worker: Skipping delivery of message due to MISSING MEDIATOR." ); }
                            
#ifdef ENABLE_MESSAGE_EXT_DISPATCH
							if ( sendExt )
								env->callbacks.OnMessageExt ( hEnvirons, deviceID, notif->areaName, notif->appName, dataNotif->sourceIdent, dataNotif->payload, dataNotif->context );
#endif
						}
                        else {
							env->callbacks.OnMessage ( hEnvirons, dataNotif->sourceID, dataNotif->sourceIdent, dataNotif->payload, dataNotif->context );
                        }
					}
					else { CVerb ( "Worker: Skipping delivery of message due to INSTANCE < STARTED." ); }
                }
			}
			else if ( notif->type == NOTIFY_TYPE_SEND )
			{
				EnvironsNotifyExt * extNotif = (EnvironsNotifyExt *) notif;
				if ( active ) {
					DeviceBase * device = GetDevice ( (int) extNotif->sourceID );
					if ( device )
					{
						CVerbArgID ( "Worker: send payload of size [ %u ] type [ %c ] over [ %s ]", extNotif->context, extNotif->notification & 0xFF, (((extNotif->notification >> 8) & 0xFF) != 0) ? "ComDat" : "Interact" );

						if ( device->SendBuffer ( ((extNotif->notification >> 8) & 0xFF) != 0, extNotif->notification & 0xFF,
							extNotif->sourceIdent, 0, (extNotif->notification >> 16) & 0xFFFF, (char *) extNotif->payload, extNotif->context ) <= 0 ) {
							CWarnID ( "Worker: Failed to async send payload to the device." );
						}
						UnlockDevice ( device );
					}
				}
			}
            
            DisposeNotif ( notif );
		}
		
	//Finish:
		CLog ( "Worker: bye bye..." );
        isRunning = false;
		return 0;
	}


	void NotificationQueue::DisposeResonses ()
	{
		CVerbVerb ( "DisposeResonses" );

		LockAcquireVA ( responseLock, "DisposeResonses" );

        EnvironsRepsonse * er = responseQueue;

        while ( er ) {
			if ( pthread_cond_signal ( &er->responseEvent ) ) {
				CErr ( "DisposeResonses: Failed to signal responseEvent!" );
			}

			EnvironsRepsonse * toDelete = er;
            er = er->next;

			free ( toDelete );
        }

		responseQueue = 0;
		responseQueueLast = 0;

		LockReleaseVA ( responseLock, "DisposeResonses" );
	}
    
    
    unsigned int NotificationQueue::GetResponse ( int nativeID, unsigned short optionID, unsigned int resultCapacity, void * resultBuffer )
    {
        CVerb ( "GetResponse" );
        
        /*struct timespec   ts;
         struct timeval    timeNow;*/
        //int rc;
        
		unsigned int ret = 0;
		unsigned int responses [ 2 ];
		unsigned int length = sizeof ( unsigned int );

		EnvironsRepsonse * response = ( EnvironsRepsonse * ) calloc ( 1, sizeof ( EnvironsRepsonse ) );
		if ( !response ) {
			return 0;
		}

		response->deviceID		= nativeID;
		response->response		= resultBuffer;
		response->responseBytes = resultCapacity;

		if ( !CondInit ( &response->responseEvent ) )
			goto Finish;
        
        CVerbArg ( "GetResponse: resultCapacity [ %i ]", resultCapacity );

		LockAcquireVA ( responseLock, "GetResponse" );
        
        responses [0] = ++responseCounter;
        response->responseID = responses [0];
        
        if ( !PushResponse ( response ) )
            goto Finish;
        
        if ( optionID == MSG_OPT_PORTAL_INFO_GET ) {
            /// For portal infos we need to include the portalID
            responses [1] = *((unsigned int *)resultBuffer);
            length += sizeof(unsigned int);
        }
        
        if ( !DeviceBase::SendTcpBuffer ( nativeID, true, MSG_TYPE_OPTIONS, optionID, &responses, length ) ) {
            RemoveResponse ( responses [0] );
            goto Finish;
        }
        
        //gettimeofday ( &timeNow, NULL );
        
        /*ts.tv_sec  = timeNow.tv_sec;
         ts.tv_nsec = timeNow.tv_usec * 1000;
         ts.tv_sec += 1;*/
        
        //rc = pthread_cond_timedwait ( &er->responseEvent, &responseMutex, &ts );
        pthread_cond_wait ( &response->responseEvent, &responseLock );
        
        // remove er from queue
        if ( !RemoveResponse ( responses [0] ) ) {
            // If it's not in the queue anymore, then don't touch er any longer (memory has already been disposed by higher forces...)
            response = 0;
            goto Finish;
        }
        CVerbArg ( "GetResponse: response size [ %i ]", response->responseBytes );
        
        /*if ( ETIMEDOUT == rc )
         goto Finish;*/
        
        ret = response->responseBytes;
        
    Finish:
        if ( response ) {
            if ( pthread_cond_valid ( response->responseEvent ) && pthread_cond_destroy ( &response->responseEvent ) ) {
                CErr ( "GetResponse: Failed to destroy responseEvent." );
            }
            free ( response );
        }

		LockReleaseVA ( responseLock, "GetResponse" );
        return ret;
    }
    
    
	bool NotificationQueue::PushResponse ( EnvironsRepsonse * er )
	{
		CVerb ( "PushResponse" );
        
		if ( !active )
			return false;

		// Append the notif
		if ( !responseQueue ) {
			responseQueue = er;
			responseQueueLast = er;
		}
		else if ( responseQueueLast ) {
			responseQueueLast->next = er;
			responseQueueLast = er;
		}
        else
            return false;
        
        return true;
	}
    

	bool NotificationQueue::RemoveResponse ( unsigned int responseID )
	{
		CVerb ( "RemoveResponse" );
        
        EnvironsRepsonse * erp = 0;
        EnvironsRepsonse * er = responseQueue;
        
        while ( er ) {
            if ( er->responseID == responseID ) {
                if ( erp ) {
                    erp->next = er->next;
					if ( er == responseQueueLast ) {
						responseQueueLast = erp;
					}
				}
                else {
                    responseQueue = 0;
                    responseQueueLast = 0;
                }
                
                return true;
            }
            erp = er;
            er = erp->next;
        }
        
        return false;
	}


	void NotificationQueue::HandleResponse ( unsigned int payloadSize, char * payload )
	{
		CVerb ( "HandleResponse" );
        
		if ( !active || !payload || !payloadSize )
			return;

		CVerbArg ( "HandleResponse: payloadSize [%i]", payloadSize );

		payloadSize -= 4;
		/*if ( payloadSize > 16 )
			payloadSize = 16;*/

		unsigned int responseID = *((unsigned int *) payload);
		CVerbArg ( "HandleResponse: responseID [%i]", responseID );
		
		if ( !LockAcquireA ( responseLock, "HandleResponse" ) )
			return;

        EnvironsRepsonse * er = responseQueue;
        while ( er ) {
			CVerbArg ( "HandleResponse: responseID [%i]", er->responseID );
            if ( er->responseID == responseID ) {
				CVerb ( "HandleResponse: found" );
                break;
            }
            er = er->next;
        }

		if ( er )
		{
			CVerbArg ( "HandleResponse: response entry [%i] found.", er->responseID );

			// Make sure that..
			if ( payloadSize > 0		// we have received something
				&& payloadSize <= er->responseBytes // the payload fits into the result buffer
				)
			{
				CVerbArg ( "HandleResponse: response 1 [%i] size [%i]", *((unsigned int *)(payload + 4)), payloadSize );

				er->responseBytes = payloadSize;
				memcpy ( er->response, payload + 4, payloadSize );
			}

			if ( pthread_cond_signal ( &er->responseEvent ) ) {
				CErr ( "HandleResponse: Failed to signal responseEvent!" );
			}
		}
		else {
			CVerbArg ( "HandleResponse: response entry [%i] NOT found.", responseID );
		}

		LockReleaseA ( responseLock, "HandleResponse" );
	}

}
