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

#if (defined(CLI_CPP) && defined(CLI_PS))

#include "Environs.Types.h.cli.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Cli.h"
#include "Message.Instance.h"
#include "File.Instance.h"
#include "Portal.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Interop/Stat.h"
#include "Environs.h"
#include "Environs.Cli.TouchDevice.h"

#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace Microsoft::Surface::Presentation::Input;

namespace environs
{

	bool Environs::InitSurfacePlatformLayer ()
	{
		EnvironsTouchDevice::Init ( appWindow );

		latencyIndicator = gcnew LatencyIndicator ();

		deviceHandler = gcnew DeviceHandler ( this );

		applicationEnvironments	= gcnew System::Collections::Generic::Dictionary<int, ApplicationEnvironment ^ > ();
				
		// Detect whether we are running on a real PixelSense device or not
		int platform = environs::API::GetPlatformN ();

		int pixelSense = ( int ) Platforms::SAMSUR40;

		if ( ( platform & pixelSense ) == pixelSense ) {
			environs::API::SetUseDeviceMarkerAutomaticN ( false );
			environs::API::SetDeviceMarkerReducedPrecisionN ( false );
			isSurface = true;
		}
		else {
			// Not a PixelSense device, so make use of the more efficient native handling of device marker
			environs::API::SetUseDeviceMarkerAutomaticN ( true );
			environs::API::SetDeviceMarkerReducedPrecisionN ( true );

			// In order to disable native marker handling
			// Set both options to false and isSurface to true
		}
		//InitRawImageCapture ( new WindowInteropHelper ( appWindow ).Handle );

		return true;
	}

	void Environs::InitLatencyIndicator ( System::Windows::Controls::TextBlock ^ tb )
	{
		latencyIndicator->Init ( tb );
		latencyIndicator->Start ();
	}



	ref class EnvironsInjectTouch
	{
		InputPack ^ pack;
		int nativeID;

	public:
		EnvironsInjectTouch ( int n, InputPack ^ p ) : nativeID ( n ), pack ( p ) {}
		void Run () {
			switch ( pack->state )
			{
			case INPUT_STATE_ADD: // touch down from client id on x/y
				EnvironsTouchDevice::ContactDown ( nativeID, pack );
				break;
			case INPUT_STATE_CHANGE: // touch moved from client id to x/y
				EnvironsTouchDevice::ContactChanged ( nativeID, pack );
				break;
			case INPUT_STATE_DROP: // touch up from client id on x/y
				EnvironsTouchDevice::ContactUp ( nativeID, pack );
				break;
			}
		}
	};


	/// <summary>
	/// A static method of the surface specific handling of injection of contact down events.
	/// </summary>
	/// <param name="id"></param>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void Environs::InjectTouch ( int nativeID, InputPack ^ pack )
	{
		EnvironsInjectTouch ^ act = gcnew EnvironsInjectTouch ( nativeID, pack );

		Action ^ action = gcnew Action ( act, &EnvironsInjectTouch::Run );

		environs::Environs::dispatch ( action );
	}


	void Environs::appTouchDown ( Object ^ sender, TouchEventArgs ^ e )
	{
		TouchDevice ^ c = e->TouchDevice;
		//if (c->GetType() == typeof(EnvironsTouchDevice))
		//    return;

		//Debug.WriteLine("appTouchDown");
		
		if ( EnvironsTouchExtensions::GetIsTagRecognized ( c ) )
		{
			if ( isSurface && updateDeviceTagPosition )
			{
				System::Windows::Point ^ pt = EnvironsTouchExtensions::GetPosition ( c, appWindow );
				float orientation = (float)EnvironsTouchExtensions::GetOrientation ( c, appWindow );
				int deviceID = (int)EnvironsTouchExtensions::GetTagData ( c )->Value;

				//FilterTagUp.down(deviceID);
				deviceHandler->detected ( deviceID, (int)pt->X, (int)pt->Y, orientation );
			}

			e->Handled = true;
			return;
		}
		else if ( EnvironsTouchExtensions::GetIsFingerRecognized ( c ) )
		{
			return;
		}
	}


	void Environs::appTouchMove ( Object ^ sender, TouchEventArgs ^ e )
	{
		TouchDevice ^ c = e->TouchDevice;

		// Debug.WriteLine("appTouchMove");

		if ( EnvironsTouchExtensions::GetIsTagRecognized ( c ) )
		{
			if (  isSurface && updateDeviceTagPosition )
			{
				System::Windows::Point ^ pt = EnvironsTouchExtensions::GetPosition ( c, appWindow );
				float orientation = (float)EnvironsTouchExtensions::GetOrientation ( c, appWindow );
				int deviceID = (int)EnvironsTouchExtensions::GetTagData ( c )->Value;
				//FilterTagUp.down(deviceID);
				deviceHandler->moved ( deviceID, (int)pt->X, (int)pt->Y, orientation );
			}

			e->Handled = true;
			return;
		}
		else if ( EnvironsTouchExtensions::GetIsFingerRecognized ( c ) )
		{
			return;
		}
	}


	void Environs::appTouchUp ( Object ^ sender, TouchEventArgs ^ e )
	{
		TouchDevice ^ c = e->TouchDevice;

		//Debug.WriteLine("appTouchUp");

		if ( EnvironsTouchExtensions::GetIsTagRecognized ( c ) )
		{
			if (  isSurface && updateDeviceTagPosition )
			{
				System::Windows::Point ^ pt = EnvironsTouchExtensions::GetPosition ( c, appWindow );
				float orientation = (float)EnvironsTouchExtensions::GetOrientation ( c, appWindow );
				int deviceID = (int)EnvironsTouchExtensions::GetTagData ( c )->Value;
				//FilterTagUp.up(deviceID, pt, orientation);
				deviceHandler->vanished ( deviceID, (int)pt->X, (int)pt->Y, orientation );
			}

			e->Handled = true;
			return;
		}
		else if ( EnvironsTouchExtensions::GetIsFingerRecognized ( c ) )
		{
			return;
		}
	}


	namespace lib
	{
	}

}


#endif