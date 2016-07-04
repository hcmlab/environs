/**
 * QueueList Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUELIST_H
#define	INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUELIST_H


#ifndef CLI_CPP

//#define ENABLE_VECTOR_LIST

namespace environs
{
	namespace lib
	{
		class IQueueItem
		{
			friend class QueueList;

		private:
			IQueueItem * next;
		};

		/**
		* QueueList
		*
		*/
		class QueueList
		{
		public:
			/** Constructor */
			QueueList ();

			~QueueList ();


			/**
			* Append the item at the end of the queue.
			*
			* @param position
			*/
			bool push ( IQueueItem * obj );


			/**
			* Removes and returns the first object in the queue.
			*
			* @param position
			*
			* @ return The first object in the queue.
			*/
			void * pop ();


			/**
			* Query empty state of the queue.
			*
			* @param empty state
			*
			* @ return The first object in the queue.
			*/
			bool empty ();

#ifndef NDEBUG
			bool remove ( IQueueItem * obj );
#endif
			/**
			 * Get the size of the list.
			 *
			 * @return size of the list
			 */
			size_t size ();

			size_t size_;

		private:
			IQueueItem	*	front;
			IQueueItem	*	end;
		};
        
        
#ifdef ENABLE_VECTOR_LIST
        
        template <class T>
        class VectorListSPProxy
        {
        public:
            VectorListSPProxy () : next ( 0 )
            {};
            
            T                     item;
            
            VectorListSPProxy   * next;
        };
        
        
        template <class T>
        class VectorListSP {
            
        public:
            /** Constructor */
            VectorListSP ();
            
            ~VectorListSP ();
            
            
            
            /**
             * Append the item at the end of the queue.
             *
             * @param position
             */
            bool push_back ( const T & obj );
            
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
            VectorListSPProxy<T> ** items;
            size_t capacity;
            size_t next;
            size_t end;
            size_t sizeArray;
            
            VectorListSPProxy<T> * root;
            VectorListSPProxy<T> * last;
            
            
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
            VectorListSPProxy<T> * front ();
            
            
            /**
             * Allocate new proxies.
             *
             * @ return success.
             */
            bool alloc ( size_t start, size_t count );
        };
#endif
        
	}
}

#endif
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_QUEUELIST_H








