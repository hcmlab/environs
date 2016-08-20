/**
 * Device Instance Objects
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H

#ifdef __cplusplus

#include "Environs.Observer.CLI.h"
#include "Environs.Platforms.h"
#include "Interop/Smart.Pointer.h"
#include "Environs.Msg.Types.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Device.Info.h"
#include "Device.Display.Decl.h"

#ifdef CLI_CPP
#else

#	include "Interfaces/IDevice.Instance.h"
#	include "Queue.List.h"
#	include <queue>

#	define USE_QUEUE_VECTOR_T_CLASS

#   ifdef USE_QUEUE_VECTOR_T_CLASS
#       include "Queue.Vector.h"
#   endif

#ifndef NDEBUG

#endif

#endif

#include <string>

/* Namespace: environs -> */
namespace environs
{
    class Instance;
	

	namespace lib
	{
        CLASS Environs;
        
#ifndef CLI_CPP
        class PortalInstance;
        class FileInstance;
        class MessageInstance;
#endif

		CLASS DeviceNotifierContext C_Only ( : public IQueueItem )
		{
        public:
#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT
            DeviceNotifierContext ();
            ~DeviceNotifierContext ();
#endif
			int                 type;
			int                 flags;
			DeviceInstanceSP	device;
			MessageInstanceESP  message;
			FileInstanceESP     fileData;

#ifdef CLI_CPP
			String ^			propertyName;
#endif
		};        
        
        
		PUBLIC_CLASS UdpDataPack
		{
		public:
			UdpDataPack ( Addr_ptr ptr, int sizeInBytes );
			~UdpDataPack ();

			UCharArray_ptr	data;
			int				size;
        };
        
        
        STRUCT UdpDataContext
        {
        public:
            OBJIDType                                   objID;
            int                                         size;
            environs::SensorFrame           OBJ_ptr     sensorFrame;
            Addr_ptr                                    dataPtr;
        };
        

		PUBLIC_CLASS DeviceInstance CLI_ABSTRACT DERIVE_c_only ( environs::DeviceInstance ) DERIVE_DISPOSABLE 
		{
            MAKE_FRIEND_CLASS ( Environs );
            MAKE_FRIEND_CLASS ( DeviceList );
            MAKE_FRIEND_CLASS ( PortalInstance );
            MAKE_FRIEND_CLASS ( MessageInstance );
            MAKE_FRIEND_CLASS ( FileInstance );
            MAKE_FRIEND_CLASS ( DeviceInstanceProxy );
            MAKE_FRIEND_CLASS ( DeviceListProxy );
            MAKE_FRIEND_CLASS ( DLObserver );

		public:
			ENVIRONS_LIB_API    DeviceInstance ();
			ENVIRONS_LIB_API    ~DeviceInstance ();


			/**
			 * Add an observer (DeviceObserver) that notifies about device property changes.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void AddObserver ( environs::DeviceObserverPtr observer );

			/**
			 * Remove an observer (DeviceObserver) that was added before.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void RemoveObserver ( environs::DeviceObserverPtr observer );


			/**
			 * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForData ( environs::DataObserverPtr observer );


			/**
			 * Remove an observer (SensorObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForData ( environs::DataObserverPtr observer );


			/**
			 * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForSensors ( environs::SensorObserverPtr observer );


			/**
			 * Remove an observer (DataObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForSensors ( environs::SensorObserverPtr observer );


			/**
			 * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void AddObserverForMessages ( environs::MessageObserverPtr observer );


			/**
			 * Remove an observer (MessageObserver) that was added before.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForMessages ( environs::MessageObserverPtr observer );


			/**
			 * Notify to all observers (DeviceObserver) that the appContext has changed.
			 *
			 * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
			 */
			ENVIRONS_LIB_API void NotifyAppContextChanged ( int customFlags );


			/** The device properties structure into a DeviceInfo object. */
			environs::DeviceInfo OBJ_ptr info ();
        
            /** A descriptive string with the most important details. */
			ENVIRONS_LIB_API STRING_T toString ();
        
            /** IP from device. The IP address reported by the device which it has read from network configuration. */
			ENVIRONS_LIB_API STRING_T ips ();

        
            /** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
             * This address could be different from IP due to NAT, Router, Gateways behind the device.
             */
			ENVIRONS_LIB_API STRING_T ipes ();

			ENVIRONS_LIB_API bool EqualsAppEnv ( environs::DeviceInfoPtr equalTo );
			ENVIRONS_LIB_API bool EqualsAppEnv ( CString_ptr areaName, CString_ptr appName );

			ENVIRONS_LIB_API bool LowerThanAppEnv ( environs::DeviceInfoPtr compareTo );
			ENVIRONS_LIB_API bool LowerThanAppEnv ( CString_ptr areaName, CString_ptr appName );

			ENVIRONS_LIB_API bool EqualsID ( environs::DeviceInstancePtr equalTo );
			ENVIRONS_LIB_API bool EqualsID ( int deviceID, CString_ptr areaName, CString_ptr appName );


			ENVIRONS_LIB_API CString_ptr DeviceTypeString ( environs::DeviceInfoPtr info );
			ENVIRONS_LIB_API CString_ptr DeviceTypeString ();

			ENVIRONS_LIB_API CString_ptr GetBroadcastString ( bool fullText );
        
            /** isConnected is true if the device is currently in the connected state. */
			ENVIRONS_LIB_API bool isConnected ();
        
            ENVIRONS_LIB_API bool directStatus ();
        
            /** Determines whether this instance provides location node features. */
            ENVIRONS_LIB_API bool isLocationNode ();
        
            /** sourceType is a value of environs::DeviceSourceType and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
			ENVIRONS_LIB_API environs::DeviceSourceType_t sourceType ();

        
            /** Determines whether the remote device has attached an observer. */
            ENVIRONS_LIB_API bool isObserverReady ();
        
            /** Determines whether the remote device has attached a message observer. */
			ENVIRONS_LIB_API bool isMessageObserverReady ();
        
            /** Determines whether the remote device has attached a data observer. */
			ENVIRONS_LIB_API bool isDataObserverReady ();
        
            /** Determines whether the remote device has attached a sesnor observer. */
			ENVIRONS_LIB_API bool isSensorObserverReady ();
        
            /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
             Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
            ENVIRONS_LIB_API bool   GetAllowConnect ();
        
            /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
             Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
            ENVIRONS_LIB_API void   SetAllowConnect ( bool value );
        
#ifdef CLI_CPP
			/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
			environs::Call		async;

            static bool			notifyPropertyChangedDefault;
#endif        
			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			ENVIRONS_LIB_API bool disposed ();

			/** An identifier that is unique for this object. */
			ENVIRONS_LIB_API long objID () { return ( long ) objID_; };

			/**
			* Connect to this device asynchronously.
			*
			* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
			* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
			* 					true: A new connection has been triggered and is in progress
			*/
			ENVIRONS_LIB_API bool Connect ();

			/**
			* Connect to this device using the given mode.
			*
			* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
			*
			* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
			* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
			* 					true: A new connection has been triggered and is in progress
			*/
			ENVIRONS_LIB_API bool Connect ( environs::Call_t Environs_CALL_ );

			/**
			* Disconnect the device with the given id and a particular application environment.
			*
			* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
			*/
			ENVIRONS_LIB_API bool Disconnect ();

			/**
			* Disconnect the device using the given mode with the given id and a particular application environment.
			*
			* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
			*
			* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
			*/
			ENVIRONS_LIB_API bool Disconnect ( environs::Call_t Environs_CALL_ );

			/**
			* Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
			*
			* @return PortalInstance-object
			*/
			ENVIRONS_LIB_API environs::DeviceDisplay GetDisplayProps ();

			/**
			* Load the file that is assigned to the fileID received by deviceID into an byte array.
			*
			* @param fileID        The id of the file to load (given in the onData receiver).
			* @param size        An int pointer, that receives the size of the returned buffer.
			* @return byte-array
			*/
			ENVIRONS_LIB_API UCharArray_ptr GetFile ( int fileID, int OBJ_ref size );

			/**
			* Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
			*
			* @param fileID        The id of the file to load (given in the onData receiver).
			* @return absolutePath
			*/
			ENVIRONS_LIB_API CString_ptr GetFilePath ( int fileID );


			/**
			* Creates a portal instance that requests a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			PortalInstanceESP PortalRequest ( environs::PortalType_t portalType );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalRequestRetained ( environs::PortalType_t portalType );
#endif

			/**
			* Creates a portal instance that provides a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			PortalInstanceESP PortalProvide ( environs::PortalType_t portalType );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalProvideRetained ( environs::PortalType_t portalType );
#endif


			/**
			* Query the first PortalInstance that manages an outgoing portal.
			*
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalGetOutgoing ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalGetOutgoingRetained ();
#endif


			/**
			* Query the first PortalInstance that manages an incoming portal.
			*
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalGetIncoming ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalGetIncomingRetained ();
#endif


			/**
			* Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param filePath      The path to the file to be send.
			* @return success
			*/
			ENVIRONS_LIB_API bool SendFile ( int fileID, CString_ptr fileDescriptor, CString_ptr filePath );

			/**
			* Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param buffer        A buffer to be send.
			* @param bytesToSend number of bytes in the buffer to send
			* @return success
			*/
            ENVIRONS_LIB_API bool SendBuffer ( int fileID, CString_ptr fileDescriptor, UCharArray_ptr buffer, int bytesToSend );
            
            
            /**
             * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
             * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
             * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
             *
             * @return FileInstance
             */
            ENVIRONS_LIB_API FileInstanceESP ReceiveBuffer ();
        
#ifndef CLI_CPP
            ENVIRONS_LIB_API environs::FileInstance OBJ_ptr ReceiveBufferRetained ();
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
			 * @param async			(Environs.Call.NoWait) Perform asynchronous. (Environs.Call.Wait) Non-async means that this call blocks until the call finished.
			 * @param message		A message to send.
			 * @param length		Length of the message to send.
			 * @return success
			 */
			ENVIRONS_LIB_API bool SendMessage ( environs::Call_t async, CString_ptr msg, int length );

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
			ENVIRONS_LIB_API bool SendMessage ( CString_ptr msg );

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
            * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
			* @param message		A message to be send.
			* @return success
			*/
            ENVIRONS_LIB_API bool SendMessage ( environs::Call_t async, CString_ptr msg );
            
            
            /**
             * Receives a message send using SendMessage by the DeviceInstance.
             * This call blocks until a new message has been received or until the DeviceInstance gets disposed.
             * Messages that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
             *
             * @return MessageInstance
             */
            ENVIRONS_LIB_API MessageInstanceESP Receive ();
        
#ifndef CLI_CPP
            ENVIRONS_LIB_API environs::MessageInstance OBJ_ptr ReceiveRetained ();
#endif

			/**
			* Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
			*
			* @param buffer        A buffer to be send.
			* @param offset        A user-customizable id that identifies the file to be send.
			* @param bytesToSend number of bytes in the buffer to send
			* @return success
			*/
			ENVIRONS_LIB_API bool SendDataUdp ( UCharArray_ptr buffer, int offset, int bytesToSend );
        
        
            /**
             * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
             *
             * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
             * @param buffer        A buffer to be send.
             * @param offset        A user-customizable id that identifies the file to be send.
             * @param bytesToSend number of bytes in the buffer to send
             * @return success
             */
            ENVIRONS_LIB_API bool SendDataUdp ( environs::Call_t async, UCharArray_ptr buffer, int offset, int bytesToSend );
            
            
            /**
             * Receives a data buffer sent by the DeviceInstance using SendDataUdp.
             * This call blocks until new data has been received or until the DeviceInstance gets disposed.
             *
             * @return byte buffer
             */
            ENVIRONS_LIB_API UCharArray_ptr ReceiveData ();

			/**
			* Receives the raw pointer of a data buffer (including size) that was sent by the DeviceInstance using SendDataUdp.
			* This call blocks until new data has been received or until the DeviceInstance gets disposed.
			*
			* @return byte buffer
			*/
			ENVIRONS_LIB_API UCharArray_ptr ReceiveDataRawPointer ( int * size );


			/**
			* Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
			*
			*/
			ENVIRONS_LIB_API void ClearMessages ();

			/**
			* Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
			*
			*/
			ENVIRONS_LIB_API void ClearStorage ();

            /**
             * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
			*
			* @return Collection with objects of type FileInstance with the fileID as the key.
			*/
            NLayerMapType ( int, EPSPACE FileInstance ) GetFiles ();
            
            /**
             * Get a dictionary with all files that this device has received (and sent) from the storage.
             *
             * @return Collection with objects of type FileInstance with the fileID as the key.
             */
            NLayerMapType ( int, EPSPACE FileInstance ) GetFilesInStorage ();

#ifndef CLI_CPP
            ENVIRONS_LIB_API envArrayList OBJ_ptr GetFilesRetained ();
        
            ENVIRONS_LIB_API envArrayList OBJ_ptr GetFilesInStorageRetained ();
#endif
            /**
             * Clear cached MessageInstance and FileInstance objects for this DeviceInstance.
             *
             */
            ENVIRONS_LIB_API void DisposeStorageCache ();
        
            /**
             * Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
             * Note: These messages would otherwise been delivered by calls to Receive, that is by calling GetMessages,
             *       the messages will not be delivered to Receive afterwards.
             *
             * @return Collection with objects of type MessageInstance
             */
            NLayerVecType ( EPSPACE MessageInstance ) GetMessages ();
            
            /**
             * Get a list with all messages that this device has received (and sent) from the storage.
             *
             * @return Collection with objects of type MessageInstance
             */
            NLayerVecType ( EPSPACE MessageInstance ) GetMessagesInStorage ();
        
#ifndef CLI_CPP
			ENVIRONS_LIB_API envArrayList OBJ_ptr GetMessagesRetained ();
        
            ENVIRONS_LIB_API envArrayList OBJ_ptr GetMessagesInStorageRetained ();
#endif
            /**
             * Acquire or release lock on file and message instances.
             * Client code MUST balance successful locks. Otherwise deadlocks may happen.
             *
             * @param lock true = acquire, false = release.
             *
             * @return success
             */
            ENVIRONS_LIB_API bool StorageLock ( bool lock );
        

			/**
			* Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
			*
			* @return absolutePath
			*/
			ENVIRONS_LIB_API CString_ptr GetStoragePath ();

			/**
			 * Enable sending of sensor events to this DeviceInstance.
			 * Events are send if the device is connected and stopped if the device is disconnected.
			 *
			 * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
			 * @param enable true = enable, false = disable.
			 *
			 * @return success true = enabled, false = failed.
			 */
			ENVIRONS_LIB_API bool SetSensorEventSending ( environs::SensorType_t type, bool enable );


			/**
			* Query whether sending of the given sensor events to this DeviceInstance is enabled or not.
			*
			* @param type	A value of type Environs::SensorType / environs::SensorType_t.
			*
			* @return success true = enabled, false = disabled.
			*/
			ENVIRONS_LIB_API bool IsSetSensorEventSending ( environs::SensorType_t type );
        
#if ( defined(ENVIRONS_OSX) || defined(ENVIRONS_IOS) )
            void                    *   platformKeep;
#endif
        
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
            bool disposalEnqueued;
            bool disposalNotified;
            size_t lastObserversSize;
			int gotInserts;
            int gotInserts1;
            int gotInserts2;
            int gotInserts3;
            int gotInserts4;
			int gotUpdates;
			int gotUpdates1;
			int gotRemoves;
			int gotRemoves1;
			int gotRemoves2;
			int gotRemoves3;
			int gotRemoves41;
			int gotRemoves42;
			int gotRemoves5;
			int gotDisposes;
#endif

#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
			LONGSYNC	atLists;
			void CheckSPConsistency ();	
			void CheckSPConsistency1 ( long listsAlive );
#endif
			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		INTERNAL:

			ENVIRONS_OUTPUT_ALLOC_WP_RESOURCE ( DeviceInstance );

			static bool                 globalsInit;
			static bool                 GlobalsInit ();
			static void                 GlobalsDispose ();

			int                         hEnvirons;
		
#ifndef CLI_CPP
			Instance                *   env;

            DeviceInstanceESP           clearMessagesSP;
            DeviceInstanceESP           clearStorageSP;
#else
			static String ^ appContext0String = "appContext0";
			static String ^ appContext1String = "appContext1";
			static String ^ appContext2String = "appContext2";
			static String ^ appContext3String = "appContext3";
#endif

#ifdef DEBUG_TRACK_DEVICE_INSTANCE
    public:
        EnvironsPtr                 envObj;
        wp ( DeviceInstance )       myselfWP;
        bool    IsValid ();
    private:
#else
        EnvironsPtr                 envObj;
#endif

			pthread_mutex_t				devicePortalsLock;

			/** A collection of PortalInstances that this device has established or is managing. */
			NLayerVecTypeObj ( PortalInstanceEP )	devicePortals;

			/**
			 * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			LONGSYNC					disposed_;

			bool						isSameAppArea;
        
			void                        PlatformDispose ();

			static environs::DeviceInfo emptyInfo;

			/** The device properties structure into a DeviceInfo object. */
			environs::DeviceInfoPtr 	info_;

			/** The previous object id within the environment. 0 means invalid (not connected).
			* The previous object id is required if broadcast and mediator managed devices have to be merged.
			* In this case, we keep the previous objID to manage old events that may come after the objID update.*/
			OBJIDType					objIDPrevious;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIDeviceObserver, environs::DeviceObserver )	observers;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIMessageObserver, environs::MessageObserver )	observersForMessages;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIDataObserver, environs::DataObserver )		observersForData;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IISensorObserver, environs::SensorObserver )	observersForSensorData;

			char					storageLoaded;

			bool                    directStatus_;

			/** Determines whether the device shall receive sensor events from us. */
			unsigned int            enableSensorSender;

			STRING_T	            toString_;

			STRING_T	            ips_;
			STRING_T	            ipes_;
			STRING_T	            filePath;

			STRING_T				storagePath;
        
            pthread_cond_t          messagesEvent;
            bool                    messagesEnqueue;
        
#   ifdef USE_QUEUE_VECTOR_T_CLASS
            QueueVectorSP < MessageInstanceESP >   messages;
#   else
            stdQueue ( MessageInstanceESP ) messages;
#   endif
        
            NLayerVecType ( EPSPACE MessageInstance ) messagesCache;
        
        
            pthread_cond_t          filesEvent;
            int                     filesLast;
            NLayerMapType ( int, EPSPACE FileInstance ) files;
            NLayerMapType ( int, EPSPACE FileInstance ) filesCache;
			
			pthread_cond_t                   udpDataEvent;
            bool                             udpDataDisposeFront;
			stdQueue ( UdpDataPack OBJ_ptr ) udpData;

			pthread_mutex_t			storageLock;

#ifdef CLI_CPP
			virtual DeviceInstanceEP ^ GetPlatformObj () = 0;

			bool					notifyPropertyChanged;
#endif
			pthread_cond_t          changeEvent;
			pthread_mutex_t			changeEventLock;

			bool                    changeEventPending;

			short                   connectProgress;

			/** A DeviceDisplay structure that describes the device's display properties. */
			environs::DeviceDisplay display;

            bool                        IsEnqueueSafe ();
			void						EnqueueNotification ( DeviceNotifierContextPtr ctx );

			static void c_OBJ_ptr		 NotifierThread ( pthread_param_t envObj );

			static DeviceInstanceESP	Create ( int hInst, environs::DeviceInfoPtr device );
			bool						Init ( int hInst );

			bool			CopyInfo ( environs::DeviceInfoPtr device );
			bool			Update ( environs::DeviceInfoPtr device );

			void			SetProgress ( int progress );
			bool			SetDirectContact ( int status );
            bool			SetFileProgress ( int fileID, int progress, bool send );
        
            /**
             * Update device flags to native layer and populate them to the environment.
             *
             * @param	flags    The internal flags to set or clear.
             * @param	set    	 true = set, false = clear.
             */
            void			SetDeviceFlags ( environs::DeviceFlagsInternal_t flags, environs::Call_t async, bool set );

            void			DisposeInstance ( );
            void			DisposeMessages ( );
            void			DisposeMessages ( c_const NLayerVecType ( EPSPACE MessageInstance ) c_ref list );
            void			DisposeMessagesQ ( );
        
            void			DisposeFiles ( );
            void			DisposeFiles ( c_const NLayerMapType ( int, EPSPACE FileInstance ) c_ref list );
            void			DisposeUdpPacks ();
        
			void			NotifyObservers ( int flags, bool enqueue );

#ifdef CLI_CPP
			virtual void	OnPropertyChanged ( String ^ prop, bool ignoreDefaultSetting ) = 0;
#endif
			void			NotifyObserversForMessage ( c_const MessageInstanceESP c_ref message, environs::MessageInfoFlag_t flags, bool enqueue );
			
            void			NotifyObserversForData ( c_const FileInstanceESP c_ref fileInst, environs::FileInfoFlag_t flags, bool enqueue );
			
            void			NotifySensorObservers ( environs::SensorFrame OBJ_ptr pack );
        
            void            NotifyUdpData ( UdpDataContext OBJ_ptr ctx );

			void			ClearMessagesDo ();
			static void c_OBJ_ptr	ClearMessagesThread ( pthread_param_t arg );

			void			ClearStorageDo ();
			static void c_OBJ_ptr	ClearStorageThread ( pthread_param_t arg );

			void			VerifyStoragePath ();

			void			AddMessage ( CString_ptr message, int length, bool sent, char connection );
			void			AddFile ( int type, int fileID, CString_ptr fileDescriptor, CString_ptr filePath, int  bytesToSend, bool sent );

			static void c_OBJ_ptr	FileParseThread ( pthread_param_t pack );
			bool			ParseStoragePath ( bool wait );

			static void		ParseAllFiles ( c_const NLayerVecType ( DeviceInstanceEP ) c_ref devices );

			/**
			* Creates a portal instance.
			*
			* @param request   The portal request.
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalCreate ( int request );

			/**
			* Creates a portal instance.
			*
			* @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
			* @param portalType
			* @param slot
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalCreate ( int Environs_PORTAL_DIR, environs::PortalType_t portalType, int slot );

			/**
			* Creates a portal instance with a given portalID.
			*
			* @param portalID   The portalID received from native layer.
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalCreateID ( int portalID );


			/**
			* Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
			*
			* @return PortalInstance-object
			*/
			PortalInstanceESP PortalGetWaiting ( bool outgoing );

			PortalInstanceESP PortalGet ( bool outgoing );
		};
	}

} /* namepace Environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H
