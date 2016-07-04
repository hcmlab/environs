/**
 * ChatUser
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
#include "Environs.h"
#include "Interop/Threads.h"

using namespace std;
using namespace environs;

class Observer;


class ChatUser
{
	friend class Observer;

private:

	pthread_mutex_t		messagesLock;

public:
	ChatUser ();
	~ChatUser ();

	char  *		profilePic;

	string		lastStatus;
	string		lastMessage;

	vsp ( MessageInstance ) messages;
	string		userName;

	static bool InitWithDevice ( const sp ( DeviceInstance ) &device );

	const string & GetProfileText ();
	char  * GetProfileImage ();
	static bool IsChatCommand ( const string & msg );

	static const char * ToBase64 ( void * rawData );
	static const char * FromBase64 ( const char * base64 );

	bool Init ();
	sp ( DeviceInstance ) device;

private:
	string		profileText;

	bool		lastStatusRequested;
	bool		profilePicRequested;
	bool		userNameRequested;

	bool		HandleChatCommand ( const string & msg, bool processIncoming );
	void		RequestProfile ();

};

