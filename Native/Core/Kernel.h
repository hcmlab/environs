/**
 * Environs Kernel
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
#ifndef INCLUDE_HCM_ENVIRONS_KERNEL_H
#define INCLUDE_HCM_ENVIRONS_KERNEL_H

#include "Kernel.Display.h"
#include "Kernel.Mobile.h"

#include "Kernel.Windows.h"
#include "Kernel.Mobile.Platform.h"
#include "Kernel.Display.OSX.h"


namespace environs
{
	/**
	*	Environs Kernel
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/20/14
	*	@version	1.0
	*	@remarks	Header for an Environs Kernel
	* ****************************************************************************************
	*/
	class Kernel : public KernelDevice, public KernelPlatform
	{
	public:
		Kernel ( void );
		~Kernel ( void );

		static char       *	touchRecognizerNames [ ENVIRONS_TOUCH_RECOGNIZER_MAX ];
		static unsigned int touchRecognizerNamesCount;

		static void			ReleaseLibrary ();
		int                 SetMainAppWindow ( WNDHANDLE appWnd );
		void				UpdateAppWindowSize ();

		// This must be implemented by platform kernels
		static int          SetUseTouchRecognizer ( const char * moduleName, bool enable );

		static int          GetUniqueInputID ();

	private:
		int					onPreInit ();

		int					onPreStart ();
		int					onStarted ();

		int					onPreStop ();
		int					onStopped ();

	protected:
		static int          AddTouchRecognizer ( const char * moduleName );
		static int          RemoveTouchRecognizer ( const char * moduleName );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_KERNEL_H
