/**
 * DeviceController for mobile device platforms
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

#ifndef DISPLAYDEVICE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifndef WINDOWS_PHONE
#	include <string.h>
#	include <stdio.h>
#	include <errno.h>
#endif

#include "Environs.Obj.h"
using namespace environs::API;

#include "Environs.Modules.h"

#include "Device.Mobile.h"
#include "Mediator.h"
#include "Core/Kernel.Mobile.h"
#include "Portal/Portal.Stream.h"
#include "Portal/Portal.Generator.iOS.h"
#include "Portal/Portal.Generator.Android.h"
#include "Portal/Portal.Generator.WP.h"


#ifdef WINDOWS_PHONE
#   include "winsock2.h"
#else
#   include <arpa/inet.h>
#   include <netinet/tcp.h>
#   include <sys/socket.h>
#   include <unistd.h>
#endif


#define	CLASS_NAME 	"Device.Platform. . . . ."


/* Namespace: environs -> */
namespace environs
{
    extern void CreateCopyString ( const char * src, char ** dest );
    
    
    void DevicePlatform::Construct ()
    {
        CVerbID ( "Construct" );
        
        
        streamOptions.limitMaxResolution	= false;
    }
    
    
    DevicePlatform::DevicePlatform ( )
    {
        CVerbID ( "Constructor" );
    }
    
    
    DevicePlatform::DevicePlatform ( int deviceID )
    {
        this->deviceID = deviceID;
        
        CVerbID ( "Constructor" );
        
        Construct ();
    }
    
    
    DevicePlatform::DevicePlatform ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr )
    {
        this->deviceID = deviceID;
        
        CVerbID ( "Constructor" );
        
        Construct ();
        
        if ( isInteractChannel ) {
            interactSocket			= sock;
			interactSocketForClose	= sock;
            if ( addr ) {
                memcpy ( &interactAddr, addr, sizeof(struct sockaddr_in) );
                memcpy ( &udpAddr.sin_addr, &interactAddr.sin_addr, sizeof(interactAddr.sin_addr) );
            }
        }
        else {
            comDatSocket			= sock;
			comDatSocketForClose	= sock;
            if ( addr ) {
                memcpy ( &comDatAddr, addr, sizeof(struct sockaddr_in) );
                memcpy ( &udpAddr.sin_addr, &comDatAddr.sin_addr, sizeof(comDatAddr.sin_addr) );
            }
        }
    }
    
    
    DevicePlatform::~DevicePlatform ( )
    {
        CVerbID ( "Destructor" );
        
		Release ();
        
        CVerbID ( "Destructor done" );
    }
    
    
    void DevicePlatform::Release ( )
    {
		LockAcquireVA ( spLock, "DisposePlatform" );

		activityStatus |= DEVICE_ACTIVITY_PLATFORM_DISPOSED;

        CVerbID ( "Release: setting deviceStatus to Deleteable" );
        deviceStatus = DeviceStatus::Deleteable;
        
        LockReleaseVA ( spLock, "DisposePlatform" );
        
        CloseListeners ();
    }
    
    
    /**
     * Connect to device with the given ID.
     *
     * @param deviceID	Destination device ID
     * @return	0 Connection can't be conducted (maybe environs is stopped or the device ID is invalid)
     * @return	1 A connection to the device already exists or a connection task is already in progress)
     * @return	2 A new connection has been triggered and is in progress
     */
    int DevicePlatform::DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle )
    {
        return DeviceBase::ConnectToDevice ( hInst, Environs_CALL_, deviceID, areaName, appName );
    }
    
    
    void DevicePlatform::CreatePortalGeneratorPlatform ( int portalIDent )
    {
        CVerbID ( "CreatePortalGeneratorPlatform" );

		portalGenerators [ portalIDent ] = new PortalGeneratorMobile ();
        
        if ( !portalGenerators [ portalIDent ] )
            return;
        
        portalGenerators [ portalIDent ]->env = env;
    }
    
    
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
    
    bool DevicePlatform::EvaluateDeviceConfig ( char * msg )
    {
        CVerbID ( "EvaluateDeviceConfig" );
        
        // id:int;ip:ip-addr;wp:width-pixel;hp:height-pixel;w:width;h:height;tr:h4<EOF>
        
        int clientID = 0;
        char appName [ MAX_NAMEPROPERTY ];
        unsigned int appNameLen = 0;
        *appName = 0;
        
        char areaName [ MAX_NAMEPROPERTY ];
        unsigned int areaNameLen = 0;
        *areaName = 0;
        
        
        char * context = 0;
        char * psem = strtok_s ( msg, ";", &context );
        
        while (psem != NULL)
        {
            switch ( psem [ 0 ] ) {
                case 'a': // an
                    if ( psem [1] == 'n' && psem [2] == ':' ) {
                        appNameLen = (unsigned int) strlen ( psem + 3 );
                        if ( appNameLen >= MAX_NAMEPROPERTY ) {
                            CWarnArgID ( "EvaluateDeviceConfig: Application name [%s] is too long.", psem + 3 );
                            appNameLen = 0;
                        }
                        else {
                            memcpy ( appName, psem + 3, appNameLen );
                            appName [appNameLen] = 0;
                        }
                    }
                    break;
                    
                case 'd': // do
                    if ( psem [ 1 ] == 'o' && psem [ 2 ] == ':'  ) {
                        if ( 1 != sscanf_s ( psem + 3, "%i", &display.orientation ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for display orientation." );
                        }
                    }
                    break;
                case 'i': // id | ip
                    if ( psem [ 1 ] == 'd' && psem [ 2 ] == ':'  )
                    {
                        sscanf ( psem + 3, "%d", &clientID );
                    }
                    break;
                    
                case 'h': // h | hp
                    if ( psem [ 1 ] == 'p' && psem [ 2 ] == ':'  ) {
                        sscanf ( psem + 3, "%d", &display.height );
                    }
                    else {
                        sscanf ( psem + 2, "%d", &display.height_mm );
                    }
                    break;
                    
                case 'p': // pn
                    if ( psem [1] == 'n' && psem [2] == ':' ) {
                        areaNameLen = (unsigned int)strlen ( psem + 3 );
                        if ( areaNameLen >= MAX_NAMEPROPERTY ) {
                            CWarnArgID ( "EvaluateDeviceConfig: Area name [%s] is too long.", psem + 3 );
                            areaNameLen = 0;
                        }
                        else {
                            memcpy ( areaName, psem + 3, areaNameLen );
                            areaName [areaNameLen] = 0;
                        }
                    }
                    else if ( psem [ 1 ] == 'l' && psem [ 2 ] == ':' ) {
                        int dplatform = 0;
                        if ( sscanf_s ( psem + 3, "%i", &dplatform ) == 1 )
							platform = (Platforms_t)dplatform;
                    }
                    break;
                    
                case 'w': // w | wp
                    if ( psem [ 1 ] == 'p' && psem [ 2 ] == ':' ) {
                        sscanf ( psem + 3, "%d", &display.width );
                    }
                    else {
                        sscanf ( psem + 2, "%d", &display.width_mm );
                    }
                    break;
            }
            
            psem = strtok_s ( NULL, ";", &context );
        }
        
        if ( clientID != deviceID ) {
            // We are connecting to the desired client! Quit the game.. say bye bye instead of helo
            return false;
        }
        
        if ( !*appName || !appNameLen || !*areaName || !areaNameLen )
            return false;
        
        if ( deviceAreaName && deviceAppName )
            return true;
        
        if ( strncmp ( env->areaName, areaName, MAX_NAMEPROPERTY + 1 ) || strncmp ( env->appName, appName, MAX_NAMEPROPERTY + 1 ) ) {
            CreateCopyString ( areaName, &deviceAreaName );
            CreateCopyString ( appName, &deviceAppName );
        }
        return true;
    }
    
#ifdef _WIN32
#pragma warning( pop )
#endif
    
    
    void DevicePlatform::TuneReceiveBuffer ( int sock )
    {        
#ifdef DISABLE_BUFFER_TUNING_FOR_NAT
        if ( behindNAT )
            return;
#endif
        
        int value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
        if ( env->useNativeResolution )
            value = PORTAL_SOCKET_BUFFER_SIZE_NATRES;
        
        if ( setsockopt ( sock, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(value) ) < 0 ) {
            CErrArgID ( "TuneReceiveBuffer: Failed to set socket receive buffer size! [%i bytes]", value );
            LogSocketError ();
        }
    }
    
    
    void DevicePlatform::ProccessPortalProvided ( int portalID, PortalStreamType_t streamType )
    {
        CVerbArgID ( "ProccessPortalProvided: type [%i]", streamType );
        
        DeviceBase::ProccessPortalProvided ( portalID, streamType );
    }
    
    
    void DevicePlatform::ProccessPortalStartAck ( int portalID )
    {
        CVerbID ( "ProccessPortalStartAck" );
        
        DeviceBase::ProccessPortalStartAck ( portalID );
    }
    
    
    void DevicePlatform::ProccessPortalPauseAck ( int portalID )
    {
        CVerbID ( "ProccessPortalPauseAck" );
        
        DeviceBase::ProccessPortalPauseAck ( portalID );
    }
    
    
    void DevicePlatform::ProccessPortalStopAck ( int portalID )
    {
        CVerbID ( "ProccessPortalStopAck" );
        
        DeviceBase::ProccessPortalStopAck ( portalID );
    }
    
    
    void DevicePlatform::HandleOptionsMessage ( unsigned short payloadType, char * payload )
    {
        CVerbID ( "HandleOptionsMessage" );
        
        if (payloadType == MSG_OPT_CONTACT_DIRECT_SET) {
            hasPhysicalContact = (*((unsigned int *)payload) != 0);
            
            CLogArgID ( "HandlePortalMessage: Device / Display contact has changed [%d]!", hasPhysicalContact );
            
            
            onEnvironsNotifier1 ( env, deviceID, deviceAreaName, deviceAppName, NOTIFY_CONTACT_DIRECT_CHANGED, hasPhysicalContact );
        }
        else
            DeviceBase::HandleOptionsMessage ( payloadType, payload );
    }
    
    
    void DevicePlatform::OnInteractListenerClosed ()
    {
        CVerbID ( "OnInteractListenerClosed" );
        
        DeviceBase::OnInteractListenerClosed ();
    }
    
    
    void DevicePlatform::OnConnectionEstablished ()
    {
        CVerbID ( "OnConnectionEstablished" );
        
        DeviceBase::OnConnectionEstablished ();
    }
    
    
    void DevicePlatform::OnUdpConnectionEstablished ()
    {
        CVerbID ( "OnUdpConnectionEstablished" );
        
        DeviceBase::OnUdpConnectionEstablished();
    }
    
    
    void DevicePlatform::PerformEnvironsTouch ( Input * pack )
    {
    }
    
    
    void DevicePlatform::UpdatePosition ( int x, int y )
    {
    }
    
    
    void DevicePlatform::UpdatePortalsize ( int width, int height )
    {
    }



} // <-- namespace environs


#endif /// end->DISPLAYDEVICE

