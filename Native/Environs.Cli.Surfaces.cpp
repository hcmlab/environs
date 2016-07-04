/**
* Environs CLI Tabletop surface part
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

#if (defined(CLI_CPP) && (defined(CLI_STT)  || defined(CLI_PS)))


#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Native.h"

#define CLASS_NAME	"Environs.Cli.Surfaces. ."


namespace environs
{
	void InvokeNetworkNotifier ( int hInst, bool enable )
	{

	}

	void EnvironsPlatformInit ( int hInst )
	{

	}

	bool Environs::InitPlatformLayer ()
	{
		// Load platform specific requirements

		int platform = environs::API::GetPlatformN ();
		int pfTest = ( int ) Platforms::MultiTaction55;

		if ( ( platform & pfTest ) == pfTest )
		{
			//Environs.SetUseTracker(Environs.CALL_ASYNC, "libEnv-TrackMTFi");

			/// Enable a tuio tracker module. Environs does not need to feed this module with images
			environs::API::SetUseTrackerN ( hEnvirons, CALL_NOWAIT, "libEnv-TrackTUIO" );

			// Let's handle device markers in native layer
			environs::API::SetUseDeviceMarkerAutomaticN ( false );
			environs::API::SetDeviceMarkerReducedPrecisionN ( true );
		}
		//Environs.SetUseTracker("libEnv-TrackMTFi.dll");

		return InitSurfacePlatformLayer ();
	}


	void Environs::ReleasePlatformLayer ()
	{
	}



	// This is called by EnvironsInstance constructor (located in Common.cs code for specific event handler initialization)
	void Environs::AttachSpecificEventHandlers ()
	{
		// Attach touch/input handler to the base class
		appWindow->PreviewTouchDown += gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchDown );
		appWindow->PreviewTouchMove += gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchMove );
		appWindow->PreviewTouchUp += gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchUp );

		appWindow->PreviewMouseDown += gcnew MouseButtonEventHandler ( this, &Environs::appMouseDown );
		appWindow->PreviewMouseMove += gcnew System::Windows::Input::MouseEventHandler ( this, &Environs::appMouseMove );
		appWindow->PreviewMouseUp += gcnew MouseButtonEventHandler ( this, &Environs::appMouseUp );

		appWindow->PreviewMouseLeftButtonDown += gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseDown );
		appWindow->PreviewMouseLeftButtonUp += gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseUp ); 
		appWindow->PreviewMouseRightButtonDown += gcnew MouseButtonEventHandler ( this, &Environs::appRightMouseDown ); 

		/*appWindow->MouseLeftButtonDown += gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseDown );
		appWindow->MouseLeftButtonUp += gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseUp );
		appWindow->MouseRightButtonDown += gcnew MouseButtonEventHandler ( this, &Environs::appRightMouseDown );*/
	}


	// This is called by EnvironsInstance constructor (located in Common.cs code for specific event handler initialization)
	void Environs::DetachSpecificEventHandlers ()
	{
		// Attach touch/input handler to the base class
		appWindow->PreviewTouchDown -= gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchDown );
		appWindow->PreviewTouchMove -= gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchMove );
		appWindow->PreviewTouchUp -= gcnew System::EventHandler <TouchEventArgs ^> ( this, &Environs::appTouchUp );

		appWindow->PreviewMouseDown -= gcnew MouseButtonEventHandler ( this, &Environs::appMouseDown );
		appWindow->PreviewMouseMove -= gcnew System::Windows::Input::MouseEventHandler ( this, &Environs::appMouseMove );
		appWindow->PreviewMouseUp -= gcnew MouseButtonEventHandler ( this, &Environs::appMouseUp );

		appWindow->PreviewMouseLeftButtonDown -= gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseDown );
		appWindow->PreviewMouseLeftButtonUp -= gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseUp ); 
		appWindow->PreviewMouseRightButtonDown -= gcnew MouseButtonEventHandler ( this, &Environs::appRightMouseDown ); 

		/*appWindow->MouseLeftButtonDown -= gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseDown );
		appWindow->MouseLeftButtonUp -= gcnew MouseButtonEventHandler ( this, &Environs::appLeftMouseUp );
		appWindow->MouseRightButtonDown -= gcnew MouseButtonEventHandler ( this, &Environs::appRightMouseDown );*/
	}


	void Environs::itemRightMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if CLI_PS
		//Debug.WriteLine("itemRightMouseUp");
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}
#endif
		if ( e->LeftButton != MouseButtonState::Pressed )
			return;

		return;
	}


	void Environs::appMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if CLI_PS
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}
#endif
	}


	void Environs::appLeftMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if (defined(CLI_PS) && !defined(XNA))
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}

		//FilterTagUp.down(1);

		//if (longTapHandler != null)
		//{
		//    /// Create a counter animation and Start it here
		//    /// Set user data with animation object
		//    leftMouseIDCounter++;
		//    e.Device.SetUserData("id", leftMouseIDCounter);

		//    LongTap.tapDown(sender, leftMouseIDCounter, e.GetPosition(appWindow), 0.0, e);
		//}
#endif
	}


	void Environs::appLeftMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if (defined(CLI_PS) && !defined(XNA))
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}

		//FilterTagUp.up(1, e.GetPosition(appWindow), 90);

		//if (longTapHandler != null)
		//{
		//    object oid = e.Device.GetUserData("id");
		//    if (oid != null)
		//        LongTap.tapUp(sender, (uint)oid, e.GetPosition(appWindow), 0.0, e);
		//}
#endif
	}


	void Environs::appRightMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if CLI_PS
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}
#endif
#if XNA
#else
		if ( e->RightButton == MouseButtonState::Pressed )
		{
			System::Windows::Point ^ pt = e->MouseDevice->GetPosition ( appWindow );

			if ( e->LeftButton == MouseButtonState::Pressed )
				appHandleRightClickBothButtons ( (int)pt->X, (int)pt->Y );
			else
			{
				if ( !appHandleRightClick ( (int)pt->X, (int)pt->Y ) )
					return;
			}

			e->Handled = true;
			return;
		}
#endif
	}

	/// <summary>
	/// Handles mouse klicks when only the right mouse button is pressed for debug deviceID
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	bool Environs::appHandleRightClick ( int x, int y )
	{
		if ( x < 130 || y < 240 )
			return false;

		if ( (DeviceStatus) environs::API::GetDeviceConnectStatusN ( hEnvirons, debug_device_tagID ) == DeviceStatus::Connected)
		{
			// If we're connected
			if ( Keyboard::Modifiers == ModifierKeys::Shift )
				// and shift key is pressed, then disconnect the debug deviceID
				deviceHandler->remove ( debug_device_tagID );
			else
				// if shift key is NOT pressed, then update the position
				deviceHandler->moved ( debug_device_tagID, x, y, 90 );
		}
		else
			// If we're NOT connected, then connect the debug deviceID
			deviceHandler->detected ( debug_device_tagID, x, y, 90 );

		return true;
	}


	/// <summary>
	/// Handles mouse klicks when both mouse buttons are pressed for debug deviceID
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	bool Environs::appHandleRightClickBothButtons ( int x, int y )
	{
		if ( x < 130 || y < 240 )
			return false;

		// If we're connected, then lift off the device from surface
		bool onSurface = environs::API::GetDirectContactStatusN ( hEnvirons, debug_device_tagID );

		deviceHandler->vanished ( debug_device_tagID, x, y, 90 );
		//if (onSurface)
		//else
		//    onDeviceDetected(debug_device_tagID, x, y, 90);

		return true;
	}


	void Environs::appMouseMove ( Object ^ sender, System::Windows::Input::MouseEventArgs ^ e )
	{
#if CLI_PS
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}

#if !XNA
		//if (longTapHandler != null)
		//{
		//    object oid = e.Device.GetUserData("id");
		//    if (oid != null)
		//        LongTap.tapMove((uint)oid, e.GetPosition(appWindow), 0.0);
		//}
#endif
#endif
	}

	void Environs::appMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
#if CLI_PS
		CVerbVerb ( "appMouseUp" );
		if ( isSurface )
		{
			// filter out accidental mouse ghosts on real surface device
			e->Handled = true;
			return;
		}
#endif
	}

	namespace API
	{
		bool RenderSurfaceCallback ( int type, void * surface, void * decoderOrByteBuffer )
		{
			return false;
		}
	}


}


#endif