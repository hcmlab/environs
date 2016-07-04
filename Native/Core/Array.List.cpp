/**
 * ArrayList Implementation
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

#include "Environs.Native.h"
#include "Device.List.h"
#include "Device.Instance.h"
#include "File.Instance.h"
#include "Core/Array.List.h"
#include "Portal.Instance.h"

using namespace std;


#define CLASS_NAME	"Array.List . . . . . . ."

#define GROW_SIZE	6


namespace environs
{
	namespace lib
	{
		ENVIRONS_OUTPUT_ALLOC ( ArrayList );


		ArrayList::ArrayList ()
		{
			CVerbArg ( "Construct", listType );

			size_       = 0;
            capacity    = 0;
			items       = nill;

			ENVIRONS_OUTPUT_ALLOC_INIT ();
		}


		ArrayList::~ArrayList ()
		{
			CVerbArg ( "Destruct [ %i ]: [ %s ]", objID_, listType );
		}


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		void ArrayList::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE ();

			if ( localRefCount == 0 )
				delete this;
		}


		/**
		* Internal Release used by Environs to clean up internal resources while object is locked.
		*
		*/
		void ArrayList::ReleaseLocked ()
		{
			CVerbArg ( "ReleaseLocked [ %i ]: [ %s ]", objID_, listType );
            
#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
			bool isDevList = (strcmp ( listType, "IDeviceInstanceList" ) == 0);
#endif
			void ** toDispose = items;

			if ( toDispose != nill ) {
				items = nill;

				CVerbVerbArg ( "ReleaseLocked [ %s ]: Releaseasing [ %i ] devices", listType, ( int ) size_ );

				for ( size_t i = 0; i < size_; ++i )
				{
					IEnvironsDispose * item = ( IEnvironsDispose * ) toDispose [ i ];
					if ( item )
                    {
                        CVerbVerbArg ( "ReleaseLocked [ %s ]: Release [ %i ]", listType, ( int ) i );
#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
						if ( isDevList ) {
							DeviceInstance * dev = (DeviceInstance *) item;

							LONGSYNC r = __sync_sub_and_fetch ( &dev->atLists, 1 );
							if ( r > 0 ) {
								dev->CheckSPConsistency1 ( r );
							}
						}
#endif
						item->Release ();
					}
				}
				free ( toDispose );
			}

            capacity    = 0;
            size_       = 0;
			deviceList  = nill;
			messageList = nill;
			fileList    = nill;
		}

        
        /**
         * Get the item at the given position.
         *
         * @param position
         *
         * @ return The object at given position
         */
        void * ArrayList::item ( size_t pos )
		{
			CVerbVerbArg ( "item [ %s ]: at [ %i ]", listType, ( int ) pos );

			if ( pos >= size_ )
				return nill;

			return items [ pos ];
        }


        /**
         * Get the item at the given position and increase reference count by one.
         *
         * @param position
         *
         * @ return The object at given position
         */
        void * ArrayList::itemRetained ( size_t pos )
        {
            CVerbVerbArg ( "item [ %s ]: at [ %i ]", listType, ( int ) pos );

            if ( pos < size_ ) {
                IEnvironsDispose * item = ( IEnvironsDispose * ) items [ pos ];

#ifndef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
                if ( item && item->Retain () )
                    return item;
#else
                if ( item ) {
					DeviceInstance * dev = (DeviceInstance *)item;

					dev->CheckSPConsistency ();

					if ( item->Retain () )
						return item;
				}
#endif
            }
            return nill;
        }


        /**
         * Get the size of the list.
         *
         * @return size of the list
         */
        size_t ArrayList::size ()
		{
			CVerbVerbArg ( "size [%s]: [%i]", listType, ( int ) size_ );

			return size_;
		}


		ArrayList * ArrayList::CreateWithDevicesRetained ( const svsp ( DeviceInstance ) &listSP )
		{
			CVerbVerb ( "CreateWithDevicesRetained" );

			vsp ( DeviceInstance ) * list = listSP.get ();

			if ( list == nill )
				return nill;

			size_t size = list->size ();
			if ( !size )
				return nill;

			CVerbArg ( "CreateWithDevices: [ %i ]", ( int ) size );

			ArrayList * al = new ArrayList;
			while ( al != nill )
			{
				al->listType = "IDeviceInstanceList";

				if ( !al->Retain () )
				{
					delete al; al = nill; break;
				}

				al->items = ( void ** ) calloc ( 1, size * sizeof ( void * ) );
				if ( al->items == nill )
				{
					delete al; al = nill; break;
				}

				size_t j = 0;

				for ( size_t i = 0; i < size; ++i )
				{
                    const sp ( DeviceInstance ) &itemSP = list->at ( i ); // Should be safe as we must have a lock on the list
					//sp ( DeviceInstance ) itemSP = list->at ( i );

					IEnvironsDispose * item = ( IEnvironsDispose * ) itemSP.get ();
					if ( item )
					{
                        if ( item->Retain () ) {
#ifdef ENABLE_DISPOSER_DEVICEINSTANCE_CONSISTENCY_CHECK
							__sync_add_and_fetch ( &itemSP->atLists, 1 );
#endif
							al->items [ j ] = item;  j++;
						}
					}
				}

				al->capacity    = size;
				al->size_       = j;
				al->deviceList  = listSP;
				break;
			}
			return al;
        }
        
        
        bool ArrayList::UpdateWithDevices ( const svsp ( DeviceInstance ) &listSP )
        {
            CVerbVerb ( "UpdateWithDevices" );
            
			vsp ( DeviceInstance ) * list = listSP.get ();

            if ( list == nill || list->size () <= 0 ) {
                if ( items != nill ) {
                    CVerbVerbArg ( "UpdateWithDevices [ %s ]: Releaseasing [ %i ] devices", listType, ( int ) size_ );
                    
                    for ( size_t i = 0; i < size_; ++i )
                    {
						IEnvironsDispose * item = ( IEnvironsDispose * ) items [ i ];
                        if ( item )
                        {
                            CVerbVerbArg ( "ReleaseLocked [ %s ]: Release [ %i ]", listType, ( int ) i );
							item->Release ();
							items [ i ] = 0;
                        }
                    }
                    size_ = 0;
                }
                return true;
            }
            
            size_t sizeList = list->size ();
            
            CVerbArg ( "UpdateWithDevices: [ %i ]", ( int ) sizeList );
            
            if ( sizeList > capacity ) {
				size_t newCap = sizeList + GROW_SIZE;

                void ** tmp = ( void ** ) calloc ( 1, newCap * sizeof ( void * ) );
                if ( !tmp )
                    return false;

				memcpy ( tmp, items, sizeof ( void * ) * size_ );
                free ( items );
                
                items	 = tmp;
                capacity = newCap;
            }
            
            size_t j = 0;
            
            for ( size_t i = 0; i < sizeList; ++i )
            {
                const sp ( DeviceInstance ) &itemSP = list->at ( i ); // Should be safe as we must have a lock on the list
				//sp ( DeviceInstance ) itemSP = list->at ( i );

                IEnvironsDispose * src = ( IEnvironsDispose * ) itemSP.get ();
                if ( !src )
                    continue;
                
				IEnvironsDispose * item = ( IEnvironsDispose * ) items [ j ];
                
                if ( src == item ) {
                    j++; continue;
                }

				if ( src->Retain () ) {
					items [ j ] = src; j++;
				}
                
                if ( item )
					item->Release ();
            }
            
            size_ = j;
            
            for ( ; j < capacity; ++j )
            {
				IEnvironsDispose * item = ( IEnvironsDispose * ) items [ j ];                
                if ( item ) {
                    item->Release (); items [ j ] = 0;
                }
            }
            
            return true;
        }


		ArrayList * ArrayList::CreateWithMessagesRetained ( const svsp ( MessageInstance ) &listSP, const DeviceInstanceSP device )
		{
			CVerbVerb ( "CreateWithMessagesRetained" );

			vsp ( MessageInstance ) * list = listSP.get ();

			if ( list == nill || !device )
				return nill;

            ArrayList * al = nill;
            
            if ( !device->StorageLock ( true ) )
                return nill;
            
			size_t size = list->size ();
			if ( !size )
                goto Finish;

			CVerbArg ( "CreateWithMessages: [ %i ]", ( int ) size );

			al = new ArrayList;
            if ( al == nill )
                goto Finish;
            
			al->listType = "IMessageList";

			al->items = ( void ** ) calloc ( 1, size * sizeof ( void * ) );
			if ( al->items == nill )
            {
                delete al;
                al = nill;
			}
            else {
                size_t j = 0;
                for ( size_t i = 0; i < size; ++i )
                {
                    const sp ( MessageInstance ) &itemSP = list->at ( i );

                    IEnvironsDispose * item = ( IEnvironsDispose * ) itemSP.get ();
                    if ( item  ) {
                        if ( item->Retain () ) {
                            al->items [ j ] = item; j++;
                        }
                    }
                }
                
                al->size_		= j;
                al->messageList = listSP;
                al->Retain ();
            }

        Finish:
            device->StorageLock ( false );
			return al;
        }


#   ifdef USE_QUEUE_VECTOR_T_CLASS
        ArrayList * ArrayList::CreateWithMessagesQueueRetained ( QueueVectorSP < MessageInstanceESP > &list, const DeviceInstanceSP device )
#   else
        ArrayList * ArrayList::CreateWithMessagesQueueRetained ( stdQueue ( MessageInstanceESP ) &list, const DeviceInstanceSP device )
#   endif
        {
            CVerbVerb ( "CreateWithMessagesRetained" );
            
            if ( stdQueue_empty ( list ) || !device )
                return nill;
            
            ArrayList * al = nill;
            
            if ( !device->StorageLock ( true ) )
                return nill;
            
            size_t size = list.size ();
            if ( !size )
                goto Finish;
            
            CVerbArg ( "CreateWithMessages: [ %i ]", ( int ) size );
            
            al = new ArrayList;
            if ( al == nill )
                goto Finish;
            
            al->listType = "IMessageList";
            
            al->items = ( void ** ) malloc ( size * sizeof ( void * ) );
            if ( al->items == nill )
            {
                delete al;
                al = nill;
            }
            else {
                size_t j = 0;
                
                while ( !stdQueue_empty ( list ) )
                {
                    c_const MessageInstanceESP item = stdQueue_front ( list );
                    
                    stdQueue_pop ( list );
                    
                    IEnvironsDispose * p = ( IEnvironsDispose * ) item.get ();
                    if ( p != nill ) {
                        if ( p->Retain () ) {
                            al->items [ j ] = p;
                            j++;
                        }
                    }
                }
                
                al->size_	= j;
                //al->messageList = list;
                al->Retain ();
            }
            
        Finish:
            device->StorageLock ( false );
            return al;
        }


		ArrayList * ArrayList::CreateWithFilesRetained ( const smsp ( int, FileInstance ) &listSP, const DeviceInstanceSP device )
		{
			CVerbVerb ( "CreateWithFilesRetained" );

			msp ( int, FileInstance ) * list = listSP.get ();

			if ( list == nill || !device )
                return nill;
            
            ArrayList * al = nill;
            
            if ( !device->StorageLock ( true ) )
                return nill;

			size_t size = list->size ();
            if ( !size )
                goto Finish;

			CVerbArg ( "CreateWithFiles: [ %i ]", ( int ) size );

			al = new ArrayList;
            if ( al == nill )
                goto Finish;
            
			al->listType = "IFileList";

			al->items = ( void ** ) malloc ( size * sizeof ( void * ) );
			if ( al->items == nill )
            {
                delete al;
                al = nill;
			}
            else {
                map < int, sp ( FileInstance ) >::iterator it;
                
                size_t i = 0;
                for ( it = list->begin (); it != list->end (); ++it )
                {
                    const sp ( FileInstance ) &fi = it->second;
                    
                    if ( fi == nill ) continue;
                    
                    IEnvironsDispose * p = ( IEnvironsDispose * ) fi.get ();
                    if ( p != nill ) {
                        if ( p->Retain () ) {
                            al->items [ i ] = p;
                            ++i;
                        }
                    }
                }
                
                al->size_	 = i;
                al->fileList = listSP;
                al->Retain ();
            }
            
        Finish:
            device->StorageLock ( false );
			return al;
		}
	}
}






