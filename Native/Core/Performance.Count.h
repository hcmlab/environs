/**
 * Performance count
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
#ifndef INCLUDE_HCM_ENVIRONS_PERFORMANCE_COUNT_H
#define INCLUDE_HCM_ENVIRONS_PERFORMANCE_COUNT_H

#ifdef PERFORMANCE_MEASURE
#include "Interfaces/IPortalRenderer.h"

namespace environs
{
	extern  unsigned int		averageOverall;
	extern  unsigned int		averageOverallCount;
	extern  unsigned int		averageCapture;
	extern  unsigned int		averageCaptureCount;
	extern  unsigned int		averageRender;
	extern  unsigned int		averageRenderCount;
	extern  unsigned int		averageEncode;
	extern  unsigned int		averageEncodeCount;
	extern  unsigned int		averageSend;
	extern  unsigned int		averageSendCount;


	extern inline void pc_initPerformanceCounter ();
	extern inline void pc_MeasureInit ( PortalContext * ctx );
	extern inline void pc_MeasureDone ( PortalContext * ctx );
	extern inline void pc_MeasureSend ( PortalContext * ctx );
	extern inline void pc_MeasureEncode ( PortalContext * ctx );
	extern inline void pc_MeasureRender ( PortalContext * ctx );
	extern inline void pc_MeasureCapture ( PortalContext * ctx );

}

#else

#define pc_initPerformanceCounter()
#define pc_MeasureInit(a)
#define pc_MeasureDone(a)
#define pc_MeasureSend(a)
#define pc_MeasureEncode(a)
#define pc_MeasureRender(a)
#define pc_MeasureCapture(a)

#endif

#endif	/// INCLUDE_HCM_ENVIRONS_PERFORMANCE_COUNT_H