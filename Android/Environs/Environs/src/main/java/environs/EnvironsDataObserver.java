package environs;
/**
 * EnvironsDataObserver
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
*	EnvironsDataObserver
 *	Attachable to **Environs** objects in order to receive all data transmissions that the Environs instance received.
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public interface EnvironsDataObserver {
	/** 
	 * OnData is called whenever new binary data (files, buffers) has been received.
	 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
	 *
	 * @param context		An object reference of type EnvironsMessageObserver.
	 * valid members are:
	 *                      nativeID    The native device id of the sender device.
	 *                      type        The type of the data
	 *                      fileID      A user-customizable id that identifies the file
	 *                      dscriptor	A text that describes the file
	 *                      size        The size in bytes
	 */
	void OnData ( ObserverDataContext context );
}
