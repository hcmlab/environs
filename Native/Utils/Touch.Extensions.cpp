/**
* Environs CPP DeviceHandler
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

#include "Interop.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Environs.Cli.TouchDevice.h"
#include "Touch.Extensions.h"
#include "Environs.Native.h"

using namespace System::Windows::Controls;

#define CLASS_NAME	"Environs.TouchExtensions"


namespace environs
{
	double EnvironsTouchExtensions::GetOrientation ( System::Windows::Input::InputDevice ^ inputDevice, System::Windows::IInputElement ^ relativeTo )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^) inputDevice;
			return device->GetOrientation ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetOrientation ( inputDevice, relativeTo );
	}


	bool EnvironsTouchExtensions::GetIsTagRecognized ( System::Windows::Input::InputDevice ^ inputDevice )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetIsTagRecognized ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetIsTagRecognized ( inputDevice );
	}


	bool EnvironsTouchExtensions::GetIsFingerRecognized ( System::Windows::Input::InputDevice ^ inputDevice )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetIsFingerRecognized ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetIsFingerRecognized ( inputDevice );
	}


	bool EnvironsTouchExtensions::GetIsPenRecognized ( System::Windows::Input::InputDevice ^ inputDevice )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetIsPenRecognized ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetIsFingerRecognized ( inputDevice );
	}


	Microsoft::Surface::Presentation::Input::TagData ^ EnvironsTouchExtensions::GetTagData ( System::Windows::Input::InputDevice ^ inputDevice )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetTagData ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetTagData ( inputDevice );
	}


	System::Windows::Point ^ EnvironsTouchExtensions::GetPosition ( System::Windows::Input::InputDevice ^ inputDevice, System::Windows::IInputElement ^ relativeTo )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetPosition ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetPosition ( inputDevice, relativeTo );
	}


	double EnvironsTouchExtensions::GetPhysicalArea ( System::Windows::Input::InputDevice ^ inputDevice )
	{
		if ( inputDevice->GetType () == EnvironsTouchDevice::typeid )
		{
			EnvironsTouchDevice ^ device = (EnvironsTouchDevice ^)inputDevice;
			return device->GetPhysicalArea ();
		}
		return Microsoft::Surface::Presentation::Input::TouchExtensions::GetPhysicalArea ( inputDevice );
	}
}


#endif