/**
 * Environs mobile specific
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
#ifndef INCLUDE_HCM_ENVIRONS_MOBILE_COMMON_H
#define INCLUDE_HCM_ENVIRONS_MOBILE_COMMON_H

#ifndef DISPLAYDEVICE

#   include "Environs.Native.h"
#   include "Environs.Msg.Types.h"
#   include "Environs.Android.h"
#   include "Environs.iOS.h"
#   include "Environs.Win.Phone.h"

namespace environs
{
	extern lib::SensorFrame	g_OrientationFrame;
	extern lib::SensorFrame	g_AccelFrame;
	extern unsigned int		g_sendSequenceNumber;

	extern bool				opt_useSensors;
	extern bool				opt_usePushNotifications;
	extern bool				opt_useNativeDecoder;
	extern bool				opt_useHardwareEncoder;
    
    extern bool             AllocNativePlatformMobile ();
    extern bool             DetermineAndInitWorkDir ();
    
    extern void             DeallocNativePlatform ();
    
	namespace API
	{
        
		extern bool opt ( int hInst, const char * key, const char * value );
		extern bool opt ( JNIEnv * jenv, int hInst, const char * key, jstring value );
		extern const char * opt ( int hInst, const char * key );
		extern bool opt ( int hInst, const char * key, bool value );
		extern bool optBool ( int hInst, const char * key );
		extern const char * optString ( int hInst, const char * key );
        
        extern void BridgeForData ( jint hInst, jint objID, jint nativeID, jint type, jint fileID, const char * fileDescriptor, jint size );
        extern void BridgeForUdpData ( jint hInst, jint objID, SensorFrame * pack, int packSize );
        
        extern void BridgeForMessage ( jint hInst, jint objID, int type, const void * message, int length );
        extern void BridgeForMessageExt ( jint hInst, int deviceID, const char * areaName, const char * appName, int type, const void * message, int length );
        
        extern void BridgeForNotify ( jint hInst, jint objID, int notification, jint source, void * contextPtr, int context );
        extern void BridgeForNotifyExt ( jint hInst, jint deviceID, const char * areaName, const char * appName, jint notification, jint source, void * contextPtr );
        
        extern void BridgeForStatusMessage ( jint hInst, const char * message );        
        
	} /* namespace API */

} /* namespace environs */

#endif


#endif  /// end-INCLUDE_HCM_ENVIRONS_MOBILE_COMMON_H



