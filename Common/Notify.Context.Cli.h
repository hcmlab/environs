/**
* Notify Context CLI Declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_CLI_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_CLI_DECLARATIONS_H_

#ifdef CLI_CPP
#	include "Interop.h"
#	include "Environs.Types.h.Cli.h"

	using namespace System;

namespace environs
{

	public ref class ObserverNotifyContext
	{
	public:
		/** The native/device identifier that targets the device */
		OBJIDType		destID;
		/** Area name of the application environment */
		CString_ptr		areaName;
		/** Area name of the application environment */
		CString_ptr		appName;
		/**  */
		int				notification;
		/**  */
		int				sourceIdent;
		/**  */
		Addr_ptr		contextPtr;
		/**  */
		int				context;
		/**  */
		int				hEnvirons;
	};


	public ref class ObserverMessageContext
	{
	public:
		/** The native/device identifier that targets the device */
		OBJIDType		destID;
		/** Area name of the application environment */
		CString_ptr		areaName;
		/** Area name of the application environment */
		CString_ptr		appName;
		/** Determines the source ( either from a device, environs, or native layer ) */
		int				sourceType;
		/** The message as string text */
		CVString_ptr	message;
		/** The length of the message */
		int             length;
		/** A connection type */
		char            connection;
	};


	public ref class ObserverDataContext
	{
	public:
		/** The object id of the sender device. */
		OBJIDType		objID;
		/** The native device id of the sender device. */
		int				nativeID;

		/** The type of this message. */
		int             type;
		/** A fileID that was attached to the buffer. */
		int             fileID;
		/** A descriptor that was attached to the buffer. */
		CString_ptr		descriptor;
		/** The size of the data buffer. */
		int             size;
	};
}

#endif

#endif /* INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_ */
