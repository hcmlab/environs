/**
 * Interface for a portal encoder/compressor
   (create transport packages)
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
//#define DEBUGVERB
//#define ENCODERDEBUGVERB
#endif

#include "Environs.h"
#include "Environs.Native.h"
#include "Interfaces/IPortal.Encoder.h"

#define CLASS_NAME	"IPortalEncoder"


namespace environs
{
	__forceinline bool ResizeBuffer ( RenderContext * context, unsigned int payloadSize )
	{
		ByteBuffer * buffer = context->outputBuffer;

		if ( buffer && buffer->capacity >= payloadSize ) {
			buffer->payloadSize = payloadSize;
			return true;
		}

		buffer = relocateBuffer ( context->outputBuffer, true, payloadSize + 4 );
		if ( buffer ) {
			context->outputBuffer = buffer;
			buffer->payloadSize = payloadSize;
			return true;
		}
		return false;
	}

}
