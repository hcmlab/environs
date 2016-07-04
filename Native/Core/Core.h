/**
 * Environs Core
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
#ifndef INCLUDE_HCM_ENVIRONS_CORE_H
#define INCLUDE_HCM_ENVIRONS_CORE_H
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"

#define MSG_HEADER_STREAM_LEN	(MSG_HEADER_LEN + 12)

#if ( defined(_WIN32) )
#	define ENABLE_WINSOCK_THREADED_CORE
#endif

namespace environs
{
	class Instance;
    

	/**
	*	The core includes functionality and state that is common for all core implementations.
	*	Device specific kernels derive from the core and adds appropriate functionality/state.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	* ****************************************************************************************
	*/
	class Core
	{
        friend class NotificationQueue;
		friend class DeviceBase;
        
	public:
		static volatile bool	active;
		int						appStatus;

		Core ( void );
		virtual ~Core ();

		int				Init ( Instance * obj );
		int				Start ();
        int				Stop ();
        
        void			StopNetLayer ();
        
		void			Release ();

		void			SetAppStatus ( int status );

		static bool		EstablishMediator ( Instance * env );
        static void		MediatorEvent ( void * eventArg );
        
        static void     GenerateRandomDeviceID ( Instance * env );

	protected:
		int             hEnvirons;
		Instance	*	env;

		int				appStatusNext;
		pthread_mutex_t	appStatusMutex;
		EnvThread       appStatusThread;
        
		static void *	StartAppStatusUpdater ( void * object );
		void			AppStatusUpdater ();

        bool            InitCrypt ();
        
		virtual int		onPreInit ();
		virtual int		onInitialized ();

		virtual int		onPreStart ();
		virtual int		onStarted ();

		virtual int		onPreStop ();
		virtual int		onStopped ();
        
        void			StopListener ( bool wait );
        
		void			DetermineDefaultDisplay ();

		LONGSYNC		tcpHandlers;
		EnvSignal       tcpHandlerSync;
		static void *	TcpHandler ( void * arg );
        
        char        *   udpBuffer;
        
        static void *	StartListener ( void * object );
        void 			Listener ();
        
        ThreadSync      thread;
        sp ( Instance )	threadAlive;
        
        bool 			TcpAcceptor ();
        bool 			UdpAcceptor ();
        
        // Resource. Socket
		int				tcpAcceptSocket;
        
		// Resource. Socket
        int				udpAcceptSocket;

#ifdef ENABLE_WINSOCK_THREADED_CORE
		HANDLE			events [ 3 ];
#endif

	private:
        bool            allocated;
		LONGSYNC		stopInProgress;
		EnvSignal		stopSync;

	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_CORE_H
