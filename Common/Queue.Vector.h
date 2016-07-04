/**
 * QueueVector Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUEVECTOR_H
#define	INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUEVECTOR_H


#ifndef CLI_CPP

namespace environs
{
	class AsyncWorker;
	class NotificationQueue;
	class MediatorDaemon;
    class MediatorClient;

	namespace lib
	{
		/**
		* QueueVector
		*
		*/
		class QueueVector
		{
            friend class environs::AsyncWorker;
			friend class environs::NotificationQueue;
            friend class environs::MediatorDaemon;
            friend class environs::MediatorClient;

		public:
			/** Constructor */
			QueueVector ();

			~QueueVector ();


			/**
			* Append the item at the end of the queue.
			*
			* @param obj    The object to push to the back of the queue.
			*/
			bool push ( void * obj );


			/**
			* Removes and returns the first object in the queue.
			*
			* @return The first object in the queue.
			*/
            void * pop ();
            
            
            /**
             * Returns the first object in the queue, but don't remove it
             *
             * @return The first object in the queue.
             */
            void * first ();
            
            
            /**
			* Query empty state of the queue.
			*
			* @return empty state.
			*/
			bool empty ();


			/**
			 * Get the size of the list.
			 *
			 * @return size of the list
			 */
			size_t size ();

			size_t size_;

		private:
			void ** items;
			size_t capacity;
			size_t next;
			size_t end;

			size_t compressThreshold;

			bool grow ();
			bool compress ();
		};
        
        
        
        template <class T>
        class QueueVectorSPProxy
        {
        public:
            T item;
        };
        
        
        template <class T>
        class QueueVectorSP {
            
        public:
            /** Constructor */
            QueueVectorSP ();
            
            ~QueueVectorSP ();
            
            
            /**
             * Append the item at the end of the queue.
             *
             * @param position
             */
            bool push ( const T & obj );
            
            
            /**
             * Removes and returns the first object in the queue.
             *
             * @param position
             *
             * @ return The first object in the queue.
             */
            T front ();
            
            void pop ();
            
            /**
             * Query empty state of the queue.
             *
             * @param empty state
             *
             * @ return The first object in the queue.
             */
            bool empty ();
            
            
            /**
             * Get the size of the list.
             *
             * @return size of the list
             */
            size_t size ();
            
            size_t size_;
            
        private:
            QueueVectorSPProxy<T> ** items;
            
            size_t capacity;
            size_t next;
            size_t end;

			size_t compressThreshold;

			bool grow ();
			bool compress ();
        };
        
	}
}

#endif


#endif	/// -> INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUEVECTOR_H








