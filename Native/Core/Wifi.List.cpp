/**
 * WifiList Implementation
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
#include "Core/Wifi.List.h"

using namespace std;


#define CLASS_NAME	"Wifi.List. . . . . . . ."

#define GROW_SIZE	6


namespace environs
{
	namespace lib
	{
		ENVIRONS_OUTPUT_ALLOC ( WifiListInstance );


		WifiListInstance::WifiListInstance ()
		{
			CVerbArg ( "Construct", listType );

			size_       = 0;
            capacity    = 0;
			items       = nill;
			data		= nill;

			ENVIRONS_OUTPUT_ALLOC_INIT ();
		}


		WifiListInstance::~WifiListInstance ()
		{
			CVerbArg ( "Destruct [ %i ]: [ %s ]", objID_, listType );

            if ( items != nill )
                delete [] items;

            if ( data != nill )
                free ( data );
		}


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		void WifiListInstance::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE ();

			if ( localRefCount == 0 )
				delete this;
		}


		/**
		* Internal Release used by Environs to clean up internal resources while object is locked.
		*
		*/
		void WifiListInstance::ReleaseLocked ()
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
        WifiEntry * WifiListInstance::item ( size_t pos )
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
        WifiEntry * WifiListInstance::itemRetained ( size_t pos )
        {
            CVerbVerbArg ( "item [ %s ]: at [ %i ]", listType, ( int ) pos );

            return nill;
        }


        /**
         * Get the size of the list.
         *
         * @return size of the list
         */
        size_t WifiListInstance::size ()
		{
			CVerbVerbArg ( "size [%s]: [%i]", listType, ( int ) size_ );

			return size_;
		}


        WifiListInstance * WifiListInstance::CreateWithWifisRetained ( char * data )
        {
            if ( !data )
                return nill;

			char			* wifis = data;
			unsigned int	* sizes = reinterpret_cast< unsigned int * > ( data );

            unsigned int count = *sizes;
            if ( count <= 0 || count > 256 )
                return nill;

			wifis += 4;

			unsigned int remainSize = *( sizes + 1 );
			if ( remainSize < sizeof ( WifiItem ) )
				return nill;

            WifiListInstance * wl = new WifiListInstance;
            if ( wl )
            {
                WifiEntry * entries = new WifiEntry [ count ];
                while ( entries )
                {
					memset ( entries, 0, sizeof ( WifiEntry ) * count );

                    int         entryCount  = 0;
                    WifiEntry * entry       = entries;
                    char      * wifi        = wifis + 4;

                    for ( unsigned int i = 0; i < count; ++i )
                    {
						if ( remainSize < sizeof ( WifiItem ) )
							break;
                        entry->item = ( WifiItem * ) wifi;

                        wifi		+= sizeof ( WifiItem );
						remainSize  -= sizeof ( WifiItem );

                        unsigned int ssidSize = ( unsigned int ) entry->item->sizeOfssid;

                        if ( ssidSize > 0 ) {
							if ( ssidSize > 32 )
								break;

							if ( remainSize < ssidSize )
								break;
                            entry->ssid = wifi;
							wifi [ ssidSize - 1 ] = 0;

                            wifi		+= ssidSize;
							remainSize  -= ssidSize;
                        }

						entryCount++;
						entry++;
                    }

                    if ( entryCount <= 0 ) {
                        delete[] entries;
                        entries = nill;
                        break;
                    }

                    wl->data = data;
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






