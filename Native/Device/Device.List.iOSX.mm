/**
 * DeviceLists iOSX
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
//#   define DEBUGVERBList
#endif

#include "Environs.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Core/Array.List.h"

#import "Environs.Observer.h"
#import "Device.Instance.iOSX.IAPI.h"
#import "Device.List.iOSX.IAPI.h"

using namespace std;
using namespace environs::API;

#define	CLASS_NAME 	"Device.List.iOSX . . . ."



NSArray * allocDeviceList ( const svsp ( environs::lib::DeviceInstance ) &list );

namespace environs
{
    namespace lib
    {
        class DeviceListProxy
        {
        public:
            environs::lib::DeviceList      *    inst;
            
            void DisposeLists () { inst->DisposeLists (); };
            
            environs::Instance * GetEnvInstance () { return inst->env; };
            
            static long ObjID ( const sp ( environs::lib::DeviceList ) inst ) { if ( !inst ) return 0; return inst->objID_; };

            static pthread_mutex_t * GetDeviceLock ( environs::lib::DeviceInstance * d )
            {
                return &d->devicePortalsLock;
            }
        };
        
        class DLObserver : public environs::lib::IIListObserver
        {
        public:
            /** Constructor */
            DLObserver () { OnListChangedInternal_ = true; };
            
            ~DLObserver () {}

#ifdef OSX_USE_MANUAL_REF_COUNT
            ::DeviceList * parent;
#else
            __weak ::DeviceList * parent;
#endif
            void OnListChanged ( const sp ( environs::DeviceInstanceList ) &vanished, const sp ( environs::DeviceInstanceList ) &appeared ) { OnListChanged_ = false; };
            
            void OnListChangedInterface ( environs::DeviceInstanceList * vanished, environs::DeviceInstanceList * appeared ) { OnListChangedInterface_ = false; };
            
            void OnListChangedInternal ( const svsp ( environs::lib::DeviceInstance ) &vanished, const svsp ( environs::lib::DeviceInstance ) &appeared );
        };
        
        void DLObserver::OnListChangedInternal ( const svsp ( environs::lib::DeviceInstance ) &vanished, const svsp ( environs::lib::DeviceInstance ) &appeared )
        {
#ifdef OSX_USE_MANUAL_REF_COUNT
            NSArray * v = nil;
            if ( vanished )
                v = allocDeviceList ( vanished );

            NSArray * a = nil;
            if ( appeared )
                a = allocDeviceList ( appeared );

            if ( v != nil || a != nil )
                [parent NotifyListObservers:v appeared:a];

            if ( v )
                [v release];
            if ( a )
                [a release];
#else
            // This fixed the ARC issue
            @autoreleasepool
            {
                NSArray * v = nil;
                if ( vanished )
                    v = allocDeviceList ( vanished );
                NSArray * a = nil;
                if ( appeared )
                    a = allocDeviceList ( appeared );
                
                if ( v != nil || a != nil )
                    [parent NotifyListObservers:v appeared:a];
            }
#endif
        }
        
        
        void DeviceList::PlatformDispose ()
        {
            CVerbVerb ( "PlatformDispose" );
            /*
            void * p = 0;
            
            if ( platformRef ) {
                MutexLockV ( &envObj->listLock, "PlatformDispose" );
                
                p = platformRef;
                platformRef = 0;
                
                MutexUnlockV ( &envObj->listLock, "PlatformDispose" );
            }
            
            if ( p ) {
                ::DeviceList * fNil = (__bridge_transfer ::DeviceList *)p;
                fNil = nil;
            }
            */
        }
    }
}



@interface DeviceList ()
{
    int                                 hEnvirons;
    environs::Instance              *   env;
    environs::lib::DeviceListProxy      p;
    sp ( environs::lib::DeviceList )    instSP;
    sp ( environs::lib::DLObserver )    observer;
    
    NSLock                          *   lock;
    NSMutableArray                  *   observers;
}
@end



@implementation DeviceList (internal)


- (bool) SetInst : ( sp ( environs::lib::DeviceList ) ) inst_
{
    CVerbVerbArg ( "SetInst [%i]", p.ObjID ( inst_ ) );
    
    if ( !inst_ )
        return false;
    
    observer = make_shared < environs::lib::DLObserver > ();
    if ( !observer )
        return false;
    observer->parent = self;
    
    self->instSP = inst_;
    
    p.inst = inst_.get ();
    
    //p.inst->platformRef = (__bridge_retained void *) self;
    p.inst->platformRef = (__bridge void *) self;
    
    env = p.GetEnvInstance ();
    hEnvirons = env->hEnvirons;
    
    return true;
}


- (void) NotifyListObservers:(NSArray *)vanished appeared:(NSArray *)appeared
{
    onEnvironsNotifier1 ( env, 0, NOTIFY_MEDIATOR_DEVICELISTS_CHANGED, SOURCE_NATIVE );
    
    CListLog ( "NotifyListObservers: Devicelist has changed.");
    
    if ( [observers count] <= 0 ) return;
    
    for ( int i=0; i < [observers count]; i++ )
    {
        id<ListObserver> obs = [observers objectAtIndex:i];
        try {
            [obs OnListChanged:vanished appeared:appeared];
        }
        catch(...) {
            CErr ( "NotifyListObservers: Exception!" );
        }
    }
    
    /*NSArray * vanishedDevices = nil;
    NSArray * appearedDevices = nil;
    
    if ( vanished != nil && [vanished count] > 0 ) {
        vanishedDevices = [vanished copy];
    }
    
    if ( appeared != nil && [appeared count] > 0 ) {
        appearedDevices = [appeared copy];
    }
     
    
    for ( int i=0; i < [observers count]; i++ )
    {
        id<ListObserver> obs = [observers objectAtIndex:i];
        try {
            [obs OnListChanged:vanishedDevices appeared:appearedDevices];
        }
        catch(...) {
            [observers removeObject:obs];
            i--;
        }
    }
     */
}


/**
 * Dispose all device lists.
 * This method is intended to be used by the platform layer when the framework shuts down.
 */
- (void) DisposeLists
{
    CVerbVerb ( "DisposeLists" );
    
    p.DisposeLists ();
}

@end


@implementation DeviceList
{
}

NSLock * listLock         = [[NSLock alloc] init];

- (id) init
{
    CVerb ( "init" );
    
    self = [super init];
    
    if ( self ) {
        lock         = [[NSLock alloc] init];
        observers    = [[NSMutableArray alloc] init];
        
        if ( !lock || !observers )
            return nil;
        
    }
    return self;
}


- (void) dealloc
{
#ifdef OSX_USE_MANUAL_REF_COUNT
    [super dealloc];
#endif

    CVerbArg ( "dealloc [%i]", environs::lib::DeviceListProxy::ObjID ( instSP ) );
    
    if ( [observers count] > 0 )
        p.inst->RemoveObserver ( (environs::ListObserver *) observer.get () );
    
    if ( p.inst )
        ((environs::lib::DeviceList *) p.inst)->platformRef = 0;

#ifdef OSX_USE_MANUAL_REF_COUNT
    [lock release];

    [observers release];
#endif

    instSP = 0;
}


- (bool) 	disposed { return p.inst->disposed (); }


- (void) DisposeList
{
    p.inst->DisposeList ();
}


/** An array with the devices in the list. */
//@property (readonly, nonatomic) NSArray *   devices;

- (void) SetListType : (environs::DeviceClass_t) MEDIATOR_DEVICE_CLASS_
{
    CVerbVerb ( "SetListType" );
    
    p.inst->SetListType ( MEDIATOR_DEVICE_CLASS_ );
}


- (void) SetIsUIAdapter : (bool) enable
{
    p.inst->SetIsUIAdapter ( enable );
}


DeviceInstance * allocDeviceInstance ( sp ( environs::lib::DeviceInstance ) &deviceSP )
{
    environs::lib::DeviceInstance * dev = deviceSP.get ();
    if ( !dev )
        return nil;
    
    DeviceInstance * device = nil;
    
    //[listLock lock];
    do
    {
        pthread_mutex_t * lock = environs::lib::DeviceListProxy::GetDeviceLock ( dev );
        if ( !lock )
            break;

        LockAcquireV ( lock, "allocDeviceInstance" );

        if ( dev->platformKeep ) {
            device = (__bridge DeviceInstance *) dev->platformKeep;
        }

        LockReleaseV ( lock, "allocDeviceInstance" );

        if ( device ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
            [device retain];
#endif
            CVerbArg ( "allocDeviceInstance: Reusing objID [ %i ]\tplatformRef [ 0x%X ]", dev->objID (), dev->platformKeep );
            break;
        }
        
        if ( dev->disposed () )
            break;

        device = [[DeviceInstance alloc] init]; // retain +1
        if ( !device )
            break;
        CVerbArg ( "allocDeviceInstance: Created new objID [ %i ]", dev->objID () );
        
        if ( ![device SetInst:deviceSP] ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
            [device release];
#endif
            device = nil;
            break;
        }
    }
    while ( 0 );
    
    //[listLock unlock];
    
    return device;
}


NSArray * allocDeviceList ( const svsp ( environs::lib::DeviceInstance ) &list )
{
    CVerbVerb ( "allocDeviceList" );
    
    if ( !list )
        return nil;
    
    size_t size = list->size ();
    
    if ( size <= 0 )
        return nil;
    
    NSMutableArray * ar = [[NSMutableArray alloc] init];
    while ( ar )
    {
        for ( size_t i = 0; i < size; ++i )
        {
            sp ( environs::lib::DeviceInstance ) &deviceSP = list->at ( i );
            
            if ( !deviceSP )
                continue;
            
            DeviceInstance * device = allocDeviceInstance ( deviceSP );
            if ( device ) {
                [ ar addObject : device ];

#ifdef OSX_USE_MANUAL_REF_COUNT
                [device release];
#endif
            }
        }
        break;
    };
    return ar;
}


- (DeviceInstance *) GetItem : (int) position
{
    CVerbVerb ( "GetItem" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetItem ( position );

    return allocDeviceInstance ( deviceSP );
}


- (int) GetCount
{
    CVerbVerb ( "GetCount" );
    
    return p.inst->GetCount ();
}




- (void) AddObserver:(id<ListObserver>) obs
{
    CVerbArg ( "AddObserver [%i]", environs::lib::DeviceListProxy::ObjID ( instSP ) );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observers containsObject:obs] ) {
        [observers addObject:obs];

#ifdef OSX_USE_MANUAL_REF_COUNT
        [obs retain];
#endif
        if ( [observers count] == 1 ) {
            p.inst->AddObserver ( (environs::ListObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}


- (void) RemoveObserver:(id<ListObserver>) obs
{
    CVerbArg ( "RemoveObserver [%i]", environs::lib::DeviceListProxy::ObjID ( instSP ) );
    
    if ( observer == nil ) return;
    
    [lock lock];
    
    if ( [observers containsObject:obs] ) {
        [observers removeObject:obs];

#ifdef OSX_USE_MANUAL_REF_COUNT
        [obs retain];
#endif        
        if ( [observers count] <= 0 ) {
            p.inst->RemoveObserver ( (environs::ListObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}


- (DeviceInstance *) RefreshItem:(DeviceInstance *) source Observer:(id<DeviceObserver>)obs
{
    CVerb ( "RefreshItem" );
    
    environs::lib::DeviceInstance * sdev = [source GetInstancePtr];
    if ( !sdev )
        return nil;
    
    sp ( environs::lib::DeviceInstance ) dev = p.inst->RefreshItem ( sdev, 0 );
    if ( !dev )
        return nil;
    
    return allocDeviceInstance ( dev );
}


/**
 * Get a collection that holds all available devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
- (NSArray *) GetDevices
{
    CVerb ( "GetDevices" );
    
    svsp ( environs::lib::DeviceInstance ) list = p.inst->GetDevices ();
    
    return allocDeviceList ( list );
}

/**
 * Query the number of all available devices within the environment (including those of the Mediator)
 *
 * @return numberOfDevices
 */
- (int) GetDevicesCount
{
    CVerb ( "GetDevicesCount" );
    
    return p.inst->GetDevicesCount ();
}


/**
 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
 *
 * @param deviceID      The device id of the target device.
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInstance-object
 */
- (DeviceInstance *) GetDevice:(int) deviceID Area:(const char *)areaName App:(const char *)appName
{
    CVerb ( "GetDevice" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDevice ( deviceID, areaName, appName, 0 );
    return allocDeviceInstance ( deviceSP );
}



/**
 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
 *
 * @param   objID      The device id of the target device.
 *
 * @return  DeviceInstance-object
 */
- (DeviceInstance *) GetDevice:(OBJIDType) objID
{
    CVerb ( "GetDevice" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDevice ( objID );
    return allocDeviceInstance ( deviceSP );
}

/**
 * Query a DeviceInstance object that best match the deviceID only.
 * Usually the one that is in the same app environment is picked up.
 * If there is no matching in the app environment,
 * then the areas are searched for a matchint deviceID.
 *
 * @param deviceID      The portalID that identifies an active portal.
 * @return DeviceInstance-object
 */
- (DeviceInstance *) GetDeviceBestMatch:(int) deviceID
{
    CVerb ( "GetDeviceBestMatch" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDeviceBestMatch ( deviceID );
    return allocDeviceInstance ( deviceSP );
}

/**
 * Query a DeviceInstance object that best match the deviceID only from native layer.
 * Usually the one that is in the same app environment is picked up.
 * If there is no matching in the app environment,
 * then the areas are searched for a matchint deviceID.
 *
 * @param deviceID      The portalID that identifies an active portal.
 * @return DeviceInstance-object
 */
- (DeviceInstance *) GetDeviceBestMatchNative:(int) deviceID
{
    CVerb ( "GetDeviceBestMatchNative" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDeviceBestMatchNative ( deviceID );
    return allocDeviceInstance ( deviceSP );
}


/**
 * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
- (NSArray *) GetDevicesNearby
{
    CVerb ( "GetDevicesNearby" );
    
    svsp ( environs::lib::DeviceInstance ) list = p.inst->GetDevicesNearby ();
    
    return allocDeviceList ( list );
}

/**
 * Query the number of nearby (broadcast visible) devices within the environment.
 *
 * @return numberOfDevices
 */
- (int) GetDevicesNearbyCount
{
    CVerb ( "GetDevicesNearbyCount" );
    
    return p.inst->GetDevicesNearbyCount ();
}

/**
 * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
 *
 * @param nativeID      The device id of the target device.
 * @return DeviceInstance-object
 */
- (DeviceInstance *) GetDeviceNearby:(int) nativeID
{
    CVerb ( "GetDeviceNearby" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDeviceNearby ( nativeID );
    return allocDeviceInstance ( deviceSP );
}

/**
 * Release the ArrayList that holds the nearby devices.
 */
- (void) ReleaseDevicesNearby
{
    CVerb ( "ReleaseDevicesNearby" );
    
    p.inst->ReleaseDevicesNearby ();
}

/**
 * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
 *
 * @return ArrayList with DeviceInstance objects
 */
- (NSArray *) GetDevicesFromMediator
{
    CVerb ( "GetDevicesFromMediator" );
    
    svsp ( environs::lib::DeviceInstance ) list = p.inst->GetDevicesNearby ();
    
    return allocDeviceList ( list );
}

/**
 * Query a DeviceInstance object of Mediator managed devices within the environment.
 *
 * @param nativeID      The device id of the target device.
 * @return DeviceInstance-object
 */
- (DeviceInstance *) GetDeviceFromMediator:(int) nativeID
{
    CVerb ( "GetDeviceFromMediator" );
    
    sp ( environs::lib::DeviceInstance ) deviceSP = p.inst->GetDeviceFromMediator ( nativeID );
    return allocDeviceInstance ( deviceSP );
}

/**
 * Query the number of Mediator managed devices within the environment.
 *
 * @return numberOfDevices (or -1 for error)
 */
- (int) GetDevicesFromMediatorCount
{
    CVerb ( "GetDevicesFromMediatorCount" );
    
    return p.inst->GetDevicesFromMediatorCount ();
}

/**
 * Release the ArrayList that holds the Mediator server devices.
 */
- (void) ReleaseDevicesMediator
{
    CVerb ( "ReleaseDevicesMediator" );
    
    p.inst->ReleaseDevicesMediator ();
}

/**
 * Reload all device lists. Applications may call this if they manually stopped and started Environs again.
 * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
 */
- (void) ReloadLists
{
    CVerb ( "ReloadLists" );
    
    p.inst->ReloadLists ();
}

/**
 * Reload device lists.
 */
- (void) Reload
{
    CVerb ( "Reload" );
    
    p.inst->Reload ();
}


@end












