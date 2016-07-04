/**
 * External Thread Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_IEXTTHREAD_H
#define INCLUDE_HCM_ENVIRONS_IEXTTHREAD_H

#include "IEnvirons.Base.h"
#include "Interface.Exports.h"
#include "Core/Callbacks.h"


namespace environs 
{
	/**
	*	Interface for Environs sensors
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsIdentInterface ( IExtThread )

	public:
		/** Default constructor */
		IExtThread ( ) :
			/** Base class initialization */
			IEnvironsIdent ( InterfaceType::ExtThread ),

			/** Default initialization */
            hEnvirons ( 0 )
		{};

		virtual ~IExtThread ( ) {};

		/** Interface initializer. */
		virtual int			Init ( ) { return true; };

		virtual bool		ThreadFunc ( void * obj ) { return false; };
						
        int                 hEnvirons;

	protected:

		
	EndInterface


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_IEXTTHREAD_H
