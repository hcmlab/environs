package environs.ChatApp.Watch;
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
     * @param Environs_STATUS_      A status constant of type STATUS_*
     */
    public void OnStatus ( int Environs_STATUS_ )
    {
        if (ChatActivity.menu != null) {
            String menuText = "Start";

            if (Environs_STATUS_ >= Environs.STATUS_STARTED) {
                menuText = "Stop";
            }

            // Get menu item
            /*final MenuItem item = ChatActivity.menu.findItem(R.id.actionEnvStart);
            if (item != null) {
                final String text = menuText;

                ChatActivity.instance.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        item.setTitle(text);
                    }
                });
            }
            */
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

}
