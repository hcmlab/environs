/**
 * Observer.h
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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H
#define INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H

#include "Environs.Observer.h"

using namespace environs;


class Observer :
	public EnvironsObserver, public EnvironsMessageObserver, public EnvironsDataObserver, public EnvironsSensorObserver,
    public ListObserver,    
	public DeviceObserver,
	public PortalObserver, public MessageObserver, public DataObserver, public SensorObserver
{
public:
    /** Constructor */
    /*Observer () {};     */
	~Observer ();
	

	///
	/// EnvironsObserver
	/// ------------------
	/// Attachable to **IEnvirons** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
	///
	/**
	* OnStatus is called whenever the framework status changes.&nbsp;
	*
	* @param status      A status constant of type Status
	*/
	void OnStatus ( Status_t status );
    
    
    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
	 * @param context		An object reference of type environs::NotifyContext.
     */
	void OnNotify ( environs::ObserverNotifyContext * context );

    
    /**
     * OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
	 * @param context		An object reference of type environs::NotifyContext.
     */
	void OnNotifyExt ( environs::ObserverNotifyContext * context );
    
    
    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The PortalInstance object.
     */
	void OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal );


	///
	/// ListObserver 
	/// ------------------
	/// Attachable to **IDeviceList** objects in order to receive list changes of a particular IDeviceList.
	///
	/**
	* OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
	*
	* @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
	* @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
	*/
	void OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared );


	///
	/// DeviceObserver 
	/// ------------------
	/// Attachable to **IDeviceInstance** objects in order to receive changes of a particular device.
	///
	/**
	* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
	* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
	*
	* @param device			The DeviceInstance object that sends this notification.
	* @param changedFlags	The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
	*/
	void OnDeviceChanged ( const sp ( DeviceInstance ) &device, environs::DeviceInfoFlag_t changedFlags );


	///
	/// EnvironsMessageObserver 
	/// ------------------
	/// Attachable to **IEnvirons** objects in order to receive all messages that the Environs instance received.
	///
	/**
	* OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
	*
	* @param context		An object reference of type environs::ObserverMessageContext.
	*/
	void OnMessage ( environs::ObserverMessageContext * context );


	/**
	* OnMessageExt is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
	*
	* @param context		An object reference of type environs::ObserverMessageContext.
	*/
	void OnMessageExt ( environs::ObserverMessageContext * context );


	/**
	* OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
	*
	* @param message    The status as a text message.
	*/
	void OnStatusMessage ( const char * message );


	///
	/// MessageObserver 
	/// ------------------
	/// Attachable to **IDeviceInstance** objects in order to receive messages of a particular device.
	///    
	/**
	* OnMessage is called whenever a text message has been received from a device.
	*
	* @param msg			The corresponding message object of type MessageInstance
	* @param changedFlags	Flags that indicate the object change.
	*/
	void OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t changedFlags );


	///
	/// EnvironsDataObserver 
	/// ------------------
	/// Attachable to **IEnvirons** objects in order to receive all data transmissions that the Environs instance received.
	///       
	/**
	* OnData is called whenever new binary data (files, buffers) has been received.
	* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
	*
	* @param context		An object reference of type environs::ObserverDataContext.
	*/
	void OnData ( environs::ObserverDataContext * context );


	///
	/// DataObserver 
	/// ------------------
	/// Attachable to **IDeviceInstance** objects in order to receive data transmissions of a particular device.
	///  
	/**
	* OnData is called whenever new binary data (files, buffers) has been received.
	* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
	*
	* @param fileData		The corresponding file object of type FileInstance
	* @param changedFlags	Flags that indicate the object change.
	*/
	void OnData ( const sp ( FileInstance ) &fileData, environs::FileInfoFlag_t changedFlags );


	///
	/// EnvironsSensorDataObserver 
	/// ------------------
	/// Attachable to **IEnvirons** objects in order to receive all sensor data that the Environs instance received.
	///    
	/**
	* OnSensorData is called whenever new binary data (files, buffers) has been received.
	* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
	*
	* @param nativeID      The native identifier that targets the device.
	* @param pack          The frame containing the sensor data
	*/
	void OnSensorData ( int nativeID, environs::SensorFrame * pack );


	///
	/// SensorObserver 
	/// ------------------
	/// Attachable to **IDeviceInstance** objects in order to receive sensor data from a particular device.
	///
	/**
	* OnSensorData is called whenever new sensor data has been received.
	*
	* @param pack     The corresponding SensorFrame of sensor data
	*/
	void OnSensorData ( environs::SensorFrame * pack );


	/// PortalObserver 
	/// ------------------
	/// Attachable to **PortalInstance** objects in order to receive changes of a particular interactive portal.
	///
	/**
	* OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
	*
	* @param portal		The PortalInstance object.
	* @param notify		The notification (Notify::Portal) that indicates the change.
	*/
	void OnPortalChanged ( const sp ( PortalInstance ) &portal, Notify::Portal_t notify );
    
};


#endif /* INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H */



