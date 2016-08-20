/**
 * Environs OSX platform specific
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

#include "Environs.OSX.h"
#include "Environs.Sensors.h"
#

#if defined(ENVIRONS_OSX)


#include "Environs.Obj.h"

// Releases should change installation directory to @executable_path/../../../

// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.OSX . . . . . ."


namespace environs 
{

    namespace API
    {
        
        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailableImpl ( int hInst, environs::SensorType_t sensorType )
        {
            return false;
        }
        
        
        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         */
        bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            //synchronized (sensorManager) {
            switch ( sensorType )
            {
                case environs::SensorType::Accelerometer:
                    
                    break;
                    
                case environs::SensorType::MagneticField:
                    
                    break;
                    
                case environs::SensorType::Gyroscope:

                    break;
                    
                default:
                    break;
            }
            //}
            return false;
        }
        
        
        /**
         * Deregister to sensor events and stop listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         */
        void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            if (sensorType == -1) {
                return;
            }
            
            switch ( sensorType )
            {
                case environs::SensorType::Accelerometer:
                    
                    break;
                    
                case environs::SensorType::MagneticField:
                    
                    break;
                    
                case environs::SensorType::Gyroscope:
                    
                    break;
                    
                default:
                    break;
            }
        }
    }

} /* namespace environs */

#endif

