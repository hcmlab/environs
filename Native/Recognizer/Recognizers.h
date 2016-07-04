/**
 * Recognizer manager
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
#ifndef INCLUDE_HCM_ENVIRONS_RECOGNIZERS_MANAGER_H
#define INCLUDE_HCM_ENVIRONS_RECOGNIZERS_MANAGER_H

#include "Human.Input.Decl.h"
#include "Device.Display.Decl.h"
#include "Interop/Threads.h"

#include "Interfaces/IInput.Recognizer.h"
#include "Interfaces/IPortal.Renderer.h"


namespace environs 
{
    class Instance;
    
    
	/**
	*	Recognizers is a container/manager for recognizer plugins
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class Recognizers
	{
        friend class TouchSource;
        
	public:
		int                         deviceID;
		bool                        useThread;
        int                         activeRecognizer;
        
		static volatile bool        active;

		Recognizers ( int deviceID, DeviceDisplay * display );
		virtual ~Recognizers ( );

        static Recognizers    *     GetRecognizers ( Instance * obj, int deviceID, DeviceDisplay * display );
        
		bool                        Init ( bool useThread );
		bool                        Start ( void * device );
		bool                        Stop ( );
		void                        Dispose ();
        
        int                         Perform ( environs::lib::InputPackRec ** touches, int touchesSize );
        
        void                        Finish ( environs::lib::InputPackRec ** touches, int touchesSize );
        void                        Flush ();

		bool						SetIncomingPortalID ( int portalID );

	protected:

	private:
		bool						allocated;
        Instance             *   env;
        RenderDimensions		*	renderDims;
        
        DeviceDisplay               recognizerDisplay;
        
        void					*   device;

		// Touch visualization resources
		pthread_t					recognizerThreadID;
		pthread_cond_t				recognizerEvent;
		pthread_mutex_t             accessMutex;

        int                         recognizerContainerIndexer;
		environs::lib::InputPackRec			*	recognizerTouches [2];
        int                         recognizerTouchesCount [2];
		bool						recognizerTouchesHandled [2];

		IInputRecognizer		**	recognizers;
        int                         recognizersCount;
		bool						recognizedGesture;
        
        bool                        AddRecognizer ( const char * modName, bool checkDuplicate );
        bool                        RemoveRecognizer ( const char * modName );

        
        void                        SetDeviceBase ( void * device );
        
		static void *				recognizerThreadStarter ( void * arg );
		void *						Thread_Recognizer ( );
	};


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_RECOGNIZERS_MANAGER_H
