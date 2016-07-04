/**
*	Environs Tracker Bridge that receives TUIO
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
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

/// We include Environs.native.h only for the debug functions.
/// Third party plugins may use their own debug log methods.
#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Interfaces/Interface.Exports.h"
#include "Environs.Build.Lnk.h"

using namespace environs;
using namespace environs::lib;

#include "Track.TUIO.h"


// The TAG for prefixing log messages
#define CLASS_NAME	"TrackTUIO"

#ifdef _WIN32
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")
#endif


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


static const char		*		TrackTUIO_extensionNames[]	= { "TUIO Tracker", "End" };
static const InterfaceType_t	interfaceTypes[]			= { InterfaceType::Tracker, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( TrackTUIO_extensionNames );


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
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();

#if !defined(ENVIRONS_MISSING_TUIO_HEADERS)

/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( TrackTUIO );


namespace environs 
{

	TrackTUIO::TrackTUIO () : initialized (false), tracking (false), tuioReceiver (0), tuioClient (0)
	{		
		name = TrackTUIO_extensionNames [0];
		
		CVerbArg ( "Construct: [%s]", name );

		Zero ( thread_tracking );
	}


	TrackTUIO::~TrackTUIO ()
	{
		CLogArg ( "Destruct: [%s]", name );

		Release ( );

		CVerb ( "Destruct destroyed." );
	}


	void TrackTUIO::Release ()
	{
		CVerb ( "Release" );

		Stop ();

		ReleaseResources ();
	}


	bool TrackTUIO::IsRuntimeSupported ( unsigned int platform, unsigned int sdks )
	{
		CVerb ( "IsRuntimeSupported" );

		/// Verify that we're actually on an MT cell
		unsigned int check = (unsigned int) Platforms::MultiTaction_Flag;

		if ( platform & check )
			return true;

		return false;
	}


	void TrackTUIO::ReleaseResources ()
	{
		CVerb ( "ReleaseResources" );

		if ( tuioClient ) {
			if ( tuioClient->isConnected () )
				tuioClient->disconnect ();

			delete tuioClient;
			tuioClient = 0;
		}

		if ( tuioReceiver ) {
			delete tuioReceiver;
			tuioReceiver = 0;
		}
	}


	int TrackTUIO::AllocateResources ()
	{
		CVerb ( "AllocateResources" );
		
		tuioReceiver = new UdpReceiver ( env->tuioPort );
		if ( !tuioReceiver )
			return 0;

		tuioClient = new TuioClient ( tuioReceiver );
		if ( !tuioClient )
			return 0;

		tuioClient->addTuioListener ( this );
		return 1;
	}


	int	TrackTUIO::Init ()
	{
		CVerb ( "Init" );

		if ( initialized ) {
			Stop ();
			Release ();
			initialized = false;
		}

		if ( !env ) {
			CErr ( "Init: Callback channel to Environs is missing." );
			return 0;
		}

		if ( !width || !height ) {
			CErr ( "Init: Device display with / height is invalid." );
			return 0;
		}

		if ( !AllocateResources () )
			return 0;


		initialized = true;

		CVerbVerb ( "Init successful." );

		return 1;
	}


	int TrackTUIO::Start ()
	{
		CVerb ( "Start" );

		if ( !initialized )
			return 2; /// We return 2 since we expect the framework to initialize and start later.

		if ( !tuioClient )
			return 2;

		if ( !tuioClient->isConnected () ) 
		{
			tuioClient->connect ();
			if ( !tuioClient->isConnected () )
				return 0;
		}

		/// Start tracking thread
		tracking = true;
		
		/*if ( !pthread_valid ( thread_tracking ) ) {
			pthread_create ( &thread_tracking, NULL, &TrackTUIO::Thread_TrackingStarter, this );

			if ( !pthread_valid ( thread_tracking ) ) {
				CErrID ( "Start: Failed to create thread!" );
				return 0;
			}

			if ( callEnvirons )
				callEnvirons->OnStatusMessage ( "Surface Tracker started..." );
		}*/
		return 1;
	}


	int TrackTUIO::Stop ()
	{
		CVerb ( "Stop" );

		tracking	= false;

		//DisposeThread ( thread_tracking, "surface_tracker", event_tracking );

		if ( env )
			env->callbacks.OnStatusMessage ( hEnvirons, "Surface Tracker stopped..." );
		return 1;
	}


	/// return -1: A not recoverable error ocurred
	/// return  0: Queue is still busy
	/// return  1: Image data has been taken over successfuly
	///
	int TrackTUIO::Perform ( void * rawImage, unsigned int size )
	{
		//CVerbVerb ( "Perform" );

		int		ret		= 0;
		char *	buffer	= 0;

		return ret;
	}


	void * TrackTUIO::Thread_TrackingStarter ( void * object )
	{
		TrackTUIO * tracker = (TrackTUIO*) object;

		/// Execute thread
		tracker->Thread_Tracking ( );

		return 0;
	}


	void TrackTUIO::Thread_Tracking ()
	{
		CLog ( "Tracking thread created." );

		pthread_setname_current_envthread ( "TrackTUIO::Worker" );

		while ( tracking ) 
		{
			bool isFree = false;
			
		}
	}


	bool TrackTUIO::Execute ( int command )
	{
		CVerb ( "Execute" );

		switch ( command ) {
		case '?':
			env->callbacks.OnStatusMessage ( hEnvirons, "TUIO Tracker\r\n--------------------\r\ne: Enable tracker\r\nd: Disable Tracker\r\ns: Toggle source window\r\nv: Toggle videofile output" );
			return true;
		}

		return false;
	}

	void TrackTUIO::ConvertToPixel ( float x, float y, int &posX, int &posY )
	{
		posX = (int)(width * x);
		posY = (int) (height * y);
	}

	inline float getDegrees(float rad) 
	{
		return (float) ((rad * 180) / PI);
		//double rad = PI * (dims->orientation - 90) / 180;
	}

	void TrackTUIO::addTuioObject(TuioObject *tobj) {
		int x, y;
		ConvertToPixel ( tobj->getX (), tobj->getY (), x, y );

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_ADD;
		raw->type = INPUT_TYPE_MARKER;
		raw->x = x;
		raw->y = y;
		raw->angle = getDegrees ( tobj->getAngle () );
		raw->value = tobj->getSymbolID ();
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "add obj id [%i] (%i/%i)    x [%i] y[%i] angle [%f]", raw->value, tobj->getSessionID (), tobj->getTuioSourceID (), x, y, raw->angle );
	}

	void TrackTUIO::updateTuioObject(TuioObject *tobj) {
		int x, y;
		ConvertToPixel ( tobj->getX (), tobj->getY (), x, y );

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_CHANGE;
		raw->type = INPUT_TYPE_MARKER;
		raw->x = x;
		raw->y = y;
		raw->angle = getDegrees ( tobj->getAngle () );
		raw->value = tobj->getSymbolID ();
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "set obj id [%i] (%i/%i)    x [%i] y[%i] angle [%f] motionSpeed [%f] rotationSpeed [%f] motAccel [%f] rotAccel [%f]", raw->value,
			tobj->getSessionID (), tobj->getTuioSourceID (), x, y, raw->angle,
			tobj->getMotionSpeed (), tobj->getRotationSpeed (), tobj->getMotionAccel (), tobj->getRotationAccel () );
	}

	void TrackTUIO::removeTuioObject(TuioObject *tobj) {
		int x, y;
		ConvertToPixel(tobj->getX(), tobj->getY(), x, y);

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_DROP;
		raw->type = INPUT_TYPE_MARKER;
		raw->x = x;
		raw->y = y;
		raw->angle = getDegrees ( tobj->getAngle () );
		raw->value = tobj->getSymbolID ();
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "del obj id [%i] (%i/%i) x [%i] y[%i]", raw->value, tobj->getSessionID (), tobj->getTuioSourceID (), x, y );
	}

	void TrackTUIO::addTuioCursor ( TuioCursor *tobj ) {
		int x, y;
		ConvertToPixel ( tobj->getX (), tobj->getY (), x, y );

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_ADD;
		raw->type = INPUT_TYPE_FINGER;
		raw->x = x;
		raw->y = y;
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "add cur id [%i] (%i/%i)    x [%i] y[%i]", tobj->getCursorID (), tobj->getSessionID (), tobj->getTuioSourceID (), x, y );
	}

	void TrackTUIO::updateTuioCursor ( TuioCursor *tobj ) {
		int x, y;
		ConvertToPixel ( tobj->getX (), tobj->getY (), x, y );

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_CHANGE;
		raw->type = INPUT_TYPE_FINGER;
		raw->x = x;
		raw->y = y;
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "add cur id [%i] (%i/%i)    x [%i] y[%i] motionSpeed [%f] motAccel [%f] ", tobj->getCursorID (), tobj->getSessionID (), tobj->getTuioSourceID (), x, y,
			tobj->getMotionSpeed (), tobj->getMotionAccel () );
	}

	void TrackTUIO::removeTuioCursor ( TuioCursor *tobj ) {
		int x, y;
		ConvertToPixel ( tobj->getX (), tobj->getY (), x, y );

		environs::Input in;
		Zero ( in );

		InputPackRaw * raw = &in.pack.raw;

		raw->state = INPUT_STATE_DROP;
		raw->type = INPUT_TYPE_FINGER;
		raw->x = x;
		raw->y = y;
		raw->id = (int)tobj->getSessionID ();

		env->callbacks.OnHumanInput ( hEnvirons, 0, &in );

		CVerbArg ( "del cur id [%i] (%i/%i)", tobj->getCursorID (), tobj->getSessionID (), tobj->getTuioSourceID () );
	}

	void TrackTUIO::addTuioBlob ( TuioBlob *tobj ) {
		CLogArg ( "add blb id [%i] (%i/%i)    x [%f] y[%f] angle [%f] width [%f] height [%f] area [%f]", tobj->getBlobID (), tobj->getSessionID (), tobj->getTuioSourceID (), tobj->getX (), tobj->getY (),
			tobj->getAngle (), tobj->getWidth (), tobj->getHeight (), tobj->getArea () );
	}

	void TrackTUIO::updateTuioBlob ( TuioBlob *tobj ) {
		CLogArg ( "add blb id [%i] (%i/%i)    x [%f] y[%f] angle [%f] width [%f] height [%f] area [%f] motionSpeed [%f] rotationSpeed [%f] motAccel [%f] rotAccel [%f]",
			tobj->getBlobID (), tobj->getSessionID (), tobj->getTuioSourceID (), tobj->getX (), tobj->getY (),
			tobj->getAngle (), tobj->getWidth (), tobj->getHeight (), tobj->getArea (),
			tobj->getMotionSpeed (), tobj->getRotationSpeed (), tobj->getMotionAccel (), tobj->getRotationAccel () );
	}

	void TrackTUIO::removeTuioBlob ( TuioBlob *tobj ) {
		CLogArg ( "del blb id [%i] (%i/%i)", tobj->getBlobID (), tobj->getSessionID (), tobj->getTuioSourceID () );
	}


	void TrackTUIO::refresh ( TuioTime frameTime ) {
	}
} /* namespace environs */

#endif

