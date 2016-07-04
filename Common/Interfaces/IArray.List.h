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
#ifndef INCLUDE_HCM_ENVIRONS_INTERFACES_ARRAYLIST_H
#define	INCLUDE_HCM_ENVIRONS_INTERFACES_ARRAYLIST_H

#include "Interfaces/IEnvirons.Dispose.h"

namespace environs
{
	/**
	* ArrayList
	*
	*/
	class ArrayList : public lib::IEnvironsDispose
	{
	public:
		const char * listType;

		/** Constructor */
		ArrayList () : listType ( "IArrayList" ) { };

		virtual ~ArrayList () { };


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		virtual void Release () = 0;


		/**
		* Get the size of the list.
		*
		* @return size of the list
		*/
		virtual size_t size () = 0;


		/**
		* Get the item at the given position.
		*
		* @param position
		*
		* @ return The object at given position
		*/
        virtual void * item ( size_t pos ) = 0;


        /**
         * Get the item at the given position and increase reference count by one.
         *
         * @param position
         *
         * @ return The object at given position
         */
        virtual void * itemRetained ( size_t pos ) = 0;
    };
}
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H








