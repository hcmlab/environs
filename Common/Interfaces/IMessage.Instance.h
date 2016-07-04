/**
 * MessageInstance Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_INTERFACE_H

#include "Interfaces/IEnvirons.Dispose.h"


/**
 *	MessageInstance Interface
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


	class MessageInstance : public lib::IEnvironsDispose
	{

	public:
		MessageInstance () { };
		~MessageInstance () { };


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
		* sent is true if this MessageInstance is data that was sent or received (false).
		* */
		virtual bool sent () = 0;

		/**
		* created is a posix timestamp that determines the time and date that this MessageInstance
		* has been received or sent.
		* */
		virtual unsigned long long created () = 0;

		/**
		* The length of the text message in bytes (characters).
		* */
		virtual int length () = 0;

		/**
		* The text message.
		* */
		virtual const char * text () = 0;

		/**
		* Determins the type of connection (channel type) used to exchange this message.
		* c = in connected state
		* d = in not connected state through a direct device to device channel.
		* m = in not connected state by means of a Mediator service.
		* */
		virtual char connection () = 0;

		/**
		* Get the DeviceInstance that this MessageInstance is attached to.
		* */
		sp ( DeviceInstance ) device ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, deviceRetained () );
		}

		/**
		* Get an Interface to the DeviceInstance that this MessageInstance is attached to.
		* */
		virtual DeviceInstance * deviceRetained () = 0;


		virtual const char * toString () = 0;
		virtual const char * shortText () = 0;

	};
}

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_INTERFACE_H




