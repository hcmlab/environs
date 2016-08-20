/**
 * BtList Interface Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_BTLIST_H
#define	INCLUDE_HCM_ENVIRONS_IMPLEMENTATION_BTLIST_H

#ifndef CLI_CPP

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IBt.List.h"

namespace environs
{
	namespace lib
	{
        class Environs;

		/**
		* BtList
		*
		*/
		class BtListInstance : public environs::BtList
		{
            friend class Environs;

		public:
			/** Constructor */
			ENVIRONS_LIB_API BtListInstance ();

			ENVIRONS_LIB_API ~BtListInstance ();


			/**
			 * Get the item at the given position.
			 *
			 * @param position
			 *
			 * @ return The object at given position
			 */
            ENVIRONS_LIB_API BtEntry * item ( size_t pos );


            /**
             * Get the item at the given position and increase reference count by one.
             *
             * @param position
             *
             * @ return The object at given position
             */
            ENVIRONS_LIB_API BtEntry * itemRetained ( size_t pos );


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


            static BtListInstance * CreateWithBtsRetained ( char * data );

            size_t			size_;
            size_t			capacity;
            char		*	data;
			BtEntry		*	items;

		};
	}
}

#endif
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H








