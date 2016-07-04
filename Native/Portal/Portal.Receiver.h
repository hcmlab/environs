/**
 *	A portal receiver receives portal stream packages, decodes them 
    and dispatches them to the renderer
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
#include "Environs.Native.h"
#include "Decoder/Decoder.Base.h"
#include "Portal.Info.Base.h"
#include "Core/Touch.Source.h"

#ifndef INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_BASE_H
#define INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_BASE_H



namespace environs 
{
	class Instance;


	/**
	*	A portal receiver receives portal stream packages, decodes them and dispatches them to the renderer
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class PortalReceiver
	{
		friend class DeviceBase;
		friend class DeviceController;
        friend class TouchSource;

	public:
		PortalReceiver ();
		virtual	~PortalReceiver ();

		void						Dispose ( );

		bool						Init ( int deviceID, int portalID );
		bool						IsActive ();
		bool						Start ( );
		void						Stop ( );
		void						StopNonBlock ( );
        
        int                         portalID;
		PortalStream			*	stream;
        
#ifndef DISPLAYDEVICE
        TouchSource             *   touchSource;
#endif
		virtual bool				SetRenderSurface ( void * penv, void * newSurface, int width, int height );

		bool						hasSurface ( );
		/** Release the render surface. */
		virtual void				ReleaseRenderSurface ( bool useLock );

		IPortalDecoder			*	decoder;
        
        bool                        SetRenderCallback ( ptRenderCallback callback, int callbackType );
        
        /** Release the render callback. */
        virtual void				ReleaseRenderCallback ( bool useLock = true );

		void  	*					renderSurface;
        void    *                   device;

    protected:
        bool						enabled;
        Instance				*	env;
        
		unsigned int				deviceID;
        bool                        ignoreBufferstatus;
        
        bool                        allocated;
        pthread_mutex_t             resourcesMutex;
        
        PortalInfoBase				portalInfo;
        
        /// A platform specific render callback called by the portal sink in order to render the rgb data
        ptRenderCallback			renderCallback;
        int                         renderCallbackType;

	private:
		
		virtual bool				CreateStream ( );
		void						DisposeStream ( );

		virtual int                 CreateDecoder ( );
		void						DisposeDecoder ( );

		pthread_t 					threadID;

		static void * 				StartPortalReceiver ( void * arg );
		void 	* 					Thread_Receiver ( );
	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTALRECEIVER_BASE_H
