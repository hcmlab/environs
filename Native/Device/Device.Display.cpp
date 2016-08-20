/**
 * DeviceBase for stationary display devices
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

#ifdef DISPLAYDEVICE

#include "Environs.Obj.h"
#include "Environs.Utils.h"
#include "Device/Device.Controller.h"
#include "Device/Devices.h"
#include "Core/Callbacks.h"
#include "Portal.Info.Base.h"
#include <cmath>
#include <errno.h>

using namespace environs::lib;
using namespace environs::API;

// The TAG for prepending in log messages
#define CLASS_NAME	"Device.Platform. . . . ."

//#define DEBUG_PORTAL_SEND
#define TEST_MAX_PORTAL_RESOLUTION
//#define ADAPT_ORIENTATION
//#define HIDE_SECOND_LEVEL_BACKUPS

#define PI 3.14159265

using namespace std;


namespace environs
{
	extern void CreateCopyString ( const char * src, char ** dest );

	// Initialization of static variables
	bool	DevicePlatform::inject_touch			= false;
	int		DevicePlatform::app_pid					= 0;

    
    DevicePlatform::DevicePlatform ( int deviceID )
    {
        this->deviceID = deviceID;
        
        Construct ();
    }
    
    
    DevicePlatform::DevicePlatform ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr )
    {
        this->deviceID = deviceID;
        
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
    
    
    void DevicePlatform::Construct ()
    {
        CVerbID ( "Construct..." );
        
#ifdef ENABLE_RECOGNIZERS_OBJECT_USAGE
        touchRecognizers	= 0;
#endif
        
        allocated			= false;
        
        frameNumber			= 0;
		seqNumberOrientation = 0;
        
        orientationLast		= 90.0f;
        deviceAzimut		= 0.0f;
        deviceAzimutLast	= 0.0f;
        
		inputs				= 0;
		inputsTemp			= 0;
        inputsCountCurrent	= 0;
    }
    
    
    DevicePlatform::~DevicePlatform()
    {
        CVerbID ( "Destruct" );
        
        Release ();
        
        if ( inputs ) {
            CVerbID ( "Destruct: Deleting inputs." );
            delete inputs;
            inputs = 0;
        }
        if ( inputsTemp ) {
            CVerbID ( "Destruct: Deleting inputsTemp." );
            delete inputsTemp;
            inputsTemp = 0;
        }

        if ( allocated ) {
            allocated = false;
            
            LockDisposeA ( inputsMutex );
        }
        
        CVerbID ( "Destructed." );
    }
    
    
    bool DevicePlatform::Init ( const sp ( Instance ) &envObj, const char * areaName, const char * appName )
    {
        CVerbID ( "Init" );
        
        if ( !allocated )
        {
            inputs = new vector<Input *> ();
            if ( !inputs )
                return false;
            
            inputsTemp = new vector<Input *> ();
            if ( !inputsTemp )
                return false;
            
            if ( !LockInitA ( inputsMutex ) )
                return false;
            
            allocated = true;
        }
        
        return DeviceBase::Init ( envObj, areaName, appName );
    }
    
    
    void DevicePlatform::TuneReceiveBuffer ( int sock )
    {        
#ifdef DISABLE_BUFFER_TUNING_FOR_NAT
        if ( behindNAT )
            return;
#endif
        
        // Tune send buffer for native resolution portal (on sending devices)
        int value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
        if ( env->useNativeResolution )
            value = PORTAL_SOCKET_BUFFER_SIZE_NATRES;
        
        if ( setsockopt ( sock, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(value) ) < 0 ) {
            CErrArgID ( "TuneReceiveBuffer: Failed to set socket send buffer size! [%i bytes] on tcp socket", value );
            LogSocketError ();
        }
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
		Instance * env = instances [ hInst ];
		if ( !env )
			return 0;

        DevicePlatform * device = (DevicePlatform *) GetDevice ( env, deviceID, areaName, appName );
        if ( device ) {
            if ( device->deviceStatus < DeviceStatus::Connected ) {
                
                if ( !device->IsConnectingValid () ) {                    
                    UnlockDevice ( device );
                    device = 0;
                }
            }
            
            if ( device ) {
                device->UpdatePosition ( 0, x, y, angle );
                device->orientationLast = angle;
                device->SetDirectContactStatus ( true );
                
                UnlockDevice ( device );
                return 1;
            }
        }
        
        if ( DeviceBase::ConnectToDevice ( hInst, Environs_CALL_, deviceID, areaName, appName ) == 0 )
            return 0;
        
        device = (DevicePlatform *) GetDevice ( env, deviceID, areaName, appName );
        if ( device ) {
            device->UpdatePosition ( 0, x, y, angle );
            device->SetDirectContactStatus ( true );
            
            UnlockDevice ( device );
            return 2;
        }
        else {
            RemoveDevice ( env, deviceID, areaName, appName );
            CErrArgID ( "DeviceDetected: failed to initialize controller for device [%i].", deviceID );
        }
        
        return 0;
    }
    
    
    
    bool DevicePlatform::IsIP ( unsigned int ip )
    {
        return (ip == interactAddr.sin_addr.s_addr );
    }
    
    
    void DevicePlatform::OnPreConnectionEstablished ( )
    {
        CVerbID ( "OnPreConnectionEstablished" );
        
        if ( inject_touch ) {
            Zero ( localAddr );
            localAddr.sin_family      = PF_INET;
            localAddr.sin_port        = htons(5911);
            localAddr.sin_addr.s_addr = inet_addr ( "127.0.0.1" );
        }	
    }
    
    
    bool DevicePlatform::EvaluateDeviceConfig ( char * msg )
    {
        CVerbID ( "EvaluateDeviceConfig on stationary device" );
        
        // id:int;ip:ip-addr;wp:width-pixel;hp:height-pixel;w:width;h:height;tr:h4<EOF>
        
        int clientID = 0;
        
        char * context = 0;
        char * psem = strtok_s ( msg, ";", &context );
        
        char appName [ MAX_NAMEPROPERTY ];
        unsigned int appNameLen = 0;
        *appName = 0;
        
        char areaName [ MAX_NAMEPROPERTY ];
        unsigned int areaNameLen = 0;
        *areaName = 0;
        
        while ( psem != NULL )
        {
            //printf ("%s\n",psem);
            switch ( psem [0] ) {
                case 'a': // an
                    if ( psem [1] == 'n' && psem [2] == ':' ) {
                        appNameLen = (unsigned int) strlen ( psem + 3 );
                        if ( appNameLen >= sizeof ( appName ) ) {
                            CWarnArgID ( "EvaluateDeviceConfig: Application name [%s] is too long.", psem + 3 );
                        }
                        else {
                            memcpy ( appName, psem + 3, appNameLen );
                            appName [appNameLen] = 0;
                        }
                    }
                    break;
                    
                case 'd': // do
                    if ( psem [1] == 'o' && psem [2] == ':' ) {
                        if ( 1 != sscanf_s ( psem + 3, "%i", &display.orientation ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for display orientation." );
                        }
                    }
                    break;
                    
                case 'i': // id | ip
                    if ( psem [ 1 ] == 'd' && psem [ 2 ] == ':'  ) {
                        if ( 1 != sscanf_s ( psem + 3, "%i", &clientID ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for deviceID." );
                        }
                    }
                    break;
                    
                case 'h': // h | hp
                    if ( psem [1] == 'p' && psem [2] == ':' ) {
                        if ( 1 != sscanf_s ( psem + 3, "%d", &display.height ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for height." );
                        }
                    }
                    else {
                        if ( 1 != sscanf_s ( psem + 2, "%d", &display.height_mm ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for height_mm." );
                        }
                        else {
                            height_coverage = (int) ((display.height_mm * native.display.height) / native.display.height_mm);
                            
                            // Ensure that dimension is divisible by 2 (requirement for h264); Nevertheless, one pixel more or less is mostly not preceiveable by humans
                            if ( height_coverage % 2 )
                                height_coverage++;
                            portalInfoOff.height = height_coverage;
                        }
                    }
                    break;
                    
                case 'r': // rsize
                    if ( psem [1] == 's' && psem [2] == 'i' && psem [3] == 'z' && psem [4] == 'e' && psem [5] == ':' ) {
                        if ( 1 != sscanf_s ( psem + 6, "%d", &receiveBufferSize ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for receive buffer size." );
                        }
                    }
                    break;
                    
                case 'p': // pt portal tcp
                    if ( psem [1] == 't' && psem [2] == ':' ) {
                        int enable = 0;
                        if ( sscanf_s ( psem + 3, "%i", &enable ) == 1 )
                            streamOptions.streamOverCom = (enable == 1 ? true : false);
                    }
                    else if ( psem [1] == 'l' && psem [2] == ':' ) {
                        int dplatform = 0;
                        if ( sscanf_s ( psem + 3, "%i", &dplatform ) == 1 )
							platform = (Platforms_t) dplatform;
                    }
                    else if ( psem [1] == 'r' && psem [2] == ':' ) {
                        int enable = 0;
                        if ( 1 != sscanf_s ( psem + 3, "%i", &enable ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for native resolution usage." );
                        }
                        else
                            streamOptions.useNativeResolution = ((enable == 0) ? false : true);
                    }
                    else if ( psem [1] == 'n' && psem [2] == ':' ) {
                        areaNameLen = (unsigned int) strlen ( psem + 3 );
                        if ( areaNameLen >= sizeof ( areaName ) ) {
                            CWarnArgID ( "EvaluateDeviceConfig: Area name [%s] is too long.", psem + 3 );
                            areaNameLen = 0;
                        }
                        else {
                            memcpy ( areaName, psem + 3, areaNameLen );
                            areaName [areaNameLen] = 0;
                        }
                    }
                    break;
                    
                case 't':
                    if ( psem [ 1 ] == 'r') { // tr - transport : h4 = h264
                        if ( psem [2] == ':' && psem [3] == 'h' && psem [4] == '4' ) {
                            if ( streamOptions.useStream ) {
                                int streamProtocolVersion = 0;
                                if ( 1 != sscanf_s ( psem + 5, "%d", &streamProtocolVersion ) ) {
                                    CWarnID ( "EvaluateDeviceConfig: failed parsing for supported stream protocol version." );
                                }
                                else if ( streamProtocolVersion == 0 )
                                    streamOptions.useStream = false;
                            }
                        }
                    }
                    break;
                    
                case 'w': // w | wp
                    if ( psem [1] == 'p' && psem [2] == ':' ) {
                        if ( 1 != sscanf_s ( psem + 3, "%d", &display.width ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for width." );
                        }
                    }
                    else {
                        if ( 1 != sscanf_s ( psem + 2, "%d", &display.width_mm ) ) {
                            CWarnID ( "EvaluateDeviceConfig: failed parsing for width_mm." );
                        }
                        else {
                            width_coverage = (int) ((display.width_mm * native.display.width) / native.display.width_mm);
                            
                            // Ensure that dimension is divisible by 2 (requirement for h264); Nevertheless, one pixel more or less is mostly not preceiveable by humans
                            if ( width_coverage % 2 )
                                width_coverage++;
                            portalInfoOff.width = width_coverage;
                        }
                    }
                    break;
            }
            
            psem = strtok_s ( NULL, ";", &context );
        }
        
        if ( clientID != deviceID ) {
            // We are not connecting to the desired device! Quit the game.. say bye bye instead of helo
            return false;
        }
        
        if ( !*appName || !appNameLen || !*areaName || !areaNameLen )
            return false;
        
        if ( deviceAreaName && deviceAppName )
            return true;
        
        if ( strncmp ( env->areaName, areaName, sizeof ( env->areaName ) ) || strncmp ( env->appName, appName, sizeof ( env->appName ) ) ) {
            CreateCopyString ( areaName, &deviceAreaName );
            CreateCopyString ( appName, &deviceAppName );
        }
        
        return true;
    }
    
    
    void DevicePlatform::OnInteractListenerClosed ( )
    {
        CVerbID ( "OnInteractListenerClosed" );
        
        DeviceBase::OnInteractListenerClosed ( );
        
        // Set kernel status if we are the last device that disconnects
        if ( !GetConnectedDevicesManagedCount ( ) )
            SetEnvironsState ( env, environs::Status::Started );
    }
    
    
    void DevicePlatform::ClearTouches ( bool execTouch )
    {
        CVerb ( "ClearTouches" );
        
		if ( allocated && !LockAcquireA ( inputsMutex, "ClearTouches" ) )
			return;
        
        if ( inputs && inputs->size () > 0 ) {
            CVerb ( "ClearTouches: Clearing list of remaining touches..." );
            
            for ( unsigned int i=0; i < inputs->size (); i++ ) {
                Input * in = (*inputs) [i];
                in->pack.raw.state = INPUT_STATE_DROP;
                
                if ( execTouch ) {
                    if ( inject_touch ) {
                        InjectTouch ( in );
                    }
                    else {
                        onEnvironsInput ( env, nativeID, in );
                    }
                }
                delete in;
            }
            inputs->clear ();
        }
        
		if ( allocated )
			LockReleaseVA ( inputsMutex, "ClearTouches" );
    }
    
    
    void DevicePlatform::DisposePlatform ()
    {
		LockAcquireVA ( spLock, "DisposePlatform" );

        activityStatus |= DEVICE_ACTIVITY_PLATFORM_DISPOSED;        
        
        CVerbID ( "DisposePlatform: setting deviceStatus to Deleteable" );
        deviceStatus = DeviceStatus::Deleteable;
        
#ifdef ENABLE_RECOGNIZERS_OBJECT_USAGE
        if ( touchRecognizers ) {
            delete touchRecognizers;
            touchRecognizers = 0;
        }
#endif
		LockReleaseVA ( spLock, "DisposePlatform" );
        
        ClearTouches ( false );
    }
    
    
    void DevicePlatform::Release ()
    {
        DisposePlatform ();
        
        CloseListeners ();
    }
    
    
    void DevicePlatform::CreatePortalGeneratorPlatform ( int portalIndex )
    {
        CVerbID ( "CreatePortalGeneratorPlatform" );
        
        portalGenerators [portalIndex] = new PortalGenerator ();
		portalGenerators [ portalIndex ]->env = env;
    }

    
    
    void DevicePlatform::CreatePortalReceiverPlatform ( int portalIndex )
    {
        CVerbID ( "CreatePortalReceiverPlatform: Not implemented." );
    }
    
    
    
    void DevicePlatform::HandleOptionsMessage ( unsigned short payloadType, char * payload )
    {
        CVerbID ( "HandleOptionsMessage" );
        
        unsigned int values [12];
        int value1;
        
        int type = payloadType & MSG_OPTION_TYPE;
        switch ( type )
        {
            case MSG_OPTION_SET:
                switch ( payloadType )
            {
                case MSG_OPT_TRANSP_TCP_PORTAL_SET:
                    CVerb ( "HandleOptionsMessage: set tcp portal" );
                    
                    value1 = *((int *) payload);
                    CLogArgID ( "HandleOptionsMessage: tcp portal = %i", value1 );
                    
                    streamOptions.streamOverCom = (value1 == 0) ? false : true;
                    break;
                    
                default:
                    DeviceBase::HandleOptionsMessage ( payloadType, payload );
            }
                break;
                
            case MSG_OPTION_GET:
            {
                // fill response with request id
                values [0] = *((unsigned int *) payload);
                
                switch ( payloadType )
                {
                    case MSG_OPT_TRANSP_TCP_PORTAL_GET:
                        CVerbID ( "HandleOptionsMessage: get portal tcp" );
                        
                        values [1] = streamOptions.streamOverCom ? 1 : 0;
                        
                        SendBuffer ( true, MSG_TYPE_OPTIONS_RESPONSE, 0, 0, 0, values, sizeof(unsigned int)* 2 );
                        break;
                        
                    case MSG_OPT_CONTACT_DIRECT_GET:
                        CVerbID ( "HandleOptionsMessage: get direct contact value" );
                        
                        values [1] = (unsigned int)GetDirectContactStatus ( );
                        
                        SendBuffer ( true, MSG_TYPE_OPTIONS_RESPONSE, 0, 0, 0, values, sizeof(unsigned int)* 2 );
                        break;
                        
                    default:
                        DeviceBase::HandleOptionsMessage ( payloadType, payload );
                }
            }
                break;
        }
    }
    
    
    void DevicePlatform::TranslateToDisplayCoord ( RenderDimensions * dims, environs::lib::InputPackRaw * pack )
    {
        //CLogArg ( "TranslateToDisplayCoord: x/y [%i/%i]", x, y );
        // 0 = left
        // 90 = top
        // 180 = right
		pack->x = (int) (((double) dims->width_cap * (double) pack->x) / (double) display.width);
		pack->x += dims->left;
        
		pack->y = (int) (((double) dims->height_cap * (double) pack->y) / (double) display.height);
		pack->y += dims->top;
        
        int x_center = dims->left + (dims->width_cap / 2);
        int y_center = dims->top + (dims->height_cap / 2);
        
        double rad = PI * (dims->orientation - 90) / 180;
        
        double co = cos ( rad );
        double si = sin ( rad );
		int xD = ( pack->x - x_center );
		int yD = ( pack->y - y_center );
        
		pack->x = (int)(co * xD - si * yD + x_center);
		pack->y = (int)(si * xD + co * yD + y_center);
    }
    
    
    void DevicePlatform::HandleTouchPacket ( char * buffer, int length )
    {
        if ( deviceStatus != DeviceStatus::Connected )
            return;
   
		if ( sizeof ( InputFrame ) > ( unsigned ) length )
			return; // received buffer is less than expected

		GetStructPointerFromBuffer ( InputFrame, frame, buffer );
        
        if ( frame->frameNumber <= this->frameNumber )
            return; // Drop frameSequence of an old or duplicated frame
        
        this->frameNumber = frame->frameNumber; // We have a new frame
        
        unsigned int	touchCount		= (unsigned int)frame->count;
        
        if ( !inputsCountCurrent && !touchCount )
            return; // Nothing to do
        
        bool			extTouches		= frame->version >= 3;

#ifndef USE_INPUT_PACK_STDINT_CPP
        unsigned int	touchPakSize    = extTouches ? INPUTPACK_V3_SIZE : INPUTPACK_V2_SIZE;
        unsigned int	touchPakIntSize = extTouches ? INPUTPACK_V3_INT_SIZE : INPUTPACK_V2_INT_SIZE;
#else
        unsigned int	touchPakSize	= extTouches ? INPUTPACK_V3_SIZE : INPUTPACK_V2_SIZE;
		unsigned int	touchPakIntSize = touchPakSize;
#endif

        if ( sizeof ( InputFrame ) + (touchCount * touchPakSize) > (unsigned)length )
            return; // received buffer is less than expected
        
        //char					eventOffset		= 0;
        
        RenderDimensions	*	dims			= 0;
        InputPackRec		*	recoContainer	= 0;
        unsigned int			recoIndex		= 0;
        unsigned int			recoCount		= 0;
        bool					locked			= false;
        
        PortalDevice		*	portal			= HoldPortalDeviceID ( portalGeneratorsDeviceInput );
        PortalGenerator		*	gen				= 0;
        if ( portal ) {
            gen = portal->generator;
            if ( gen ) {
                gen->GetDimensionsLock ( dims, recoContainer, recoIndex );
                
#ifdef ENVIRONS_ENABLE_RECOGNIZER_MANAGER
                WARNING ( "Needs implementation" );
#else
                //if ( gen->recognizedGesture )
                //    eventOffset = 10;
#endif
            }
            if ( !gen || !dims ) {
                ReleasePortalDevice ( portal );
                portal = 0;
            }
        }
        
        // Get recognizer container, if required
        unsigned int        pos				= 0;
        char *              pData;
        size_t              inputsCount;
        vector<Input *>	*	ins;
        vector<Input *>	*	insTemp;
        
        if ( pthread_mutex_lock ( &inputsMutex ) ) {
            CErr ( "HandleTouchPacketV4: Failed to lock inputsMutex." );
            goto Finish;
        }
        locked = true;
        
        ins = inputs;
        if ( !ins )
            goto Finish;
        
        insTemp = this->inputsTemp;
        if ( !insTemp )
            goto Finish;
        
        insTemp->clear ();
        inputsCount = ins->size ();
        
        for ( ; pos < inputsCount; pos++ )
            ((*ins) [pos])->pack.raw.valid = false;
        
        pData = buffer + INPUTPACK_START_IN_INPUTFRAME;
        
        for ( unsigned int i = 0; i < touchCount; i++ )
        {
			GetStructPointerFromBuffer ( InputPackRaw, pack, pData );

            if ( dims )
                TranslateToDisplayCoord ( dims, pack );
            
            int		id		= pack->id;
            Input * touch	= 0;
            
            for ( pos = 0; pos < inputsCount; pos++ ) {
                Input * t = (*ins) [pos];
                if ( t->pack.raw.id == id ) {
                    touch = t;
                    break;
                }
            }
            
            if ( touch )
            {
                if ( pack->state == INPUT_STATE_DROP ) // drop
                {
                    touch->Update ( pack, touchPakIntSize );
                    touch->pack.raw.valid = false;
                    
                    if ( !inject_touch )
                        onEnvironsInput ( env, nativeID, touch );
                    else
                        InjectTouch ( touch );
                    
                    if ( recoContainer && recoCount < MAX_TOUCH_VISUALS ) {
                        memcpy ( recoContainer + recoCount, touch, sizeof ( Input ) );
                        recoCount++;
                    }
                }
                else
                {
                    insTemp->push_back ( touch );
                    
                    if ( touch->Update ( pack, touchPakIntSize ) )
                    {
                        touch->pack.raw.state = INPUT_STATE_CHANGE;
                        
                        if ( !inject_touch )
                            onEnvironsInput ( env, nativeID, touch );
                        else
                            InjectTouch ( touch );
                    }
                    else
                        touch->pack.raw.state = INPUT_STATE_NOCHANGE;
                    
                    touch->pack.raw.valid = true;
                    
                    if ( recoContainer && recoCount < MAX_TOUCH_VISUALS ) {
                        memcpy ( recoContainer + recoCount, touch, sizeof ( Input ) );
                        recoCount++;
                    }
                }
            }
            else
            {
                if ( pack->state != INPUT_STATE_DROP ) // If the state is drop, then a remote recognizer has taken over this touch contact and dont want us to consider that touch entity
                {
                    // We have a new touch
                    touch = new Input ( pack, touchPakIntSize );
                    if ( !touch ) {
                        goto Finish;
                    }
                    
                    if ( !inject_touch )
                        onEnvironsInput ( env, nativeID, touch );
                    else
                        InjectTouch ( touch );
                    insTemp->push_back ( touch );
                    
                    if ( recoContainer && recoCount < MAX_TOUCH_VISUALS ) {
                        memcpy ( recoContainer + recoCount, touch, sizeof ( Input ) );
                        recoCount++;
                    }
                }
            }
            
            pData += touchPakSize;
        }
        
        //CLogArg ( "Touch: %d -> %d", inputsTemp->size(), inputs->size () );
        
        inputsCount = ins->size ();
        if ( inputsCount ) {
            for ( pos = 0; pos < inputsCount; pos++ ) {
                Input * t = (*ins) [pos];
                if ( !t->pack.raw.valid )
                {
                    if ( t->pack.raw.state != INPUT_STATE_DROP ) {
                        t->pack.raw.state = INPUT_STATE_DROP;
                        if ( !inject_touch ) {
                            onEnvironsInput ( env, nativeID, t );
                        }
                        else
                            InjectTouch ( t );
                    }
                    
                    if ( recoContainer && recoCount < MAX_TOUCH_VISUALS ) {
                        memcpy ( recoContainer + recoCount, t, sizeof ( InputPackRec ) );
                        recoCount++;
                    }
                    delete t;
                }
            }
            ins->clear ();
            
            // Submit a flush if the new frame is empty
            /*if ( !inputsTemp->size () ) {
             Input flush;
             flush.pack.uniqueID = -1;
             flush.pack.raw.state = INPUT_STATE_DROP;
             
             onEnvironsInput ( nativeID, &flush );
             }
             */
        }
        
        inputsCountCurrent = (int)insTemp->size ();
        
        // Swap the touch containers
        this->inputs = insTemp;
        this->inputsTemp = ins;
        
    Finish:
        if ( portal ) {
            gen->ReleaseDimensionsLock ( recoCount, recoIndex );
            ReleasePortalDevice ( portal );
        }
        
        if ( locked && pthread_mutex_unlock ( &inputsMutex ) ) {
            CErr ( "HandleTouchPacketV1: Failed to unlock inputsMutex." );
        }
    }
    
    
    void DevicePlatform::PerformEnvironsTouch ( Input * in )
    {
        if ( inject_touch )
            InjectTouch ( in );
        else
            onEnvironsInput ( env, nativeID, in );
    }


} /* namespace environs */

#endif

