/**
 * Touch source that serves as the sink of touch events dispatched by the application
 * and as a source of touch events send to connected devices.
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
#ifndef INCLUDE_HCM_ENVIRONS_TOUCHSOURCE_H_
#define INCLUDE_HCM_ENVIRONS_TOUCHSOURCE_H_

#ifndef DISPLAYDEVICE

#include "Interop/Threads.h"

#include "stdint.h"
#include "Human.Input.Decl.h"
#include "Interfaces/IInput.Recognizer.h"
#include "Device.Info.h"
#include "Recognizer/Recognizers.h"

#include "Environs.Native.h"

namespace environs
{

#define CAPACITY_INCREASE_VALUE	6

#define FRAME_ALIVE				0x00
#define	FRAME_INIT				0x01
#define FRAME_ADD				0x02
#define	FRAME_FLUSH				0x10

#ifdef XCODE
#define TIMEVAL    uint64_t
#else
#define TIMEVAL    time_t
#endif

	/**
	*	Touch source that serves as the sink of touch events dispatched by the application
	*	--------------------------------------------------------------------------------------
	*	Copyright (C) 2013-2014 Chi-Tai Dang
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/07/13
	*	@version	1.0
	*	@remarks	Header file
	* ****************************************************************************************
	*/
	class TouchSource
	{        
	public:
		TouchSource ( int deviceID, unsigned int capacity );
		virtual ~TouchSource ( );

        bool                    Init ( void * device, int portalID );
        
        int                     deviceID;

		bool					viewAdapt;
		float					xScale;
		float					yScale;
        
        bool					Start ();
		bool					Stop ( );

        void                    Handle ( environs::lib::InputPackRaw * touchPacks, unsigned int count, bool init );
        
		void					Flush ( );
		void					Cancel ( );
		void					SendFrame ( bool useLock = true );
        
        lib::InputPackRec   *	AllocTouch ( lib::InputPackRaw * touchPack );


	private:
        bool                    allocated;
        void                *   device;
        int                     portalID;
        
		bool					alive;
        int                     frameNumber;
        
		int						capacity;
		INTEROPTIMEVAL			last_heartBeat;

        pthread_mutex_t         accessMutex;
        
		environs::lib::InputPackRec         **	touches;
		int						touchesSize;
		char					sendBuffer [66000];
		
		pthread_t				aliveThreadID;
		static void			*	AliveThread ( void * object );
		void				*	AliveThreadInstance ();

		void					Clear ( );

        Recognizers         *   recognizers;
        
        bool					AddRecognizer ( const char * modName );
        bool					RemoveRecognizer ( const char * modName );
	};

}
#endif /* TOUCHSOURCE_H_ */

#endif

