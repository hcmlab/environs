/**
 * MessageInstance CLI
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_CLI_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_CLI_H

#ifdef _WIN32

#include "Message.Instance.h"


/**
 *	MessageInstance CLI
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	public ref class MessageInstance : public environs::lib::MessageInstance
	{
	public:
		MessageInstance ();
		~MessageInstance ();

		/**
		* disposed is true if the object is no longer valid. Nothing will be updated anymore.
		* disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
		* */
		property bool disposed { bool get () { return (disposed_ == 1); } };

		/**
		* sent is true if this MessageInstance is data that was sent or received (false).
		* */
		property bool sent { bool get () { return sent_; } };

		/**
		* created is a posix timestamp that determines the time and date that this MessageInstance
		* has been received or sent.
		* */
		property unsigned long long created { unsigned long long get () { return created_; } };

		/**
		* The length of the text message in bytes (characters).
		* */
		property int length { int get () { return length_; } };

		/**
		* The text message.
		* */
		property String ^ text { String ^ get () { return text_; } };

		/**
		* Determins the type of connection (channel type) used to exchange this message.
		* c = in connected state
		* d = in not connected state through a direct device to device channel.
		* m = in not connected state by means of a Mediator service.
		* */
		property char connection { char get () { return connection_; } };

		/**
		* A reference to the DeviceInstance that is responsible for this FileInstance.
		* */
		property environs::DeviceInstance ^ device { environs::DeviceInstance ^ get () { return device_; } };

		virtual String ^ ToString () override;

		property String ^ shortText { String ^ get () { return lib::MessageInstance::shortText(); } };
	};
}

#endif // _WIN32

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_CLI_H


