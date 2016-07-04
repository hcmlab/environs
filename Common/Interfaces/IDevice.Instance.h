/**
 * Device Instance Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_INTERFACE_H

#include "Environs.Platforms.h"
#include "Environs.Platform.Decls.h"
#include "Interfaces/IMessage.List.h"
#include "Interfaces/IFile.List.h"
#include "Interfaces/IPortal.Instance.h"
#include "Device.Info.h"

#include "Device.Display.Decl.h"
#include "Interop/Threads.h"
#include "Interop/jni.h"
#include <string.h>

/**
 *	Device Instance Interface
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	/// Forwards
	//class PortalInstance;
	class MessageInstance;
	class FileInstance;
	class DataObserver;
	class SensorObserver;
	class MessageObserver;
	class DeviceObserver;

	namespace lib
	{
		class IIDataObserver;
		class IISensorObserver;
		class IIMessageObserver;
		class IIDeviceObserver;
	}


	class DeviceInstance : public lib::IEnvironsDispose
	{
	public:
		DeviceInstance () : appContext0 ( 0 ), appContext1 ( 0 ), appContext2 ( 0 ), appContext3 ( 0 ), async ( environs::Call::Wait ) { };
		virtual ~DeviceInstance () { };

		/** The device properties structure into a DeviceInfo object. */
		virtual environs::DeviceInfo * info () = 0;

        
        /** Application defined context 0 for arbitrary use. */
		int                     appContext0;
        
        /** Application defined context 1 for arbitrary use. */
		void *                  appContext1;
        
        /** Application defined context 2 for arbitrary use. */
		void *                  appContext2;
        
        /** Application defined context 3 for arbitrary use. */
		void *                  appContext3;

		/** Perform the tasks asynchronously. If set to Environs.Call.Wait, the commands will block (if possible) until the task finishes. */
        environs::Call_t		async;
        
        /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
         Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
        virtual void            SetAllowConnect ( bool value ) = 0;
        
        /** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
         Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
        virtual bool            GetAllowConnect () = 0;
        
		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		virtual void Release () = 0;

		/**
		* Add an observer (DeviceObserver) that notifies about device property changes.
		*
		* @param observer A DeviceObserver
		*/
		virtual void AddObserver ( DeviceObserver * observer ) = 0;

		/**
		* Remove an observer (DeviceObserver) that was added before.
		*
		* @param observer A DeviceObserver
		*/
		virtual void RemoveObserver ( DeviceObserver * observer ) = 0;


		/**
		* Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
		*
		* @param observer A DataObserver
		*/
		virtual void AddObserverForData ( DataObserver * observer ) = 0;


		/**
		* Remove an observer (SensorObserver) that was added before.
		*
		* @param observer A DataObserver
		*/
		virtual void RemoveObserverForData ( DataObserver * observer ) = 0;


		/**
		* Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
		*
		* @param observer A DataObserver
		*/
		virtual void AddObserverForSensors ( SensorObserver * observer ) = 0;


		/**
		* Remove an observer (DataObserver) that was added before.
		*
		* @param observer A DataObserver
		*/
		virtual void RemoveObserverForSensors ( SensorObserver * observer ) = 0;


		/**
		* Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
		*
		* @param observer A MessageObserver
		*/
		virtual void AddObserverForMessages ( MessageObserver * observer ) = 0;


		/**
		* Remove an observer (MessageObserver) that was added before.
		*
		* @param observer A MessageObserver
		*/
		virtual void RemoveObserverForMessages ( MessageObserver * observer ) = 0;


		/**
		* Notify to all observers (DeviceObserver) that the appContext has changed.
		*
		* @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
		*/
		virtual void NotifyAppContextChanged ( int customFlags ) = 0;
        
        /** A descriptive string with the most important details. */
		virtual std::string toString () = 0;
        
        /** IP from device. The IP address reported by the device which it has read from network configuration. */
        virtual std::string ips () = 0;
        
        /** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
         * This address could be different from IP due to NAT, Router, Gateways behind the device.
         */
		virtual std::string ipes () = 0;

		virtual bool EqualsAppEnv ( environs::DeviceInfo * equalTo ) = 0;
		virtual bool EqualsAppEnv ( const char * areaName, const char * appName ) = 0;

		virtual bool LowerThanAppEnv ( environs::DeviceInfo * compareTo ) = 0;
		virtual bool LowerThanAppEnv ( const char * areaName, const char * appName ) = 0;

		virtual bool EqualsID ( DeviceInstance * equalTo ) = 0;
		virtual bool EqualsID ( int deviceID, const char * areaName, const char *appName ) = 0;


		virtual const char * DeviceTypeString ( environs::DeviceInfo * info ) = 0;
		virtual const char * DeviceTypeString () = 0;

		virtual const char * GetBroadcastString ( bool fullText ) = 0;

        
        /** isConnected is true if the device is currently in the connected state. */
		virtual bool isConnected () = 0;
        
        /**
         * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
         * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
         * */
		virtual bool disposed () = 0;
        
        /** Determines whether this instance provides location node features. */
        virtual bool isLocationNode () = 0;
        
        /** sourceType is a value of environs::DeviceSourceType and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
		virtual environs::DeviceSourceType_t sourceType () = 0;

        
        /** Determines whether the remote device has attached an observer. */
		virtual bool isObserverReady () = 0;
        
        /** Determines whether the remote device has attached a message observer. */
		virtual bool isMessageObserverReady () = 0;
        
        /** Determines whether the remote device has attached a data observer. */
		virtual bool isDataObserverReady () = 0;
        
        /** Determines whether the remote device has attached a sesnor observer. */
		virtual bool isSensorObserverReady () = 0;

        
        /** An identifier that is unique for this object of this type. */
		virtual long objID () = 0;

		/**
		* Connect to this device asynchronously.
		*
		* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
		* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
		* 					true: A new connection has been triggered and is in progress
		*/
		virtual bool Connect () = 0;

		/**
		* Connect to this device using the given mode.
		*
		* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
		*
		* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
		* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
		* 					true: A new connection has been triggered and is in progress
		*/
		virtual bool Connect ( environs::Call_t Environs_CALL_ ) = 0;

		/**
		* Disconnect the device with the given id and a particular application environment.
		*
		* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
		*/
		virtual bool Disconnect () = 0;

		/**
		* Disconnect the device using the given mode with the given id and a particular application environment.
		*
		* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
		*
		* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
		*/
		virtual bool Disconnect ( environs::Call_t Environs_CALL_ ) = 0;

		/**
		* Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
		*
		* @return PortalInstance-object
		*/
		virtual environs::DeviceDisplay GetDisplayProps () = 0;

		/**
		* Load the file that is assigned to the fileID received by deviceID into an byte array.
		*
		* @param fileID        The id of the file to load (given in the onData receiver).
		* @param size        An int pointer, that receives the size of the returned buffer.
		*
		* @return byte-array
		*/
		virtual unsigned char * GetFile ( int fileID, int & size ) = 0;

		/**
		* Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
		*
		* @param fileID        The id of the file to load (given in the onData receiver).
		* @return absolutePath
		*/
		virtual const char * GetFilePath ( int fileID ) = 0;


		/**
		* Creates a portal instance that requests a portal.
		*
		* @param 	portalType	        Project name of the application environment
		*
		* @return 	PortalInstance-object
		*/
		sp ( PortalInstance ) PortalRequest ( environs::PortalType_t portalType )
		{
			ENVIRONS_IR_SP1_RETURN ( PortalInstance, PortalRequestRetained ( portalType ) );
		}

		virtual PortalInstance * PortalRequestRetained ( environs::PortalType_t portalType ) = 0;

		/**
		* Creates a portal instance that provides a portal.
		*
		* @param 	portalType	        Project name of the application environment
		*
		* @return 	PortalInstance-object
		*/
		sp ( PortalInstance ) PortalProvide ( environs::PortalType_t portalType )
		{
			ENVIRONS_IR_SP1_RETURN ( PortalInstance, PortalProvideRetained ( portalType ) );
		}

		virtual PortalInstance * PortalProvideRetained ( environs::PortalType_t portalType ) = 0;


		/**
		* Query the first PortalInstance that manages an outgoing portal.
		*
		* @return PortalInstance-object
		*/
		sp ( PortalInstance ) PortalGetOutgoing ()
		{
			ENVIRONS_IR_SP1_RETURN ( PortalInstance, PortalGetOutgoingRetained () );
		}

		virtual PortalInstance * PortalGetOutgoingRetained () = 0;

		/**
		* Query the first PortalInstance that manages an incoming portal.
		*
		* @return PortalInstance-object
		*/
		sp ( PortalInstance ) PortalGetIncoming ()
		{
			ENVIRONS_IR_SP1_RETURN ( PortalInstance, PortalGetIncomingRetained () );
		}

		virtual PortalInstance * PortalGetIncomingRetained () = 0;

		/**
		* Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
		*
		* @param fileID        A user-customizable id that identifies the file to be send.
		* @param fileDescriptor (e.g. filename)
		* @param filePath      The path to the file to be send.
		* @return success
		*/
		virtual bool SendFile ( int fileID, const char * fileDescriptor, const char * filePath ) = 0;

		/**
		* Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
		*
		* @param fileID        A user-customizable id that identifies the file to be send.
		* @param fileDescriptor (e.g. filename)
		* @param buffer        A buffer to be send.
		* @param bytesToSend number of bytes in the buffer to send
		* @return success
		*/
        virtual bool SendBuffer ( int fileID, const char * fileDescriptor, unsigned char * buffer, int bytesToSend ) = 0;


        /**
         * Receives a buffer sent by the DeviceInstance using  SendBuffer/SendFile.
         * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
         * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
         *
         * @return FileInstance
         */
        sp ( FileInstance ) ReceiveBuffer ()
        {
            ENVIRONS_IR_SP1_RETURN ( FileInstance, ReceiveBufferRetained () );
        }

        virtual FileInstance * ReceiveBufferRetained () = 0;


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
		* @param async			(Environs.CALL_NOWAIT) Perform asynchronous. (Environs.CALL_WAIT) Non-async means that this call blocks until the call finished.
		* @param message		A message to send.
		* @param length			Length of the message to send.
		* @return success
		*/
		virtual bool SendMessage ( environs::Call_t async, const char * msg, int length ) = 0;

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
		virtual bool SendMessage ( const char * message ) = 0;

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
		* @param message		A message to be send.
		* @return success
		*/
        virtual bool SendMessage ( environs::Call_t async, const char * message ) = 0;


        /**
         * Receives a message sent by the DeviceInstance using SendMessage.
         * This call blocks until a new message has been received or until the DeviceInstance gets disposed.
         * Messages that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
         *
         * @return MessageInstance
         */
        sp ( MessageInstance ) Receive ()
        {
            ENVIRONS_IR_SP1_RETURN ( MessageInstance, ReceiveRetained () );
        }

        virtual MessageInstance * ReceiveRetained () = 0;


		/**
		* Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
		*
		* @param buffer        A buffer to be send.
		* @param offset        A user-customizable id that identifies the file to be send.
		* @param bytesToSend number of bytes in the buffer to send
		* @return success
		*/
		virtual bool SendDataUdp ( unsigned char * buffer, int offset, int bytesToSend ) = 0;


        /**
         * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
         *
         * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
         * @param buffer        A buffer to be send.
         * @param offset        A user-customizable id that identifies the file to be send.
         * @param bytesToSend number of bytes in the buffer to send
         * @return success
         */
        virtual bool SendDataUdp ( environs::Call_t async, unsigned char * buffer, int offset, int bytesToSend ) = 0;


        /**
         * Receives a data buffer sent by the DeviceInstance using SendDataUdp.
         * This call blocks until new data has been received or until the DeviceInstance gets disposed.
         *
         * @return byte buffer
         */
		up ( unsigned char [ ] ) ReceiveData ( int * size )
		{
			int sizeInBytes = 0;
            
			UCharArray_ptr data = ReceiveDataRawPointer ( &sizeInBytes );
            
			if ( data && sizeInBytes > 0 ) {
				up ( unsigned char [ ] ) bufferUP ( new unsigned char [ sizeInBytes ] );
				if ( bufferUP ) {
					memcpy ( bufferUP.get (), data, (size_t) sizeInBytes );
                    if ( size )
                        *size = sizeInBytes;
					return bufferUP;
				}
			}
			return 0;
		};

		virtual UCharArray_ptr ReceiveDataRawPointer ( int * size ) = 0;


        /**
		* Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
		*
		*/
        virtual void ClearMessages () = 0;

		/**
		* Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
		*
		*/
        virtual void ClearStorage () = 0;

        /**
         * Clear cached MessageInstance and FileInstance objects for this DeviceInstance.
         *
         */
        virtual void DisposeStorageCache () = 0;


		/**
		* Get a dictionary with all files that this device has received (and sent) from the storage.
		*
		* @return Collection with objects of type MessageInstance
		*/
		sp ( FileList ) GetFilesInStorage ()
		{
			ENVIRONS_IR_SP1_RETURN ( FileList, GetFilesInStorageRetained () );
        };
        
        
        /**
         * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
         *
         * @return Collection with objects of type MessageInstance
         */
        sp ( FileList ) GetFiles ()
        {
            ENVIRONS_IR_SP1_RETURN ( FileList, GetFilesRetained () );
        };


		/**
		* Get a list with all messages that this device has received (and sent) from the storage.
		*
		* @return Collection with objects of type MessageInstance
		*/
		sp ( MessageList ) GetMessagesInStorage ()
		{
			ENVIRONS_IR_SP1_RETURN ( MessageList, GetMessagesInStorageRetained () );
        };
        
                
        /**
         * Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
         * Note: These messages would otherwise been delivered by calls to Receive, that is by calling GetMessages,
         *       the messages will not be delivered to Receive afterwards.
         *
         * @return Collection with objects of type MessageInstance
         */
        sp ( MessageList ) GetMessages ()
        {
            ENVIRONS_IR_SP1_RETURN ( MessageList, GetMessagesRetained () );
        };


		/**
		* Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
		*
		* @return absolutePath
		*/
		virtual const char * GetStoragePath () = 0;

		/**
		* Enable sending of sensor events to this DeviceInstance.
		* Events are send if the device is connected and stopped if the device is disconnected.
		*
		* @param type	A value of type environs::SensorType_t.
		* @param enable true = enable, false = disable.
		*
		* @return success true = enabled, false = failed.
		*/
		virtual bool SetSensorEventSending ( environs::SensorType_t type, bool enable ) = 0;

		/**
		* Query whether sending of the given sensor events to this DeviceInstance is enabled or not.
		*
		* @param type	A value of type environs::SensorType_t.
		*
		* @return success true = enabled, false = disabled.
		*/
		virtual bool IsSetSensorEventSending ( environs::SensorType_t type ) = 0;

	private:
		/**
		* Get a dictionary with all files that this device instance has received since the Device instance has appeared.
		*
		* @return Collection with objects of type FileInstance with the fileID as the key.
		*/
        virtual ArrayList * GetFilesRetained () = 0;
        
        /**
         * Get a dictionary with all files that this device instance has received from the storage.
         *
         * @return Collection with objects of type FileInstance with the fileID as the key.
         */
        virtual ArrayList * GetFilesInStorageRetained () = 0;


		/**
		* Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
		*
		* @return Collection with objects of type MessageInstance
		*/
        virtual ArrayList * GetMessagesRetained () = 0;
        
        /**
         * Get a list with all messages that this device has received (and sent) from the storage.
         *
         * @return Collection with objects of type MessageInstance
         */
        virtual ArrayList * GetMessagesInStorageRetained () = 0;


    };

} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_INTERFACE_H




