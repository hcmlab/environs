/**
 * FileInstance Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_INTERFACE_H

#include "Interfaces/IEnvirons.Dispose.h"


/**
 *	FileInstance Interface
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
/* Namespace: environs -> */
namespace environs
{
	class DeviceInstance;

	class FileInstance : public lib::IEnvironsDispose
	{
	public:
		FileInstance () { };
		virtual ~FileInstance () { };


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		virtual void Release () = 0;

		/**
		* disposed is true if the object is no longer valid. Nothing will be updated anymore.
		* disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
		* */
		virtual bool disposed () = 0;

		/**
		* An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
		* A value of 0 indicates an invalid fileID.
		* */
		virtual int fileID () = 0;

		/**
		* Used internally.
		* */
		virtual int type () = 0;

		/**
		* A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
		* */
		virtual const char * descriptor () = 0;

		/**
		* sent is true if this FileInstance is data that was sent or received (false).
		* */
		virtual bool sent () = 0;

		/**
		* created is a posix timestamp that determines the time and date that this FileInstance
		* has been received or sent.
		* */
		virtual unsigned long long created () = 0;

		/**
		* The size in bytes of a buffer to send or data received.
		* */
		virtual long size () = 0;

		/**
		* The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
		* */
		virtual const char * path () = 0;

		/**
		* Load data into a binary byte array and returns a pointer to that data
		* */
		virtual unsigned char * data () = 0;

		/**
		* sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
		* file or buffer that has already been sent.
		* If this value changes, then the corresponding device's DeviceObserver is notified
		* with this FileInstance object as the sender
		* and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
		**/
		virtual int sendProgress () = 0;

		/**
		* receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
		* file or buffer that has already been received.
		* If this value changes, then the corresponding device's DeviceObserver is notified
		* with this FileInstance object as the sender
		* and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
		* */
		virtual int receiveProgress () = 0;

		/**
		* Get the DeviceInstance that this FileInstance is attached to.
		* */
		sp ( DeviceInstance ) device ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, deviceRetained () );
		}

		/**
		* Get an Interface to the DeviceInstance that this FileInstance is attached to.
		* */
		virtual DeviceInstance * deviceRetained () = 0;

		virtual const char * toString () = 0;

		virtual const char * GetPath () = 0;
	};
}

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_INTERFACE_H

