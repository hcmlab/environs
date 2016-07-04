/**
* Environs build options
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
#ifndef INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_LIBRARY_LINKER_H
#define INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_LIBRARY_LINKER_H


#if ( defined(_WIN32) && !defined(NDEBUG) )


#	ifdef _USE_VLD
#		ifdef _C_X64
#			ifdef _M_X64
#				pragma comment(lib, "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib")
#			else
#				pragma comment(lib, "C:/Program Files (x86)/Visual Leak Detector/lib/Win32/vld.lib")
#			endif
#		else
#			pragma comment(lib, "C:/Program Files/Visual Leak Detector/lib/Win32/vld.lib")
#		endif
#	endif
#endif


#endif // INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_LIBRARY_LINKER_H


