/**
 * BtList Implementation
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
#include "Core/Bt.List.h"

using namespace std;


#define CLASS_NAME	"Bt.List. . . . . . . . ."

#define GROW_SIZE	6


namespace environs
{
	namespace lib
	{
		ENVIRONS_OUTPUT_ALLOC ( BtListInstance );


		BtListInstance::BtListInstance ()
		{
			CVerbArg ( "Construct", listType );

			size_       = 0;
            capacity    = 0;
			items       = nill;
			data		= nill;

			ENVIRONS_OUTPUT_ALLOC_INIT ();
		}


		BtListInstance::~BtListInstance ()
		{
			CVerbArg ( "Destruct [ %i ]: [ %s ]", objID_, listType );

            if ( items != nill )
                delete [] items;

            if ( data != nill )
                free ( data );

            ENVIRONS_OUTPUT_DISPOSE_OBJLOCK ();
		}


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		void BtListInstance::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE ();

			if ( localRefCount == 0 )
				delete this;
		}


		/**
		* Internal Release used by Environs to clean up internal resources while object is locked.
		*
		*/
		void BtListInstance::ReleaseLocked ()
		{
			CVerbArg ( "ReleaseLocked [ %i ]: [ %s ]", objID_, listType );

            capacity    = 0;
            size_       = 0;
		}

        
        /**
         * Get the item at the given position.
         *
         * @param position
         *
         * @ return The object at given position
         */
        BtEntry * BtListInstance::item ( size_t pos )
		{
			CVerbVerbArg ( "item [ %s ]: at [ %i ]", listType, ( int ) pos );

			if ( pos >= size_ )
				return nill;

			return items + pos;
        }


        /**
         * Get the item at the given position and increase reference count by one.
         *
         * @param position
         *
         * @ return The object at given position
         */
		BtEntry * BtListInstance::itemRetained ( size_t pos )
        {
            CVerbVerbArg ( "item [ %s ]: at [ %i ]", listType, ( int ) pos );

            return nill;
        }


        /**
         * Get the size of the list.
         *
         * @return size of the list
         */
        size_t BtListInstance::size ()
		{
			CVerbVerbArg ( "size [%s]: [%i]", listType, ( int ) size_ );

			return size_;
		}


		BtListInstance * BtListInstance::CreateWithBtsRetained ( char * data )
        {
            if ( !data )
                return nill;

			char			* bts	= data;
			unsigned int	* sizes = reinterpret_cast< unsigned int * > ( data );

            unsigned int count = *sizes;
            if ( count <= 0 || count > 256 )
                return nill;
			if ( count > 256 )
				count = 256;

			bts += 4;

			unsigned int remainSize = *( sizes + 1 );
			if ( remainSize < sizeof ( BtItem ) )
				return nill;

			BtListInstance * wl = new BtListInstance;
            if ( wl )
            {
                ENVIRONS_OUTPUT_INITD_OBJLOCK ( wl );

                BtEntry * entries = new BtEntry [ count ];
                while ( entries )
                {
                    memset ( entries, 0, sizeof ( BtEntry ) * count );

                    int         entryCount  = 0;
					BtEntry   * entry       = entries;
                    char      * bt			= bts + 4;

                    for ( unsigned int i = 0; i < count; ++i )
                    {
						if ( remainSize < sizeof ( BtItem ) )
							break;
                        entry->item = ( BtItem * ) bt;

						bt			+= sizeof ( BtItem );
						remainSize	-= sizeof ( BtItem );

                        unsigned int ssidSize = ( unsigned int ) entry->item->sizeOfssid;

                        if ( ssidSize > 0 ) {
							if ( ssidSize > 32 )
								break;
							
							if ( remainSize < ssidSize )
								break;
                            entry->ssid = bt;
							bt [ ssidSize - 1 ] = 0;

							bt			+= ssidSize;
							remainSize	-= ssidSize;
                        }

						entryCount++;
						entry++;
                    }

                    if ( entryCount <= 0 ) {
						delete [ ] entries;
						entries = nill;
                        break;
                    }

                    wl->data  = data;
                    wl->size_ = entryCount;
                    break;
                }

                if ( !entries ) {
                    delete wl;
                    wl = nill;
                }
                else {
                    wl->items = entries;
					wl->Retain ();
                }
            }

            return wl;
        }
        
	}
}






