/**
 * QueueVector
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifndef CLI_CPP

#include "Environs.Native.h"
#include "Queue.Vector.h"

#ifndef CLI_CPP
#   include <memory>
#	ifndef MEDIATORDAEMON
#		include "Device.Instance.h"
#		include "Message.Instance.h"
#	endif
#endif

#define CLASS_NAME	"Queue.Vector . . . . . ."

#ifdef MEDIATORDAEMON
#   define GROW_SIZE 250
#else
#   define GROW_SIZE 50
#endif

namespace environs
{
	namespace lib
	{
		QueueVector::QueueVector ()
		{
			CVerb ( "Construct" );

			size_   = 0;
			items   = 0;
			next    = 0;
            end     = 0;

            capacity			= 0;
			compressThreshold	= 0;
		}


		QueueVector::~QueueVector ()
		{
			CVerb ( "Destruct" );

			if ( items )
				free ( items );
		}


		bool QueueVector::grow ()
		{
			// Grow the buffer
			void ** tmp = ( void ** ) calloc ( 1, ( capacity + GROW_SIZE ) * sizeof ( void * ) );
			if ( !tmp )
				return false;

			// copy from next to end
			if ( next == end ) {
				// This must be true
				if ( !next ) {
					memcpy ( tmp, items, capacity * sizeof ( void * ) );
				}
				else {
					size_t toCopy = capacity - next;

					memcpy ( tmp, items + next, toCopy * sizeof ( void * ) );

					memcpy ( tmp + toCopy, items, next * sizeof ( void * ) );
				}
			}
			else {
				CErr ( "grow: Invalid queue state!" );

				free ( tmp );
				return false;
			}

			compressThreshold = capacity; // Do not compress at all

			end = capacity;
			next = 0;

			free ( items );
			items = tmp;
			capacity += GROW_SIZE;

			return true;
		}


		bool QueueVector::compress ()
		{
			// Grow the buffer
			size_t compCap = GROW_SIZE;
			if ( size_ >= compCap ) {
				compCap = capacity - GROW_SIZE;
			}

			void ** tmp = ( void ** ) calloc ( 1, compCap * sizeof ( void * ) );
			if ( !tmp )
				return false;

			if ( size_ > 0 ) {
				// copy from next to end
				if ( next < end ) {
					memcpy ( tmp, items + next, size_ * sizeof ( void * ) );
				}
				else {
					size_t count = capacity - next;

					memcpy ( tmp, items + next, count * sizeof ( void * ) );

					if ( end > 0 )
						memcpy ( tmp + count, items, end * sizeof ( void * ) );
				}
				end = size_;
			}
			else
				end = 0;
			next = 0;

			capacity = compCap;

			if ( ( ( int ) capacity - GROW_SIZE ) > GROW_SIZE )
				compressThreshold = capacity - GROW_SIZE;
			else
				compressThreshold = GROW_SIZE; // Keep at least the grow size as cache

			free ( items );
			items = tmp;

			return true;
		}


		bool QueueVector::push ( void * item )
		{
			CVerbVerbArg ( "push: Elements in the queue [ %i ].", size_ );

			if ( items == 0 ) {
				items = ( void ** ) calloc ( 1, GROW_SIZE * sizeof ( void * ) );
				if ( !items )
					return false;

				compressThreshold = capacity = GROW_SIZE; // Keep at least the grow size as cache
			}
			else if ( size_ >= capacity ) {
				if ( !grow () )
					return false;
			}

			items [ end ] = item;

			end++;
			++size_;

			if ( end >= capacity )
				end = 0;
			return true;
		}


		void * QueueVector::pop ()
		{
			if ( size_ <= 1 ) 
			{
				if ( compressThreshold > GROW_SIZE ) { // && size_ < compressThreshold ) {
					if ( !compress () )
						return 0;
				}
				if ( size_ <= 0 )
					return 0;
			}

			void * item = items [ next ];
			items [ next ] = 0;
			next++;

			if ( next >= capacity )
				next = 0;

			--size_;

			return item;
        }
        
        
        void * QueueVector::first ()
        {
            return items [ next ];
        }
        
        
        bool QueueVector::empty ()
		{
			if ( size_ <= 0 )
			{
				if ( size_ < compressThreshold && compressThreshold > GROW_SIZE ) {
					if ( !compress () )
						return true;
				}
				return true;
			}
			return false;
		}


		size_t QueueVector::size ()
		{
			CVerbVerbArg ( "size: [ %i ]", ( int ) size_ );

			return size_;
		}

        
        
        template <class T>
        QueueVectorSP<T>::QueueVectorSP ()
        {
            CVerb ( "Construct" );
            
            size_   = 0;
            items   = 0;
            next    = 0;
            end     = 0;

            capacity			= 0; 
			compressThreshold	= 0;
        }
        
        
        template <class T>
        QueueVectorSP<T>::~QueueVectorSP ()
        {
            CVerb ( "Destruct" );
            
            while ( size_ > 0 )
                front ();
            
            free ( items );
        }
        
        
        template <class T>
        bool QueueVectorSP<T>::push ( const T & item )
        {
            CVerbVerbArg ( "push: Elements in the queue [ %i ].", size_ );
            
            if ( items == 0 ) {
                items = ( QueueVectorSPProxy<T> ** ) calloc ( 1, GROW_SIZE * sizeof ( QueueVectorSPProxy<T> * ) );
                if ( !items )
                    return false;
                
				compressThreshold = capacity = GROW_SIZE;
            }
            else if ( size_ >= capacity ) {
                // Grow the buffer
				if ( !grow () )
					return false;
            }
            
            QueueVectorSPProxy<T> * proxy = new QueueVectorSPProxy<T>  ();
            if ( !proxy )
                return false;
            
            proxy->item = item;
            
            items [ end ] = proxy;
            
            end++;
            ++size_;
            
            if ( end >= capacity )
                end = 0;
            return true;
        }
        
        
        template <class T>
        T QueueVectorSP<T>::front ()
        {
			if ( size_ <= 1 )
			{
				if ( compressThreshold > GROW_SIZE ) {
					if ( !compress () )
						return nullptr;
				}
				if ( size_ <= 0 )
					return nullptr;
			}
            
            QueueVectorSPProxy<T> * proxy = items [ next ];
            
            items [ next ] = 0;
            next++;
            
            if ( next >= capacity )
                next = 0;
            
            --size_;
            
            T item;
            
            if ( proxy ) {
                item = proxy->item;
                
                delete proxy;
            }
            
            return item;
        }
        
        template <class T>
        void QueueVectorSP<T>::pop ()
        {
        }
    
        
        template <class T>
        bool QueueVectorSP<T>::empty ()
        {
			if ( size_ <= 0 )
			{
				if ( size_ < compressThreshold && compressThreshold > GROW_SIZE ) {
					if ( !compress () )
						return true;
				}
				return true;
			}
			return false;
        }
        
        
        template <class T>
        size_t QueueVectorSP<T>::size ()
        {
            CVerbVerbArg ( "size: [ %i ]", ( int ) size_ );
            
            return size_;
        }


		template <class T>
		bool QueueVectorSP<T>::grow ()
		{
			// Grow the buffer
			QueueVectorSPProxy<T> ** tmp = ( QueueVectorSPProxy<T> ** ) calloc ( 1, ( capacity + GROW_SIZE ) * sizeof ( QueueVectorSPProxy<T> * ) );
			if ( !tmp )
				return false;

			// copy from next to end
			if ( next == end ) {
				// This must be true
				if ( !next ) {
					memcpy ( tmp, items, capacity * sizeof ( QueueVectorSPProxy<T> * ) );
				}
				else {
					size_t toCopy = capacity - next;

					memcpy ( tmp, items + next, toCopy * sizeof ( QueueVectorSPProxy<T> * ) );

					memcpy ( tmp + toCopy, items, next * sizeof ( QueueVectorSPProxy<T> * ) );
				}
			}
			else {
				free ( tmp );
				return false;
			}

			compressThreshold = capacity; // Do not compress at all

			end = capacity;
			next = 0;

			free ( items );
			items = tmp;

			capacity += GROW_SIZE;

			return true;
		}


		template <class T>
		bool QueueVectorSP<T>::compress ()
		{
			// Grow the buffer
			size_t compCap = GROW_SIZE;
			if ( size_ >= compCap ) {
				compCap = capacity - GROW_SIZE;
			}

			QueueVectorSPProxy<T> ** tmp = ( QueueVectorSPProxy<T> ** ) calloc ( 1, compCap * sizeof ( QueueVectorSPProxy<T> * ) );
			if ( !tmp )
				return false;

			if ( size_ > 0 ) {
				// copy from next to end
				if ( next < end ) {
					memcpy ( tmp, items + next, size_ * sizeof ( void * ) );
				}
				else {
					size_t count = capacity - next;

					memcpy ( tmp, items + next, count * sizeof ( void * ) );

					if ( end > 0 )
						memcpy ( tmp + count, items, end * sizeof ( void * ) );
				}
				end = size_;
			}
			else
				end = 0;
			next = 0;

			capacity = compCap;

			if ( ( ( int ) capacity - GROW_SIZE ) > GROW_SIZE )
				compressThreshold = capacity - GROW_SIZE;
			else
				compressThreshold = GROW_SIZE; // Keep at least the grow size as cache

			free ( items );
			items = tmp;

			return true;
        }
        
#ifndef MEDIATORDAEMON
        class MessageInstance;
        template class QueueVectorSP < std::shared_ptr < environs::lib::MessageInstance > >;
#endif
	}
}

#endif






