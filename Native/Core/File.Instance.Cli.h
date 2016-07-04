/**
 * FileInstance CLI
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_CLI_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_CLI_H

#ifdef CLI_CPP

#include "File.Instance.h"


/**
 *	FileInstance CLI
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	public ref class FileInstance : public environs::lib::FileInstance
	{
	public:
		FileInstance ();
		~FileInstance ();

		/**
		* disposed is true if the object is no longer valid. Nothing will be updated anymore.
		* disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
		* */
		property bool disposed { bool get () { return ( disposed_ == 1 ); } };

		/**
		* An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
		* A value of 0 indicates an invalid fileID.
		* */
		property int fileID { int get () { return fileID_; } };

		/**
		* Used internally.
		* */
		property int type { int get () { return type_; } };

		/**
		* A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
		* */
		property String ^ descriptor { String ^ get () { return descriptor_; } };

		/**
		* sent is true if this FileInstance is data that was sent or received (false).
		* */
		property bool sent { bool get () { return sent_; } };

		/**
		* created is a posix timestamp that determines the time and date that this FileInstance
		* has been received or sent.
		* */
		property unsigned long long created { unsigned long long get () { return created_; } };

		/**
		* The size in bytes of a buffer to send or data received.
		* */
		property long size { long get () { return size_; } };

		/**
		* The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
		* */
		property String ^ path { String ^ get () { return path_; } };

		/**
		* Load data into a binary byte array and returns a pointer to that data.
		* */
		property UCharArray_ptr data { UCharArray_ptr get () { return lib::FileInstance::data(); } };

		/**
		* sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
		* file or buffer that has already been sent.
		* If this value changes, then the corresponding device's DeviceObserver is notified
		* with this FileInstance object as the sender
		* and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
		* */
		property int sendProgress { int get () { return sendProgress_; } };

		/**
		* receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
		* file or buffer that has already been received.
		* If this value changes, then the corresponding device's DeviceObserver is notified
		* with this FileInstance object as the sender
		* and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
		* */
		property int receiveProgress { int get () { return receiveProgress_; } };

		/**
		* Get the DeviceInstance that this FileInstance is attached to.
		*
		* */
		property environs::DeviceInstance ^ device { environs::DeviceInstance ^ get () { return device_; } };

		virtual String ^ ToString () override;
	};
}

#endif // CLI_CPP

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_CLI_H


