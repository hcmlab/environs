/**
 * EnvironsObserver
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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_H

#include "Environs.Observer.IAPI.h"
#include "Environs.Platform.Decls.h"


namespace environs
{
	/**
	* EnvironsObserver: Attachable to **IEnvirons** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
	*/
	class EnvironsObserver : lib::IIEnvironsObserver
	{
	public:
		/** Constructor */
		EnvironsObserver () { };
		virtual ~EnvironsObserver () { };

		/**
		* OnStatus is called whenever the framework status changes.&nbsp;
		*
		* @param status      A status constant of type environs::Status_t
		*/
		virtual void OnStatus ( environs::Status_t status ) { OnEnvironsStatus_ = false; };


		/**
		* OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
		* The notification parameter is an integer value which represents one of the values as listed in Types.*
		* The string representation can be retrieved through TypesResolver.get(notification).
		*
		* @param nativeID      The native identifier that targets the device.
		* @param notification  The notification
		* @param source	    A value of the enumeration Types.EnvironsSource
		* @param context       A value that provides additional context information (if available).
		*/
		virtual void OnNotify ( environs::ObserverNotifyContext * context ) { OnEnvironsNotify_ = false; };


		/**
		* OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
		* The notification parameter is an integer value which represents one of the values as listed in Types.*
		* The string representation can be retrieved through TypesResolver.get(notification).
		*
		* @param deviceID      The device id of the sender device.
		* @param areaName      Area name of the application environment
		* @param appName		Application name of the application environment
		* @param notification  The notification
		* @param source		A value of the enumeration Types.EnvironsSource
		* @param context       A value that provides additional context information (if available).
		*/
		virtual void OnNotifyExt ( environs::ObserverNotifyContext * context ) { OnEnvironsNotifyExt_ = false; };


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
		/**
		* OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
		*
		* @param portal 		The PortalInstance object.
		*/
		virtual void OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal ) { OnEnvironsPortalRequestOrProvided_ = false; };


		/**
		* OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
		*
		* @param portal 		The PortalInstance object.
		*/
		virtual void OnPortalRequestOrProvidedInterface ( PortalInstance * portal ) { OnEnvironsPortalRequestOrProvidedInterface_ = false; };
#endif
	};


    /**
     * ListObserver: Attachable to **IDeviceList** objects in order to receive list changes of a particular IDeviceList.
     * Important: You must not call methods of DeviceList objects within the context of the observer callback. Otherwise deadlocks might occur.
     */
	class ListObserver : lib::IIListObserver
	{
	public:
		/** Constructor */
		ListObserver () { };
		virtual ~ListObserver () { };

        /**
         * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
         * Important: You must not call methods of DeviceList objects within the context of the observer callback. Otherwise deadlocks might occur.
         *
         * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
         * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
         */
		virtual void OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared ) { OnListChanged_ = false; };

        /**
         * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
         * Important: You must not call methods of DeviceList objects within the context of the observer callback. Otherwise deadlocks might occur.
         *
         * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
         * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
         */
		virtual void OnListChangedInterface ( DeviceInstanceList * vanished, DeviceInstanceList * appeared ) { OnListChangedInterface_ = false; };
	};


	/**
	* DeviceObserver: Attachable to **IDeviceInstance** objects in order to receive changes of a particular device.
	*/
	class DeviceObserver : lib::IIDeviceObserver
	{
	public:
		/** Constructor */
		DeviceObserver () { };
		virtual ~DeviceObserver () { };

		/**
		* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
		* The flags parameter provides a bit set which indicates the member that has changed.
		*
		* @param device     The DeviceInstance object that sends this notification.
		* @param flags      The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
		*/
		virtual void OnDeviceChanged ( const sp ( DeviceInstance ) &device, environs::DeviceInfoFlag_t flags ) { OnDeviceChanged_ = false; };


		/**
		* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
		* The flags parameter provides a bit set which indicates the member that has changed.
		*
		* @param device     The DeviceInstance object that sends this notification.
		* @param flags      The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
		*/
        virtual void OnDeviceChangedInterface ( DeviceInstance * device, environs::DeviceInfoFlag_t flags ) { OnDeviceChangedInterface_ = false; };


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
        /**
         * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
         *
         * @param portal 		The PortalInstance object.
         */
        virtual void OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal ) { OnEnvironsPortalRequestOrProvided_ = false; };


        /**
         * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
         *
         * @param portal 		The PortalInstance object.
         */
        virtual void OnPortalRequestOrProvidedInterface ( PortalInstance * portal ) { OnEnvironsPortalRequestOrProvidedInterface_ = false; };
#endif
	};


	/**
	* EnvironsMessageObserver: Attachable to **IEnvirons** objects in order to receive all messages that the Environs instance received.
	*/
	class EnvironsMessageObserver : lib::IIEnvironsMessageObserver
	{
	public:
		/** Constructor */
		EnvironsMessageObserver () { };
		virtual ~EnvironsMessageObserver () { };

		/**
		* OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
		*
		* @param nativeID      The native identifier that targets the device.
		* @param type	        Determines the source (either from a device, environs, or native layer)
		* @param message       The message as string text
		* @param length        The length of the message
		*/
		virtual void OnMessage ( environs::ObserverMessageContext * context ) { OnEnvironsMessage_ = false; };


		/**
		* OnMessageExt is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
		*
		* @param deviceID      The device id of the sender device.
		* @param areaName      Area name of the application environment
		* @param appName		Application name of the application environment
		* @param type	        Determines the source (either from a device, environs, or native layer)
		* @param message       The message as string text
		* @param length        The length of the message
		*/
		virtual void OnMessageExt ( environs::ObserverMessageContext * context ) { OnEnvironsMessageExt_ = false; };


		/**
		* OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
		*
		* @param message      The status as a text message.
		*/
		virtual void OnStatusMessage ( const char * message ) { OnEnvironsStatusMessage_ = false; };
	};


	/**
	* MessageObserver: Attachable to **IDeviceInstance** objects in order to receive messages of a particular device.
	*/
	class MessageObserver : lib::IIMessageObserver
	{
	public:
		/** Constructor */
		MessageObserver () { };
		virtual ~MessageObserver () { };

		/**
		* OnMessage is called whenever a text message has been received from a device.
		*
		* @param msg    The corresponding message object of type MessageInstance
		* @param flags	Flags that indicate the object change.
		*/
		virtual void OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t flags ) { OnMessage_ = false; };

		/**
		* OnMessage is called whenever a text message has been received from a device.
		*
		* @param msg    The corresponding message object of type MessageInstance
		* @param flags	Flags that indicate the object change.
		*/
		virtual void OnMessageInterface ( MessageInstance * msg, environs::MessageInfoFlag_t flags ) { OnMessageInterface_ = false; };
	};


	/**
	* EnvironsDataObserver: Attachable to **IEnvirons** objects in order to receive all data transmissions that the Environs instance received.
	*/
	class EnvironsDataObserver : lib::IIEnvironsDataObserver
	{
	public:
		/** Constructor */
		EnvironsDataObserver () { };
		virtual ~EnvironsDataObserver () { };

		/**
		* OnData is called whenever new binary data (files, buffers) has been received.
		* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
		*
		* @param nativeID      The native identifier that targets the device.
		* @param type          The type of the data
		* @param fileID        A user-customizable id that identifies the file
		* @param descriptor    A text that describes the file
		* @param size          The size in bytes
		*/
		virtual void OnData ( environs::ObserverDataContext * context ) { OnEnvironsData_ = false; };
	};


	/**
	* DataObserver: Attachable to **IDeviceInstance** objects in order to receive data transmissions of a particular device.
	*/
	class DataObserver : lib::IIDataObserver
	{
	public:
		/** Constructor */
		DataObserver () { };
		virtual ~DataObserver () { };

		/**
		* OnData is called whenever new binary data (files, buffers) has been received.
		* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
		*
		* @param fileData   The corresponding file object of type FileInstance
		* @param flags      Flags that indicate the object change.
		*/
		virtual void OnData ( const sp ( FileInstance ) &fileData, environs::FileInfoFlag_t flags ) { OnData_ = false; };

		/**
		* OnData is called whenever new binary data (files, buffers) has been received.
		* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
		*
		* @param fileData   The corresponding file object of type FileInstance
		* @param flags      Flags that indicate the object change.
		*/
		virtual void OnDataInterface ( FileInstance * fileData, environs::FileInfoFlag_t flags ) { OnDataInterface_ = false; };
	};


	/**
	* EnvironsSensorDataObserver: Attachable to **IEnvirons** objects in order to receive all sensor data that the Environs instance received.
	*/
	class EnvironsSensorObserver : lib::IIEnvironsSensorObserver
	{
	public:
		/** Constructor */
		EnvironsSensorObserver () { };
		virtual ~EnvironsSensorObserver () { };

		/**
		* OnSensorData is called whenever new binary data (files, buffers) has been received.
		* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
		*
		* @param objID          The native identifier that targets the device.
		* @param sensorFrame    The frame containing the sensor data
		*/
		virtual void OnSensorData ( OBJIDType objID, ::environs::SensorFrame * sensorFrame ) { OnSensorData_ = false; };
	};


	/**
	* SensorObserver: Attachable to **IDeviceInstance** objects in order to receive sensor data from a particular device.
	*/
	class SensorObserver : lib::IISensorObserver
	{
	public:
		/** Constructor */
		SensorObserver () { };
		virtual ~SensorObserver () { };

		/**
		* OnSensorData is called whenever new sensor data has been received.
		*
		* @param sensorFrame     The corresponding SensorFrame of sensor data
		*/
		virtual void OnSensorData ( ::environs::SensorFrame * sensorFrame ) { OnSensorData_ = false; };
	};


	/**
	* PortalObserver: Attachable to **PortalInstance** objects in order to receive changes of a particular interactive portal.
	*/
	class PortalObserver : lib::IIPortalObserver
	{
	public:
		/** Constructor */
		PortalObserver () { };
		virtual ~PortalObserver () { };

		/**
		* OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
		*
		* @param portal		The PortalInstance object.
		* @param notify		The notification that indicates the change.
		*/
		virtual void OnPortalChanged ( const sp ( PortalInstance ) &portal, environs::Notify::Portale_t notify ) { OnPortalChanged_ = false; };

		/**
		* OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
		*
		* @param portal		The PortalInstance object.
		* @param notify		The notification that indicates the change.
		*/
		virtual void OnPortalChangedInterface ( PortalInstance * portal, environs::Notify::Portale_t notify ) { OnPortalChangedInterface_ = false; };
	};

}

#endif

