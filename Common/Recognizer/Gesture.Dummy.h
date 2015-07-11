/**
 * Dummy Touch Gestures Recognizer
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
#ifndef INCLUDE_HCM_ENVIRONS_DUMMY_TOUCH_GESTURES_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_DUMMY_TOUCH_GESTURES_RECOGNIZER_H

#include "Interfaces/IInput.Recognizer.h"
#include "Portal.Info.h"


namespace environs 
{
	/**
	*	Dummy Touch Gestures Recognizer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@remarks
	* ****************************************************************************************
	*/
	class GestureDummy : implements IInputRecognizer
	{
	public:
		GestureDummy ( );
		virtual ~GestureDummy ( );
        
        bool		Init ( );
        
        int			Trigger ( InputPackRec **	touches, int inputCount );
        int			Perform ( InputPackRec **	touches, int inputCount );
        void        Flush ( );

	private:
	};

} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_DUMMY_TOUCH_GESTURES_RECOGNIZER_H
