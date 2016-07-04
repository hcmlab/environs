/**
 * PortalInstance Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_INTERFACE_H

#include "Environs.Types.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Portal.Info.h"


/**
 *	PortalInstance Interface
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
/* Namespace: environs -> */
namespace environs
{
	namespace lib
	{
		class DeviceInstance;
	}

	class DeviceInstance;

	class PortalInfo;
	class PortalObserver;

	class ENVIRONS_LIB_API PortalInstance : public lib::IEnvironsDispose
	{
	public:
		PortalInstance () : startIfPossible ( false ), appContext0 ( 0 ), appContext1 ( 0 ), appContext2 ( 0 ), appContext3 ( 0 ) { };
		virtual ~PortalInstance () { };


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		virtual void            Release () = 0;

		/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
		Call_t                  async;

		/** An ID that identifies this portal across all available portals. */
		virtual int             portalID () = 0;

		/** true = Object is disposed and not updated anymore. */
		virtual bool            disposed () = 0;

		/**
		* Get the DeviceInstance that this PortalInstance is attached to.
		* */
		sp ( DeviceInstance ) device ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, deviceRetained () );
		}

		/**
		* Get an Interface to the DeviceInstance that this PortalInstance is attached to.
		* */
		virtual DeviceInstance * deviceRetained () = 0;


		virtual environs::PortalInfo  * info () = 0;

		virtual environs::PortalStatus_t status () = 0;

		virtual bool            disposeOngoing () = 0;

		bool                    startIfPossible;

		/** true = outgoing (Generator), false = incoming (Receiver). */
		virtual bool            outgoing () = 0;

		/** true = outgoing (Generator), false = incoming (Receiver). */
		virtual bool            isOutgoing () = 0;

		/** true = outgoing (Generator), false = incoming (Receiver). */
		virtual bool            isIncoming () = 0;

		virtual environs::PortalType_t  portalType () = 0;

		/** Application defined contexts for arbitrary use. */
		void *                  appContext0;
		void *                  appContext1;
		void *                  appContext2;
		void *                  appContext3;

		virtual void AddObserver ( PortalObserver * observer ) = 0;
		virtual void RemoveObserver ( PortalObserver * observer ) = 0;

		virtual int GetObserverCount () = 0;
		virtual bool Establish ( bool askForType ) = 0;

		virtual bool Start () = 0;
		virtual bool Stop () = 0;

		virtual bool HasPortalRenderer () = 0;

		virtual bool SetRenderSurface ( void * surface_ ) = 0;
		virtual bool SetRenderSurface ( void * surface_, int width, int height ) = 0;

		virtual bool ReleaseRenderSurface () = 0;

		virtual  PortalInfoBase * GetIPortalInfo () = 0;
		virtual  bool SetIPortalInfo ( PortalInfoBase * infoBase ) = 0;
	};
}

#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_INTERFACE_H







