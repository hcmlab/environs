/**
 * Environs CPP Login Dialog Win32
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_LOGIN_DIALOG_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_LOGIN_DIALOG_H

#if (defined(_WIN32))

#include "Environs.h"
#include <string>

namespace environs 
{
	/**
	*	Login Dialog Win32
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	class LoginDialog
	{
	public:
		/**
		* Create an instance of the login dialog with the given parameters.
		* The dialog has to invoked/shown using ShowResult.
		*
		* @param message       The message shown within the dialog.
		* @param title         The title of the dialog.
		* @param userName		The username if already known. This may be null.
		* @return              An instance of the login dialog.
		*/
		static LoginDialog * SingletonInstance ( int hInst, const char * message, const char * title, const char * userName );

		/**
		* Default constructor of LoginDialog. This class should not be instantiated using the default constructor.
		* Use SingletonInstance to get an instance which makes sure that there is only one instance at any time.
		*
		* @param message       The message shown within the dialog.
		* @param title         The title of the dialog.
		* @param userName		The username if already known. This may be null.
		*/
		LoginDialog ( int hInst, const char * message, const char * title, const char * userName = 0 );


		/**
		* Show the login dialog within a thread and unblock the calling thread.
		*
		* @return  returns always true
		*/
		bool ShowResult ();

	private:
		std::string userName;
		std::string password;
		std::string message;
		std::string title;

		int hEnvirons;

		//pthread_cond_t	hAliveTimer;

		static LONGSYNC dialogCount;


		/**
		* The thread instance that shows the login dialog.
		* Start a no activity timer before showing the dialog.
		*
		* @return  returns always true
		*/
		void ShowResultThreaded ();
		static void * ShowResultThreadStarter ( void * arg );
	};
}


#endif

#endif