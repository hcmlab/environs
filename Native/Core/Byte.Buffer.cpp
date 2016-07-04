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
#include "stdafx.h"
#define ENVIRONS_NATIVE_MODULE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Byte.Buffer.h"
#include "Environs.Obj.h"

#ifndef WINDOWS_PHONE
#	include <string.h>
#endif

#include "Environs.Lib.h"
#include "Environs.Native.h"
#include "Environs.Mobile.h"

#define	CLASS_NAME 	"Byte.Buffer. . . . . . ."


/* Namespace: environs -> */
namespace environs
{

	bool disposeJBuffer ( void * buffer );


	namespace API
	{

		/*
		* Class:     hcm_environs_Environs
		* Method:    GetBufferHeaderSize
		* Signature: ()I
		*/
		ENVIRONSAPI jint EnvironsProc ( GetBufferHeaderSizeN )
		{
			return ( SIZE_OF_BYTEBUFFER + BUFFER_PREFIX_SIZE );
		}

		/*
		 * Class:     hcm_environs_Environs
		 * Method:    GetBufferHeaderBytesToSize
		 * Signature: ()I
		 */
		ENVIRONSAPI jint EnvironsProc ( GetBufferHeaderBytesToSizeN )
		{
			return SIZE_OF_BYTEBUFFER_TO_SIZE;
		}

		/*
		 * Class:     hcm_environs_Environs
		 * Method:    GetBufferHeaderBytesToType
		 * Signature: ()I
		 */
		ENVIRONSAPI jint EnvironsProc ( GetBufferHeaderBytesToTypeN )
		{
			return SIZE_OF_BYTEBUFFER_TO_TYPE;
		}

		/*
		 * Class:     hcm_environs_Environs
		 * Method:    GetBufferHeaderBytesToStartValue
		 * Signature: ()I
		 */
		ENVIRONSAPI jint EnvironsProc ( GetBufferHeaderBytesToStartValueN )
		{
			return SIZE_OF_BYTEBUFFER_TO_STARTVALUE;
		}
	}


	ByteBuffer * allocBuffer ( unsigned int capacity )
	{
		CVerbArg ( "allocBuffer: allocate capacity of %i", capacity );

		ByteBuffer * buffer = 0;

		unsigned int size = capacity + SIZE_OF_BYTEBUFFER + 8;

#ifdef ANDROID
		int status;
		JNIEnv *env;
		bool isAttached = false;

		if ( !g_JavaVM || !g_java_mallocID || !g_EnvironsClass ) {
			CErr ( "allocBuffer: Failed to call from jni to java - invalid jni resources!" );
			return 0;
		}

		status = g_JavaVM->GetEnv ( ( void ** ) &env, JNI_VERSION_1_6 );
		if ( status < 0 ) {
			//CInfo ( "allocBuffer: No JNI environment available, assuming native thread." );

			status = g_JavaVM->AttachCurrentThread ( &env, 0 );
			if ( status < 0 ) {
				CErr ( "allocBuffer: failed to attach current thread" );
				return 0;
			}
			isAttached = true;
		}

		if ( !env ) {
			CErr ( "allocBuffer: failed to get JNI environment" );
			return 0;
		}

		jobject jByteBuffer = ( jobject ) env->CallStaticObjectMethod ( g_EnvironsClass, g_java_mallocID, size, true );

		if ( jByteBuffer ) {
			buffer = ( ByteBuffer * ) env->GetDirectBufferAddress ( jByteBuffer );
			if ( !buffer ) {
				CErr ( "allocBuffer: Failed to get reference to memory of the shared buffer!" );
				disposeJBuffer ( jByteBuffer );
			}
			else {
				// Initialize header of ByteBuffer
				//			ByteBuffer * header = (ByteBuffer *) buffer;
				memset ( buffer, 0, SIZE_OF_BYTEBUFFER );

				// Save capacity of buffer
				buffer->capacity = capacity;

				jobject jByteBufferGlobal = env->NewGlobalRef ( jByteBuffer );
				if ( !jByteBufferGlobal ) {
					CErr ( "allocBuffer: Failed to get GLOBAL reference to memory of the shared buffer!" );
					disposeJBuffer ( jByteBuffer );
				}
				else {
					buffer->managed.reference = jByteBufferGlobal;
				}
			}
		}

		if ( isAttached )
			g_JavaVM->DetachCurrentThread ();
#else
		buffer = ( ByteBuffer * ) calloc ( 1, size );
		if ( buffer ) {
			// Save capacity of buffer
			( ( ByteBuffer * ) buffer )->capacity = capacity;
		}
#endif

		CVerbArg ( "allocBuffer: done (%s)", buffer == 0 ? "failed" : "success" );
		return buffer;
	}


	bool disposeBuffer ( ByteBuffer * buffer )
	{
		if ( !buffer ) {
			CErr ( "disposeBuffer: invalid memory buffer parameter! (NULL)" );
			return false;
		}

		CVerb ( "disposeBuffer: disposing buffer" );
#ifdef ANDROID
		return disposeJBuffer ( ( void * ) buffer->managed.reference );
#else
		free ( buffer );
		return true;
#endif
	}


#ifdef ANDROID
	bool disposeJBuffer ( void * buffer )
	{
		CVerb ( "disposeBuffer" );

		int status;
		JNIEnv *env;
		bool isAttached = false;

		if ( !buffer ) {
			CErr ( "disposeBuffer: invalid memory buffer parameter! (NULL)" );
			return false;
		}

		if ( !g_JavaVM || !g_java_freeID || !g_EnvironsClass ) {
			CErr ( "disposeBuffer: Failed to call from jni to java - invalid jni resources!" );
			return false;
		}

		status = g_JavaVM->GetEnv ( ( void ** ) &env, JNI_VERSION_1_6 );
		if ( status < 0 ) {
			//CInfo ( "disposeBuffer: No JNI environment available, assuming native thread" );

			status = g_JavaVM->AttachCurrentThread ( &env, 0 );
			if ( status < 0 ) {
				CErr ( "disposeBuffer: failed to attach current thread" );
				return false;
			}
			isAttached = true;
		}

		if ( !env ) {
			CErr ( "disposeBuffer: failed to get JNI environment" );
			return false;
		}


		status = env->CallStaticBooleanMethod ( g_EnvironsClass, g_java_freeID, buffer );

		env->DeleteGlobalRef ( ( jobject ) buffer );

		if ( isAttached )
			g_JavaVM->DetachCurrentThread ();

		CVerbArg ( "disposeBuffer: done with status %i", status );
		return status;
	}
#endif


	ByteBuffer * relocateBuffer ( ByteBuffer * buffer, bool dispose, unsigned int capacity )
	{
		CVerb ( "relocateBuffer" );

		if ( buffer && buffer->capacity >= capacity )
			return buffer;

		ByteBuffer * newBuffer = allocBuffer ( capacity );
		if ( !newBuffer )
			return 0;

		if ( buffer && dispose )
			disposeBuffer ( buffer );

		return newBuffer;
	}


	jobject allocJByteBuffer ( JNIEnv * jenv, unsigned int capacity, char * &buffer )
	{
		CVerbArg ( "allocJByteBuffer: allocate capacity of %i", capacity );

		jobject jByteBuffer;
#ifdef ANDROID
		jByteBuffer = ( jobject ) jenv->CallStaticObjectMethod ( g_EnvironsClass, g_java_mallocID, capacity, false );

		if ( jByteBuffer ) {
			buffer = ( char * ) jenv->GetDirectBufferAddress ( jByteBuffer );
			if ( !buffer ) {
				CErr ( "allocJByteBuffer: Failed to get reference to memory of the shared buffer!" );
			}
		}
#else
		buffer = ( char * ) malloc ( capacity );
		jByteBuffer = buffer;
#endif

		CVerbArg ( "allocJByteBuffer: done (%s)", buffer == 0 ? "failed" : "success" );
		return jByteBuffer;
	}

	
	void releaseJByteBuffer ( jobject &buffer )
	{
#ifndef ANDROID
		if ( buffer ) {
			free ( buffer );
			buffer = 0;
		}
#endif
	}

} // <- namespace environs



