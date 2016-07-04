/**
* Environs Touch Visualizer for Windows
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
#ifndef ENVIRONS_TOUCH_VISUALIZER_H
#define ENVIRONS_TOUCH_VISUALIZER_H

#include "Interfaces/IInput.Recognizer.h"
#include "Interop.h"

//#define MSG_HEADER_STREAM_LEN	(MSG_HEADER_LEN + 12)


namespace environs 
{
	/**
	*	Touch Visualizer Class for Windows.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	Header file
	* ****************************************************************************************
	*/
	class TouchVisualizer : implements IInputRecognizer
	{
	public:
		TouchVisualizer ( void );
		virtual ~TouchVisualizer ();

        int				Trigger ( environs::lib::InputPackRec **	inputs, int inputCount );
		int				Perform ( environs::lib::InputPackRec **	inputs, int inputCount );
		void			Finish ( environs::lib::InputPackRec **	inputs, int inputCount );

		bool			Init ( WNDHANDLE hWnd );
		static bool		enabled;

	private:
		bool Init ();

		WNDHANDLE		hAppWnd;

#ifdef _WIN32
		HDC				hDCTouchDisplay;
		HBRUSH			hTouchBrush;
#endif
	};

} /* namespace environs */

#endif // ENVIRONS_TOUCH_VISUALIZER_H
