/**
 * ByteBuffer
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
#ifndef INCLUDE_HCM_ENVIRONS__BYTEBUFFER_H_
#define INCLUDE_HCM_ENVIRONS__BYTEBUFFER_H_

#include "Interop/jni.h"
#include "Interop/Export.h"

/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif

	//#define BUFFER_PREFIX_SIZE  128
#define BUFFER_PREFIX_SIZE  0

	typedef struct _ByteBuffer
	{
		unsigned int	capacity;	// 4
		unsigned int	acquired;	// +4 = 8
		unsigned int	type;		// +4 = 12

		union
		{
			jobject     reference;  // allow up to 16 bytes
			char        pad [ 16 ];
		}
		managed;                    // +16 = 28

		int				payloadSize;// +4 = 32;

		int             payloadStart;
	}
	ByteBuffer;

#define SIZE_OF_BYTEBUFFER                      44
#define SIZE_OF_BYTEBUFFER_TO_SIZE              (16 + (3 * sizeof(int)))
#define SIZE_OF_BYTEBUFFER_TO_TYPE              (2 * sizeof(int))
#define SIZE_OF_BYTEBUFFER_TO_STARTVALUE        (SIZE_OF_BYTEBUFFER_TO_SIZE + sizeof(int))

	//#define BYTEBUFFER_DATA_POINTER(buffer)         (((char *)buffer) + SIZE_OF_BYTEBUFFER)
#define BYTEBUFFER_DATA_POINTER_START(buffer)   (((char *)buffer) + SIZE_OF_BYTEBUFFER + BUFFER_PREFIX_SIZE)



#ifdef __cplusplus

	typedef ByteBuffer * ( CallConv * pallocBuffer ) ( unsigned int capacity );

	typedef ByteBuffer * ( CallConv * prelocateBuffer ) ( ByteBuffer * buffer, bool dispose, unsigned int capacity );

	typedef jobject ( CallConv * pallocJByteBuffer ) ( JNIEnv * jenv, unsigned int capacity, char * &buffer );

	typedef bool ( CallConv * pdisposeBuffer ) ( ByteBuffer * buffer );

#if defined(ENVIRONS_CORE_LIB)
	//|| !defined(ENVIRONS_MODULE)
	ByteBuffer * allocBuffer ( unsigned int capacity );

	ByteBuffer * relocateBuffer ( ByteBuffer * buffer, bool dispose, unsigned int capacity );

	jobject allocJByteBuffer ( JNIEnv * jenv, unsigned int capacity, char * &buffer );
	void releaseJByteBuffer ( jobject &buffer );

	bool disposeBuffer ( ByteBuffer * buffer );
#else

#define allocBuffer(c)              ((environs::Instance *)pEnvirons)->AllocBuffer(c)
#define relocateBuffer(b,d,c)       ((environs::Instance *)pEnvirons)->RelocateBuffer(b,d,c)
#define allocJByteBuffer(a,c,b)     ((environs::Instance *)pEnvirons)->AllocJByteBuffer(a,c,b)
#define disposeBuffer(c)            ((environs::Instance *)pEnvirons)->DisposeBuffer(c)

#endif

#endif


#ifdef __cplusplus
} /* namespace */
#endif


#endif /* INCLUDE_HCM_ENVIRONS__BYTEBUFFER_H_ */



