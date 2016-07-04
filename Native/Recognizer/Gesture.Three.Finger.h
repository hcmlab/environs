/**
 * Three Finger Gesture Recognizer
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
#ifndef INCLUDE_HCM_ENVIRONS_THREE_FINGER_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_THREE_FINGER_RECOGNIZER_H

#include "Interfaces/IInput.Recognizer.h"

#define MSG_HEADER_STREAM_LEN	(MSG_HEADER_LEN + 12)


namespace environs 
{
	/**
	*	Three Finger Gesture Recognizer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@remarks
	* ****************************************************************************************
	*/
	class GestureThreeFinger : implements IInputRecognizer
	{
	public:
		GestureThreeFinger ( void );
		virtual ~GestureThreeFinger ( );

		int				Trigger ( lib::InputPackRec **	inputs, int inputCount );
		int				Perform ( lib::InputPackRec **	inputs, int inputCount );
		void			Finish ( lib::InputPackRec **	inputs, int inputCount );

	private:
		bool Init ();

		int	mode;
		int MoveID;
		int MoveXInit;
		int MoveYInit;
		int Referece1X;
		int Referece1Y;
		int Referece2X;
		int Referece2Y;
		int MoveXDiff;
		int MoveYDiff;
		int MiddleX;
		int MiddleY;
		int InitWidth;
		int InitHeight;
		int MiddleID;
	};

} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_THREE_FINGER_RECOGNIZER_H
