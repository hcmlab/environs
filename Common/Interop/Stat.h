/**
 *	Interop stat declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_STAT_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_STAT_H


#ifdef _WIN32
#include <sys/stat.h>
#define stat(a,b)			_stat(a,b)
#define STAT_STRUCT(a)		struct _stat a;

#include <direct.h>
#define mkdir(dir,mode)		_mkdir(dir)
#define unlink(filepath)	_unlink(filepath)

#else /// _WIN32

#include <unistd.h>
#include <sys/stat.h>

#ifdef ANDROID
//#include <stl/config/_android.h>
#endif /// end->ANDROID

#define STAT_STRUCT(a)		struct stat a;

#endif /// end->_WIN32

#if (!defined(_POSIX_SOURCE) && !defined(_WIN32))
#define unixEpoch(a)		a.st_mtimespec.tv_sec
#else
#define unixEpoch(a)		a.st_mtime
#endif

#endif /// INCLUDE_HCM_ENVIRONS_INTEROP_STAT_H
