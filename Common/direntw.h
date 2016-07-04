/**
* Dirent basic wrapper that includes either dirent.h (POSIX) 
* or provides wrapping minimalistic dirent functionality around win32
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

#ifndef INCLUDE_HCM_ENVIRONS_DIRENT_WRAPPER_H
#define INCLUDE_HCM_ENVIRONS_DIRENT_WRAPPER_H

#include "Interop.h"

// Declare minimalistic dirent for windows platforms.
#ifdef CLI_CPP
#	define _DIRENT_HAVE_D_NAMLEN

	STRUCT DIR;
#else
	typedef struct DIR DIR;
#endif

PUBLIC_STRUCT dirent
{
	unsigned char	d_type;
	String_ptr		d_name;

#ifdef CLI_CPP
	int				d_namlen;
#endif
};

extern DIR           OBJ_ptr	opendir		( CString_ptr );
extern int						closedir	( DIR OBJ_ptr );
extern STRUCT dirent OBJ_ptr	readdir		( DIR OBJ_ptr );

#define DT_UNKNOWN	0	// unknown
#define DT_DIR		4	// directory
#define DT_REG		8	// regular file

#endif // INCLUDE_HCM_ENVIRONS_UNISTD_MINIMAL_H

#else

// Include POSIX header for non-windows platforms.
#include <dirent.h>

#endif // INCLUDE_HCM_ENVIRONS_DIRENT_WRAPPER_H
