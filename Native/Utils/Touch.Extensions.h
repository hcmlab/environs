/**
 * Environs CPP Environs.TouchExtensions
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_TOUCH_EXTENSIONS_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_TOUCH_EXTENSIONS_H

#if (defined(CLI_CPP) && defined(CLI_PS))


namespace environs 
{
	/**
	*	Environs.TouchExtensions
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	public ref class EnvironsTouchExtensions 
	{
	public:
		static double GetPhysicalArea ( System::Windows::Input::InputDevice ^ inputDevice );

		static System::Windows::Point ^ GetPosition ( System::Windows::Input::InputDevice ^ inputDevice, System::Windows::IInputElement ^ relativeTo );

		static Microsoft::Surface::Presentation::Input::TagData ^ GetTagData ( System::Windows::Input::InputDevice ^ inputDevice );

		static bool GetIsPenRecognized ( System::Windows::Input::InputDevice ^ inputDevice );

		static bool GetIsFingerRecognized ( System::Windows::Input::InputDevice ^ inputDevice );

		static bool GetIsTagRecognized ( System::Windows::Input::InputDevice ^ inputDevice );

		static double GetOrientation ( System::Windows::Input::InputDevice ^ inputDevice, System::Windows::IInputElement ^ relativeTo );

	};
}


#endif

#endif