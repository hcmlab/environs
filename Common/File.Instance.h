/**
 * FileInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Environs.Platform.Decls.h"

#ifndef CLI_CPP
#	include "Interfaces/IFile.Instance.h"
#endif

#include <string>

/**
 *	FileInstance CPP Object
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
/* Namespace: environs -> */
namespace environs
{
	namespace lib
    {
#ifndef CLI_CPP
        class DeviceInstance;
#endif

		PUBLIC_CLASS FileInstance DERIVE_c_only ( environs::FileInstance ) DERIVE_DISPOSABLE
		{
            MAKE_FRIEND_CLASS ( DeviceList );
            MAKE_FRIEND_CLASS ( DeviceInstance );
            MAKE_FRIEND_CLASS ( FileInstanceProxy );
            MAKE_FRIEND_CLASS ( Environs );

		public:
			ENVIRONS_LIB_API FileInstance ();
			ENVIRONS_LIB_API ~FileInstance ();

			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			ENVIRONS_LIB_API bool disposed ();

			/**
			 * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
			 * A value of 0 indicates an invalid fileID.
			 * */
			ENVIRONS_LIB_API int fileID ();

			/**
			 * Used internally.
			 * */
			ENVIRONS_LIB_API int type ();

			/**
			 * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
			 * */
			ENVIRONS_LIB_API CString_ptr descriptor ();

			/**
			 * sent is true if this FileInstance is data that was sent or received (false).
			 * */
			ENVIRONS_LIB_API bool sent ();

			/**
			 * created is a posix timestamp that determines the time and date that this FileInstance
			 * has been received or sent.
			 * */
			ENVIRONS_LIB_API unsigned long long created ();

			/**
			 * The size in bytes of a buffer to send or data received.
			 * */
			ENVIRONS_LIB_API long size ();

			/**
			 * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
			 * */
			ENVIRONS_LIB_API CString_ptr path ();

			/**
			* Load data into a binary byte array and returns a pointer to that data.
			* */
			ENVIRONS_LIB_API UCharArray_ptr data ();

			/**
			 * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
			 * file or buffer that has already been sent.
			 * If this value changes, then the corresponding device's DeviceObserver is notified
			 * with this FileInstance object as the sender
			 * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
			 * */
			ENVIRONS_LIB_API int sendProgress ();

			/**
			 * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
			 * file or buffer that has already been received.
			 * If this value changes, then the corresponding device's DeviceObserver is notified
			 * with this FileInstance object as the sender
			 * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
			 * */
			ENVIRONS_LIB_API int receiveProgress ();

			/**
			 * Get the DeviceInstance that this FileInstance is attached to.
			 *
			 * */
#ifdef CLI_CPP
			ENVIRONS_LIB_API DeviceInstanceEPtr device ();
#else
			ENVIRONS_LIB_API environs::DeviceInstancePtr deviceRetained ();
#endif


			ENVIRONS_LIB_API CString_ptr toString ();

			ENVIRONS_LIB_API CString_ptr GetPath ();
        
        
#if ( defined(ENVIRONS_OSX) || defined(ENVIRONS_IOS) )
            void                    *   platformKeep;
#endif

			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		INTERNAL:

			ENVIRONS_OUTPUT_ALLOC_WP_RESOURCE ( FileInstance );

			void DisposeInstance ();

			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			LONGSYNC		disposed_;

			void            PlatformDispose ();

			/**
			* An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
			* A value of 0 indicates an invalid fileID.
			* */
			int             fileID_;

			/**
			* Used internally.
			* */
			int             type_;

			/**
			* A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
			* */
			STRING_T		descriptor_;

			/**
			* sent is true if this FileInstance is data that was sent or received (false).
			* */
			bool            sent_;

			/**
			* created is a posix timestamp that determines the time and date that this FileInstance
			* has been received or sent.
			* */
			unsigned long long created_;

			/**
			* The size in bytes of a buffer to send or data received.
			* */
			long            size_;

			/**
			* The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
			* */
			STRING_T		path_;

			/**
			* sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
			* file or buffer that has already been sent.
			* If this value changes, then the corresponding device's DeviceObserver is notified
			* with this FileInstance object as the sender
			* and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
			* */
			int             sendProgress_;

			/**
			* receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
			* file or buffer that has already been received.
			* If this value changes, then the corresponding device's DeviceObserver is notified
			* with this FileInstance object as the sender
			* and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
			* */
			int             receiveProgress_;

			/**
			* A reference to the DeviceInstance that is responsible for this FileInstance.
			* */
			DeviceInstanceESP device_;

			/**
			 * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
			 * */
			STRING_T		toString_;

			static FileInstanceESP Create ( c_const DeviceInstanceESP c_ref device, int fileID, String_ptr fullPath, size_t length );
		};

	}
}

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H

