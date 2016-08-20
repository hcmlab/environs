/**
 * Environs Sensor Handling
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
#ifndef INCLUDE_HCM_ENVIRONS_SENSORS_H
#define INCLUDE_HCM_ENVIRONS_SENSORS_H

#include "Environs.Lib.Inc.h"
#include "Interop/Threads.h"

#ifndef CLI_CPP
#include "Device/Device.Base.h"

#else
#endif

/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif

#ifndef CLI_CPP
    
    extern pthread_mutex_t                 sensorsMutex;
        
#endif
    
    /**
     * Objects for handling sensor services
	*/
	extern unsigned int sensorRegistered;
	extern int          sensorSender [ ENVIRONS_SENSOR_TYPE_MAX ];
    extern int          sensorsEventRate [ ENVIRONS_SENSOR_TYPE_MAX ];

    /** Place declarations to global namespace for plain C */
#ifdef __cplusplus
    namespace API
    {
#endif

#ifdef __cplusplus
		extern "C"
		{
#endif
			void DetermineSensorSupport ();

#ifdef ANDROID
            void    InitSensorMainThreaded ();
            void    DisposeSensorImpl ();
#else
#   define  InitSensorMainThreaded()
#   define  DisposeSensorImpl()
#endif
			/**
			* Determine whether the given sensorType is available.
			*
			* @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
			*
			* @return success true = enabled, false = failed.
			*/
			CLI_INC
				LIBEXPORT int                   IsSensorAvailableN ( int hInst, int ENVIRONS_SENSOR_TYPE_ );

			/**
			* Enable sending of sensor events to this DeviceInstance.
			* Events are send if the device is connected and stopped if the device is disconnected.
			*
			* @param nativeID 				Destination native id
			* @param objID                  Destination object id
			* @param ENVIRONS_SENSOR_TYPE_  A value of type ENVIRONS_SENSOR_TYPE_*.
			* @param enable 				true = enable, false = disable.
			*
			* @return success true = enabled, false = failed.
			*/
			CLI_INC
				LIBEXPORT EBOOL CallConv		SetSensorEventSenderN ( int hInst, int nativeID, int objID, int ENVIRONS_SENSOR_TYPE_, EBOOL enable );


            /**
             * Enable sending of sensor events to this DeviceInstance.
             * Events are send if the device is connected and stopped if the device is disconnected.
             *
             * @param hInst                 The Environs instance identifier.
             * @param nativeID 				Destination native device id
             * @param objID 				Destination object device id
             * @param flags            		A bitfield with values of type SensorType
             * @param enable 				true = enable, false = disable.
             *
             * @return success 1 = enabled, 0 = failed.
             */
			CLI_INC
				LIBEXPORT int CallConv          SetSensorEventSenderFlagsN ( int hInst, int nativeID, int objID, int flags, int enable );
            

			/**
			* Get registered DeviceInstance objects for sending of sensor events.
			*
			* @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
			*
			* @return success true = enabled, false = failed.
			*/
			CLI_INC
				LIBEXPORT int CallConv          GetSensorEventSenderCountN ( int hInst, int ENVIRONS_SENSOR_TYPE_ );

			CLI_INC
                LIBEXPORT void CallConv			PushSensorDataN ( int ENVIRONS_SENSOR_TYPE_, float x, float y, float z );

            CLI_INC
                LIBEXPORT void CallConv			PushSensorDataDoublesN ( int ENVIRONS_SENSOR_TYPE_, double x, double y, double z );
            
			CLI_INC
				LIBEXPORT void CallConv			PushSensorDataExtN ( int ENVIRONS_SENSOR_TYPE_, double x, double y, double z, float m, float n, float o );

            /**
             * Set use of Tcp transport channel of the given sensorType.
             *
             * @param hInst             The Environs instance identifier.
             * @param sensorType        A value of type environs::SensorType_t.
             * @param enable            true = TCP, false = UDP.
             *
             */
            CLI_INC
                LIBEXPORT void CallConv         SetUseSensorChannelTcpN ( int hInst, int sensorType, bool enable );


            /**
             * Get use of Tcp transport channel of the given sensorType.
             *
             * @param hInst             The Environs instance identifier.
             * @param sensorType        A value of type environs::SensorType_t.
             *
             * @return success          1 = TCP, 0 = UDP, -1 = error.
             */
            CLI_INC
                LIBEXPORT int CallConv          GetUseSensorChannelTcpN ( int hInst, int sensorType );

            /**
             * Set sample rate of the given sensorType in microseconds.
             *
             * @param hInst             The Environs instance identifier.
             * @param sensorType        A value of type environs::SensorType_t.
             * @param microseconds      The sensor sample rate in microseconds.
             *
             */
            CLI_INC
                LIBEXPORT void CallConv         SetUseSensorRateN ( int hInst, int sensorType, int microseconds );


            /**
             * Get sample rate of the given sensorType in microseconds.
             *
             * @param hInst             The Environs instance identifier.
             * @param sensorType        A value of type environs::SensorType_t.
             *
             * @return microseconds     The sensor sample rate in microseconds. -1 means error.
             */
            CLI_INC
                LIBEXPORT int CallConv          GetUseSensorRateN ( int hInst, int sensorType );

            
			/**
			* Stop all sensors that the given Environs instance have subscribed to.
			*
			* @param hInst 				The Environs instance identifier.
			*
			*/
			CLI_INC
				LIBEXPORT void CallConv			StopSensorListeningAllN ( int hInst );

			/**
			* Start all sensors that the given Environs instance have subscribed to.
			*
			* @param hInst 				The Environs instance identifier.
			*
			*/
			CLI_INC
				LIBEXPORT void CallConv			StartSensorListeningAllN ( int hInst );



            /**
             * Register to sensor events and listen to sensor data events.
             * This implementation is platform specific and needs to be implemented
             * in the particular platform layer.
             *
             * @param sensorType A value of type environs::SensorType.
             *
             */
            CLI_INC
                LIBEXPORT int CallConv			StartSensorListeningN ( int hInst, int sensorType );


            /**
             * Deregister to sensor events and stop listen to sensor data events.
             * This implementation is platform specific and needs to be implemented
             * in the particular platform layer.
             *
             * @param sensorType A value of type environs::SensorType.
             *
             */
            CLI_INC
                LIBEXPORT int CallConv			StopSensorListeningN ( int hInst, int sensorType );

#ifdef __cplusplus
		}
#endif

#ifndef CLI_CPP

		bool    EnvironsSensors_GlobalsInit ();
		bool    EnvironsSensors_GlobalsDispose ();
        
#ifdef __cplusplus
        void    DisposeSensorSender ( int hInst, DeviceBase * device );
        
        /**
         * Get registered DeviceInstance objects for sending of sensor events.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         * @return success true = enabled, false = failed.
         */
        int GetSensorEventSenderCount ( int hInst, environs::SensorType_t sensorType );
#endif
        
#endif
        
#ifdef __cplusplus
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailable ( int hInst, environs::SensorType_t sensorType, const char * sensorName );
        
        bool IsSensorAvailableImpl ( int hInst, environs::SensorType_t sensorType );
        
        
		/**
		* Register to sensor events and listen to sensor data events.
		* This implementation is platform specific and needs to be implemented
		* in the particular platform layer.
		*
		* @param sensorType A value of type environs::SensorType.
		*
		*/
        //void StartSensorListening ( int hInst, environs::SensorType_t sensorType );
        
        bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName );
        

		/**
		* Deregister to sensor events and stop listen to sensor data events.
		* This implementation is platform specific and needs to be implemented
		* in the particular platform layer.
		*
		* @param sensorType A value of type environs::SensorType.
		*
		*/
        //void StopSensorListening ( int hInst, environs::SensorType_t sensorType );
        
        void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType, const char * sensorName );


        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param hInst 				The Environs instance identifier.
         *
         */
        void StopSensorListeningAll ( int hInst );

        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param hInst 				The Environs instance identifier.
         *
         */
        void StartSensorListeningAll ( int hInst );
#endif


#ifdef __cplusplus
    } /** namespace API */
#endif
    
    
#ifdef __cplusplus
} /* namespace environs */
#endif


#endif




