/**
 * Async Worker
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
//#   define DEBUGDISPOSESEND
#endif

#ifdef DEBUGDISPOSESEND
#   define  CDisend(a)          CLog(a)
#   define  CDisendArg(a,...)   CLogArg(a,__VA_ARGS__)
#else
#   define  CDisend(a)          CVerbVerb(a)
#   define  CDisendArg(a,...)   CVerbVerbArg(a,__VA_ARGS__)
#endif

#ifndef WINDOWS_PHONE
#	include <string.h>
#	include <stdio.h>
#	include <stdarg.h>
#endif
#include "Async.Worker.h"
#include "Notifications.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Release.h"
#include "Callbacks.h"
#include "Device/Device.Controller.h"
#include "Portal/Portal.Device.h"


#define	CLASS_NAME 	"Async.Worker . . . . . ."


#ifdef DISPLAYDEVICE
#   define WORKER_QUEUE_MAX_SIZE	8888
#   define WORKER_SEND_MAX_SIZE     8888
#else
#   define WORKER_QUEUE_MAX_SIZE	4444
#   define WORKER_SEND_MAX_SIZE     4444
#endif

/* Namespace: environs -> */
namespace environs
{
	AsyncWorker * asyncWorker = 0;
    

	AsyncWorker::AsyncWorker ()
	{
		CVerbN ( "Construct..." );

        allocated       = false;
        active			= false;
        env				= 0;
		sendEnabled		= false;
        sendThreadsRunning = 0;
        
        isRunning       = false;
        
        isSendRunning   = false;
        deviceStatus    = DeviceStatus::ConnectInProgress;
        
        CVerbN ( "Construct: done" );
	}


	AsyncWorker::~AsyncWorker ()
	{
        CVerb ( "Destruct..." );
        
        Dispose ();
        
		CVerb ( "Destructed." );
    }


	INLINEFUNC void DisposeAsyncWork ( AsyncWork  * work )
	{
		if ( work->context )
			free ( work->context );
		free ( work );
	}

    
    bool AsyncWorker::Init ( Instance * obj )
    {
        CVerb ( "Init" );

		if ( !obj )
			return false;
		env = obj;
        
		if ( !allocated )
        {
            if ( !thread.Init () )
                return false;
            thread.autoreset = true;
            
            for ( int i = 0; i < ASYNCWORK_SEND_POOL_SIZE; i++ )
            {
                if ( !sendThreads [ i ].Init () )
                    return false;
            }
            
            if ( !sendThreadsSync.Init () )
                return false;
            sendThreadsSync.autoreset = true;
            
            allocated = true;
        }
        return true;
    }


	bool AsyncWorker::GetIsActive ()
	{
		return active;
	}

    
    bool AsyncWorker::Start ()
    {
        if ( thread.isRunning () )
        {
            if ( isRunning ) {
                CWarn ( "Start: AsyncWorker already running!" );
                return true;
            }
            thread.Join ( "AsyncWorker.Start" );
        }

		active		= true;

		if ( thread.Lock ( "AsyncWorker.Start" ) )
		{
			if ( !workerAlive )
			{
				workerAlive = env->myself;
				if ( workerAlive )
				{
					if ( !thread.Run ( pthread_make_routine ( &StartQueue ), this, "AsyncWorker.Start", false ) )
					{
						CErr ( "Start: Failed to create worker thread." );
						active	= false;
						workerAlive = 0;
					}
					else {
						sendEnabled = isRunning = true;
					}
				}
				else active = false;
			}

			thread.Unlock ( "AsyncWorker.Start" );
		}

		if ( active && sendThreadsSync.Lock ( "AsyncWorker.Start" ) )
		{
			StartSend ( 0 );

			sendThreadsSync.Unlock ( "AsyncWorker.Start" );
		}
        
		return active;
    }
    
    
    void AsyncWorker::Stop ()
    {
        active = false;
        
        thread.Notify ( "AsyncWorker.Stop" );
        
        StopSend ();
        
        thread.Join ( "AsyncWorker.Stop" );
        
        CVerbArg ( "Dispose: Elements in the queue %i. Disposing them now.", queue.size_ );
        
        thread.Lock ( "AsyncWorker.Dispose" );
        
        while ( queue.size_ > 0 ) {
            AsyncWork * cur = ( AsyncWork * ) queue.pop ();
            
            if ( cur )
                DisposeAsyncWork ( cur );
        }
        
        thread.Unlock ( "AsyncWorker.Dispose" );
    }
    
    
    void AsyncWorker::Dispose ()
    {
        CVerb ( "Dispose" );
        
        Stop ();
    }
    
    
    void AsyncWorker::Push ( AsyncWork * work )
    {
        if ( !work )
            return;
        
        CVerbVerb ( "Push" );

		if ( active && queue.size_ < WORKER_QUEUE_MAX_SIZE && thread.Lock ( "Push" ) )
		{
			CVerbVerbArg ( "Push: Elements in the queue %i.", queue.size_ );

			// Append the notif
			if ( queue.push ( work ) )
				work = 0;

			if ( !thread.Notify ( "Push", false ) ) {
				CErr ( "Push: Failed to signal queueEvent!" );
			}

			thread.Unlock ( "Push" );
		}

		if ( work ) {
#ifndef NDEBUG
			CErrArg ( "Push: Dismissing async work packet. Queue size [ %i ]", queue.size_ );
#endif
			DisposeAsyncWork ( work );
		}
    }
    
    
    void AsyncWorker::Push ( int deviceID, unsigned int workType )
    {
        CVerbVerbID ( "Push" );

		if ( !active || queue.size_ > WORKER_QUEUE_MAX_SIZE )
            return;
        
        // Create a notif
        AsyncWork * work = ( AsyncWork * ) calloc ( 1, sizeof ( AsyncWork ) );
        if ( !work )
            return;
        
        work->deviceID	= deviceID;
        work->type		= workType;
        
        Push ( work );
    }
    
    
    int AsyncWorker::Push ( int nativeID, unsigned int workType, int portalID, int x, int y )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type		= workType;
            work->deviceID  = nativeID;
            work->arg0      = x;
            work->arg1      = y;
            work->arg2      = portalID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::Push ( int nativeID, unsigned int workType, int portalID, int x, int y, float angle )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type		= workType;
            work->deviceID  = nativeID;
            work->arg0      = x;
            work->arg1      = y;
            work->argFloat  = angle;
            work->arg2      = portalID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::Push ( int nativeID, unsigned int workType, int contextID )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type		= workType;
            work->deviceID  = nativeID;
            work->arg0      = contextID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::Push ( int nativeID, unsigned int workType, int portalID, float angle )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type		= workType;
            work->deviceID  = nativeID;
            work->context   = 0;
            work->argFloat  = angle;
            work->arg2      = portalID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::Push ( int nativeID, unsigned int workType, int contextID, void * contextManaged, int contextAdd1, int contextAdd2 )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type				= workType;
            work->deviceID          = nativeID;
            work->arg0              = contextID;
            work->contextManaged    = contextManaged;
            work->arg1              = contextAdd1;
            work->arg2              = contextAdd2;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
	int AsyncWorker::Push ( int deviceID, unsigned int workType, const char * areaName, const char * appName, int portalID, int x, int y, float angle )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			environs::CopyAppEnvirons ( work->areaName, work->appName, areaName, appName );

			work->type		= workType;
            work->deviceID  = deviceID;
            work->arg0      = x;
            work->arg1      = y;
            work->argFloat  = angle;
            work->arg2      = portalID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::PushPortal ( unsigned int workType, int contextID, void * contextManaged, int contextAdd1, int contextAdd2 )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type				= workType;
            work->arg0              = contextID;
            work->contextManaged    = contextManaged;
            work->arg1              = contextAdd1;
            work->arg2              = contextAdd2;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::PushPortal ( unsigned int workType, int contextID )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type	= workType;
            work->arg0	= contextID;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    int AsyncWorker::PushPortal ( unsigned int workType, int contextID, int contextAdd1, int contextAdd2 )
    {
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( work ) {
			work->type = workType;
            work->arg0 = contextID;
            work->arg1 = contextAdd1;
            work->arg2 = contextAdd2;
            
            Push ( (AsyncWork *) work );
            return 1;
        }
        return 0;
    }
    
    
    void AsyncWorker::PushMediatorMsg ( char * pmsg, int workType )
    {        
        MediatorMsg * msg = (MediatorMsg *) pmsg;

		if ( !msg || msg->size <= 0 || msg->size > 512 )
			return;

		AsyncWork * work = ( AsyncWork * ) calloc ( 1, sizeof ( AsyncWork ) );
		if ( !work )
			return;

		work->type = workType;

		char * ctx = ( char * ) malloc ( msg->size + 1 );
		if ( !ctx ) {
			free ( work );
			return;
		}

		memcpy ( ctx, pmsg, msg->size );

		ctx [ msg->size ] = 0;

		work->context = ctx;
		Push ( work );
    }
    
    
    bool AsyncWorker::PushData ( unsigned int workType, int nativeID, void * data, int dataSize )
    {
        if ( !data || dataSize <= 0 )
            return false;
        
        AsyncWorkDevice * work = (AsyncWorkDevice *) calloc ( 1, sizeof(AsyncWorkDevice) );
        if ( !work )
            return false;

		work->type		= workType;
        work->deviceID	= nativeID;
        
        char * ctx;
        
        if ( workType == ASYNCWORK_TYPE_SEND_UDP ) {
            dataSize += 4;

			ctx = ( char * ) malloc ( dataSize );
            if ( !ctx )
                goto Failed;
            
            memcpy ( ctx + 4, data, dataSize );
            ctx [ 0 ] = 'c';
            ctx [ 1 ] = 'd';
            ctx [ 2 ] = ':';
            ctx [ 3 ] = UDP_MSG_PROTOCOL_VERSION;
        }
        else {
			ctx = ( char * ) malloc ( dataSize );
            if ( !ctx )
                goto Failed;
            
            memcpy ( ctx, data, dataSize );
        }
        
        work->context	= ctx;
        work->arg0		= dataSize;

        Push ( (AsyncWork *) work );
        return true;
        
    Failed:
        free ( work );
        return false;
    }
    

    AsyncWork * AsyncWorker::Pop ()
    {
        CVerbVerb ( "Pop" );
        
        if ( !active )
            return 0;
        
        AsyncWork * notif = 0;
        
        thread.Lock ( "Pop" );
        
        //CLogArg ( "Push: Elements in the queue %i.", size );
        
        // Remove a notif
		notif = ( AsyncWork * ) queue.pop ();
     
        thread.Unlock ( "Pop" );
        
        return notif;
    }
    
    
    void QueryMediatorVersion ( Instance * env )
    {
        CVerb ( "QueryMediatorVersion" );
        
        unsigned int version, revision;
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator )
            return;
        
        if ( mediator->IsRegistered () )
        {
            if ( mediator->GetMediatorServiceVersion ( version, revision ) )
			{
                unsigned int versions [3];
                versions[0] = (version & 0xFFF);
                versions[1] = ((version >> 12) & 0xFFF);
                versions[2] = ((version >> 24) & 0xFF);
                
                CVerbVerbArg("QueryMediatorVersion: Mediator version [%u.%u.%u] and revision [%u]", versions[0], versions[1], versions[2], revision );
                
                if ( versions[0] != BUILD_MAJOR_VERSION || versions[1] != BUILD_MINOR_VERSION || versions[2] != BUILD_RELEASE_COUNTER /*|| revision != BUILD_REVISION*/ )
                {
                    if ( revision > BUILD_REVISION ) {
                        CLogArg ( "QueryMediatorVersion: The Mediator indicates the availability of a newer version of Environs [%u.%u.%u] with revision [%u]", versions[0], versions[1], versions[2], revision );
                    }
                    else {
                        CLogArg ( "QueryMediatorVersion: The Mediator runs an older version of Environs [%u.%u.%u] with revision [%u]", versions[0], versions[1], versions[2], revision );
                    }
                }
                else {
                    CVerbArg ( "QueryMediatorVersion: Mediator version [%u.%u.%u] and revision [%u] are the same as our build.", versions[0], versions[1], versions[2], revision );
                }
            }
            else {
                CWarn ( "QueryMediatorVersion: Failed to query Mediator version" );
            }
        }
    }
    
    
    void AsyncWorker::RegisterStuntSockets ( void * msg )
    {
        CVerb ( "RegisterStuntSockets" );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator )
            return;
        
        MediatorInstance * med = mediator->GetAvailableMediator ();
        if ( !med )
            return;
        
        char * appName	= 0;
        char * areaName	= 0;
        
        STUNTRegReqPacketV8	* req = ( STUNTRegReqPacketV8 * ) msg;
        
        if ( req->sizes [ 0 ] > 1 && req->sizes [ 1 ] > 1 )
        {
            appName = ( char * ) req->sizes + 2;
            areaName = appName + req->sizes [ 0 ];
        }
        
        mediator->RegisterStuntSocket ( true, med, req->deviceID, appName, areaName, 0, req->channelType, req->token, true );
    }
    
    
    void * AsyncWorker::StartQueue ( void * arg )
    {
        AsyncWorker * worker = (AsyncWorker *) arg;
        
        worker->Worker ();

		worker->thread.Lock ( "AsyncWorker.StartQueue" );

        worker->workerAlive.reset ();

        worker->thread.Unlock ( "AsyncWorker.StartQueue" );
        
        return 0;
    }

    
    void * AsyncWorker::Worker ()
    {
        isRunning = true;
        
        CVerb ( "Worker thread started ..." );
        CVerbVerbArg ( "Worker thread id [ %16X ]", GetCurrentThreadId () );
        
        pthread_setname_current_envthread ( "AsyncWorker.Worker" );

#ifndef NDEBUG
        try {
#endif
        while ( active )
        {
            // pop a notification
            AsyncWork * work = Pop ();
            if ( !work ) {
                if ( !thread.LockCond ( "AsyncWorker.Worker" ) )
                    break;
                
                if ( active )
                    thread.WaitLocked ( "AsyncWorker.Worker" );
                
                if ( !thread.UnlockCond ( "AsyncWorker.Worker" ) )
                    break;
                continue;
            }
            
            CVerbVerbArg ( "Worker: [ %16X : %i ]", GetCurrentThreadId (), work -> type );            
            
            // process the work
            if ( work -> type == ASYNCWORK_TYPE_GET_MEDIATOR_VERSION ) {
                QueryMediatorVersion ( env );
            }
            else if ( work -> type == ASYNCWORK_TYPE_REGISTER_STUNT_SOCKETS ) {
                RegisterStuntSockets ( work -> context );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_CONNECT )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
				DeviceController::DeviceDetected ( env->hEnvirons, CALL_NOWAIT, context->deviceID, context->areaName, context->appName, context->arg0, context->arg1, context->argFloat );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_DISCONNECT )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                RemoveDevice ( context->deviceID );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_UPDATE_XYANG )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                UpdateDevice ( context->deviceID, context->arg2, ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG, context->arg0, context->arg1, context->argFloat, true );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_UPDATE_ANG )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                UpdateDevice ( context->deviceID, context->arg2, ASYNCWORK_PARAM_DEVICE_UPDATE_ANG, 0, 0, context->argFloat, true );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_REMOVED )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                UpdateDevice ( context->deviceID, context->arg2, ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG, context->arg0, context->arg1, context->argFloat, false );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_REMOVED_ID )
            {
				AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                UpdateDevice ( context->deviceID, context->arg2, ASYNCWORK_PARAM_DEVICE_UPDATE_STATUS, 0, 0, 0, false );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_INIT_REQUEST )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalRequest ( context->deviceID, context->arg2, context->arg0, context->arg1 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_PROVIDE )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalProvide ( context->deviceID, context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_PROVIDE_REQUEST )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalProvideRequest ( context->deviceID, context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_SEND_INIT )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalSendInit ( context->arg0, context->arg1, context->arg2 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_START )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalStart ( context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_PAUSE )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalPause ( context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_STOP )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalStop ( context->arg0, context->arg1 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_SET_RENDERCALLBACK )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalSetRenderCallback ( context->arg0, context->contextManaged, context->arg1 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_PORTAL_RELEASE_RENDERSURFACE )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                PortalReleaseRenderSurface ( context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_STUN )
            {                
                StunRequest::HandleIncomingRequest ( env, work->context );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_FLAGS_UPDATE )
            {
				UpdateDeviceFlags ( (char *) work->context );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC )
            {
				SyncDeviceFlags ( work->deviceID );
            }
            else if ( work -> type == ASYNCWORK_TYPE_SENSOR_DATA )
            {
                AsyncWorkDevice * sensor = (AsyncWorkDevice *) work;
                
                DeviceBase::SendTcpBuffer ( sensor->deviceID, true, MSG_TYPE_SENSOR, 0, sensor->context, sensor->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_SEND_UDP )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                SendDataUdp ( context->deviceID, context->context, context->arg0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC_ASYNC )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                SyncDeviceFlagsAsync ( context->deviceID, context->arg2, context->arg0 != 0 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_SAVE_MEDIATOR_TOKENS )
            {
#ifdef DISPLAYDEVICE
                environs::SaveConfig ();
#else
                if ( *env->DefaultMediatorToken ) {
                    env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT, env->DefaultMediatorToken );
                    env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT_N, env->DefaultMediatorUserName );
                }
                
                if ( *env->CustomMediatorToken ) {
                    env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM, env->CustomMediatorToken );
                    env->opt ( APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM_N, env->CustomMediatorUserName );
                }
#endif
            }
            
#ifdef DISPLAYDEVICE
            else if ( work -> type == ASYNCWORK_TYPE_TRACKER_USAGE )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
				TrackerUsage ( context->areaName, context->arg2 );
            }
            else if ( work -> type == ASYNCWORK_TYPE_TRACKER_COMMAND )
            {
                AsyncWorkDevice * context = (AsyncWorkDevice *) work;
                
                TrackerCommand ( context->arg2, context->arg0 );
            }
#endif            
            // Dispose work item
			DisposeAsyncWork ( work );
        }

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "AsyncWorker: Exception !!!\n" );
            _EnvDebugBreak ( "AsyncWorker" );
        }
#endif
        //	Finish:
        //pthread_reset ( workerThreadID ); // Will be done by the "closer"
        CLog ( "Worker: bye bye..." );
        
        isRunning = false;
        return 0;
    }


	/**
	* Update flags from mediator daemon to a particular device.
	*
	* @param	msgDec	privdes flags to set or clear. (of type DeviceFlagsInternal::Observer*)
	*					We change them to CPTypes of DeviceFlagsInternal
	*/
	void AsyncWorker::UpdateDeviceFlags ( char * msgDec )
    {
		CVerbVerb ( "UpdateDeviceFlags" );

		MediatorStatusMsg * msg = ( MediatorStatusMsg * ) msgDec;

		char * appName;
		char * areaName;
        
        if ( msg->sizes [0] > 1 && msg->sizes [1] > 1 && msg->sizes [0] < MAX_NAMEPROPERTY && msg->sizes [1] < MAX_NAMEPROPERTY ) {
            appName   = (( MediatorStatusMsgExt * ) msg)->appArea;
            areaName  = appName + msg->sizes [0];
            
            if ( !*appName || !*areaName ) {
                appName  = env->appName;
                areaName = env->areaName;
            }
            else if ( env->mediatorFilterLevel >= MEDIATOR_FILTER_AREA_AND_APP )
            {
                if ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) || strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) ) {
                    CVerbVerbArg ( "UpdateDeviceFlags: Device filter level restricted [ 0x%X : %s / %s ]", msg->deviceID, appName, areaName  );
                    return;
                }
            }
		}
        else {
            appName  = env->appName;
            areaName = env->areaName;
        }
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		if ( !mediator ) {
			CVerb ( "UpdateDeviceFlags: Mediator layer is missing." );
			return;
		}        
        
        CVerbArg ( "UpdateDeviceFlags: [ 0x%X ]", msg->status1 );
		
		sp ( DeviceInstanceNode ) deviceSP = mediator->GetDeviceSP ( msg->deviceID, areaName, appName );
        if ( !deviceSP ) {
            CVerbArg ( "UpdateDeviceFlags: Device not found [ 0x%X : %s / %s ]", msg->deviceID, appName, areaName  );
			return;
        }
        
		DeviceInstanceNode * device = deviceSP.get ();
        
        int cpflags = ( ( msg->status1 << 8 ) & DeviceFlagsInternal::CPNotifyMask );
        int flags   = ( ( msg->status1 >> 8 ) & DeviceFlagsInternal::NotifyMask );
        
        bool doBackPropagate = false;
        bool doNotify        = false;
        
        unsigned short deviceFlags = device->info.flags;
        
        CVerbsArg ( 2, "UpdateDeviceFlags: CPFlags [ 0x%X ] Flags [ 0x%X ] DeviceFlags [ 0x%X ]", cpflags, flags, deviceFlags );
        
        if ( ( deviceFlags & DeviceFlagsInternal::CPNotifyMask ) != cpflags )
        {
            doNotify = true;
            device->info.flags |= cpflags;
        }
        
        if ( ( deviceFlags & DeviceFlagsInternal::CPPlatformReady ) != ( cpflags & DeviceFlagsInternal::CPPlatformReady ) )
            doBackPropagate = true;
        else if ( flags && ( deviceFlags & DeviceFlagsInternal::NotifyMask ) != flags )
        {
            doBackPropagate = true;
        }
        
        if ( doNotify ) {
            CVerbsArg ( 2, "UpdateDeviceFlags: Flags [ 0x%X ]", cpflags );
            
            if ( deviceFlags & DeviceFlagsInternal::PlatformReady )
                API::onEnvironsNotifierContext1 ( env, device->info.objID, Notify::Environs::DeviceFlagsUpdate, cpflags, 0, msg->status2 );
#ifndef NDEBUG
            else {
                CVerb ( "UpdateDeviceFlags: PLATFORM NOT READY." );
            }
#endif
        }

        if ( doBackPropagate ) {
            CVerbsArg ( 2, "UpdateDeviceFlags: Backpropagate flags [ 0x%X ]", deviceFlags );

			// Push this to the send thread pool?
            mediator->SetDeviceFlags ( device, true );
            
            CVerb ( "UpdateDeviceFlags: Do Backpropagate due to PLATFORM BECAME READY." );
		}
    }
    
    
    /**
     * Sync flags from a particular device to the Mediator server.
     *
     * @param	context
     */
    void AsyncWorker::SyncDeviceFlagsAsync ( int objID, int flags, bool set )
    {
        CVerb ( "SyncDeviceFlags" );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator )
            return;
        
        sp ( DeviceInstanceNode ) deviceSP = mediator->GetDeviceSP ( objID );
        if ( !deviceSP )
            return;
        
        bool doBackPropagate = false;
        
        DeviceInstanceNode * device = deviceSP.get ();
        
        
        flags &= DeviceFlagsInternal::NotifyMask;
        
        int f = ( int ) ( deviceSP->info.flags & ( unsigned short ) DeviceFlagsInternal::NotifyMask );
        
        if ( set ) {
            if ( ( f & flags ) == flags )
                return;
            
            if ( ( device->info.flags & DeviceFlagsInternal::PlatformReady ) != ( flags & DeviceFlagsInternal::PlatformReady ) )
                doBackPropagate = true;
            
            device->info.flags |= flags;
        }
        else {
            if ( ( f & flags ) != flags )
                return;
            
            device->info.flags &= ~flags;
        }
        
        if ( doBackPropagate ) {
            CVerb ( "SyncDeviceFlagsAsync: Do backnotify due to PLATFORM BECAME READY." );
            
            API::onEnvironsNotifierContext1 ( env, device->info.objID, Notify::Environs::DeviceFlagsUpdate, device->info.flags, 0, 1 );
        }
        
        mediator->SetDeviceFlags ( device, set );
    }
    

	/**
	* Sync flags from a particular device to the Mediator server.
	*
	* @param	objID	The objID that identifies the target device.
	*/
	void AsyncWorker::SyncDeviceFlags ( int objID )
    {
        CVerb ( "SyncDeviceFlags" );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator ) {
            CVerb ( "SyncDeviceFlags: Mediator layer is missing." );
			return;
        }

		sp ( DeviceInstanceNode ) device = mediator->GetDeviceSP ( objID );
		if ( !device ) {
			CVerbArg ( "SyncDeviceFlags: Device objID [ %i ] not found.", objID );
			return;
		}

		mediator->SetDeviceFlags ( device.get (), true );
	}
    
    
    bool AsyncWorker::SendDataUdp ( int nativeID, void * data, int dataSize )
    {
        bool success = false;
        
        DeviceBase * device = GetDevice ( nativeID );
        if ( device )
        {
            success = device->SendDataPacket ( (const char *) data, (unsigned int) dataSize );
            
            UnlockDevice ( device );
        }
        return success;
    }
    
    
    bool AsyncWorker::SendDataUdpPrefix ( int nativeID, void * data, int dataSize )
    {
		if ( dataSize <= 0 )
			return true;

        bool success = false;
        
        DeviceBase * device = GetDevice ( nativeID );
        if ( device )
        {
			char * tmp = ( char * ) malloc ( dataSize + 4 );
            if ( tmp ) {
				tmp [ 0 ] = 'c';
				tmp [ 1 ] = 'd';
				tmp [ 2 ] = ':';
				tmp [ 3 ] = UDP_MSG_PROTOCOL_VERSION;

                memcpy ( tmp + 4, data, dataSize );

				success = device->SendDataPacket ( ( const char * ) tmp, ( unsigned int ) dataSize + 4 );

				free ( tmp );
            }
                        
            UnlockDevice ( device );
        }
        return success;
    }
    
    
    int AsyncWorker::TrackerUsage ( const char * module, int command )
    {
        CVerb ( "TrackerUsage" );
        
        int success = 0;
        
#ifdef DISPLAYDEVICE
        if ( module ) {
            switch ( command ) {
                case 1:
                    success = env->kernel->SetUseTracker ( module );
                    break;
                case 2:
                    success = env->kernel->GetUseTracker ( module );
                    break;
                default:
                    success = env->kernel->DisposeTracker ( module );
                    break;
            }
        }
#endif
        return success;
    }
    
    
    EBOOL AsyncWorker::TrackerCommand ( int module, int command )
    {
        CVerb ( "TrackerCommand" );
        
#ifdef DISPLAYDEVICE
        if ( env->kernel ) {
            ITracker * t = env->kernel->GetTracker ( module );
            if ( t )
                return t->Execute ( command );
        }
#endif
        return false;
    }
    
    
    EBOOL AsyncWorker::UpdateDevice ( int nativeID, int portalID, int updTypes, int x, int y, float angle, bool contactStatus )
    {
        EBOOL           success = false;
        DeviceBase *    device  = 0;
        
        if ( portalID )
			device = GetDeviceIncLock ( portalID );
        
		if ( !device )
			device = GetDevice ( nativeID );

        if ( device ) {
            if ( updTypes == ASYNCWORK_PARAM_DEVICE_UPDATE_ANG )
                device->UpdateAngle ( portalID, angle );
            else
                if ( updTypes == ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG )
                    device->UpdatePosition ( portalID, x, y, angle );
            
            device->SetDirectContactStatus ( contactStatus );
            UnlockDevice ( device );
            
            success = true;
        }
        return success;
    }
    
    
    
	EBOOL AsyncWorker::PortalRequest ( int nativeID, int portalDetails, int width, int height )
    {
        EBOOL success = false;

		DeviceController * device = ( DeviceController * ) GetDevice ( nativeID );
        
        if ( !device ) {
            CWarnIDN ( "PortalRequest: Device is not available anymore." );
            return false;
        }
        
        char buffer [24];
        
        int * pUI = reinterpret_cast<int *>(buffer);
        
        *pUI = width; pUI++;
        *pUI = height;
        
		/*if ( device->SendBuffer ( false, MSG_TYPE_PORTAL, portalDetails | PORTAL_DIR_INCOMING, 0, MSG_PORTAL_REQUEST, buffer, sizeof ( int ) * 2 ) < 0 ) {
            CErrIDN ( "PortalRequest: Failed to send portal message" );
         }*/
        
        if ( device->BringUpInteractThread () )
        {
            if ( device->SendBuffer ( true, MSG_TYPE_PORTAL, portalDetails | PORTAL_DIR_INCOMING, 0, MSG_PORTAL_REQUEST, buffer, sizeof ( int ) * 2 ) < 0 ) {
                CErrIDN ( "PortalRequest: Failed to send portal message" );
            }
            else success = true;
        }
        
        UnlockDevice ( device );
        return success;
    }
    
    
    EBOOL AsyncWorker::PortalProvide ( int nativeID, int portalID )
    {
        EBOOL success = false;

		DeviceController * device = ( DeviceController * ) GetDevice ( nativeID );
        if ( !device ) {
            CWarnIDN ( "PortalProvide: Device is not available anymore." );
            return false;
        }
        
        success = device->ProvidePortal ( portalID );
        
        UnlockDevice ( device );
        return success;
    }
    
    
    EBOOL AsyncWorker::PortalProvideRequest ( int nativeID, int portalID )
    {
        EBOOL success = false;

		DeviceController * device = ( DeviceController * ) GetDevice ( nativeID );
        if ( !device ) {
            CWarnIDN ( "PortalProvideRequest: Device is not available anymore." );
            return false;
        }
        
        success = device->SendPortalMessage ( MSG_PORTAL_ASK_FOR_REQUEST, portalID );
        
        UnlockDevice ( device );
        return success;
    }
    
    
	EBOOL AsyncWorker::PortalSetRenderCallback ( int portalID, void * callback, int callbackType )
    {
        PortalDevice * portal = GetLockedPortalDevice ( portalID );
        if ( !portal ) {
            CErr ( "PortalSetRenderCallback: No portal resource found." );
            return false;
        }

		if ( portal->receiver ) {
			portal->receiver->SetRenderCallback ( (ptRenderCallback)callback, callbackType );
		}
		else {
			CErrArg ( "PortalSetRenderCallback: PortalID [0x%X] is not a receiver!", portalID );
		}
        
        ReleasePortalDevice ( portal );
        return true;
    }


	EBOOL AsyncWorker::PortalReleaseRenderSurface ( int portalID )
	{
		CVerb ( "PortalReleaseRenderSurface" );
        
        PortalDevice * portal = GetLockedPortalDevice ( portalID );
        if ( !portal ) {
            CWarn ( "PortalReleaseRenderSurface: No portal resource found." );
            return false;
		}

		if ( portal->receiver ) {
			portal->receiver->ReleaseRenderCallback ();
			portal->receiver->ReleaseRenderSurface ( true );
		}
        
        ReleasePortalDevice ( portal );
		return true;
	}
    
    
    EBOOL AsyncWorker::PortalStart ( int portalID )
    {
        EBOOL success = false;
        
        PortalDevice * portal = GetLockedPortalDevice ( portalID );
        if ( !portal ) {
            CErr ( "PortalStart: No portal resource found." );
            return false;
        }
        
        if ( IsPortalReceiver ( ) ) {
            if ( portal->stream )
                portal->stream->Start ();
        }
        
        DeviceBase * device = portal->device;
        
        IncLockDevice ( device );
        
        ReleasePortalDevice ( portal );
        
        success = device->SendPortalMessage ( MSG_PORTAL_START, portalID );
        
        UnlockDevice ( device );
        return success;
    }
    
    
    EBOOL AsyncWorker::PortalPause ( int portalID )
    {
        EBOOL success = false;
        
        DeviceBase * device = GetDeviceIncLock ( portalID );
        if ( device )
        {
            success = device->SendPortalMessage ( MSG_PORTAL_PAUSE, portalID );
            
            UnlockDevice ( device );
        }
        return success;
    }
    
    
    EBOOL AsyncWorker::PortalStop ( int nativeID, int portalID )
    {
        EBOOL success = false;
        
        DeviceBase * device = GetDeviceIncLock ( portalID );
        if ( !device ) {
            device = GetDevice ( nativeID );
        }
        
        if ( device ) {
            success = device->SendPortalMessage ( MSG_PORTAL_STOP, portalID );
            
            device->DisposePortal ( portalID );
            
            UnlockDevice ( device );
        }
        return success;
    }
    
    
    EBOOL AsyncWorker::PortalSendInit ( int portalID, int width, int height )
    {
        CVerb ( "PortalSendInit" );
        
        PortalDevice * portal = GetLockedPortalDevice ( portalID );
        if ( !portal ) {
            CErr ( "PortalSendInit: No portal resource found." );
            return false;
        }
        
        EBOOL success = false;
        
        if ( IsPortalGenerator ( ) ) {
            if ( portal->generator )
                success = portal->generator->SendStreamInit ( width, height );
        }
        
        ReleasePortalDevice ( portal );
        
        return success;
    }
    
    
#undef	CLASS_NAME
#define	CLASS_NAME 	"Async.Worker.Send. . . ."
    
	typedef struct StartSendContext
	{
		int			  workerID;
		AsyncWorker * worker;
	}
	StartSendContext;


	/**
	* Start a particular send thread from the thread pool. A  call to this method must be locked/secured by the caller (on sendThread[0]).
	*
	* @param	i		The sender thread within the threadpool identified by an index.
	*/
    void AsyncWorker::StartSend ( int i )
    {
        CVerbArg ( "StartSend: [ %i ]", i  );
        
		if ( i < 0 || i >= ASYNCWORK_SEND_POOL_SIZE )
			return;

		StartSendContext * ctx = ( StartSendContext * ) malloc ( sizeof( StartSendContext ) );
		if ( !ctx )
			return;

		ctx->workerID		= i;
		ctx->worker			= this;

		if ( i == 0 ) {
			sendEnabled     = true;
			deviceStatus    = DeviceStatus::ConnectInProgress;
		}

		if ( !workerSendAlives [ i ] )
		{
			workerSendAlives [ i ] = env->myself;
			if ( workerSendAlives [ i ] )
			{
				if ( !sendThreads [ i ].Run ( pthread_make_routine ( &StartSendQueue ), ctx, "AsyncWorker::StartSend" ) )
				{
					workerSendAlives [ i ].reset ();

					if ( i == 0 )
						sendEnabled = false;
				}
				else
					return;
			}
		}

		free ( ctx );
    }
    
    
    void DisposeSendPack ( AsyncWorkSend * work )
    {
        CVerbVerb ( "DisposeSendPack" );
        
        if ( work->descriptor )
            free ( work->descriptor );
        if ( work->filePath )
            free ( work->filePath );
        free ( work );
    }


	void AsyncWorker::SignalStopSend ()
	{
		sendEnabled     = false;
		deviceStatus    = DeviceStatus::Deleteable;
        
        sendThreadsSync.Notify ( "AsyncWorker.SignalStopSend", true );
	}
    
    
    void AsyncWorker::StopSend ()
    {
		SignalStopSend ();
        
        for ( int i = 0; i < ASYNCWORK_SEND_POOL_SIZE; i++ )
        {
            sendThreads [ i ].Join ( "AsyncWorker send thread" );
        }
        
        CVerbArg ( "StopSend: Elements in the send queue [ %i ]. Disposing them now.", queue.size_ );
        
        sendThreadsSync.Lock ( "AsyncWorker.StopSend" );

		while ( queueSend.size_ > 0 ) {
			AsyncWorkSend * cur = ( AsyncWorkSend * ) queueSend.pop ();

			if ( cur )
				DisposeSendPack ( cur );
		}

        sendThreadsSync.Unlock ( "AsyncWorker.StopSend" );
    }
    
    
    void AsyncWorker::PushSend ( AsyncWorkSend * work )
    {
        if ( !work )
            return;
        
        CVerbVerb ( "PushSend" );
        
        bool unlock = false;
        
        do
        {
			if ( !active || queueSend.size_ > WORKER_SEND_MAX_SIZE || !sendEnabled ) {
                CVerb ( "PushSend: AsyncWorker inactive or queue full!" );
                break;
            }
            
            if ( !sendThreadsSync.Lock ( "AsyncWorker.PushSend" ) )
                break;
            unlock = true;
            
            CVerbVerbArg ( "PushSend: Elements in the queue [%i].", queueSend.size_ );
            
            // Append the notif
			if ( queueSend.push ( work ) )
				work = 0;
			else break;

			CVerbsArg ( 5, "PushSend: Elements in the queue new [ %i ].", queueSend.size_ );
            if ( sendEnabled )
            {
                if ( !isSendRunning ) {
                    sendThreadsSync.Unlock ( "AsyncWorker.PushSend" );
                    unlock = false;
                    
                    StartSend ( 0 );
                }
                else {
					if ( queueSend.size_ > 1 && sendThreadsRunning < ( ASYNCWORK_SEND_POOL_SIZE - 1 ) )
                    {
                        for ( int i = 1; i < ASYNCWORK_SEND_POOL_SIZE; i++ )
                        {
                            if ( !sendThreads [ i ].isRunning () )
                            {
                                StartSend ( i );
                                
                                if ( sendThreads [ i ].isRunning () )
                                    break;
                            }
                        }
                    }
                }
            }
            
            sendThreadsSync.Notify ( "AsyncWorker.PushSend", !unlock);
            
        }
        while ( false );
        
        if ( unlock )
            sendThreadsSync.Unlock ( "AsyncWorker.PushSend" );
        
		if ( work ) {
#ifndef NDEBUG
			CErrArg ( "PushSend: Failed. Queue size [ %i ]", queueSend.size_ );
#endif
			DisposeSendPack ( work );
		}
    }
    
    
    int AsyncWorker::PushSend ( int deviceID, unsigned int workType, const char * areaName, const char * appName,
                               const char * msg, int length )
    {
        CVerbVerbID ( "PushSend" );

		if ( !msg || length <= 0 || !sendEnabled || !active || queueSend.size_ > WORKER_SEND_MAX_SIZE ) {
            CVerb ( "PushSend: Invalid message or send disabled!" );
            return 0;
        }

		char * tmp = ( char * ) calloc ( 1, length + 2 );
        if ( !tmp )
            return 0;
        
        memcpy ( tmp, msg, length );
        
        AsyncWorkSend * work = (AsyncWorkSend *) calloc ( 1, sizeof(AsyncWorkSend) );
        if ( !work ) {
            free ( tmp );
            return 0;
        }
        work->descriptor = tmp;
        
        environs::CopyAppEnvirons ( work->areaName, work->appName, areaName, appName );
        
        work->type		= workType;
        work->deviceID	= deviceID;
        work->bufferSize = length;
        
        PushSend ( work );
        
        return 1;
    }
    
    
    int AsyncWorker::PushSend ( int nativeID, int fileID, const char * fileDescriptor, const char * filePath )
    {
        CVerbVerbIDN ( "PushSend" );

		if ( !filePath || !sendEnabled || !active || queueSend.size_ > WORKER_SEND_MAX_SIZE )
            return 0;
        
		char * tmp = 0;

        size_t len = strlen ( filePath );
		if ( len > 0 )
		{
			tmp = ( char * ) malloc ( len + 1 );
			if ( !tmp )
				return 0;

			memcpy ( tmp, filePath, len );

			tmp [ len ]		= 0;
		}

		AsyncWorkSend * work = ( AsyncWorkSend * ) calloc ( 1, sizeof ( AsyncWorkSend ) );
        if ( !work ) {
			if ( tmp )
				free ( tmp );
            return 0;
        }
        work->filePath	 = tmp;
        work->isNativeID = true;
        work->fileID	 = fileID;
        work->type		 = ASYNCWORK_TYPE_SEND_FILE;
        work->deviceID	 = nativeID;
        
        tmp = 0;
        
        if ( fileDescriptor ) 
		{
            len = strlen ( fileDescriptor );
			if ( len > 0 ) 
			{
				tmp = ( char * ) malloc ( len + 1 );
				if ( tmp ) {
					memcpy ( tmp, fileDescriptor, len );

					tmp [ len ]		= 0;

					work->descriptor = tmp;
				}
			}
        }
        
        
        PushSend ( work );
        
        return 1;
    }
    
    
    int AsyncWorker::PushSend ( int nativeID, int fileID, const char * fileDescriptor, const char * buffer, int bufferSize )
    {
        CVerbVerbIDN ( "PushSend" );

		if ( !buffer || bufferSize <= 0 || !sendEnabled || !active || queueSend.size_ > WORKER_SEND_MAX_SIZE )
            return 0;

		char * tmp = ( char * ) malloc ( bufferSize + 1 );
        if ( !tmp )
            return 0;
        
        memcpy ( tmp, buffer, bufferSize );

		tmp [ bufferSize ] = 0;

		AsyncWorkSend * work = ( AsyncWorkSend * ) calloc ( 1, sizeof ( AsyncWorkSend ) );
        if ( !work ) {
            free ( tmp );
            return 0;
        }
        work->filePath	 = tmp;
        work->isNativeID = true;
        
        work->fileID	 = fileID;
        work->type		 = ASYNCWORK_TYPE_SEND_BUFFER;
        work->deviceID	 = nativeID;
        work->bufferSize = bufferSize;
        
        tmp = 0;
        
        if ( fileDescriptor ) 
		{
            size_t len = strlen ( fileDescriptor );
			if ( len > 0 ) 
			{
				tmp = ( char * ) malloc ( len + 1 );
				if ( tmp ) {
					memcpy ( tmp, fileDescriptor, len );

					tmp [ len ] = 0;

					work->descriptor = tmp;
				}
			}
        }
        
        PushSend ( work );
        
        return 1;
    }
    
    
    void AsyncWorker::DisposeSends ( DeviceInfo * device )
    {
        CDisend ( "DisposeSends" );

		if ( !device || !active || queueSend.size_ <= 0 )
            return;

        if ( !sendThreadsSync.Lock ( "AsyncWorker.DisposeSends" ) )
            return;
        
        CDisendArg ( "DisposeSends: Elements in the queue [%i].", queueSend.size_ );
        
        int nativeID = device->nativeID;

		AsyncWorkSend   ** packets = ( AsyncWorkSend   ** ) queueSend.items;
		
		int end, end1 = -1;

		if ( queueSend.end > queueSend.next )
			end = ( int ) queueSend.end;
		else {
			end = ( int ) queueSend.capacity;
			end1 = ( int ) queueSend.end;
		}

		for ( int i = ( int ) queueSend.next; i < end; i++ )
		{
			AsyncWorkSend   * packet = packets [ i ];
			if ( !packet )
				continue;

			if ( packet->isNativeID )
			{
				if ( packet->deviceID == nativeID ) {
					packet->disposed = true;
					CDisendArg ( "DisposeSends: Found nativeID [%i].", nativeID );
				}
			}
			else {
				if ( device->deviceID == packet->deviceID )
				{
					if ( !*packet->appName || !*packet->areaName || ( !strncmp ( device->areaName, packet->areaName, MAX_NAMEPROPERTY - 1 )
						&& !strncmp ( device->appName, packet->appName, MAX_NAMEPROPERTY - 1 ) ) )
					{
						packet->disposed = true;
						CDisendArg ( "DisposeSends: Found deviceID [%i].", packet->deviceID );
					}
				}
			}
		}

		if ( end1 >= 0 ) 
		{
			for ( int i = 0; i < end1; i++ )
			{
				AsyncWorkSend   * packet = packets [ i ];
				if ( !packet )
					continue;

				if ( packet->isNativeID )
				{
					if ( packet->deviceID == nativeID ) {
						packet->disposed = true;
						CDisendArg ( "DisposeSends: Found nativeID [%i].", nativeID );
					}
				}
				else {
					if ( device->deviceID == packet->deviceID )
					{
						if ( !*packet->appName || !*packet->areaName || ( !strncmp ( device->areaName, packet->areaName, MAX_NAMEPROPERTY - 1 )
							&& !strncmp ( device->appName, packet->appName, MAX_NAMEPROPERTY - 1 ) ) )
						{
							packet->disposed = true;
							CDisendArg ( "DisposeSends: Found deviceID [%i].", packet->deviceID );
						}
					}
				}
			}
		}
        
        sendThreadsSync.Unlock ( "AsyncWorker.DisposeSends" );
    }
    
    
    AsyncWorkSend * AsyncWorker::PopSend ()
    {
        CVerbVerb ( "PopSend" );
        
        if ( !active || !sendEnabled )
            return 0;
        
        AsyncWorkSend * notif = 0;
        
        if ( !sendThreadsSync.Lock ( "AsyncWorker.PopSend" ) )
            return 0;
        
        //CLogArg ( "Push: Elements in the queue %i.", size );
        
        // Remove a notif
		notif	  = ( AsyncWorkSend * ) queueSend.pop ();
        
        sendThreadsSync.Unlock ( "AsyncWorker.PopSend" );
        
        return notif;
    }
    
    
    void * AsyncWorker::StartSendQueue ( void * arg )
    {
		StartSendContext * ctx = ( StartSendContext * ) arg;
		if ( !ctx )
			return 0;

        AsyncWorker * worker = (AsyncWorker *) ctx->worker;
		int i = ctx->workerID;

		free ( arg );

		if ( i < 0 || i >= ASYNCWORK_SEND_POOL_SIZE )
			return 0;

		worker->sendThreadsRunning++;

		if ( i == 0 )
			worker->isSendRunning   = true;

		worker->WorkerSend ( i );

		worker->sendThreadsSync.Lock ( "AsyncWorker.StartSendQueue" );

		worker->workerSendAlives [ i ].reset ();

		if ( i == 0 )
			worker->isSendRunning   = false;

		worker->sendThreadsSync.Unlock ( "AsyncWorker.StartSendQueue" );

		worker->sendThreadsRunning--;

        return 0;
    }
    
    
    void * AsyncWorker::WorkerSend ( int i )
    {
        deviceStatus    = DeviceStatus::Connected;
        
        CVerbArg ( "WorkerSend thread [ %i ] started ...", i );
        
        CVerbVerbArg ( "WorkerSend [ %i ] thread id [ %16X ]", i, GetCurrentThreadId () );
        
        pthread_setname_current_envthread ( "AsyncWorker.WorkerSend" );
        
        while ( active && sendEnabled )
        {
            // pop a notification
            AsyncWorkSend * work = PopSend ();
            if ( !work ) {
                if ( i != 0 )
                    break;
                
                if ( !sendThreadsSync.LockCond ( "AsyncWorker.WorkerSend" ) )
                    break;
                
                if ( active && sendEnabled )
                    sendThreadsSync.WaitLocked ( "AsyncWorker.WorkerSend" );
                
                if ( !sendThreadsSync.UnlockCond ( "AsyncWorker.WorkerSend" ) )
                    break;
                continue;
            }
            
			if ( !work->disposed ) {
				CVerbArg ( "WorkerSend: [ %i ] starting send ...", i );

				if ( work -> type == ASYNCWORK_TYPE_SEND_FILE ) {
					DeviceBase::SendFile ( work->deviceID, work->fileID, work->descriptor, ( const void* ) work->filePath );
				}
				else if ( work -> type == ASYNCWORK_TYPE_SEND_BUFFER ) {
					DeviceBase::SendBuffer ( work->deviceID, work->fileID, work->descriptor, work->filePath, work->bufferSize );
				}
				else if ( work -> type == ASYNCWORK_TYPE_SEND_MESSAGE ) {
					CVerbsArg ( 4, "WorkerSend: sending message [ %s ]", work->descriptor );
					DeviceBase::SendMessage ( env->hEnvirons, &deviceStatus, work->deviceID, work->areaName, work->appName, work->descriptor, work->bufferSize );

					CVerbsArg ( 5, "PushSend: Elements in the queue new [ %i ].", queueSend.size_ );
				}
			}
            
            CVerbArg ( "WorkerSend: [ %i ] send done", i );
            
			DisposeSendPack ( work );
        }
        
        sendThreads [ i ].Detach ( "AsyncWorker.WorkerSend" );
        
        CVerbArg ( "WorkerSend: [ %i ] done", i );
        
        return 0;
    }
    
    

}
