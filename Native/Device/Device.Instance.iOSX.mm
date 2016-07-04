/**
 * DeviceInfo object
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
//#	define DEBUGVERBList
#endif

#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"
#include <unistd.h>

#include "Environs.h"
#include "Interop/Threads.h"
#include "Message.Instance.h"
#include "Core/Array.List.h"
#import "Environs.iOSX.h"
#import "Device.List.iOSX.h"
#import "Portal.Instance.iOSX.IAPI.h"
#import "Message.Instance.iOSX.IAPI.h"
#import "File.Instance.iOSX.IAPI.h"
#import "Device.Instance.iOSX.IAPI.h"

#import "Environs.iOSX.h"

using namespace environs::API;

#define	CLASS_NAME 	"Device.Instance.iOSX . ."

//#define USE_TEMP_SP_IN_OBJC_PROPERTY

using namespace std;

NSLock              *   portalInstancesLock = [[NSLock alloc] init];
NSLock              *   msgInstancesLock    = [[NSLock alloc] init];
NSLock              *   fileInstancesLock   = [[NSLock alloc] init];

#ifdef DEBUGVERBList
int                     deviceInstanceIDs   = 0;
#endif

MessageInstance *   allocMessageInstance  ( const sp ( environs::lib::MessageInstance ) &item );
FileInstance *      allocFileInstance     ( const sp ( environs::lib::FileInstance ) &item );

extern Environs             *   instancesPlt [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
extern ::PortalInstance * GetPortalInstanceInterface ( environs::PortalInstance * portal );
#endif

namespace environs
{
    namespace lib
    {
        class DeviceInstanceProxy
        {
        public:
            DeviceInstanceProxy () : inst ( 0 ) {};

            environs::lib::DeviceInstance      *    inst;
            
            Call_t async ( ) { return inst->async; }
            void async ( Call_t value ) { inst->async = value; }
            int hEnvirons () { return inst->hEnvirons; }
            
            bool GetAllowConnect ( ) { return inst->GetAllowConnect (); }
            void SetAllowConnect ( bool value ) { inst->SetAllowConnect ( value ); }
            
            short connectProgress ( ) { return inst->connectProgress; }
            
            int directStatus ( ) { return inst->directStatus (); }
            
            environs::DeviceInfo * info ( ) { return inst->info_; }
            
            static const sp ( environs::lib::PortalInstance ) & GetPortalSP ( environs::lib::PortalInstance * portal )
            {
                return portal->myself;
            }

            pthread_mutex_t * GetPortalLock () {
                return &inst->devicePortalsLock;
            }
        };
        
        
        class DIObserver : public IIDeviceObserver
        {
        public:
            /** Constructor */
            DIObserver ( ::DeviceInstance * p ) : parent ( p ) { OnDeviceChangedInternal_ = true; };
            
            ~DIObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::DeviceInstance * parent;
            
            void OnDeviceChanged ( const sp ( environs::DeviceInstance ) &device, environs::DeviceInfoFlag_t changedFlags )  { OnDeviceChanged_ = false; };
            
            void OnDeviceChangedInterface ( environs::DeviceInstance * device, environs::DeviceInfoFlag_t changedFlags ) { OnDeviceChangedInterface_ = false; };
            
            void OnDeviceChangedInternal ( environs::DeviceInfoFlag_t changedFlags );

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            void OnPortalRequestOrProvided ( const sp ( environs::PortalInstance ) &portal ) { OnEnvironsPortalRequestOrProvided_ = false; };


            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            void OnPortalRequestOrProvidedInterface ( environs::PortalInstance * portal ) { OnEnvironsPortalRequestOrProvidedInterface_ = false; };


            void OnPortalRequestOrProvidedBase ( environs::PortalInstance * portal );
#endif
        };
        
        void DIObserver::OnDeviceChangedInternal ( environs::DeviceInfoFlag_t changedFlags )
        {
            if ( parent )
                [parent NotifyObservers:changedFlags];
        };


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
        void DIObserver::OnPortalRequestOrProvidedBase ( environs::PortalInstance * portal )
        {
            if ( !parent )
                return;
            ::PortalInstance * p = GetPortalInstanceInterface ( portal );
            if ( p ) {
                [parent OnPortalRequestOrProvided:p];
            }
        }
#endif
        
        
        class DIDataObserver : public IIDataObserver
        {
        public:
            /** Constructor */
            DIDataObserver ( ::DeviceInstance * p ) : parent ( p ) { OnDataInternal_ = true; };
            
            ~DIDataObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::DeviceInstance * parent;
            
            void OnData ( const sp ( environs::FileInstance ) &fileData, environs::FileInfoFlag_t changedFlags ) { OnData_ = false; };
            
            void OnDataInterface ( environs::FileInstance * fileData, environs::FileInfoFlag_t changedFlags ) { OnDataInterface_ = false; };
            
            void OnDataInternal ( const sp ( environs::lib::FileInstance ) &fileData, environs::FileInfoFlag_t changedFlags );
        }; 
        
        void DIDataObserver::OnDataInternal ( const sp ( environs::lib::FileInstance ) &fileData, environs::FileInfoFlag_t changedFlags )
        {
            if ( !parent )
                return;
            ::FileInstance * item = allocFileInstance ( fileData );
            if ( item )
                [parent NotifyObserversForData:item Flags:changedFlags];
        }
        
        
        class DISensorObserver : public IISensorObserver
        {
        public:
            /** Constructor */
            DISensorObserver ( ::DeviceInstance * p ) : parent ( p ) {};
            
            ~DISensorObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::DeviceInstance * parent;
            
            void OnSensorData ( environs::SensorFrame * sensorFrame );
        };
        
        void DISensorObserver::OnSensorData ( environs::SensorFrame * sensorFrame )
        {
            if ( parent )
                [parent NotifyObserversForSensorData:sensorFrame];
        }
        
        
        class DIMessageObserver : public IIMessageObserver
        {
        public:
            /** Constructor */
            DIMessageObserver ( ::DeviceInstance * p ) : parent ( p ) { OnMessageInternal_ = true; };
            
            ~DIMessageObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::DeviceInstance * parent;
            
            void OnMessage ( const sp ( environs::MessageInstance ) &msg, environs::MessageInfoFlag_t changedFlags )  { OnMessage_ = false; };
            
            void OnMessageInterface ( environs::MessageInstance * msg, environs::MessageInfoFlag_t changedFlags ) { OnMessageInterface_ = false; };
            
            void OnMessageInternal ( const sp ( environs::lib::MessageInstance ) &msgInstance, environs::MessageInfoFlag_t changedFlags );
        };
        
        void DIMessageObserver::OnMessageInternal ( const sp ( environs::lib::MessageInstance ) &msgInstance, environs::MessageInfoFlag_t changedFlags )
        {
            if ( !parent )
                return;
            ::MessageInstance * item = allocMessageInstance ( msgInstance );
            if ( item ) {
                [parent NotifyObserversForMessage:item Flags:changedFlags];

#ifdef OSX_USE_MANUAL_REF_COUNT
                [item release];
#endif
            }
        }
        
        
        void DeviceInstance::PlatformDispose ()
        {
            CVerbVerb ( "PlatformDispose" );

            LockAcquireVA ( devicePortalsLock, "PlatformDispose" );

            void * p = 0;
            
            if ( platformKeep ) {
                p = platformKeep;
                platformKeep = nil;
            }

            if ( p ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
                ::DeviceInstance * fNil = (::DeviceInstance *)p;
                [fNil release];
#else
                ::DeviceInstance * fNil = (__bridge_transfer ::DeviceInstance *)p;
                if ( fNil )
                    fNil = nil;
#endif
            }

            LockReleaseVA ( devicePortalsLock, "PlatformDispose" );
        }
    }
}


@interface DeviceInstance ()
{
    environs::lib::DeviceInstanceProxy      p;
    
    sp ( environs::lib::DeviceInstance )    instSP;
    sp ( environs::lib::DIObserver )        observer;
    sp ( environs::lib::DIMessageObserver ) observerForMessages;
    sp ( environs::lib::DIDataObserver )    observerForData;
    sp ( environs::lib::DISensorObserver )  observerForSensors;
    
    NSLock              *   lock;
    
    /** A collection of observers that observe this device instance for changes and events. */
    NSMutableArray      *   observers;
    
    /** A collection of observers that observe this device instance for changes and events. */
    NSMutableArray      *   observersForMessages;
    
    /** A collection of observers that observe this device instance for changes and events. */
    NSMutableArray      *   observersForData;
    
    /** A collection of observers that observe this device instance for changes and events. */
    NSMutableArray      *   observersForSensorData;
    
#ifdef DEBUGVERBList
    int                     deviceInstanceID;
#endif
}
@end



@implementation DeviceInstance (internal)


- (environs::DeviceInfo *) info
{
    return p.inst->info();
}


- (bool) SetInst : ( sp ( environs::lib::DeviceInstance ) & ) deviceSP
{
    instSP = deviceSP;
    
    if ( !instSP )
        return false;
    
    p.inst = instSP.get ();

    pthread_mutex_t * instLock = p.GetPortalLock ();

    pthread_mutex_lock ( instLock );

    if ( p.inst->platformRef || p.inst->platformKeep )
    {
        instSP.reset();
        p.inst = 0;
        
        pthread_mutex_unlock ( instLock );
        return false;
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    [self retain];
    p.inst->platformRef = (void *) self;
    p.inst->platformKeep = p.inst->platformRef;
#else
    p.inst->platformRef = (__bridge_retained void *) self;
    p.inst->platformKeep = p.inst->platformRef;
#endif
    pthread_mutex_unlock ( instLock );

    CVerbArg ( "SetInst: objID [ %i ] platformRef [ 0x%X ]", p.inst->objID (), p.inst->platformRef );
    
    observer = make_shared < environs::lib::DIObserver > ( self );
    if ( !observer )
        return false;
    
    observerForMessages = make_shared < environs::lib::DIMessageObserver > ( self );
    if ( !observerForMessages )
        return false;
    
    observerForData = make_shared < environs::lib::DIDataObserver > ( self );
    if ( !observerForData )
        return false;
    
    observerForSensors = make_shared < environs::lib::DISensorObserver > ( self );
    if ( !observerForSensors )
        return false;
    
    return true;
}


- (environs::lib::DeviceInstance *) GetInstancePtr
{
    return p.inst;
}


- (void) NotifyObservers:(environs::DeviceInfoFlag_t) changedFlags
{
    [self NotifyObservers:self changed:changedFlags];
}


- (void) NotifyObservers:(id)sender changed:(environs::DeviceInfoFlag_t) changedFlags
{
    NSArray * obss = nil;

    try {
        obss = [observers copy];
        
        if ( [obss count] > 0 )
        {
            for ( int i=0; i < [obss count]; i++ )
            {
                id<DeviceObserver> obs = [obss objectAtIndex:i];
                if ( obs ) {
                    try {
                        [obs OnDeviceChanged:sender Flags:changedFlags];
                    }
                    catch(...) {
                        CErr ( "NotifyObservers: Exception!" );
                    }
                }
            }
        }
    }
    catch ( ... ) {
        CErr ( "NotifyObservers: Outer Exception!" );
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    if ( obss != nil ) {
        [obss release];
        return;
    }
#endif
}


- (void) NotifyObserversForMessage:(MessageInstance *) message Flags:(environs::MessageInfoFlag_t) changedFlags
{
    if ( !message ) return;

    NSArray * obss = nil;
    try {
        obss = [observersForMessages copy];
        
        if ( [obss count] > 0 )
        {
            for ( int i=0; i < [obss count]; i++ )
            {
                id<MessageObserver> obs = [obss objectAtIndex:i];
                if ( obs ) {
                    try {
                        [obs OnMessage:message Flags:changedFlags];
                    }
                    catch(...) {
                        CErr ( "NotifyObserversForMessage: Exception!" );
                    }
                }
            }
        }
    }
    catch ( ... ) {
        CErr ( "NotifyObserversForMessage: Outer Exception!" );
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    if ( obss != nil ) {
        [obss release];
        return;
    }
#endif
}


- (void) NotifyObserversForData:(FileInstance *) fileInst Flags:(environs::FileInfoFlag_t) changedFlags
{
    NSArray * obss = nil;

    try {
        obss = [observersForData copy];
        
        if ( [obss count] > 0 )
        {
            for ( int i=0; i < [obss count]; i++ )
            {
                id<DataObserver> obs = [obss objectAtIndex:i];
                if ( obs ) {
                    try {
                        [obs OnData:fileInst Flags:changedFlags];
                    }
                    catch ( ... ) {
                        CErr ( "NotifyObserversForData: Exception!" );
                    }
                }
            }
        }
    }
    catch ( ... ) {
        CErr ( "NotifyObserversForData: Outer Exception!" );
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    if ( obss != nil ) {
        [obss release];
        return;
    }
#endif
}


- (void) NotifyObserversForSensorData:(environs::SensorFrame *) sensorFrame
{
    NSArray * obss = nil;
    try {
        obss = [observersForSensorData copy];
        
        if ( [obss count] > 0 )
        {
            sensorFrame->devicePlatform = (__bridge void *) self;

            for ( int i=0; i < [obss count]; i++ )
            {
                id<SensorObserver> obs = [obss objectAtIndex:i];
                if ( obs ) {
                    try {
                        [obs OnSensorData:sensorFrame];
                    }
                    catch ( ... ) {
                        CErr ( "NotifyObserversForSensorData: Exception!" );
                    }
                }
            }
        }
    }
    catch ( ... ) {
        CErr ( "NotifyObserversForSensorData: Outer Exception!" );
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    if ( obss != nil ) {
        [obss release];
        return;
    }
#endif
}


#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance
{

    for ( int i=0; i < [observers count]; i++ )
    {
        id<DeviceObserver> obs = [observers objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnPortalRequestOrProvided:portalInstance];
            }
            catch(...) {
                CErr ( "OnPortalRequestOrProvided: Exception!" );
            }
        }
    }
}
#endif

@end



@implementation DeviceInstance


/** Application defined contexts for arbitrary use. */
- (int)     appContext0 { return p.inst->appContext0; }
- (void)    setAppContext0 : (int) value { p.inst->appContext0 = value; }

#ifdef OSX_USE_MANUAL_REF_COUNT

- (id)      appContext1 { id obj = (id)p.inst->appContext1; if ( !obj ) return nil; [obj retain]; return obj; }
- (void)    setAppContext1 : (id) value { id obj = (id)p.inst->appContext1; if (obj)[obj release]; p.inst->appContext1 = (void *) value; if (value) [value retain]; }

- (id)      appContext2 { id obj = (id)p.inst->appContext2; if ( !obj ) return nil; [obj retain]; return obj; }
- (void)    setAppContext2 : (id) value { id obj = (id)p.inst->appContext2; if (obj)[obj release]; p.inst->appContext2 = (void *) value; if (value) [value retain]; }

- (id)      appContext3 { id obj = (id)p.inst->appContext3; if ( !obj ) return nil; [obj retain]; return obj; }
- (void)    setAppContext3 : (id) value { id obj = (id)p.inst->appContext3; if (obj)[obj release]; p.inst->appContext3 = (void *) value; if (value) [value retain]; }

#else

- (id)      appContext1 { return (__bridge id)p.inst->appContext1; }
- (void)    setAppContext1 : (id) value { p.inst->appContext1 = (__bridge void *) value; }

- (id)      appContext2 { return (__bridge id)p.inst->appContext2; }
- (void)    setAppContext2 : (id) value { p.inst->appContext2 = (__bridge void *) value; }

- (id)      appContext3 { return (__bridge id)p.inst->appContext3; }
- (void)    setAppContext3 : (id) value { p.inst->appContext3 = (__bridge void *) value; }

#endif

#ifdef USE_TEMP_SP_IN_OBJC_PROPERTY
- (bool) 	disposed { sp ( environs::lib::DeviceInstance ) tempSP = self->instSP; if ( tempSP ) return p.inst->disposed (); return true; }
#else
- (bool) 	disposed { return p.inst->disposed (); }
#endif

/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
- (environs::Call_t) 	async { return p.async (); }

- (void)    setAsync: (environs::Call_t) value { p.async ( value ); }

/** An identifier that is unique for this object. */
- (long) 	objID { return p.inst->objID (); }


/** Allow connects by this device. The default value of for this property can be changed by chaning the static class value DeviceInstance.allowConnectDefault.
 Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
- (bool) 	GetAllowConnect { return p.GetAllowConnect (); }

- (void)    SetAllowConnect: (bool) value { p.SetAllowConnect ( value ); }


/** The device ID within the environment */
- (int) 	deviceID { return p.info()->deviceID; }

/** The ID that is used by the native layer to identify this particular device within the environment.
 A value of 0 means that this device is not connected and therefore not actively managed. */
- (int) 	nativeID { return p.info()->nativeID; }

/** IP from device. The IP address reported by the device which it has read from network configuration. */
- (unsigned int) 	ip { return p.info()->ip; }

/** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
 * This address could be different from IP due to NAT, Router, Gateways behind the device.
 */
- (unsigned int) 	ipe { return p.info()->ipe; }  // The external IP or the IP resolved from the socket address

/** The tcp port on which the device listens for device connections. */
- (unsigned short) 	tcpPort { return p.info()->tcpPort; }

/** The udp port on which the device listens for device connections. */
- (unsigned short) 	udpPort { return p.info()->udpPort; }

/** The number of alive updates noticed by the mediator layer since its appearance within the application environment. */
- (unsigned int) 	updates { return p.info()->updates; }

/** A value that describes the device platform. */
- (int) 	platform { return p.info()->platform; }

/** sourceType is a value of environs::DeviceSourceType and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
- (environs::DeviceSourceType_t) 	sourceType { return p.inst->sourceType (); }

- (bool) 	isObserverReady { return p.inst->isObserverReady (); }
- (bool) 	isMessageObserverReady { return p.inst->isMessageObserverReady (); }
- (bool) 	isDataObserverReady { return p.inst->isDataObserverReady (); }
- (bool) 	isSensorObserverReady { return p.inst->isSensorObserverReady (); }

- (bool) 	unavailable { return p.info()->unavailable; }

/** isConnected is true if the device is currently in the connected state. */
- (bool) 	isConnected { return p.info()->isConnected; }
- (char) 	internalUpdates { return p.info()->internalUpdates; }

- (bool) 	isLocationNode { return p.inst->isLocationNode (); }

/** Used internally by native layer. */
- (char) 	flags { return p.info()->flags; }

/** The device name. */
- (const char *) 	deviceName { return p.info()->deviceName; }

/** The area name of the appliction environment. */
- (const char *) 	areaName { return p.info()->areaName; }

/** The application name of the appliction environment. */
- (const char *) 	appName { return p.info()->appName; }

- (short) 	connectProgress { return p.connectProgress (); }

/** A DeviceDisplay structure that describes the device's display properties. */
- (environs::DeviceDisplay) 	display { return p.inst->GetDisplayProps (); }

- (NSString *) 	toString { return [ [NSString alloc ] initWithUTF8String : p.inst->toString ().c_str () ]; }


- (id) init
{
    CVerb ( "init" );
    
    self = [super init];
    if ( self ) {
        p.inst              = 0;
        
        lock                    = [[NSLock alloc] init];
        
        observers               = [[NSMutableArray alloc] init];
        observersForMessages    = [[NSMutableArray alloc] init];
        observersForData        = [[NSMutableArray alloc] init];
        observersForSensorData  = [[NSMutableArray alloc] init];

#ifdef DEBUGVERBList
        deviceInstanceID    = __sync_add_and_fetch ( &deviceInstanceIDs, 1 );
        
        CLogArg ( "init: [ %i ]", deviceInstanceID );
#endif        
    }
    return self;
}


- (void) dealloc
{
    CVerb ( "dealloc" );
    
#ifdef DEBUGVERBList
    CLogArg ( "dealloc: [ %i ]", deviceInstanceID );
#endif
    
    if ( [observers count] > 0 ) {
        p.inst->RemoveObserver ( (environs::DeviceObserver *) observer.get () );
    }
    
    if ( [observersForData count] > 0 ) {
        p.inst->RemoveObserverForData ( (environs::DataObserver *) observerForData.get () );
    }
    
    if ( [observersForMessages count] > 0 ) {
        p.inst->RemoveObserverForMessages ( (environs::MessageObserver *) observerForMessages.get () );
    }
    
    if ( [observersForSensorData count] > 0 ) {
        p.inst->RemoveObserverForSensors ( (environs::SensorObserver *) observerForSensors.get () );
    }

#ifdef OSX_USE_MANUAL_REF_COUNT
    [lock release];

    [observers release];
    [observersForMessages release];
    [observersForData release];
    [observersForSensorData release];

    id obj = (id) p.inst->appContext1;
    if ( obj ) {
        [obj release];
    }
    obj = (id) p.inst->appContext2;
    if ( obj ) {
        [obj release];
    }
    obj = (id) p.inst->appContext3;
    if ( obj ) {
        [obj release];
    }
#endif
    observers               = nil;
    observersForMessages    = nil;
    observersForData        = nil;
    observersForSensorData  = nil;

    if ( p.inst ) {
        p.inst->platformRef = 0;
        p.inst = 0;
    }
    
    instSP = 0;

    IOSX_SUPER_DEALLOC ();
}


/**
 * Add an observer (DeviceObserver) that notifies about device property changes.
 *
 * @param observer A DeviceObserver
 */
- (void) AddObserver:(id<DeviceObserver>) obs
{
    CVerb ( "AddObserver" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observers containsObject:obs] ) {
        CVerbVerb ( "AddObserver: Adding observer" );
        [observers addObject:obs];
        
        if ( [observers count] == 1 ) {
            CVerbVerb ( "AddObserver: Adding bridge observer" );
            p.inst->AddObserver ( (environs::DeviceObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}


/**
 * Remove an observer (DeviceObserver) that was added before.
 *
 * @param observer A DeviceObserver
 */
- (void) RemoveObserver:(id<DeviceObserver>) obs
{
    CVerb ( "RemoveObserver" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( [observers containsObject:obs] ) {
        CVerbVerb ( "RemoveObserver: Removing observer" );
        [observers removeObject:obs];
        
        if ( [observers count] <= 0 ) {
            CVerbVerb ( "RemoveObserver: Removing bridge observer" );
            p.inst->RemoveObserver ( (environs::DeviceObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}


/**
 * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
 *
 * @param observer A DataObserver
 */
- (void) AddObserverForData:(id<DataObserver>) obs
{
    CVerb ( "AddObserverForData" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observersForData containsObject:obs] ) {
        [observersForData addObject:obs];
        
        if ( [observersForData count] == 1 ) {
            p.inst->AddObserverForData ( (environs::DataObserver *) observerForData.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Remove an observer (SensorObserver) that was added before.
 *
 * @param observer A DataObserver
 */
- (void) RemoveObserverForData:(id<SensorObserver>) obs
{
    CVerb ( "RemoveObserverForData" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( [observersForData containsObject:obs] ) {
        [observersForData removeObject:obs];
        
        if ( [observersForData count] <= 0 ) {
            p.inst->RemoveObserverForData ( (environs::DataObserver *) observerForData.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
 *
 * @param observer A DataObserver
 */
- (void) AddObserverForSensors:(id<SensorObserver>) obs
{
    CVerb ( "AddObserverForSensors" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observersForSensorData containsObject:obs] ) {
        [observersForSensorData addObject:obs];
        
        if ( [observersForSensorData count] == 1 ) {
            p.inst->AddObserverForSensors ( (environs::SensorObserver *) observerForSensors.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Remove an observer (DataObserver) that was added before.
 *
 * @param observer A DataObserver
 */
- (void) RemoveObserverForSensors:(id<SensorObserver>) obs
{
    CVerb ( "RemoveObserverForSensors" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( [observersForSensorData containsObject:obs] ) {
        [observersForSensorData removeObject:obs];
        
        if ( [observersForSensorData count] <= 0 ) {
            p.inst->RemoveObserverForSensors ( (environs::SensorObserver *) observerForSensors.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
 *
 * @param observer A MessageObserver
 */
- (void) AddObserverForMessages:(id<MessageObserver>) obs
{
    CVerb ( "AddObserverForMessages" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observersForMessages containsObject:obs] ) {
        CVerbVerb ( "AddObserverForMessages: Adding observer" );
        [observersForMessages addObject:obs];
        
        if ( [observersForMessages count] == 1 ) {
            CVerbVerb ( "AddObserverForMessages: Adding bridge observer" );
            p.inst->AddObserverForMessages ( (environs::MessageObserver *) observerForMessages.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Remove an observer (MessageObserver) that was added before.
 *
 * @param observer A MessageObserver
 */
- (void) RemoveObserverForMessages:(id<MessageObserver>) obs
{
    CVerb ( "RemoveObserverForMessages" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( [observersForMessages containsObject:obs] ) {
        CVerbVerb ( "RemoveObserverForMessages: Removing observer" );
        [observersForMessages removeObject:obs];
        
        if ( [observersForMessages count] <= 0 ) {
            CVerbVerb ( "RemoveObserverForMessages: Removing bridge observer" );
            p.inst->RemoveObserverForMessages ( (environs::MessageObserver *) observerForMessages.get () );
        }
    }
    
    [lock unlock];
}

/**
 * Notify to all observers (DeviceObserver) that the appContext has changed.
 *
 * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
 */
- (void) NotifyAppContextChanged:(int) customFlags
{
    if ( customFlags == 0 )
        customFlags = DEVICE_INFO_ATTR_APP_CONTEXT;
    
    [self NotifyObservers:(environs::DeviceInfoFlag_t)customFlags];
}


- (NSString *) ips
{
    return [ [NSString alloc ] initWithUTF8String : p.inst->ips ().c_str () ];
}

- (NSString *) ipes
{
    return [ [NSString alloc ] initWithUTF8String : p.inst->ipes ().c_str () ];
}

- (bool) EqualsAppEnv:(environs::DeviceInfo *) equalTo
{
    return p.inst->EqualsAppEnv ( equalTo );
}

- (bool) EqualsAppEnv:(const char *)areaName App:(const char *)appName
{
    return p.inst->EqualsAppEnv ( areaName, appName );
}

- (bool) LowerThanAppEnv:(environs::DeviceInfo *) compareTo
{
    return p.inst->LowerThanAppEnv ( compareTo );
}

- (bool) LowerThanAppEnv:(const char *)areaName App:(const char *)appName
{
    return p.inst->LowerThanAppEnv ( areaName, appName );
}

- (bool) EqualsID:(DeviceInstance *) equalTo
{
    return p.inst->EqualsID ( equalTo->p.inst );
}

- (bool) EqualsID:(int) deviceID Area:(const char *)areaName App:(const char *)appName
{
    return p.inst->EqualsID ( deviceID, areaName, appName );
}


+ (NSString *) DeviceTypeString:(environs::DeviceInfo *) info
{
    return @"";
}

- (NSString *) DeviceTypeString
{
    return [ [NSString alloc ] initWithUTF8String : p.inst->DeviceTypeString () ];
}

- (const char *) GetBroadcastString:(bool) fullText
{
    return p.inst->GetBroadcastString ( fullText );
}

/**
 * Connect to this device asynchronously.
 *
 * @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
 *
 * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					true: A new connection has been triggered and is in progress
 */
- (bool) Connect
{
    return p.inst->Connect ();
}

/**
 * Connect to this device using the given mode.
 *
 * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					true: A new connection has been triggered and is in progress
 */
- (bool) Connect:(environs::Call_t) Environs_CALL_
{
    return p.inst->Connect ( Environs_CALL_ );
}

/**
 * Disconnect the device with the given id and a particular application environment.
 *
 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
 */
- (bool) Disconnect
{
    return p.inst->Disconnect ();
}

/**
 * Disconnect the device using the given mode with the given id and a particular application environment.
 *
 * @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
 *
 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
 */
- (bool) Disconnect:(environs::Call_t) Environs_CALL_
{
    return p.inst->Disconnect ( Environs_CALL_ );
}

/**
 * Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
 *
 * @return PortalInstance-object
 */
- (environs::DeviceDisplay) GetDisplayProps
{
    return p.inst->GetDisplayProps ();
}

/**
 * Load the file that is assigned to the fileID received by deviceID into an byte array.
 * MEMORY must bee freed by callee
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @param size        An int pointer, that receives the size of the returned buffer.*
 
 * @return byte-array
 */
- (NSData *) GetFile:(int) fileID Size:(int *)size
{
    environs::DeviceInfo * info = p.inst->info();
    
    return [instancesPlt[p.hEnvirons()] GetFile:(info->nativeID > 0) nativeID:info->nativeID deviceID:info->deviceID areaName:info->areaName appName:info->appName fileID:fileID size:size ];
}

/**
 * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @return absolutePath
 */
- (const char *) GetFilePath:(int) fileID
{
    return p.inst->GetFilePath ( fileID );
}


PortalInstance * GetPortalInstanceInterface ( environs::PortalInstance * portal )
{
    if ( !portal ) return nil;
    
    sp ( environs::lib::PortalInstance ) p = environs::lib::DeviceInstanceProxy::GetPortalSP ( (environs::lib::PortalInstance *) portal );
    
    return GetPortalInstance ( p );
}


PortalInstance * GetPortalInstance ( sp ( environs::lib::PortalInstance ) portal )
{
    if ( !portal )
        return nil;
    
    PortalInstance * p = nil;
    
    [portalInstancesLock lock];
    
    if ( portal->platformRef )
        p = (__bridge ::PortalInstance *) portal->platformRef;
    
    while ( !p ) {
        p = [[PortalInstance alloc] init];
        if ( !p )
            break;
        
        if ( ![p SetInst:portal] ) {
            p = nil;
        }
        break;
    }
    
    [portalInstancesLock unlock];
    
    return p;
}

/**
 * Creates a portal instance that requests a portal.
 *
 * @param 	portalType	        Project name of the application environment
 *
 * @return 	PortalInstance-object
 */
- (PortalInstance *) PortalRequest:(environs::PortalType_t) portalType
{
    return GetPortalInstance ( p.inst->PortalRequest ( portalType ) );
}

/**
 * Creates a portal instance that provides a portal.
 *
 * @param 	portalType	        Project name of the application environment
 *
 * @return 	PortalInstance-object
 */
- (PortalInstance *) PortalProvide:(environs::PortalType_t) portalType
{
    return GetPortalInstance ( p.inst->PortalProvide ( portalType ) );
}


/**
 * Query the first PortalInstance that manages an outgoing portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetOutgoing
{
    return GetPortalInstance ( p.inst->PortalGetOutgoing () );
}

/**
 * Query the first PortalInstance that manages an incoming portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetIncoming
{
    return GetPortalInstance ( p.inst->PortalGetIncoming () );
}

/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param filePath      The path to the file to be send.
 * @return success
 */
- (bool) SendFile:(int)fileID Desc:(const char *)fileDescriptor Path:(const char *)filePath
{
    return p.inst->SendFile ( fileID, fileDescriptor, filePath );
}

/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param buffer        A buffer to be send.
 * @param bytesToSend number of bytes in the buffer to send
 * @return success
 */
- (bool) SendBuffer:(int)fileID Desc:(const char *)fileDescriptor Data:(unsigned char *)buffer Size:(int)bytesToSend
{
    return p.inst->SendBuffer ( fileID, fileDescriptor, buffer, bytesToSend );
}


/**
 * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
 * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
 * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
 *
 * @return FileInstance
 */
- (FileInstance *) ReceiveBuffer
{
    sp ( environs::lib::FileInstance ) fileInst = p.inst->ReceiveBuffer ();
    if ( !fileInst )
        return nil;
    
    return allocFileInstance ( fileInst );
}



/**
 * Send a string message to a device through one of the following ways.&nbsp;
 * If a connection with the destination device has been established, then use that connection.
 * If the destination device is not already connected, then distinguish the following cases:
 * (1) If the destination is within the same network, then try establishing a direct connection.
 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
 * 		a STUNT connection with the latest connection details that are available.
 *
 * On successful transmission, Environs returns true if the devices already had an active connection,
 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
 * a registered EnvironsObserver instance.
 *
 * @param async			(environs::Call::NoWait) Perform asynchronous. (environs::Call::Wait) Non-async means that this call blocks until the call finished.
 * @param message       A message to send.
 * @param length       Length of the message to send.
 * @return success
 */
- (bool) SendMessage:(environs::Call_t) async message:(const char *)message length:(int)length
{
    return p.inst->SendMessage ( async, message, length );
}

/**
 * Send a string message to a device through one of the following ways.&nbsp;
 * If a connection with the destination device has been established, then use that connection.
 * If the destination device is not already connected, then distinguish the following cases:
 * (1) If the destination is within the same network, then try establishing a direct connection.
 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
 * 		a STUNT connection with the latest connection details that are available.
 *
 * On successful transmission, Environs returns true if the devices already had an active connection,
 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
 * a registered EnvironsObserver instance.
 *
 * @param message       A message to be send.
 * @return success
 */
- (bool) SendMessage:(NSString *)message
{
    return p.inst->SendMessage ( [message UTF8String] );
}

/**
 * Send a string message to a device through one of the following ways.&nbsp;
 * If a connection with the destination device has been established, then use that connection.
 * If the destination device is not already connected, then distinguish the following cases:
 * (1) If the destination is within the same network, then try establishing a direct connection.
 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
 * 		a STUNT connection with the latest connection details that are available.
 *
 * On successful transmission, Environs returns true if the devices already had an active connection,
 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
 * a registered EnvironsObserver instance.
 *
 * @param async			(environs::Call::NoWait) Perform asynchronous. (environs::Call::Wait) Non-async means that this call blocks until the call finished.
 * @param message       A message to be send.
 * @return success
 */
- (bool) SendMessage:(environs::Call_t) async message:(NSString *)message
{
    return p.inst->SendMessage ( async, [message UTF8String] );
}



/**
 * Receives a message send using SendMessage by the DeviceInstance.
 * This call blocks until a new message has been received or until the DeviceInstance gets disposed.
 * Messages that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
 *
 * @return MessageInstance
 */
- (MessageInstance *) Receive
{
    sp ( environs::lib::MessageInstance ) msgInst = p.inst->Receive ();
    if ( !msgInst )
        return nil;
    
    return allocMessageInstance ( msgInst );
}


/**
 * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param buffer        A buffer to be send.
 * @param offset        A user-customizable id that identifies the file to be send.
 * @param bytesToSend number of bytes in the buffer to send
 * @return success
 */
- (bool) SendDataUdp : (char *)buffer offset:(int)offset bytesToSend:(int)bytesToSend
{
    return p.inst->SendDataUdp ( (unsigned char *)buffer, offset, bytesToSend );
}


/**
 * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
 * @param buffer        A buffer to be send.
 * @param offset        A user-customizable id that identifies the file to be send.
 * @param bytesToSend number of bytes in the buffer to send
 * @return success
 */
- (bool) SendDataUdp : (environs::Call_t) async buffer:(char *)buffer offset:(int)offset bytesToSend:(int)bytesToSend
{
    return p.inst->SendDataUdp ( async, (unsigned char *)buffer, offset, bytesToSend );
}


/**
 * Receives a data buffer send using SendDataUdp by the DeviceInstance.
 * This call blocks until new data has been received or until the DeviceInstance gets disposed.
 *
 * @return byte buffer
 */
- (UCharArray_ptr) ReceiveData
{
    return p.inst->ReceiveData ();
}


/**
 * Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
 *
 * @return absolutePath
 */
- (NSString *) GetStoragePath
{
    return [ [NSString alloc ] initWithUTF8String : p.inst->GetStoragePath () ];
}

/**
 * Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
 *
 */
- (void) ClearMessages
{
    p.inst->ClearMessages ();
}

/**
 * Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
 *
 */
- (void) ClearStorage
{
    p.inst->ClearStorage ();
}


FileInstance * allocFileInstance ( const sp ( environs::lib::FileInstance ) &item )
{
    CVerbVerb ( "allocFileInstance" );

    environs::lib::FileInstance * f = item.get ();
    if ( !f )
        return nil;
    
    FileInstance * resItem = nil;
    
    [fileInstancesLock lock];
    do
    {
        if ( f->platformRef ) {
            resItem = (__bridge FileInstance *) f->platformRef;

#ifdef OSX_USE_MANUAL_REF_COUNT
            [resItem retain];
#endif
            break;
        }

        if ( f->disposed () )
            break;

        resItem = [[FileInstance alloc] init];
        if ( !resItem )
            break;
        
        if ( ![resItem SetInst:item] ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
            [resItem release];
#endif
            resItem = nil;
            break;
        }
    }
    while ( 0 );
    
    [fileInstancesLock unlock];
    
    return resItem;
}


/**
 * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
 *
 * @return Collection with objects of type FileInstance with the fileID as the key.
 */
- (NSMutableArray *) GetFiles
{
    smsp ( int, environs::lib::FileInstance ) list = p.inst->GetFiles ();
    if ( !list )
        return nil;

    size_t size = list->size ();
    if ( size <= 0 )
        return nil;
    
    NSMutableArray * ar = [[NSMutableArray alloc] init];
    while ( ar )
    {
        map < int, sp ( environs::lib::FileInstance ) >::iterator it;
        const map < int, sp ( environs::lib::FileInstance ) >::iterator & end = list->end();
        
        for ( it = list->begin(); it != end; ++it )
        {
            const sp ( environs::lib::FileInstance ) &item = it->second;
            
            if ( !item ) continue;
            
            if ( item->platformRef ) {
                [ ar addObject : (__bridge FileInstance *) item->platformRef ];
            }
            else {
                FileInstance * item1 = allocFileInstance ( item );
                if ( item1 ) {
                    [ ar addObject : item1 ];

#ifdef OSX_USE_MANUAL_REF_COUNT
                    [item1 release];
#endif
                }
            }
        }
        break;
    };
    
    return ar;
}


/**
 * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
 *
 * @return Collection with objects of type FileInstance with the fileID as the key.
 */
- (NSMutableArray *) GetFilesInStorage
{
    smsp ( int, environs::lib::FileInstance ) list = p.inst->GetFilesInStorage ();
    if ( !list )
        return nil;
    
    
    size_t size = list->size ();
    if ( size <= 0 )
        return nil;
    
    NSMutableArray * ar = [[NSMutableArray alloc] init];
    while ( ar )
    {
        map < int, sp ( environs::lib::FileInstance ) >::iterator it;
        
        for ( it = list->begin(); it != list->end(); ++it )
        {
            const sp ( environs::lib::FileInstance ) &item = it->second;
            
            if ( !item ) continue;
            
            if ( item->platformRef ) {
                [ ar addObject : (__bridge FileInstance *) item->platformRef ];
            }
            else {
                FileInstance * item1 = allocFileInstance ( item );
                if ( item1 ) {
                    [ ar addObject : item1 ];

#ifdef OSX_USE_MANUAL_REF_COUNT
                    [item1 release];
#endif
                }
            }
        }
        break;
    };
    
    return ar;
}


/**
 * Clear cached MessageInstance and FileInstance objects for this DeviceInstance.
 *
 */
- (void) DisposeStorageCache
{
    p.inst->DisposeStorageCache ();
}


MessageInstance * allocMessageInstance ( const sp ( environs::lib::MessageInstance ) &item )
{
    CVerbVerb ( "allocMessageInstance" );

    environs::lib::MessageInstance * msg = item.get ();
    if ( !msg )
        return nil;
    
    MessageInstance * resItem = nil;
    
    [msgInstancesLock lock];
    do
    {
        if ( msg->platformRef ) {
            resItem = (__bridge MessageInstance *) msg->platformRef;

#ifdef OSX_USE_MANUAL_REF_COUNT
            [resItem retain];
#endif
            break;
        }

        if ( msg->disposed () )
            break;
        
        resItem = [[MessageInstance alloc] init]; // +1 retain
        if ( !resItem )
            break;
        
        if ( ![resItem SetInst:item] ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
            [resItem release];
#endif
            resItem = nil;
            break;
        }
    }
    while ( 0 );

    [msgInstancesLock unlock];
    
    return resItem;
}


/**
 * Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
 *
 * @return Collection with objects of type MessageInstance
 */
- (NSMutableArray *) GetMessages
{
    svsp ( environs::lib::MessageInstance ) list = p.inst->GetMessages ();
    if ( !list )
        return nil;
    
    if ( !p.inst->StorageLock ( true ) )
        return nil;
    
    @autoreleasepool
    {
        NSMutableArray * ar = nil;
        
        size_t size = list->size ();
        if ( size <= 0 ) {
            goto Finish;
        }
        
        ar = [[NSMutableArray alloc] init];
        while ( ar )
        {
            for ( int i = 0; i < size; ++i ) {
                sp ( environs::lib::MessageInstance ) item = list->at ( i );
                if ( !item )
                    continue;
                
                if ( item->platformRef ) {
                    [ ar addObject : (__bridge MessageInstance *) item->platformRef ];
                }
                else {
                    MessageInstance * item1 = allocMessageInstance ( item );
                    if ( item1 ) {
                        [ ar addObject : item1 ];

#ifdef OSX_USE_MANUAL_REF_COUNT
                        [item1 release];
#endif
                    }
                }
            }
            break;
        };
        
    Finish:
        p.inst->StorageLock ( false );
        
        return ar;
    }
}


/**
 * Get a list with all messages that this device has received (and sent) from the storage.
 *
 * @return Collection with objects of type MessageInstance
 */
- (NSMutableArray *) GetMessagesInStorage
{
    svsp ( environs::lib::MessageInstance ) list = p.inst->GetMessagesInStorage ();
    if ( !list )
        return nil;
    
    if ( !p.inst->StorageLock ( true ) )
        return nil;
    
    @autoreleasepool
    {
        NSMutableArray * ar = nil;
        
        size_t size = list->size ();
        if ( size <= 0 ) {
            goto Finish;
        }
        
        ar = [[NSMutableArray alloc] init];
        while ( ar )
        {
            for ( int i = 0; i < size; ++i ) {
                sp ( environs::lib::MessageInstance ) item = list->at ( i );
                if ( !item )
                    continue;
                
                if ( item->platformRef ) {
                    [ ar addObject : (__bridge MessageInstance *) item->platformRef ];
                }
                else {
                    MessageInstance * item1 = allocMessageInstance ( item );
                    if ( item1 ) {
                        [ ar addObject : item1 ];

#ifdef OSX_USE_MANUAL_REF_COUNT
                        [item1 release];
#endif
                    }
                }
            }
            break;
        };
        
    Finish:
        p.inst->StorageLock ( false );
        
        return ar;
    }
}


/**
 * Enable sending of sensor events to this DeviceInstance.
 * Events are send if the device is connected and stopped if the device is disconnected.
 *
 * @param type A value of type environs::SensorType_t.
 * @param enable true = enable, false = disable.
 *
 * @return success true = enabled, false = failed.
 */
- (bool) SetSensorEventSending :(environs::SensorType_t) type enable:(bool) enable
{
    return p.inst->SetSensorEventSending ( type, enable );
}

@end











