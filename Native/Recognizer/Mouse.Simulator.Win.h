/**
* Mouse simulator for Windows
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
#ifndef INCLUDE_HCM_ENVIRONS_MOUSE_SIMULATOR_WINDOWS_H
#define INCLUDE_HCM_ENVIRONS_MOUSE_SIMULATOR_WINDOWS_H

#ifdef _WIN32

#include "Interfaces/IInput.Recognizer.h"
#include "Interop.h"


namespace environs 
{
	/**
	*	Mouse simulator for Windows
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	Declares the exported API functions
	* ****************************************************************************************
	*/
	class MouseSimulator : implements IInputRecognizer
	{
	public:
		MouseSimulator ( void );
		virtual ~MouseSimulator ();

		int				Trigger ( lib::InputPackRec **	inputs, int inputCount );
		int				Perform ( lib::InputPackRec **	inputs, int inputCount );
		void			Finish ( lib::InputPackRec **	inputs, int inputCount );

		bool Init ( WNDHANDLE hWnd );

		static bool			enabled;
		static int desktopWidth;
		static int desktopHeight;

	private:
		bool Init ();
	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_MOUSE_SIMULATOR_WINDOWS_H
