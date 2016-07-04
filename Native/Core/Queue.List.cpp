/**
 * QueueList
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
#include "Queue.List.h"
#include "Device.Instance.h"

#define CLASS_NAME	"Queue.List . . . . . . ."

#define GROW_SIZE 50

namespace environs
{
	namespace lib
	{

		QueueList::QueueList ()
		{
			CVerb ( "Construct" );

			size_ = 0;
			front = 0;
			end = 0;
		}


		QueueList::~QueueList ()
		{
			CVerb ( "Destruct" );
		}


		bool QueueList::push ( IQueueItem * item )
		{
			CVerbArg ( "push: Elements in the queue [ %i ].", size_ );

			item->next = 0;

			if ( !front ) {
				front = item; end = item; ++size_;
			}
			else if ( end ) {
				end->next = item; end = item; ++size_;
			}
			else {
				CErrArg ( "push: Failed. Queue size [ %i ] - Start [ %p ] - End [ %p ]", size_, front, end );
				return false;
			}
			return true;
		}


		void * QueueList::pop ()
		{
			IQueueItem * item = front;

			if ( front ) {
				front = front->next;
				if ( !front )
					end = 0;
				--size_;
			}

			return item;
		}


		bool QueueList::empty ()
		{
			return ( front == 0 );
		}


		size_t QueueList::size ()
		{
			CVerbVerbArg ( "size: [ %i ]", ( int ) size_ );

			return size_;
		}


#ifndef NDEBUG
		bool QueueList::remove ( IQueueItem * obj )
		{
			IQueueItem * item = front;
			IQueueItem * prev = item;

			while ( item )
			{
				if ( item == obj ) {
					if ( item == front ) {
						front = front->next;
						if ( !front )
							end = 0;
						--size_;
						return true;
					}
					else if ( prev ) {
						prev->next = item->next;
						--size_;
						return true;
					}
					else
						return false;
				}
				prev = item;
				item = item->next;
			}
			return false;
		}
#endif
        
        
#ifdef ENABLE_VECTOR_LIST
        
        
        template <class T>
        VectorListSP<T>::VectorListSP ()
        {
            CVerb ( "Construct" );
            
            root    = 0;
            last    = 0;
            items   = 0;
            next    = 0;
            end     = 0;
            size_   = 0;
            
            sizeArray = 0;
            capacity  = 0;
        }
        
        
        template <class T>
        VectorListSP<T>::~VectorListSP ()
        {
            CVerb ( "Destruct" );
            
            for ( size_t i = 0; i < capacity; ++i )
            {
                VectorListSPProxy<T> * proxy = items [ i ];
                                
                if ( proxy ) {
                    delete proxy;
                }
            }
            
            free ( items );
        }
        
        
        template <class T>
        bool VectorListSP<T>::push ( const T & item )
        {
            CVerbArg ( "push: Elements in the queue [ %i ].", size_ );
            
            
            VectorListSPProxy<T> * proxy = new VectorListSPProxy<T>  ();
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
        bool VectorListSP<T>::alloc ( size_t start, size_t count )
        {
            size_t lastPos = start + count;
            
            for ( ; start < lastPos; ++start )
            {
                VectorListSPProxy<T> * proxy = new VectorListSPProxy<T> ();
                if ( !proxy )
                    return false;
                
                items [ start ] = proxy;
            }
            return true;
        }
        
        
        template <class T>
        VectorListSPProxy<T> * VectorListSP<T>::front ()
        {
            if ( size_ >= capacity )
            {
                if ( items == 0 ) {
                    items = ( VectorListSPProxy<T> ** ) calloc ( 1, GROW_SIZE * sizeof ( VectorListSPProxy<T> * ) );
                    if ( !items )
                        return 0;
                    capacity = GROW_SIZE;
                    
                    if ( !alloc ( 0, capacity ) )
                        return 0;
                }
                else if ( size_ >= capacity ) {
                    // Grow the buffer
                    VectorListSPProxy<T> ** tmp = ( VectorListSPProxy<T> ** ) calloc ( 1, ( capacity + GROW_SIZE ) * sizeof ( VectorListSPProxy<T> * ) );
                    if ( !tmp )
                        return 0;
                    
                    // copy from front to end
                    if ( next == end ) {
                        // This must be true
                        if ( !next ) {
                            memcpy ( tmp, items, capacity * sizeof ( VectorListSPProxy<T> * ) );
                        }
                        else {
                            size_t toCopy = capacity - next;
                            
                            memcpy ( tmp, items + next, toCopy * sizeof ( VectorListSPProxy<T> * ) );
                            
                            memcpy ( tmp + toCopy, items, next * sizeof ( VectorListSPProxy<T> * ) );
                        }
                        
                        
						if ( !alloc ( capacity, GROW_SIZE ) ) {
							free ( tmp );
							return 0;
						}
                    }
                    
                    end = capacity;
                    next = 0;
                    
                    free ( items );
                    items = tmp;
                    capacity += GROW_SIZE;
                }
                
            }
            
            VectorListSPProxy<T> * proxy = items [ next ];
            
            next++;
            
            if ( next >= capacity )
                next = 0;
            
            --size_;
            
            
            return proxy;
        }
        
        
        template <class T>
        bool VectorListSP<T>::empty ()
        {
            return ( size_ == 0 );
        }
        
        
        template <class T>
        size_t VectorListSP<T>::size ()
        {
            CVerbVerbArg ( "size: [ %i ]", ( int ) size_ );
            
            return size_;
        }
        
        
        template class VectorListSP < std::shared_ptr < environs::lib::DeviceInstance > >;
#endif
        
	}
}


#endif




