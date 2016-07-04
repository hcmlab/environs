/**
 * Render context declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_RENDERCONTEXT_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_RENDERCONTEXT_DECLARATIONS_H_

#define USE_ENCODER_BYTEBUFFER

#include "Environs.Types.h"
#include "Interop/Threads.h"

#ifdef USE_ENCODER_BYTEBUFFER
#	include "Core/Byte.Buffer.h"
#endif

namespace environs
{

	typedef struct RenderContext
	{
		unsigned int			id;
		bool					isInitialized;
#ifdef ENABLE_IMPROVED_PORTAL_GENERATOR
        long                    hasContent;
#else
		bool					hasContent;
#endif
		bool					hasChanged;
		bool					hasSendLoad;

		// This flag is true, if there is no renderer. In this case, the renderedData pointers may be used by the capturer and encoder arbitrarily.
		bool					renderSkipped;

		PortalBufferType_t		renderedDataType;
		void				*	renderedData;
		int						renderedDataCapacity;
		void				*	renderedDataHandle;

		// For PortalBufferType::ARGB we need to supply width/height/stride of the renderedData
		unsigned int			width;
		unsigned int			height;
		unsigned int			stride;

#ifdef ENABLE_WORKER_STAGES_LOCKS
		sem_tp					renderSem;
		sem_tp					encoderSem;
		sem_tp					sendSem;
#else
#ifdef USE_WORKER_THREADS
		pthread_cond_t			eventRendered;
		pthread_cond_t			eventEncoded;
#endif
#endif
        
        bool                    isIFrame;
        
		int						frameCounter;
#ifdef USE_ENCODER_BYTEBUFFER
		ByteBuffer			*	outputBuffer;
#else
		char				*	outputBuffer;
#endif

#ifdef PERFORMANCE_MEASURE
		LARGE_INTEGER			pc_counterStart;
		LARGE_INTEGER			pc_counterLast;
		unsigned int			averageFrame;
		unsigned int			averageFrameCount;
		unsigned int			averageCaptureLast;
		unsigned int			averageRenderLast;
		unsigned int			averageEncodeLast;
		unsigned int			averageSendLast;
#endif

	} RenderContext;
}


#endif /* INCLUDE_HCM_ENVIRONS_RENDERCONTEXT_DECLARATIONS_H_ */
