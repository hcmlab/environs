/**
 * Environs large display platform commons
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
#ifndef INCLUDE_HCM_ENVIRONS_LARGE_DISPLAY_PLATFORM_COMMON_H
#define INCLUDE_HCM_ENVIRONS_LARGE_DISPLAY_PLATFORM_COMMON_H

#ifdef DISPLAYDEVICE

#include "Environs.OSX.h"
#include "Environs.Windows.h"


namespace environs
{
    class Instance;
    
    extern bool				opt_useDeviceMarkerHandler;
	extern bool				opt_useDeviceMarkerReducedPrecision;
    
	extern unsigned int		g_rawImageMemorySize;
	extern unsigned int		g_rawImageMemoryWidth;
	extern unsigned int		g_rawImageMemoryHeight;
	extern unsigned int		g_rawImageMemoryStride;
	extern char			*	g_rawImageMemory;
	
    bool SaveConfig ();
    void DeallocNativePlatform ();

	namespace API {
        
	}

#define onEnvironsInput(hEnv,nativeID,input)		hEnv->callbacks.OnHumanInput ( hEnv->hEnvirons, nativeID, input )

} /* namespace environs */

#endif


#endif  /// end-INCLUDE_HCM_ENVIRONS_LARGE_DISPLAY_PLATFORM_COMMON_H



