/**
*	Environs Tracker Plugin for MS Surface
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
#ifndef INCLUDE_HCM_ENVIRONS_TRACKER_SURFACE_RAWIMAGE_H
#define INCLUDE_HCM_ENVIRONS_TRACKER_SURFACE_RAWIMAGE_H

#include "Interfaces/ITracker.h"
//#include "Interop/Stat.h"
#include "Interop/threads.h"
#include "DynLib/dyn.Lib.Cv.h"

#define MAX_TRACKER_IMAGE_BUFFERS	2

namespace environs 
{
	/**
	*	Environs Tracker for raw images of the Microsoft Surface Tabletops
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class TrackSurface : implements ITracker
	{
	public:
		TrackSurface ( );
		~TrackSurface ( );

		bool			IsRuntimeSupported ( unsigned int platform, unsigned int sdks );

		void			ReleaseResources ();
		int				AllocateResources ();

		int				Stop ( );
		int				Start ( );
		int				Perform  ( void * rawImage, unsigned int size );

		bool			Execute ( int command );
		void			ToggleSource ( );
		void			ToggleVideoOut ( );

	private:
		bool			allocated;
		bool			initialized;
		bool			tracking;
		bool			windowSourceVisible;
		
		IplImage		image;
		unsigned int	imageBufferFill;
		unsigned int	imageBufferTracking;
		char		*	imageBuffers [MAX_TRACKER_IMAGE_BUFFERS];
		bool			imageBuffersFree [MAX_TRACKER_IMAGE_BUFFERS];

		unsigned int	imageSize;
		pthread_mutex_t imageBuffer_mutex;

		int				Init ( );
		void			Release ( );

		pthread_cond_t	event_tracking;
		pthread_mutex_t event_mutex;
		pthread_t		thread_tracking;
		static void *	Thread_TrackingStarter ( void * object );
		void			Thread_Tracking ( );
	};


} /* namespace environs */


#endif	/// -> INCLUDE_HCM_ENVIRONS_TRACKER_SURFACE_RAWIMAGE_H