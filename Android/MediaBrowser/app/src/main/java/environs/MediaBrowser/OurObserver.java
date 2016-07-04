package environs.MediaBrowser;
/**
 *	Environs listener
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
*	Environs listener
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class OurObserver implements EnvironsObserver, EnvironsMessageObserver, EnvironsDataObserver {

	private static String className = "OurObserver";

	/**
	 * OnStatus is called whenever the framework status changes.&nbsp;
	 *
	 * @param status      A status constant of type Status
	 */
	public void OnStatus ( int status )
	{
		MainTab.updateUI ();
		SettingsTab.updateUI();

		if ( status == Status.Started ) {
			DevicelistTab.InitDeviceList();
		}
	}
	
	static int lastDeviceID = 0;
	static int centerX = 0;
	//static int centerY = 0;
	//static float angle = 90;


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
	public void OnNotify ( ObserverNotifyContext context )
	{
		if ( context.notification != Notify.Options.PortalLocationChanged && context.notification != Notify.Options.PortalSizeChanged ) {
			Utils.Log (4, className, "Notification: nativeID [0x" + Integer.toHexString(context.destID) + "] \tsource ["
					+ context.sourceIdent + "] \tnotification [" + Environs.resolveName(context.notification) + "]");
		}

		int notifyType = Environs.GetNotifyType(context.notification);
		switch (notifyType)
		{
			case Environs.NOTIFY_TYPE_CONNECTION:
				HandleConnectionNotify ( context.destID, context.notification, context.sourceIdent );
				break;
			case Environs.NOTIFY_TYPE_ENVIRONS:
				HandleEnvironsNotify ( context.destID, context.notification );
				break;
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
	public void OnNotifyExt ( ObserverNotifyContext context )
	{
		if ( context.notification != Notify.Portal.LocationChanged && context.notification != Notify.Portal.SizeChanged ) {
			Utils.Log(4, className, "NotificationExt: deviceID [0x" + Integer.toHexString(context.destID) + "] \tsource ["
					+ context.sourceIdent + "] \tnotification [" + Environs.resolveName(context.notification) + "]");
		}
		
		int notifyType = Environs.GetNotifyType(context.notification);
		switch (notifyType)
		{
		case Environs.NOTIFY_TYPE_CONNECTION:
			HandleConnectionNotify(context.destID, context.notification, context.sourceIdent);
			break;
		}
	}


	/**
	 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
	 *
	 * @param msg      The status as a text message.
	 */
	public void OnStatusMessage ( String msg ) {
		MainTab.showEnvironsMessage(msg);
	}


	/**
	 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
	 *
	 * @param context		An object reference of type ObserverMessageContext.
	 * valid members are:
	 *                      destID = nativeID	The native device id of the sender device.
	 *                      sourceType	        Determines the source (either from a device, environs, or native layer)
	 *                      message       		The message as string text
	 *                      length        		The length of the message
	 */
	public void OnMessage ( ObserverMessageContext context ) {
		//Utils.Log("[TRACE] MyEnvironsListener.onMessage: Message (" + msg + ") received!");

		MainTab.showEnvironsMessage(context.message);

		if (context.message.startsWith("T:")) {
			lastDeviceID = context.destID;
		}
		else {
			// Show toast
			Utils.Message ( TabActivity.instance, "Message from " + context.destID + ": " + context.message );
		}
	}


	/**
	 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
	 *
	 * @param context		An object reference of type ObserverMessageContext.
	 * valid members are:
	 *                      destID = deviceID	The device id of the sender device.
	 *                      areaName			Area name of the application environment
	 *                      appName				Application name of the application environment
	 *                      type	        	Determines the source (either from a device, environs, or native layer)
	 *                      message       		The message as string text
	 *                      length        		The length of the message
	 */
	public void OnMessageExt ( ObserverMessageContext context ) {
		//Utils.Log("[TRACE] MyEnvironsListener.onMessage: Message (" + msg + ") received!");

		MainTab.showEnvironsMessage(context.message);

		if (context.message.startsWith("T:")) {
			lastDeviceID = context.destID;
		}
		else {
			// Show toast
			Utils.Message ( TabActivity.instance, "Message from " + context.destID + ": " + context.message );
		}
	}


	/**
	 * OnData is called whenever new binary data (files, buffers) has been received.
	 * Pass deviceID/fileID to Environs.getFile() in order to retrieve a byte array with the content received.
	 *
	 * @param context		An object reference of type EnvironsMessageObserver.
	 * valid members are:
	 *                      nativeID    The native device id of the sender device.
	 *                      type        The type of the data
	 *                      fileID      A user-customizable id that identifies the file
	 *                      dscriptor	A text that describes the file
	 *                      size        The size in bytes
	 */
	public void OnData ( ObserverDataContext context ) {
		Utils.Log(3, className, "OnData from " + context.nativeID + " with fileID " + context.fileID + " and " + context.size + " bytes and desc: " + context.descriptor);

		if (DevicelistTab.adapter == null)
			return;

		DeviceInstance device = DevicelistTab.adapter.GetDevice(context.nativeID);

		if (device != null) {
			String path = device.GetFilePath(context.fileID);
			Utils.Log ( 3, className, "OnData path to file [" + path + "]" );


			byte[] fileData = device.GetFile(context.fileID);
			if (fileData != null)
				Utils.Log(3, className, "OnData loaded buffer with [" + fileData.length + "] bytes.");

		}
	}
	

	@SuppressWarnings ( "all" )
	public void HandleConnectionNotify ( int nativeOrDeviceID, int event, int sourceIdent )
	{
		switch (event)
		{
			case Notify.Connection.Established:
				MainTab.updateUI();
				break;

			case Notify.Connection.Closed:
				MainTab.updateUI();

				break;
		}
	}


	@SuppressWarnings ( "all" )
	public void HandleEnvironsNotify ( int nativeOrDeviceID, int event )
	{
		/*if (event == Notify.Environs.StartSuccess) {
			//Environs.setAdditionalTouchRecognizer ( "libEnv-RecDummy.so" );
		}
		else */
		if (event == Notify.Network.Changed) {
			MainTab.updateUI();
		}

		else if ( event == Notify.Environs.SettingsChanged ) {
			SettingsTab.updateUI();
		}
		/*
        	// if set
        	//Environs.setUseMediatorLoginDialog(false);
        	// otherwise Environs will display a login dialog

        	if ( event == Environs.NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL ||
        			event == Environs.NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING )
        	{
        		/// Create dialog for mediator login credentials
        	}
        	*/
	}
}
