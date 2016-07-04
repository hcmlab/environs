/**
 *	Portal stream functionality
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
#ifdef __cplusplus

#include "Environs.Native.h"
#include "Core/Byte.Buffer.h"
#include "Interop/Threads.h"

#ifndef INCLUDE_HCM_ENVIRONS_PORTALSTREAM_H
#define INCLUDE_HCM_ENVIRONS_PORTALSTREAM_H



/* Namespace: environs -> */
namespace environs
{
	class Instance;


	/**
	*	PortalStream class
	*	--------------------------------------------------------------------------------------
	*	Copyright (C) 2012 Chi-Tai Dang
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/07/13
	*	@version	1.0
	*	@remarks	Header file
	* ****************************************************************************************
	*/
	class PortalStream
	{
		friend class PortalReceiver;

		public:
			PortalStream ();
			~PortalStream ( );
			bool					Init ( int deviceID );
            void					Start ();
			void					Stop ( );
			void					StopNonBlock ( );

			bool					GetLock ( );
			void					ReleaseLock ( );

			void					ReleasePortalStream ();
			void					ReleaseStreamBuffers ();
			void					ReleaseStreamStorage ();

			ByteBuffer			*	GetNextStreamBuffer ( unsigned int minCapacity, bool returnUnlocked = true );
        
            void					PushStreamBuffer ( ByteBuffer * payload, unsigned int payloadSize, unsigned int type );
			bool					PushStreamPacket ( void * payload, unsigned int payloadSize, unsigned int type );
			jobject					ReceiveStreamPack ();

            bool                    stalled;
        
		private:
			bool					allocated;
        
			Instance			*	env;
			int                     deviceID;
			unsigned int			accessCount;
        
			// Stream / Portal resources
			unsigned int			bufferMax;
			unsigned int			bufferCount;
			ByteBuffer **			buffers;
			
			int						nextBuffer;
			int						busyBuffer;

			unsigned int			nextPushBuffer;

			pthread_mutex_t 		resourcesMutex;
			pthread_cond_t 			availEvent;
			pthread_cond_t 			queueEvent;

			bool					enabled;
	};

}

#endif

#endif /* INCLUDE_HCM_ENVIRONS_PORTALSTREAM_H_ */
