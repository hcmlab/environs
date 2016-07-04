/**
* Notify Context Declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_

#include "Interop.h"

#ifndef CLI_CPP

#	ifdef __cplusplus

namespace environs
{
	namespace lib
    {
#	endif
		typedef struct ObserverNotifyContext
		{
			/** The native/device identifier that targets the device */
			OBJIDType		destID;
			/** Area name of the application environment */
			const char *	areaName;
			/** Area name of the application environment */
			const char *	appName;
			/** The notification */
			int				notification;
			/** A value of the enumeration Types.EnvironsSource */
			int				sourceIdent;
			/** A value that provides additional context information (if available). */
            void *			contextPtr;
            /**  */
            int				context;
			/**  */
			int				hEnvirons;
		}
		ObserverNotifyContext;


		typedef struct ObserverMessageContext
		{
			/** The native/device identifier that targets the device */
			OBJIDType		destID;
			/** Area name of the application environment */
			const char *    areaName;
			/** Area name of the application environment */
			const char *    appName;
			/** Determines the source ( either from a device, environs, or native layer ) */
			int             sourceType;
			/** The message as string text */
			const char *    message;
			/** The length of the message */
			int             length;
			/** A connection type */
			char            connection;
		}
		ObserverMessageContext;

		typedef struct ObserverDataContext
		{
			/** The native device id of the sender device. */
			OBJIDType		objID;
			/** The native device id of the sender device. */
			int				nativeID;
			/** The type of this message. */
			int             type;
			/** A fileID that was attached to the buffer. */
			int             fileID;
			/** A descriptor that was attached to the buffer. */
			const char *	descriptor;
			/** The size of the data buffer. */
			int             size;
		}
        ObserverDataContext;
        
#	ifdef __cplusplus
	}


		///
		/// We map these to those in lib (non cli)
		///
		typedef lib::ObserverNotifyContext  ObserverNotifyContext;

#		define ObserverNotifyContextPtr    ObserverNotifyContext  OBJ_ptr;

		typedef lib::ObserverMessageContext  ObserverMessageContext;

#		define ObserverMessageContextPtr    ObserverMessageContext  OBJ_ptr;

		typedef lib::ObserverDataContext  ObserverDataContext;

#		define ObserverDataContextPtr    ObserverDataContext  OBJ_ptr;

}

	///
	/// We need these typedefs for plain objective c
	///
	typedef environs::lib::ObserverNotifyContext  ObserverNotifyContext;

	typedef environs::lib::ObserverMessageContext  ObserverMessageContext;

	typedef environs::lib::ObserverDataContext  ObserverDataContext;

#	endif

#endif

#endif /* INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_ */
