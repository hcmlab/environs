/**
 * Three Finger Gesture Recognizer
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


#ifndef DISPLAYDEVICE
#   define DISPLAYDEVICE
#endif

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

#include "Environs.Obj.h"
#include "Gesture.Three.Finger.h"
#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"
#include "Portal.Info.Base.h"
#include "Environs.Build.Lnk.h"

using namespace environs;
using namespace environs::lib;

#include <cmath>

#define CLASS_NAME	"Gesture.Three.Finger . ."

static const char		*		GestureThreeFingerRecognizer_extensionNames []	= { "Three Finger Touch Recognizer", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	GestureThreeFingerRecognizer_interfaceTypes []	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };

/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( GestureThreeFingerRecognizer_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( GestureThreeFingerRecognizer_interfaceTypes );


/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( GestureThreeFinger );


/**
 * SetEnvironsMethods
 *
 *	Injects environs runtime methods.
 *
 */
BUILD_INT_SETENVIRONSOBJECT ();
#endif

namespace environs 
{

// -------------------------------------------------------------------
// Constructor
//		Initialize member variables
// -------------------------------------------------------------------
GestureThreeFinger::GestureThreeFinger ()
{
	CLogID ( "Construct" );

	name				= GestureThreeFingerRecognizer_extensionNames[0];
	
	MoveID = 0;
	MoveXInit = 0;
	MoveYInit = 0;
	Referece1X = 0;
	Referece1Y = 0;
	Referece2X = 0;
	Referece2Y = 0;
	MoveXDiff = 0;
	MoveYDiff = 0;
	MiddleX = 0;
	MiddleY = 0;
	InitWidth = 0;
	InitHeight = 0;
	MiddleID = 123456789;
	mode = 0;
}


GestureThreeFinger::~GestureThreeFinger ()
{
	CLogID ( "Destructor" );
}


bool GestureThreeFinger::Init ()
{
	/*if ( !parent )
		return false;

	deviceID = ((DeviceBase *)parent)->deviceID;*/

	CVerbID ( "Init" );

	return true;
}


bool IntersectionPoint ( double yintercept1, double mx1, double my1, double yintercept2, double mx2, double my2, int &x3, int &y3 )
{
	double mipy = ( my1 * mx2 ) - ( my2 * mx1 );
	double mipx = mx1 * mx2;

	//double mip = mipy / mipx;

	x3 = ( int ) ( ( ( yintercept2 - yintercept1 ) * mipx ) / mipy );
	y3 = ( int ) ( ( ( ( ( my1 * yintercept2 ) / mx1 ) - ( ( my2 * yintercept1 ) / mx2 ) ) * mipx ) / mipy );

	return true;
}


int GestureThreeFinger::Trigger ( InputPackRec **	inputs, int inputCount )
{
	return 2;
}


void GestureThreeFinger::Finish ( InputPackRec **	inputs, int inputCount )
{
}


int GestureThreeFinger::Perform ( InputPackRec **	inputs, int inputCount )
{
	Input * inputContainer = ( Input * ) inputs;

	Input * t = 0;

	if ( inputCount < 3 )
	{
		if ( mode )
		{
			/*int mX = MiddleX, my = MiddleY;
			TranslateToSurfaceCoord ( this, mX, my );
			onEnvironsTouch ( MiddleID, deviceID, 2, mX, my, 0 );*/

			// We were detecting, while the minimum finger count has been dismissed, go for regular touch
			mode = 0;

			/*for ( unsigned int i = 0; i < count; i++ )
			{
			t = (*touches)[i];

			if (inject_touch) {
			InjectedTouchAdd t ( 0, t->id, t->x, t->y, t->angle );
			InjectTouch ( (InjectedTouch *)&t );
			}
			else
			onEnvironsTouch ( t->id, deviceID, 0, t->x, t->y, t->angle );

			CLogArg ( "RecognizeGestures: Touch id [%i] reAdded as new", t->id );
			}*/
		}
		return false;
	}

	if ( !mode )
	{
		mode = 1;

		t = inputContainer;
		MoveXInit = t->pack.raw.x;
		MoveYInit = t->pack.raw.y;
		Referece1X = t->x_raw;
		Referece1Y = t->y_raw;
		MoveID	 = t->pack.raw.id;
		MoveXDiff = 0;
		MoveYDiff = 0;

		t = inputContainer + 1;
		Referece2X = t->x_raw;
		Referece2Y = t->y_raw;

		for ( int i = 0; i < inputCount; i++ )
		{
			t = inputContainer + i;
			t->pack.raw.state = INPUT_STATE_DROP;

			if ( deviceBase ) {
				( ( DeviceBase * ) deviceBase )->PerformEnvironsTouch ( t );
			}

			CVerbArgID ( "RecognizeGestures: Touch id [%i] dropped touch and switched to gesture recognizer", t->pack.raw.id );
		}
	}

	// Recognizing...

	// 1. Looking for the posture that all three contacts form and calculate their middle point

	// Looking for two movement vectors and their intersection point
	if ( mode == 1 )
	{
		// Determine the middle point
		Input * t1 = inputContainer;
		Input * t2 = inputContainer + 1;
		Input * t3 = inputContainer + 2;

		// Determine middle point (M12X / M12Y) between t1 and t2
		double M12X = ( t1->x_raw + t2->x_raw ) / 2;
		double M12Y = ( t1->y_raw + t2->y_raw ) / 2;
		double c = t3->x_raw - M12X;

		if ( c == 0 )
			c += 0.25f;
		double yintercept123 = M12Y - ( ( ( t3->y_raw - M12Y ) * M12X ) / c );

		// Determine middle point (M23X / M23Y) between t2 and t3
		double M23X = ( t3->x_raw + t2->x_raw ) / 2;
		double M23Y = ( t3->y_raw + t2->y_raw ) / 2;
		c = t1->x_raw - M23X;

		if ( c == 0 )
			c += 0.25f;
		double yintercept231 = M23Y - ( ( ( t1->y_raw - M23Y ) * M23X ) / c );

		IntersectionPoint ( yintercept123, ( t3->x_raw - M12X ), ( t3->y_raw - M12Y ), yintercept231, ( t1->x_raw - M23X ), ( t1->y_raw - M23Y ), MiddleX, MiddleY );

		CVerbArgID ( "RecognizeGestures: P1[%i, %i] - P2[%i, %i] - P3[%i, %i] - Intersect[%i, %i]",
			t1->x_raw, t1->y_raw, t2->x_raw, t2->y_raw, t3->x_raw, t3->y_raw, MiddleX, MiddleY );

		/*int mX = MiddleX, my = MiddleY;
		TranslateToSurfaceCoord ( this, mX, my );
		onEnvironsTouch ( MiddleID, deviceID, 0, mX, my, 0 );*/

		// Calculate the distances to the middle point

		double distM1 = sqrt ( pow ( ( double ) ( t1->x_raw - MiddleX ), 2 ) + pow ( ( double ) ( t1->y_raw - MiddleY ), 2 ) );
		double distM2 = sqrt ( pow ( ( double ) ( t2->x_raw - MiddleX ), 2 ) + pow ( ( double ) ( t2->y_raw - MiddleY ), 2 ) );
		double distM3 = sqrt ( pow ( ( double ) ( t3->x_raw - MiddleX ), 2 ) + pow ( ( double ) ( t3->y_raw - MiddleY ), 2 ) );
		CVerbArgID ( "RecognizeGestures: distM1[%f] - distM1[%f]  - distM1[%f]", distM1, distM2, distM3 );

		double distMax =  fmax ( fmax ( distM1, distM2 ), distM3 );
		double distThreshold = distMax / 2;

		if ( std::abs ( distM1 - distMax ) < distThreshold && std::abs ( distM2 - distMax ) < distThreshold && std::abs ( distM3 - distMax ) < distThreshold )
		{
			CVerbID ( "RecognizeGestures: Recognized zoom/pinch gesture" );
			mode = 20;

			InitWidth = display.width;
			InitHeight = display.height;
		}
		else
		{
			CVerbID ( "RecognizeGestures: Recognized navigation gesture" );
			mode = 10;
		}

		return true;
	}

	if ( mode == 2 )
	{
		// Determine the motion vectors
		/*Touch * t1 = (*touches)[0];
		Touch * t2 = (*touches)[1];
		Touch * t3 = (*touches)[2];*/

		// Orthovec1 t3, 1/-m12
		// Orthovec2 t1, 1/-m23
		InitWidth = display.width;
		InitHeight = display.height;

		CVerbArgID ( "RecognizeGestures: Init size[%i, %i]", InitWidth, InitHeight );
		mode = 20;
	}

	if ( mode == 10 ) // Moving mode
	{
		// Are they going into the same direction? -> move
		// Looking for the reference touch	
		for ( int i = 0; i < inputCount; i++ )
		{
			t = inputContainer + i;
			if ( t->pack.raw.id == MoveID )
			{
				int diffX = t->x_raw - Referece1X;
				int diffY = t->y_raw - Referece1Y;

				if ( diffX != MoveXDiff || diffY != MoveYDiff ) {
					MoveXDiff = diffX;
					MoveYDiff = diffY;

					/*CVerbArg ( "RecognizeGestures: left:[%i] StartX:[%i] NowX:[%i] diffX[%i] - diffY [%i] NowY:[%i] StartY:[%i] - Moving to [%i : %i]",
					left, Referece1X, t->x, diffX, diffY, t->y, Referece1Y, MoveXInit - diffX, MoveYInit - diffY );*/

					// Move the portal
					if ( deviceBase ) {
						( ( DeviceBase * ) deviceBase )->UpdatePosition ( 0, MoveXInit - diffX, MoveYInit - diffY, -1 );
					}

					return true;
				}
			}
		}
		return false;
	}

	if ( mode == 20 ) // Zooming mode
	{
		// Looking for the reference touch	
		for ( int i = 0; i < inputCount; i++ )
		{
			t = inputContainer + 1;
			if ( t->pack.raw.id == MoveID )
			{
				// Calculate length of initial point to middle point
				double lenRef = sqrt ( pow ( ( double ) ( MiddleX - Referece1X ), 2 ) + pow ( ( double ) ( MiddleY - Referece1Y ), 2 ) );

				// Calculate length of reference point to middle point
				double lenNow = sqrt ( pow ( ( double ) ( MiddleX - t->x_raw ), 2 ) + pow ( ( double ) ( MiddleY - t->y_raw ), 2 ) );

				// Determine the scale and apply this to the portal size

				int diffX = ( int ) lenNow;

				if ( diffX != MoveXDiff ) {
					MoveXDiff = diffX;

					double scale = ( lenRef / lenNow );
					int width_new = ( int ) ( scale * InitWidth );

					int height_new = ( int ) ( scale * InitHeight );

					CVerbArgID ( "RecognizeGestures: Length of Ref:[%i] - Length now:[%i] - Scale:[%4.2f] - Width:[%i] - Height[%i]",
						( int ) lenRef, diffX, scale, width_new, height_new );

					// Scale the portal
					if ( deviceBase ) {
						( ( DeviceBase * ) deviceBase )->UpdatePortalSize ( 0, width_new, height_new );
					}
					return true;
				}
			}
		}
		return false;
	}

	if ( mode == 30 ) // Rotating mode
	{
		return false;
	}

	return false;
}


} /* namespace environs */
