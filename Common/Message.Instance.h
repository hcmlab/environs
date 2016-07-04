/**
 * MessageInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Environs.Platform.Decls.h"

#ifndef CLI_CPP
#	include "Interfaces/IMessage.Instance.h"
#endif

#include <string>

/**
 *	MessageInstance
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

		PUBLIC_CLASS MessageInstance DERIVE_c_only ( environs::MessageInstance ) DERIVE_DISPOSABLE
        {
            MAKE_FRIEND_CLASS ( DeviceList );
            MAKE_FRIEND_CLASS ( DeviceInstance );
            MAKE_FRIEND_CLASS ( MessageInstanceProxy );
            MAKE_FRIEND_CLASS ( Environs );

		public:
			ENVIRONS_LIB_API MessageInstance ();
			ENVIRONS_LIB_API ~MessageInstance ();

			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			ENVIRONS_LIB_API bool disposed ();

			/**
			 * sent is true if this MessageInstance is data that was sent or received (false).
			 * */
			ENVIRONS_LIB_API bool sent ();

			/**
			 * created is a posix timestamp that determines the time and date that this MessageInstance
			 * has been received or sent.
			 * */
			ENVIRONS_LIB_API unsigned long long created ();

			/**
			 * The length of the text message in bytes (characters).
			 * */
			ENVIRONS_LIB_API int length ();

			/**
			 * The text message.
			 * */
			ENVIRONS_LIB_API CString_ptr text ();

			/**
			 * Determins the type of connection (channel type) used to exchange this message.
			 * c = in connected state
			 * d = in not connected state through a direct device to device channel.
			 * m = in not connected state by means of a Mediator service.
			 * */
			ENVIRONS_LIB_API char connection ();

			/**
			 * A reference to the DeviceInstance that is responsible for this FileInstance.
			 * */
			DeviceInstanceESP device_;

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstancePtr deviceRetained ();
#endif
			ENVIRONS_LIB_API CString_ptr toString ();

			ENVIRONS_LIB_API CString_ptr shortText ();

			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

#ifndef NDEBUG
        public:
#else
        INTERNAL:
#endif
			ENVIRONS_OUTPUT_ALLOC_WP_RESOURCE ( MessageInstance );
        
#ifndef NDEBUG
        INTERNAL:
#endif
			void            DisposeInstance ();

			void            PlatformDispose ();

			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			LONGSYNC		disposed_;

			STRING_T		toString_;

#ifdef CLI_CPP
			String_ptr		textShortEnd;
#else
			char            textShortEnd;
#endif
			char            connection_;
			String_ptr      text_;
			int             length_;
			bool            sent_;

			unsigned long long created_;

			static MessageInstanceESP Create ( char * line, int length, c_const DeviceInstanceESP c_ref device );

			static bool HasPrefix ( char * line, int length );

		};
	}

}

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H




