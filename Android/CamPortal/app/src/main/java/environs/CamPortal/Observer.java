package environs.CamPortal;
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
import android.view.MenuItem;

import environs.*;

/**
 * Observer observes notification from Environs.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class Observer implements EnvironsObserver, EnvironsMessageObserver
{
    /**
     * OnStatus is called whenever the framework status changes.&nbsp;
     *
     * @param Environs_STATUS_      A status constant of type STATUS_*
     */
    public void OnStatus ( @Status.Value int Environs_STATUS_ )
    {
        if (DeviceListActivity.menu != null) {
            String menuText = "Start";

            boolean started = false;
            if ( Environs_STATUS_ >= Status.Started ) {
                menuText = "Stop";

                started = true;
            }

            // Get menu item
            final MenuItem item = DeviceListActivity.menu.findItem( environs.CamPortal.R.id.actionEnvStart);
            if (item != null) {
                final String text = menuText;

                final boolean reload = started;

                DeviceListActivity.instance.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        item.setTitle(text);

                        if (reload)
                            DeviceListActivity.adapter.notifyDataSetChanged ();
                    }
                });
            }
        }
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


    /**
     * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
     *
     * @param msg      The status as a text message.
     */
    public void OnStatusMessage ( String msg ) {
        if (DeviceListActivity.instance != null)
        DeviceListActivity.instance.ShowStatusMessage(msg);
    }


    /**
     * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     *
     * @param context		An object reference of type ObserverNotifyContext.
     * valid members are:
     *                      destID = nativeID	The native device id of the sender device.
     *                      sourceType	        Determines the source (either from a device, environs, or native layer)
     *                      message       		The message as string text
     *                      length        		The length of the message
     */
    public void OnMessage ( ObserverMessageContext context ) {

    }


    /**
     * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     *
     * @param context		An object reference of type ObserverNotifyContext.
     * valid members are:
     *                      destID = deviceID	The device id of the sender device.
     *                      areaName			Area name of the application environment
     *                      appName				Application name of the application environment
     *                      type	        	Determines the source (either from a device, environs, or native layer)
     *                      message       		The message as string text
     *                      length        		The length of the message
     */
    public void OnMessageExt ( ObserverMessageContext context ) {

    }
}
