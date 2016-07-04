/**
 * PortalInstance object
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

#include "Environs.iOSX.Imp.h"
#include "Environs.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"

#import "Environs.iOSX.h"
#import "Touch.Listener.h"
#import "Device.Instance.iOSX.IAPI.h"
#import "Portal.Instance.iOSX.IAPI.h"
#import "Device.List.iOSX.h"

using namespace environs::API;

#define	CLASS_NAME 	"Portal.Instance.iOSX . ."


#define PORTAL_OUTGOING_DIALOG  1
#define PORTAL_INCOMING_DIALOG  2
#define PORTAL_TRAFFIC_DIALOG   3




using namespace std;


namespace environs
{
    namespace lib
    {
        class PortalInstanceProxy
        {
        public:
            environs::lib::PortalInstance      *    inst;
            
            void SetPortalType (PortalType_t value ) { inst->portalType_ = value; }
            
            sp ( environs::lib::PortalInstance ) GetPortal ( int nativeID, int portalID ) { return inst->GetPortal (nativeID, portalID); };
            
            int GetHandle () {
                return inst->hEnvirons_;
            }
            
            void SetEstablishedCalled ( bool enable ) {
                inst->establishCalled = enable;
            }
            
            void SetAskForTypeValue ( bool enable ) {
                inst->askForTypeValue = enable;
            }
            
            bool GetAskForTypeValue () {
                return inst->askForTypeValue;
            }
            
            void SetNetworkOK ( bool enable ) {
                inst->networkOK = enable;
            }
            
            bool GetNetworkOK () {
                return inst->networkOK;
            }
            
            void Dispose () {
                inst->DisposeInstance ( true );
            }
            
            static void SetDialogID ( environs::lib::PortalInstance * portal, int dialogID ) {
                portal->dialogID = dialogID;
            }
            
            static const sp ( DeviceInstance ) & GetDevice ( environs::lib::PortalInstance * portal ) {
                return portal->device_;
            }
        };
        
        
        class PObserver : public environs::lib::IIPortalObserver
        {
        public:
            /** Constructor */
            PObserver ( ::PortalInstance * p ) : parent ( p ) { OnPortalChangedInterface_ = true; };
            
            ~PObserver () {}

#ifdef OSX_USE_MANUAL_REF_COUNT
            ::PortalInstance * parent;
#else
            __weak ::PortalInstance * parent;
#endif
            void OnPortalChanged ( const sp ( environs::PortalInstance ) &portal, environs::Notify::Portal_t notify ) { OnPortalChanged_ = false; };
            
            void OnPortalChangedInterface ( environs::PortalInstance * sender, environs::Notify::Portal_t notify );
        };
        
        
        void PObserver::OnPortalChangedInterface ( environs::PortalInstance * portal, environs::Notify::Portal_t notify )
        {
            if ( !parent )
                return;
            [parent NotifyObservers:notify];
        };
        
        
        bool PortalInstance::ShowDialogOutgoingPortal ()
        {            
            return [::PortalInstance ShowDialogOutgoingPortal:this];
        }
        
        
        void PortalInstance::PlatformDispose ()
        {
            CVerbVerb ( "PlatformDispose" );

            void * p = 0;
            
            if ( platformRef ) {
                LockAcquireVA ( device_->devicePortalsLock, "PlatformDispose" );
                
                p = platformRef;
                platformRef = 0;
                
                LockReleaseVA ( device_->devicePortalsLock, "PlatformDispose" );
            }
            
            if ( p ) {
                ::PortalInstance * fNil = (__bridge_transfer ::PortalInstance *) p;
                if ( fNil )
                    fNil = nil;
            }
        }
    }
}




@interface PortalInstance ()
{
    environs::lib::PortalInstanceProxy    p;
    
    sp ( environs::PortalInstance )       instSP;
    sp ( environs::lib::PObserver )       observer;
    
    
    NSLock              *   lock;
    NSMutableArray      *   observers;
    
    int                     dialogID;
    
#ifdef ENVIRONS_IOS
    TouchListener *         touchListener;
#endif
}
@end


@implementation PortalInstance (internal)


- (bool) SetInst : ( sp ( environs::lib::PortalInstance ) ) inst_
{
    CVerbVerb ( "SetInst" );
    
    if ( !inst_ )
        return false;
    
    observer = make_shared < environs::lib::PObserver > ( self );
    if ( !observer )
        return false;
    observer->parent = self;
    
    self->instSP = inst_;
    
    p.inst = inst_.get ();
    
    p.inst->platformRef = (__bridge_retained void *) self;

    return true;
}


- (void) NotifyObservers: (environs::Notify::Portal_t) notification
{
    CVerbVerb ( "NotifyObservers" );
    
    for ( int i=0; i < [observers count]; i++ )
    {
        id<PortalObserver> obs = [observers objectAtIndex:i];
        try {
            [obs OnPortalChanged:self Notify:notification];
        }
        catch(...) {
            [observers removeObject:obs];
            i--;
        }
    }
}


- (void) ShowDialogOutgoingPortalView
{
    CVerb ( "ShowDialogOutgoingPortalView" );
    
    dialogID = PORTAL_OUTGOING_DIALOG;
    
#ifdef ENVIRONS_IOS
    UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Portal request"
                                                      message:@"Device 0x... request a portal.."
                                                     delegate:self
                                            cancelButtonTitle:@"No thanks!"
                                            otherButtonTitles:@"Back camera", @"Front camera", nil];
    [message show];
#else
    NSAlert *alert = [[NSAlert alloc]init];
    [alert setAlertStyle:NSInformationalAlertStyle];
    
    [alert addButtonWithTitle:@"No thanks!"];
    [alert addButtonWithTitle:@"Use Camera"];
    
    const char * devName = "Unknown";
    
    DeviceInstance * dev = self.device;
    if ( dev )
        devName = dev.info->deviceName;
    
    [alert setMessageText:[[NSString alloc ] initWithFormat:@"Device %s requests a portal.", devName ]];
    
    if ([alert runModal] == NSAlertSecondButtonReturn) {
        [self Establish:false];
    }
#endif
}


- (void) ShowDialogOutgoingPortal
{
    CVerb ( "ShowDialogOutgoingPortal" );
    
    if ( environs::IsUIThread () )
        [self ShowDialogOutgoingPortalView];
    else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [self ShowDialogOutgoingPortalView];
        });
    }
}


+ (void) ShowDialogOutgoingPortalView: (environs::lib::PortalInstance *) portal
{
    CVerb ( "ShowDialogOutgoingPortalView" );
    
    environs::lib::PortalInstanceProxy::SetDialogID ( portal, PORTAL_OUTGOING_DIALOG );
    
#ifdef ENVIRONS_IOS
    UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Portal request"
                                                      message:@"Device 0x... request a portal.."
                                                     delegate:self
                                            cancelButtonTitle:@"No thanks!"
                                            otherButtonTitles:@"Back camera", @"Front camera", nil];
    [message show];
#else
    NSAlert *alert = [[NSAlert alloc]init];
    [alert setAlertStyle:NSInformationalAlertStyle];
    
    [alert addButtonWithTitle:@"No thanks!"];
    [alert addButtonWithTitle:@"Use Camera"];
    
    const char * devName = "Unknown";
    
    sp ( environs::lib::DeviceInstance ) dev = environs::lib::PortalInstanceProxy::GetDevice(portal);
    if ( dev )
        devName = dev->info()->deviceName;
    
    [alert setMessageText:[[NSString alloc ] initWithFormat:@"Device %s requests a portal.", devName ]];
    
    if ([alert runModal] == NSAlertSecondButtonReturn) {
        portal->Establish ( false );
    }
#endif
}


+ (bool) ShowDialogOutgoingPortal: (environs::lib::PortalInstance *) portal
{
    CVerb ( "ShowDialogOutgoingPortal" );
    
    if ( environs::IsUIThread () )
        [ PortalInstance ShowDialogOutgoingPortalView : portal ];
    else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [ PortalInstance ShowDialogOutgoingPortalView : portal ];
        });
    }
    return true;
}

@end


@implementation PortalInstance

- (id) init
{
    CVerbVerb ( "init" );
    
    self = [super init];
    if ( self ) {
        dialogID    = 0;
        
        lock        = [[NSLock alloc] init];
        
        observers   = [[NSMutableArray alloc] init];
        
#ifdef ENVIRONS_IOS
        touchListener       = [[TouchListener alloc]init];
#endif
        
    }
    return self;
}


- (void) dealloc
{
    CVerbVerb ( "dealloc" );
    
    if ( [observers count] > 0 )
        p.inst->RemoveObserver ( (environs::PortalObserver *) observer.get () );
    
#ifdef ENVIRONS_IOS
    touchListener = 0;
#endif
    
    instSP = 0;

    IOSX_SUPER_DEALLOC ();
}


/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
- (id) 	device {
    sp ( environs::DeviceInstance ) idev = p.inst->device();
    if ( !idev )
        return nil;
    environs::lib::DeviceInstance * dev = (environs::lib::DeviceInstance *) idev.get();
    if ( !dev->platformRef )
        return nil;
    return (__bridge id) dev->platformRef;
}


/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
- (environs::Call_t) 	async { return p.inst->async; }
- (void) 	setAsync : (environs::Call_t) value { p.inst->async = value; }

/** An ID that identifies this portal across all available portals. */
- (int) 	portalID { return p.inst->portalID (); }

/** true = Object is disposed and not updated anymore. */
- (bool) 	disposed { return p.inst->disposed (); }

- (environs::PortalInfo *) 	info { return p.inst->info (); }

- (environs::PortalStatus_t) status { return p.inst->status (); }

- (bool) 	disposeOngoing { return p.inst->disposeOngoing (); }

- (bool) 	startIfPossible { return p.inst->startIfPossible; }
- (void) 	setStartIfPossible : (bool) value { p.inst->startIfPossible = value; }

/** true = outgoing (Generator), false = incoming (Receiver). */
- (bool) 	outgoing { return p.inst->outgoing (); }

- (bool) 	isIncoming { return !p.inst->outgoing (); }

- (bool) 	isOutgoing { return p.inst->outgoing (); }


- (environs::PortalType_t) 	portalType { return p.inst->portalType (); }
- (void) 	setPortalType : (environs::PortalType_t) value { p.SetPortalType ( value ); }

/** Application defined contexts for arbitrary use. */
- (id)      appContext0 { return (__bridge id)p.inst->appContext0; }
- (void)    setAppContext0 : (id) value { p.inst->appContext0 = (__bridge void *) value; }

- (id)      appContext1 { return (__bridge id)p.inst->appContext1; }
- (void)    setAppContext1 : (id) value { p.inst->appContext1 = (__bridge void *) value; }

- (id)      appContext2 { return (__bridge id)p.inst->appContext2; }
- (void)    setAppContext2 : (id) value { p.inst->appContext2 = (__bridge void *) value; }

- (id)      appContext3 { return (__bridge id)p.inst->appContext3; }
- (void)    setAppContext3 : (id) value { p.inst->appContext3 = (__bridge void *) value; }


- (void) AddObserver:(id<PortalObserver>) obs
{
    CVerb ( "AddObserver" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( ![observers containsObject:obs] ) {
        [observers addObject:obs];
        
        if ( [observers count] == 1 ) {
            p.inst->AddObserver ( (environs::PortalObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}

- (void) RemoveObserver:(id<PortalObserver>) obs
{
    CVerb ( "RemoveObserver" );
    
    if ( obs == nil ) return;
    
    [lock lock];
    
    if ( [observers containsObject:obs] ) {
        [observers removeObject:obs];
        
        if ( [observers count] <= 0 ) {
            p.inst->RemoveObserver ( (environs::PortalObserver *) observer.get () );
        }
    }
    
    [lock unlock];
}


- (bool) Establish:(bool) askForType
{
    CVerb ( "Establish" );
    
    DeviceInstance * dev = self.device;
    
    if ( p.inst->disposed() || !dev || dev.disposed || !dev.info->isConnected )
        return false;
    
    p.SetEstablishedCalled ( true );

    p.SetAskForTypeValue ( askForType );
    
    if ( !p.GetNetworkOK () && ![self CheckNetworkConnection] )
        return true;
    
    WARNING (TODO TOCHECK)
    
    if ( p.inst->outgoing () ) {
        if ( askForType ) {
            [self ShowDialogOutgoingPortal];
        }
        else {
            CVerbArg ( "Establish: Provide [0x%X]", self.portalID );
            
            return p.inst->Establish ( askForType );
            /*
            int status = inst->status ();
            if (status == PORTAL_STATUS_CREATED_FROM_REQUEST) {
                return ProvidePortalStream1 ( dev->info.nativeID, async, portalID | portalType );
            }
            else if (status == PORTAL_STATUS_CREATED_ASK_REQUEST) {
                return ProvideRequestPortalStream1 ( dev->info.nativeID, async, portalID | portalType);
            }
             */
        }
    }
    else {
        CVerbArg ( "Establish: Request [0x%X]", self.portalID );
        
        return p.inst->Establish ( askForType );
        //return RequestPortalStream1 ( dev->info.nativeID, async, portalID | portalType, info.base.width, info.base.height );
    }
    return false;
}

- (bool) Start
{
    return p.inst->Start ();
}

- (bool) Stop
{
    return p.inst->Stop ();
}

- (bool) SetRenderSurface:(id)surface
{
    return p.inst->SetRenderSurface ( (__bridge void *) surface );
}

- (bool) SetRenderSurface:(id)surface Width:(int)width Height:(int)height
{
    bool success = p.inst->SetRenderSurface ( (__bridge void *)  surface, width, height );
    
#ifdef ENVIRONS_IOS
    if ( surface ) {
        touchListener->hEnvirons = p.GetHandle ();
        touchListener->env = environs::instances [ touchListener->hEnvirons ] ;
        [touchListener UpdateView:surface Portal:self];
    }
#endif
    
    return success;
}

- (bool) ReleaseRenderSurface
{
    return p.inst->ReleaseRenderSurface ();
}

- (environs::PortalInfoBase *) GetPortalInfo:(int)portalID
{
    return p.inst->GetIPortalInfo();
}

- (bool) SetPortalInfo:(environs::PortalInfoBase *)infoBase
{
    return p.inst->SetIPortalInfo ( infoBase );
}

- (PortalInstance *) GetPortal:(int)nativeID  PortalID:(int)portalID
{
    CVerbVerb ( "GetPortal" );
    
    sp ( environs::lib::PortalInstance ) portal = p.GetPortal ( nativeID, portalID );
    
    void * pr = portal->platformRef;    
    if ( pr )
        return 0;
    
    return (__bridge PortalInstance *) pr;
}


- (void) ShowDialogNoWiFiWarn
{
    CVerb ( "ShowDialogNoWiFiWarn" );
    
    dialogID = PORTAL_TRAFFIC_DIALOG;
    
#ifdef ENVIRONS_IOS
    UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"High Traffic Warning"
                                                      message:@"You are not connected by WiFi! A portal will create high data traffic. Continue?"
                                                     delegate:self
                                            cancelButtonTitle:@"No, Cancel!"
                                            otherButtonTitles:@"Yes", nil];
    [message show];
#else
#endif
}


- (bool) CheckNetworkConnection
{
    CVerb ( "CheckNetworkConnection" );
    
    dialogID = PORTAL_TRAFFIC_DIALOG;
    
    if ( GetNetworkStatusN () >= NETWORK_CONNECTION_WIFI ) {
        p.SetNetworkOK ( true );
    }
    else {
        p.SetNetworkOK ( false );
        
#ifdef ENVIRONS_IOS
        if ( environs::IsUIThread () ) {
            [self ShowDialogNoWiFiWarn];
        }
        else {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [self ShowDialogNoWiFiWarn];
            });
        }
#endif
    }
    
    return p.GetNetworkOK ();
}


#ifdef ENVIRONS_IOS

- (void) alertView:(UIAlertView *) alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    CVerbVerb ( "alertView" );
    
    NSString *title = [alertView buttonTitleAtIndex:buttonIndex];
    
    if ( dialogID == PORTAL_OUTGOING_DIALOG )
    {
        if ( [title isEqualToString:@"Back camera"] )
        {
            p.SetPortalType ( environs::PortalType::BackCam );
            
            [self Establish:false];
        }
        else if ( [title isEqualToString:@"Front camera"] )
        {
            p.SetPortalType ( environs::PortalType::FrontCam );
            
            [self Establish:false];
        }
        else
            p.Dispose ();
    }
    else if ( dialogID == PORTAL_TRAFFIC_DIALOG )
    {
        if ( [title isEqualToString:@"Yes"] )
        {
            p.SetNetworkOK ( true );
            
            [self Establish : p.GetAskForTypeValue () ];
        }
        else
            p.Dispose ();
    }
}

#else


- (void) alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)retcode contextInfo:(void *)context
{
    CVerbVerb ( "alertDidEnd" );
    
}

#endif



@end










