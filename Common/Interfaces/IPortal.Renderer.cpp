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
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Interfaces/IPortal.Renderer.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"IPortalRenderer"

namespace environs
{	
	bool IPortalRenderer::Init ( int _deviceID, void * _parent, void * appWindow ) {
		deviceID		= _deviceID;
		parent			= _parent;
		hAppWindow		= appWindow;

		if ( !_parent )
			return false;

		return Init ( );
	}
}
