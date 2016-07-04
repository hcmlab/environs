/**
 * PortalInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H

#include "Environs.Platforms.h"
#include "Environs.Platform.Decls.h"
#include "Interop/Smart.Pointer.h"
#include "Notify.Context.h"
#include "Portal.Info.h"
#include "Environs.Types.h"
#include "Environs.Observer.CLI.h"

#ifndef CLI_CPP
#	include "Interfaces/IPortal.Instance.h"
#endif


/**
 *	PortalInstance CPP Object
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
#ifndef CLI_CPP
		class DeviceInstance;
		class PortalObserver;
		class IIPortalObserver;
#endif

		PUBLIC_CLASS PortalInstance CLI_ABSTRACT DERIVE_c_only ( environs::PortalInstance ) DERIVE_DISPOSABLE
		{
            MAKE_FRIEND_CLASS ( Environs );
            MAKE_FRIEND_CLASS ( DeviceList );
			MAKE_FRIEND_CLASS ( DeviceInstance );
            MAKE_FRIEND_CLASS ( environs::PortalInfo );
            MAKE_FRIEND_CLASS ( PortalInstanceProxy );
            MAKE_FRIEND_CLASS ( DeviceInstanceProxy );
        
		public:
			ENVIRONS_LIB_API PortalInstance ();
            ENVIRONS_LIB_API ~PortalInstance ();
            
#ifdef CLI_CPP
			environs::Call			async;

			bool					startIfPossible;
#else
            /** An ID that identifies this portal across all available portals. */
            int                     portalID ();
            
            
            /** true = Object is disposed and not updated anymore. */
            bool                    disposed ();
            
            
            /**
             * Get an Interface to the DeviceInstance that this PortalInstance is attached to.
             * */
			environs::DeviceInstancePtr  deviceRetained ();
            
            
            environs::PortalInfo OBJ_ptr  info ();
            
            
			environs::PortalStatus_t status ();

            bool                    disposeOngoing ();
            
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    outgoing ();
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    isOutgoing ();
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    isIncoming ();

			ENVIRONS_LIB_API int    GetObserverCount ();
            
			virtual environs::PortalType_t  portalType ();
#endif
            ENVIRONS_LIB_API void AddObserver ( environs::PortalObserver OBJ_ptr observer );
            ENVIRONS_LIB_API void RemoveObserver ( environs::PortalObserver OBJ_ptr observer );

            ENVIRONS_LIB_API bool Establish ( bool askForType );
            
            ENVIRONS_LIB_API bool Start ();
            ENVIRONS_LIB_API bool Stop ();
            
            ENVIRONS_LIB_API bool HasPortalRenderer ();
            
            ENVIRONS_LIB_API bool SetRenderSurface ( void * surface_ );
            ENVIRONS_LIB_API bool SetRenderSurface ( void * surface_, int width, int height );
            
            ENVIRONS_LIB_API bool ReleaseRenderSurface ();

            ENVIRONS_LIB_API PortalInfoBasePtr GetIPortalInfo ( );
            
            static bool SetPortalInfo ( int hInst, PortalInfoBasePtr infoBase );

			ENVIRONS_LIB_API bool SetIPortalInfo ( PortalInfoBasePtr infoBase );

			static PortalInstanceSP GetPortal ( int nativeID, int portalID );

			static PortalInfoBaseSP GetPortalInfo ( int hInst, int portalID );

			bool ReleaseRenderCallback ();

#ifdef DISPLAYDEVICE
			bool SetPortalOverlayARGB ( int layerID, int left, int top, int width, int height, int stride, Addr_obj renderData, int alpha, bool positionDevice );
#endif

			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();
		
		INTERNAL:

			ENVIRONS_OUTPUT_ALLOC_SP_RESOURCE ( PortalInstance );
                        
            static bool             globalsInit		INIT_to_false_in_cli;
            static bool				GlobalsInit ();
            static void				GlobalsDispose ();

			static NLayerMapTypeObj ( int, PortalInstanceEP ) OBJ_ptr	portals; //	INIT_to_EXP_in_cli ( gcnew ( NLayerMapTypeObj ( int, PortalInstanceEP ) ) );

			static pthread_mutex_t	portalInstancelock;
			static pthread_mutex_t	portalsLock;

#ifdef CLI_CPP
			virtual PortalInstanceEP ^ GetPlatformObj () = 0;
#endif

            int                     hEnvirons_;
            
            /** An ID that identifies this portal across all available portals. */
            int                     portalID_;
            
            /** true = Object is disposed and not updated anymore. */
			LONGSYNC                disposed_;
            
            void                    PlatformDispose ();
            
            environs::PortalInfo    info_;
            
            environs::PortalStatus_t status_;

            bool                    disposeOngoing_;
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    outgoing_;
            
			environs::PortalType_t  portalType_;
            
            /** A DeviceInstance object that this portal relates to. */
			DeviceInstanceESP    device_;

			ENVOBSERVER ( lib::IIPortalObserver, environs::PortalObserver )	observers;

			int                 dialogID;
			bool                networkOK;
			INTEROPTIMEVAL      createdTickCount;


			bool                establishCalled;
			bool                askForTypeValue;

			void			*   surface;
			bool                renderActivated;

			int					portalKey;

            static int			GetKey ( int nativeID, int portalID_ );

            bool				CreateInstance ( c_const DeviceInstanceESP c_ref device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot );
        
			bool				Create ( c_const DeviceInstanceESP c_ref device, int destID );
			bool				Create ( c_const DeviceInstanceESP c_ref device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot );

			static void c_OBJ_ptr PortalPresenterThread ( pthread_param_t pack );
			static void			PresentPortalToObservers ( c_const PortalInstanceSP c_ref portal, int notification );

			static void			HandleSuccessfulPortal ( c_const PortalInstanceSP c_ref portal, int notification );
            
			static bool			UpdateWidthHeight ( c_const PortalInstanceSP c_ref portal, Addr_ptr context );
        
            static void			UpdateOptions ( environs::ObserverNotifyContext OBJ_ptr ctx );
            
            void				Update ();
			
			virtual void		UpdateCallbacks ( int notification ) { };

			static void			Update ( int hInst, environs::ObserverNotifyContext OBJ_ptr ctx );
            
            bool				CheckNetworkConnection ();
        
            void				ShowDialogOutgoingPortalView ();
            void				ShowDialogNoWiFiWarn ();
			bool				ShowDialogOutgoingPortal ();
            
			void				NotifyObservers ( environs::Notify::Portale_t notification);
        
			void				DisposeInstance ( bool removeFromGlobal );
            static void			Dispose ( int nativeID, int portalID );

            static void c_OBJ_ptr Destroyer ( pthread_param_t _targets );
        
			static void			KillZombies ();
		};
	}

}

#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H

