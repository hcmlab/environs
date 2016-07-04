/**
 * Environs Mouse simulator example
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
#ifndef ENVIRONS_MOUSE_SIMULATOR_EXAMPLE_H
#define ENVIRONS_MOUSE_SIMULATOR_EXAMPLE_H

#ifndef _WIN32

#include "Interfaces/IInput.Recognizer.h"

namespace environs 
{
	class MouseSimulator : implements IInputRecognizer
	{
	public:
		MouseSimulator ( void );
		virtual ~MouseSimulator ();
        
        int				Trigger ( lib::InputPackRec **	inputs, int inputCount );
        int				Perform ( lib::InputPackRec **	inputs, int inputCount );

		bool Init ( );

		static bool		enabled;

	private:
		bool Init ( WNDHANDLE wnd );
	};

} /* namespace environs */

#endif

#endif // ENVIRONS_MOUSE_SIMULATOR_EXAMPLE_H
