/**
*	Environs Tracker Plugin for MS Surface
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
#ifndef ENVIRONS_NATIVE_MODULE
#	define ENVIRONS_NATIVE_MODULE
#endif

#ifndef NDEBUG
#	define DEBUGVERB
#	define DEBUGVERBVerb
#endif

/// We include Environs.native.h only for the debug functions.
/// Third party plugins may use their own debug log methods.
#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Interop/Stat.h"
#include "Interfaces/Interface.Exports.h"
#include "Environs.Build.Lnk.h"

#include "Track.Surface.h"
using namespace environs;

// The TAG for prefixing log messages
#define CLASS_NAME	"TrackSurface"

#include <windows.h>


BOOL APIENTRY DllMain ( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


static const char		*		TrackSurface_extensionNames []	= { "Surface Tabletop Tracker", "End" };
static const InterfaceType_t	interfaceTypes[]				= { InterfaceType::Tracker, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( TrackSurface_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( TrackSurface );


/**
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();


namespace environs 
{	
	const char *	windowSourceName		= "Surface Tracker Source";
	CvVideoWriter * videoOut				= 0;

	TrackSurface::TrackSurface ( )
	{		
		name = TrackSurface_extensionNames [ 0 ];
		
		CVerbArg ( "Construct: [%s]", name );

		videoOut			= 0;

		allocated			= false;
		initialized			= false;
		tracking			= false;
		windowSourceVisible	= false;

		Zero ( thread_tracking );

		imageSize			= 0;
		imageBufferFill		= 0;
		imageBufferTracking	= 0;

		Zero ( image );
		Zero ( imageBuffers );
		Zero ( imageBuffersFree );
	}


	TrackSurface::~TrackSurface ( )
	{
		CLogArg ( "Destruct: [%s]", name );

		Release ( );

		ReleaseCvLib ();

		if ( allocated ) {
			CVerb ( "Destruct: Destroying tracking mutex." );

			if ( pthread_mutex_destroy ( &event_mutex ) ) {
				CVerb ( "Destruct: Failed to destroy event_mutex." );
			}
			
			CVerb ( "Destruct: Closing tracking event." );

			if ( pthread_cond_destroy ( &event_tracking ) ) {
				CVerb ( "Destruct: Failed to destroy event_tracking." );
			}

			CVerb ( "Destruct: Destroying imageBuffer mutex." );

			if ( pthread_mutex_destroy ( &imageBuffer_mutex ) ) {
				CVerb ( "Destruct: Failed to destroy imageBuffer_mutex." );
			}
		}

		CVerb ( "Destruct destroyed." );
	}


	void TrackSurface::Release ( )
	{
		CVerb ( "Release" );

		Stop ();

		if ( videoOut ) {
			dcvReleaseVideoWriter ( &videoOut );
		}

		if ( windowSourceVisible ) {
			dcvWaitKey ( 1 );
			dcvDestroyWindow ( windowSourceName );
		}

		ReleaseResources ();
	}


	bool TrackSurface::IsRuntimeSupported ( unsigned int platform, unsigned int sdks )
	{
		CVerb ( "IsRuntimeSupported" );

		/// Verify that the surface sdk is installed?

		STAT_STRUCT ( st );

		//if ( stat ( "C:/Program Files (x86)/Microsoft Surface/v2.0/SurfaceShell.exe", &st ) != 0 ) {
		if ( stat ( "C:/Program Files (x86)/Microsoft Surface/v2.0", &st ) != 0 ) {
			CWarn ( "IsValid: Surface 2 platform not found." );
			return false;
		}
		
		if ( !InitCvLib ( env, 0 ) ) {
			CWarn ( "IsValid: Initialization of opencv lib failed." );
			return false;
		}
		dcvStartWindowThread ();
		return true;
	}


	void TrackSurface::ReleaseResources ()
	{
		CVerb ( "ReleaseBuffers" );

		for ( int i=0; i < MAX_TRACKER_IMAGE_BUFFERS; i++ ) {
			if ( imageBuffers [i] )
				free ( imageBuffers [i] );
		}
		Zero ( imageBuffers );
		Zero ( imageBuffersFree );
				
		imageSize = 0;
	}


	int TrackSurface::AllocateResources ()
	{
		CVerb ( "AllocateResources" );
		
		if ( !imageSize ) {
			CErr ( "AllocateResources: Invalid image size [0]." );
			return 0;
		}

		if ( &image != dcvInitImageHeader ( &image, cvSize ( width, height ), IPL_DEPTH_8U, channels, 0, 4 ) ) {
			CErr ( "AllocateResources: Failed to initialize image." );
			return 0;
		}
		image.widthStep = stride;

		for ( int i=0; i < MAX_TRACKER_IMAGE_BUFFERS; i++ ) 
		{
			imageBuffers [i] = (char *) malloc ( imageSize );
			if ( !imageBuffers [i] ) {
				CErrArg ( "AllocateResources: Failed to allocate image buffer [%i].", i );
				return 0;
			}

			imageBuffersFree [i] = true;
		}

		image.imageData = imageBuffers [0];

		imageBufferFill		= 0;
		imageBufferTracking = 0;
		
		return 1;
	}


	int	TrackSurface::Init ( )
	{
		CVerb ( "Init" );

		if ( !allocated ) {
			Zero ( event_tracking );
			if ( pthread_cond_init ( &event_tracking, NULL ) ) {
				CErr ( "Init: Failed to init event_tracking!" );
				return 0;
			}

			Zero ( event_mutex );
			if ( pthread_mutex_init ( &event_mutex, NULL ) ) {
				CErr ( "Init: Failed to init event_mutex!" );
				return 0;
			}

			Zero ( imageBuffer_mutex );
			if ( pthread_mutex_init ( &imageBuffer_mutex, NULL ) ) {
				CErr ( "Init: Failed to init imageBuffer_mutex!" );
				return 0;
			}

			allocated = true;
		}

		if ( initialized ) {
			Stop ();
			Release ();
			initialized = false;
		}

		if ( !env ) {
			CErr ( "Init: Callback channel to Environs is missing." );
			return 0;
		}

		imageSize = stride * height * channels;
		imageSize += 4;

		if ( !AllocateResources () )
			return 0;

		initialized = true;

		CVerbVerb ( "Init successful." );

		return 1;
	}


	int TrackSurface::Start ( )
	{
		CVerb ( "Start" );

		if ( !initialized )
			return 2; /// We return 2 since we expect the framework to initialize and start later.
		
		/// Start tracking thread
		tracking = true;
		
		if ( !pthread_valid ( thread_tracking ) ) {
			pthread_create ( &thread_tracking, NULL, &TrackSurface::Thread_TrackingStarter, this );

			if ( !pthread_valid ( thread_tracking ) ) {
				CErrID ( "Start: Failed to create thread!" );
				return 0;
			}

			env->callbacks.OnStatusMessage ( hEnvirons, "Surface Tracker started..." );
		}
		return 1;
	}


	int TrackSurface::Stop ( )
	{
		CVerb ( "Stop" );

		tracking	= false;

		DisposeThread ( 0, thread_tracking, 0, "surface_tracker", event_tracking );

		env->callbacks.OnStatusMessage ( hEnvirons, "Surface Tracker stopped..." );
		return 1;
	}


	/// return -1: A not recoverable error ocurred
	/// return  0: Queue is still busy
	/// return  1: Image data has been taken over successfuly
	///
	int TrackSurface::Perform ( void * rawImage, unsigned int size )
	{
		//CVerbVerb ( "Perform" );

		int		ret		= 0;
		char *	buffer	= 0;

		if ( imageSize < size || !tracking ) {
			CErrArg ( "Perform: Size of raw image [%u] is larger than allocated buffer [%u]", size, imageSize );
			return -1;
		}

		if ( pthread_mutex_lock ( &imageBuffer_mutex ) ) {
			CErr ( "Perform: Failed to aquire mutex!" );
			return -1;
		}

		if ( !imageBuffersFree [imageBufferFill] )
			goto Unlock;

		if ( pthread_mutex_unlock ( &imageBuffer_mutex ) ) {
			CErr ( "Perform: Failed to release mutex!" );
			return -1;
		}
		//CVerbVerbArg ( "Perform: copy [%i]", imageBufferFill );

		memcpy ( imageBuffers [imageBufferFill], rawImage, size );

		if ( pthread_mutex_lock ( &imageBuffer_mutex ) ) {
			CErr ( "Perform: Failed to aquire mutex!" );
			return -1;
		}

		imageBuffersFree [imageBufferFill] = false;
		ret = 1;

	Unlock:
		if ( pthread_mutex_unlock ( &imageBuffer_mutex ) ) {
			CErr ( "Perform: Failed to release mutex!" );
			return -1;
		}

		if ( ret == 1 ) {
			/// Trigger tracking thread
			if ( pthread_cond_signal ( &event_tracking ) ) {
				CErr ( "Perform: Failed to signal tracking event!" );
			}

			imageBufferFill++;
			if ( imageBufferFill >= MAX_TRACKER_IMAGE_BUFFERS )
				imageBufferFill = 0;
		}
		return ret;
	}


	void * TrackSurface::Thread_TrackingStarter ( void * object )
	{
		TrackSurface * tracker = (TrackSurface*) object;

		/// Execute thread
		tracker->Thread_Tracking ( );

		return 0;
	}


	void TrackSurface::Thread_Tracking ( )
	{
		CLog ( "Tracking thread created." );

		pthread_setname_current_envthread ( "TrackSurface::Worker" );

		unsigned int imageBufferToRelease = MAX_TRACKER_IMAGE_BUFFERS;

		while ( tracking ) 
		{
			bool isFree = false;
			
			//CVerbVerbArg ( "Tracking check  [%i] ...", imageBufferTracking );
			if ( pthread_mutex_lock ( &imageBuffer_mutex ) ) {
				CErr ( "Thread_Tracking: Failed to aquire mutex!" );
				break;
			}

			if ( imageBufferToRelease < MAX_TRACKER_IMAGE_BUFFERS ) 
			{
				imageBuffersFree [imageBufferTracking] = true;
				imageBufferToRelease = MAX_TRACKER_IMAGE_BUFFERS;
			}

			isFree = imageBuffersFree [imageBufferTracking];

			if ( pthread_mutex_unlock ( &imageBuffer_mutex ) ) {
				CErr ( "Thread_Tracking: Failed to release mutex!" );
				break;
			}

			if ( isFree ) {
				pthread_cond_mutex_lock ( &event_mutex );

				if ( pthread_cond_wait_time ( &event_tracking, &event_tracking, INFINITE ) || !tracking )
				{
					pthread_cond_mutex_unlock ( &event_mutex );
					break;
				}
				pthread_cond_mutex_unlock ( &event_mutex );
				continue;
			}
			
			//CVerbVerbArg ( "Tracking  [%i] ...", imageBufferTracking );

			image.imageData = imageBuffers [imageBufferTracking];

			/// Tracking start...
			if ( windowSourceVisible ) {
				dcvShowImage ( windowSourceName, &image );
				//dcvWaitKey ( 1 );
			}

			if ( videoOut )
				dcvWriteFrame ( videoOut, &image );

			/// Tracking end
			imageBufferToRelease = imageBufferTracking;

			imageBufferTracking++;
			if ( imageBufferTracking >= MAX_TRACKER_IMAGE_BUFFERS )
				imageBufferTracking = 0;
		}
	}


	bool TrackSurface::Execute ( int command )
	{
		CVerb ( "Execute" );

		switch ( command ) {
		case 's':
			ToggleSource ();
			return true;
		case 'v':
			ToggleVideoOut ( );
			return true;
		case '?':
			env->callbacks.OnStatusMessage ( hEnvirons, "Surface Tracker\r\n--------------------\r\ne: Enable tracker\r\nd: Disable Tracker\r\ns: Toggle source window\r\nv: Toggle videofile output" );
			return true;
		}

		return false;
	}


	/// Debug functionality

	void TrackSurface::ToggleSource ( )
	{
		CVerb ( "ToggleSource" );

		if ( windowSourceVisible ) {
			dcvWaitKey ( 1 );
			dcvDestroyWindow ( windowSourceName );
			windowSourceVisible = false;
			env->callbacks.OnStatusMessage ( hEnvirons, "Tracking source closed." );
		}
		else {
			dcvNamedWindow ( windowSourceName, 1 );
			windowSourceVisible = true;
			env->callbacks.OnStatusMessage ( hEnvirons, "Tracking source visible." );
		}
	}


	void TrackSurface::ToggleVideoOut ( )
	{
		CVerb ( "ToggleVideoOut" );

		if ( videoOut ) {
			CvVideoWriter * videoOutTmp = videoOut;
			videoOut = 0;
			Sleep ( 100 );
			dcvReleaseVideoWriter ( &videoOutTmp );

			env->callbacks.OnStatusMessage ( hEnvirons, "Video out finalized." );
		}
		else {
			videoOut = dcvCreateVideoWriter (
				"./surface_tracker.avi",
				//CV_FOURCC ( 'M', 'J', 'P', 'G' ),
				//CV_FOURCC ( 'X', 'V', 'I', 'D' ),
				-1,
				10,
				(!width ||  !height) ? cvSize ( 640, 480 ) : cvSize ( width, height ),
				0
				);

			if ( videoOut )
				env->callbacks.OnStatusMessage ( hEnvirons, "Video out created." );
			else
				env->callbacks.OnStatusMessage ( hEnvirons, "Failed to create video out ." );
		}
	}

} /* namespace environs */