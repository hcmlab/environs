/**
 * ArrayList Interface Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_ARRAYLIST_H
#define	INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_ARRAYLIST_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IArray.List.h"

//#define ENABLE_ARRAYLIST_LOCK

namespace environs
{
	namespace lib
	{
		class DeviceInstance;
		class MessageInstance;
		class FileInstance;


		/**
		* ArrayList
		*
		*/
		class ArrayList : public environs::ArrayList
		{
			friend class DeviceList;
			friend class DeviceInstance;
			friend class MessageList;

		public:
			/** Constructor */
			ENVIRONS_LIB_API ArrayList ();

			ENVIRONS_LIB_API ~ArrayList ();


			/**
			 * Get the item at the given position.
			 *
			 * @param position
			 *
			 * @ return The object at given position
			 */
            ENVIRONS_LIB_API void * item ( size_t pos );


            /**
             * Get the item at the given position and increase reference count by one.
             *
             * @param position
             *
             * @ return The object at given position
             */
            ENVIRONS_LIB_API void * itemRetained ( size_t pos );


			/**
			 * Get the size of the list.
			 *
			 * @return size of the list
			 */
			ENVIRONS_LIB_API size_t size ();


			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		private:
			/**
			* Internal Release used by Environs to clean up internal resources while object is locked.
			*
			*/
			void ReleaseLocked ();


            size_t size_;
            size_t capacity;
			void ** items;

#ifdef ENABLE_ARRAYLIST_LOCK
            bool                        disposed;
            pthread_mutex_t             lock;
#endif
			svsp ( DeviceInstance )		deviceList;
			svsp ( MessageInstance )	messageList;
			smsp ( int, FileInstance )	fileList;

            bool UpdateWithDevices ( const svsp ( DeviceInstance ) &list );
            
			static ArrayList * CreateWithDevicesRetained ( const svsp ( DeviceInstance ) &list );

            static ArrayList * CreateWithMessagesRetained ( const svsp ( MessageInstance ) &list, const DeviceInstanceSP device );
            
#   ifdef USE_QUEUE_VECTOR_T_CLASS
            static ArrayList * CreateWithMessagesQueueRetained ( QueueVectorSP < MessageInstanceESP > &list, const DeviceInstanceSP device );
#   else
            static ArrayList * CreateWithMessagesQueueRetained ( stdQueue ( MessageInstanceESP ) &list, const DeviceInstanceSP device );
#   endif
			static ArrayList * CreateWithFilesRetained ( const smsp ( int, FileInstance ) &list, const DeviceInstanceSP device );
		};
	}
}
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H








