/**
 * Environs Base Interface declarations
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
#include "IEnvirons.Ident.h"

#ifndef INCLUDE_HCM_ENVIRONS_BASE_INTERFACE_H
#define	INCLUDE_HCM_ENVIRONS_BASE_INTERFACE_H

#define DeclareEnvironsInterface(interfacename) EnvironsInterface interfacename : implements IEnvironsBase { \
	public:

#define DeclareEnvironsIdentInterface(interfacename) EnvironsInterface interfacename : implements IEnvironsIdent { \
	public:


#ifdef __cplusplus


namespace environs
{
	/**
	* IEnvironsBase
	*
	*/
	class IEnvironsBase : public IEnvironsIdent
	{
	public:
		/** Constructor */
		IEnvironsBase ( InterfaceType_t type ) : IEnvironsIdent ( type ),
			/**
			* Initialize interface members
			* */
			outputType ( PortalBufferType_Unknown ), outputTypes ( 0 ), outputTypesLength ( 0 ),
			inputType ( PortalBufferType::Unknown ), inputTypes ( 0 ), inputTypesLength ( 0 ) 
		{};

		virtual ~IEnvironsBase () {};


		PortalBufferType_t		outputType;
		PortalBufferType_t	*	outputTypes;
		unsigned int							outputTypesLength;
		virtual bool							ApplyOutput () { return true; };

		PortalBufferType_t		inputType;
		PortalBufferType_t	*	inputTypes;
		unsigned int							inputTypesLength;
		virtual bool							ApplyInput () { return true; };

	};

	/// Connector method that tries to find a matching output/input pair between compatible modules
	extern bool ConnectModules ( IEnvironsBase * src, IEnvironsBase * dst );

}

#endif



#endif	/// -> INCLUDE_HCM_ENVIRONS_BASE_INTERFACE_H








