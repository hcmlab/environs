/**
 * Device Instance Object
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
//#	define DEBUGVERBList
#endif

#ifndef CLI_CPP
#include "Environs.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Sensors.h"
#include "Core/Array.List.h"
#include "Environs.Observer.h"
#include "Device.Instance.h"
#include "File.Instance.h"
#include "Message.Instance.h"
#include "Portal.Instance.h"
#else
#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Environs.h"
#include "Device/Device.Instance.Cli.h"
#include "Core/File.Instance.Cli.h"
#include "Core/Message.Instance.Cli.h"
#include "Portal/Portal.Instance.Cli.h"
#endif

#include "Environs.Native.h"
#include "Environs.Utils.h"
#include "Environs.Sensors.h"
#include "Tracer.h"

#include <stdio.h>
#include <direntw.h>
#include <unistdw.h>

#ifndef CLI_CPP
#include <queue>

using namespace std;
#endif

using namespace environs;

#define CLASS_NAME	"Device.Instance. . . . ."

#define DEVICE_INSTANCT_MAX_CACHE_SIZE		128

#define USE_WP_MYSELF


namespace environs
{
	namespace lib
    {
        
#ifndef CLI_CPP
		bool DeviceInstance::globalsInit         = false;

		environs::DeviceInfo DeviceInstance::emptyInfo;
#endif
        
        bool DeviceInstance::GlobalsInit ()
        {
            CVerb ( "GlobalsInit" );
            
            if ( globalsInit )
                return true;
            
            globalsInit = true;

#ifdef CLI_CPP
			notifyPropertyChangedDefault		= true;

			emptyInfo.areaName                  = "Environs";
			emptyInfo.appName                   = "DefaultApp";
			emptyInfo.deviceName                = "DefaultDeviceName";
#else
			Zero ( emptyInfo );
#endif
            return true;
        }
        
        
        void DeviceInstance::GlobalsDispose ()
        {
            CVerb ( "GlobalsDispose" );
            
            if ( !globalsInit )
                return;

            globalsInit = false;
        }
        

		UdpDataPack::UdpDataPack ( Addr_ptr ptr, int sizeInBytes )
		{
            size = sizeInBytes;
            
            if ( sizeInBytes <= 0 )
                data = nill;
            
            data = new__UCharArray ( sizeInBytes );
            
            if ( data != nill )
            {
#ifdef CLI_CPP
                System::Runtime::InteropServices::Marshal::Copy ( IntPtr ( ptr ), data, 0, sizeInBytes );
#else
                memcpy ( data, ptr, sizeInBytes );
#endif
            }
		}


		UdpDataPack::~UdpDataPack ()
		{
            if ( data != nill ) {
                delete__obj ( data );
            }
		}


#undef CLASS_NAME
#define CLASS_NAME	"Device.Instance. . . . ."

        ENVIRONS_OUTPUT_WP_ALLOC ( DeviceInstance );

        
        DeviceInstance::DeviceInstance ()
        {
            CVerbVerb ( "Construct" );

            TraceDeviceInstanceAdd ( this );
            
            hEnvirons           = 1;
            
            disposed_           = 0;
			isSameAppArea       = false;
            changeEventPending  = false;
            udpDataDisposeFront = false;
            platformRef         = nill;
            storageLoaded       = 0;
            envObj				= nill;

			info_				= Addr_of ( emptyInfo );
            objIDPrevious       = -1;
            messagesEnqueue     = true;
            filesLast           = -1;

#ifndef CLI_CPP
			env                 = nill;
            
            Zero ( display );
#else
			notifyPropertyChanged = notifyPropertyChangedDefault;
#endif

#if ( defined(ENVIRONS_OSX) || defined(ENVIRONS_IOS) )
            platformKeep        = nill;
#endif
            
            enableSensorSender  = 0;
            
            async               = Call::NoWait;
            connectProgress     = 0;
            directStatus_       = 0;

			files               = sp_make ( NLayerMapTypeObj ( int, EPSPACE FileInstance ) );

            TraceDeviceInstanceReset ( this );

#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
			atLists				= 0;
#endif
			ENVIRONS_OUTPUT_ALLOC_INIT_WSP ();
        }

        
        DeviceInstance::~DeviceInstance ()
        {
            CListLogArg1 ( "Destruct", "objID", "i", objID_ );

            TraceDeviceInstanceRemove ( this );

            DisposeUdpPacks ();
            
            CondDisposeA ( udpDataEvent );
            CondDisposeA ( filesEvent );
            CondDisposeA ( messagesEvent );
            
            CondDisposeA ( changeEvent );
            
            LockDisposeA ( changeEventLock );
            
            LockDisposeA ( devicePortalsLock );
            
#ifndef CLI_CPP
            // This should not be necessary at all because we cannot be deleted if an MessageInstance or FileInstance has an SP that references us
            // but just to make sure that no weired (experimental) code paths happen to do something else ...
            if ( !messages.empty () ) {
#ifndef NDEBUG
                CWarn ( "Destruct: There are still undisposed messages!!!" );
#endif
                LockAcquireVA ( storageLock, "Destruct" );

                DisposeMessagesQ ();

                LockReleaseVA ( storageLock, "Destruct" );
            }
            
            if ( files != nill && files->size () > 0 ) {
#ifndef NDEBUG
                CWarn ( "Destruct: There are still undisposed messages!!!" );
#endif
                LockAcquireVA ( storageLock, "Destruct" );

                // Remove file instances
                DisposeFiles ();

                LockReleaseVA ( storageLock, "Destruct" );
            }
#endif
            LockDisposeA ( storageLock );

#ifndef CLI_CPP
            if ( info_ != Addr_of ( emptyInfo ) ) {
                environs::DeviceInfo * tmp = info_;

                info_ = Addr_of ( emptyInfo );

                if ( tmp )
                    delete tmp;
            }
#endif
            CListLogArg1 ( "Destruct: done ", "objID", "i", objID_ );
        }
        
#define DEBUG_SP_CONSISTENCY
        
        /**
         * Release ownership on this interface and mark it disposable.
         * Release must be called once for each Interface that the Environs framework returns to client code.
         * Environs will dispose the underlying object if no more ownership is hold by anyone.
         *
         */
        void DeviceInstance::Release ()
        {
#ifndef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
			ENVIRONS_OUTPUT_RELEASE_SP ( DeviceInstance );
#else
			LONGSYNC localRefCount = __sync_sub_and_fetch ( &refCountSP, 1 ); 
    
			if ( localRefCount == 0 )  { 
				CVerbVerbArg ( "Release  [%i]: -> Disposing SP", objID_ );  
        
				sp ( DeviceInstance ) toDispose; bool doRelease = false; 
        
				pthread_mutex_lock ( &objLock ); 
			
				if ( refCountSP == 0 ) { 
					checkSP = true;

					CVerbVerbArg ( "Release  [%i]: -> [%i]", objID_, localRefCount ); 
			
					if ( atLists > 0 )
						abort ();

					toDispose = myselfAtClients; doRelease = true; 
					myselfAtClients = 0; 
				}  
				pthread_mutex_unlock ( &objLock ); 
        
				if ( doRelease ) { ReleaseLocked (); } 
			} 
#endif
        }
        

#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
		void DeviceInstance::CheckSPConsistency ()
        {
			pthread_mutex_lock ( &objLock );

			LONGSYNC spCount = refCountSP;

			if ( spCount <= 0 || !myselfAtClients )
				abort ();

			pthread_mutex_unlock ( &objLock );
        }	

		void DeviceInstance::CheckSPConsistency1 ( long listsAlive )
        {
			pthread_mutex_lock ( &objLock );

			LONGSYNC spCount = refCountSP;

			if ( atLists > 0 && ( spCount <= 1 || !myselfAtClients ) )
				abort ();

			pthread_mutex_unlock ( &objLock );
        }		
#endif
        
        bool DeviceInstance::Init ( int hInst )
        {
            CVerbVerb ( "Init" );
            
            if ( files == nill || hInst <= 0 )
                return false;
            
            hEnvirons = hInst;
            
            C_Only ( env = instances [ hInst ] );

            envObj = EnvironsAPI ( hInst );
            
            if ( envObj == nill )
                return false;
            
            if ( !LockInitA ( devicePortalsLock ) )
                return false;
            
            if ( !LockInitA ( storageLock ) )
                return false;
            
            if ( !LockInitA ( changeEventLock ) )
                return false;
            
            if ( !CondInitA ( changeEvent ) )
                return false;
            
            if ( !CondInitA ( messagesEvent ) )
                return false;
            
            if ( !CondInitA ( filesEvent ) )
                return false;
            
            if ( !CondInitA ( udpDataEvent ) )
                return false;

            return true;
        }
        
        
        void DeviceInstance::DisposeMessages ( )
        {
            if ( !stdQueue_empty ( messages ) )
                DisposeMessagesQ ();
            
            if ( messagesCache != nill )
                DisposeMessages ( messagesCache );
        }
        
        
        void DeviceInstance::DisposeMessages ( c_const NLayerVecType ( EPSPACE MessageInstance ) c_ref msgList )
        {
#ifdef CLI_CPP
            for each ( EPSPACE MessageInstance ^ inst in msgList )
            {
                if ( inst == nill )
                    continue;
                inst->DisposeInstance ();
                delete inst;
            }
#else
            size_t size = msgList->size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                c_const sp ( MessageInstance ) c_ref item = msgList->at ( vctSize i );
                
                if ( item == nill )
                    continue;
                
                if ( item->platformRef ) {
                    item->PlatformDispose ();
                    
                    item->myself.reset ();
                }
                else
                    item->DisposeInstance ();
            }
#endif
            ContainerClear ( msgList );
        }
        
        
        void DeviceInstance::DisposeMessagesQ ( )
        {
            while ( !stdQueue_empty ( messages ) )
            {
                MessageInstanceESP item = stdQueue_front ( messages );
                
                stdQueue_pop ( messages );
                
                if ( item == nill )
                    continue;
#ifdef CLI_CPP
				item->DisposeInstance();
				delete item;
#else
                if ( item->platformRef ) {
                    item->PlatformDispose ();
                    
                    item->myself.reset ();
                }
                else
                    item->DisposeInstance ();
#endif
            }
        }
        
        
        void DeviceInstance::DisposeFiles ( )
        {
            if ( files != nill )
                DisposeFiles ( files );
            
            if ( filesCache != nill )
                DisposeFiles ( filesCache );
        }
        
        
        void DeviceInstance::DisposeFiles ( c_const NLayerMapType ( int, EPSPACE FileInstance ) c_ref list )
        {
#ifdef CLI_CPP
            for each ( KeyValuePair<int, EPSPACE FileInstance ^>^ inst in list )
            {
                if ( inst->Value == nill )
                    continue;
                inst->Value->DisposeInstance ();
                delete inst->Value;
            }
#else
            map < int, sp ( FileInstance ) >::iterator it;
            
            for ( it = list->begin (); it != list->end (); ++it )
            {
                c_const sp ( FileInstance ) c_ref item = it->second;
                
                if ( item == nill )
                    continue;
                
                if ( item->platformRef ) {
                    item->PlatformDispose ();
                    
                    item->myself.reset ();
                }
                else
                    item->DisposeInstance ();
            }
#endif
            ContainerClear ( list );
        }
        
        
        void DeviceInstance::DisposeUdpPacks ()
        {
            LockAcquireVA ( storageLock, "DisposeUdpPacks" );
            
            while ( !stdQueue_empty ( udpData ) )
            {
                UdpDataPack OBJ_ptr item = stdQueue_front ( udpData );
                
                stdQueue_pop ( udpData );
                
                if ( item != nill )
                    delete__obj ( item );
            }
            
            LockReleaseVA ( storageLock, "DisposeUdpPacks" );
        }
        
        
        void DeviceInstance::DisposeInstance ()
        {
            CVerbVerb ( "DisposeInstance" );        
            
            if ( ___sync_val_compare_and_swap ( c_Addr_of ( disposed_ ), 0, 1 ) != 0 )
			{
                CVerbVerbArg1 ( "DisposeInstance: Already disposed", "deviceID", "X", info_->deviceID );
                return;
            }
            
            if ( enableSensorSender != 0 ) {
                envObj->SetSensorEventSenderFlags ( info_->nativeID, info_->objID, enableSensorSender, false );
            }

            LockAcquireVA ( storageLock, "DisposeInstance" );

            // Remove file instances
            DisposeFiles ();
            
            // Remove message instances
            DisposeMessages ();

			LockReleaseVA ( storageLock, "DisposeInstance" );            
            
            // Remove portal instances
#ifdef CLI_CPP
			for each ( PortalInstanceEP ^ inst in devicePortals )
			{
				if ( inst == nill )
					continue;
				inst->DisposeInstance ( false );
				delete inst;
			}
#else
            size_t size = devicePortals.size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
				c_const sp ( PortalInstance ) c_ref inst = devicePortals.at ( vctSize i );
                
                if ( inst == nill )
                    continue;
				inst->DisposeInstance ( false );
            }
#endif
			ContainerdClear ( devicePortals );
            
            NotifyObservers ( ENVIRONS_OBJECT_DISPOSED, true );
            
            // Will be called by list notifier thread (the thread needs the platform object)
            //PlatformDispose ();

			sp_reset ( myself );
        }
        
        
        void c_OBJ_ptr DeviceInstance::NotifierThread ( pthread_param_t arg )
        {
            CVerbVerb ( "NotifierThread" );
            
            EnvironsPtr                                 envObj  = (EnvironsPtr) arg;
            
            DeviceNotifierContextPtr                     ctx     = nill;
            
			envQueueVector ( DeviceNotifierContextPtr ) OBJ_ref q      = envObj->deviceNotifierQueue;
            pthread_mutex_t             OBJ_ref			 lock   = envObj->deviceNotifierLock;
            
            ThreadSync                             OBJ_ref thread  = envObj->deviceNotifierThread;
            
            while ( 1 )
            {
                LockAcquireVA ( lock, "NotifierThread" );
                
                if ( envQueue_empty ( q ) )
                    ctx = nill;
                else {
                    CVerbVerb ( "NotifierThread: Dequeue" );
                    ctx = C_Only ( ( DeviceNotifierContextPtr ) ) envQueue_front ( q );
                    
                    CVerbVerb ( "NotifierThread: pop" );
					envQueue_pop ( q );
                }
                
                LockReleaseVA ( lock , "NotifierThread" );
                
                if ( ctx == nill ) {
                    if ( !envObj->deviceNotifierThreadRun || !thread.WaitOne ( "NotifierThread", ENV_INFINITE_MS, true, false ) )
                        break;
                    continue;
                }

				DeviceInstancePtr device = sp_get ( ctx->device );
                
                if ( device != nill )
                {
                    switch ( ctx->type ) {
                        case 1:
							device->NotifyObservers ( ctx->flags, false );
                            break;
                            
                        case 2:
                            device->NotifyObserversForMessage ( ctx->message, ( environs::MessageInfoFlag_t ) ctx->flags, false );
                            break;
                            
                        case 3:
                            device->NotifyObserversForData ( ctx->fileData, ( environs::FileInfoFlag_t ) ctx->flags, false );
							break;
                    }
                }
                
#ifndef CLI_CPP
                CVerbVerb ( "NotifierThread: delete" );

                CheckNotifierContextContextsWithLock ( &lock, ctx, true );

                delete__obj_n ( ctx );
#endif
                CVerbVerb ( "NotifierThread: next" );
            }
            
            //
            // Drain the queue. Clear all references to device instances and other objects
            //
            envObj->DeviceNotifierQueueClear ();
            
            CVerbVerb ( "NotifierThread: done" );
            
            C_Only ( return nill );
        }
        
        
        bool DeviceInstance::IsEnqueueSafe ()
        {
            // Allow enqueue if the notifier thread is running
            // If not, then don't enqueue if we (our thread context) are safe to deliver the notification ourself
            
            return (envObj->deviceNotifierThreadRun || !envObj->IsDisposalContextSafe ());
        }
        
        
        void DeviceInstance::EnqueueNotification ( DeviceNotifierContextPtr ctx )
        {
            CVerbVerb ( "EnqueueNotification" );
            
            if ( ctx == nill ) return;

			// We must enqueue this notification if the caller thread context is not legible, that is client thread contexts, because
			// we may be called by a client thread (through SendMessage or the like with Call::Wait option
			// In such a case it's the client thread context which would deliver this notification.
			// If we dispose the NotifierContext and a containing DeviceInstance, then it might happen that the client thread waits for itself (the SendMessage thread)
			// to finish .. and deadlock
			// If we don't dispose the DeviceInstance, then memory leaks could occur (depending on client usage)
			// Therefore, we defer the disposal of the enqueued notification to a legible thread context.

			/*if ( !envObj->deviceNotifierThreadRun && envObj->IsDisposalContextSafe () )
			{
                if ( ctx->device != nill )
                    ctx->device->NotifyObservers ( ctx->flags, false );
                
				envObj->DisposeNotifierContext ( ctx );
				delete__obj_n ( ctx );
				return;
			}
             */
            
            LockAcquireVA ( envObj->deviceNotifierLock, "EnqueueNotification" );

            CheckNotifierContextContexts ( ctx, false );
            
#ifdef CLI_CPP
            envQueue_push ( envObj->deviceNotifierQueue, ctx );
            
            CVerbVerb ( "EnqueueNotification: Enqueue" );
#else
			if ( !envQueue_push ( envObj->deviceNotifierQueue, ctx ) )
            {
                CVerbVerb ( "EnqueueNotification: Enqueue: Failed!" );
                
                envObj->DisposeNotifierContext ( ctx );
                delete ( ctx );
            }
            else {
                CVerbVerb ( "EnqueueNotification: Enqueue" );
                
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
                if ( ctx->flags == DeviceInfoFlag::Disposed )
                    ctx->device->disposalEnqueued = true;
#endif
            }
#endif
            envObj->deviceNotifierThread.Notify ( "EnqueueCommand", true );
            
            LockReleaseVA ( envObj->deviceNotifierLock , "EnqueueNotification" );
            
            CVerbVerb ( "EnqueueNotification: done" );
        }
        

        void DeviceInstance::NotifyObservers ( int flags, bool enqueue )
        {
            CVerbVerb ( "NotifyObservers" );

            TRACE_DEVICE_INSTANCE ( lastObserversSize = 0 );

#ifndef CLI_CPP
            if ( observers.size () <= 0 )
                return;
#endif
			if ( enqueue && IsEnqueueSafe () && !CPP_CLI ( env->disposing, environs::API::GetDisposingN ( hEnvirons ) ) )
            {
                DeviceNotifierContextPtr item = new__obj ( DeviceNotifierContext );
                if ( item == nill )
                    return;
                item->type      = 1;
                item->flags     = flags;
#ifdef USE_WP_MYSELF
				item->device    = CPP_CLI ( myself.lock (), this );
#else
                item->device    = CPP_CLI ( myself, this );
#endif
				Cli_Only ( item->propertyName = nill );

                EnqueueNotification ( item );
                return;
            }
            
            //CVerbVerbArg1 ( "NotifyObservers", "Observer size", "i", vp_size ( observers ) );
            
#ifdef CLI_CPP
			observers ( GetPlatformObj (), ( DeviceInfoFlag ) flags );
#else
            vct ( lib::IIDeviceObserver * ) obss;
            
            LockAcquireVA ( changeEventLock, "NotifyObservers" );
            
            size_t size = observers.size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIDeviceObserver OBJ_ptr obs = observers.at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( changeEventLock, "NotifyObservers" );
            
            size = obss.size ();

            TRACE_DEVICE_INSTANCE ( lastObserversSize = size );

            if ( size <= 0 )
                return;
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIDeviceObserver OBJ_ptr obs = obss.at ( vctSize i );
                
                try {
                    CVerbVerbArg1 ( "NotifyObservers", "", "i", i );
                    
                    if ( obs->OnDeviceChangedInternal_ )
                    {
                        obs->OnDeviceChangedInternal ( ( DeviceInfoFlag_t ) flags );
                    }
                    else {
                        if ( obs->OnDeviceChanged_ )
                        {
                            obs->OnDeviceChangedBase ( this, ( DeviceInfoFlag_t ) flags );
                        }
                        if ( obs->OnDeviceChangedInterface_ )
                        {
                            obs->OnDeviceChangedInterface ( this, ( DeviceInfoFlag_t ) flags );
                        }
                    }
                }
                catch ( ... ) {
                    CErr ( "NotifyObservers: Exception!" );
                }
            }
            
#	ifdef DEBUG_TRACK_DEVICE_INSTANCE
            if ( flags == DeviceInfoFlag::Disposed )
                disposalNotified = true;
#	endif            
#endif
        }
        
        
		void DeviceInstance::NotifyObserversForMessage ( c_const MessageInstanceESP c_ref message, environs::MessageInfoFlag_t flags, bool enqueue )
        {
            CVerbVerb ( "NotifyObserversForMessage" );
            
            if ( message == nill ) return;
            
#ifndef CLI_CPP
            if ( observersForMessages.size () <= 0 )
                return;
#endif
			if ( enqueue && IsEnqueueSafe () && !CPP_CLI ( env->disposing, environs::API::GetDisposingN ( hEnvirons ) ) )
            {
				DeviceNotifierContextPtr item = new__obj ( DeviceNotifierContext );
                if ( item == nill )
                    return;
                
                item->type      = 2;
				item->flags     = ( int ) flags;
#ifdef USE_WP_MYSELF
				item->device    = CPP_CLI ( myself.lock (), this );
#else
				item->device    = CPP_CLI ( myself, this );
#endif
                item->message   = message;
                
                EnqueueNotification ( item );
                return;
            }

#ifdef CLI_CPP
			observersForMessages ( message, flags );
#else
            vct ( lib::IIMessageObserver * ) obss;
            
            LockAcquireVA ( changeEventLock, "NotifyObserversForMessage" );
            
            size_t size = observersForMessages.size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIMessageObserver OBJ_ptr obs = observersForMessages.at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( changeEventLock, "NotifyObserversForMessage" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; ++i )
            {
                lib::IIMessageObserver OBJ_ptr obs = obss.at ( vctSize i );
                try {
                    if ( !obs->OnMessage_ )
                        continue;
                    
                    if ( obs->OnMessageInternal_ )
                    {
                        obs->OnMessageInternal ( message, flags );
                    }
                    else {
                        if ( obs->OnMessageInterface_ )
                        {
                            obs->OnMessageInterface ( sp_get ( message ), flags );
                        }
                        if ( obs->OnMessage_ )
                        {
                            obs->OnMessageBase ( sp_get ( message ), flags );
                        }
                    }
                }
                catch ( ... ) {
                    CErr ( "NotifyObservers: Exception!" );
                }
            }
#endif
        }
        
        
		void DeviceInstance::NotifyObserversForData ( c_const FileInstanceESP c_ref fileSP, environs::FileInfoFlag_t flags, bool enqueue )
        {
            CVerbVerb ( "NotifyObserversForData" );
            
            if ( fileSP == nill ) return;
            
#ifndef CLI_CPP
            if ( observersForData.size () <= 0 )
                return;
#endif

			if ( enqueue && IsEnqueueSafe () && !CPP_CLI ( env->disposing, environs::API::GetDisposingN ( hEnvirons ) ) )
            {
				DeviceNotifierContextPtr item = new__obj ( DeviceNotifierContext );
                if ( item == nill )
                    return;
                
                item->type      = 3;
				item->flags     = ( int ) flags;
#ifdef USE_WP_MYSELF
				item->device    = CPP_CLI ( myself.lock (), this );
#else
				item->device    = CPP_CLI ( myself, this );
#endif
                item->fileData	= fileSP;
            
                EnqueueNotification ( item );
                return;
            }

#ifdef CLI_CPP
			observersForData ( fileSP, flags );
#else
            vct ( lib::IIDataObserver * ) obss;
            
            LockAcquireVA ( changeEventLock, "NotifyObserversForData" );
            
            size_t size = observersForData.size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IIDataObserver OBJ_ptr obs = observersForData.at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( changeEventLock, "NotifyObserversForData" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; ++i )
            {
                lib::IIDataObserver OBJ_ptr obs = obss.at ( vctSize i );
                try {
                    if ( !obs->OnData_ )
                        continue;
                    
                    if ( obs->OnDataInternal_ )
                    {
                        obs->OnDataInternal ( fileSP, flags );
                    }
                    else {
                        if ( obs->OnDataInterface_ )
                        {
                            obs->OnDataInterface ( sp_get ( fileSP ), flags );
                        }
                        if ( obs->OnData_ )
                        {
                            obs->OnDataBase ( sp_get ( fileSP ), flags );
                        }
                    }
                }
                catch ( ... ) {
                    CErr ( "NotifyObservers: Exception!" );
                }
            }
#endif
        }
        
        
        /**
         * Add an observer (DeviceObserver) that notifies about device property changes.
         *
         * @param observer A DeviceObserver
         */
        void DeviceInstance::AddObserver ( environs::DeviceObserverPtr observer )
        {
            CVerbVerb ( "AddObserver" );
            
            if ( observer == nill )
                return;            
#ifdef CLI_CPP
			observers += observer;

			notifyPropertyChanged = false;

			SetDeviceFlags ( DeviceFlagsInternal::ObserverReady, environs::Call::NoWait, true );
#else
            LockAcquireVA ( changeEventLock, "AddObserver" );
            
            size_t i=0;
			size_t size = observers.size ();

			lib::IIDeviceObserver OBJ_ptr obsc = ( lib::IIDeviceObserver OBJ_ptr ) observer;
            
            for ( ; i<size; i++ ) {
				lib::IIDeviceObserver OBJ_ptr obs = observers.at ( vctSize i );
                
				if ( obsc == obs )
                    break;
            }
            
            if ( i >= size ) {
                observers.push_back ( obsc );
                
                SetDeviceFlags ( DeviceFlagsInternal::ObserverReady, environs::Call::NoWait, true );
            }
            
            LockReleaseVA ( changeEventLock, "AddObserver" );
#endif
        }
        
        
        /**
         * Remove an observer (DeviceObserver) that was added before.
         *
         * @param observer A DeviceObserver
         */
        void DeviceInstance::RemoveObserver ( environs::DeviceObserverPtr observer )
        {
            CVerbVerb ( "RemoveObserver" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
			observers -= observer;

            notifyPropertyChanged = notifyPropertyChangedDefault;

			//SetDeviceFlags ( ( int ) DeviceFlagsInternal::ObserverReady, environs::Call::NoWait, false );
#else
            LockAcquireVA ( changeEventLock, "RemoveObserver" );
            
			size_t size = observers.size ();

			lib::IIDeviceObserver OBJ_ptr obsc = ( lib::IIDeviceObserver OBJ_ptr ) observer;
            
            for ( size_t i=0; i<size; i++ ) {
				lib::IIDeviceObserver OBJ_ptr obs = observers.at ( vctSize i );
                
				if ( obsc == obs ) {
                    observers.erase ( observers.begin () + vctSize i );
                    
                    if ( observers.size () <= 0 )
                        SetDeviceFlags ( DeviceFlagsInternal::ObserverReady, environs::Call::NoWait, false );
                    break;
                }
            }
            
            LockReleaseVA ( changeEventLock, "RemoveObserver" );
#endif
        }
        
        
        /**
         * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
         *
         * @param observer A DataObserver
         */
        void DeviceInstance::AddObserverForData ( environs::DataObserverPtr observer )
        {
            CVerbVerb ( "AddObserverForData" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
            observersForData += observer;
            
            SetDeviceFlags ( DeviceFlagsInternal::DataReady, environs::Call::NoWait, true );
#else
            LockAcquireVA ( changeEventLock, "AddObserverForData" );
            
            size_t i=0;
			size_t size = observersForData.size ();

			lib::IIDataObserver OBJ_ptr obsc = ( lib::IIDataObserver OBJ_ptr ) observer;
            
            for ( ; i<size; i++ ) {
				lib::IIDataObserver OBJ_ptr obs = observersForData.at ( vctSize i );
                
				if ( obsc == obs )
                    break;
            }
            
            if ( i >= size ) {
                observersForData.push_back ( obsc );
                
                SetDeviceFlags ( DeviceFlagsInternal::DataReady, environs::Call::NoWait, true );
            }
            
            LockReleaseVA ( changeEventLock, "AddObserverForData" );
#endif
            filesLast = -1;
        }
        
        
        /**
         * Remove an observer (SensorObserver) that was added before.
         *
         * @param observer A DataObserver
         */
        void DeviceInstance::RemoveObserverForData ( environs::DataObserverPtr observer )
        {
            CVerbVerb ( "RemoveObserverForData" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
			observersForData -= observer;
#else
            LockAcquireVA ( changeEventLock, "RemoveObserverForData" );
            
			size_t size = observersForData.size ();

			lib::IIDataObserver OBJ_ptr obsc = ( lib::IIDataObserver OBJ_ptr ) observer;
            
            for ( size_t i=0; i<size; i++ ) {
				lib::IIDataObserver OBJ_ptr obs = observersForData.at ( vctSize i );
                
				if ( obsc == obs ) {
                    observersForData.erase ( observersForData.begin () + vctSize i );
                    
                    if ( observersForData.size () <= 0 && !disposed_ ) {
                        filesLast = (int) files->size ();
                        
                        SetDeviceFlags ( DeviceFlagsInternal::DataReady, environs::Call::NoWait, false );
                    }
                    break;
                }
            }
            
            LockReleaseVA ( changeEventLock, "RemoveObserverForData" );
#endif
        }
        
        
        /**
         * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
         *
         * @param observer A DataObserver
         */
        void DeviceInstance::AddObserverForSensors ( environs::SensorObserverPtr observer )
        {
            CVerbVerb ( "AddObserverForSensors" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
			observersForSensorData += observer;

			SetDeviceFlags ( DeviceFlagsInternal::SensorReady, environs::Call::NoWait, true );
#else
            LockAcquireVA ( changeEventLock, "AddObserverForSensors" );
            
            size_t i=0;
			size_t size = observersForSensorData.size ();

			lib::IISensorObserver OBJ_ptr obsc = ( lib::IISensorObserver OBJ_ptr ) observer;
            
            for ( ; i<size; i++ ) {
				lib::IISensorObserver OBJ_ptr obs = observersForSensorData.at ( vctSize i );
                
				if ( obsc == obs )
                    break;
            }
            
            if ( i >= size ) {
                observersForSensorData.push_back ( obsc );

				SetDeviceFlags ( DeviceFlagsInternal::SensorReady, environs::Call::NoWait, true );
            }
            
            LockReleaseVA ( changeEventLock, "AddObserverForSensors" );
#endif
        }
        
        
        /**
         * Remove an observer (DataObserver) that was added before.
         *
         * @param observer A DataObserver
         */
        void DeviceInstance::RemoveObserverForSensors ( environs::SensorObserverPtr observer )
        {
            CVerbVerb ( "RemoveObserverForSensors" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
			observersForSensorData -= observer;
#else
            LockAcquireVA ( changeEventLock, "RemoveObserverForSensors" );
            
			size_t size = observersForSensorData.size ();

			lib::IISensorObserver OBJ_ptr obsc = ( lib::IISensorObserver OBJ_ptr ) observer;
            
            for ( size_t i=0; i<size; i++ ) {
				lib::IISensorObserver OBJ_ptr obs = observersForSensorData.at ( vctSize i );
                
				if ( obsc == obs ) {
                    observersForSensorData.erase ( observersForSensorData.begin () + vctSize i );
                    break;
                }
            }
            
            LockReleaseVA ( changeEventLock, "RemoveObserverForSensors" );
#endif
        }
        
        
        /**
         * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
         *
         * @param observer A MessageObserver
         */
        void DeviceInstance::AddObserverForMessages ( environs::MessageObserverPtr observer )
        {
            CVerbVerb ( "AddObserverForMessages" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
            messagesEnqueue = false;
            
            observersForMessages += observer;
            
            SetDeviceFlags ( DeviceFlagsInternal::MessageReady, environs::Call::NoWait, true );
#else
            LockAcquireVA ( changeEventLock, "AddObserverForMessages" );
            
            size_t i=0;
			size_t size = observersForMessages.size ();

			lib::IIMessageObserver OBJ_ptr obsc = ( lib::IIMessageObserver OBJ_ptr ) observer;
            
            for ( ; i<size; i++ ) {
				lib::IIMessageObserver OBJ_ptr obs = observersForMessages.at ( vctSize i );
                
				if ( obsc == obs )
                    break;
            }
            
            if ( i >= size ) {
                messagesEnqueue = false;
                
                observersForMessages.push_back ( obsc );
                
                SetDeviceFlags ( DeviceFlagsInternal::MessageReady, environs::Call::NoWait, true );
            }
            
            LockReleaseVA ( changeEventLock, "AddObserverForMessages" );
#endif
        }
        
        
        /**
         * Remove an observer (MessageObserver) that was added before.
         *
         * @param observer A MessageObserver
         */
        void DeviceInstance::RemoveObserverForMessages ( environs::MessageObserverPtr observer )
        {
            CVerbVerb ( "RemoveObserverForMessages" );
            
            if ( observer == nill )
                return;
#ifdef CLI_CPP
			observersForMessages -= observer;
#else
            LockAcquireVA ( changeEventLock, "RemoveObserverForMessages" );
            
			size_t size = observersForMessages.size ();

			lib::IIMessageObserver OBJ_ptr obsc = ( lib::IIMessageObserver OBJ_ptr ) observer;
            
            for ( size_t i=0; i<size; i++ ) {
				lib::IIMessageObserver OBJ_ptr obs = observersForMessages.at ( vctSize i );
                
				if ( obsc == obs ) {
                    observersForMessages.erase ( observersForMessages.begin () + vctSize i );
                    
                    if ( observersForMessages.size () <= 0 && !disposed_ ) {
                        messagesEnqueue = true;
                        
                        SetDeviceFlags ( DeviceFlagsInternal::MessageReady, environs::Call::NoWait, false );
                    }
                    break;
                }
            }
            
            LockReleaseVA ( changeEventLock, "RemoveObserverForMessages" );
#endif
        }
        
        
        /**
         * Update device flags to native layer and populate them to the environment.
         *
         * @param	flags    The internal flags to set or clear.
         * @param	set    	 true = set, false = clear.
         */
        void DeviceInstance::SetDeviceFlags ( environs::DeviceFlagsInternal_t flags, environs::Call_t callAsync, bool set )
        {
            if ( disposed_ )
                return;
            
			unsigned short f = ( unsigned short ) flags;
            if ( set ) {
                if ( (info_->flags & f ) != 0 )
                    return;
                info_->flags |= f;
            }
            else {
                if ( (info_->flags & f ) == 0 )
                    return;
                info_->flags &= f;
            }

			environs::API::SetDeviceFlagsN ( hEnvirons, ( int ) callAsync, info_->objID, set ? info_->flags : ( int ) flags, set );
        }
        
        
		DeviceInstanceESP DeviceInstance::Create ( int hInst, environs::DeviceInfoPtr device )
        {
            CVerbVerb ( "Create" );
            
            if ( device != nill )
            {
				DeviceInstanceESP deviceSP = sp_make ( DeviceInstanceEP );
                if ( deviceSP != nill )
                {
                    if ( deviceSP->Init ( hInst ) && deviceSP->CopyInfo ( device ) )
                    {
                        C_Only ( deviceSP->myself = deviceSP; );

                        TRACE_DEVICE_INSTANCE ( deviceSP->myselfWP = deviceSP );

                        return deviceSP;
                    }
                    
                    deviceSP->DisposeInstance ();
                    deviceSP->PlatformDispose ();
                    
                    //sp_set_no_cli ( deviceSP->myself, nill );
                }
            }
            return nill;
        }

        
        bool DeviceInstance::CopyInfo ( environs::DeviceInfoPtr device )
        {
            CVerbVerb ( "CopyInfo" );
            
            if ( device != nill ) {
#ifdef CLI_CPP
				info_ = device;
#else
                environs::DeviceInfo * tmp      = nill;
                environs::DeviceInfo * copyTo   = info_;

				if ( copyTo == &emptyInfo ) {
                    tmp = new environs::DeviceInfo ();
					if ( !tmp )
						return false;
                    copyTo = tmp;
				}

                memcpy ( copyTo, device, sizeof ( environs::DeviceInfo ) );

                if ( tmp )
                    info_ = tmp;
#endif           
                isSameAppArea = ( info_->hasAppEnv == 0 ); //EqualsAppEnv ( nill, nill );
#ifndef CLI_CPP
				CVerbVerbArg ( "CopyInfo    deviceID [ %X / %i ]    name [ %s ]  areaApp [ %s : %s]", device->deviceID, device->broadcastFound, device->deviceName, device->areaName, device->appName );
                CVerbVerbArg ( "CopyInfo    deviceID [ %X / %i ]    name [ %s ]  areaApp [ %s : %s]", info_->deviceID, info_->broadcastFound, info_->deviceName, info_->areaName, info_->appName );
#endif
                return true;
            } 
            return false;
        }
        
        
        bool DeviceInstance::Update ( environs::DeviceInfoPtr device )
        {
            CVerbVerb ( "Update" );
            
			int changed = 0; bool rebuild = false;

            TRACE_DEVICE_INSTANCE ( gotUpdates1++ );
            
            if ( info_->deviceID != device->deviceID ) {
				changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;
				info_->deviceID = device->deviceID;

				DeviceInstancePropertyNotify ( "deviceID", false );
            }
            
            if ( info_->nativeID != device->nativeID ) {
                changed |= DEVICE_INFO_ATTR_NATIVEID;
				info_->nativeID = device->nativeID;

				CVerbVerbArg1 ( "Update: Set ", "nativeID", "i", info_->nativeID );

				DeviceInstancePropertyNotify ( "nativeID", false );
            }
            
            if ( info_->objID != device->objID ) {
                changed |= DEVICE_INFO_ATTR_OBJID;
                
                objIDPrevious = info_->objID;
                info_->objID = device->objID;
                
                //DeviceInstanceNotify ( notifyPropertyChanged ? gcnew String ( "nativeID" ) : nill, DEVICE_INFO_ATTR_NATIVEID );
            }
            
            if ( info_->ip != device->ip ) {
                changed |= DEVICE_INFO_ATTR_IP; rebuild = true;
				info_->ip = device->ip;
#ifdef CLI_CPP
				ips_ = "";
#else
				LockAcquireVA ( changeEventLock, "Update" );

				ips_.clear ();

				LockReleaseVA ( changeEventLock, "Update" );
#endif
				DeviceInstancePropertyNotify ( "ip", false );
            }
            
            if ( info_->ipe != device->ipe ) {
                changed |= DEVICE_INFO_ATTR_IPE; rebuild = true;
				info_->ipe = device->ipe;
#ifdef CLI_CPP
				ipes_ = "";
#else
				LockAcquireVA ( changeEventLock, "Update" );

				ipes_.clear ();

				LockReleaseVA ( changeEventLock, "Update" );
#endif
				DeviceInstancePropertyNotify ( "ipe", false );
            }
            
            if ( info_->tcpPort != device->tcpPort ) {
                changed |= DEVICE_INFO_ATTR_TCP_PORT; rebuild = true;
				info_->tcpPort = device->tcpPort;

				DeviceInstancePropertyNotify ( "tcpPort", false );
            }
            
            if ( info_->udpPort != device->udpPort ) {
                changed |= DEVICE_INFO_ATTR_UDP_PORT; rebuild = true;
				info_->udpPort = device->udpPort;

				DeviceInstancePropertyNotify ( "udpPort", false );
            }
			info_->updates = device->updates;
			
            if ( info_->platform != device->platform ) {
                changed |= DEVICE_INFO_ATTR_DEVICE_PLATFORM; rebuild = true;
				info_->platform = device->platform;

				DeviceInstancePropertyNotify ( "platform", false );
            }
            
            if ( info_->broadcastFound != device->broadcastFound ) {
                changed |= DEVICE_INFO_ATTR_BROADCAST_FOUND; rebuild = true;
				info_->broadcastFound = device->broadcastFound;

				DeviceInstancePropertyNotify ( "broadcastFound", false );
            }
            
            if ( info_->unavailable != device->unavailable ) {
                changed |= DEVICE_INFO_ATTR_UNAVAILABLE;
				info_->unavailable = device->unavailable;

				DeviceInstancePropertyNotify ( "unavailable", false );
            }
            
            if ( info_->isConnected != device->isConnected ) {
                changed |= DEVICE_INFO_ATTR_ISCONNECTED; Cli_Only ( rebuild = true; )
				info_->isConnected = device->isConnected;

				CVerbVerbArg1 ( "Update: Set ", "isConnected", "i", info_->isConnected );

				if ( !info_->isConnected ) {
					info_->nativeID = 0;
					changed |= DEVICE_INFO_ATTR_NATIVEID;
                    
                    DisposeUdpPacks ();
				}
				DeviceInstancePropertyNotify ( "isConnected", false );
                
				CVerbVerbArg1 ( "Update: Changed connect status ", "", "i", info_->isConnected );
                
                if ( device->isConnected && changeEventPending ) 
				{
                    if ( changeEventPending ) {
                        /// Lock the changeEvent mutex
                        pthread_cond_mutex_lock ( &changeEventLock );
                        
                        /// Signal a changeEvent
                        if ( pthread_cond_signal ( c_Addr_of ( changeEvent ) ) ) {
                            CVerbVerb ( "Update: Failed to signal changeEvent!" );
                        }
                        
                        /// UnLock the changeEvent mutex
                        pthread_cond_mutex_unlock ( &changeEventLock );
                        
                        changeEventPending = false;
                    }
                    
                    if ( info_->nativeID > 0 && enableSensorSender != 0 ) {
                        envObj->SetSensorEventSenderFlags ( info_->nativeID, info_->objID, enableSensorSender, true );
                    }
                }
            }
            
			if ( CString_ptr_empty ( info_->deviceName ) || CString_ptr_empty ( device->deviceName ) || CString_compare ( info_->deviceName, device->deviceName, sizeof ( info_->deviceName ) ) ) {
                changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;

				C_Only ( info_->deviceName [ sizeof ( info_->deviceName ) - 1 ] = 0; ) // Make sure a concurrent read will not overrun the buffer

				CString_copy ( info_->deviceName, sizeof ( info_->deviceName ), device->deviceName );

				DeviceInstancePropertyNotify ( "deviceName", false );
			}
            
            if ( CString_ptr_empty ( info_->areaName ) || CString_ptr_empty ( device->areaName ) || CString_compare ( info_->areaName, device->areaName, sizeof ( info_->areaName ) ) ) {
                changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;
#ifndef CLI_CPP
                info_->areaName [ sizeof ( info_->areaName ) - 1 ] = 0; // Make sure a concurrent read will not overrun the buffer
#endif
				CString_copy ( info_->areaName, sizeof ( info_->areaName ), device->areaName );

				DeviceInstancePropertyNotify ( "areaName", false );
            }
            
            if ( CString_ptr_empty ( info_->areaName ) ) {
                changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;
#ifndef CLI_CPP
                info_->areaName [ sizeof ( info_->areaName ) - 1 ] = 0; // Make sure a concurrent read will not overrun the buffer
#endif
				CString_copy ( info_->areaName, sizeof ( info_->areaName ), CPP_CLI ( env->areaName, CCharToString ( environs::API::GetAreaNameN ( hEnvirons ) ) ) );

				DeviceInstancePropertyNotify ( "areaName", false );
            }
            
            if ( CString_ptr_empty ( info_->appName ) || CString_ptr_empty ( device->appName ) || CString_compare ( info_->appName, device->appName, sizeof ( info_->appName ) ) ) {
                changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;
#ifndef CLI_CPP
                info_->appName [ sizeof ( info_->appName ) - 1 ] = 0; // Make sure a concurrent read will not overrun the buffer
#endif
				CString_copy ( info_->appName, sizeof ( info_->appName ), device->appName );

				DeviceInstancePropertyNotify ( "appName", false );
            }
            
            if ( CString_ptr_empty ( info_->appName ) ) {
                changed |= DEVICE_INFO_ATTR_IDENTITY; rebuild = true;
#ifndef CLI_CPP
                info_->appName [ sizeof ( info_->appName ) - 1 ] = 0; // Make sure a concurrent read will not overrun the buffer
#endif
				CString_copy ( info_->appName, sizeof ( info_->appName ), CPP_CLI ( env->appName, CCharToString ( environs::API::GetApplicationNameN ( hEnvirons ) ) ) );

				DeviceInstancePropertyNotify ( "appName", false );
            }
			
			if ( rebuild ) {
#ifdef CLI_CPP
				toString_ = "";
#else
				LockAcquireVA ( changeEventLock, "Update" );

				toString_.clear ();

				LockReleaseVA ( changeEventLock, "Update" );
#endif
			}

            if ( changed ) 
			{
                if ( changed & DEVICE_INFO_ATTR_IDENTITY )
                    isSameAppArea = ( info_->hasAppEnv == 0 ); //EqualsAppEnv ( nill, nill );

				NotifyObservers ( changed, true );

				if ( (changed & DEVICE_INFO_ATTR_ISCONNECTED) == DEVICE_INFO_ATTR_ISCONNECTED && device->isConnected )
                {
					void * ddn = environs::API::GetDeviceDisplayPropsN ( hEnvirons, device->nativeID );

					environs::DeviceDisplay OBJ_ptr dd = ( environs::DeviceDisplay OBJ_ptr ) DisplayPropsToPlatform ( ddn, device->nativeID );
					
                    if ( dd != nill ) {
#ifdef CLI_CPP
						display = *dd;
#else
                        memcpy ( &display, dd, sizeof ( display ) );
#endif
                    }
					if ( ddn )
						free_plt ( ddn );
                }
            }
            return (changed != 0);
        }
        
        
        bool DeviceInstance::isConnected ( )
        {
            CVerbVerb ( "isConnected" );
            
            return info_->isConnected;
        }

		bool DeviceInstance::isObserverReady ()
		{
			CVerbVerb ( "isObserverReady" );

			return ( ( info_->flags & ( unsigned short ) DeviceFlagsInternal::CPObserverReady ) != 0 );
		}

		bool DeviceInstance::isMessageObserverReady ()
		{
			CVerbVerb ( "isMessageObserverReady" );

			return ( ( info_->flags & ( unsigned short ) DeviceFlagsInternal::CPMessageReady ) != 0 );
		}

		bool DeviceInstance::isDataObserverReady ()
		{
			CVerbVerb ( "isDataObserverReady" );

			return ( ( info_->flags & ( unsigned short ) DeviceFlagsInternal::CPDataReady ) != 0 );
		}

		bool DeviceInstance::isSensorObserverReady ()
		{
			CVerbVerb ( "isSensorObserverReady" );

			return ( ( info_->flags & ( unsigned short ) DeviceFlagsInternal::CPSensorReady ) != 0 );
		}
        
        
        bool DeviceInstance::isLocationNode ( )
        {
            CVerbVerb ( "isLocationNode" );

			return ( ( info_->platform & ( int ) Platforms::LocationNode_Flag ) != 0 );
        }
        

		bool DeviceInstance::directStatus ()
		{
			CVerbVerb ( "directStatus" );

			return directStatus_;
		}


		environs::DeviceSourceType_t DeviceInstance::sourceType ()
		{
			CVerbVerb ( "sourceType" );
            
			return ( environs::DeviceSourceType_t) info_->broadcastFound;
		}
        
        
        /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
         Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
        bool DeviceInstance::GetAllowConnect ()
        {
            return (environs::API::AllowConnectN ( hEnvirons, info_->objID, -1 ) != 0);
        }
        
        
        /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
         Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
        void DeviceInstance::SetAllowConnect ( bool value )
        {
            environs::API::AllowConnectN ( hEnvirons, info_->objID, value ? 1 : 0 );
        }
        
        
        bool DeviceInstance::disposed ( )
        {
            CVerbVerb ( "disposed" );

			return ( disposed_ == 1 );
        }
        
        
        /** The device properties structure into a DeviceInfo object. */
        environs::DeviceInfoPtr DeviceInstance::info ()
        {
            CVerbVerb ( "info" );
            
            return info_;
        }
        
        
        void DeviceInstance::SetProgress ( int progress )
        {
            CVerbVerb ( "SetProgress" );
            
            if ( connectProgress == (short) progress )
                return;
            
            connectProgress = (short) progress;
            
            NotifyObservers ( (int) DeviceInfoFlag::ConnectProgress, true );
        }

        
        bool DeviceInstance::SetDirectContact ( int status )
        {
            CVerbVerb ( "SetDirectContact" );
            
			bool s = ( status != 0 );
            if ( directStatus_ == s )
                return false;
            directStatus_ = s;
            
            NotifyObservers ( (int) DeviceInfoFlag::DirectContact, true );
            
#ifdef ENVIRONS_IOS
            if ( devicePortals.size() <= 0 ) {
                if ( status == 1 && env->usePortalAutoStart )
                {
                    sp ( environs::PortalInstance ) portal = PortalGetIncoming ();
                    if ( portal == nill )
                        API::RequestPortalStreamN ( hEnvirons, info_->nativeID, async, PortalType::Any, 0, 0 );
                }
                
            }
            else {
#endif
				for ( int i=0; i < ( int ) vd_size ( devicePortals ); i++ )
                {
					PortalInstanceESP portal = vd_at ( devicePortals, vctSize i );
                    
                    if ( portal != nill && portal->portalID_ > 0 )
                    {
                        try {
                            portal->NotifyObservers ( Notify::Portale::ContactChanged );
                            
#ifdef ENVIRONS_IOS
                            if ( status == 1 && env->usePortalAutoStart ) {
                                portal->Start ();
                            }
#endif
                        }
                        catch ( ... ) {
                            //if ( vd_size ( devicePortals ) > (unsigned) i ) {
                            //    ContainerdRemoveAt ( devicePortals, i );
                            //}
                            //if ( i > 0 ) i--;
                        }
                    }
                }
#ifdef ENVIRONS_IOS
            }
#endif
            return true;
        }
        
        
        bool DeviceInstance::SetFileProgress ( int fileID, int progress, bool send )
        {
            CVerbVerb ( "SetFileProgress" );
            
			FileInstanceESP fileInst;

			LockAcquireVA ( storageLock, "SetFileProgress" );
			
			ContainerIfContains ( files, fileID )
				fileInst = ( *files ) [ fileID ];

			LockReleaseVA ( storageLock, "SetFileProgress" );

            if ( fileInst == nill )
                return false;
            
            if ( send ) {
                fileInst->sendProgress_ = progress;
                
                NotifyObserversForData ( fileInst, FileInfoFlag::SendProgress, true );
            }
            else {
                fileInst->receiveProgress_ = progress;
                
                NotifyObserversForData ( fileInst, FileInfoFlag::ReceiveProgress, true );
            }
            
			if ( progress >= 100 ) {
				// Determine final size
				if ( !STRING_empty ( fileInst->path_ ) )
					fileInst->size_ = ( long ) GetSizeOfFile ( STRING_get ( fileInst->path_ ) );

				NotifyObserversForData ( fileInst, FileInfoFlag::Available, true );				
			}
            return true;
        }
        
        
        /**
         * Enable sending of sensor events to this DeviceInstance.
         * Events are send if the device is connected and stopped if the device is disconnected.
         *
         * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
         * @param enable true = enable, false = disable.
         *
         * @return success true = enabled, false = failed.
         */
        bool DeviceInstance::SetSensorEventSending ( environs::SensorType_t type, bool enable )
        {
            CVerbVerb ( "SetSensorEventSending" );
            
			int typeID = ( int ) type;

            if ( typeID < 0 || typeID >= ENVIRONS_SENSOR_TYPE_MAX )
                return false;
            
            if ( !API::IsSensorAvailableN ( hEnvirons, (int) type ) )
                return false;
            
            //if ( enable == ( ( enableSensorSender & sensorFlags [ typeID ] ) != 0))
            //    return true;
            
            if (enable)
                enableSensorSender |= sensorFlags [ typeID ];
            else
                enableSensorSender &= ~(sensorFlags [ typeID ]);

            if ( info_->isConnected && info_->nativeID > 0 ) {
                return envObj->SetSensorEventSender ( info_->nativeID, info_->objID, type, enable ) != 0;
            }
            return true;
        }


		/**
		* Query whether sending of the given sensor events to this DeviceInstance is enabled or not.
		*
		* @param type	A value of type Environs::SensorType / environs::SensorType_t.
		*
		* @return success true = enabled, false = disabled.
		*/
		bool DeviceInstance::IsSetSensorEventSending ( environs::SensorType_t type )
		{
			CVerbVerb ( "IsSetSensorEventSending" );

			int typeID = ( int ) type;

			if ( typeID < 0 || typeID >= ENVIRONS_SENSOR_TYPE_MAX )
				return false;

			return ( ( enableSensorSender & sensorFlags [ typeID ] ) != 0 );
		}
        
                
        /**
         * Notify to all observers (DeviceObserver) that the appContext has changed.
         *
         * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
         */
        void DeviceInstance::NotifyAppContextChanged ( int customFlags )
        {
            CVerbVerb ( "NotifyAppContextChanged" );
            
            if ( customFlags == 0 )
                customFlags = DEVICE_INFO_ATTR_APP_CONTEXT;

			NotifyObservers ( customFlags, true );
			
#ifdef CLI_CPP
			String ^ cf = appContext0String;

			switch(customFlags) {
			case 1:
				cf = appContext1String; break;
			case 2:
				cf = appContext2String; break;
			case 3:
				cf = appContext3String; break;
			default: break;
			}

			DeviceInstancePropertyNotify ( cf, true );
#endif
        }
        
        
		STRING_T DeviceInstance::ips ()
        {
            CVerbVerb ( "GetIP" );

			unsigned int ip = info_->ip;
#ifdef CLI_CPP

			return String::Format ( "{0:D}.{1:D}.{2:D}.{3:D}",
				( ip & 0xff ),
				( ip >> 8 & 0xff ),
				( ip >> 16 & 0xff ),
				( ip >> 24 & 0xff ));
#else
			LockAcquireVA ( changeEventLock, "ips" );

            if ( ips_.empty() )
            {
                char * s = inet_ntoa ( *((struct in_addr *)&ip) );
                if ( s )
                    ips_ = s;
            }

			STRING_T copy = ips_;

			LockReleaseVA ( changeEventLock, "ips" );

            return copy;
#endif
        }
        
		STRING_T DeviceInstance::ipes ()
        {
            CVerbVerb ( "GetIPe" );

			unsigned int ip = info_->ipe;

#ifdef CLI_CPP
			return String::Format ( "{0:D}.{1:D}.{2:D}.{3:D}",
				( ip & 0xff ),
				( ip >> 8 & 0xff ),
				( ip >> 16 & 0xff ),
				( ip >> 24 & 0xff ));
#else
			LockAcquireVA ( changeEventLock, "ipes" );

            if ( ipes_.empty() )
            {
                char * s = inet_ntoa ( *( ( struct in_addr * )&ip ) );
                if ( s )
                    ipes_ = s;
            }

			STRING_T copy = ipes_;

			LockReleaseVA ( changeEventLock, "ipes" );

			return copy;
#endif
        }
        
        
        bool DeviceInstance::EqualsAppEnv ( environs::DeviceInfoPtr equalTo )
        {
            CVerbVerb ( "EqualsAppEnv" );
            
            if ( !equalTo )
                return false;

			return ( EqualsAppEnv ( ( CString_ptr ) c_Addr_of ( equalTo->areaName ), ( CString_ptr ) c_Addr_of ( equalTo->appName ) ) );
        }
        
        bool DeviceInstance::EqualsAppEnv ( CString_ptr areaName, CString_ptr appName )
        {
            CVerbVerb ( "EqualsAppEnv" );

			if ( areaName == nill )
#ifdef CLI_CPP
				areaName = envObj->GetAreaName ();
#else
				areaName = env->areaName;
#endif
            
			if ( appName == nill )
#ifdef CLI_CPP
				appName = envObj->GetApplicationName ();
#else
				appName = env->appName;
#endif			
			return ( !CString_compare ( info_->areaName, areaName, sizeof ( info_->areaName ) ) &&
				!CString_compare ( info_->appName, appName, sizeof ( info_->appName ) ) );
        }
        
        
        bool DeviceInstance::LowerThanAppEnv ( environs::DeviceInfoPtr compareTo )
        {
            CVerbVerb ( "LowerThanAppEnv" );
            
            if ( !compareTo )
                return false;

			return ( LowerThanAppEnv ( ( CString_ptr ) c_Addr_of ( compareTo->areaName ), ( CString_ptr ) c_Addr_of ( compareTo->appName ) ) );
        }
        
        bool DeviceInstance::LowerThanAppEnv ( CString_ptr areaName, CString_ptr appName )
        {
            CVerbVerb ( "LowerThanAppEnv" );
            
            if ( areaName == nill )
#ifdef CLI_CPP
				areaName = envObj->GetAreaName ();
#else
				areaName = env->areaName;
#endif
            
            if ( appName == nill )
#ifdef CLI_CPP
				appName = envObj->GetApplicationName ();
#else
				appName = env->appName;
#endif
            
            return ( CString_compare ( info_->areaName, areaName, sizeof ( info_->areaName ) ) < 0 ||
				CString_compare ( info_->appName, appName, sizeof ( info_->appName ) ) < 0);
        }
        
		
        bool DeviceInstance::EqualsID ( environs::DeviceInstancePtr equalTo )
        {
            CVerbVerb ( "EqualsID" );
            
            if ( equalTo == nill )
                return false;
			
			environs::DeviceInfoPtr di = ( ( DeviceInstancePtr ) equalTo )->info_;

			return ( EqualsAppEnv ( ( CString_ptr ) c_Addr_of ( di->areaName ), ( CString_ptr ) c_Addr_of ( di->appName ) )
				&& info_->deviceID == di->deviceID );
        }
        
        bool DeviceInstance::EqualsID ( int deviceID, CString_ptr areaName, CString_ptr appName )
        {
            CVerbVerb ( "EqualsID" );

			return ( EqualsAppEnv ( areaName, appName ) && deviceID == info_->deviceID );
        }
        
        
		bool IsPlatformType ( int src, Platforms_t platform )
        {
            int dst = (int) platform;
            return ((src & dst) == dst);
        }
        
        
		CString_ptr DeviceInstance::DeviceTypeString ( environs::DeviceInfoPtr info )
        {
            CVerbVerb ( "DeviceTypeString" );
            
            if ( IsPlatformType ( info->platform, Platforms::Tablet_Flag ) )
                return "Tablet";
            else if ( IsPlatformType ( info->platform, Platforms::Smartphone_Flag ) )
                return "Smartphone";
            else if ( IsPlatformType ( info->platform, Platforms::MSSUR01 ) )
                return "Surface 1";
            else if ( IsPlatformType ( info->platform, Platforms::SAMSUR40 ) )
                return "Surface 2";
            else if ( IsPlatformType ( info->platform, Platforms::Tabletop_Flag ) )
                return "Tabletop";
            else if ( IsPlatformType ( info->platform, Platforms::Display_Flag ) )
                return "Display";
            return "Unknown";
        }
        
		CString_ptr DeviceInstance::DeviceTypeString ()
        {
            CVerbVerb ( "DeviceTypeString" );

			return DeviceTypeString ( info_ );
        }
        
        
		CString_ptr DeviceInstance::GetBroadcastString ( bool fullText )
        {
            CVerbVerb ( "GetBroadcastString" );
			
            switch ( info_->broadcastFound )
            {
                case DEVICEINFO_DEVICE_BROADCAST:
                    return fullText ? "Nearby" : "B ";
                case DEVICEINFO_DEVICE_MEDIATOR:
                    return fullText ? "Mediator" : "M ";
                case DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR:
                    return fullText ? "Med+Near" : "MB";
            }
            return "U ";
        }
        
        
		STRING_T DeviceInstance::toString ()
        {
            CVerbVerb ( "toString" );
            
#ifdef CLI_CPP
            if ( STRING_empty ( toString_ ) )
            {
                StringBuilder ^ sbl = gcnew StringBuilder ( info_->isConnected ? "* " : "" );
                
                sbl->Append ( GetBroadcastString ( false ) )->Append ( " " )->Append ( info_->deviceID.ToString ( "X" ) )->Append ( " \t" )->Append ( DeviceTypeString () )
                ->Append ( ": " )->Append ( info_->deviceName )->Append ( " [ " )
                ->Append ( info_->appName )->Append ( " / " )->Append ( info_->areaName )->Append ( " ]" );
                
                toString_ = sbl->ToString ();
            }
            
            return toString_;
#else
			LockAcquireVA ( changeEventLock, "toString" );

            if ( STRING_empty ( toString_ ) )
            {
                char buffer [ 384 ];

                buffer [ sizeof ( buffer ) - 1 ] = 0;

                if ( snprintf ( buffer, sizeof ( buffer ) - 1, "* %s %-4X\t%s: %s [ %s / %s ]", GetBroadcastString ( false ), info_->deviceID,  DeviceTypeString (), info_->deviceName,
                               info_->appName, info_->areaName ) <= 0 )
                    *buffer = 0;
                
                toString_ = buffer;
            }

			STRING_T copy = toString_.c_str () + ( info_->isConnected ? 0 : 2 );

			LockReleaseVA ( changeEventLock, "toString" );
            
			return copy;
#endif
        }
        
        
        /**
         * Connect to this device asynchronously.
         *
         * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
         * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
         * 					true: A new connection has been triggered and is in progress
         */
        bool DeviceInstance::Connect ()
        {
            CVerb ( "Connect" );
            
            return Connect ( async );
        }
        
        /**
         * Connect to this device using the given mode.
         *
         * @param asyncl    A value of Environs.Call that determines whether (only this call) is performed synchronous or asynchronous.
         *
         * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
         * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
         * 					true: A new connection has been triggered and is in progress
         */
        bool DeviceInstance::Connect ( environs::Call_t asyncl )
        {
            CVerb ( "Connect" );
            
            if ( asyncl == Call::Wait )
                changeEventPending = true;

			bool success = ( environs::API::DeviceConnectN ( hEnvirons, info_->deviceID, isSameAppArea ? nill : info_->areaName, isSameAppArea ? nill : info_->appName, ( int ) asyncl ) != 0 );
            if ( asyncl == Call::Wait )
            {
                pthread_cond_mutex_lock ( c_Addr_of ( changeEventLock ) );
                

				if ( pthread_cond_wait_time ( c_Addr_of ( changeEvent ), c_Addr_of ( changeEventLock ), 30000 ) ) {
                    CErr ( "Connect: Failed to wait for changeEvent!" );
                }
                
                pthread_cond_mutex_unlock ( c_Addr_of ( changeEventLock ) );
                
                return info_->isConnected;
            }
            
            return success;
        }
        
        /**
         * Disconnect the device with the given id and a particular application environment.
         *
         * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
         */
        bool DeviceInstance::Disconnect ()
        {
            CVerbVerb ( "Disconnect" );
            
            return Disconnect ( async );
        }
        
        /**
         * Disconnect the device using the given mode with the given id and a particular application environment.
         *
         * @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
         *
         * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
         */
        bool DeviceInstance::Disconnect ( environs::Call_t Environs_CALL_ )
        {
            CVerb ( "Disconnect" );
            
            int count = (int) vd_size ( devicePortals );
            
            while ( vd_size ( devicePortals ) > 0 && count > 0 )
            {
                try {
                    
					PortalInstanceESP portal = vd_at ( devicePortals, 0 );
                    if ( portal != nill )
                    {
                        CVerbVerbArg1 ( "Disconnect: Disposing portal ", "count", "X", count );
                        portal->DisposeInstance ( true );
                    }
                }
                catch ( ... ) {
                }
                count--;
            }
            
            CVerbVerb ( "Disconnect: Clearing portal collection" );
            ContainerdClear ( devicePortals );

			return environs::API::DeviceDisconnectN ( hEnvirons, info_->nativeID, (int) Environs_CALL_ ) != 0;
        }
        
        
        /**
         * Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
         *
         * @return DeviceDisplay-object
         */
        DeviceDisplay DeviceInstance::GetDisplayProps ()
        {
            CVerbVerb ( "GetDisplayProps" );
            
            return display;
        }
        

        /**
         * Load the file that is assigned with the fileID received by deviceID into an byte array.
         *
         * @param fileID        The id of the file to load (given in the onData receiver).
         * @param size        An int pointer, that receives the size of the returned buffer.
         * @return byte-array
         */
		UCharArray_ptr DeviceInstance::GetFile ( int fileID, int OBJ_ref size )
        {
            CVerbVerb ( "GetFile" );
            
			return EnvironsAPI ( hEnvirons )->GetFile ( info_->nativeID > 0, info_->nativeID, info_->deviceID,
				isSameAppArea ? nill : info_->areaName, isSameAppArea ? nill : info_->appName, fileID, size );
        }
        

        /**
         * Query the absolute path on the local filesystem that is assigned with the fileID received by deviceID.
         *
         * @param fileID        The id of the file to load (given in the onData receiver).
         *
         * @return absolutePath
         */
		CString_ptr DeviceInstance::GetFilePath ( int fileID )
        {
            CVerbVerb ( "GetFilePath" );

            LockAcquireVA ( changeEventLock, "GetFilePath" );

			if ( STRING_empty ( filePath ) )
            {
                CString_ptr path = nill;
                
                if ( info_->nativeID > 0 )
					path = CCharToString ( environs::API::GetFilePathNativeN ( hEnvirons, info_->nativeID, fileID ) );
                else
					path = CCharToString ( environs::API::GetFilePathN ( hEnvirons, info_->deviceID, isSameAppArea ? nill : info_->areaName, isSameAppArea ? nill : info_->appName, fileID ) );
                
                if ( path != nill )
                    filePath = path;
            }

			LockReleaseVA ( changeEventLock, "GetFilePath" );

            return STRING_get ( filePath );
        }
        
        
        /**
         * Creates a portal instance.
         *
         * @param request   The portal request.
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalCreate ( int request )
        {
            CVerbVerb ( "PortalCreate" );
            
			return PortalCreate ( (request & PORTAL_DIR_MASK), ( environs::PortalType_t ) (request & PORTAL_TYPE_MASK), -1 );
        }
        
        /**
         * Creates a portal instance.
         *
         * @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
         * @param portalType
         * @param slot
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalCreate ( int Environs_PORTAL_DIR, environs::PortalType_t portalType, int slot )
        {
            CVerb ( "PortalCreate" );
            
            if ( !info_->isConnected )
                return nill;
            
			PortalInstanceESP portal = sp_make ( PortalInstanceEP );
            if ( portal == nill  )
                return nill;
            
            sp_set_no_cli ( portal->myself, portal );

#ifdef USE_WP_MYSELF
			if ( !portal->Create ( CPP_CLI ( myself.lock (), GetPlatformObj () ), Environs_PORTAL_DIR, portalType, slot ) )
#else
			if ( !portal->Create ( CPP_CLI ( myself, GetPlatformObj () ), Environs_PORTAL_DIR, portalType, slot ) )
#endif
            {
                portal->DisposeInstance ( true );
                
                C_Only ( portal->device_ = nill );
                return nill;
            }
            
            portal->async = async;
            return portal;
        }
        
        /**
         * Creates a portal instance with a given portalID.
         *
         * @param portalID   The portalID received from native layer.
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalCreateID ( int portalID )
        {
			CVerbVerbArg1 ( "PortalCreateID:", "portalID", "i", portalID );

			if ( !info_->isConnected )
                return nill;

			PortalInstanceESP portal = sp_make ( PortalInstanceEP );
            if ( portal == nill )
                return nill;
            
			sp_set_no_cli ( portal->myself, portal );

#ifdef USE_WP_MYSELF
			if ( !portal->Create ( CPP_CLI ( myself.lock (), GetPlatformObj () ), portalID ) )
#else
			if ( !portal->Create ( CPP_CLI ( myself, GetPlatformObj () ), portalID ) )
#endif
            {
                portal->DisposeInstance ( true );
                
                C_Only ( portal->device_ = nill );
                return nill;
            }

			portal->async = async;
            return portal;
        }
        
        
        /**
         * Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
         *
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalGetWaiting ( bool outgoing )
        {
            CVerbVerb ( "PortalGetWaiting" );
            
			PortalInstanceESP portal;
            
            PortalInstance::KillZombies ();
            
            if ( !LockAcquireA ( devicePortalsLock, "PortalGetWaiting" ) )
                return nill;
            
            for ( size_t i=0; i < vd_size ( devicePortals ); i++ )
            {
                portal = vd_at ( devicePortals, i );
                if ( !portal->disposeOngoing_ && portal->portalID_ < 0 && portal->outgoing_ == outgoing )
                    break;
                portal = nill;
            }
            
            LockReleaseVA ( devicePortalsLock, "PortalGetWaiting" );
            
            return portal;
        }
        
        
        /**
         * Creates a portal instance that requests a portal.
         *
         * @param 	portalType	        Project name of the application environment
         *
         * @return 	PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalRequest ( environs::PortalType_t portalType )
        {
			CVerbVerb ( "PortalRequest" );
            
            return PortalCreate ( PORTAL_DIR_INCOMING, portalType, -1 );
        }

#ifndef CLI_CPP
		environs::PortalInstance OBJ_ptr DeviceInstance::PortalRequestRetained ( PortalType_t portalType )
        {
			CVerbVerb ( "PortalRequestRetained" );
            
            const sp ( PortalInstance ) & res = PortalRequest ( portalType );
            if ( res != nill )
            {
                if ( res->Retain () )
                    return ( environs::PortalInstance * ) res.get ();
            }
            return nill;
        }
#endif
        
        
        /**
         * Creates a portal instance that provides a portal.
         *
         * @param 	portalType	        Project name of the application environment
         *
         * @return 	PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalProvide ( environs::PortalType_t portalType )
        {
			CVerbVerb ( "PortalProvide" );
            
			PortalInstanceESP portal = PortalCreate ( PORTAL_DIR_OUTGOING, portalType, -1 );
            if ( portal != nill )
                portal->status_ = PortalStatus::CreatedAskRequest;
            return portal;
        }

#ifndef CLI_CPP
		environs::PortalInstance OBJ_ptr DeviceInstance::PortalProvideRetained ( PortalType_t portalType )
        {
			CVerbVerb ( "PortalProvideRetained" );
            
			const sp ( PortalInstance ) & res = PortalProvide ( portalType );
            if ( res != nill )
            {
				if ( res->Retain () )
                    return ( environs::PortalInstance * ) res.get ();
            }
            return nill;
        }
#endif
        
        
		PortalInstanceESP DeviceInstance::PortalGet ( bool outgoing )
        {
            CVerbVerb ( "PortalGet" );
            
			PortalInstanceESP portal;
            
            PortalInstance::KillZombies ();
            
            if ( !LockAcquireA ( devicePortalsLock, "PortalGet" ) )
                return nill;
            
            for ( size_t i=0; i < vd_size ( devicePortals ); i++ )
            {
				portal = vd_at ( devicePortals, i );
                if ( !portal->disposeOngoing_ && portal->outgoing_ == outgoing )
                    break;
                portal = nill;
            }
            
            LockReleaseVA ( devicePortalsLock, "PortalGet" );
            
            return portal;
        }
        
        
        /**
         * Query the first PortalInstance that manages an outgoing portal.
         *
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalGetOutgoing ()
        {
			CVerbVerb ( "PortalGetOutgoing" );
            
            return PortalGet ( true );
        }

#ifndef CLI_CPP
		environs::PortalInstance OBJ_ptr DeviceInstance::PortalGetOutgoingRetained ()
        {
			CVerbVerb ( "PortalGetOutgoingRetained" );
            
			const sp ( PortalInstance ) & res = PortalGet ( true );
            if ( res != nill )
            {
				if ( res->Retain () )
                    return ( environs::PortalInstance * ) res.get ();
            }
            return nill;
        }
#endif
        
        
        /**
         * Query the first PortalInstance that manages an incoming portal.
         *
         * @return PortalInstance-object
         */
		PortalInstanceESP DeviceInstance::PortalGetIncoming ()
        {
			CVerbVerb ( "PortalGetIncoming" );
            
            return PortalGet ( false );
        }

#ifndef CLI_CPP
		environs::PortalInstance OBJ_ptr DeviceInstance::PortalGetIncomingRetained ()
        {
			CVerbVerb ( "PortalGetIncomingRetained" );
            
			const sp ( PortalInstance ) & res = PortalGet ( false );
            if ( res != nill )
            {
				if ( res->Retain () )
                    return ( environs::PortalInstance * ) res.get ();
            }
            return nill;
        }
#endif
        
        
        /**
         * Refresh files from fileStorage.
         *
         * @return byte-array
         */
        CLASS ThreadPackFileParse
        {
        public:
			NLayerVecType ( DeviceInstanceEP ) devices;
        };
        
        typedef ThreadPackFileParse OBJ_ptr ThreadPackFileParsePtr;
        
        
		void DeviceInstance::ParseAllFiles ( c_const NLayerVecType ( DeviceInstanceEP ) c_ref devices )
        {
			CVerbVerb ( "ParseAllFiles" );
            
            if ( devices == nill )
                return;

			ThreadPackFileParsePtr threadPack = new__obj ( ThreadPackFileParse );
            
            threadPack->devices = devices;

			pthread_t thread;

			int ret = pthread_create ( c_Addr_of ( thread ), NULL, &FileParseThread, ( pthread_param_t ) threadPack );
			if ( ret != 0 ) {
				CErr ( "ParseAllFiles: Failed to create handler thread." );
				delete__obj ( threadPack );
                return;
			}
            
#ifndef CLI_CPP
            DetachThread ( nill, nill, thread, "DeviceInstance::ParseAllFiles" );
#endif
        }
        
        
        void c_OBJ_ptr DeviceInstance::FileParseThread ( pthread_param_t pack )
        {
			CVerbVerb ( "FileParseThread" );
            
            ThreadPackFileParsePtr thread = ( ThreadPackFileParsePtr ) pack;
            
            for ( size_t i=0; i < vp_size ( thread->devices ); i++ )
            {
				DeviceInstanceESP c_ref device = vp_at ( thread->devices, vctSize i );
                device->ParseStoragePath ( false );
            }
            
            ContainerClear ( thread->devices );
			delete__obj ( thread );
            
            CVerbVerb ( "FileParseThread: Done" );
            return C_Only ( nill );
        }
        
        
        bool DeviceInstance::ParseStoragePath ( bool wait )
        {
			CVerbVerb ( "ParseStoragePath" );
            
            VerifyStoragePath ();

			if ( STRING_empty ( storagePath ) )
                return false;
            
            bool success = false;
            
            int loaded = 0;
            
            if ( !LockAcquireA ( storageLock, "ParseStoragePath" ) )
                return false;
            
            if ( storageLoaded > 1 ) {
                success = true;
            }
            else if ( storageLoaded == 1 ) {
                // Loading is ongoing
                if ( !wait ) {
                    success = true;
                }
            }
            else
                storageLoaded = 1;
            
            LockReleaseVA ( storageLock, "ParseStoragePath" );
            
            if ( success )
                return true;
            
            char * msgContentBin = nill;
            
            if ( !LockAcquireA ( devicePortalsLock, "ParseStoragePath" ) )
                return false;
            
            LockAcquireVA ( storageLock, "ParseStoragePath" );
            
            if ( messagesCache != nill )
                DisposeMessages ( messagesCache );
            if ( filesCache != nill )
                DisposeFiles ( filesCache );
            
            messagesCache   = sp_make ( NLayerVecTypeObj ( EPSPACE MessageInstance ) );
            filesCache      = sp_make ( NLayerMapTypeObj ( int, EPSPACE FileInstance ) );
            
            loaded = storageLoaded;
            
            if ( loaded > 1 ) {
                success = true;
				LockReleaseVA ( storageLock, "ParseStoragePath" );
				goto Finish;
            }

            try
            {
                int msgSize = 0;
                
                // Read descriptor
				STRING_T msgPath = storagePath + "receivedMessages.txt";
                do
                {
					msgContentBin = environs::API::LoadBinaryN ( CString_get_cstr ( msgPath ), &msgSize );
                    if ( msgContentBin == nill )
                        break;
                    
                    char * cur      = msgContentBin;
                    char * msgEnd   = cur + msgSize;
                    char * line     = cur;
                    char * curEnd   = 0;
                    
                    while ( cur < msgEnd )
                    {
                        if ( *cur == '\n' ) {
                            if ( cur > msgContentBin && *(cur - 1) == '\r' ) {
                                curEnd = cur - 1;
                            }
                            else
                                curEnd = cur;
                            
                            cur++;
                            // NewLine found
                            
                            if ( line == msgContentBin )
                            {
                                // Nothing checked before this current position
                                int length = (int) (curEnd - line);
                                
                                if ( !MessageInstance::HasPrefix ( line, length ) ) {
                                    line = cur;
                                }
                            }
                            
                            // Line found
                            int length = (int) (msgEnd - cur);
                            
                            if ( MessageInstance::HasPrefix ( cur, length ) )
                            {
                                length = (int) (curEnd - line);
                                if ( length > 0 ) {
#ifndef NDEBUG
									int cutLength = length;
									if ( cutLength > 586 )
										cutLength = 586;

                                    char c = line [ cutLength ]; line [ cutLength ] = 0;
#ifndef CLI_CPP
                                    CVerbVerbArg2 ( "ParseStoragePath:", "length", "i", length, "line", "s", line );
#endif
                                    line [ cutLength ] = c;
#endif
#ifdef USE_WP_MYSELF
									MessageInstanceESP msg = MessageInstance::Create ( line, length, CPP_CLI ( myself.lock (), GetPlatformObj () ) );
#else
									MessageInstanceESP msg = MessageInstance::Create ( line, length, CPP_CLI ( myself, GetPlatformObj () ) );
#endif
                                    if ( msg != nill ) {
                                        CVerbVerb ( "ParseStoragePath: Adding message" );
										ContainerAppend ( messagesCache, msg );
                                    }
                                }
                                line = cur;
                            }
                        }
                        else ++cur;
                    }
                    
                    if ( line < msgEnd )
                    {
                        int length = (int) (msgEnd - line);
                        if ( length <= 0 )
							break;
#ifndef NDEBUG
						int cutLength = length;
						if ( cutLength > 586 )
							cutLength = 586;

						char c = line [ cutLength ]; line [ cutLength ] = 0;
#ifndef CLI_CPP
						CVerbVerbArg2 ( "ParseStoragePath:", "length", "i", length, "line", "s", line );
#endif
						line [ cutLength ] = c;
#endif                        
                        // Remove trailing \r\n
                        if ( line [length - 2] == '\r' )
                            line [length - 2] = 0;
                        else if ( line [length - 1] == '\n' )
                            line [length - 1] = 0;
#ifdef USE_WP_MYSELF
						MessageInstanceESP msg = MessageInstance::Create ( line, length, CPP_CLI ( myself.lock (), GetPlatformObj () ) );
#else
						MessageInstanceESP msg = MessageInstance::Create ( line, length, CPP_CLI ( myself, GetPlatformObj () ) );
#endif
                        if ( msg != nill ) {
                            CVerbVerb ( "ParseStoragePath: Adding message" );
							ContainerAppend ( messagesCache, msg );
                        }
                    }
                    break;
                }
                while ( 0 );
            }
            catch ( ... ) {
                CErr ( "ParseStoragePath: Exception occured!" );
            }
            
            free_plt ( msgContentBin );
            
            try
            {
				String_ptr fullPath = nill;
#ifndef CLI_CPP
				size_t storagePathLength = STRING_length ( storagePath );

                char * fileName = nill;
                size_t fileNameMax = 128;
#else
				size_t storagePathLength = 0;
				int storagePrefixLength = STRING_length ( storagePath );
#endif
                
                STRUCT dirent OBJ_ptr dirEntry;
				DIR OBJ_ptr dir = opendir ( CString_get_cstr ( storagePath ) );
                
                if ( dir != nill )
                {
                    while ( ( dirEntry = readdir ( dir ) ) != nill )
                    {
#ifndef CLI_CPP
                        if ( dirEntry->d_type == DT_DIR )
                            continue;
#endif
                        
                        String_ptr d_name = dirEntry->d_name;
                        
#ifdef _DIRENT_HAVE_D_NAMLEN
                        int length = dirEntry->d_namlen;
#else
                        size_t length = strlen ( d_name );
#endif
                        if ( length < 4 )
                            continue;
                        
                        if ( !CString_contains ( d_name, ".bin" ) )
                            continue;                        
#ifndef CLI_CPP
                        if ( length >= fileNameMax ) {
                            fileNameMax = length * 2;
                            
                            if ( fullPath != nill ) {
                                free ( fullPath ); fullPath = 0;
                            }
                        }
                        
                        if ( fullPath == nill ) {
                            fullPath = (char *) malloc ( storagePathLength + fileNameMax );
                            if ( !fullPath )
                                break;

							memcpy ( fullPath, STRING_get_cstr ( storagePath ), storagePathLength );
                            
                            fileName = fullPath + storagePathLength;
                        }
                        
                        memcpy ( fileName, d_name, length );
                        fileName [ length ] = 0;
                        
                        fileName [length - 4] = 0;
                        
                        int fileID = atoi ( fileName );
                        
                        fileName [length - 4] = '.';
#else
						fullPath = d_name;
						
						int fileID = 0;

						try
						{
							String ^ idS = d_name->Substring ( storagePrefixLength );
							if ( idS == nill )
								continue;

							idS = idS->Substring ( 0, idS->Length - 4 );
							if ( idS == nill )
								continue;

							fileID = Convert::ToInt32 ( idS );
						}
						catch ( Exception OBJ_ptr ex )
						{
							CErr ( "ParseStoragePath: " + ex ->Message );
							continue;
						}
#endif                       
#ifdef USE_WP_MYSELF
						FileInstanceESP instance = FileInstance::Create ( CPP_CLI ( myself.lock (), GetPlatformObj () ), fileID, fullPath, storagePathLength + length );
#else
						FileInstanceESP instance = FileInstance::Create ( CPP_CLI ( myself, GetPlatformObj () ), fileID, fullPath, storagePathLength + length );
#endif
                        if ( instance ) {
#ifndef CLI_CPP
                            CVerbVerbArg ( "ParseStoragePath: Adding file [%i] [%s] of size [%i]", fileID, instance->descriptor_.c_str (), (int) instance->size_ );
#endif
                            (*filesCache) [fileID] = instance;
                        }
                    }

#ifndef CLI_CPP
                    if ( fullPath != nill )
                        free ( fullPath );
#endif
                    closedir ( dir );
                }
                
                success = true;
                
                //LockAcquireVA ( storageLock, "ParseStoragePath" );
                
                storageLoaded = 2;
                
                //LockReleaseVA ( storageLock, "ParseStoragePath" );
            }
            catch ( ... ) {
                CErr ( "ParseStoragePath: Exception occured!" );
            }

			LockReleaseVA ( storageLock, "ParseStoragePath" );
            
        Finish:
            LockReleaseVA ( devicePortalsLock, "ParseStoragePath" );
            
			CVerbVerb ( "ParseStoragePath: Done" );
            return success;
        }
       
        
#ifdef CLI_CPP
#   define msg  msgSP
#else
#endif
        void DeviceInstance::AddMessage ( CString_ptr message, int length, bool sent, char connection )
        {
            CVerbVerb ( "AddMessage" );
            
            if ( length <= 0 ) {
                CVerb ( "AddMessage: Invalid size." );
                return;
            }
            
            if ( connection == 'u' )
            {
                if ( info_->nativeID > 0 )
                    connection = 'c';
                else if ( info_->broadcastFound != DEVICEINFO_DEVICE_MEDIATOR )
                    connection = 'd';
                else
                    connection = 'm';
            }
            
#ifndef NDEBUG
            if ( !sent ) {
                CVerbsArg2 ( 3, "AddMessage: Delivering Incoming ", "message", "s", message, "device", "s", toString ().c_str () );
                
                //printf ( "Message: %s", message );
                CVerbsArg1 ( 3, "AddMessage: ", "con", "c", connection );
                
                /*char tmp [ 256000 ];
                
                ConvertToHexSpaceBuffer ( (char *) message, length, tmp, false );
                printf ( "AddMessage: [%s]", tmp );
                */
            }
            else {
                CVerbsArg2 ( 5, "AddMessage: Delivering Outgoing ", "message", "s", message, "device", "s", toString ().c_str () );
                
                CVerbsArg1 ( 5, "AddMessage: ", "con", "c", connection );
            }
#endif
			MessageInstanceESP msgSP = sp_make ( EPSPACE MessageInstance );
            if ( msgSP == nill ) {
                CVerb ( "AddMessage: Failed to allocate object." );
                return;
            }
            
            C_Only ( MessageInstance * msg = msgSP.get (); )

#ifdef USE_WP_MYSELF
			msg->device_        = CPP_CLI ( myself.lock (), GetPlatformObj () );
#else
			msg->device_        = CPP_CLI ( myself, GetPlatformObj () );
#endif
            if ( msg->device_ == nill ) {
                CVerb ( "AddMessage: Related DeviceInstance is disposed." );
                return;
            }
            
#ifdef CLI_CPP
            if ( message->Length < length )
                length  = message->Length;
            
            msg->text_  = message->Substring ( 0, length );
            msg->length_ = length;
#else
            msg->text_  = (char *) malloc ( length + 1 );
            if ( !msg->text_ ) {
                CVerbsArg1 ( 5, "AddMessage: Failed to allocate bytes of", "size", "i", length );
                return;
            }
            
            memcpy ( msg->text_, message, length );
            msg->text_ [ length ] = 0;
            
            msg->length_ = length;
#endif
            msg->sent_          = sent;
            msg->length_        = length;
            msg->connection_    = connection;
            msg->created_       = GetUnixEpoch ();

            C_Only ( msg->myself = msgSP; )

			if ( messagesEnqueue TRACE_MESSAGE_INSTANCE ( || true ) )
			{
				if ( LockAcquireA ( storageLock, "AddMessage" ) )
				{
					if ( !disposed_ )
					{
						stdQueue_push ( messages, msgSP );

						// Signal a receive event
						if ( pthread_cond_signal ( c_Addr_of ( messagesEvent ) ) ) {
							CVerbVerb ( "AddMessage: Failed to signal event!" );
						}
					}

					LockReleaseVA ( storageLock, "AddMessage" );
				}

                TRACE_MESSAGE_INSTANCE_NOT ( return );
			}
			
			if ( !disposed_ ) {
				NotifyObserversForMessage ( msgSP, MessageInfoFlag::Created, true );
				return;
			}
            
            msg->DisposeInstance ();
        }
        
        
#ifdef CLI_CPP
#   define fileInst  fileInstSP
#else
#endif
        void DeviceInstance::AddFile ( int type, int fileID, CString_ptr fileDescriptor, CString_ptr path, int  bytesToSend, bool sent )
        {
            CVerbVerb ( "AddFile" );

			FileInstanceESP fileInstSP = sp_make ( EPSPACE FileInstance );
            if ( fileInstSP == nill )
                return;
            
            C_Only ( FileInstance * fileInst = fileInstSP.get (); )

#ifdef USE_WP_MYSELF
			fileInst->device_ = CPP_CLI ( myself.lock (), GetPlatformObj () );
#else
			fileInst->device_ = CPP_CLI ( myself, GetPlatformObj () );
#endif
            
            if ( fileInst->device_ == nill )
            {
                CVerb ( "AddFile: Related DeviceInstance is disposed." );
                return;
            }
            
            fileInst->fileID_ = fileID;
            
            if ( fileDescriptor != nill )
                fileInst->descriptor_ = fileDescriptor;
            
            if ( path != nill ) {
                fileInst->path_ = path;                
            }
			else {
				fileInst->path_ = GetFilePath ( fileID );
			}

            if ( bytesToSend ) {
                fileInst->size_ = bytesToSend;
            }
			else if ( fileInst->size_ <= 0 ) {
				if ( !STRING_empty ( fileInst->path_ ) )
					fileInst->size_ = ( long ) GetSizeOfFile ( STRING_get ( fileInst->path_ ) );
			}
            
            fileInst->type_     = type;
            fileInst->sent_     = sent;
            fileInst->created_  = GetUnixEpoch ();                       
			
			FileInstanceESP fileInstOld;

			if ( LockAcquireA ( storageLock, "AddFile" ) )
			{
				if ( !disposed_ )
				{
					C_Only ( fileInst->myself = fileInstSP );

					ContainerIfContains ( files, fileID )
						fileInstOld = ( *files ) [ fileID ];

					( *files ) [ fileID ] = fileInstSP;

					// Signal a receive event
					if ( filesLast >= 0 && pthread_cond_signal ( c_Addr_of ( filesEvent ) ) ) {
						CVerbVerb ( "AddFile: Failed to signal event!" );
					}
				}

				LockReleaseVA ( storageLock, "AddFile" );
			}

			if ( fileInstOld ) {
				if ( fileInstOld->platformRef )
					fileInstOld->PlatformDispose ();
				else
					fileInstOld->DisposeInstance ();
			}

			if ( filesLast < 0 ) {
				NotifyObserversForData ( fileInstSP, FileInfoFlag::Created, true );
			}

			if ( !disposed_ )
				return;
            
            fileInstSP->DisposeInstance ();
        }
        
        
        /**
         * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
         *
         * @param fileID        A user-customizable id that identifies the file to be send.
         * @param fileDescriptor (e.g. filename)
         * @param filePath      The path to the file to be send.
         * @return success
         */
        bool DeviceInstance::SendFile ( int fileID, CString_ptr fileDescriptor, CString_ptr path )
        {
            CVerb ( "SendFile" );
            
            if ( path == nill || disposed_ ) return false;
            
            AddFile ( 0, fileID, ( CString_ptr ) fileDescriptor, ( CString_ptr ) path, 0, true );
            
            return environs::API::SendFileN ( hEnvirons, info_->nativeID, ( int ) async, fileID, fileDescriptor, path ) != 0;
        }
        
        
        /**
         * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
         *
         * @param fileID        A user-customizable id that identifies the file to be send.
         * @param fileDescriptor (e.g. filename)
         * @param buffer        A buffer to be send.
         * @param bytesToSend number of bytes in the buffer to send
         * @return success
         */
        bool DeviceInstance::SendBuffer ( int fileID, CString_ptr fileDescriptor, UCharArray_ptr buffer, int bytesToSend )
        {
            CVerb ( "SendBuffer" );
            
            if ( buffer == nill || disposed_ ) return false;
            
            AddFile ( 0, fileID, fileDescriptor, nill, 0, true );

			PIN_PTR ( pBuffer, unsigned char, buffer );

            return environs::API::SendBufferN ( hEnvirons, info_->nativeID, ( int ) async, fileID, fileDescriptor, pBuffer, bytesToSend ) != 0;
        }
        
        
        /**
         * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
         * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
         * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
         *
         * @return MessageInstance
         */
        FileInstanceESP DeviceInstance::ReceiveBuffer ()
        {
            if ( filesLast < 0 || disposed_ || !info_->isConnected )
                return nill;
            
			FileInstanceESP item;
            
            if ( !LockAcquireA ( storageLock, "ReceiveBuffer" ) )
                return nill;
        Retry:
            if ( filesLast < (int) vp_size ( files ) )
            {
                item = vp_at ( files, filesLast );
                
                filesLast++;
            }
            else {
#if ( defined(_WIN32) && !defined(CLI_CPP) )
				pthread_cond_wait ( c_Addr_of ( filesEvent ), c_Addr_of ( storageLock ) );
#elif defined(CLI_CPP)
				LockReleaseVA ( storageLock, 0 );

				filesEvent->WaitOne ();

				LockAcquireVA ( storageLock, 0 );
#else
                if ( pthread_cond_wait ( c_Addr_of ( filesEvent ), c_Addr_of ( storageLock ) ) ) {
                    CErr ( "ReceiveBuffer: Failed to wait for event!" );
                }
                else {
#endif
                    if ( !disposed_ )
                        goto Retry;
#if ( !defined(_WIN32) )
				}
#endif
            }
            
            LockReleaseVA ( storageLock, "ReceiveBuffer" );
            
            return item;
        }
        
        
#ifndef CLI_CPP
        environs::FileInstance * DeviceInstance::ReceiveBufferRetained ()
        {
            if ( disposed_ )
                return nill;
            
            sp ( FileInstance) fileInst = DeviceInstance::ReceiveBuffer ();
            if ( fileInst != nill ) {
                if ( fileInst->Retain () )
                    return fileInst.get ();
            }
            return nill;
        }
#endif
        
        
        /**
         * Send a string message to a device through one of the following ways.&nbsp;
         * If a connection with the destination device has been established, then use that connection.
         * If the destination device is not already connected, then distinguish the following cases:
         * (1) If the destination is within the same network, then try establishing a direct connection.
         * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
         * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
         * 		a STUNT connection with the latest connection details that are available.
         *
         * On successful transmission, Environs returns true if the devices already had an active connection,
         * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
         * a registered EnvironsObserver instance.
         *
		 * @param waitType		(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
         * @param message       A message to send.
         * @param length		Length of the message to send.
         * @return success
         */
        bool DeviceInstance::SendMessage ( environs::Call_t waitType, CString_ptr message, int length )
        {
            CVerb ( "SendMessage" );
            
            if ( message == nill || disposed_ ) return false;
            
            AddMessage ( message, length, true, 'u' );
            
#ifdef CLI_CPP
			IntPtr p = ToPlatformType ( message );
#else
			void * p = ( void * ) message;
#endif
			int success = environs::API::SendMessageN ( hEnvirons, info_->deviceID, 
				isSameAppArea ? nill : info_->areaName, 
				isSameAppArea ? nill : info_->appName, 
				( int ) waitType, ToPlatAddr ( p ), length );

			DisposePlatPointer ( p );

			return (success != 0);
        }
        
        
        /**
         * Send a string message to a device through one of the following ways.&nbsp;
         * If a connection with the destination device has been established, then use that connection.
         * If the destination device is not already connected, then distinguish the following cases:
         * (1) If the destination is within the same network, then try establishing a direct connection.
         * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
         * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
         * 		a STUNT connection with the latest connection details that are available.
         *
         * On successful transmission, Environs returns true if the devices already had an active connection,
         * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
         * a registered EnvironsObserver instance.
         *
         * @param message       A message to be send.
         * @return success
         */    
        bool DeviceInstance::SendMessage ( CString_ptr message )
        {
            CVerb ( "SendMessage" );
            
            if ( message == nill ) return false;
            
            int length = (int) CString_length ( message );
            
            return SendMessage ( async, message, length );
        }
        
        
        /**
         * Send a string message to a device through one of the following ways.&nbsp;
         * If a connection with the destination device has been established, then use that connection.
         * If the destination device is not already connected, then distinguish the following cases:
         * (1) If the destination is within the same network, then try establishing a direct connection.
         * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
         * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
         * 		a STUNT connection with the latest connection details that are available.
         *
         * On successful transmission, Environs returns true if the devices already had an active connection,
         * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
         * a registered EnvironsObserver instance.
         *
		 * @param waitType		(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
         * @param message       A message to be send.
         * @return success
         */    
        bool DeviceInstance::SendMessage ( environs::Call_t waitType, CString_ptr message )
        {
            CVerb ( "SendMessage" );
            
            if ( message == nill ) return false;
            
            int length = (int) CString_length ( message );
            
            return SendMessage ( waitType, message, length );
        }
        
        
        /**
         * Receives a message send using SendMessage by the DeviceInstance.
         * This call blocks until a new message has been received or until the DeviceInstance gets disposed.
         * Messages that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
         *
         * @return MessageInstance
         */
        MessageInstanceESP DeviceInstance::Receive ()
        {
            if ( !messagesEnqueue || disposed_  )
                return nill;
            
            MessageInstanceESP item;
            
            if ( !LockAcquireA ( storageLock, "Receive" ) )
                return nill;
        Retry:
            if ( !stdQueue_empty ( messages ) )
            {
                item = stdQueue_front ( messages );
                
                stdQueue_pop ( messages );
            }
            else 
			{
#if ( defined(_WIN32) && !defined(CLI_CPP) )
				pthread_cond_wait ( c_Addr_of ( messagesEvent ), c_Addr_of ( storageLock ) );
#elif defined(CLI_CPP)
				LockReleaseVA ( storageLock, 0 );

				messagesEvent->WaitOne ();

				LockAcquireVA ( storageLock, 0 );
#else
				if ( pthread_cond_wait ( c_Addr_of ( messagesEvent ), c_Addr_of ( storageLock ) ) ) {
					CErr ( "Receive: Failed to wait for event!" );
				}
				else {
#endif
					if ( !disposed_ )
						goto Retry;
#if ( !defined(_WIN32) )
				}
#endif
            }
            
            LockReleaseVA ( storageLock, "Receive" );
            
            return item;
        }
        
        
#ifndef CLI_CPP
        environs::MessageInstance * DeviceInstance::ReceiveRetained ()
        {
            if ( disposed_ )
                return nill;
            
            const sp ( MessageInstance) &msgInst = DeviceInstance::Receive ();
            if ( msgInst != nill ) {
                if ( msgInst->Retain () )
                    return msgInst.get ();
            }
            return nill;
        }
#endif


		/**
		* Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
		*
		* @param buffer        A buffer to be send.
		* @param offset        A user-customizable id that identifies the file to be send.
		* @param bytesToSend number of bytes in the buffer to send
		* @return success
		*/
		bool DeviceInstance::SendDataUdp ( UCharArray_ptr buffer, int offset, int bytesToSend )
		{
			CVerb ( "SendDataUdp" );

			return SendDataUdp ( async, buffer, offset, bytesToSend );
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
        bool DeviceInstance::SendDataUdp ( environs::Call_t asyncSend, UCharArray_ptr buffer, int offset, int bytesToSend )
        {
            CVerb ( "SendDataUdp" );
            
            if ( buffer == nill || bytesToSend <= 0 || disposed_ || !info_->isConnected ) return false;
            
            PIN_PTR ( pBuffer, unsigned char, buffer );
            
            return environs::API::SendDataUdpN ( hEnvirons, info_->nativeID, ( int ) asyncSend, pBuffer, offset, bytesToSend ) != 0;
        }
        
        
        /**
         * Receives a data buffer send using SendDataUdp by the DeviceInstance.
         * This call blocks until new data has been received or until the DeviceInstance gets disposed.
         *
         * @return byte buffer
         */
        UCharArray_ptr DeviceInstance::ReceiveData ()
        {
			return ReceiveDataRawPointer ( 0 );
        }


		/**
		* Receives a data buffer send using SendDataUdp by the DeviceInstance.
		* This call blocks until new data has been received or until the DeviceInstance gets disposed.
		*
		* @return byte buffer
		*/
		UCharArray_ptr DeviceInstance::ReceiveDataRawPointer ( int * size )
		{
            if ( disposed_ || !info_->isConnected )
                return nill;
            
			UdpDataPack OBJ_ptr item = nill;

			if ( !LockAcquireA ( storageLock, "ReceiveDataRawPointer" ) )
                return nill;
            
            if ( udpDataDisposeFront ) {
                if ( !stdQueue_empty ( udpData ) )
                {
                    item = stdQueue_front ( udpData );
                    
                    stdQueue_pop ( udpData );
                    
                    if ( item != nill ) {
                        delete__obj ( item );
                        item = nill;
                    }
                }
                udpDataDisposeFront = false;
            }
            
		Retry:
			if ( !stdQueue_empty ( udpData ) )
			{
				item = stdQueue_front ( udpData );
                
                udpDataDisposeFront = true;
			}
			else {
#if ( defined(_WIN32) && !defined(CLI_CPP) )
				pthread_cond_wait ( c_Addr_of ( udpDataEvent ), c_Addr_of ( storageLock ) );
#elif defined(CLI_CPP)
				LockReleaseVA ( storageLock, 0 );

				udpDataEvent->WaitOne ();

				LockAcquireVA ( storageLock, 0 );
#else
				if ( pthread_cond_wait ( c_Addr_of ( udpDataEvent ), c_Addr_of ( storageLock ) ) ) {
					CErr ( "ReceiveDataRawPointer: Failed to wait for event!" );
				}
				else {
#endif
					if ( !disposed_ )
						goto Retry;
#if ( !defined(_WIN32) )
				}
#endif
			}

			LockReleaseVA ( storageLock, "ReceiveBuffer" );

            if ( item != nill ) {
                if ( size )
                    *size = item->size;
                
                return item->data;
            }
			return nill;
		}
        
        
        /**
         * Clear cached MessageInstance and FileInstance objects for this DeviceInstance.
         *
         */
        void DeviceInstance::DisposeStorageCache ()
        {
            CVerb ( "DisposeStorageCache" );
            
            if ( !LockAcquireA ( storageLock, "DisposeStorageCache" ) )
                return;
            try
            {
                DisposeMessages ();
                
                DisposeFiles ();
                
                storageLoaded = 0;
            }
            catch ( ... )
            {
            }
            
            LockReleaseVA ( storageLock, "DisposeStorageCache" );
        }
        
        
        /**
         * Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
         *
         */
        void DeviceInstance::ClearMessages ()
        {
            CVerb ( "ClearMessages" );
            
            if ( async == Call::NoWait )
            {
#ifndef CLI_CPP
                LockAcquireA ( storageLock, "ClearMessages" );

                clearMessagesSP = myself.lock ();

                LockReleaseA ( storageLock, "ClearMessages" );

                if ( !clearMessagesSP )
                    return;
#endif
                pthread_t thread;
                
                int ret = pthread_create ( c_Addr_of ( thread ), NULL, &ClearMessagesThread, ( pthread_param_t ) this );
                if ( ret != 0 ) {
#ifndef CLI_CPP
                    LockAcquireA ( storageLock, "ClearMessages" );

                    clearMessagesSP.reset ();

                    LockReleaseA ( storageLock, "ClearMessages" );
#endif
                    CErr ( "ClearMessages: Failed to create handler thread." );
                    return;
                }
#ifndef CLI_CPP
                DetachThread ( nill, nill, thread, "DeviceInstance::ClearMessages" );
#endif
                return;
            }
            
            ClearMessagesDo ();
        }
        
        void c_OBJ_ptr DeviceInstance::ClearMessagesThread ( pthread_param_t arg )
        {
            CVerb ( "ClearMessagesThread" );

			DeviceInstancePtr argDev = ( DeviceInstancePtr ) arg;
			if ( argDev != nill )
            {
#ifndef CLI_CPP
                DeviceInstanceESP device = argDev->clearMessagesSP;

                LockAcquireA ( argDev->storageLock, "ClearMessagesThread" );

                argDev->clearMessagesSP.reset ();

                LockReleaseA ( argDev->storageLock, "ClearMessagesThread" );
                
                if ( device != nill )
#endif
					argDev->ClearMessagesDo ();
			}
            
            return C_Only ( nill );
        }
        

        void DeviceInstance::ClearMessagesDo ()
        {
            CVerb ( "ClearMessagesDo" );
            
            if ( disposed_ ) return;
            
            VerifyStoragePath ();
            
            if ( !LockAcquireA ( storageLock, "ClearMessagesDo" ) )
                return;
            try
            {
                DisposeMessages ();
                
                STRING_T msgPathString = storagePath + "receivedMessages.txt";
#ifdef CLI_CPP
				PlatformIntPtr platformMsgPath = StringToPlatformIntPtr ( msgPathString );
#else
				const STRING_T &platformMsgPath = msgPathString;
#endif
				const char * msgPath = ( const char * ) CString_get_cstr ( ToPlatAddr ( platformMsgPath ) );

				if ( _access ( msgPath, F_OK ) != -1 ) {
					_unlink ( msgPath );
                }

				DisposePlatPointer ( platformMsgPath );
            }
            catch ( ... )
            {
            }
            
            LockReleaseVA ( storageLock, "ClearMessagesDo" );
        }
        
        
        /**
         * Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
         *
         */
        void DeviceInstance::ClearStorage ()
        {
            CVerb ( "ClearStorage" );
            
            if ( async == Call::NoWait )
            {
#ifndef CLI_CPP
                LockAcquireA ( storageLock, "ClearStorage" );

                clearStorageSP = myself.lock ();

                LockReleaseA ( storageLock, "ClearStorage" );

                if ( !clearStorageSP )
                    return;
#endif
                pthread_t thread;

                int ret = pthread_create ( c_Addr_of ( thread ), NULL, &ClearStorageThread, ( pthread_param_t ) this );
                if ( ret != 0 ) {
                    CErr ( "ClearStorage: Failed to create handler thread." );
#ifndef CLI_CPP
                    LockAcquireA ( storageLock, "ClearStorage" );

                    clearStorageSP.reset ();

                    LockReleaseA ( storageLock, "ClearStorage" );
#endif
                    return;
                }                
#ifndef CLI_CPP
                DetachThread ( nill, nill, thread, "DeviceInstance::ClearStorage" );
#endif
                return;
            }
            
            ClearStorageDo ();
        }
        
        
        void c_OBJ_ptr DeviceInstance::ClearStorageThread ( pthread_param_t arg )
        {
            CVerb ( "ClearStorageThread" );

			DeviceInstancePtr argDev = ( DeviceInstancePtr ) arg;
			if ( argDev != nill )
			{
#ifndef CLI_CPP
                DeviceInstanceESP device = argDev->clearStorageSP;

                LockAcquireA ( argDev->storageLock, "ClearStorageThread" );

                argDev->clearStorageSP.reset ();

                LockReleaseA ( argDev->storageLock, "ClearStorageThread" );

				if ( device != nill )
#endif
					argDev->ClearStorageDo ();
			}
            
            return C_Only ( nill );
        }
        
        
        void DeviceInstance::ClearStorageDo ()
        {
            CVerb ( "ClearStorageDo" );
            
            if ( disposed_ ) return;
            
            VerifyStoragePath ();
            
            if ( !LockAcquireA ( storageLock, "ClearStorageDo" ) )
                return;
            try
            {
                DisposeFiles ();
                
                try
                {
#ifndef CLI_CPP
					char * fullPath = 0;
					size_t storagePathLength = STRING_length ( storagePath );

                    char * fileName = 0;
                    size_t fileNameMax = 128;
#endif                    
                    STRUCT dirent OBJ_ptr dirEntry;
					DIR OBJ_ptr dir = opendir ( CString_get_cstr ( storagePath ) );
                    
                    if ( dir != nill )
                    {
                        while ( (dirEntry = readdir ( dir )) != nill )
                        {
                            if ( dirEntry->d_type == DT_DIR )
                                continue;
                            
							String_ptr d_name = dirEntry->d_name;
                            
                            if ( CString_contains ( d_name, "receivedMessages.txt" ) )
                                continue;                   
#ifndef CLI_CPP
#	ifdef _DIRENT_HAVE_D_NAMLEN
							size_t length = dirEntry->d_namlen;
#	else
							size_t length = strlen ( d_name );
#	endif    
                            if ( length >= fileNameMax ) {
                                fileNameMax = length * 2;
                                
                                if ( fullPath ) {
                                    free ( fullPath ); fullPath = 0;
                                }
                            }
                            
                            if ( !fullPath ) {
                                fullPath = (char *) malloc ( storagePathLength + fileNameMax );
                                if ( !fullPath )
                                    break;
                                
                                memcpy ( fullPath, STRING_get_cstr ( storagePath ), storagePathLength );
                                
                                fileName = fullPath + storagePathLength;
                            }
                            
                            memcpy ( fileName, d_name, length );
                            fileName [ length ] = 0;

							unlink ( fullPath );
#else
							try
							{
								File::Delete ( d_name );
							}
							catch ( Exception OBJ_ptr ex )
							{
								CErr ( "ClearStorageDo: " + ex->Message );
								continue;
							}
#endif                            
                        }
#ifndef CLI_CPP
                        if ( fullPath )
                            free ( fullPath );
#endif
                        closedir ( dir );
                    }
                }
                catch ( ... )
                {
                    CErr ( "ClearStorageDo: Exception occured!" );
                }
            }
            catch ( ... )
            {
            }
            
            LockReleaseVA ( storageLock, "ClearStorageDo" );
        }
        
        
        /**
         * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
         *
         * @return Collection with objects of type FileInstance with the fileID as the key.
         */
		NLayerMapType ( int, EPSPACE FileInstance ) DeviceInstance::GetFiles ()
        {
            CVerb ( "GetAllFiles" );
            
            return files;
        }
        
        
        /**
         * Get a dictionary with all files that this device has received (and sent) from the storage.
         *
         * @return Collection with objects of type FileInstance with the fileID as the key.
         */
        NLayerMapType ( int, EPSPACE FileInstance ) DeviceInstance::GetFilesInStorage ()
        {
            CVerb ( "GetFilesInStorage" );
            
            if ( storageLoaded < 2 )
                ParseStoragePath ( true );
            return filesCache;
        }

#ifndef CLI_CPP
		environs::ArrayList * DeviceInstance::GetFilesRetained ()
        {
            CVerb ( "GetFilesRetained" );

            if ( !LockAcquireA ( storageLock, "GetFilesRetained" ) )
                return nill;
            
            //c_const smsp ( int, FileInstance ) c_ref filesSP = GetFiles ();

            environs::ArrayList * list =
            
#ifdef USE_WP_MYSELF
                ArrayList::CreateWithFilesRetained ( files, myself.lock () );
#else
                ArrayList::CreateWithFilesRetained ( files, myself );
#endif
            LockReleaseVA ( storageLock, "GetFilesRetained" );

            return list;
        }


        environs::ArrayList * DeviceInstance::GetFilesInStorageRetained ()
        {
            CVerb ( "GetFilesInStorageRetained" );
            
            c_const smsp ( int, FileInstance ) c_ref filesSP = GetFilesInStorage ();

            if ( !LockAcquireA ( storageLock, "GetFilesInStorageRetained" ) )
                return nill;

            environs::ArrayList * list =

#ifdef USE_WP_MYSELF
                ArrayList::CreateWithFilesRetained ( filesSP, myself.lock () );
#else
                ArrayList::CreateWithFilesRetained ( filesSP, myself );
#endif
            LockReleaseVA ( storageLock, "GetFilesInStorageRetained" );

            return list;
        }
#endif
        
        /**
         * Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
         * Note: These messages would otherwise been delivered by calls to Receive, that is by calling GetMessages,
         *       the messages will not be delivered to Receive afterwards.
         *
         * @return Collection with objects of type MessageInstance
         */
		NLayerVecType ( EPSPACE MessageInstance ) DeviceInstance::GetMessages ()
        {
            CVerb ( "GetMessages" );
            
            NLayerVecType ( EPSPACE MessageInstance ) msgs = sp_make ( NLayerVecTypeObj ( EPSPACE MessageInstance ) );
            
            LockAcquireVA ( storageLock, "StorageLock" );
            
            while ( !stdQueue_empty ( messages ) )
            {
                MessageInstanceESP item = stdQueue_front ( messages );
                
                stdQueue_pop ( messages );
                
                ContainerAppend ( msgs, item );
            }
            
            LockReleaseVA ( storageLock, "StorageLock" );
            
            return msgs;
        }
        
        /**
         * Get a list with all messages that this device has received (and sent) from the storage.
         *
         * @return Collection with objects of type MessageInstance
         */
        NLayerVecType ( EPSPACE MessageInstance ) DeviceInstance::GetMessagesInStorage ()
        {
            CVerb ( "GetMessagesInStorage" );
            
            if ( storageLoaded < 2 )
                ParseStoragePath ( true );
            return messagesCache;
        }
        
        
#ifndef CLI_CPP
        environs::ArrayList * DeviceInstance::GetMessagesRetained ()
        {
            CVerb ( "GetMessagesRetained" );
            
            LockAcquireVA ( storageLock, "StorageLock" );

			environs::ArrayList * list =

#ifdef USE_WP_MYSELF
			ArrayList::CreateWithMessagesQueueRetained ( messages, myself.lock () );
#else
			ArrayList::CreateWithMessagesQueueRetained ( messages, myself );
#endif            
            LockReleaseVA ( storageLock, "StorageLock" );
            
            return list;
        }
        
        environs::ArrayList * DeviceInstance::GetMessagesInStorageRetained ()
        {
            CVerb ( "GetMessagesInStorageRetained" );
            
            c_const svsp ( MessageInstance ) c_ref msgs = GetMessagesInStorage ();

			LockAcquireVA ( storageLock, "StorageLock" );

			environs::ArrayList * list =
#ifdef USE_WP_MYSELF
			ArrayList::CreateWithMessagesRetained ( msgs, myself.lock () );
#else
			ArrayList::CreateWithMessagesRetained ( msgs, myself );
#endif
			LockReleaseVA ( storageLock, "StorageLock" );

			return list;
        }
#endif
        
        /**
         * Acquire or release lock on file and message instances.
         * Client code MUST balance successful locks. Otherwise deadlocks may happen.
         *
         * @param lock true = acquire, false = release.
         *
         * @return success
         */
        bool DeviceInstance::StorageLock ( bool lock )
        {
			CVerbVerbArg1 ( "StorageLock", "lock", "i", lock );
            
            if ( lock )
                return LockAcquireA ( storageLock, "StorageLock" );
            
            return LockReleaseA ( storageLock, "StorageLock" );
        }
        
        
        void DeviceInstance::VerifyStoragePath ()
        {
			CVerbVerb ( "VerifyStoragePath" );
            
            if ( !LockAcquireA ( storageLock, "VerifyStoragePath" ) )
                return;

			if ( STRING_empty ( storagePath ) ) {
                char * path = environs::API::GetFilePathForStorageN ( hEnvirons, info_->deviceID, isSameAppArea ? nill : info_->areaName, isSameAppArea ? nill : info_->appName );
                if ( path != 0 ) {
                    storagePath = CCharToString ( path );
					free_plt ( path );
                }
            }
            
            LockReleaseVA ( storageLock, "VerifyStoragePath" );
        }
        
        
        /**
         * Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
         *
         * @return absolutePath
         */
        CString_ptr DeviceInstance::GetStoragePath ()
        {
            CVerbVerb ( "GetStoragePath" );
            
            VerifyStoragePath ();

			return STRING_get ( storagePath );
        }
        
        
        void DeviceInstance::NotifySensorObservers ( environs::SensorFrame OBJ_ptr sensorFrame )
        {
            CVerbVerb ( "NotifySensorObservers" );
            
            if ( sensorFrame == nill ) return;
            
#ifdef CLI_CPP
			observersForSensorData ( sensorFrame );
#else
            vct ( lib::IISensorObserver * ) obss;
            
            LockAcquireVA ( changeEventLock, "NotifySensorObservers" );
            
            size_t size = observersForSensorData.size ();
            
            for ( size_t i = 0; i < size; ++i )
            {
                lib::IISensorObserver OBJ_ptr obs = observersForSensorData.at ( vctSize i );
                if ( obs != nill ) {
                    obss.push_back ( obs );
                }
            }
            
            LockReleaseVA ( changeEventLock, "NotifySensorObservers" );
            
            size = obss.size ();
            if ( size <= 0 )
                return;
            
            for ( size_t i=0; i < size; i++ )
            {
                lib::IISensorObserver OBJ_ptr obs = obss.at ( vctSize i );
                
                try {
                    obs->OnSensorData ( sensorFrame );
                }
                catch ( ... ) {
                    CErr ( "NotifySensorObservers: Exception!" );
                }
            }
#endif
        }
        
        
        void DeviceInstance::NotifyUdpData ( UdpDataContext OBJ_ptr ctx )
        {
            CVerbVerb ( "NotifyUdpData" );
            
            if ( !LockAcquireA ( storageLock, "NotifyUdpData" ) )
                return;
            
            if ( vd_size ( udpData ) < DEVICE_INSTANCT_MAX_CACHE_SIZE )
            {
                UdpDataPack OBJ_ptr item = new__obj2 ( UdpDataPack, ctx->dataPtr, ctx->size );
                
                if ( item != nill )
                {
                    stdQueue_push ( udpData, item );
                    
                    if ( pthread_cond_signal ( c_Addr_of ( udpDataEvent ) ) ) {
                        CVerbVerb ( "NotifyUdpData: Failed to signal event!" );
                    }
                }
            }
            
            LockReleaseVA ( storageLock, "ReceiveBuffer" );
        }
	}

}









