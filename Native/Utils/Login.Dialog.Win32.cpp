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
#include "stdafx.h"

#if (defined(_WIN32))

#include "Login.Dialog.Win32.h"
#include "Interop/Threads.h"
#include "Environs.Native.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "WinCred.h"

#pragma comment ( lib, "Credui.lib" )

#define	CLASS_NAME 	"Login.Dialog . . . . . ."


namespace environs
{

	LONGSYNC LoginDialog::dialogCount = 0;


	/*void CALLBACK LoginDialogTimerFunc ( void * lpParametar, EBOOL TimerOrWaitFired )
	{
		CVerbVerb ( "LoginDialogTimerFunc" );

		LoginDialog * dlg = ( LoginDialog * ) lpParametar;
	}*/


	/**
	* Default constructor of LoginDialog. This class should not be instantiated using the default constructor.
	* Use SingletonInstance to get an instance which makes sure that there is only one instance at any time.
	*
	*/
	LoginDialog::LoginDialog ( int hInst, const char * message_, const char * title_, const char * userName_ )
	{
		hEnvirons = hInst;

		password = "";
		if ( message_ )
			message = message_;
		else
			message = "Enter username and pasword.";

		if ( title_ )
			title = title_;
		else
			title = "Mediator Logon";

		if ( !userName_ )
			userName = "";
		else
			userName = userName_;

		//hAliveTimer = 0;

		/*if ( userName.length() > 3 )
		{
			return;
		}*/
	}

	/**
	* Create an instance of the login dialog with the given parameters.
	* The dialog has to invoked/shown using ShowResult.
	*
	* @param message       The message shown within the dialog.
	* @param title         The title of the dialog.
	* @param userName		The username if already known. This may be null.
	* @return              An instance of the login dialog.
	*/
	LoginDialog * LoginDialog::SingletonInstance ( int hInst, const char * message_, const char * title_, const char * userName_ )
	{
		if ( ___sync_val_compare_and_swap ( &dialogCount, 0, 1 ) > 0 )
			return 0;

		LoginDialog * dialog = new LoginDialog ( hInst, message_, title_, userName_ );
		return dialog;
	}


	/**
	* Show the login dialog within a thread and unblock the calling thread.
	*
	* @return  success on starting the thread
	*/
	bool LoginDialog::ShowResult ()
	{
		pthread_t threadID;

		int ret = pthread_create ( &threadID, NULL, &LoginDialog::ShowResultThreadStarter, ( void * ) this );
		if ( ret != 0 ) {
			CErr ( "ShowResult: Failed to create thread." );

			__sync_sub_and_fetch ( &dialogCount, 1 );
			return false;
		}

		return true;
	}


	void * LoginDialog::ShowResultThreadStarter ( void * arg )
	{
		LoginDialog * dlg = ( LoginDialog * ) arg;

		dlg->ShowResultThreaded ();

		return 0;
	}



	/**
	* The thread instance that shows the login dialog.
	* Start a no activity timer before showing the dialog.
	*
	* @return  returns always true
	*/
	void LoginDialog::ShowResultThreaded ( )
	{
		CREDUI_INFOA cui;
		
		char lUserName [MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1];
		char lPassword [ENVIRONS_USER_PASSWORD_LENGTH + 2];

		RtlSecureZeroMemory ( lUserName, sizeof ( lUserName ) );
		RtlSecureZeroMemory ( lPassword, sizeof ( lPassword ) );

		if ( userName.length () > 0 )
			strlcpy ( lUserName, userName.c_str (), sizeof ( lUserName ) );

		int saveState;
		DWORD dwErr;

		cui.cbSize = sizeof ( CREDUI_INFOA );
		cui.hwndParent = NULL;

		cui.pszMessageText = message.c_str ();
		cui.pszCaptionText = title.c_str ();
		cui.hbmBanner = NULL;
		saveState = FALSE;

		//BOOL success = CreateTimerQueueTimer ( &hAliveTimer, NULL,
		//	( WAITORTIMERCALLBACK ) LoginDialogTimerFunc, ( PVOID )this, 0,
		//	ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT * 1000,
		//	WT_EXECUTEDEFAULT ); // WT_EXECUTEINTIMERTHREAD ); // WT_EXECUTEDEFAULT
		//if ( !success || !pthread_cond_valid ( hAliveTimer ) )
		//{
		//	CErr ( "ShowResultThreaded: Failed to create timer for alive check!" );
		//}

		dwErr = CredUIPromptForCredentialsA (
			&cui, 
			"",     
			NULL,
			0,    
			lUserName,      
			MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1,
			lPassword,        
			ENVIRONS_USER_PASSWORD_LENGTH + 2,
			&saveState,           
			CREDUI_FLAGS_GENERIC_CREDENTIALS | 
			CREDUI_FLAGS_ALWAYS_SHOW_UI |
			CREDUI_FLAGS_DO_NOT_PERSIST );

		if ( !dwErr )
		{
			if ( strlen ( lUserName ) > 0  && strlen ( lPassword ) > 0 )
			{
				environs::API::SetMediatorUserNameN ( hEnvirons, lUserName );
				environs::API::SetMediatorPasswordN ( hEnvirons, lPassword );

				environs::API::RegisterAtMediatorsN ( hEnvirons );
			}
			else
				CErr ( "ShowResultThreaded: Invalid username and password for Mediator entered!" );

			RtlSecureZeroMemory ( lUserName, sizeof ( lUserName ) );
			RtlSecureZeroMemory ( lPassword, sizeof ( lPassword ) );
		}

		//if ( pthread_cond_valid ( hAliveTimer ) )
		//	DeleteTimerQueueTimer ( NULL, hAliveTimer, INVALID_HANDLE_VALUE );

		__sync_sub_and_fetch ( &dialogCount, 1 );
	}

}


#endif