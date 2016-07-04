/**
 *	Portal stream functionality
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
//#   define DEBUGVERBLocks
#endif


#include "Environs.Obj.h"
using namespace environs;

#include "Environs.Lib.h"
using namespace environs::API;

#include "Device/Device.Controller.h"
#include "Portal.Info.Base.h"
#include "Portal/Portal.Stream.h"
#include "Portal/Portal.Device.h"
#include "Core/Async.Worker.h"
#include "Core/Notifications.h"
#include "Environs.Crypt.h"

#ifndef WINDOWS_PHONE
#	include <string.h>
#endif

#define	CLASS_NAME 	"Portal.Stream. . . . . ."


/* Namespace: environs -> */
namespace environs
{

	PortalStream::PortalStream ()
	{
		CVerb ( "Construct" );

		allocated		= false;
		env				= 0;

		deviceID		= 0;

		bufferMax		= 0;
		bufferCount		= 0;
		buffers			= 0;
		accessCount		= 1;

		nextBuffer		= 0;
		busyBuffer		= -1;
		nextPushBuffer	= 1;

		stalled         = false;

		enabled			= true;
	}


	PortalStream::~PortalStream ()
	{
		CVerbID ( "Destruct" );

		Stop ();

		ReleasePortalStream ();
		ReleaseStreamBuffers ();
		ReleaseStreamStorage ();

		if ( allocated )
        {
            allocated = false;
            
			CondDispose ( &availEvent );
			CondDispose ( &queueEvent );

			LockDispose ( &resourcesMutex );
		}
	}


	bool PortalStream::GetLock ()
	{
		CVerbID ( "GetLock" );

		CVerbLockPortalRecRes ( "GetNextStreamBuffer" );

		if ( !LockAcquire ( &resourcesMutex, "GetLock" ) )
			return false;

		bool success = false;

		if ( accessCount ) {
			accessCount++;
			success = true;
		}

		CVerbUnLockPortalRecRes ( "GetNextStreamBuffer" );

		LockReleaseV ( &resourcesMutex, "GetLock" );

		return success;
	}


	void PortalStream::ReleaseLock ()
	{
		CVerbID ( "ReleaseLock" );

		CVerbLockPortalRecRes ( "ReleaseLock" );

		LockAcquireV ( &resourcesMutex, "ReleaseLock" );

		if ( accessCount )
			accessCount--;

		if ( pthread_cond_signal ( &availEvent ) ) {
			CErr ( "ReleaseLock: Failed to signal availEvent!" );
		}

		if ( pthread_cond_signal ( &queueEvent ) ) {
			CErr ( "ReleaseLock: Failed to signal queueEvent!" );
		}

		CVerbUnLockPortalRecRes ( "ReleaseLock" );

		LockReleaseV ( &resourcesMutex, "ReleaseLock" );
	}


	bool PortalStream::Init ( int deviceIDa )
	{
		deviceID = deviceIDa;

		CVerbID ( "Init" );

		if ( !env )
			return false;

		bufferMax		= env->streamBufferCount;

		if ( !allocated ) {
			if ( !LockInit ( &resourcesMutex ) )
				return false;

			if ( !CondInit ( &availEvent ) )
				return false;

			if ( !CondInit ( &queueEvent ) )
				return false;

			allocated = true;
		}

		if ( bufferCount < bufferMax ) {
			ReleaseStreamBuffers ();

			CVerbLockPortalRecRes ( "Init" );

			if ( !LockAcquire ( &resourcesMutex, "Init" ) )
				return false;

			if ( buffers ) {
				free ( buffers );
				buffers = 0;
			}

			buffers = ( ByteBuffer ** ) calloc ( sizeof ( ByteBuffer * ), bufferMax );
			if ( !buffers ) {
				CErrID ( "Init: Failed to allocate memory for stream buffer storage!" );

				CVerbUnLockPortalRecRes ( "Init" );

				LockReleaseV ( &resourcesMutex, "Init" );
				return false;
			}

			for ( unsigned int i = 0; i < bufferMax; i++ ) {
				void * mem = allocBuffer ( STREAM_BUFFER_SIZE_MIN );
				if ( !mem ) {
					CErrArgID ( "Init: Failed to allocated memory for streamBuffer %i of size %i", i, STREAM_BUFFER_SIZE_MIN );

					CVerbUnLockPortalRecRes ( "Init" );

					LockReleaseV ( &resourcesMutex, "Init" );
					return false;
				}
				buffers [ i ] = ( ByteBuffer * ) mem;

				bufferCount++;
			}

			CVerbUnLockPortalRecRes ( "Init" );

			if ( !LockRelease ( &resourcesMutex, "Init" ) )
				return false;
		}

		Start ();

		return true;
	}


	void PortalStream::Start ()
	{
		CVerbID ( "Start" );

		enabled = true;
	}


	void PortalStream::StopNonBlock ()
	{
		CVerbID ( "StopNonBlock" );

		enabled = false;

		CVerbLockPortalRecRes ( "StopNonBlock" );

		LockAcquireV ( &resourcesMutex, "StopNonBlock" );

		if ( accessCount > 1 ) // add a release stream thread stop
		{
			CLogID ( "StopNonBlock: Signaling queue event to stop a potential receiver from receiving." );

			if ( pthread_cond_signal ( &availEvent ) ) {
				CErr ( "StopNonBlock: Failed to signal availEvent!" );
			}
		}

		CVerbUnLockPortalRecRes ( "StopNonBlock" );

		LockReleaseV ( &resourcesMutex, "StopNonBlock" );
	}


	void PortalStream::Stop ()
	{
		CVerbID ( "Stop" );

		enabled = false;

		CVerbLockPortalRecRes ( "Stop" );

		LockAcquireV ( &resourcesMutex, "Stop" );

	WaitAgain:
		if ( accessCount > 1 ) // add a release stream thread stop
		{
			CLogID ( "Stop: Signaling queue event to stop a potential receiver from receiving." );

			if ( pthread_cond_signal ( &availEvent ) ) {
				CErr ( "Stop: Failed to signal availEvent!" );
			}
			//		pthread_cond_prepare ( &availEvent );

			CInfoArgID ( "Stop: Waiting for [%u] receiver to be disposed.", accessCount );
			/*LeaveCriticalSection ( &resourcesMutex );

			DWORD res = WaitForSingleObject ( availEvent, INFINITE );
			if ( res != WAIT_OBJECT_0 ) {
			CErrID ( "Stop: Failed to wait..." );
			}

			EnterCriticalSection ( &resourcesMutex );*/

			//pthread_cond_wait ( &availEvent, &resourcesMutex );
			CVerbUnLockPortalRecRes ( "Stop" );

			int rc = pthread_cond_timedwait_msec ( &queueEvent, &resourcesMutex, WAIT_TIME_FOR_CONNECTIONS );

			CVerbLockPortalRecRes ( "Stop" );

			if ( rc ) {
				accessCount--;
				CErr ( "Stop: Wait failed!" );
			}

			//pthread_cond_wait ( &queueEvent, &resourcesMutex );


			goto WaitAgain;
		}
		accessCount = 0;
		CVerbID ( "Stop: All receivers disposed." );

		CVerbUnLockPortalRecRes ( "Stop" );

		LockReleaseV ( &resourcesMutex, "Stop" );
	}


	void PortalStream::ReleasePortalStream ()
	{
		CVerbArgID ( "ReleasePortalStream [%i]", bufferCount );

		CVerbLockPortalRecRes ( "ReleasePortalStream" );

		if ( !LockAcquire ( &resourcesMutex, "ReleasePortalStream" ) )
			return;

		for ( unsigned int i=0; i<bufferCount; i++ ) {
			( ( ByteBuffer * ) buffers [ i ] )->payloadSize = -1;
		}

		if ( pthread_cond_signal ( &availEvent ) ) {
			CErr ( "ReleasePortalStream: Failed to signal availEvent!" );
		}

		CVerbUnLockPortalRecRes ( "ReleasePortalStream" );

		LockReleaseV ( &resourcesMutex, "ReleasePortalStream" );
	}


	void PortalStream::ReleaseStreamBuffers ()
	{
		CVerbArgID ( "ReleaseStreamBuffers [%i]", bufferCount );

		CVerbLockPortalRecRes ( "ReleaseStreamBuffers" );

		if ( !LockAcquire ( &resourcesMutex, "ReleaseStreamBuffers" ) )
			return;

		// Releasing shared buffers
		for ( unsigned int i=0; i<bufferCount; i++ )
		{
			ByteBuffer * header = ( ByteBuffer * ) buffers [ i ];
			if ( header ) {
				disposeBuffer ( header );
				buffers [ i ] = 0;
			}
		}

		bufferCount = 0;

		CVerbUnLockPortalRecRes ( "ReleaseStreamBuffers" );

		LockReleaseV ( &resourcesMutex, "ReleaseStreamBuffers" );
	}


	void PortalStream::ReleaseStreamStorage ()
	{
		CVerbArgID ( "ReleaseStreamStorage [%i]", bufferCount );

		CVerbLockPortalRecRes ( "ReleaseStreamStorage" );

		if ( !LockAcquire ( &resourcesMutex, "ReleaseStreamStorage" ) )
			return;

		if ( buffers ) {
			free ( buffers );
			buffers = 0;
		}

		CVerbUnLockPortalRecRes ( "ReleaseStreamStorage" );

		LockReleaseV ( &resourcesMutex, "ReleaseStreamStorage" );
	}


	ByteBuffer * PortalStream::GetNextStreamBuffer ( unsigned int minCapacity, bool returnUnlocked )
	{
		CVerbVerbID ( "GetNextStreamBuffer" );

		if ( !enabled )
			return 0;

		ByteBuffer * bufferFound = 0;


		CVerbLockPortalRecRes ( "GetNextStreamBuffer" );

		if ( pthread_mutex_lock ( &resourcesMutex ) ) {
			CErrID ( "GetNextStreamBuffer: Failed to acquire mutex" );
			return 0;
		}

		int buffersRemaining = bufferCount;
		unsigned int availableBuffers = buffersRemaining;

		while ( buffersRemaining )
		{
			ByteBuffer * buffer = buffers [ nextPushBuffer ];

			CVerbVerbArgID ( "GetNextStreamBuffer: remain %i, buffer %i, lock %i", buffersRemaining, nextPushBuffer, buffer->acquired );
			if ( !buffer->acquired )
			{
				CVerbVerbArgID ( "GetNextStreamBuffer: buffer %i, payloadSize %i", nextPushBuffer, buffer->payloadSize );
				if ( buffer->payloadSize == 0 )
				{
					if ( buffer->capacity < minCapacity ) {
						ByteBuffer * newBuffer = relocateBuffer ( buffer, true, minCapacity );
						if ( !newBuffer ) {
							// Capacity of current buffer is not sufficient and a suitable one failed to allocate
							goto ContinueLooking;
						}
						//disposeBuffer ( buffer );

						buffers [ nextPushBuffer ] = newBuffer;
						buffer = newBuffer;
					}

					bufferFound = buffer;

					// Lock it for the caller
					bufferFound->acquired = true;

					nextPushBuffer++;
					if ( nextPushBuffer >= availableBuffers )
						nextPushBuffer = 0;
					break;
				}
			}

		ContinueLooking:
			nextPushBuffer++;
			if ( nextPushBuffer >= availableBuffers )
				nextPushBuffer = 0;

			buffersRemaining--;
		}

		if ( !bufferFound ) {
			if ( pthread_cond_signal ( &availEvent ) ) {
				CErr ( "GetNextStreamBuffer: Failed to signal availEvent!" );
			}
		}

		if ( returnUnlocked || !bufferFound ) {
			CVerbUnLockPortalRecRes ( "GetNextStreamBuffer" );

			if ( pthread_mutex_unlock ( &resourcesMutex ) ) {
				CErrID ( "GetNextStreamBuffer: Failed to release mutex" );
				return 0;
			}
		}

		return bufferFound;
	}


void PortalStream::PushStreamBuffer ( ByteBuffer * buffer, unsigned int payloadSize, unsigned int type )
{
	if ( payloadSize <= 0 || !buffer ) {
		return;
	}
    
    CVerbLockPortalRecRes ( "PushStreamBuffer" );
    
	if ( pthread_mutex_lock ( &resourcesMutex ) ) {
		CErrID ( "PushStreamBuffer: Failed to acquire mutex" );
		return;
	}

	buffer->payloadSize = payloadSize;
	buffer->type = type;

	if ( pthread_cond_signal ( &availEvent ) ) {
		CErrID ( "PushStreamBuffer: Failed to signal availEvent!" );
	}
    
    CVerbUnLockPortalRecRes ( "PushStreamBuffer" );
    
	if ( pthread_mutex_unlock ( &resourcesMutex ) ) {
		CErrID ( "PushStreamBuffer: Failed to release mutex" );
	}
}


bool PortalStream::PushStreamPacket ( void * payload, unsigned int payloadSize, unsigned int type )
{
	CVerbVerbID ( "PushStreamPacket" );

	if ( payloadSize <= 0 || !payload ) {
		return true;
	}

	ByteBuffer * buffer = GetNextStreamBuffer ( payloadSize, false );
	if ( buffer )
	{
		memcpy ( BYTEBUFFER_DATA_POINTER_START ( buffer ), payload, payloadSize );

		buffer->payloadSize = payloadSize;
		buffer->type = type;
		buffer->payloadStart = (SIZE_OF_BYTEBUFFER + BUFFER_PREFIX_SIZE);

		if ( pthread_cond_signal ( &availEvent ) ) {
			CErrID ( "PushStreamPacket: Failed to signal availEvent!" );
		}

		CVerbUnLockPortalRecRes ( "PushStreamPacket" );

		if ( pthread_mutex_unlock ( &resourcesMutex ) ) {
			CErrID ( "PushStreamPacket: Failed to release mutex" );
		}
		return true;
	}
	return false;
}
    

jobject PortalStream::ReceiveStreamPack ()
{
	int bufferIndex = -1;
	jobject retBuffer = 0;
    
	CVerbVerbID ( "ReceiveStreamPack: Requesting data buffer ..." );
    
    CVerbLockPortalRecRes ( "ReceiveStreamPack" );
    
	if ( pthread_mutex_lock ( &resourcesMutex ) ) {
		CErrID ( "ReceiveStreamPack: Failed to acquire mutex" );
		return 0;
	}
	accessCount++;

	if ( env->environsState < STATUS_STARTED || !enabled ) {
		bufferIndex = -1;
		goto ReturnWithBuffer;
	}

	unsigned int availableBuffers;
	int buffersRemaining;

	// If the buffers haven't been initialized, then immediately goto wait state
	if ( bufferCount < bufferMax )
		goto WaitForNext;

	//
	// Look for the next buffer with data
	//

	// Release a busy buffer if we have one claimed from last call
	if ( busyBuffer >= 0 ) {
		ByteBuffer * lastBuffer = (ByteBuffer *)buffers [busyBuffer];
		lastBuffer->payloadSize = 0;
		lastBuffer->acquired = false;

		CVerbVerbArgID ( "ReceiveStreamPack: Releasing buffer (%i)", busyBuffer );
	}

LookThroughStorage:
	availableBuffers = bufferCount;
	buffersRemaining = availableBuffers;

	while ( buffersRemaining )
	{
		ByteBuffer * buffer = (ByteBuffer *)buffers [nextBuffer];

		CVerbVerbArgID ( "ReceiveStreamPack: remain %i, buffer %i, lock %i", buffersRemaining, nextBuffer, buffer->acquired );
		if ( buffer->acquired )
		{
			// Data buffer is assigned to a thread, check whether it contains data !!!
			// TODO Make sure that the buffer contains at least one byte for only messages!!!

			int payloadSize = buffer->payloadSize;
			CVerbVerbArgID ( "ReceiveStreamPack: buffer %i, payloadSize %i", nextBuffer, payloadSize );
			if ( payloadSize < 0 )
				goto ReturnWithBuffer;

			if ( payloadSize > 0 ) // We have found data
			{
				// Now nextBuffer is busy
				busyBuffer = nextBuffer;

				bufferIndex = nextBuffer;

				// Select the buffer index for next call
				nextBuffer++;
				if ( nextBuffer >= (int) availableBuffers )
					nextBuffer = 0;

				CVerbVerbArgID ( "ReceiveStreamPack: Claiming buffer %i as busy; Next buffer is %i", busyBuffer, nextBuffer );
				goto ReturnWithBuffer;
			}
		}

		nextBuffer++;
		if ( nextBuffer >= (int) availableBuffers )
			nextBuffer = 0;

		buffersRemaining--;
	}

WaitForNext:
    CVerbVerbID ( "ReceiveStreamPack: Waiting for next data buffer..." );
    
    CVerbUnLockPortalRecRes ( "ReceiveStreamPack" );
    
    pthread_cond_wait ( &availEvent, &resourcesMutex );
    
    CVerbLockPortalRecRes ( "ReceiveStreamPack" );

	if ( !enabled ) {

		/// If we return a null buffer, then the receiver thread will be closed and we can safely release the access lock
		bufferIndex = -1;
		goto ReturnWithBuffer;
	}
	goto LookThroughStorage;


ReturnWithBuffer:
	if ( bufferIndex < 0 ) {
		if ( pthread_cond_signal ( &queueEvent ) ) {
			CErrID ( "ReceiveStreamPack: Failed to signal queueEvent!" );
		}
	}
	else {
#if defined(ANDROID)
		if ( !opt_useNativeDecoder )
			retBuffer = (jobject) buffers [bufferIndex]->managed.reference;
		else
#endif
			retBuffer = (jobject) buffers [ bufferIndex ];
	}

	accessCount--;
    
    CVerbUnLockPortalRecRes ( "ReceiveStreamPack" );
    
	if ( pthread_mutex_unlock ( &resourcesMutex ) ) {
		CErrID ( "ReceiveStreamPack: Failed to acquire mutex" );
		return 0;
	}

	return retBuffer;
}

    
int GetDeviceOptionInt ( JNIEnv * jenv, int nativeID, unsigned short optionID, unsigned int capacity, jintArray results )
{
    unsigned int ret = 0;
        /*
    jint * pResults = (jint *) results;
        
    // Rouch check of result reference
#ifdef ANDROID
    capacity = jenv->GetArrayLength ( results );
    //CLogArgID ( "GetDeviceIntOption: capacity [%i].", cap );
    if ( capacity == 0 )
        return false;
    //CLogArgID ( "GetDeviceIntOption: capacity [%i].", cap );
        
    pResults = jenv->GetIntArrayElements ( results, NULL );
    if ( !pResults )
        return false;
#else
    if ( !results )
        return false;
#endif
    capacity *= sizeof ( jint );
        
    ret = onEnvironsGetResponse ( env, nativeID, optionID, capacity, pResults );
    if ( ret > 0 )
        ret /= sizeof ( jint );
        
#ifdef ANDROID
    Finish :
    jenv->ReleaseIntArrayElements ( results, pResults, 0 );
#endif
    */
    return (int) ret;
}
    
    
int GetDeviceOptionFloat ( JNIEnv * jenv, int nativeID, unsigned short optionID, unsigned int capacity, jfloatArray results )
{
    
    unsigned int ret = 0;
    /*
    jfloat * pResults = (jfloat *) results;
        
    // Rouch check of result reference
#ifdef ANDROID
    capacity = jenv->GetArrayLength ( results );
    CVerbArgIDN ( "getTCPPortal: capacity [%i].", capacity );
    if ( capacity == 0 )
        return false;
    CVerbArgIDN ( "getTCPPortal: capacity [%i].", capacity );
        
    pResults = jenv->GetFloatArrayElements ( results, NULL );
    if ( !pResults )
        return false;
#else
    if ( !results )
        return false;
#endif
    capacity *= sizeof ( jfloat );
        
    ret = onEnvironsGetResponse ( env, nativeID, optionID, capacity, pResults );
    if ( ret > 0 )
        ret /= sizeof ( jfloat );
        
#ifdef ANDROID
    Finish :
    jenv->ReleaseFloatArrayElements ( results, pResults, 0 );
#endif
    */
    return (int) ret;
}

	namespace API
	{
		/**
		* Get details about portal associated with the portalID.
		*
		* @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 						Applications should store them in order to address the correct portal within Environs.
		* @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
		*/
		ENVIRONSAPI EBOOL GetPortalInfo1 ( int hInst, void * portalInfo )
		{
			CVerb ( "GetPortalInfo1" );

			if ( !portalInfo )
				return false;

			PortalInfoBase * info = ( PortalInfoBase * ) portalInfo;

			bool success = false;

			DeviceBase * device = GetDeviceIncLock ( info->portalID );
			if ( device )
			{
				success = device->GetPortalInfo ( info );

				UnlockDevice ( device );
			}
			return success;
		}


		/**
		* Get details about portal associated with the portalID.
		*
		* @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 						Applications should store them in order to address the correct portal within Environs.
		* @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalInfoN, jint hInst, jobject buffer )
		{
			CVerb ( "GetPortalInfoN" );

			if ( !buffer ) {
				CErr ( "GetPortalInfoN: called with NULL buffer argument!" );
				return false;
			}

			EBOOL	success = false;

#ifdef ANDROID
			PortalInfoBase * info = (PortalInfoBase *) jenv->GetDirectBufferAddress ( buffer );
			if ( !info ){
				CErr ( "GetPortalInfoN: Failed to get reference to memory of the shared buffer!" );
				goto Finish;
			}
			success = GetPortalInfo1 ( hInst, info );

		Finish:
#else
			success = GetPortalInfo1 ( hInst, buffer );
#endif
			return success;
		}


		/**
		* Set details for the portal associated with the portalID.
		*
		* @param   info        A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
		* 			            The portalID member of the PortalInfo object must have valid values.
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 						Applications should store them in order to address the correct portal within Environs.
		* @return success
		*/
		ENVIRONSAPI EBOOL SetPortalInfo1 ( int hInst, void * portalInfo )
		{
			CVerbVerb ( "SetPortalInfo1" );

			if ( !portalInfo )
				return false;

			bool success = false;

            PortalInfoBase * info = ( PortalInfoBase * ) portalInfo;

            CVerbArg ( "SetPortalInfo1: portalID [ 0x%X ]", info->portalID );

			DeviceBase * device = GetDeviceIncLock ( info->portalID );
			if ( device )
			{
				success = device->SetPortalInfo ( portalInfo );

				UnlockDevice ( device );
			}
            /*else {
                CErrArg ( "SetPortalInfo1: PortalID not found [ 0x%X ]", info->portalID );
            }*/

			return success;
		}


		/**
		* Set details for the portal associated with the portalID.
		*
		* @param   info        A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
		* 			            The portalID member of the PortalInfo object must have valid values.
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 						Applications should store them in order to address the correct portal within Environs.
		* @return success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetPortalInfoN, jint hInst, jobject buffer )
		{
			CVerb ( "SetPortalInfoN" );

			if ( !buffer ) {
				CErr ( "SetPortalInfoN: Called with NULL buffer argument!" );
				return false;
			}

			EBOOL	success = false;

#ifdef ANDROID
			PortalInfoBase * info = (PortalInfoBase *)jenv->GetDirectBufferAddress ( buffer );
			if ( !info ){
				CErr ( "SetPortalInfoN: Failed to get reference to memory of the shared buffer!" );
			}
			else success = SetPortalInfo1 ( hInst, info );
#else
			success = SetPortalInfo1 ( hInst, buffer );
#endif
			return success;
		}


		/**
		* Find a free portalID slot for the direction encoded into the given portalDetails.
		*
		* @param 	nativeID    	The native device id of the target device.
		* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
		* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
		*/
		ENVIRONSAPI jint EnvironsFunc ( GetPortalIDFreeSlotN, jint hInst, jint nativeID, jint portalDetails )
		{
			CVerbArgIDN ( "GetPortalIDFreeSlotN: [0x%X]", portalDetails );

			DeviceBase * device = (DeviceBase *) environs::GetDevice ( nativeID );
			if ( !device ) {
				CWarnIDN ( "GetPortalIDFreeSlotN: Device not found." );
				return -1;
			}

			int portalID = device->GetPortalIDFreeSlot ( portalDetails );

			UnlockDevice ( device );

			return portalID;
		}


		/**
		* Request a portal stream from the device with the given id.&nbsp;The device must be connected before with deviceConnect ().
		*
		* @param 	nativeID		The native id of the target device.
		* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
		* @param 	portalDetails	An application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
		* @param 	width       	The width of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
		*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
		* @param 	height      	The height of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
		*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
		*
		* @return 	success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( RequestPortalStreamN, jint hInst, jint nativeID, jint async, jint portalDetails, jint width, jint height )
        {
			CVerbArgIDN ( "RequestPortalStreamN: [%i]", portalDetails );
            
            EBOOL success = false;
            
            Instance * env = instances[hInst];
            
            if ( async )
				success = ToEBOOL ( env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_PORTAL_INIT_REQUEST, portalDetails, width, height ) );
            else
				success = env->asyncWorker.PortalRequest ( nativeID, portalDetails, width, height );
            
            return success;
        }
        
        
        /**
         * Provide a portal stream to the device with the given id.&nbsp;
         * The device must be in a connected state by means of prior call to deviceConnect ().
         *
         * @param 	nativeID		The native id of the target device.
         * @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
		 *
         * @return success
         */
		ENVIRONSAPI EBOOL EnvironsFunc ( ProvidePortalStreamN, jint hInst, jint nativeID, jint async, jint portalDetails )
        {
            CVerbArgIDN ( "ProvidePortalStreamN: [0x%X]", portalDetails );
            
            EBOOL success = false;
            
            Instance * env = instances[hInst];
            
            if ( async )
				success = ToEBOOL ( env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_PORTAL_PROVIDE, portalDetails ) );
            else
				success = env->asyncWorker.PortalProvide ( nativeID, portalDetails );
            
            return success;
        }
        
        
        /**
         * Provide a portal stream to be requested by the device with the given id.&nbsp;
         * The device must be in a connected state by means of prior call to deviceConnect ().
         *
         * @param 	nativeID		The native id of the target device.
         * @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
         *
         * @return success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( ProvideRequestPortalStreamN, jint hInst, jint nativeID, jint async, jint portalDetails )
        {
            CVerbArgIDN ( "ProvideRequestPortalStreamN: [0x%X]", portalDetails );
            
            EBOOL success = false;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.Push ( nativeID, ASYNCWORK_TYPE_PORTAL_PROVIDE_REQUEST, portalDetails ) );
            else
                success = env->asyncWorker.PortalProvideRequest ( nativeID, portalDetails );
            
            return success;
        }


		/**
		* Start streaming of portal to or from the portal identifier (received in notification).
		*
		* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
		* @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
		* @return 	success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( StartPortalStreamN, jint hInst, jint async, jint portalID )
		{
			CVerbArg ( "StartPortalStreamN: [0x%X]", portalID );
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_START, portalID ) );
            else
                success = env->asyncWorker.PortalStart ( portalID );

			return success;
		}


		/**
		* Pause streaming of portal to or from the portal identifier (received in notification).
		*
		* @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
		* @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
		* 						It is provided within the notification listener as sourceIdent.&nbsp;
		* 						Applications should store them in order to address the correct portal within Environs.
		* @return success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( PausePortalStreamN, jint hInst, jint async, jint portalID )
		{
			CVerbArg ( "PausePortalStreamN: [0x%X]", portalID );
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_PAUSE, portalID ) );
            else
                success = env->asyncWorker.PortalStart ( portalID );

			return success;
		}


		/**
		* Stop streaming of portal to or from the portal identifier (received in notification).
		*
        * @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
        * @param nativeID   The native device id of the target device.
		* @param portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
		* 					It is provided within the notification listener as sourceIdent.&nbsp;
		* 					Applications should store them in order to address the correct portal within Environs.
		* @return success
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( StopPortalStreamN, jint hInst, jint async, jint nativeID, jint portalID )
		{
			CVerbArg ( "StopPortalStreamN: [0x%X]", portalID );
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_STOP, nativeID, portalID, 0 ) );
            else
                success = env->asyncWorker.PortalStop ( nativeID, portalID );

			return success;
        }
        
        
        /**
         * Send portal init packet with dimensions of the portal.
         *
         * @param async         Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
         * 						It is provided within the notification listener as sourceIdent.&nbsp;
         * 						Applications should store them in order to address the correct portal within Environs.
         * @param width		 	The portal width in pixel.
         * @param height		The portal height in pixel.
         * @return success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SendPortalInitN, jint hInst, jint async, jint portalID, jint width, jint height )
        {
            CVerbArg ( "SendPortalInitN: [0x%X]", portalID );
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_SEND_INIT, portalID, width, height ) );
            else
                success = env->asyncWorker.PortalSendInit ( portalID, width, height );
            
            return success;
        }
        
        
        /**
         * Acquire a native layer send identifier (and increase instanceLock on device) to be used in sendTcpPortal. This resource must be released on disposal of the portal generator.
         *
         * @param portalUnitType	e.g. MSG_TYPE_STREAM
         * @param portalID    Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
         * @return sendID
         */
        ENVIRONSAPI jint EnvironsFunc ( AcquirePortalSendIDN, int portalUnitType, int portalID )
        {
            CVerb ( "AcquirePortalSendIDN" );
            
            PortalDevice * portal = GetLockedPortalDevice ( portalID );
            if ( !portal ) {
                CErr ( "AcquirePortalSendIDN: No portal resource found." );
                return false;
            }
            
            portal->portalUnitType  = portalUnitType;
			portal->frameCounter    = 0;

			CVerb ( "AcquirePortalSendIDN: Increase device lock." );
            
            return GetPortalDeviceID ( portalID );
        }
        
        
        /**
         * Release a native layer send identifier that was acquired by a call to acquireTcpPortalSendID.
         *
         * @param sendID      The portal send id resource to be released.
         */
        ENVIRONSAPI void EnvironsFunc ( ReleasePortalSendIDN, int sendID )
        {
            CVerb ( "ReleasePortalSendIDN" );
            
            if ( sendID < 0 || sendID >= MAX_PORTAL_INSTANCES )
                return;
            
            PortalDevice * portal = portalDevices + sendID;
            
            /*if ( !LockPortalDevice ( portal ) )
                return;
            */
            /*
			if ( portal->device ) {
				CVerb ( "releaseTcpPortalSendID: Unlocking device." );
				UnlockDevice ( portal->device );
			}
			portal->sendLocked = false;
            */
            
            ReleasePortalDevice ( portal );
        }
        
        
        /**
         * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
         *
         * @param sendID      		The device id of the target device.
         * @param portalUnitFlags	Flags that will be or-ed with the portalUnitType
         * @param prefixBuffer    	A prefix that prepend the actual buffer.
         * @param prefixSize   		The size of the content within the prefixbuffer.
         * @param buffer        	A buffer to be send.
         * @param offset        	An offset into the buffer.
         * @param contentSize   	The size of the content within the buffer.
         * @return success
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SendTcpPortalN, int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize )
        {
            CVerbVerb ( "SendTcpPortalN" );
            
            if ( sendID < 0 || sendID >= MAX_PORTAL_INSTANCES )
                return false;
            
            PortalDevice * portal = portalDevices + sendID;
            
            if ( !portal->device )
                return false;
            
#if defined(ANDROID)
            char * buffer = (char *) jenv->GetDirectBufferAddress ( byteBuffer );
            if ( !buffer ){
                CErr ( "SendTcpPortalN: Failed to get reference to memory of the shared buffer!" );
                return false;
            }
            
            char * prefix = 0;
            if ( prefixBuffer ) {
                prefix = (char *) jenv->GetDirectBufferAddress ( prefixBuffer );
                if ( !prefix ){
                    CErr ( "SendTcpPortalN: Failed to get reference to memory of the prefix buffer!" );
                    return false;
                }
            }
#else
            char * buffer = (char *) byteBuffer;
            char * prefix = (char *) prefixBuffer;
#endif
            bool success = false;
            
            if ( contentSize > 0 ) {
                /*const char * str = ConvertToHexSpaceString ( buffer + offset, contentSize > 128 ? 128 : contentSize );
                
                CLogArg ( "sendTcpPortal: [%s]", str );
                
                str = ConvertToHexSpaceString ( prefix, prefixSize > 128 ? 128 : prefixSize );
                
                CLogArg ( "sendTcpPortal: Prefix [%s]", str );
				*/
                
                success = portal->device->SendTcpPortal ( (unsigned short)(portal->portalUnitType | portalUnitFlags), portal->portalID, portal->frameCounter, prefix, prefixSize, buffer + offset, contentSize );
                portal->frameCounter++;
            }
            
            return success;
        }
        
        
        /**
         * Acquire a native layer receive identifier (and increase instanceLock on device) to be used in sendTcpPortal. This resource must be released on disposal of the portal generator.
         *
         * @param portalID    Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
         * @return sendID
         */
        ENVIRONSAPI jint EnvironsFunc ( AcquirePortalReceiveIDN, int portalID )
        {
            CVerb ( "AcquirePortalReceiveIDN" );
            
            PortalDevice * portal = GetLockedPortalDevice ( portalID );
            if ( !portal ) {
                CErr ( "AcquirePortalReceiveIDN: No portal resource found." );
                return false;
            }
            
            int receiveID = -1;
            
            if ( portal->receiver ) {
                if ( portal->receiver->stream->GetLock () )
                {
                    portal->stream = portal->receiver->stream;
                    receiveID = GetPortalDeviceID ( portalID );
                }
                else {
                    CWarn ( "AcquirePortalReceiveIDN: Failed to get stream lock." );
                }
            }
            else {
                CWarnArg ( "AcquirePortalReceiveIDN: No portal available with portalID [0x%X]", portalID );
            }

			ReleasePortalDevice ( portal );
            return receiveID;
        }
        
        
        /**
         * Release a native layer send identifier that was acquired by a call to acquirePortalReceiveID.
         *
         * @param receiveID      The portal receive id resource to be released.
         */
        ENVIRONSAPI void EnvironsFunc ( ReleasePortalReceiveIDN, int receiveID )
        {
            CVerb ( "ReleasePortalReceiveIDN" );
            
            if ( receiveID < 0 || receiveID >= MAX_PORTAL_INSTANCES )
                return;
            
            PortalDevice * portal = portalDevices + receiveID;
            
            if ( !HoldPortalDevice ( portal ) )
                return;
            
            if ( portal->stream ) {
                portal->stream->ReleaseLock ();
            }
            
            ReleasePortalDevice ( portal );
        }
        
        
        /*
         * Class:     hcm_environs_Environs
         * Method:    receivePortalUnit
         * Signature: ()Ljava/nio/ByteBuffer;
         */
        ENVIRONSAPI jobject EnvironsFunc ( ReceivePortalUnitN, jint receiveID )
        {
            if ( receiveID < 0 || receiveID >= MAX_PORTAL_INSTANCES ) {
                CErrArg ( "ReceivePortalUnitN: Invalid receiveID [%i].", receiveID );
                return 0;
            }
            
            PortalDevice * portal = portalDevices + receiveID;
            
            if ( !portal->device ) {
                CErrArg ( "ReceivePortalUnitN: No portal device available for receiveID [%i].", receiveID );
                return 0;
            }
            
            return portal->stream->ReceiveStreamPack ();
        }

        
		/**
		* Set render callback.
		*
		* @param async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
		* @param portalDeviceID	This is an ID that Environs use to manage multiple portals. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
		* @param callback		The pointer to the callback.
		* @param callbackType	A value of type RENDER_CALLBACK_TYPE_* that tells the portal receiver what we actually can render..
		* @return				true = success, false = failed.
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetRenderCallbackN, int hInst, int async, jint portalID, void * callback, int callbackType )
		{
			CVerb ( "SetRenderCallbackN" );

			if ( !callback ) {
				CErr ( "SetRenderCallbackN: Invalid callback argument." );
				return false;
			}
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
				success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_SET_RENDERCALLBACK, portalID, callback, callbackType ) );
            else
				success = env->asyncWorker.PortalSetRenderCallback ( portalID, callback, callbackType );

			return success;
		}

        
		/**
		* Set render surface.
		*
		* @param portalID		The portal id.
		* @param renderSurface	A surface on which the portal shall be rendered.
		* @param width		    The width in of the surface in pixel.
		* @param height		    The height of the surface in pixel.
		* @return		true = yes, false = no
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( SetRenderSurfaceN, jint hInst, jint portalID, jobject renderSurface, jint width, jint height )
		{
			CVerb ( "SetRenderSurfaceN" );

			if ( !width || !height ) {
				CErrArg ( "SetRenderSurfaceN: Invalid width/height [%u/%u]", width, height );
				return false;
            }
            
            PortalDevice * portal = GetLockedPortalDevice ( portalID );
            if ( !portal ) {
                CErr ( "SetRenderSurfaceN: No portal resource found." );
                return false;
            }
            
            DUMBJENV ();
            EBOOL success = false;
            
            if ( portal->receiver ) {
                success = portal->receiver->SetRenderSurface ( jenv, renderSurface, width, height );
            }
            
            ReleasePortalDevice ( portal );
            
            return success;
		}


		/**
		* Release render surface.
		*
		* @param async        	Execute asynchronous. Non-async means that this call blocks until the command has finished.
		* @param portalID		The portal id.
		* @return		true = yes, false = no
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( ReleaseRenderSurfaceN, jint hInst, jint async, jint portalDeviceID )
		{
			CVerb ( "ReleaseRenderSurfaceN" );
            
            EBOOL success;
            
            Instance * env = instances[hInst];
            
            if ( async )
                success = ToEBOOL ( env->asyncWorker.PushPortal ( ASYNCWORK_TYPE_PORTAL_RELEASE_RENDERSURFACE, portalDeviceID ) );
            else
                success = env->asyncWorker.PortalReleaseRenderSurface ( portalDeviceID );

			return success;
        }
        
        
        /*
         * Class:     hcm_environs_Environs
         * Method:    requestPortalIntraFrame
         * Signature: ()Z
         */
        ENVIRONSAPI void EnvironsFunc ( RequestPortalIntraFrameN, jint async, jint portalID )
        {
            CVerb ( "RequestPortalIntraFrameN" );
            
            DeviceBase * device = GetDeviceIncLock ( portalID );
            if ( device )
            {
                device->SendPortalMessage ( MSG_PORTAL_IFRAME_REQUEST, portalID );
                
                UnlockDevice ( device );
            }
        }


	}
} // <- namespace environs




