/**
 *	Three Finger Touch Gestures Recognizer
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

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Gesture.Three.Touch.h"
#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"

using namespace environs;
using namespace environs::lib;

#include <cmath>

#define CLASS_NAME	"Gesture.Three.Touch. . ."
#define BEZEL_THRESHOLD		20


static const char		*		GestureThreeTouchRecognizer_extensionNames []	= { "Three Finger Touch Recognizer", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	GestureThreeTouchRecognizer_interfaceTypes []	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };



/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( GestureThreeTouchRecognizer_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( GestureThreeTouchRecognizer_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( GestureThreeTouch );


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
	GestureThreeTouch::GestureThreeTouch ( )
	{
		CLogID ( "Construct" );

		name				= GestureThreeTouchRecognizer_extensionNames[0];

		ZeroStruct ( info, PortalInfoBase );
		triggerTouchCount	= 3; /// makes sense...

		prevX 				= 0;
		prevY 				= 0;
		iniDist 			= 0;
		scaleDist			= 0;
		portalID			= 0;
	}


	GestureThreeTouch::~GestureThreeTouch ( )
	{
		CVerbID ( "Destructor" );
	}
	

	bool GestureThreeTouch::Init ( )
	{
		CVerbID ( "Init" );

		if ( display.width == 0 || display.height == 0 || display.width_mm == 0 || display.height_mm == 0 )
			return false;

		CVerbArg ( "Init: device_width [%u]  device_height [%u]", display.width, display.height );

		return true;
	}


	bool GestureThreeTouch::SetIncomingPortalID ( int portalIDa )
	{
		portalID = portalIDa;
		return true;
	}
	

	int GestureThreeTouch::Trigger ( InputPackRec ** touches, int touchesCount )
	{
		CVerbVerbID ( "Trigger" );

		prevXcached	= 0;
		prevYcached	= 0;

		prevFingCount = touchesCount;
        
        if ( !deviceBase )
            return RECOGNIZER_REJECT;
        
        deviceID = ((DeviceBase *)deviceBase)->deviceID;
		return RECOGNIZER_TAKEN_OVER_INPUTS;
	}


	int GestureThreeTouch::Perform ( InputPackRec ** touches, int touchesCount )
	{
		CVerbVerbID ( "Perform" );
        
        if ( deviceBase && !((DeviceBase *)deviceBase)->SetPortalInfoPosibble() )
            return RECOGNIZER_HANDLED;
        
		if ( touchesCount == 1 ) 
		{
			CVerbVerbID ( "Perform: 1 finger" );

			InputPackRaw * touch = &touches [0]->raw;

            iniDist = 0;
            
			if ( prevFingCount != 1 ) {
				prevX = touch->x;
				prevY = touch->y;
				ZeroStruct ( info, PortalInfoBase );

				info.portalID = portalID;
                
                if ( !deviceBase || !((DeviceBase *)deviceBase)->GetPortalInfo ( &info ) ) {
                    CVerbArgID ( "Perform: GetPortalInfo portalID [%u] failed.", info.portalID );
                    return RECOGNIZER_REJECT;
                }

				prevFingCount = 1;
				CVerbArgID ( "Perform: GetPortalInfo portalID [%u] centerX [%i]  centerY [%i]  width [%i]  height [%i]", info.portalID, info.centerX, info.centerY, info.width, info.height );
				return RECOGNIZER_HANDLED;
			}

			int dX = prevX - touch->x;
			int dY = prevY - touch->y;
			prevX = touch->x; prevY = touch->y;
			iniDist = 0;

			if ( dX || dY ) {
				int dXP = (int) ((float) (dX * info.width) / display.width);
				int dYP = (int) ((float) (dY * info.height) / display.height);
				//CVerbArgID ( "Perform: dx [%i]  dy [%i] - width [%i/%i]  height [%i/%i] - px [%i]  py [%i] ", dX, dY, info.width, device_width, info.height, device_height, dXP, dYP );

				if ( prevXcached == dXP && prevYcached == dYP )
					return 1;

				prevXcached = dXP; prevYcached = dYP;

				if (info.orientation == 90.0f) {
					info.centerX += dXP;
					info.centerY += dYP;
				}
				else if (info.orientation == 0.0f) {
					info.centerX += dYP;
					info.centerY -= dXP;
				}
				else if (info.orientation == 180.0f) {
					info.centerX -= dYP;
					info.centerY += dXP;
				}
				else if (info.orientation == 270.0f) {
					info.centerX -= dXP;
					info.centerY -= dYP;
				}
				else {
					//CLogArgID ( "Perform: portalInfo angle [%f]", info.orientation );
					/// Transform surface angle to cartesian angle
					/// double theta = 180 - info.orientation;

					/// Add marker angle offset (showing upwards on the surface means 0 degree on the tablet) + 90
					/// double theta = 270 - info.orientation;
					double theta = (double)(((270.0f - info.orientation) * (double)PI) / (double)180.0);

					theta = -theta;

					//double theta = info.orientation + 90;
	                //if (theta < 0)
	                //	theta = 360 + theta;
	                double cosV = cos(theta);
	                double sinV = sin(theta);

	                double xV = (double)dXP * cosV - (double)dYP * sinV;
	                double yV = (double)dXP * sinV + (double)dYP * cosV;

					info.centerX -= (int)xV;
					info.centerY -= (int)yV;
				}

				CVerbArgID ( "Perform: setPortalInfo1 portalID [%u] x [%i]  y [%i]", info.portalID, info.centerX, info.centerY );

				info.portalID = portalID;
				info.flags = PORTAL_INFO_FLAG_LOCATION;
                
                if ( deviceBase ) {
                    ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false );
                }
			}

			return RECOGNIZER_HANDLED;
		}
		
		if ( touchesCount == 2 ) 
		{
			CVerbVerbID ( "Perform: 2 finger" );
            
			prevFingCount = 2;

			InputPackRaw * t1 = &touches [0]->raw;
			InputPackRaw * t2 = &touches [1]->raw;

			int td1 = (t1->x - t1->y);
			int td2 = (t2->x - t2->y);
			int dist = (int) sqrt ( (double) (td1 * td1) + (td2 * td2) );
            
			CVerbVerbArgID ( "Perform: prevDist [%i]  dist [%i]", iniDist, dist );
            
			if ( iniDist == 0 ) {
				iniDist = dist;
                
                if ( prevFingCount != 1 ) {
                    info.portalID = portalID;
                }
                
                if ( !deviceBase || !((DeviceBase *)deviceBase)->GetPortalInfo ( &info ) ) {
                    CVerbArgID ( "Perform: GetPortalInfo portalID [%u] failed.", info.portalID );
                    return RECOGNIZER_REJECT;
                }
                
                iniWidth = info.width;
                iniHeight = info.height;
                
				if ( info.width > 0 )
                    scaleDist = ((double) info.width / (double) display.width);
				else
                    scaleDist = 0.4;
                
                CVerbArgID ( "Perform: Start pinch gesture with dist [%i] scaleDist [%f]  width [%i]  height [%i].", iniDist, scaleDist, info.width, info.height );
			}
			else {
				int distDiff = iniDist - dist;
				distDiff = (int) ((double) distDiff * scaleDist);
                
				if ( info.width > 0 && (distDiff > 6 || distDiff < -6) )
                {
					int newWidth = iniWidth + distDiff;
					if ( abs ( (double) (info.width - newWidth) ) > 4 ) {
                        info.width = newWidth;
                        
                        /// Keep aspect ratio
                        int distDiffY = (distDiff * info.height) / info.width;
                        info.height = iniHeight + distDiffY;
                        
                        CVerbArgID ( "Perform: setPortalInfo1 portalID [%u] w [%i]  h [%i]", info.portalID, info.width, info.height );
                        
                        info.portalID = portalID;
                        info.flags = PORTAL_INFO_FLAG_SIZE;
                        
                        if ( deviceBase && ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false ) )
                            return RECOGNIZER_HANDLED;
                        return RECOGNIZER_REJECT;
                    }
				}
			}

			return RECOGNIZER_HANDLED;
		}

		if ( touchesCount == 3 && prevFingCount == 3 ) {
			CVerbVerbID ( "Perform: 3 finger" );
            iniDist = 0;
			return RECOGNIZER_TAKEN_OVER_INPUTS;
		}

		return RECOGNIZER_GIVE_BACK_INPUTS;
    }
    
    
    void GestureThreeTouch::Flush ( )
    {
        CVerbVerbID ( "Flush" );
        
        /*if ( deviceBase )
            ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false );
        */
    }


} /* namespace environs */
