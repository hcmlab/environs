/**
* unistd that includes either unistd.h (POSIX) 
* or provides wrapping minimalistic help for win32
* --------------------------------------------------------------------
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

#ifdef _WIN32

#ifndef INCLUDE_HCM_ENVIRONS_UNISTD_MINIMAL_H
#define INCLUDE_HCM_ENVIRONS_UNISTD_MINIMAL_H

#include <io.h>

// For access
#define	F_OK		0
#define	X_OK		0x01
#define	W_OK		0x02
#define	R_OK		0x04

#endif // INCLUDE_HCM_ENVIRONS_UNISTD_MINIMAL_H

#else

// Include POSIX header for non-windows platforms.
#include <unistd.h>

#define	_unlink(a)	unlink(a)
#define	_access(a,b)	access(a,b)

#endif

