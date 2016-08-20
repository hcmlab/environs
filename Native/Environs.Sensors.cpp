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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <vector>
using namespace std;

#include "Environs.Sensors.h"
#include "Environs.Obj.h"
#include "Device/Devices.h"
using namespace environs;


// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.Sensors . . . ."


namespace environs 
{
    
#define SENSOR_QUEUE_CAP_MAX        300
#define SENSOR_QUEUE_MAX            (SENSOR_QUEUE_CAP_MAX - 1)
    
    typedef struct SensorPack
    {
        int     packType;
        int     type;
        float   f1;
        float   f2;
        float   f3;
    }
    SensorPack;


    typedef struct SensorPackDoubles
    {
        int     packType;
        int     type;
        double  d1;
        double  d2;
        double  d3;
    }
    SensorPackDoubles;

    
    typedef struct SensorPackExt
    {
        int     packType;
        int     type;
        float   f1;
        float   f2;
        float   f3;
        double  d1;
        double  d2;
        double  d3;
    }
    SensorPackExt;


    static SensorPackExt   *        sensorQueue                 = 0;
	static bool						sensorLayerAllocated		= false;
    
    static  vector<DeviceBase *> **	sensorDevices				= 0;
    static  vector<int>  **         sensorInstances             = 0;
    
    int								sensorSender [ ENVIRONS_SENSOR_TYPE_MAX ];

    int                             sensorsEventRate [ SensorType::Max ];
    
    static int						queueNext					= 0;
    static int						queueEnd					= 0;
    static int						sensorFrameSeqNr			= 0;
    
    pthread_mutex_t                 sensorQueueMutex;
    
    pthread_mutex_t                 sensorsMutex;
    pthread_cond_t                  sensorSendEvent;
    pthread_t                       sensorSendThreadID;
    bool                            sensorThreadActive = false;

    static bool sensorChannelTcp [ ENVIRONS_SENSOR_TYPE_MAX ];
    
    /**
     * Objects for handling sensor services
     */
    unsigned int sensorRegistered       = 0;
    
    
	namespace API
	{
		void * SensorEventSender ( void * arg );

        
        bool InitSensors ()
        {
            if ( !sensorInstances ) {
                sensorInstances = new vector<int> * [ ENVIRONS_SENSOR_TYPE_MAX ];

                if ( !sensorInstances )
                    return false;

                memset ( sensorInstances, 0, sizeof(void *) * ENVIRONS_SENSOR_TYPE_MAX );
            }

            for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
            {
                if ( sensorInstances [ i ] )
                    continue;

                sensorInstances [ i ] = new vector<int> ();
                if ( !sensorInstances [ i ] )
                    return false;
            }

            if ( !sensorDevices ) {
                sensorDevices = new vector<DeviceBase *> * [ ENVIRONS_SENSOR_TYPE_MAX ];
                
                if ( !sensorDevices )
                    return false;
                
                memset ( sensorDevices, 0, sizeof(void *) * ENVIRONS_SENSOR_TYPE_MAX );
            }
            
            for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
            {
                if ( sensorDevices [ i ] )
                    continue;
                
                sensorDevices [ i ] = new vector<DeviceBase *> ();
                if ( !sensorDevices [ i ] )
                    return false;
            }
            
            return true;
        }
        
        
        void DisposeSensors ()
        {
            if ( sensorDevices ) {
                for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
                {
                    if ( sensorDevices [ i ] )
                        delete sensorDevices [ i ];
                }

                delete [] sensorDevices;
                sensorDevices = 0;
            }

            if ( sensorInstances ) {
                for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
                {
                    if ( sensorInstances [ i ] )
                        delete sensorInstances [ i ];
                }

                delete [] sensorInstances;
                sensorInstances = 0;
            }

            DisposeSensorImpl ();
        }
        
        
        bool EnvironsSensors_GlobalsInit ()
        {
            CVerb ( "EnvironsSensors_GlobalsInit" );
            
            Zero ( sensorSender );

			for ( size_t i = 0; i < ( sizeof ( sensorsEventRate ) / sizeof ( sensorsEventRate [ 0 ] ) ); ++i )
				sensorsEventRate [ i ] = 33000; // microseconds

			memcpy ( &sensorChannelTcp, &sensorChannelTcpDefault, sizeof ( sensorChannelTcp ) );
            
            if ( !InitSensors () )
                return false;
            
            Zero ( sensorSendThreadID );
            
            if ( !CondInit ( &sensorSendEvent ) )
                return false;
            
            if ( !LockInit ( &sensorsMutex ) )
                return false;
            
            if ( !LockInit ( &sensorQueueMutex ) )
                return false;
            
            queueNext   = 0;
            queueEnd    = 0;
            
            sensorLayerAllocated = true;
            
            return true;
        }
        

		bool EnvironsSensors_GlobalsStart ()
		{
			CVerb ( "EnvironsSensors_GlobalsStart" );
            
			if ( !sensorQueue )
			{
				sensorQueue = ( SensorPackExt * ) malloc ( sizeof ( SensorPackExt ) * SENSOR_QUEUE_CAP_MAX );

				if ( !sensorQueue ) {
					CErr ( "EnvironsSensors_GlobalsInit: Failed to alloc memory for sensor queue!" );
					return false;
				}
				queueNext   = 0;
				queueEnd    = 0;
            }
            
            if ( !InitSensors () )
                return false;

			if ( !sensorThreadActive ) {
				sensorThreadActive = true;

				int ret = pthread_create ( &sensorSendThreadID, NULL, &SensorEventSender, 0 );
				if ( ret != 0 ) {
					CErr ( "EnvironsSensors_GlobalsInit: Failed to create worker thread." );
					sensorThreadActive	= false;
				}
			}

			return true;
		}
        
        
		bool EnvironsSensors_GlobalsDispose ()
		{
			CVerb ( "EnvironsSensors_GlobalsDispose" );

			if ( !sensorLayerAllocated )
				return false;
			sensorLayerAllocated = false;
            
			if ( sensorThreadActive ) 
			{
				sensorThreadActive = false;

				LockAcquireV ( &sensorsMutex, "EnvironsSensors_GlobalsDispose" );

				if ( pthread_cond_signal ( &sensorSendEvent ) ) {
					CErr ( "EnvironsSensors_GlobalsDispose: Failed to signal sensorSendEvent!" );
				}

				LockReleaseV ( &sensorsMutex, "EnvironsSensors_GlobalsDispose" );

				pthread_t thrd = sensorSendThreadID;
				if ( pthread_valid ( thrd ) ) {
					pthread_reset ( sensorSendThreadID );

					CVerb ( "EnvironsSensors_GlobalsDispose: Waiting for async worker to be terminated..." );
#ifdef ANDROID
					pthread_kill ( thrd, SIGUSR1 );
#else
					pthread_join ( thrd, NULL );
#endif
					pthread_reset ( sensorSendThreadID );
				}
			}
            
            if ( sensorQueue ) {
                free ( sensorQueue );
                sensorQueue = 0;
            }
            
            DisposeSensors ();
            
            LockDispose ( &sensorQueueMutex );
            
            CondDispose ( &sensorSendEvent );
            
			LockDispose ( &sensorsMutex );

            return true;
        }
        
        
        void DisposeSensorSender ( int hInst, DeviceBase * device )
		{
			CVerb ( "DisposeSensorSender" );

            if ( !device )
                return;
            
            int objID       = 0;
            int nativeID    = device->nativeID;
            
            sp ( DeviceInstanceNode ) instanceNode = device->deviceNode;
            if ( instanceNode ) {
                objID = instanceNode->info.objID;
                instanceNode = 0;
            }
            
            FAKEJNI ();

            if ( objID || nativeID ) 
			{
				unsigned int flag = 1;

                for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
                {
                    SensorType_t sensorType = (SensorType_t) i;

                    if ( device->sensorSender & flag ) {
                        CVerbArg ( "DisposeSensorSender:\tType [ %i : %s ] Stopping ...", i, sensorFlagDescriptions [ i ] );

                        EnvironsCallArg ( SetSensorEventSenderN, hInst, nativeID, objID, sensorType, false );

                        EnvironsCallArg ( StopSensorListeningN, hInst, sensorType );
                    }

					flag <<= 1;
                }
            }
            
            device->sensorSender = 0;
        }
                
        
        /**
         * Get registered DeviceInstance objects for sending of sensor events.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        int GetSensorEventSenderCount ( int hInst, environs::SensorType_t sensorType )
		{
			CVerbVerbArg ( "GetSensorEventSenderCount:\tType [ %i ]", sensorType );

            FAKEJNI ();
            
            int devs = EnvironsCallArg ( GetSensorEventSenderCountN, hInst, sensorType );
            
            if ( sensorType >= 0 && sensorType < SensorType::Max )
                sensorSender [ sensorType ] = devs;
            
            return devs;
        }
        
        
        void RemoveDeviceFromReceivers ( DeviceBase * device )
        {
            for ( int i = 0; i < SensorType::Max; ++i )
            {
                if ( sensorDevices [ i ] )
                    continue;
                
                vector<DeviceBase *>	* devices = sensorDevices [ i ];                
                if ( !devices )
                    return;
                
                int devs = (int) devices->size ();
                
                for ( int j=0; j < devs; j++ )
                {
                    if ( (*devices) [j] == device )
                    {
                        devices->erase ( devices->begin() + j );
                        environs::UnlockDevice ( device );
                        break;
                    }
                }
            }
        }
        
        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param hInst 				The Environs instance identifier.
         * @param sensorType            A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailable ( int hInst, environs::SensorType_t sensorType, const char * sensorName )
        {
            CVerbArg ( "IsSensorAvailable: \tType [ %i : %s ]", sensorType, sensorName );

            if ( sensorType < 0 || sensorType >= SensorType::Max )
                return false;

            /*
            // return immediately if the environment hasn't been started or using sensor data has been disabled.
            sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;
            
            if ( !envSP || envSP->environsState < environs::Status::Started )
                return false;
            */
            
            return IsSensorAvailableImpl ( hInst, sensorType );
        }


		/**
		* Start all sensors that the given Environs instance have subscribed to.
		*
		* @param hInst 				The Environs instance identifier.
		*
		*/
		ENVIRONSAPI void EnvironsFunc ( StartSensorListeningAllN, jint hInst )
		{
			CVerbArg ( "StartSensorListeningAllN: hInst [ %i ]", hInst );

			StartSensorListeningAll ( hInst );
		}


		/**
		* Start all sensors that the given Environs instance have subscribed to.
		*
		* @param hInst 				The Environs instance identifier.
		*
		*/
        void StartSensorListeningAll ( int hInst )
        {
            CVerbArg ( "StartSensorListeningAll:  hInst [ %i ]", hInst );

            // return immediately if the environment hasn't been started or using sensor data has been disabled.
            sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;

            Instance * env = envSP.get ();

            if ( !env || env->environsState < environs::Status::Started || !env->sensorSubscribed )
                return;

            unsigned int flags = env->sensorSubscribed;
			if ( !flags )
				return;

            for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
            {
                SensorType_t sensorType = (SensorType_t) i;

				unsigned int flag = 1 << ( ( int ) sensorType ); // sensorFlags [ sensorType ];

                if ( flags & flag )
                {
                    flags &= ~flag;

                    int registered = (sensorRegistered & flag);
                    if ( !registered )
                    {
                        registered = StartSensorListeningImpl ( hInst, sensorType, sensorFlagDescriptions [ sensorType ] );
                        if ( registered )
                            sensorRegistered |= flag;
                    }
                }

                if ( !flags )
                    break;
            }
        }


		/**
		* Stop all sensors that the given Environs instance have subscribed to.
		*
		* @param hInst 				The Environs instance identifier.
		*
		*/
		ENVIRONSAPI void EnvironsFunc ( StopSensorListeningAllN, jint hInst )
		{
			CVerbArg ( "StopSensorListeningAllN: hInst [ %i ]", hInst );

			StopSensorListeningAll ( hInst );
        }


		/**
		* Stop all sensors that the given Environs instance have subscribed to.
		*
		* @param hInst 				The Environs instance identifier.
		*
		*/
        void StopSensorListeningAll ( int hInst )
        {
            CVerbArg ( "StopSensorListeningAll:  hInst [ %i ]", hInst );

            // return immediately if the environment hasn't been started or using sensor data has been disabled.
            sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;

            Instance * env = envSP.get ();
            if ( !env || !env->sensorSubscribed )
                return;

            unsigned int flags = env->sensorSubscribed;
			unsigned int flag = 1;

            for ( int i = 0; i < ENVIRONS_SENSOR_TYPE_MAX; ++i )
            {
                SensorType_t sensorType = (SensorType_t) i;

				//unsigned int flag = 1 << ( ( int ) sensorType ); // sensorFlags [ sensorType ];

                if ( sensorRegistered & flag )
                {
                    if ( flags & flag )
                    {
                        StopSensorListeningImpl ( hInst, sensorType, sensorFlagDescriptions [ i ] );

                        sensorRegistered &= ~flag;
                        flags &= ~flag;
                    }
                }

                if ( !flags )
                    break;
				flag <<= 1;
            }
        }


        /**
         * Enable sending of sensor events to this DeviceInstance.
         * Events are send if the device is connected and stopped if the device is disconnected.
         *
         * @param hInst                 The Environs instance identifier.
         * @param nativeID 				Destination native device id
         * @param objID 				Destination object device id
         * @param sensorType            A value of type environs.SensorType.
         * @param enable 				true = enable, false = disable.
         *
         * @return success true = enabled, false = failed.
         */
        bool SetSensorEventSender ( int hInst, int nativeID, int objID, environs::SensorType_t sensorType, bool enable )
        {
            CVerbArg2 ( "SetSensorEventSender", "nativeID", "i", nativeID, "type", "i", sensorType );

            FAKEJNI ();

            bool success;
            EnvironsCallArg ( SetSensorEventSenderN, hInst, nativeID, objID, ( int ) sensorType, enable );

            if ( enable ) {
                success = EnvironsCallArg ( StartSensorListeningN,  hInst, ( int ) sensorType ) != 0;
            }
            else {
                success = EnvironsCallArg ( StopSensorListeningN,  hInst, ( int ) sensorType ) != 0;
            }

            return success;
        }


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
        ENVIRONSAPI int EnvironsFunc ( SetSensorEventSenderFlagsN, int hInst, int nativeID, int objID, int flags, int enable )
        {
            int success = 1;

			unsigned int flag = 1;

            for ( int i = 0; i<SensorType::Max; ++i )
            {
				if ( ( flags & flag ) != 0 ) {// if ( (flags & sensorFlags [ i ]) != 0 ) {
                    if ( !SetSensorEventSender ( hInst, nativeID, objID, (environs::SensorType_t) i, enable != 0 ) )
                        success = 0;
                }
				flag <<= 1;
            }
            return success;
        }


        /**
         * Register to sensor events and listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success 1 = enabled, 0 = failed.
         */
        ENVIRONSAPI int EnvironsFunc ( StartSensorListeningN, int hInst, int sensorType )
        {
			if ( sensorType < 0 || sensorType >= SensorType::Max ) {
				CWarnArg ( "StartSensorListeningN:\t\tType [ %i ] invalid.", sensorType );
				return 0;
			}

			const char * sensorName = sensorFlagDescriptions [ sensorType ];

			CVerbArg ( "StartSensorListeningN:\t\tType [ %i : %s ]", sensorType, sensorName );

            // return immediately if the environment hasn't been started or using sensor data has been disabled.
            sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;

            if ( !envSP || envSP->environsState < environs::Status::Started )
                return 0;

			unsigned int flag = 1 << ( ( int ) sensorType ); // sensorFlags [ sensorType ];

            int registered = (sensorRegistered & flag);
			if ( registered ) {
				CVerbsArg ( 1, "StartSensorListeningN:\t\tType [ %i : %s ] already started.", sensorType, sensorName );
				return 0;
			}

            registered = StartSensorListeningImpl ( hInst, ( environs::SensorType_t ) sensorType, sensorName );

            if ( registered ) {
                sensorRegistered |= flag;
                return 1;
            }
            return 0;
        }
        

        /**
         * Deregister to sensor events and stop listen to sensor data events.
         * This implementation is platform specific and needs to be implemented
         * in the particular platform layer.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success 1 = enabled, 0 = failed.
         */
        ENVIRONSAPI int EnvironsFunc ( StopSensorListeningN, int hInst, int sensorType )
        {
            environs::SensorType_t type = ( environs::SensorType_t ) sensorType;

            if ( sensorType == -1 ) {
				CLog ( "StopSensorListeningN:\tType [ All ]" );
                sensorRegistered = 0;

                StopSensorListeningImpl ( hInst, type, "All" );
                return 1;
            }

			// Make sure that the sensorType value is valid
			if ( sensorType < 0 || sensorType >= SensorType::Max ) {
				CVerbArg ( "StopSensorListeningN:\tType [ %i ] invalid.", sensorType );
				return 0;
			}

			const char * sensorName = sensorFlagDescriptions [ sensorType ];

			CLogsArg ( 1, "StopSensorListeningN:\tType [ %i : %s ]", sensorType, sensorName );

			bool			disable = true;
			unsigned int	flag	= 1 << ( ( int ) sensorType ); // sensorFlags [ sensorType ];

            sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;

            Instance * env = envSP.get ();
            if ( env ) {
                CVerbArg ( "StopSensorListeningN:\tType [ %i ] Environs subscribed [ %i ].", sensorType, env->sensorSubscribed );

				// If the sensorType is subscribed by the calling Environs instance, then do not disable the sensor
                if ( env->sensorSubscribed & flag ) {
                    CVerbArg ( "StopSensorListeningN:\tType [ %i ] Environs subscribed sensor.", sensorType );
					disable = false;
				}
			}

            int devs = GetSensorEventSenderCount ( hInst, type );
            if ( devs > 0 ) {
				// If there are DeviceControllers that have subscribed to this sensorType, then do not disable the sensor
				disable = false;
            }

            int registered = (sensorRegistered & flag);
			if ( !registered ) {
				// If the sensorType has not been registered / started, then dont do anything
				CVerbsArg ( 1, "StopSensorListeningN:\tType [ %i : %s ] already stopped.", sensorType, sensorName );
				return 0;
			}

			if ( disable ) {
				// Mark the sensorType as stopped
				sensorRegistered &= ~flag;

				CLogsArg ( 1, "StopSensorListeningN:\tType [ %i : %s ] stopping ...", sensorType, sensorName );
				StopSensorListeningImpl ( hInst, type, sensorName );
                return 1;
			}
            return 0;
        }
    
    
        /**
         * Determine whether the given sensorType is available.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        ENVIRONSAPI int EnvironsFunc ( IsSensorAvailableN, jint hInst, jint sensorType )
        {
			const char * sensorName = ( ( sensorType >= 0 && sensorType < SensorType::Max ) ? sensorFlagDescriptions [ sensorType ] : "Invalid" );

			int avail = ( IsSensorAvailable ( hInst, ( environs::SensorType_t ) sensorType, sensorName ) ? 1 : 0 );

            CVerbArg ( "IsSensorAvailableN:\tType [ %i : %s : %i ]", sensorType, sensorName, avail );
            
			return avail;
        }


        /**
         * Determine whether the given sensorType is enabled.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success 1 = enabled, 0 = failed, -1 = error.
         */
        ENVIRONSAPI int EnvironsFunc ( IsSensorEnabledN, jint hInst, jint sensorType )
        {
            if ( sensorType < 0 || sensorType >= SensorType::Max )
                return -1;

            CVerbArg ( "IsSensorEnabledN:\t\tType [ %i : %s : %i ]", sensorType, 
				( ( sensorType >= 0 && sensorType < SensorType::Max ) ? sensorFlagDescriptions [ sensorType ] : "Invalid" ),
				( ( sensorRegistered & ( 1 << ( ( int ) sensorType ) ) ) ? 1 : 0 ) );

            return ( ( sensorRegistered & ( 1 << ( ( int ) sensorType ) ) ) ? 1 : 0);
            //return ( ( sensorRegistered & sensorFlags [ sensorType ] ) ? 1 : 0);
        }
        
        
        /**
         * Enable sending of sensor events to this DeviceInstance.
         * Events are send if the device is connected and stopped if the device is disconnected.
         *
         * @param hInst         The Environs instance identifier.
         * @param nativeID      Destination native id
         * @param objID         Destination object id
         * @param sensorType    A value of type environs::SensorType_t.
         * @param enable        true = enable, false = disable.
         *
         * @return success true = enabled, false = failed.
         */
        ENVIRONSAPI EBOOL EnvironsFunc ( SetSensorEventSenderN, jint hInst, jint nativeID, jint objID, jint sensorType, EBOOL enable )
		{
			CVerbArg ( "SetSensorEventSenderN:\t\tType [ %i ], nativeID [ %i ], objID [ %i ], enable [ %d ]", sensorType, nativeID, objID, enable );

			if ( !LockAcquire ( &sensorsMutex, "SetSensorEventSenderN" ) )
				return false;

			bool                        success     = false;
			DeviceBase *                device      = 0;
			vector<DeviceBase *>    *   srcDevices  = 0;
			vector<int>             *   srcInstances  = 0;
            
			int devs = 0, found = -1; // sensorFlag = 0;
			unsigned int sensorFlag;
            
            if ( sensorType < 0 || sensorType >= SensorType::Max )
                goto Finish;

			sensorFlag = 1 << ( ( int ) sensorType ); // sensorFlags [ sensorType ];

            if ( !nativeID && !objID )
            {
                CVerbArg ( "SetSensorEventSenderN:\t\tType [ %i : %s ] Try %s Environs subscribed ...", sensorType, sensorFlagDescriptions [ sensorType ], enable ? "enabling" : "disabling" );

                if ( hInst <= 0 || hInst >= ENVIRONS_MAX_ENVIRONS_INSTANCES )
                    goto Finish;

                srcInstances = sensorInstances [ sensorType ];
                if ( !srcInstances )
                    goto Finish;

                sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;
                
                Instance * inst = envSP.get();
                if ( !inst )
                    goto Finish;

                if ( enable ) {
                    success = true;

                    inst->sensorSubscribed |= sensorFlag;

                    bool add = true;

                    size_t size = srcInstances->size ();
                    if ( size > 0 )
                    {
                        for ( size_t i = 0; i < size; i++ )
                        {
                            if ( ( *srcInstances ) [ i ] == hInst ) {
                                add = false;
                                break;
                            }
                        }
                    }

                    if ( add ) {
                        srcInstances->push_back ( hInst );
                    }
                }
                else { // disable
                    inst->sensorSubscribed &= ~sensorFlag;

                    size_t size = srcInstances->size ();
                    if ( size > 0 )
                    {
                        for ( size_t i = 0; i < size; i++ )
                        {
                            if ( ( *srcInstances ) [ i ] == hInst ) {
                                success = true;
                                srcInstances->erase ( srcInstances->begin () + i );
                                break;
                            }
                        }
                    }
                }
            }
            else {
                srcDevices = sensorDevices [ sensorType ];
                if ( !srcDevices )
                    goto Finish;

                CVerbArg ( "SetSensorEventSenderN:\t\tType [ %i : %s ] Try %s Device subscribed ...", sensorType, sensorFlagDescriptions [ sensorType ], enable ? "enabling" : "disabling" );

				devs = ( int ) srcDevices->size ();

				for ( int i=0; i < devs; i++ ) {
					DeviceBase * dev = ( *srcDevices ) [ i ];

					if ( dev ) {
						// Check whether the device is a zombie without being in the device management anymore
						sp ( DeviceInstanceNode ) node = dev->deviceNode;
						if ( !node ) {
							RemoveDeviceFromReceivers ( dev );

							i--;
							if ( devs > 0 )
								devs--;
							continue;
						}

						if ( node->info.objID == objID ) {
							device = dev;
							found = i;
							break;
						}
					}
				}

                if ( enable ) {
                    if ( device ) {
                        success = true;
                        goto Finish;
                    }

                    device = environs::GetDevice ( nativeID );
                    if ( device ) {
                        device->sensorSender |= sensorFlag;

                        srcDevices->push_back ( device );
                        success = true;
                    }
                }
                else { // disable
                    if ( !device ) {
                        goto Finish;
                    }

                    device->sensorSender &= ~sensorFlag;

                    environs::UnlockDevice ( device );

                    if ( found >= 0 )
                        srcDevices->erase ( srcDevices->begin() + found );
                    
                    success = true;
                }
            }
            
        Finish:
            if ( !LockRelease ( &sensorsMutex, "SetSensorEventSenderN" ) )
				return false;
            
			if ( success && enable ) {
				if ( !EnvironsSensors_GlobalsStart () )
					return false;
			}
            return success;
        }
        
        
        /**
         * Get registered DeviceInstance objects for sending of sensor events.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success true = enabled, false = failed.
         */
        ENVIRONSAPI int EnvironsFunc ( GetSensorEventSenderCountN, jint hInst, jint sensorType )
        {
			if ( !LockAcquire ( &sensorsMutex, "GetSensorEventSenderCountN" ) )
				return 0;
            
            int devs = 0;

			if ( sensorType >= 0 && sensorType < SensorType::Max )
				devs = ( int ) sensorDevices [ sensorType ]->size ();
            
			if ( !LockRelease ( &sensorsMutex, "GetSensorEventSenderCountN" ) )
				return 0;
            
            return devs;
        }


        /**
         * Set use of Tcp transport channel of the given sensorType.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         * @param enable        true = TCP, false = UDP.
         *
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseSensorChannelTcpN, jint hInst, jint sensorType, jboolean enable )
        {
            if ( sensorType >= 0 && sensorType < SensorType::Max ) {
                sensorChannelTcp [ sensorType ] = enable;
            }
        }


        /**
         * Get use of Tcp transport channel of the given sensorType.
         *
         * @param hInst         The Environs instance identifier.
         * @param sensorType    A value of type environs::SensorType_t.
         *
         * @return success      1 = TCP, 0 = UDP, -1 = error.
         */
        ENVIRONSAPI jint EnvironsFunc ( GetUseSensorChannelTcpN, jint hInst, jint sensorType )
        {
            if ( sensorType >= 0 && sensorType < SensorType::Max ) {
                return ( sensorChannelTcp [ sensorType ] ? 1 : 0 );
            }
            return -1;
        }


        /**
         * Set sample rate of the given sensorType in microseconds.
         *
         * @param hInst             The Environs instance identifier.
         * @param sensorType        A value of type environs::SensorType_t.
         * @param microseconds      The sensor sample rate in microseconds.
         *
         */
        ENVIRONSAPI void EnvironsFunc ( SetUseSensorRateN, jint hInst, jint sensorType, jint microseconds )
        {
            if ( sensorType >= 0 && sensorType < SensorType::Max ) {
                sensorsEventRate [ sensorType ] = microseconds;
            }
        }


        /**
         * Get sample rate of the given sensorType in microseconds.
         *
         * @param hInst             The Environs instance identifier.
         * @param sensorType        A value of type environs::SensorType_t.
         *
         * @return microseconds     The sensor sample rate in microseconds. -1 means error.
         */
        ENVIRONSAPI jint EnvironsFunc ( GetUseSensorRateN, jint hInst, jint sensorType )
        {
            if ( sensorType >= 0 && sensorType < SensorType::Max ) {
                return sensorsEventRate [ sensorType ];
            }
            return -1;
        }


        ENVIRONSAPI void EnvironsFunc ( PushSensorDataN, jint sensorType, jfloat x, jfloat y, jfloat z )
        {
            if ( !native.coresStarted )
                return;
            
			CVerbVerbArg ( "PushSensorDataN:\tType [ %i ], x [ %.2f ], y [ %.2f ], z [ %.2f ]", sensorType, x, y, z );
            
            if ( pthread_mutex_lock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataN: Failed to lock mutex on sensorQueueMutex" );
                return;
            }

			if ( queueEnd == queueNext - 1 || ( queueNext == 0 && queueEnd == SENSOR_QUEUE_MAX ) ) {
				CWarns ( 1, "PushSensorDataN: Sensor queue is full." );
			}
			else {
				SensorPack * pack = ( SensorPack * ) ( sensorQueue + queueEnd );

				pack->packType = 0;
				pack->type	= sensorType;
				pack->f1	= x;
				pack->f2	= y;
				pack->f3	= z;

				queueEnd++;
				if ( queueEnd >= SENSOR_QUEUE_CAP_MAX )
					queueEnd = 0;
			}
            
            if ( pthread_mutex_unlock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataN: Failed to release mutex on sensorQueueMutex" );
            }
            
            ///
            /// Signal send thread to work on the queue
            ///
            if ( pthread_mutex_lock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataN: Failed to lock mutex on sensorsMutex" );
            }
            
            if ( pthread_cond_signal ( &sensorSendEvent ) ) {
                CErr ( "PushSensorDataN: Failed to signal sensorSendEvent!" );
            }
            
            if ( pthread_mutex_unlock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataN: Failed to release mutex on sensorsMutex" );
            }
        }



        ENVIRONSAPI void EnvironsFunc ( PushSensorDataDoublesN, jint sensorType, jdouble x, jdouble y, jdouble z )
        {
            if ( !native.coresStarted )
                return;

            CVerbVerbArg ( "PushSensorDataDoublesN:\tType [ %i ], x [ %.2f ], y [ %.2f ], z [ %.2f ]", sensorType, x, y, z );

            if ( pthread_mutex_lock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataDoublesN: Failed to lock mutex on sensorQueueMutex" );
                return;
            }

            if ( queueEnd == queueNext - 1 || ( queueNext == 0 && queueEnd == SENSOR_QUEUE_MAX ) ) {
				CWarns ( 1, "PushSensorDataDoublesN: Sensor queue is full." );
            }
            else {
                SensorPackDoubles * pack = ( SensorPackDoubles * ) ( sensorQueue + queueEnd );

                pack->packType = 1;
                pack->type = sensorType;
                pack->d1 = x;
                pack->d2 = y;
                pack->d3 = z;

                queueEnd++;
                if ( queueEnd >= SENSOR_QUEUE_CAP_MAX )
                    queueEnd = 0;
            }

            if ( pthread_mutex_unlock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataDoublesN: Failed to release mutex on sensorQueueMutex" );
            }

            ///
            /// Signal send thread to work on the queue
            ///
            if ( pthread_mutex_lock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataDoublesN: Failed to lock mutex on sensorsMutex" );
            }

            if ( pthread_cond_signal ( &sensorSendEvent ) ) {
                CErr ( "PushSensorDataDoublesN: Failed to signal sensorSendEvent!" );
            }

            if ( pthread_mutex_unlock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataDoublesN: Failed to release mutex on sensorsMutex" );
            }
        }
        
        
        
        ENVIRONSAPI void EnvironsFunc ( PushSensorDataExtN, jint sensorType, jdouble x, jdouble y, jdouble z, jfloat m, jfloat n, jfloat o )
        {
            if ( !native.coresStarted )
                return;
            
            CVerbVerbArg ( "PushSensorDataExtN:\tType [ %i ], x [ %.2f ], y [ %.2f ], z [ %.2f ]", sensorType, x, y, z );
            
            if ( pthread_mutex_lock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataN: Failed to lock mutex on sensorQueueMutex" );
                return;
            }
            
            if ( queueEnd == queueNext - 1 || (queueNext == 0 && queueEnd == SENSOR_QUEUE_MAX) ) {
				CWarns ( 1, "PushSensorDataN: Sensor queue is full." );
            }
            else {
                SensorPackExt * pack = sensorQueue + queueEnd;
                
                pack->packType = 2;
                pack->type = sensorType;
                pack->d1 = x;
                pack->d2 = y;
                pack->d3 = z;
                
                pack->f1 = m;
                pack->f2 = n;
                pack->f3 = o;
                
                queueEnd++;
                if ( queueEnd >= SENSOR_QUEUE_CAP_MAX )
                    queueEnd = 0;
            }
            
            if ( pthread_mutex_unlock ( &sensorQueueMutex ) ) {
                CErr ( "PushSensorDataN: Failed to release mutex on sensorQueueMutex" );
            }
            
            ///
            /// Signal send thread to work on the queue
            ///
            if ( pthread_mutex_lock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataN: Failed to lock mutex on sensorsMutex" );
            }
            
            if ( pthread_cond_signal ( &sensorSendEvent ) ) {
                CErr ( "PushSensorDataN: Failed to signal sensorSendEvent!" );
            }
            
            if ( pthread_mutex_unlock ( &sensorsMutex ) ) {
                CErr ( "PushSensorDataN: Failed to release mutex on sensorsMutex" );
            }
        }
    
    
        void * SensorEventSender ( void * arg )
        {
            CVerb ( "SensorEventSender thread started..." );
            
            pthread_setname_current_envthread ( "SensorEventSender" );
            
            bool dequeued = false;
            vector<DeviceBase *>	*   srcDevices      = 0;
            vector<int>             *   srcInstances    = 0;
            
			environs::lib::SensorFrameExt frameExt;

			ZeroStruct ( frameExt, environs::lib::SensorFrameExt );

			frameExt.preamble [ 0 ] = 's';
			frameExt.preamble [ 1 ] = 'f';
			frameExt.preamble [ 2 ] = ':';
            frameExt.version = UDP_MSG_PROTOCOL_VERSION;
            
            while ( sensorThreadActive )
            {
                pthread_wait_one ( sensorSendEvent, sensorsMutex );
                
                //
                // Check whether there is something in the queue
                //
            DequeueNext:
                if ( !sensorThreadActive ) {
                    break;
                }
                
                dequeued = false;
                int packType = 0;
                
				if ( !LockAcquire ( &sensorQueueMutex, "SensorEventSender" ) )
                    break;

                int frameType = 0;
                int frameSize = 0;
                
                if ( queueEnd != queueNext )
                {
                    SensorPackExt * packExt = sensorQueue + queueNext;

					packType				= packExt->packType;
                    frameType               = packExt->type;

                    frameExt.data.floats.f1 = packExt->f1;
                    frameExt.data.floats.f2 = packExt->f2;
                    frameExt.data.floats.f3 = packExt->f3;

                    if ( packType == 1 ) {
                        frameSize       = sizeof ( environs::lib::SensorFrameDoubles );

                        frameExt.type   = (frameType | ENVIRONS_SENSOR_PACK_TYPE_DOUBLES);

                        environs::lib::SensorFrameDoubles * frameDoubles = (environs::lib::SensorFrameDoubles *) &frameExt;

                        SensorPackDoubles * packDoubles = (SensorPackDoubles *) packExt;

                        frameDoubles->data.doubles.d1 = packDoubles->d1;
                        frameDoubles->data.doubles.d2 = packDoubles->d2;
                        frameDoubles->data.doubles.d3 = packDoubles->d3;

                        CVerbsArg ( 8, "SensorEventSender:\tType [ %i : %s ]\td1 [ %lf ] d2 [ %lf ] d3 [ %lf ]", frameExt.type & 0x3FFFFFFF, sensorFlagDescriptions [ frameExt.type ], packDoubles->d1, packDoubles->d2, packDoubles->d3 );
                    }
                    else if ( packType == 2 )
                    {                        
                        frameExt.type          = (frameType | ENVIRONS_SENSOR_PACK_TYPE_EXT);
                        frameSize              = sizeof ( environs::lib::SensorFrameExt );
                        
                        frameExt.doubles.d1 = packExt->d1;
                        frameExt.doubles.d2 = packExt->d2;
                        frameExt.doubles.d3 = packExt->d3;
                        
						CVerbsArg ( 8, "SensorEventSender:\tType [ %i : %s ]\td1 [ %lf ] d2 [ %lf ] d3 [ %lf ] f1 [ %lf ] f2 [ %lf ] f3 [ %lf ]", frameExt.type & 0x3FFFFFFF, sensorFlagDescriptions [ frameExt.type ], packExt->d1, packExt->d2, packExt->d3, packExt->f1, packExt->f2, packExt->f3 );
                    }
					else {
						frameSize = sizeof ( environs::lib::SensorFrame );

						frameExt.type = frameType;

						CVerbsArg ( 8, "SensorEventSender:\tType [ %i : %s ]\tf1 [ %lf ] f2 [ %lf ] f3 [ %lf ]", frameExt.type, sensorFlagDescriptions [ frameExt.type ], packExt->f1, packExt->f2, packExt->f3 );
					}
                    
                    queueNext++;
                    if ( queueNext >= SENSOR_QUEUE_CAP_MAX )
                        queueNext = 0;
                    dequeued = true;
                }
                
				LockReleaseV ( &sensorQueueMutex, "SensorEventSender" );

                
                if ( !dequeued ) {
                    CVerbVerbArg ( "SensorEventSender: Queue is empty [ %i / %i ].", queueNext, queueEnd );
                    continue;
                }

                if ( frameType >= 0 && frameType < SensorType::Max )
                {
                    bool incSeqNr = true;

                    srcInstances = sensorInstances [ frameType ];

                    if ( srcInstances && srcInstances->size () )
                    {
#ifdef ANDROID
                        // Android GPS Location dispatching is handled by platform layer, so we just skip it here.
                        if ( frameType != SensorType::Location )
                        {
#endif
                            frameExt.seqNumber = sensorFrameSeqNr++;
                            if (sensorFrameSeqNr < 0)
                                sensorFrameSeqNr = 0;
                            incSeqNr = false;

                            if ( !LockAcquire ( &sensorsMutex, "SensorEventSender" ) )
                                break;

                            int devs = (int) srcInstances->size ();

                            for ( int i=0; i < devs; i++ )
                            {
                                int hInst = ( *srcInstances ) [ i ];

                                if ( hInst > 0 && hInst < ENVIRONS_MAX_ENVIRONS_INSTANCES )
                                {
                                    sp ( Instance ) envSP = native.instancesSP [ hInst ] MED_WP;

                                    Instance * inst = envSP.get();
                                    if ( !inst )
                                        continue;

                                    onEnvironsSensor ( inst, 0, ( environs::lib::SensorFrame * ) &frameExt, frameSize );
                                }
                            }
                            
                            if ( !LockRelease ( &sensorsMutex, "SensorEventSender" ) )
                                break;
#ifdef ANDROID
                        }
#endif
                    }

                    srcDevices = sensorDevices [ frameType ];

                    if ( srcDevices && srcDevices->size () )
                    {
                        if ( incSeqNr ) {
                            frameExt.seqNumber = sensorFrameSeqNr++;
                            if (sensorFrameSeqNr < 0)
                                sensorFrameSeqNr = 0;
                        }

                        if ( !LockAcquire ( &sensorsMutex, "SensorEventSender" ) )
                            break;

                        int devs = (int) srcDevices->size();

                        if ( sensorChannelTcp [ frameType ] )
                        {
                            for ( int i=0; i < devs; i++ )
                            {
                                DeviceBase * device = (*srcDevices) [i];
                                if ( device ) {
                                    if ( device->deviceStatus < DeviceStatus::Connected )
                                    {
                                        RemoveDeviceFromReceivers ( device );

                                        i--;
                                        if ( devs > 0 )
                                            devs--;
                                        continue;
                                    }
                                    // Send to client
                                    device->SendTcpBuffer ( true, MSG_TYPE_SENSOR, 0, &frameExt, frameSize );
                                    //DeviceBase::SendTcpBuffer ( device->nativeID, true, MSG_TYPE_SENSOR, 0, &frameExt, frameSize );      
                                }
                            }
                        }
                        else
                        {
                            for ( int i=0; i < devs; i++ )
                            {
                                DeviceBase * device = (*srcDevices) [i];
                                if ( device )
                                {
                                    if ( device->deviceStatus < DeviceStatus::Connected )
                                    {
                                        RemoveDeviceFromReceivers ( device );

                                        i--;
                                        if ( devs > 0 )
                                            devs--;
                                        continue;
                                    }
                                    // Send to client
                                    device->SendDataPacket ( ( const char * ) &frameExt, frameSize );
                                }
                            }
                        }
                        
                        if ( !LockRelease ( &sensorsMutex, "SensorEventSender" ) )
                            break;
                    }
                }
                
                if ( dequeued )
					goto DequeueNext;
            }
            
            CLog ( "SensorEventSender: Done ..." );
            return 0;
        }
	}

} /* namespace environs */




