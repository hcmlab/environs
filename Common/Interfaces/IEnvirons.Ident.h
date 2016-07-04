/**
 * Environs Interface Identification
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
#include "Interfaces.h"
#include "Environs.Types.h"

#ifndef INCLUDE_HCM_ENVIRONS_INTERFACE_IDENTIFICATION_H
#define	INCLUDE_HCM_ENVIRONS_INTERFACE_IDENTIFICATION_H

#ifdef __cplusplus

namespace environs
{
    class Instance;
    
    
	/**
	* IEnvironsIdent
	*
	*/
	class IEnvironsIdent
	{
	public:
		/** Constructor */
		IEnvironsIdent ( InterfaceType_t type ) :
			/**
			 * Initialize interface members
			 * */
			 interfaceType ( type ), name ( "Unknown" ), enabled ( false ), initialized ( false ), deviceID ( 0 ), 
			 modName ( 0 ), hModLib ( 0 ), env ( 0 )
		{};

		virtual ~IEnvironsIdent ( ) {};

		InterfaceType_t		interfaceType;

		const char 			*	name;
		bool					enabled;
		bool					initialized;

		int                     deviceID;
		
		char 				*	modName;
		void				*	hModLib;
		Instance            *	env;
	};
}

#endif

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACE_IDENTIFICATION_H








