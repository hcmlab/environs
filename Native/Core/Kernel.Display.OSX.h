/**
 * Kernel for Mobile Devices (Unspecified Platform)
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
#ifndef INCLUDE_HCM_ENVIRONS_KERNEL_DISPLAY_POSIX_H
#define INCLUDE_HCM_ENVIRONS_KERNEL_DISPLAY_POSIX_H

#if defined(DISPLAYDEVICE) && (defined(TARGET_OS_MAC) || defined(LINUX))

#include "Kernel.Display.h"


namespace environs
{
	class KernelPlatform
	{
		friend class Kernel;

	public:
		KernelPlatform ( void );
		virtual ~KernelPlatform ( void );

		static void     	ReleaseLibrary ();

        int                 SetMainAppWindow ( WNDHANDLE appWnd );
        void                UpdateAppWindowSize ();

    protected:
        Instance		*	env;

	private:

		int					onInitialized ();

		int					onPreInit ();

		int					onPreStart ();
		int					onStarted ();

		int					onPreStop ();
		int					onStopped ();
	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_KERNEL_DISPLAY_POSIX_H
