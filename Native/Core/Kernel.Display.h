/**
* Environs Kernel for Large Displays
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
#ifndef INCLUDE_HCM_ENVIRONS_KERNELDISPLAYS_H
#define INCLUDE_HCM_ENVIRONS_KERNELDISPLAYS_H

#ifdef DISPLAYDEVICE

#include "Core.h"
#include "Interfaces/ITracker.h"


namespace environs
{
	/**
	*	Environs Kernel for Large Displays
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/20/14
	*	@version	1.0
	*	@remarks	Header for Windows Kernel
	* ****************************************************************************************
	*/
	class KernelDevice : public Core
	{
		friend class Kernel;

	public:
		KernelDevice ( void );
		~KernelDevice ( void );

		static void			ReleaseLibrary ();
		int                 SetMainAppWindow ( WNDHANDLE appWnd );

		static void			InitStatics ();
		static void			ReleaseStatics ();

		static char		*	trackerNames [ ENVIRONS_TRACKER_MAX ];
		static unsigned int	trackerNamesCount;

        int                 SetUseTracker ( const char * moduleName );
        int                 GetUseTracker ( const char * moduleName );
        bool                DisposeTracker ( const char * moduleName );

		ITracker		*	tracker [ ENVIRONS_TRACKER_MAX ];
        unsigned int		trackerCount;

        void                ReleaseTrackers ();

		ITracker		*	GetTracker ( unsigned int index );
		bool				AddTracker ( const char * moduleName );

	private:
		int					onPreInit ();
		int					onInitialized ();

		int					onPreStart ();
		int					onStarted ();

		int					onPreStop ();
		int					onStopped ();

        ITracker        *   CreateInstance ( char * name );
        bool                RunInstance ( ITracker * &t, int trackerID );

	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_KERNELDISPLAYS_H
