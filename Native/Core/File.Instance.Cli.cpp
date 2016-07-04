/**
 * FileInstance CLI Object
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

#ifdef _WIN32

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Environs.Cli.Forwards.h"
#include "File.Instance.Cli.h"
#include "Environs.Native.h"

#define CLASS_NAME	"File.Instance.Cli. . . ."


namespace environs
{
	FileInstance::FileInstance ()
	{
		CVerbVerb ( "Construct" );
	}


	FileInstance::~FileInstance ()
	{
		CVerbVerbArg1 ( "Destruct", "", "i", objID_ );
	}


	CString_ptr FileInstance::ToString ()
	{
		CVerbVerb ( "ToString" );

		return toString ();
	}
}


#endif // _WIN32





