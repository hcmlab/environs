/**
 *	Portal Generator
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_BASE_H
#define INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_BASE_H

#ifdef __cplusplus

#include "Environs.Native.h"
#include "Portal.Info.Base.h"
#include "Render.Overlay.h"
#include "Portal.Stream.Options.h"
#include "Portal.Worker.Stages.h"
#include "Recognizer/Recognizers.h"
#include "Interfaces/IInput.Recognizer.h"
#include "Recognizer/Recognizers.h"

#include <vector>
#include "Interop/Threads.h"


namespace environs 
{
	class DeviceController;

	/**
	*	A portal generator renders, encodes and sends a portal to a device
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class PortalGenerator
	{
		friend class DeviceBase;
        friend class DeviceController;
        friend class DevicePlatform;
        friend class AsyncWorker;

	public:
		PortalGenerator ();
		virtual	~PortalGenerator ();

		void						Dispose ( );

		bool						InitRenderContexts ( );
		void						DisposeRenderContexts ( );

		bool						GetLock ( );
		void						GetDimensionsLock ( RenderDimensions * &dims, environs::lib::InputPackRec * &recoContainer, unsigned int &recoIndex );
		void						ReleaseLock ( );
		void						ReleaseDimensionsLock ( unsigned int touchCount, unsigned int recoIndex );

		bool						Init ( PortalStreamOptions * streamOptions, DeviceController * parent, int portalID );
		bool						Start ( );
        
#ifdef ENABLE_PORTAL_STALL_MECHS
        void                        Stall ();
        void                        UnStall ();
#endif
		bool						Pause ( );
		bool						Stop ( );
		
		void						UpdatePortalSize ( int width_new, int height_new, bool updateAll = true );
		void						UpdatePosition ( int x, int y, float orientation, bool updateAll = true );

		void						UpdateRenderDimensions ( );
		static void					UpdateDevicesCoverage ( void * device );

		//void						TranslateToSurfaceCoord ( int &x, int &y );
		bool						GetPortalInfo ( PortalInfoBase * info );
		int							GetPortalInfoIfChanged ( PortalInfoBase * info );
		
		bool						setPortalOverlayARGB ( int layerID, int left, int top, unsigned int width, unsigned int height, unsigned int stride, void * renderData, int alpha, bool positionDevice );

		PortalSourceStatus_t		status;

		static pthread_cond_t		portalWorkerEvent;
		static pthread_mutex_t		portalWorkerEventLock;
		
	private:
		bool						allocated;
#ifdef ENABLE_PORTAL_STALL_MECHS
        bool                        stalled;
        int                         stalledCounter;
#endif
	protected:
		static void					InitFrameTrigger ( );
		static void					DisposeFrameTrigger ( );

		int							portalID;
		Instance				*	env;

		DeviceController		*	parentDevice;
		LONGSYNC                    accessLocks;

		PortalStreamOptions		*	streamOptions;

		bool						InitClass ();

		bool						InitPortal ( );
		virtual void				InitRecognizers ( ) {};
		void						DisposePortal ( );

		WorkerStages				workerStages;
		void						DisposeWorkerStages ( WorkerStages * stages );
        bool                        GetWorkerStages ( WorkerStages * stages, int index );
        
        virtual bool                CreateWorkerStages ( WorkerStages * stages, int index );

		bool						SendStreamInit ( int width, int height );
		
		static LONGSYNC             referenceCount;
		static pthread_cond_t		hPortalUpdateTimer;

		int                         deviceID;
		int							centerX;
		int							centerY;

		float						orientationLast;
		float						deviceAzimut;
		float						deviceAzimutLast;

#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
		// Input recognizer 
		Recognizers				*	recognizers;
#else
        // Touch visualization resources
        LONGSYNC					recognizerThreadIDState;
		pthread_t					recognizerThreadID;
		pthread_cond_t				recognizerEvent;
		pthread_mutex_t             recognizerCritSec;

		unsigned int				recognizerContainerIndexer;
		environs::lib::InputPackRec			*	recognizerTouches [2];
		unsigned int				recognizerTouchesCount [2];
		bool						recognizerTouchesHandled [2];

		IInputRecognizer		**	recognizers;

		unsigned int				recognizersCount;
		bool						recognizedGesture;

		// Gesture recognition
		// Mode 0 = not detecting, requirements not met
		// Mode 1 = requirements do match, detecting the mode
		// Mode 10 = in moving gesture
		// Mode 20 = in zooming gesture
		// Mode 30 = in rotating gesture
		static void *				Thread_RecognizerStarter ( void * arg );
		void *						Thread_Recognizer ( );
#endif
		pthread_mutex_t				renderOverlayMutex;
		RenderOverlay			*	renderOverlays [MAX_PORTAL_OVERLAYS];

		pthread_mutex_t				renderDimensionsMutex;
		RenderDimensions			renderDimensions;
		PortalInfoBase				portalInfosCache;

		unsigned int				filledContexts; // required to fill all contexts with the latest equal frame before reusing only the previous context

#ifndef ENABLE_IMPROVED_PORTAL_GENERATOR
		pthread_mutex_t             renderContextMutex;
        
#endif
		RenderContext				renderContexts [MAX_PORTAL_CONTEXT_WORKERS];
        
        unsigned int				renderContextNext;
        unsigned int				renderLastChanged;
        
        pthread_cond_t				workerStateEvent;
		LONGSYNCNV					renderDimensionsChanged [MAX_PORTAL_CONTEXT_WORKERS];
        
        pthread_t					workerThreads [MAX_PORTAL_CONTEXT_WORKERS];
        LONGSYNC					workerThreadsID [MAX_PORTAL_CONTEXT_WORKERS];
        
        static void *				Thread_WorkerStarter ( void * object );
        void						Thread_Worker ( );
	};

} /* namespace environs */

#endif


#endif // INCLUDE_HCM_ENVIRONS_PORTALGENERATOR_H
