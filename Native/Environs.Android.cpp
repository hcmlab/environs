/**
 * Android JNI platform specific
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

#ifdef ANDROID

#define ATTACH_JAVA_THREAD

#define USE_NATIVE_SENSOR_READER

#define EnvASENSOR_TYPE_ORIENTATION				3
#define EnvASENSOR_TYPE_PRESSURE				6
#define EnvASENSOR_TYPE_TEMPERATURE				7
#define EnvASENSOR_TYPE_GRAVITY					9
#define EnvASENSOR_TYPE_ACCELEROMETER_LINEAR	10
#define EnvASENSOR_TYPE_ROTATION_VECTOR			11
#define EnvASENSOR_TYPE_HUMIDITY				12
#define EnvASENSOR_TYPE_HEART_RATE				21

#include <stdlib.h>
#include "Environs.Mobile.h"
#include "Environs.Lib.h"
#include "Environs.Obj.h"
#include "Environs.Crypt.h"
#include "Environs.Sensors.h"

#ifdef USE_NATIVE_SENSOR_READER
#   include <android/sensor.h>
#   include <android/looper.h>
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Environs.JNI . . . . . ."

using namespace environs::API;


namespace environs 
{
	JavaVM *				g_JavaVM 				= 0;
	jclass					g_EnvironsClass			= 0;
    jmethodID 				g_BridgeForNotifyID 	= 0;
    jmethodID 				g_BridgeForNotifyExtID 	= 0;
    jmethodID 				g_BridgeForMessageID 	= 0;
    jmethodID 				g_BridgeForMessageExtID 	= 0;
	jmethodID 				g_BridgeForStatusMessageID = 0;
	jmethodID 				g_BridgeForDataID 		= 0;
	jmethodID 				g_BridgeForUdpDataID    = 0;
	jmethodID 				g_BridgeForOptSaveID 	= 0;
	jmethodID 				g_BridgeForOptLoadID 	= 0;
	jmethodID 				g_BridgeForUpdateNetworkStatus = 0;
	jmethodID 				g_SHAHashCreate 		= 0;
	jmethodID 				g_EncryptMessage		= 0;
	jmethodID 				g_DecryptMessage		= 0;
	jmethodID 				g_ReleaseCert			= 0;
	jmethodID 				g_AESDeriveKeyContext	= 0;
	jmethodID 				g_AESDisposeKeyContext	= 0;
	jmethodID 				g_AESUpdateKeyContext	= 0;
	jmethodID 				g_AESTransform			= 0;
	jmethodID 				g_GenerateCertificate	= 0;

	jmethodID 				g_java_mallocID 		= 0;
	jmethodID 				g_java_freeID 			= 0;

	jmethodID 				g_CreateInstance		= 0;
	jmethodID 				g_Encoder_Init			= 0;
	jmethodID 				g_DestroyInstance		= 0;

    // Initialize available to GPS. GPS will be handled by platform layer and not subscribed if not available
    unsigned int			sensorsAvailable		= ENVIRONS_SENSOR_FLAG_LOCATION;

#ifdef USE_NATIVE_SENSOR_READER
    ALooper             *   sensorLooper             = 0;
        
    ASensorManager      *   sensorManager           = 0;
    ASensorEventQueue   *   sensorEventQueue        = 0;

    const ASensor       *	sensorsAndroid [ SensorType::Max ] = { 0 };

    int                     sensorsEventRate [ SensorType::Max ];

	const int getEnvSensorTypeByAndroidType [ ] = {
		-1,
		ENVIRONS_SENSOR_TYPE_ACCELEROMETER,                 // TYPE_ACCELEROMETER = 1;
		ENVIRONS_SENSOR_TYPE_MAGNETICFIELD,                 // TYPE_MAGNETIC_FIELD = 2;
		ENVIRONS_SENSOR_TYPE_ORIENTATION,                   // TYPE_ORIENTATION = 3;	// Deprecated
		ENVIRONS_SENSOR_TYPE_GYROSCOPE,                     // TYPE_GYROSCOPE = 4;
		ENVIRONS_SENSOR_TYPE_LIGHT,                         // TYPE_LIGHT = 5;
		ENVIRONS_SENSOR_TYPE_ALTIMETER,                     // TYPE_PRESSURE = 6;
		ENVIRONS_SENSOR_TYPE_TEMPERATURE,                   // TYPE_TEMPERATURE = 7;	 // Deprecated
		ENVIRONS_SENSOR_TYPE_PROXIMITY,                     // TYPE_PROXIMITY = 8;
		ENVIRONS_SENSOR_TYPE_MOTION_GRAVITY_ACCELERATION,	// TYPE_GRAVITY = 9;
		ENVIRONS_SENSOR_TYPE_MOTION_GRAVITY_ACCELERATION,	// TYPE_LINEAR_ACCELERATION = 10;
		ENVIRONS_SENSOR_TYPE_MOTION_ATTITUTDE_ROTATION,		// TYPE_ROTATION_VECTOR = 11;
		ENVIRONS_SENSOR_TYPE_HUMIDITY,                      // TYPE_RELATIVE_HUMIDITY = 12;
		ENVIRONS_SENSOR_TYPE_TEMPERATURE,                   // TYPE_AMBIENT_TEMPERATURE = 13;
		-1,                                                 // TYPE_MAGNETIC_FIELD_UNCALIBRATED = 14;
		-1,                                                 // TYPE_GAME_ROTATION_VECTOR = 15;
		-1,                                                 // TYPE_GYROSCOPE_UNCALIBRATED = 16;
		-1,                                                 // TYPE_SIGNIFICANT_MOTION = 17;
		-1,                                                 // TYPE_STEP_DETECTOR = 18;
		-1,                                                 // TYPE_STEP_COUNTER = 19;
		-1,                                                 // TYPE_GEOMAGNETIC_ROTATION_VECTOR = 20;
		ENVIRONS_SENSOR_TYPE_HEARTRATE,                     // TYPE_HEART_RATE = 21;
	};

#define envSensorTypeByAndroidTypeSize		( sizeof(getEnvSensorTypeByAndroidType) / sizeof(getEnvSensorTypeByAndroidType[0]) )


	const int getAndroidSensorType [ ] = {
		ASENSOR_TYPE_ACCELEROMETER,
		ASENSOR_TYPE_MAGNETIC_FIELD,        //ENVIRONS_SENSOR_FLAG_MAGNETICFIELD,
		ASENSOR_TYPE_GYROSCOPE,             //ENVIRONS_SENSOR_FLAG_GYROSCOPE,
		EnvASENSOR_TYPE_ORIENTATION,        //ENVIRONS_SENSOR_FLAG_ORIENTATION,
		ASENSOR_TYPE_LIGHT,                 //ENVIRONS_SENSOR_FLAG_LIGHT,
		-1,                                 //ENVIRONS_SENSOR_FLAG_LOCATION,
		-1,                                 //ENVIRONS_SENSOR_FLAG_HEADING,
		EnvASENSOR_TYPE_PRESSURE,			//ENVIRONS_SENSOR_FLAG_ALTIMETER,
		EnvASENSOR_TYPE_ROTATION_VECTOR,	//ENVIRONS_SENSOR_FLAG_MOTION_ATTITUTDE_ROTATION,
		EnvASENSOR_TYPE_GRAVITY,			//ENVIRONS_SENSOR_FLAG_MOTION_GRAVITY_ACCELERATION,
		EnvASENSOR_TYPE_GRAVITY,			//ENVIRONS_SENSOR_FLAG_MOTION_MAGNETICFIELD,
		EnvASENSOR_TYPE_HEART_RATE,         //ENVIRONS_SENSOR_FLAG_HEARTRATE,
		ASENSOR_TYPE_PROXIMITY,             //ENVIRONS_SENSOR_FLAG_PROXIMITY,
		-1,                                 //ENVIRONS_SENSOR_FLAG_VOC,
		-1,                                 //ENVIRONS_SENSOR_FLAG_CO2,
		EnvASENSOR_TYPE_HUMIDITY,           //ENVIRONS_SENSOR_FLAG_HUMIDITY,
		EnvASENSOR_TYPE_TEMPERATURE,		//ENVIRONS_SENSOR_FLAG_TEMPERATURE,
		-1,                                 //ENVIRONS_SENSOR_FLAG_CUSTOM,
		-1,                                 //MAX_ENVIRONS_SENSOR_TYPE_VALUE,
	};

#define androidSensorTypeSize		( sizeof(getAndroidSensorType) / sizeof(getAndroidSensorType[0]))

#endif

#ifdef NDEBUG
    const char * sensorFlagDescriptions [ ] = {
        "Accelerometer",
        "MagneticField",
        "Gyroscope",
        "Orientation",
        "Light",
        "Location",
        "Heading",
        "Altimeter/Pressure",
        "MotionAttitudeRotation",
        "MotionGravityAcceleration",
        "MotionMagneticField",
        "Heartrate",
        "Proximity",
        "VOC",
        "CO2",
        "Humidity",
        "Temperature",
        "Custom",
        "Invalid",
    };
#endif


    bool AllocNativePlatform ()
	{
        CVerb ( "AllocNativePlatform" );

        for ( size_t i = 0; i < ( sizeof ( sensorsEventRate ) / sizeof ( sensorsEventRate [0] ) ); ++i )
            sensorsEventRate [ i ] = 33000; // microseconds

        return true;
    }
        
    
    /*
     * Method:    AllocPlatform
     * Signature: ()Z
     */
    ENVIRONSAPI jboolean EnvironsProc ( AllocPlatformN )
    {
        CVerb ( "AllocPlatformN" );
        
        return AllocNativePlatformMobile ();
    }
    
    
    /**
     * Creates an application identifier by means of a UUID
     *
     * @param	buffer	The UUID will be stored in this buffer.
     * @param	bufSize	The size of the buffer. Must be at least 180 bytes.
     * @return	success
     */
    bool CreateAppID ( char * buffer, unsigned int bufSize )
    {
        return false;
    }
    
    
	/**
	* Perform SDK checks to detect ...
	*
	*/
	void DetectSDKs ( )
	{
	}


	/**
	* Perform platform checks to detect ...
	*
	*/
	void DetectPlatform ( )
    {
        CVerbVerb ( "DetectPlatform" );
        
        native.platform = Platforms::Tablet_Flag;
	}

    /**
     * This is done by the android platform layer ...
     *
     */
    bool DetermineAndInitWorkDir ()
    {
        return true;
    }
    
    
    bool CreateInstancePlatform ( Instance * env )
    {
        SetCallbacksN ( env->hEnvirons, 0, ( void * ) BridgeForUdpData, (void *)BridgeForMessage, (void *)BridgeForMessageExt, (void *)BridgeForNotify, (void *)BridgeForNotifyExt, (void *)BridgeForData, (void *)BridgeForStatusMessage );
        return true;
    }
    
	int AttachJavaThread ( JNIEnv *& env )
	{
		int ret = -1;

		do
		{
			if ( !g_JavaVM || !g_EnvironsClass ) {
				CErr ( "AttachJavaThread: Failed to call from jni to java - invalid jni resources!" );
				break;
			}

#ifdef ATTACH_JAVA_THREAD
			int status = g_JavaVM->GetEnv ( (void **) &env, JNI_VERSION_1_6 );
			if ( status < 0 ) {
				CVerbVerb ( "AttachJavaThread: No JNI environment available, assuming native thread." );
#else
			int
#endif
				status = g_JavaVM->AttachCurrentThread ( &env, NULL );
				if ( status < 0 ) {
					CErr ( "AttachJavaThread: failed to attach current thread" );
					break;
				}
				if ( !env ) {
					CErr ( "AttachJavaThread: failed to get JNI environment" );
					g_JavaVM->DetachCurrentThread ( );
					break;
				}
				ret = 1;
				break;
#ifdef ATTACH_JAVA_THREAD
			}

			if ( !env ) {
				CErr ( "AttachJavaThread: failed to get JNI environment" );
				break;
			}
			ret = 0;
#endif
		}
		while ( false );

		return ret;
    }

    bool dPreparePrivateKey ( char ** privKey )
    {
        return true;
    }


    bool dUpdateKeyAndCert ( char * priv, char * cert )
    {
        return true;
    }

    
	bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen )
	{
		if ( !msg || !hashLen || !hash )
			return false;

		int status;
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		bool ret = false;
		char * retBlob = 0;

		jbyteArray jMsg = 0;
		do
		{
			jMsg = env->NewByteArray ( *hashLen );
			if ( !jMsg ) {
				CErr ( "SHAHashCreate: Alloc memory for cert failed." ); break;
			}
			env->SetByteArrayRegion ( jMsg, 0, *hashLen, (jbyte*)msg );

			jobject jByteBuffer = env->CallStaticObjectMethod ( g_EnvironsClass, g_SHAHashCreate, jMsg );
			if ( !jByteBuffer ) {
				CErr ( "SHAHashCreate: Java call failed!" ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jByteBuffer );
			char * buffer = (char *) env->GetDirectBufferAddress ( jByteBuffer );
			if ( !buffer || capacity <= 0 ) {
				CErr ( "SHAHashCreate: Failed to get reference to memory of the shared buffer!" ); break;
			}

			retBlob = (char *) malloc ( capacity + 1 );
			if ( !retBlob ) {
				CErr ( "SHAHashCreate: Failed to allocate memory for returning the blob!" ); break;
			}

			CVerbArg ( "SHAHashCreate: [%s]", ConvertToHexSpaceString ( buffer, capacity ) );

			memcpy ( retBlob, buffer, capacity );
			retBlob [capacity] = 0;
			*hash = retBlob;
			retBlob = 0;
			*hashLen = capacity;
			ret = true;
		}
		while ( 0 );

		if ( retBlob ) 	free ( retBlob );
		if ( jMsg )		env->DeleteLocalRef ( jMsg );
		if ( rc )
			g_JavaVM->DetachCurrentThread ( );

		return ret;
	}


	bool dGenerateCertificate ( char ** priv, char ** pub )
	{
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		bool ret = false;
		char * privKey = 0;
		char * pubKey = 0;
		do
		{
			jobject jBuffer = env->CallStaticObjectMethod ( g_EnvironsClass, g_GenerateCertificate );
			if ( !jBuffer ) {
				CErr ( "GenerateCertificate: Java call failed." ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jBuffer );
			unsigned char * buffer = (unsigned char *) env->GetDirectBufferAddress ( jBuffer );
			if ( !buffer || capacity <= 0 ) {
				CErr ( "GenerateCertificate: Failed to get reference to memory of the shared buffer!" );
				break;
			}

			unsigned int length = *((unsigned int *)buffer);
			length &= 0xFFFF;
			length += 4;
			privKey = (char *) malloc ( length );
			if ( !privKey ) {
				CErr ( "GenerateCertificate: Memory allocation failed." ); break;
			}
			CLogArg ( "GenerateCertificate: Copying [%i] bytes of encrypted message.", length );
			memcpy ( privKey, buffer, length );

			unsigned int length1 = *((unsigned int *)(buffer + length));
			length1 &= 0xFFFF;
			length1 += 4;
			pubKey = (char *) malloc ( length1 );
			if ( !pubKey ) {
				CErr ( "GenerateCertificate: Memory allocation failed." ); break;
			}
			CLogArg ( "GenerateCertificate: Copying [%i] bytes of encrypted message.", length1 );
			memcpy ( pubKey, buffer + length, length1 );

			*priv = privKey;
			privKey = 0;
			*pub = pubKey;
			pubKey = 0;
			ret = true;
		}
		while ( 0 );

		if ( privKey ) free ( privKey );
		if ( pubKey ) free ( pubKey );
		if ( rc )
			g_JavaVM->DetachCurrentThread ( );
		return ret;
	}


	bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
	{
		if ( !cert || !msg || !msgLen ) {
			CErrID ( "EncryptMessage: Called with at least one null argument." );
			return false;
		}

		unsigned int certProp = *((unsigned int *)cert);

        unsigned int certSize = certProp & 0xFFFF;

        CVerbArgID ( "EncryptMessage: Encrypting buffer sized [%i], certSize [%u]", *msgLen, certSize );

        if (certSize > 64000) {
            CErrArgID ( "EncryptMessage: Invalid certSize [%u]", certSize );
            return false;
        }

		bool ret = false;
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		jbyteArray 	jCert = 0;
		jbyteArray 	jMsg = 0;
		do
		{
			jCert = env->NewByteArray ( certSize );
			if ( !jCert ) {
				CErrID ( "EncryptMessage: Alloc memory for cert failed." ); break;
			}
			env->SetByteArrayRegion ( jCert, 0, certSize, (jbyte*)(cert + 4) );

			jMsg = env->NewByteArray ( *msgLen );
			if ( !jMsg ) {
				CErrID ( "EncryptMessage: Alloc memory for message failed." ); break;
			}
			env->SetByteArrayRegion ( jMsg, 0, *msgLen, (jbyte*)msg );

			jobject jCiphers = env->CallStaticObjectMethod ( g_EnvironsClass, g_EncryptMessage, deviceID, certProp, jCert, jMsg );
			if ( !jCiphers ) {
				CErrID ( "EncryptMessage: Java call failed." ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jCiphers );
			unsigned char * buffer = (unsigned char *) env->GetDirectBufferAddress ( jCiphers );
			if ( !buffer || capacity <= 0 ) {
				CErrID ( "EncryptMessage: Failed to get reference to memory of the shared buffer!" );
			}
			else {
				CVerbArgID ( "EncryptMessage: Copying [%i] bytes of encrypted message.", capacity );
				memcpy ( msg, buffer, capacity );
				msg [capacity] = 0;
				*msgLen = capacity;
				ret = true;

				CVerbVerbArgID ( "EncryptMessage: ciphers [%s]", ConvertToHexSpaceString (msg, capacity) );
			}
		}
		while ( 0 );

		if ( jCert )	env->DeleteLocalRef ( jCert );
		if ( jMsg )		env->DeleteLocalRef ( jMsg );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );
		return ret;
	}


	bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !key || !msg || !msgLen || keySize > 64000 || !decrypted || !decryptedSize ) {
            CErr ( "DecryptMessage: Called with at least one NULL (or invalid) argument." );
			return false;
        }
        CVerbArg ( "DecryptMessage: Decrypting msg of size [%i]", msgLen );


        key += 4;
        
		bool ret = false;
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		if ( pthread_mutex_lock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to acquire lock." );
			return false;
		}

		jbyteArray jKey = 0;
		jbyteArray jMsg = 0;
		char * plainText = 0;
		do
		{
			jKey = env->NewByteArray ( keySize );
			if ( !jKey ) {
				CErr ( "DecryptMessage: Alloc memory for cert failed." ); break;
			}
			env->SetByteArrayRegion ( jKey, 0, keySize, (jbyte*)key );

			jMsg = env->NewByteArray ( msgLen );
			if ( !jMsg ) {
				CErr ( "DecryptMessage: Alloc memory for message failed." ); break;
			}
			env->SetByteArrayRegion ( jMsg, 0,msgLen, (jbyte*)msg );

			jobject jCiphers = env->CallStaticObjectMethod ( g_EnvironsClass, g_DecryptMessage, jKey, jMsg );
			if ( !jCiphers ) {
				CErr ( "DecryptMessage: Java call failed." ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jCiphers );
			unsigned char * buffer = (unsigned char *) env->GetDirectBufferAddress ( jCiphers );
			if ( !buffer || capacity <= 0 ) {
				CErr ( "DecryptMessage: Failed to get reference to memory of the shared buffer!" ); break;
			}

			plainText = (char *) malloc ( capacity + 1 );
			if ( !plainText ) {
				CErr ( "DecryptMessage: Memory alloc failed." ); break;
			}

			CVerbArg ( "DecryptMessage: Copying [%i] bytes of decrypted message.", capacity );
			memcpy ( plainText, buffer, capacity );
			plainText [capacity] = 0;

			*decryptedSize = capacity;

			*decrypted = plainText;
			plainText = 0;

			ret = true;
		}
		while ( 0 );

		if ( plainText )	free ( plainText );
		if ( jKey )		env->DeleteLocalRef ( jKey );
		if ( jMsg )		env->DeleteLocalRef ( jMsg );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );

		if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to release lock." );
			ret = false;
		}

		return ret;
	}
    
    
    void dReleaseCert ( int deviceID )
    {
		CVerbArg ( "ReleaseCert: Disposing certificate context for deviceID [%u].", deviceID );

		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return;

		env->CallStaticVoidMethod ( g_EnvironsClass, g_ReleaseCert, deviceID );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );

    }


	long keyDeviceIDCounter = 0xFFFFFFFF;

	bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
    {
		if ( !key || keyLen < AES_SHA256_KEY_LENGTH || !ctx ) {
			CErrArg ( "AESDeriveKeyContext: Called with at least one NULL argument or keyLen [%u] < [%u].", keyLen, AES_SHA256_KEY_LENGTH ); return false;
		}

		CVerbVerb ( "AESDeriveKeyContext" );
        
        //AESDisposeKeyContext ( ctx );
        
        LockInit ( &ctx->encLock );
		LockInit ( &ctx->decLock ); 
		ctx->lockAllocated = true;

		/// Assign a temporary deviceId if required
		if ( ctx->deviceID == 0xFFFFFFFF ) {
			ctx->deviceID = __sync_sub_and_fetch ( &keyDeviceIDCounter, 1 );
            CVerbArg ( "AESDeriveKeyContext: Assigned temporary deviceID [%i].", (int)ctx->deviceID );
		}

		bool ret = false;

		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		jbyteArray jKey = 0;

        do
        {
			jKey = env->NewByteArray ( keyLen );
			if ( !jKey ) {
				CErr ( "AESDeriveKeyContext: Alloc memory for jKey failed." ); break;
			}
			env->SetByteArrayRegion ( jKey, 0, keyLen, (jbyte*)key );

			jobject jKeys = env->CallStaticObjectMethod ( g_EnvironsClass, g_AESDeriveKeyContext, ctx->deviceID, jKey, keyLen, AES_SHA256_KEY_LENGTH );
			if ( !jKeys ) {
				CErr ( "AESDeriveKeyContext: Java call failed." ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jKeys );
			unsigned char * buffer = (unsigned char *) env->GetDirectBufferAddress ( jKeys );
			if ( !buffer || capacity <= 0 ) {
				CErr ( "AESDeriveKeyContext: Failed to get reference to memory of the shared buffer!" ); break;
			}

			if ( capacity < AES_SHA256_KEY_LENGTH ){
				CErr ( "AESDeriveKeyContext: Insufficient key data received from java call!" ); break;
			}
			ctx->encCtx = (char *) 1;
			ctx->decCtx = (char *) 1;
			ctx->size = AES_SHA256_KEY_LENGTH;
            ret = true;
        }
        while ( 0 );

		if ( jKey )		env->DeleteLocalRef ( jKey );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );
		return ret;
    }


	void dAESDisposeKeyContext ( AESContext * ctx )
	{
		if ( !ctx ) {
			CVerb ( "AESDisposeKeyContext: Invalid context supplied." );
			return;
		}

		if ( ctx->lockAllocated ) {
			LockDispose ( &ctx->encLock );
			LockDispose ( &ctx->decLock );

			ctx->lockAllocated = false;
		}

		CVerbArg ( "AESDisposeKeyContext: Disposing context for deviceID [%u].", ctx->deviceID );

		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return;

		env->CallStaticVoidMethod ( g_EnvironsClass, g_AESDisposeKeyContext, ctx->deviceID );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );

		memset ( ctx, 0, sizeof(AESContext) );
	}


	void dAESUpdateKeyContext ( AESContext * ctx, int deviceID )
	{

		CVerbArg ( "AESUpdateKeyContext: Update context to deviceID [%u].", deviceID );

		if ( !ctx ) {
			CVerb ( "AESUpdateKeyContext: Invalid context supplied." );
			return;
		}
		CVerbArg ( "AESUpdateKeyContext: Temporary deviceID was [%u].", ctx->deviceID );

		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return;

		env->CallStaticVoidMethod ( g_EnvironsClass, g_AESUpdateKeyContext, ctx->deviceID, deviceID );

		if ( rc )
			g_JavaVM->DetachCurrentThread ( );
	}


    bool AESTransform ( AESContext * ctx, bool encrypt, char * msg, unsigned int *msgLen, char ** result )
    {
		bool ret = false;
		JNIEnv * env;
		int rc = AttachJavaThread ( env );
		if ( rc < 0 )
			return false;

		jbyteArray jMsg = 0;
		jbyteArray jIV = 0;
        
		unsigned char * buffer = 0;
		int deviceID = ctx->deviceID;

		char IV [16];
/*
		if ( pthread_mutex_lock ( &ctx->decMutex ) ) {
			CErrID ( "AESTransform: Failed to acquire mutex." );
			return false;
		}
		*/

		do
		{
			jIV = env->NewByteArray ( 16 );
			if ( !jIV ) {
				CErrID ( "AESTransform: Alloc memory for jIV failed." ); break;
			}

			if ( encrypt ) {
		        BUILD_IV_128 ( IV );
				env->SetByteArrayRegion ( jIV, 0, 16, (jbyte*)IV );

		        CVerbVerbArgID ( "AESTransform: Encrypt IV [%s]", ConvertToHexSpaceString ( IV, 16 ) );

				jMsg = env->NewByteArray ( *msgLen );
				if ( !jMsg ) {
					CErrID ( "AESTransform: Alloc memory for jMsg failed." ); break;
				}
				env->SetByteArrayRegion ( jMsg, 0, *msgLen, (jbyte*)msg );
			}
			else {
				if ( *msgLen < 36 ) {
					CErrArgID ( "AESTransform: Decrypt requested, but msg is missing the IV [%u].", *msgLen ); break;
				}
				env->SetByteArrayRegion ( jIV, 0, 16, (jbyte*)(msg + 4) );

		        CVerbVerbArgID ( "AESTransform: Decrypt IV [%s]", ConvertToHexSpaceString ( msg + 4, 16 ) );

				jMsg = env->NewByteArray ( *msgLen - 20 );
				if ( !jMsg ) {
					CErrID ( "AESTransform: Alloc memory for jMsg failed." ); break;
				}
				env->SetByteArrayRegion ( jMsg, 0, *msgLen - 20, (jbyte*)(msg + 20) );
			}

			jobject jBuffer = env->CallStaticObjectMethod ( g_EnvironsClass, g_AESTransform, deviceID, encrypt, jMsg, jIV );
			if ( !jBuffer ) {
				CErrID ( "AESTransform: Java call failed." ); break;
			}

			int capacity = env->GetDirectBufferCapacity ( jBuffer );
			buffer = (unsigned char *) env->GetDirectBufferAddress ( jBuffer );
			if ( !buffer || capacity <= 0 ) {
				CErrID ( "AESTransform: Failed to get reference to memory of the shared buffer!" ); break;
			}

			unsigned bufSize = encrypt ? capacity + 24 : capacity + 6; // 21/5

            char * blob = (char *) malloc ( bufSize );
            if ( !blob ) {
                CErrID ( "AESTransform: Failed to allocate memory." ); break;
            }

            if ( encrypt ) {
            	/// Copy IV
            	memcpy ( blob + 4, IV, 16 );
                memcpy ( blob + 20, buffer, capacity );
                capacity += 20;
                blob [ capacity ] = 0;
    			*((unsigned int *) blob) = (0x40000000 | capacity);
    			*msgLen = capacity;
            }
            else {
                memcpy ( blob, buffer, capacity );
                blob [ capacity ] = 0;
    			*msgLen = capacity;
            }
            *result = blob;
            ret = true;
    		CVerbArgID ( "AESTransform: %s buffer to size [%i] ", encrypt ? "Encrypted" : "Decrypted", capacity );
		}
		while ( 0 );

		if ( jMsg )		env->DeleteLocalRef ( jMsg );
		if ( jIV )		env->DeleteLocalRef ( jIV );
		if ( rc )
			g_JavaVM->DetachCurrentThread ( );
/*
		if ( pthread_mutex_unlock ( &ctx->decMutex ) ) {
			CErrID ( "AESTransform: Failed to release mutex." );
			ret = false;
		}
		*/

        return ret;
    }


    bool dAESEncrypt ( AESContext * ctx, char * msg, unsigned int *msgLen, char ** cipher )
    {
		if ( !ctx || !msg || !msgLen || !cipher ) {
			CErr ( "AESEncrypt: Called with at least one NULL argument." ); return false;
		}

		CVerbArg ( "AESEncrypt: Encrypting buffer sized [%i] ",  *msgLen );

		return AESTransform ( ctx, true, msg, msgLen, cipher );
    }


    bool dAESDecrypt ( AESContext * ctx, char * msg, unsigned int *msgLen, char ** decrypted )
    {
		if ( !ctx || !msg || !msgLen || !decrypted ) {
			CErr ( "AESDecrypt: Called with at least one NULL argument." ); return false;
		}

		return AESTransform ( ctx, false, msg, msgLen, decrypted );
    }


	namespace API
    {
        /*
         * Method:    WiFiUpdateWithColonMacN
         */
        ENVIRONSAPI void EnvironsFunc ( WiFiUpdateWithColonMacN, jstring bssid, jstring ssid, jint rssi, jint channel, jint encrypt, jint updateState )
        {
            INIT_PCHAR ( szBssid, bssid );
            INIT_PCHAR ( szSsid, ssid );

            if ( updateState == 1 )
                native.wifiObserver.Begin ();

            native.wifiObserver.UpdateWithColonMac ( szBssid, szSsid, rssi, 0, ( unsigned char ) channel, ( unsigned char ) encrypt );

            CVerbArg ( "WifiObserver: BSSID [ %s ]\t: rssi [ %i ]\t: channel [ %d ]\t: encrypt [ %c ]\t: SSID [ %s ]",
                      szBssid, szSsid, rssi, channel, encrypt );

            if ( updateState == 0 )
                native.wifiObserver.Finish ();

            RELEASE_PCHAR ( szSsid, ssid );
            RELEASE_PCHAR ( szBssid, bssid );
        }


        void Environs_LoginDialog ( int hInst, const char * userName )
        {
        }
        

		bool opt ( int hInst, const char * key, const char * value )
        {
            CVerbVerbArg ( "opt [%i]: Saving [%s] [%s]", hInst, key ? key : "NULL", value ? value : "NULL" );

			if ( !key || !value || !*key || !*value ) {
				CErrArg ( "opt: key [ %s ], value [ %s ]!", (key && *key) ? "valid" : "invalid", ( value && *value ) ? "valid" : "invalid" );
				return false;
			}

			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
				return false;

			bool		success = false;
			jbyteArray  jkey	= 0;
			jbyteArray  jvalue	= 0;

			try {
				do
				{
					size_t keyLength = strnlen ( key, MAX_NAMEPROPERTY * 10 );
					if ( !keyLength )
						break;

					jkey = env->NewByteArray ( keyLength );
					if ( !jkey )
						break;
					env->SetByteArrayRegion ( jkey, 0, keyLength, ( jbyte* ) key );

					size_t valueLength = strlen ( value );
					if ( !valueLength )
						break;

					jvalue = env->NewByteArray ( valueLength );
					if ( !jvalue )
						break;
					env->SetByteArrayRegion ( jvalue, 0, valueLength, ( jbyte* ) value );

					success = env->CallStaticBooleanMethod ( g_EnvironsClass, g_BridgeForOptSaveID, hInst, jkey, jvalue );
				}
				while ( false );
			}
			catch ( ... ) {
				CErr ( "opt: Exception happened!" );
			}

			if ( jkey )		env->DeleteLocalRef ( jkey );
			if ( jvalue )	env->DeleteLocalRef ( jvalue );

			if ( rc )
				g_JavaVM->DetachCurrentThread ( );

			return success;
		}


		bool opt ( JNIEnv * jenv, int hInst, const char * key, jstring value )
        {
            CVerbVerb ( "opt" );
            
			bool ret = false;

			if ( !value ) {
				CErr ( "opt: Invalid jstring value!" );
				return false;
			}

			INIT_PCHAR ( szValue, value );

			if ( szValue == NULL ) {
				CErr ( "opt: Invalid value!" );
				return false;
            }
            
            CVerbArg ( "opt [%i]: Saving [%s] [%s]", hInst, key ? key : "NULL", szValue ? szValue : "NULL" );

			ret = opt ( hInst, key, szValue );

			RELEASE_PCHAR ( szValue, value );

			return ret;
		}


		const char * opt ( int hInst, const char * key )
        {
            CVerbVerbArg ( "opt [%i]: Loading [%s]", hInst, key ? key : "NULL" );
            
			const char * value = optString ( hInst, key );

			if ( value && *value )
				return value;

			return "1";
		}


		const char * optString ( int hInst, const char * key )
        {
            CVerbVerbArg ( "optString [%i]: Loading [%s]", hInst, key ? key : "NULL" );

			static char buffer [ 4096 ];
			buffer [ 0 ] = 0;

			if ( !key || !*key ) {
				CErrArg ( "optString: key [ %s ]!", ( key && *key ) ? "valid" : "invalid" );
				return buffer;
			}

			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
				return buffer;

			jbyteArray  jkey	= 0;

			try {
				do
				{
					size_t keyLength = strnlen ( key, MAX_NAMEPROPERTY * 10 );
					if ( !keyLength )
						break;

					jkey = env->NewByteArray ( keyLength );
					if ( !jkey )
						break;
					env->SetByteArrayRegion ( jkey, 0, keyLength, ( jbyte* ) key );

					jstring jvalue = ( jstring ) env->CallStaticObjectMethod ( g_EnvironsClass, g_BridgeForOptLoadID, hInst, jkey );
					if ( jvalue ) {
						const char * value = env->GetStringUTFChars ( ( jstring ) jvalue, 0 );
						const size_t len = strlen ( value );
						if ( len < ( sizeof ( buffer ) - 1 ) ) {
							CVerbArg ( "optString: copying [ %s ]", value );
							strlcpy ( buffer, value, len );
							buffer [ len ] = 0;
						}
						env->ReleaseStringUTFChars ( jvalue, value );
					}
				}
				while ( false );
			}
			catch ( ... ) {
				CErr ( "optString: Exception happened!" );
			}

			if ( jkey )
				env->DeleteLocalRef ( jkey );

			if ( rc )
				g_JavaVM->DetachCurrentThread ();

			return buffer;
		}

        /**
         * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
         * 		such as when a connection has been established or closed.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The object id of the sender device.
         * @param notification  The received notification.
         * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
         * @param context       A value that provides additional context information (if available).
         */
		void BridgeForNotify ( jint hInst, jint objID, jint notification, jint source, void * contextPtr, int context )
		{
			//CVerbVerb ( "BridgeForNotify" );

			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
				return;

            jbyteArray jContext = 0;
            
			//CVerbVerbArg ( "BridgeForNotify: context [%i].", context );
            if ( contextPtr && context ) {
                jContext = env->NewByteArray ( context );
				//CVerbVerbArg ( "BridgeForNotify: creating context [%i].", context );
                
                if ( jContext ) {
                    env->SetByteArrayRegion ( jContext, 0, context, (jbyte*)contextPtr );
					//CVerbVerbArg ( "BridgeForNotify: copying context [%i].", context );
                }
            }

			env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForNotifyID, hInst, objID, notification, source, jContext, context );
            
            if ( jContext ) env->DeleteLocalRef ( jContext );
            
			if ( rc )
				g_JavaVM->DetachCurrentThread ( );
        }
        
        /**
         * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
         * 		such as when a connection has been established or closed.
         *
         * @param hInst			A handle to the Environs instance
         * @param deviceID      The device id of the sender device.
         * @param areaName		Area name of the application environment.
         * @param appName		Application name of the application environment.
         * @param notification  The received notification.
         * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
         * @param context       A value that provides additional context information (if available).
         */
		void BridgeForNotifyExt ( jint hInst, jint deviceID, const char * areaName, const char * appName, jint notification, jint source, void * contextPtr )
        {
			//CVerbVerb ( "BridgeForNotifyExt" );

            JNIEnv * env;
            int rc = AttachJavaThread ( env );
            if ( rc < 0 )
                return;
            
            jbyteArray jApp = 0;
            jbyteArray jArea = 0;

            try {
                size_t areaLength = 0;
                if ( areaName ) {
                    areaLength = strnlen ( areaName, MAX_NAMEPROPERTY );
                    
                    if ( areaLength ) {
                        jArea = env->NewByteArray ( areaLength );
                        
                        if ( jArea )
                            env->SetByteArrayRegion ( jArea, 0, areaLength, (jbyte*)areaName );
                    }
                }
                
                size_t appLength = 0;
                if ( appName ) {
                    appLength = strnlen ( appName, MAX_NAMEPROPERTY );
                    
                    if ( appLength ) {
                        jApp = env->NewByteArray ( appLength );
                        
                        if ( jApp )
                            env->SetByteArrayRegion ( jApp, 0, appLength, (jbyte*)appName );
                    }
                }

				int context = ( int ) ( long ) contextPtr;

				env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForNotifyExtID, hInst, deviceID, jArea, jApp, notification, source, context );
			}
			catch ( ... ) {
				CErrID ( "BridgeForNotifyExt: Exception happened!" );
			}
            
            if ( jApp ) env->DeleteLocalRef ( jApp );
            if ( jArea ) env->DeleteLocalRef ( jArea );
            
            if ( rc )
                g_JavaVM->DetachCurrentThread ( );
        }

        /**
         * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param type          The type of this message.
         * @param msg           The message.
         * @param length        The length of the message.
         */
		void BridgeForMessage ( jint hInst, jint objID, int type, const void * message, jint length )
		{
			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
                return;
            
            jbyteArray jMsg = 0;

            try {
                jMsg = env->NewByteArray ( length );
                if ( !jMsg ) {
                    CErr ( "BridgeForMessageExt: Alloc memory for cert failed." );
                }
                else {
                    env->SetByteArrayRegion ( jMsg, 0, length, (jbyte*)message );
                    
                    //jstring msg = env->NewStringUTF ( ( const char * ) message ); // Crash on Base64 encoded data
                    
                    env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForMessageID, hInst, objID, type, jMsg, length );
                }
			}
			catch ( ... ) {
				CErr ( "BridgeForMessage: Exception happened!" );
            }
            
            if ( jMsg ) env->DeleteLocalRef ( jMsg );

			if ( rc )
				g_JavaVM->DetachCurrentThread ( );
        }
        
        /**
         * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param deviceID      The device id of the sender device.
         * @param areaName		Area name of the application environment
         * @param appName		Application name of the application environment
         * @param type          The type of this message.
         * @param msg           The message.
         * @param length        The length of the message.
         */
		void BridgeForMessageExt ( jint hInst, jint deviceID, const char * areaName, const char * appName, int type, const void * message, jint length )
        {
            JNIEnv * env;
            int rc = AttachJavaThread ( env );
            if ( rc < 0 )
                return;
            
            jbyteArray jMsg = 0;
            jbyteArray jApp = 0;
            jbyteArray jArea = 0;
            
            try {
                jMsg = env->NewByteArray ( length );
                
                if ( !jMsg ) {
                    CErr ( "BridgeForMessageExt: Alloc memory failed." );
                }
                else {
                    env->SetByteArrayRegion ( jMsg, 0, length, (jbyte*)message );
                    
                    size_t areaLength = 0;
                    if ( areaName ) {
                        areaLength = strnlen ( areaName, MAX_NAMEPROPERTY );
                        
                        if ( areaLength ) {
                            jArea = env->NewByteArray ( areaLength );
                            
                            if ( jArea )
                                env->SetByteArrayRegion ( jArea, 0, areaLength, (jbyte*)areaName );
                        }
                    }
                    
                    size_t appLength = 0;
                    if ( appName ) {
                        appLength = strnlen ( appName, MAX_NAMEPROPERTY );
                        
                        if ( appLength ) {
                            jApp = env->NewByteArray ( appLength );
                            
                            if ( jApp )
                                env->SetByteArrayRegion ( jApp, 0, appLength, (jbyte*)appName );
                        }
                    }
                    
                    env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForMessageExtID, hInst, deviceID, jArea, jApp, type, jMsg, length );
                }
			}
			catch ( ... ) {
				CErrID ( "BridgeForMessageExt: Exception happened!" );
            }
            
            if ( jMsg ) env->DeleteLocalRef ( jMsg );
            if ( jApp ) env->DeleteLocalRef ( jApp );
            if ( jArea ) env->DeleteLocalRef ( jArea );
            
            if ( rc )
                g_JavaVM->DetachCurrentThread ( );
        }

        /**
         * BridgeForStatusMessage static method to be called by native layer in order to drop a status messages.
         *
         * @param hInst			A handle to the Environs instance
         * @param msg           A status message of Environs.
         */
		void BridgeForStatusMessage ( jint hInst, const char * message )
		{
            if ( !message )
                return;
            
			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
                return;
            
            jbyteArray jMsg = 0;

            try {
                size_t length = strnlen ( message, 1024 );
                
                if ( length ) {
                    jMsg = env->NewByteArray ( length );
                    
                    if ( jMsg ) {
                        env->SetByteArrayRegion ( jMsg, 0, length, (jbyte*)message );
                        
                        env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForStatusMessageID, hInst, jMsg );
                    }
                }
			}
			catch ( ... ) {
				CErr ( "BridgeForStatusMessage: Exception happened!" );
            }
            
            if ( jMsg ) env->DeleteLocalRef ( jMsg );

			if ( rc )
				g_JavaVM->DetachCurrentThread ( );
		}

        
        /**
         * BridgeForData static method to be called by native layer in order to notify about data received from a device.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param nativeID      The native device id of the sender device.
         * @param type          The type of this message.
         * @param fileID        A fileID that was attached to the buffer.
         * @param descriptor    A descriptor that was attached to the buffer.
         * @param size          The size of the data buffer.
         */
        void BridgeForData ( jint hInst, jint objID, jint nativeID, jint type, jint fileID, const char * fileDescriptor, jint size )
        {
            JNIEnv * env;
            int rc = AttachJavaThread ( env );
            if ( rc < 0 )
                return;
            
            CVerbArg ( "BridgeForData: delivering [%s] with [%d] bytes and fileID [%d]", fileDescriptor ? fileDescriptor : "", size, fileID );
            
            jbyteArray jDesc = 0;
            
            try {
                size_t length = 0;
                if ( fileDescriptor ) {
                    length = strnlen ( fileDescriptor, 1024 );
                    
                    if ( length ) {
                        jDesc = env->NewByteArray ( length );
                        
                        if ( jDesc )
                            env->SetByteArrayRegion ( jDesc, 0, length, (jbyte*)fileDescriptor );
                    }
                }

				env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForDataID, hInst, objID, nativeID, type, fileID, jDesc, size );
            }
            catch ( ... ) {
                CErr ( "BridgeForData: Exception happened!" );
            }
            
            if ( jDesc ) env->DeleteLocalRef ( jDesc );
            
            if ( rc )
                g_JavaVM->DetachCurrentThread ( );
        }

        
        /**
         * BridgeForUdpData static method to be called by native layer in order to notify about udp data received from a device.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param pack          A udp data structure containing the received udp or sensor data.
         * @param packSize      The size of the data buffer in number of bytes.
         */
        void BridgeForUdpData ( jint hInst, jint objID, environs::SensorFrame * frame, int packSize  )
        {
			if ( !frame || packSize <= 0 )
				return;

            JNIEnv * env;
            int rc = AttachJavaThread ( env );
            if ( rc < 0 )
                return;
            
            CVerbVerb ( "BridgeForUdpData" );
            
            jbyteArray jData = 0;
            
            try {
                jData = env->NewByteArray ( packSize );
                
                if ( jData ) {
                    env->SetByteArrayRegion ( jData, 0, packSize, ( jbyte* ) frame );
                
                    env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForUdpDataID, hInst, objID, jData, packSize );
                }
            }
            catch ( ... ) {
                CErr ( "BridgeForUdpData: Exception happened!" );
            }
            
            if ( jData ) env->DeleteLocalRef ( jData );
            
            if ( rc )
                g_JavaVM->DetachCurrentThread ( );
        }


		void UpdateNetworkStatus ()
		{
			JNIEnv * env;
			int rc = AttachJavaThread ( env );
			if ( rc < 0 )
				return;

			env->CallStaticVoidMethod ( g_EnvironsClass, g_BridgeForUpdateNetworkStatus, -2 );

			if ( rc )
				g_JavaVM->DetachCurrentThread ( );
		}


		/**
		* Method:    BridgeForNotifier
		* Signature: (II)V Java_hcm_environs_Environs
         */
		ENVIRONSAPI void EnvironsFunc ( BridgeForNotifier, jint hInst, jint objID, jint notification, jint source, jint context )
		{
			onEnvironsNotifierContext1 ( instances[hInst], objID, notification, source, 0, context );
         }
        
        
        /**
         * Method:    BridgeForNotifier
         * Signature: (II)V Java_hcm_environs_Environs
        ENVIRONSAPI void EnvironsFunc ( BridgeForNotifierExt, jint nativeID, jint notification, jint source, jint context )
        {
            INIT_PCHAR ( szProjName, projName );
            INIT_PCHAR ( szAppName, appName );
            
            onEnvironsNotifierContext ( env, deviceID, szProjName, szAppName, notification, source, context );
            
            RELEASE_PCHAR ( szAppName, appName );
            RELEASE_PCHAR ( szProjName, projName );
         }
         */
        


		ENVIRONSAPI jint JNI_OnLoad ( JavaVM* vm, void* reserved )
		{
			JNIEnv * env = NULL;
			g_JavaVM = vm;
            
			CVerb ( "JNI_OnLoad: started." );

			if ( vm->GetEnv ( (void**)&env, JNI_VERSION_1_6 ) != JNI_OK ) {
				CErr ( "JNI_OnLoad: Failed to get the environment using GetEnv()!" );
				return -1;
			}

			if ( !env ) {
				CErr ( "JNI_OnLoad: Invalid environment from GetEnv()!" );
				return -1;
			}

			// Get class
			g_EnvironsClass = (jclass) env->NewGlobalRef ( env->FindClass ( "environs/Environs" ) );
			if ( !g_EnvironsClass ) {
				CErr ( "JNI_OnLoad: Failed to get the class environs/Environs using FindClass()" );
				return -1;
			}

			
			static JNINativeMethod methods[] = {
                {"BridgeForNotify",         "(IIII[BI)V",                                   (void *)&g_BridgeForNotifyID},
                {"BridgeForNotifyExt",      "(II[B[BIII)V",                                 (void *)&g_BridgeForNotifyExtID},
			    {"BridgeForMessage",        "(III[BI)V",                                    (void *)&g_BridgeForMessageID},
                {"BridgeForMessageExt",     "(II[B[BI[BI)V",                                (void *)&g_BridgeForMessageExtID},
			    {"BridgeForStatusMessage",  "(I[B)V",                                       (void *)&g_BridgeForStatusMessageID},
			    {"BridgeForData",        	"(IIIII[BI)V",									(void *)&g_BridgeForDataID},
			    {"BridgeForUdpData",        "(II[BI)V",										(void *)&g_BridgeForUdpDataID },

			    {"Java_malloc",        		"(IZ)Ljava/nio/ByteBuffer;",                    (void *)&g_java_mallocID},
			    {"Java_free",        		"(Ljava/nio/ByteBuffer;)Z",                     (void *)&g_java_freeID},
                
			    {"optBytes",        		"(I[B[B)Z",										(void *)&g_BridgeForOptSaveID},
			    {"optBytes",        		"(I[B)Ljava/lang/String;",						(void *)&g_BridgeForOptLoadID},
                
                {"UpdateNetworkStatus",    	"(I)V",                                         (void *)&g_BridgeForUpdateNetworkStatus},
                
                {"SHAHashCreate",        	"([B)Ljava/nio/ByteBuffer;",                    (void *)&g_SHAHashCreate},
                {"EncryptMessage",        	"(II[B[B)Ljava/nio/ByteBuffer;",                (void *)&g_EncryptMessage},
                {"DecryptMessage",        	"([B[B)Ljava/nio/ByteBuffer;",                  (void *)&g_DecryptMessage},
                {"ReleaseCert",             "(I)V",                                         (void *)&g_ReleaseCert},
                
                {"AESTransform",        	"(IZ[B[B)Ljava/nio/ByteBuffer;",                (void *)&g_AESTransform},
                {"AESDeriveKeyContext",     "(I[BII)Ljava/nio/ByteBuffer;",                 (void *)&g_AESDeriveKeyContext},
                {"AESDisposeKeyContext",    "(I)V",                                         (void *)&g_AESDisposeKeyContext},
                {"AESUpdateKeyContext",     "(II)V",                                        (void *)&g_AESUpdateKeyContext},
                {"GenerateCertificate",     "()Ljava/nio/ByteBuffer;",                      (void *)&g_GenerateCertificate},

				{"Plugin_CreateInstance",   "(IIII)I",                                      (void *)&g_CreateInstance},
				{"Plugin_Encoder_Init",		"(IIIIIII)I",                                   (void *)&g_Encoder_Init},
				{"Plugin_DestroyInstance",  "(II)I",										(void *)&g_DestroyInstance},
			};
            
			for ( unsigned int i=0; i<sizeof(methods)/sizeof(methods[0]); i++ ) 
			{
				JNINativeMethod jm = methods [i];

				*((jmethodID *)jm.fnPtr) = env->GetStaticMethodID ( g_EnvironsClass, jm.name, jm.signature ); 
				
				if ( !*((jmethodID *)jm.fnPtr) ){
					CErrArg ( "JNI_OnLoad: Failed to get the method [%s] using GetStaticMethodID()", jm.name );
					return -1;
				}
            }
            
            native.display.orientation = DISPLAY_ORIENTATION_PORTRAIT;

            DetermineSensorSupport ();
    
			CInfo ( "JNI_OnLoad: success." );

			return JNI_VERSION_1_6;
		}
        

		/*
		* Method:    IsNativeAllocatedN
		* Signature: ()V
		*/
		ENVIRONSAPI jboolean EnvironsProc ( IsNativeAllocatedN )
		{
			CVerb ( "IsNativeAllocatedN" );

			return (environs::native.IsNativeAllocated () == true);
        }
        
        
#ifdef USE_NATIVE_SENSOR_READER

        /**
         * @param rad   Angle in radians.
         * @return degrees for given radians
         */
        float GetDegrees ( float rad )
        {
            double deg = ((rad * 180) / PI);
            if (deg < 0)
                deg = 360 + deg;

            return (float) deg;
        }


		float envAccLinear [ 3 ];
		float envGravityLinear [ 3 ];

        int SensorEventsCallback ( int fd, int events, void* data )
        {
            CVerb ( "SensorEventsCallback" );
            
            ASensorEvent ev;
			FAKEJNI ();

            while ( ASensorEventQueue_getEvents ( sensorEventQueue, &ev, 1 ) > 0 )
            {
				if ( ev.type >= envSensorTypeByAndroidTypeSize )
					continue;

				environs::SensorType_t sensorType = ( environs::SensorType_t ) getEnvSensorTypeByAndroidType [ ev.type ];
				if ( sensorType < 0 )
					continue;

				switch ( ev.type )
				{
				case ASENSOR_TYPE_ACCELEROMETER:
				case ASENSOR_TYPE_GYROSCOPE:
				case ASENSOR_TYPE_MAGNETIC_FIELD:
				case EnvASENSOR_TYPE_ROTATION_VECTOR:
					CVerbArg ( "SensorEventsCallback: [ %s ] x [ %f ] y [ %f ] z [ %f ]", sensorFlagDescriptions [ sensorType ], ev.acceleration.x, ev.acceleration.y, ev.acceleration.z );

					EnvironsCallArg ( PushSensorDataN, sensorType, ev.acceleration.x, ev.acceleration.y, ev.acceleration.z ); // x, y, z
                        break;

                    case EnvASENSOR_TYPE_ORIENTATION:
                        CVerbArg ( "SensorEventsCallback: [ %s ] x [ %f ] y [ %f ] z [ %f ]", sensorFlagDescriptions [ sensorType ], ev.acceleration.x, ev.acceleration.y, ev.acceleration.z );

                        EnvironsCallArg ( PushSensorDataN, sensorType, GetDegrees(ev.acceleration.x), GetDegrees(ev.acceleration.y), GetDegrees(ev.acceleration.z) ); // x, y, z
                        break;
                        
                case ASENSOR_TYPE_PROXIMITY:
                case EnvASENSOR_TYPE_PRESSURE:
				case ASENSOR_TYPE_LIGHT:
				case EnvASENSOR_TYPE_TEMPERATURE:
				case EnvASENSOR_TYPE_HUMIDITY:
					CVerbArg ( "SensorEventsCallback: %s [ %f ] ", sensorFlagDescriptions [ sensorType ], ev.data [ 0 ] );

					EnvironsCallArg ( PushSensorDataN, sensorType, ev.data [ 0 ], 0, 0 ); // x, y, z
					break;

				case EnvASENSOR_TYPE_GRAVITY:
					CVerbArg ( "SensorEventsCallback: Gravity x [ %f ] y [ %f ] z [ %f ]", ev.acceleration.x, ev.acceleration.y, ev.acceleration.z );
					envGravityLinear [ 0 ] = ev.acceleration.v [ 0 ];
					envGravityLinear [ 1 ] = ev.acceleration.v [ 1 ];
					envGravityLinear [ 2 ] = ev.acceleration.v [ 2 ];

					EnvironsCallArg ( PushSensorDataExtN, false, environs::SensorType::MotionGravityAcceleration, envAccLinear [ 0 ], envAccLinear [ 1 ], envAccLinear [ 2 ], envGravityLinear [ 0 ], envGravityLinear [ 1 ], envGravityLinear [ 2 ] ); // x, y, z
					break;

				case EnvASENSOR_TYPE_ACCELEROMETER_LINEAR:
					CVerbArg ( "SensorEventsCallback: Acc linear x [ %f ] y [ %f ] z [ %f ]", ev.acceleration.x, ev.acceleration.y, ev.acceleration.z );
					envAccLinear [ 0 ] = ev.acceleration.v [ 0 ];
					envAccLinear [ 1 ] = ev.acceleration.v [ 1 ];
					envAccLinear [ 2 ] = ev.acceleration.v [ 2 ];

					EnvironsCallArg ( PushSensorDataExtN, false, environs::SensorType::MotionGravityAcceleration, envAccLinear [ 0 ], envAccLinear [ 1 ], envAccLinear [ 2 ], envGravityLinear [ 0 ], envGravityLinear [ 1 ], envGravityLinear [ 2 ] ); // x, y, z
					break;

				default:
					break;
				}
            }

            return 1; // Continue - 0 means stop handling
        }
#endif
        
		void DetermineSensorSupport ( )
		{
#ifdef USE_NATIVE_SENSOR_READER
			CLog ( "DetermineSensorSupport" );

			ASensorList sensors;

			int count = ASensorManager_getSensorList ( ASensorManager_getInstance (), &sensors );

			CLogArg ( "DetermineSensorSupport: [ %i ] sensors.", count );
			
			for ( int i = 0; i < count; i++ ) 
			{
				ASensorRef      sensor      = sensors [ i ];

				int             type        = ASensor_getType ( sensor );
				const char *    name        = ASensor_getName ( sensor );
				const char *    envName     = 0;
				bool            supported   = false;

                if ( type < envSensorTypeByAndroidTypeSize )
                {
                    environs::SensorType_t sensorType = ( environs::SensorType_t ) getEnvSensorTypeByAndroidType [ type ];

                    if ( sensorType >= 0 && sensorType < SensorType::Max )
                    {
                        envName = sensorFlagDescriptions [ sensorType ];

                        unsigned int flag = sensorFlags [ sensorType ];
                        if ( flag ) {
                            sensorsAvailable |= flag; supported = true;

                            CLogArg ( "DetermineSensorSupport: Sensor [ %i : %s / %s ].", type, name, envName ? envName : "Unknown" );
                        }
                    }
                }

				if ( !supported ) {
					CWarnArg ( "DetermineSensorSupport: Sensor [ %i : %s ] not supporeted by Environs.", type, name );
				}
			}
#endif
		}


        bool CreateSensorManager ()
        {
#ifdef USE_NATIVE_SENSOR_READER
            if ( sensorEventQueue ) {
                CLog ( "CreateSensorManager: Sensor event queue exists!" );
                return true;
            }

            CVerb ( "CreateSensorManager" );

            if ( !sensorLooper ) {
                CErr ( "CreateSensorManager: Looper is missing." );
                return false;
            }

            if ( !sensorManager ) {
                CLog ( "CreateSensorManager: Acquiring SensorManager" );
                sensorManager = ASensorManager_getInstance ();
                
                if ( !sensorManager ) {
                    CErr ( "CreateSensorManager: Acquiring SensorManager failed." );
                    return false;
                }
            }
            
            if ( !sensorEventQueue ) {
                CLog ( "CreateSensorManager: Creating EventQueue" );

                sensorEventQueue = ASensorManager_createEventQueue ( sensorManager, sensorLooper, 3, SensorEventsCallback, 0 );
                if ( !sensorEventQueue ) {
                    CErr ( "CreateSensorManager: Creating EventQueue failed." );
                    return false;
                }
            }
            
            return true;
#endif
        }
        

        void InitSensorMainThreaded ()
        {
#ifdef USE_NATIVE_SENSOR_READER
            CVerb ( "InitSensorMainThreaded" );

            if ( !sensorLooper ) {
                CLog ( "InitSensorMainThreaded: Acquiring Looper" );
                sensorLooper = ALooper_forThread ();

                if ( !sensorLooper ) {
                    CLog ( "InitSensorMainThreaded: ALOOPER_PREPARE_ALLOW_NON_CALLBACKS" );
                    sensorLooper = ALooper_prepare ( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
                }

                if ( !sensorLooper ) {
                    CErr ( "InitSensorMainThreaded: Acquiring Looper failed." );
                }
            }
#endif
        }


        void DisposeSensorImpl ()
        {
#ifdef USE_NATIVE_SENSOR_READER
            CVerb ( "DisposeSensorImpl" );

			StopSensorListeningImpl ( 1, ( environs::SensorType_t ) -1 );

            if ( sensorEventQueue ) 
			{
                if ( sensorManager ) {
                    CLog ( "DisposeSensorImpl: Releasing event queue" );
                    ASensorManager_destroyEventQueue ( sensorManager, sensorEventQueue );
                }

                sensorEventQueue = 0;
            }

            if ( sensorManager ) {
                CLog ( "DisposeSensorImpl: Releasing sensor manager" );
                sensorManager = 0;
            }

			Zero ( sensorsAndroid );

            if ( sensorLooper ) {
                CLog ( "DisposeSensorImpl: Releasing Looper" );
                ALooper_release ( sensorLooper );
                sensorLooper = 0;
            }
#endif
        }


        /**
         * Register to sensor events and listen to sensor data events.
         * This functionality is implemented in the platform layer.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         */
        bool StartSensorListeningImpl ( int hInst, environs::SensorType_t sensorType )
        {
            bool success = false;
            
#ifdef USE_NATIVE_SENSOR_READER
            CVerbArg ( "StartSensorListeningImpl: hInst [ %i ] sensorType [ %i ]", hInst, sensorType );

            if ( !CreateSensorManager () )
                return false;

			if ( IsSensorAvailableImpl ( 1, sensorType ) ) 
			{
				int type = getAndroidSensorType [ sensorType ];

				if ( !sensorsAndroid [ sensorType ] && type > 0 && type  < androidSensorTypeSize )
                {
                    CVerbArg ( "StartSensorListeningImpl: Acquire sensor [ %s ]", sensorFlagDescriptions [ sensorType ] );

					sensorsAndroid [ sensorType ] = ASensorManager_getDefaultSensor ( sensorManager, type );
				}

				if ( sensorsAndroid [ sensorType ] ) {
					CLogArg ( "StartSensorListeningImpl: enableSensor [ %s ]", sensorFlagDescriptions [ sensorType ] );

					ASensorEventQueue_enableSensor ( sensorEventQueue, sensorsAndroid [ sensorType ] );

					CVerbArg ( "StartSensorListeningImpl: setEventRate [ %s ]", sensorFlagDescriptions [ sensorType ] );

                    ASensorEventQueue_setEventRate ( sensorEventQueue, sensorsAndroid [ sensorType ], sensorsEventRate [ sensorType ] );
					success = true;
				}
				else {
					CErrArg ( "StartSensorListeningImpl: Get sensor  [ %s ] failed.", sensorFlagDescriptions [ sensorType ] );
				}
			}
#endif
            return success;
        }
        
        
        /**
         * Deregister to sensor events and stop listen to sensor data events.
         * This functionality is implemented in the platform layer.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         */
        void StopSensorListeningImpl ( int hInst, environs::SensorType_t sensorType )
        {
#ifdef USE_NATIVE_SENSOR_READER
            CVerbArg ( "StopSensorListeningImpl: hInst [ %i ] sensorType [ %i ]", hInst, sensorType );

            if ( !sensorEventQueue ) {
                CWarnArg ( "StopSensorListeningImpl: No event queue for sensorType [ %i ]", hInst, sensorType );
                return;
            }

            if ( sensorType == -1 ) {
                CLog ( "StopSensorListeningImpl: Stopping all sensors ..." );
                sensorRegistered = 0;
                
                if ( sensorEventQueue ) {
					for ( size_t i = 0; i < SensorType::Max; ++i )
					{
                        if ( sensorsAndroid [ i ] ) {
                            CLogArg ( "StopSensorListeningImpl: Stopping [ %s ]", sensorFlagDescriptions [ sensorType ] );

							ASensorEventQueue_disableSensor ( sensorEventQueue, sensorsAndroid [ i ] );
						}
					}
                }
                return;
            }
            
            if ( sensorsAndroid [ sensorType ] ) {
                CLogArg ( "StopSensorListeningImpl: hInst [ %i ] Stopping [ %s ]", hInst, sensorFlagDescriptions [ sensorType ] );

				ASensorEventQueue_disableSensor ( sensorEventQueue, sensorsAndroid [ sensorType ] );
			}
#endif
        }
        
        
        /**
         * Determine whether the given sensorType is available.
         *
         * @param sensorType A value of type environs::SensorType.
         *
         * @return success true = enabled, false = failed.
         */
        bool IsSensorAvailableImpl ( int hInst, environs::SensorType_t sensorType )
        {
			if ( sensorType >= 0 && sensorType < SensorType::Max )
				return ( ( sensorsAvailable & sensorFlags [ sensorType ] ) != 0 );
			return false;
        }
        
    } /* namespace API */


} /* namespace environs */



#endif



