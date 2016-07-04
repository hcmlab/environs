/**
 * Stunt / Stun establisher
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
#ifndef INCLUDE_HCM_ENVIRONS_STUNTREQUEST_H
#define INCLUDE_HCM_ENVIRONS_STUNTREQUEST_H

#define STUNT_INTERNAL_QUERIES
#define STUNT_HANDSHAKE_IDENT_SIZE	8

#include "Environs.Build.Opts.h"
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"
#include "Interop/Sock.h"
#include "Device.Info.h"


#ifdef __cplusplus

namespace environs	/// Namespace: environs ->
{
	class Instance;
	class DeviceBase;
	class StunTRequest;
    class DeviceController;
    class MediatorClient;
    
    struct MediatorInstance;

	extern LONGSYNC			stuntCount;
	extern LONGSYNC			stunCount;

	typedef struct StunTHandler
	{
        bool				isInternal;
        
        int					state;
        StunTRequest *		req;
        int					sock;
        int					port;
        struct sockaddr_in	address;
        char				channel;
        int                 fails;
        
        bool				Init ( StunTRequest * request, bool isConnect );
        bool				Connect ();
        bool				Accept ();
        
        sp ( StunTRequest )	stunt;
	}
	StunTHandler;


	/**
	*	Stunt establisher
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/01/13
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class StunTRequest
	{
		friend class DeviceBase;
		friend struct StunTHandler;

	public:
		StunTRequest ( int deviceID );
		~StunTRequest ( );
		bool					Init ();

		void					CloseThreads ( bool waitWorkingThread = true );
		
		static void				HandleIncomingRequest ( Instance * env, void * msg );
		static void *			ProcessIncomingRequest ( void * arg );
        
        static sp ( StunTRequest ) CreateRequest ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, char channel, unsigned int token );

		int						Establish ();

        bool					Handshake ( int sock );
        static int              Handshake ( Instance * env, int sock, char * buffer );

        int                     MakeSocket ( bool isInternal = false );
        
        void					HandleRequest ();

        static bool				EstablishRequest ( Instance * env, DeviceBase * device, char channelType, unsigned int token );

        bool                    isInitiator;
		char					channel;
		bool					doHandshake;

		bool					disposing;
		bool					detach;
        bool                    disposeDevice;

		int						deviceID;

		sp ( DeviceController )	deviceSP;

		/** The area name of the application environment. */
		char					areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char					appName [MAX_NAMEPROPERTY + 1]; // 31

        unsigned int			IP;
        unsigned short          Porti;
		unsigned int			IPe;
		unsigned short			Porte;

		struct 	sockaddr_in		addr;

		// local endpoint address of the stunt socket for stunt connection
		struct 	sockaddr_in		stuntAddr;
        
		EnvThread               thread;

        volatile DeviceStatus_t * deviceStatus;

	private:
		bool					allocated;
        
        int                     repeats;
        int						socketConnect;
        int						socketAccept;

        Instance			*	env;
        sp ( MediatorClient )   mediator;

		StunTHandler			internalStunt;
		StunTHandler			externalStunt;
        
        StunTHandler			acceptStunt;

        sp ( StunTRequest )     myself;
	};

	
	/**
	*	Stun establisher
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/01/13
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class StunRequest
	{
	public:
		StunRequest ( int deviceID );
		~StunRequest ( );

		bool				Init ();

        void				CloseThreads ( );
        void				ReleaseSP ( );

		static void *		EstablishStarter ( void * arg );
		void 				Establisher ( );

		bool				Establish ( );

		static void			HandleIncomingRequest ( Instance * env, void * msg );
        static bool			EstablishRequest ( DeviceBase * device );

#ifndef ENABLE_DEVICEBASE_WP_STUN
        void                ReleaseDeviceRequest ();
#endif
		char				doHandshake;
		char				isRequestor;
		char				disposing;
        

		bool				allocated;
		Instance		*	env;
		int					deviceID;
        unsigned int		IPi;
        unsigned short		Porti;
        unsigned int		IPe;
        unsigned short		Porte;
        
		sp ( DeviceController )	deviceSP;

		struct 	sockaddr_in	addr;
        
        LONGSYNC			threadState;
		ThreadSync          thread;
        
        sp ( StunRequest )  myself;
	};

		
	typedef struct _STUNTHandshakePacket
	{
		char			ident [ 4 ];

		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [ MAX_NAMEPROPERTY + 1 ]; // 31

		/** The application name of the application environment. */
		char			appName [ MAX_NAMEPROPERTY + 1 ]; // 31

		char			pad [ 2 ];
	}
	STUNTHandshakePacket;

} /* namepace Environs */
#endif

#endif //> --> INCLUDE_HCM_ENVIRONS_STUNTREQUEST_H