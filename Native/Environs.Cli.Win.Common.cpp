/**
 * Environs CLI window common code
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

#if ( defined ( CLI_CPP ) && ( defined ( CLI_WIN ) || defined ( CLI_PS ) ) )

#include "Environs.Cli.Forwards.h"
#include "Environs.Types.h.cli.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Cli.Base.h"
#include "Environs.Cli.h"

#include "Message.Instance.h"
#include "File.Instance.h"
#include "Portal.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Environs.Cli.Base.h"
#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Cli.TouchDevice.h"
#include "Environs.Build.Lnk.h"
#include "Environs.Native.h"

#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>

#define CLASS_NAME	"Environs.Cli.Win.Common."


namespace environs
{
	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
	* @param 	appName		The application name for the application environment.
	* @param  	areaName	The area name for the application environment.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler, String ^appName, String ^areaName )
	{
		Environs ^ env = CreateInstance ( clientWindow, handler );
		if ( env != nullptr )
		{
			env->LoadSettings ( appName, areaName );
		}
		return env;
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	appName		The application name for the application environment.
	* @param  	areaName	The area name for the application environment.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, String ^appName, String ^areaName )
	{
		return CreateInstance ( clientWindow, nullptr, appName, areaName );
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler )
	{
		Environs ^ env = gcnew Environs ();

		if ( clientWindow == nullptr )
		{
			CVerb ( "Construct: No client window handle!" );
			return nullptr;
		}

		env->appWindow = clientWindow;

		if ( !env->InitInstance ( handler ) )
			return nullptr;

		return env;
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::CreateInstance ( ENV_CLIENTWINDOW ^clientWindow )
	{
		return CreateInstance ( clientWindow, nullptr );
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
	* @param 	appName		The application name for the application environment.
	* @param  	areaName	The area name for the application environment.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::New ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler, String ^appName, String ^areaName )
	{
		return CreateInstance ( clientWindow, handler, appName, areaName );
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	appName		The application name for the application environment.
	* @param  	areaName	The area name for the application environment.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::New ( ENV_CLIENTWINDOW ^clientWindow, String ^appName, String ^areaName )
	{
		return CreateInstance ( clientWindow, nullptr, appName, areaName );
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::New ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler )
	{
		return CreateInstance ( clientWindow, handler );
	}


	/**
	* Create an Environs object.
	*
	* @param 	clientWindow    A reference to the application window.
	*
	* @return   An Environs object
	*/
	Environs ^ Environs::New ( ENV_CLIENTWINDOW ^clientWindow )
	{
		return CreateInstance ( clientWindow, nullptr );
	}


	ref class EnvironsStartClass
	{
		Environs ^ env;

	public:
		EnvironsStartClass ( Environs ^ e ) : env ( e ) { }
		void Run ()
		{
			if ( env->appWindow == nullptr )
				return;

			// Surface specific event handlers (Environs.S1 / Environs.S2)
			env->AttachEventHandlers ();

			// Attach window handle handler (required for determining the window handle used for portals)
			env->DetermineWindowHandle ( env->appWindow );
		}
	};


	void Environs::StartPlatformHandlers ()
	{
		if ( !initialized ) {
			EnvironsStartClass ^ act = gcnew EnvironsStartClass ( this );

			Action ^ action = gcnew Action ( act, &EnvironsStartClass::Run );

			environs::Environs::dispatchSync ( action );
		}
	}


	// This is called by specific constructors (in common code for Surface 2 and Surface 1
	void Environs::AttachEventHandlers ()
	{
		if ( appWindow == nullptr ) {
			CWarn ( "AttachEventHandlers: No client window!" );
			return;
		}

		// Attach touch/input handler to the base class
#if (!defined(XNA) && !defined(WINDOWS_PHONE))

		appWindow->Loaded += gcnew RoutedEventHandler ( this, &Environs::OnWindowLoaded );

		appWindow->SizeChanged += gcnew SizeChangedEventHandler ( this, &Environs::OnSizeChanged );

		if ( !onCloseAttached ) {
			onCloseAttached = true;

			appWindow->Closing += gcnew System::ComponentModel::CancelEventHandler ( this, &Environs::OnClosing );
		}

		appWindow->PreviewStylusDown += gcnew StylusDownEventHandler ( this, &Environs::AppStylusDown );

		appWindow->PreviewStylusMove += gcnew StylusEventHandler ( this, &Environs::AppStylusMove );

		appWindow->PreviewStylusUp += gcnew StylusEventHandler ( this, &Environs::AppStylusUp );

		appWindow->PreviewKeyDown += gcnew System::Windows::Input::KeyEventHandler ( this, &Environs::AppPreviewKeyDown );
#endif
		AttachSpecificEventHandlers ();
	}


	ref class EnvironsDetachHandlerClass
	{
		Environs ^ env;

	public:
		EnvironsDetachHandlerClass ( Environs ^ e ) : env ( e ) { }
		void Run () { env->DetachEventHandlers ( false ); }
	};


	// This is called by specific destructors (in common code for Surface 2 and Surface 1
	void Environs::DetachEventHandlers ( bool invoke )
	{
		if ( invoke ) {
			EnvironsDetachHandlerClass ^ act = gcnew EnvironsDetachHandlerClass ( this );

			Action ^ action = gcnew Action ( act, &EnvironsDetachHandlerClass::Run );

			environs::Environs::dispatchSync ( action );
			return;
		}

		// Detach touch/input handler to the base class
#if (!defined(XNA) && !defined(WINDOWS_PHONE))

		appWindow->Loaded -= gcnew RoutedEventHandler ( this, &Environs::OnWindowLoaded );

		appWindow->SizeChanged += gcnew SizeChangedEventHandler ( this, &Environs::OnSizeChanged );

		appWindow->PreviewStylusDown -= gcnew StylusDownEventHandler ( this, &Environs::AppStylusDown );

		appWindow->PreviewStylusMove -= gcnew StylusEventHandler ( this, &Environs::AppStylusMove );

		appWindow->PreviewStylusUp -= gcnew StylusEventHandler ( this, &Environs::AppStylusUp );

		appWindow->PreviewKeyDown -= gcnew System::Windows::Input::KeyEventHandler ( this, &Environs::AppPreviewKeyDown );
#endif
		DetachSpecificEventHandlers ();
	}


	ref class DetermineWindowHandleDispatcher
	{
		System::Windows::Window ^ window;
		Environs ^ env;

	public:
		DetermineWindowHandleDispatcher ( Environs ^ e, System::Windows::Window ^ w ) : env ( e ), window ( w ) { }
		void Run ()
		{
			IntPtr hWnd = ( gcnew WindowInteropHelper ( window ) )->Handle;
			if ( hWnd == IntPtr::Zero )
				window->SourceInitialized += gcnew System::EventHandler ( env, &Environs::OnSourceInitialized );
			else
				environs::API::SetMainAppWindowN ( env->hEnvirons, hWnd );
		}
	};


	/**
	* Determine window handle. If not available yet, then delegate it to the sourceinit handler.
	*
	* @param	window		WPF Window handle
	*/
	void Environs::DetermineWindowHandle ( System::Windows::Window ^ window )
	{
		if ( do_PortalFromAppWindow && window != nullptr )
		{
			DetermineWindowHandleDispatcher ^ act = gcnew DetermineWindowHandleDispatcher ( this, window );

			Action ^ action = gcnew Action ( act, &DetermineWindowHandleDispatcher::Run );

			environs::Environs::dispatch ( action );
		}
		else
			environs::API::SetMainAppWindowN ( hEnvirons, IntPtr::Zero );
	}


	/**
	* Handler for invoking the environment init thread. Called when the (client) application window has been loaded, rendered and shown.
	*
	* @param	obj
	* @param	args
	*/
	void Environs::OnWindowLoaded ( Object ^obj, System::Windows::RoutedEventArgs ^args )
	{
		CVerbVerb ( "OnWindowLoaded" );

		if ( windowStartStopEvent != nullptr )
		{
			CVerb ( "OnWindowLoaded set loaded event" );
			windowStartStopEvent->Set ();
		}

#if (!defined(XNA) && !defined(WINDOWS_PHONE))
		appWindow->Loaded -= gcnew System::Windows::RoutedEventHandler ( this, &Environs::OnWindowLoaded );
#endif
	}


	/**
	* Handler for determining the application window handle for Environs. Called when the (client) application window has been loaded and is available.
	*
	* @param	obj
	* @param	args
	*/
	void Environs::OnSourceInitialized ( Object ^ obj, EventArgs ^ args )
	{
		CVerb ( "OnSourceInitialized" );

#if (!defined(XNA) && !defined(WINDOWS_PHONE))

		IntPtr hWnd = ( gcnew System::Windows::Interop::WindowInteropHelper ( appWindow ) )->Handle;
		if ( hWnd == IntPtr::Zero )
			CErr ( "OnSourceInitialized: Failed to determine window handle for Environs!" );
		else
			environs::API::SetMainAppWindowN ( hEnvirons, hWnd );

#ifdef CLI_PS
		EnvironsTouchDevice::Init ( appWindow );
#endif
#endif
	}


	/**
	* Handler for updating the app window size within the native layer.
	*
	* @param	obj
	* @param	args
	*/
	void Environs::OnSizeChanged ( Object ^ obj, System::Windows::SizeChangedEventArgs ^ args )
	{
		CVerb ( "OnSizeChanged" );

		environs::API::UpdateAppWindowSizeN ( hEnvirons );
	}


	ref class CloseInstanceDispatcher
	{
		Environs ^ env;
		System::Threading::ManualResetEvent ^ waitEvent;

	public:
		CloseInstanceDispatcher ( Environs ^ e, System::Threading::ManualResetEvent ^ w ) : env ( e ), waitEvent ( w ) { }
		void Run ()
		{
			if ( env == nullptr )
				return;

			pthread_setname_current_envthread ( "Environs.CloseInstanceDispatcher" );

			env->SetAppShutdown ( true );
			env->async = Call::Wait;
			env->disposeOnClose = false;

			env->Stop ();
			env->DisposeInstance ();
			environs::Environs::instancesAPI [ env->hEnvirons ] = nullptr;

			Action ^ action = gcnew Action ( this, &CloseInstanceDispatcher::Close );
			environs::Environs::dispatchSync ( action );
		}

		void Close ()
		{
			if ( env->appWindow != nullptr ) 
				env->appWindow->Close ();
		}

		void Signal ()
		{
			if ( waitEvent != nullptr )
				waitEvent->Set ();
		}
	};


	/**
	* Handler for disposing environs before closing the app window.
	*
	* @param	obj
	* @param	args
	*/
	void Environs::OnClosing ( Object ^ sender, System::ComponentModel::CancelEventArgs ^ args )
	{
		CVerb ( "OnClosing" );

		if ( disposeOnClose && environs::Environs::instancesAPI [ hEnvirons ] != nullptr ) {
			args->Cancel = true;

			CloseInstanceDispatcher ^ closer = gcnew CloseInstanceDispatcher  ( this, nullptr );

			Thread ^ thread = gcnew Thread ( gcnew ThreadStart ( closer, &CloseInstanceDispatcher::Run ) );
			if ( thread != nullptr )
				thread->Start ();
			else
			{
				CErr ( "OnClosing: Failed to create closer thread!" );
				closer->Run ();
			}
		}
	}


	void Environs::WaitUIFinished ( System::Threading::ManualResetEvent ^ waitEvent )
	{
		CVerb ( "WaitUIFinished" );

		if ( waitEvent == nullptr )
			return;

		pthread_setname_current_envthread ( "Environs.WaitUIFinished" );

		waitEvent->Reset ();

		CloseInstanceDispatcher ^ uiSignal = gcnew CloseInstanceDispatcher ( nullptr, waitEvent );
		if ( uiSignal == nullptr )
			return;

		Action ^ action = gcnew Action ( uiSignal, &CloseInstanceDispatcher::Signal );
		if ( !environs::Environs::dispatch ( action ) )
			return;

		waitEvent->WaitOne ( 30000 );
	}


#if (!defined(WINDOWS_PHONE))
	void Environs::AppStylusDown ( Object ^ sender, StylusDownEventArgs ^ e )
	{
		CVerbVerb ( "AppStylusDown" );
		e->Handled = true;
	}

	void Environs::AppStylusMove ( Object ^ sender, StylusEventArgs ^ e )
	{
		CVerbVerb ( "AppStylusMove" );
		e->Handled = true;
	}

	void Environs::AppStylusUp ( Object ^ sender, StylusEventArgs ^ e )
	{
		CVerbVerb ( "AppStylusUp" );
		e->Handled = true;
	}
#endif


	/**
	* Instruct Environs to switch the app window to fullscreen after successful initialization and Start of Environs.
	*
	* @param	value
	*/
	void Environs::SetUseFullscreen ( bool value )
	{
		do_FullScreen = value;
	}


	/**
	* This is an Environs option to use the app window for portal streams or not. This must be set before starting Environs.
	* Not using the app window automatically uses the desktop window for portal streams.
	*
	* @param	value
	*/
	void Environs::SetPortalFromAppWindow ( bool value )
	{
		do_PortalFromAppWindow = value;
	}


}


#endif