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

#define DISABLE_ENVIRONS_OBJC_API
#include "Environs.h"

using namespace environs;


class Observer :
    public EnvironsObserver,
    public ListObserver,
    public EnvironsMessageObserver,
    public DeviceObserver,
    public SensorObserver
{
public:
    /** Constructor */
    Observer () {};
    
    ~Observer () {}
    
    
    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param nativeID      The native identifier that targets the device.
     * @param notification  The notification
     * @param sourceIdent   A value of the enumeration Types.EnvironsSource
     * @param context       A value that provides additional context information (if available).
     */
    void    OnNotify ( environs::ObserverNotifyContext * context );
    
    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param deviceID      The device id of the sender device.
     * @param areaName      Area name of the application environment
     * @param appName		Application name of the application environment
     * @param notification  The notification
     * @param sourceIdent   A value of the enumeration Types.EnvironsSource
     * @param context       A value that provides additional context information (if available).
     */
    void    OnNotifyExt ( environs::ObserverNotifyContext * context );
    
    
    /**
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param status      A status constant of type environs::Status
     */
    void    OnStatus ( environs::Status_t status );
    
    
    /**
     * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
     *
     * @param msg      The status as a text message.
     */
    void    OnStatusMessage ( const char * message );
    
    
    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The IPortalInstance object.
     */
    void    OnPortalRequestOrProvided ( sp ( environs::PortalInstance ) portal );
    
    
    void    OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared );
    
    
    void    OnDeviceChanged ( const sp ( environs::DeviceInstance ) &device, environs::DeviceInfoFlag_t flags );
    
    /**
     * OnSensorData is called whenever new sensor data has been received.
     *
     * @param sensorFrame     The corresponding SensorFrame of sensor data
     */
    void OnSensorData ( environs::SensorFrame * sensorFrame );
};


#endif /* INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H */



