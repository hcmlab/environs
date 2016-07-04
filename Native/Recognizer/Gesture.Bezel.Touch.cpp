/**
 *	Bezel Touch Gestures Recognizer
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
#   define DEBUGVERB
#   define DEBUGVERBVerb
#endif

#ifndef ENVIRONS_NATIVE_MODULE
#   define ENVIRONS_NATIVE_MODULE
#endif

#include "Environs.Obj.h"
#include "Environs.Native.h"
#include "Gesture.Bezel.Touch.h"
#include "Device/Device.Base.h"
#include "Interfaces/Interface.Exports.h"

using namespace environs;
using namespace environs::lib;


#include <cmath>

#define CLASS_NAME	"Gesture.Bezel.Touch. . ."

#define BEZEL_TRIGGER_MM    8

static const char		*		GestureBezelTouchRecognizer_extensionNames []	= { "Bezel Touch Recognizer", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	GestureBezelTouchRecognizer_interfaceTypes[]	= { InterfaceType::InputRecognizer, InterfaceType::Unknown };


/**
 * GetINames
 *
 *	@param	size	on success, this argument is filled with the count of names available in the returned array.
 *
 *	@return returns an array of user readable friendly names in ASCII encoding.
 *
 */
BUILD_INT_GETINAMES ( GestureBezelTouchRecognizer_extensionNames );


/**
 * GetITypes
 *
 *	@param	size	on success, this argument is filled with the count of types available in the returned array.
 *
 *	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
 *
 */
BUILD_INT_GETITYPES ( GestureBezelTouchRecognizer_interfaceTypes );


/**
 * CreateInstance
 *
 *	@param	index		the index value of one of the plugin types returned in the array through getITypes().
 *	@param	deviceID	the deviceID that the created interface object should use.
 *
 *	@return An object that supports the requested interface. 0 in case of error.
 *
 */
BUILD_INT_CREATEOBJ ( GestureBezelTouch );

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
	GestureBezelTouch::GestureBezelTouch ( )
	{
		CVerbID ( "Construct" );

		name				= GestureBezelTouchRecognizer_extensionNames[0];

		ZeroStruct ( info, PortalInfoBase );
		triggerTouchCount	= 1;
		prevFingCount		= 0;
		prevX 				= 0;
		prevY 				= 0;
		prevXcached			= 0;
		prevYcached			= 0;
		iniDist 			= 0;
		iniWidth			= 0;
		iniHeight			= 0;
		scaleDist			= 0;
		portalID			= 0;
		deviceDiag			= 0;
        bezelThreshold      = 20;
	}


	GestureBezelTouch::~GestureBezelTouch ( )
	{
		CVerbID ( "Destructor" );
	}
	

	bool GestureBezelTouch::Init ( )
	{
		CVerbID ( "Init" );
        
        if ( display.width == 0 || display.height == 0 || display.width_mm == 0 || display.height_mm == 0 )
            return false;

        // Determine pixels for 8mm
        bezelThreshold = display.width * BEZEL_TRIGGER_MM / display.width_mm;
        
        CLogArg ( "Init: Trigger threshold set to [%i]", bezelThreshold );
        
		deviceDiag = (unsigned int) sqrt ( (double) ((display.width * display.width) + (display.height * display.height)) );
        
		CVerbArg ( "Init: device_width [%u]  device_height [%u]  diag [%u]", display.width, display.height, deviceDiag );

		return true;
	}


	bool GestureBezelTouch::SetIncomingPortalID ( int portalID )
	{
		CVerbArgID ( "SetIncomingPortalID: [%i]", portalID );

		this->portalID = portalID;
		return true;
	}
	

	int GestureBezelTouch::Trigger ( InputPackRec ** touches, int touchesCount )
	{
		CVerbVerbID ( "Trigger" );
        
        if ( !deviceBase )
            return RECOGNIZER_REJECT;
        
        deviceID = ((DeviceBase *)deviceBase)->deviceID;
        
		InputPackRaw * touch = &touches [0]->raw;

		CVerbVerbArg ( "Trigger: x/y [%i/%i]  bezelThresholds [%i/%i]", touch->x, touch->y, bezelThreshold, (display.width - bezelThreshold) );

		if ( touch->x < bezelThreshold || (unsigned int) touch->x > ( display.width - bezelThreshold ) ) {
			prevX = touch->x; 
			prevY = touch->y;
			ZeroStruct ( info, PortalInfoBase );

			prevXcached	= 0;
			prevYcached	= 0;

			info.portalID = portalID;
            prevFingCount = 1;

            if ( !deviceBase || !((DeviceBase *)deviceBase)->GetPortalInfo ( &info ) ) {
                CVerbArgID ( "Trigger: GetPortalInfo portalID [%u] failed.", info.portalID );
                return RECOGNIZER_REJECT;
            }
            
            CVerbArg ( "Trigger: GetPortalInfo portalID [%u] centerX [%i]  centerY [%i]  width [%i]  height [%i]", info.portalID, info.centerX, info.centerY, info.width, info.height );
            return RECOGNIZER_TAKEN_OVER_INPUTS;
		}

		return RECOGNIZER_REJECT;
	}


	int GestureBezelTouch::Perform ( InputPackRec ** touches, int touchesCount )
	{
		CVerbVerb ( "Perform" );
        
        if ( !deviceBase || touchesCount <= 0 )
            return RECOGNIZER_GIVE_BACK_INPUTS;
        
        if ( !((DeviceBase *)deviceBase)->SetPortalInfoPosibble() )
            return RECOGNIZER_HANDLED;
        
		if ( touchesCount == 1 ) 
        {
            InputPackRaw * touch = &touches [0]->raw;
            
            if ( prevFingCount != 1 ) {
                prevX = touch->x;
                prevY = touch->y;
                prevFingCount = 1;
                return RECOGNIZER_HANDLED;
            }
            
			int dX = prevX - touch->x;
			int dY = prevY - touch->y;
			prevX = touch->x; prevY = touch->y;
			iniDist = 0;

			while ( dX > 1 || dX < -1 || dY > 1 || dY < -1 )
            {
				int dXP = (int) ((float) (dX * info.width) / display.width);
				int dYP = (int) ((float) (dY * info.height) / display.height);
				//CVerbArgID ( "Perform: dx [%i]  dy [%i] - width [%i/%i]  height [%i/%i] - px [%i]  py [%i] ", dX, dY, info.width, device_width, info.height, device_height, dXP, dYP );

				if ( prevXcached == dXP && prevYcached == dYP )
                    break;

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


				info.portalID = portalID;
				info.flags = PORTAL_INFO_FLAG_LOCATION;
                
                if ( deviceBase ) {
                    CVerbVerbArgID ( "Perform: setPortalInfo1 portalID [%u] x [%i]  y [%i]", info.portalID, info.centerX, info.centerY );
                    
                    ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false );
                }
                break;
			}
            
            return RECOGNIZER_HANDLED;
		}
		
		if ( touchesCount == 2 ) 
		{
            prevFingCount = 2;
            
            InputPackRaw * t1 = &touches [0]->raw;
            InputPackRaw * t2 = &touches [1]->raw;

			int td1 = (t1->x - t1->y);
			int td2 = (t2->x - t2->y);
			int dist = (int) sqrt ( (double) (td1 * td1) + (td2 * td2) );

			CVerbVerbArgID ( "Perform: prevDist [%i]  dist [%i]", iniDist, dist );

			if ( iniDist == 0 ) {
				iniDist = dist;
                
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
				//scaleDist *= TouchSource::platformScaler;

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
                        
                        info.portalID = portalID;
                        info.flags = PORTAL_INFO_FLAG_SIZE;
                        
                        if ( deviceBase ) {
                            CVerbArgID ( "Perform: setPortalInfo1 portalID [%u] w [%i]  h [%i]", info.portalID, info.width, info.height );
                            
                            ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false );
                        }
                    }
				}
			}

			return RECOGNIZER_HANDLED;
		}
        
        return RECOGNIZER_GIVE_BACK_INPUTS;
	}
    
    
    void GestureBezelTouch::Flush ( )
    {
        CVerbVerbID ( "Flush" );
        
        /*if ( deviceBase )
            ((DeviceBase *)deviceBase)->SetPortalInfo ( &info, false );*/
    }
    
    
} /* namespace environs */
