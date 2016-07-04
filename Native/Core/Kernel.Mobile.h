/**
 * Kernel for Mobile Devices
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
#ifndef INCLUDE_HCM_ENVIRONS_KERNEL_MOBILES_H
#define INCLUDE_HCM_ENVIRONS_KERNEL_MOBILES_H

#ifndef DISPLAYDEVICE

#include "Core.h"
#include "Device/Device.Mobile.h"
#include "Touch.Source.h"


namespace environs 
{
	class KernelDevice : public Core
	{
        friend class Kernel;
        
	public:
        KernelDevice ( void );
		virtual ~KernelDevice ( void );

		static void             ReleaseLibrary ();
        
        static void             InitStatics ();
        static void             ReleaseStatics ();

	private:

		int                     onInitialized ();

		int                     onPreStop ();
		int                     onStopped ();
	};

} /* namespace environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_KERNEL_MOBILES_H
