/**
 * Build commit output
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
#include "Environs.Release.h"
#include "Environs.Commit.h"

#define CLASS_NAME	"Environs.Build . . . . ."

namespace environs
{
	void LogBuildCommit ()
	{        
        CLog ( BUILD_VERSION_STRING );

        CLog ( "Build from commit: " ENVIRONS_TOSTRING(BUILD_COMMIT) );
	}
}
