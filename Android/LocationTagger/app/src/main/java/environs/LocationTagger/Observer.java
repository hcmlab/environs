package environs.LocationTagger;
/**
 *	Observer
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
import environs.*;

/**
 * Observer observes notification from Environs.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class Observer implements EnvironsObserver
{
    /**
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param Environs_Status      A status constant of type STATUS_*
     */
    public void OnStatus ( @Status.Value int Environs_Status )
    {
        if ( Environs_Status >= Status.Started ) {
            // Here's a good place to update menu items.
            UpdateDeviceID ();
        }
    }


    void UpdateDeviceID ()
    {
        int deviceID = 0;

        if (MainActivity.environs != null)
            deviceID = MainActivity.environs.GetDeviceID ();

        final int devID = deviceID;
        MainActivity.instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MainActivity.instance.setTitle ( "LocationTagger 0x" + String.format ( "%X", devID ) );
            }
        });
    }


    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param context		An object reference of type ObserverNotifyContext.
     * valid members are:
     *                      destID = nativeID	The native device id of the sender device.
     *                      notification  		The notification
     *                      sourceIdent			A value of the enumeration Types.EnvironsSource
     *                      contextPtr     		A value that provides additional context information (if available).
     */
    public void OnNotify ( ObserverNotifyContext context ) {
        if ( context.notification == Notify.Mediator.ServerConnected ) {
            UpdateDeviceID ();
        }
    }


    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param context		An object reference of type ObserverNotifyContext.
     * valid members are:
     *                      destID = deviceID	The device id of the sender device.
     *                      areaName			Area name of the application environment
     *                      appName				Application name of the application environment
     *                      notification  		The notification
     *                      sourceIdent			A value of the enumeration Types.EnvironsSource
     *                      contextPtr     		A value that provides additional context information (if available).
     */
    public void OnNotifyExt ( ObserverNotifyContext context ) {
    }
}
