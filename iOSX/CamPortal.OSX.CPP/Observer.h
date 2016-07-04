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
    public DataObserver,
    public SensorObserver,
    public PortalObserver
{
public:
    /** Constructor */
    Observer () {};
    
    ~Observer () {}
    
    
    /**
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param status      A status constant of type STATUS_*
     */
    void    OnStatus ( environs::Status_t status );
    
    
    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The IPortalInstance object.
     */
    void    OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal );
    
    
    void    OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared );
    
    
    void    OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t flags );
    
    /**
     * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
     *
     * @param sender    The PortalInstance object.
     * @param notify	The notification that indicates the change.
     */
    void OnPortalChanged ( const sp ( PortalInstance ) &portal, Notify::Portale_t notify );
};


#endif /* INCLUDE_HCM_ENVIRONS_OBSERVER_IMPL_H */



