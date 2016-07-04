/**
 *	Three Finger Touch Gestures Recognizer
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
#ifndef INCLUDE_HCM_ENVIRONS_THREE_FINGER_TOUCH_GESTURES_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_THREE_FINGER_TOUCH_GESTURES_RECOGNIZER_H

#include "Interfaces/IInput.Recognizer.h"
#include "Portal.Info.Base.h"


namespace environs 
{
	/**
	*	Three Finger Touch Gestures Recognizer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@remarks
	* ****************************************************************************************
	*/
	class GestureThreeTouch : implements IInputRecognizer
	{
	public:
		GestureThreeTouch ( void );
		virtual ~GestureThreeTouch ( );

		bool		Init ( );

		int			Trigger ( lib::InputPackRec **	touches, int inputCount );
        int			Perform ( lib::InputPackRec **	touches, int inputCount );
        void        Flush ( );

		bool		SetIncomingPortalID ( int portalID );

	private:
		PortalInfoBase	info;
        
		int			prevFingCount;
		int			prevX;
		int			prevY;

		int			prevXcached;
		int         prevYcached;
		
        int         iniDist;
        int         iniWidth;
        int         iniHeight;

		double		scaleDist;
		int			portalID;

	};

} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_BEZEL_TOUCH_GESTURES_RECOGNIZER_H
