/**
 * WifiList Interface Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_WIFILIST_H
#define	INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_WIFILIST_H

#ifndef CLI_CPP

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IWifi.List.h"

namespace environs
{
	namespace lib
	{
        class Environs;

		/**
		* WifiList
		*
		*/
		class WifiListInstance : public environs::WifiList
		{
            friend class Environs;

		public:
			/** Constructor */
			ENVIRONS_LIB_API WifiListInstance ();

			ENVIRONS_LIB_API ~WifiListInstance ();


			/**
			 * Get the item at the given position.
			 *
			 * @param position
			 *
			 * @ return The object at given position
			 */
            ENVIRONS_LIB_API WifiEntry * item ( size_t pos );


            /**
             * Get the item at the given position and increase reference count by one.
             *
             * @param position
             *
             * @ return The object at given position
             */
            ENVIRONS_LIB_API WifiEntry * itemRetained ( size_t pos );


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


            static WifiListInstance * CreateWithWifisRetained ( char * data );

            size_t			size_;
            size_t			capacity;
            char		*	data;
			WifiEntry	*	items;

		};
	}
}

#endif
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H








