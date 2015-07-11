/**
 * Interface for a portal renderer (create source images)
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
//#define DEBUGVERB
//#define DEBUGVERBVerb

#include "Environs.h"
#include "Environs.Native.h"
#include "Interop.h"
#include "Interfaces/IEnvirons.Base.h"

#define CLASS_NAME	"IEnvironsBase"


namespace environs
{
	bool ConnectModules ( IEnvironsBase * src, IEnvironsBase * dst )
	{
		unsigned int deviceID = src->deviceID;

		CVerbID ( "Connect" );

		CVerbArgID ( "Connect: Try connecting output of [%s] with input of [%s]", src->name, dst->name );


		for ( unsigned int i = 0; i < dst->inputTypesLength; i++ )
		{
			for ( unsigned int j = 0; j < src->outputTypesLength; j++ )
			{
				if ( dst->inputTypes [i] == src->outputTypes [j] )
				{
					src->outputType = src->outputTypes [j];
					dst->inputType = src->outputType;

					CVerbID ( "Connect: successfully connected." );
					return true;
				}
			}
		}

		CErrArgID ( "Connect: Failed connecting output of [%s] with input of [%s]", src->name, dst->name );
		return false;
	}
}
