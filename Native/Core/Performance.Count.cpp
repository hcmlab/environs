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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"

#define CLASS_NAME	"PerfCount"

namespace environs
{

#ifdef PERFORMANCE_MEASURE
	unsigned int			averageOverall			= 0;
	unsigned int			averageOverallCount		= 0;
	unsigned int			averageCapture			= 0;
	unsigned int			averageCaptureCount		= 0;
	unsigned int			averageRender			= 0;
	unsigned int			averageRenderCount		= 0;
	unsigned int			averageEncode			= 0;
	unsigned int			averageEncodeCount		= 0;
	unsigned int			averageSend				= 0;
	unsigned int			averageSendCount		= 0;

	LARGE_INTEGER		pc_frequency;


	inline void pc_initPerformanceCounter ( )
	{
		QueryPerformanceFrequency ( &pc_frequency );
	}


	inline void pc_MeasureInit ( PortalContext * ctx )
	{
		QueryPerformanceCounter ( & ctx->pc_counterStart );
		ctx->pc_counterLast = ctx->pc_counterStart;
	}


	inline void pc_MeasureDone ( PortalContext * ctx )
	{
		LARGE_INTEGER		curCounter;
		QueryPerformanceCounter ( & curCounter );

		unsigned int ms = (unsigned int)((curCounter.QuadPart - ctx->pc_counterStart.QuadPart) * 1000000 / pc_frequency.QuadPart);
		ctx->averageFrame += ms;
		ctx->averageFrameCount ++;

		PortalContext::averageOverall += ms;
		PortalContext::averageOverallCount ++;

		unsigned int avg = ctx->averageFrame / ctx->averageFrameCount;
		unsigned int avgAll = PortalContext::averageOverall / PortalContext::averageOverallCount;
		unsigned int avgCapture = PortalContext::averageCapture / PortalContext::averageCaptureCount;
		if ( !PortalContext::averageRenderCount )
			return;
		unsigned int avgRender = PortalContext::averageRender / PortalContext::averageRenderCount;
		if ( !PortalContext::averageEncodeCount )
			return;
		unsigned int avgEncode = PortalContext::averageEncode / PortalContext::averageEncodeCount;
		unsigned int avgSend = PortalContext::averageSend / PortalContext::averageSendCount;

		CLogArg ( "[%u]: AVGAll [%u], AVG [%u / %u], Capture [%u / %u], Render [%u / %u], Encode [%u / %u], Send [%u / %u] 탎", ctx->id, avgAll,
			ms, avg, ctx->averageCaptureLast, avgCapture,
			ctx->averageRenderLast, avgRender,
			ctx->averageEncodeLast, avgEncode,
			ctx->averageSendLast, avgSend );
	}


	inline void pc_MeasureCapture ( PortalContext * ctx )
	{
		LARGE_INTEGER		curCounter;
		QueryPerformanceCounter ( &curCounter );

		unsigned int ms = (unsigned int) ((curCounter.QuadPart - ctx->pc_counterLast.QuadPart) * 1000000 / pc_frequency.QuadPart);
		ctx->averageCapture += ms;
		ctx->averageCaptureLast = ms;
		ctx->averageCaptureCount++;
		/*unsigned int avg = ctx->averageCapture / ctx->averageCaptureCount;

		CLogArg ( "pc_Capture [%u]: cur [%u], avg [%u] 탎", ctx->id, ms, avg );*/

		ctx->pc_counterLast = curCounter;
	}


	inline void pc_MeasureRender ( PortalContext * ctx )
	{
		LARGE_INTEGER		curCounter;
		QueryPerformanceCounter ( &curCounter );

		unsigned int ms = (unsigned int) ((curCounter.QuadPart - ctx->pc_counterLast.QuadPart) * 1000000 / pc_frequency.QuadPart);
		ctx->averageRender += ms;
		ctx->averageRenderCount++;
		ctx->averageRenderLast = ms;
		/*unsigned int avg = ctx->averageRender / ctx->averageRenderCount;

		CLogArg ( "pc_Render [%u]: cur [%u], avg [%u] 탎", ctx->id, ms, avg );*/

		ctx->pc_counterLast = curCounter;
	}



	inline void pc_MeasureEncode ( PortalContext * ctx )
	{
		LARGE_INTEGER		curCounter;
		QueryPerformanceCounter ( &curCounter );

		unsigned int ms = (unsigned int) ((curCounter.QuadPart - ctx->pc_counterLast.QuadPart) * 1000000 / pc_frequency.QuadPart);
		ctx->averageEncode += ms;
		ctx->averageEncodeCount++;
		ctx->averageEncodeLast = ms;
		/*unsigned int avg = ctx->averageEncode / ctx->averageEncodeCount;

		CLogArg ( "pc_Encode [%u]: cur [%u], avg [%u] 탎", ctx->id, ms, avg );*/

		ctx->pc_counterLast = curCounter;
	}


	inline void pc_MeasureSend ( PortalContext * ctx )
	{
		LARGE_INTEGER		curCounter;
		QueryPerformanceCounter ( &curCounter );

		unsigned int ms = (unsigned int) ((curCounter.QuadPart - ctx->pc_counterLast.QuadPart) * 1000000 / pc_frequency.QuadPart);
		ctx->averageSend += ms;
		ctx->averageSendCount++;
		ctx->averageSendLast = ms;
		/*unsigned int avg = ctx->averageSend / ctx->averageSendCount;

		CLogArg ( "pc_Send [%u]: cur [%u], avg [%u] 탎", ctx->id, ms, avg );*/

		ctx->pc_counterLast = curCounter;
	}

#endif

	
}
