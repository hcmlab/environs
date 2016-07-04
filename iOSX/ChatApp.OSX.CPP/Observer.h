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
#include "Environs.Observer.h"

using namespace environs;


class Observer :
    public EnvironsObserver,
    public ListObserver,
    public EnvironsMessageObserver,
    public DeviceObserver,
    public DataObserver,
    public EnvironsDataObserver,
    public PortalObserver,
    public MessageObserver
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
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param status      A status constant of environs::Status
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
     * @param portal 		The PortalInstance object.
     */
    void    OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal );
    
    
    void    OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared );
    
    
    void    OnDeviceChanged ( const sp ( DeviceInstance ) &device, environs::DeviceInfoFlag_t flags );
    
    
    /**
     * OnData is called whenever new binary data (files, buffers) has been received.
     * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
     *
     * @param fileData  The corresponding file object of type FileInstance
     * @param flags     Flags that indicate the object change.
     */
    //void    OnData ( sp ( IFileInstance ) fileInstance, int flags );
    
    
    /**
     * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
     *
     * @param sender    The PortalInstance object.
     * @param notify	The notification (environs::Notify::Portale) that indicates the change.
     */
    void    OnPortalChanged ( const sp ( PortalInstance ) &sender, environs::Notify::Portale_t notify );
    
    
    /**
     * OnMessage is called whenever a text message has been received from a device.
     *
     * @param msg   The corresponding message object of type MessageInstance
     * @param flags Flags that indicate the object change.
     */
    void    OnMessage ( const sp ( MessageInstance ) &msg, environs::MessageInfoFlag_t flags );
};


#endif /* INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H */



