/**
*	Environs Tracker Bridge that receives TUIO
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
#ifndef INCLUDE_HCM_ENVIRONS_TRACKER_TUIO_H
#define INCLUDE_HCM_ENVIRONS_TRACKER_TUIO_H

#include "Interfaces/ITracker.h"
//#include "Interop/Stat.h"
#include "Interop/Threads.h"

#include "TuioListener.h"

#if !defined(ENVIRONS_MISSING_TUIO_HEADERS)

#include "UdpReceiver.h"
using namespace TUIO;

namespace environs 
{
	/**
	*	Environs Tracker Bridge that receives TUIO
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class TrackTUIO : implements ITracker, public TuioListener
	{
	public:
		TrackTUIO ();
		~TrackTUIO ();

		/** IsPlatformSupported verifies whether the current platform is valid, that is the platform that this tracker is designed for. */
		bool			IsRuntimeSupported ( unsigned int platform, unsigned int sdks );
		
		void			ReleaseResources ();
		int				AllocateResources ();

		int				Stop ( );
		int				Start ( );
		int				Perform  ( void * rawImage, unsigned int size );

		bool			Execute ( int command );

		/// TUIO callbacks
		void			addTuioObject ( TuioObject *tobj );
		void			updateTuioObject ( TuioObject *tobj );
		void			removeTuioObject ( TuioObject *tobj );

		void			addTuioCursor ( TuioCursor *tcur );
		void			updateTuioCursor ( TuioCursor *tcur );
		void			removeTuioCursor ( TuioCursor *tcur );

		void			addTuioBlob ( TuioBlob *tblb );
		void			updateTuioBlob ( TuioBlob *tblb );
		void			removeTuioBlob ( TuioBlob *tblb );

		void			refresh ( TuioTime frameTime );

	private:
		bool			initialized;
		bool			tracking;

		OscReceiver	*	tuioReceiver;
		TuioClient	*	tuioClient;

		int				Init ( );
		void			Release ( );

		void			ConvertToPixel ( float x, float y, int &posX, int &posY );

		pthread_t		thread_tracking;
		static void *	Thread_TrackingStarter ( void * object );
		void			Thread_Tracking ( );
	};


} /* namespace environs */

#endif

#endif	/// -> INCLUDE_HCM_ENVIRONS_TRACKER_TUIO_H