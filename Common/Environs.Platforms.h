/**
 * Common native declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORMS_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORMS_H

#if (defined(_WIN32) && !defined(CLI_CPP))
#   if defined(ENVIRONS_CORE_LIB)
#       define ENVIRONS_LIB_API   __declspec(dllexport)
#   else
#       define ENVIRONS_LIB_API   __declspec(dllimport)
#   endif
#else
#   define ENVIRONS_LIB_API
#endif

#ifdef CLI_CPP
#	define		EnvironsOBJ			environs::Environs ^
#	define		EnvironsOBJInst		environs::Environs::instancesAPI
#else
#	define		EnvironsOBJ			Environs *
#	define		EnvironsOBJInst		instancesAPI

#	define		GetSensorInputPack(f)	(environs::SensorFrame *) f
#endif

/**
 * Platform detectors
 */
#ifndef ENVIRONS_IOS

#   if (defined(_WIN32))
#		if (!defined(CLI_CPP) && !defined(WINDOWS_PHONE))
#			include "WinDef.h"
#		endif
#   else
#       define APIENTRY
#   endif

#   ifndef LINUX
#       if ( defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1 )
#           ifndef ENVIRONS_IOS
#               define ENVIRONS_IOS
#           endif
#       else
#           if ( defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR == 1 )
#               ifndef ENVIRONS_IOS
#                   define ENVIRONS_IOS
#               endif
#           else
#               if ( defined(TARGET_OS_MAC) && TARGET_OS_MAC == 1 )
#                   if  ( !defined(ENVIRONS_OSX) && !defined(LINUX) )
#                       define ENVIRONS_OSX
#                   endif
#               endif
#           endif
#       endif
#   endif
#endif


#endif  // -> INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORMS_H