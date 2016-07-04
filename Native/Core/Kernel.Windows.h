/**
* Environs Windows Kernel
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
#ifndef INCLUDE_HCM_ENVIRONS_KERNELWINDOWS_SPECIFIC_H
#define INCLUDE_HCM_ENVIRONS_KERNELWINDOWS_SPECIFIC_H

#if (defined(_WIN32))
#include "Core.h"


namespace environs 
{
	class Instance;


	/**
	*	Environs Windows Kernel
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/20/14
	*	@version	1.0
	*	@remarks	Header for Windows Kernel
	* ****************************************************************************************
	*/
	class KernelPlatform
	{
		friend class Kernel;

	public:
		KernelPlatform ( void );
		~KernelPlatform ( void );

		static void			ReleaseLibrary ();
		static bool			StartWinSock ();
		static bool			DisposeWinSock ();

		int                 SetMainAppWindow ( WNDHANDLE appWnd );
		void				UpdateAppWindowSize ();

	protected:
		Instance		*	env;

	private:
		int					onPreInit ();

		int					onPreStart ();
		int					onStarted ();

		int					onPreStop ();
		int					onStopped ();

		// Resource. Winsock
		static bool			winSockStarted;

		static ULONG_PTR	gdiplusToken;
	};

} /* namespace environs */
#endif

#endif // INCLUDE_HCM_ENVIRONS_KERNELWINDOWS_SPECIFIC_H