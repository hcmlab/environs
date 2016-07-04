package environs;
/**
 * EnvironsMessageObserver
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


/**
*	EnvironsMessageObserver
 *  Attachable to **Environs** objects in order to receive all messages that the Environs instance received.
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public interface EnvironsMessageObserver {
	/**
	 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
	 *
	 * @param msg      The status as a text message.
	 */
	void OnStatusMessage ( String msg );


	/**
	 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
	 *
	 * @param context		An object reference of type EnvironsMessageObserver.
	 * valid members are:
	 *                      destID = nativeID	The native device id of the sender device.
	 *                      sourceType	        Determines the source (either from a device, environs, or native layer)
	 *                      message       		The message as string text
	 *                      length        		The length of the message
	 */
	void OnMessage ( ObserverMessageContext context );


		/**
		 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
		 *
		 * @param context		An object reference of type EnvironsMessageObserver.
		 * valid members are:
		 *                      destID = deviceID	The device id of the sender device.
		 *                      areaName			Area name of the application environment
		 *                      appName				Application name of the application environment
		 *                      type	        	Determines the source (either from a device, environs, or native layer)
		 *                      message       		The message as string text
		 *                      length        		The length of the message
		 */
	void OnMessageExt ( ObserverMessageContext context );
}
