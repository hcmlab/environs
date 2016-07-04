package environs;
/**
 * DeviceObserver
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

import android.support.annotation.Nullable;

/**
 *	DeviceObserver
 *  Attachable to **DeviceInstance** objects in order to receive changes of a particular device.
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface DeviceObserver {
    /**
     * Change notifications of object properties are conducted through the implementation of
     * the environs.ListObserver interface.
     *
     * The execution context is not guaranteed to be the UI thread context!
     * List changes are notified through onListChanged()
     * Device instance changes are notified through onItemChanged()
     *
     */

    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param device                                The DeviceInstance object that sends this notification.
     * @param Environs_DEVICE_INFO_ATTR_changed     The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
     */
    void OnDeviceChanged ( @Nullable DeviceInstance device, @DeviceInfoFlag.Value int Environs_DEVICE_INFO_ATTR_changed );


    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The PortalInstance object.
     */
    void OnPortalRequestOrProvided ( PortalInstance portal );
}
