/**
 * Environs.iOSX.mm
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

#include "Environs.h"
#include "Environs.Lib.h"
#include "Environs.Native.h"
#include "Interfaces/IPortal.Decoder.h"
#include "Environs.Obj.h"
#include "Environs.Sensors.h"

#import "Environs.iOSX.h"
#import "Login.Dialog.h"
#import "Device.List.iOSX.IAPI.h"
#import "Portal/Portal.Instance.iOSX.IAPI.h"

#import <SystemConfiguration/SystemConfiguration.h>
#import <CommonCrypto/CommonHMAC.h>
#import <CommonCrypto/CommonCryptor.h>

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#ifdef ENVIRONS_IOS
//******** iOS *************
#import "Touch.Listener.h"
#import <AdSupport/ASIdentifierManager.h>
#else
//******** OSX *************
#define UIScreen    NSScreen
#endif

#include <sys/sysctl.h>
#include <mach/mach_time.h>

using namespace environs::API;

#define	CLASS_NAME 	"Environs.iOSX. . . . . ."


/********************************************
 * Forwards (C++)
 ********************************************
 */
namespace environs
{    
    namespace lib { class EObserver; class EMessageObserver; class EDataObserver; class ESensorDataObserver; }
}

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
extern ::PortalInstance * GetPortalInstanceInterface ( environs::PortalInstance * portal );
#endif


/********************************************
 * iOSX Interface of Environs
 ********************************************
 */
@interface Environs ()
{
    int                                     hEnvirons;
    sp ( environs::lib::Environs )          inst;
    
    sp ( environs::lib::EObserver )         observer;
    sp ( environs::lib::EMessageObserver )  observerForMessages;
    sp ( environs::lib::EDataObserver )     observerForData;
    sp ( environs::lib::ESensorDataObserver ) observerForSensors;
}

@end



/********************************************
 * iOSX Interface of Environs (internal)
 ********************************************
 */
@interface Environs (internal)

- (void) OnStatus:(environs::Status_t) status;

- (void) OnNotify:(environs::ObserverNotifyContext *) context;
- (void) OnNotifyExt:(environs::ObserverNotifyContext *) context;

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance;
#endif

- (void) OnMessage:(environs::ObserverMessageContext *) context;
- (void) OnMessageExt:(environs::ObserverMessageContext *) context;


- (void) OnData:(environs::ObserverDataContext *) context;

- (void) OnStatusMessage:(const char *) message;
- (void) OnSensorData:(int) objID Frame:(environs::SensorFrame *)sensorFrame;

@end


#ifndef OSX_USE_MANUAL_REF_COUNT
__weak
#endif
Environs             *   instancesPlt [ ENVIRONS_MAX_ENVIRONS_INSTANCES ];

static bool                     monitorReach        = false;
static SCNetworkReachabilityRef reachabilityConnection = 0;
static void ReachabilityCallback ( SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info );

NSMutableArray              *   environsObservers = [[NSMutableArray alloc] init ];

NSMutableArray              *   environsObserversForMessages = [[NSMutableArray alloc] init ];

NSMutableArray              *   environsObserversForData = [[NSMutableArray alloc] init ];
NSMutableArray              *   environsObserversForSensor = [[NSMutableArray alloc] init ];

NSLock                      *   envObserverLock = [[NSLock alloc] init];


bool InitNetworkNotifier ();

/********************************************
 * C++ Implementations
 ********************************************
 */
namespace environs
{
    extern mach_timebase_info_data_t environs_time_base_info;
    
    
    bool AllocNativePlatformIOSX ()
    {
        CVerb ( "AllocNativePlatformIOSX" );
        
        mach_timebase_info ( &environs_time_base_info );
        
        return true;
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
        CFUUIDRef appIDRef = CFUUIDCreate ( kCFAllocatorDefault );
        if ( !appIDRef )
            return false;

        bool success = false;

        @autoreleasepool {
            CFStringRef appIDStr = CFUUIDCreateString ( kCFAllocatorDefault, appIDRef );
            if ( appIDStr )
            {
#ifdef OSX_USE_MANUAL_REF_COUNT
                NSString * appID = (NSString *) appIDStr;
#else
                NSString * appID = (__bridge NSString *) appIDStr;
#endif
                NSString * appIDlower = [appID lowercaseString];
                if ( appIDlower )
                {
                    if ( snprintf ( buffer, bufSize, "%s", [appIDlower UTF8String] ) > 0 )
                        success = true;
                }
                CFRelease ( appIDStr );
            }
        }

        CFRelease ( appIDRef );

        return success;
    }
    
    
    void EnvironsPlatformInit ( int hInst )
    {
        // Initialize parameters for the surface connection
        if ( !environs::native.IsNativeAllocated () )
            return;
        
        environs::InitIOSX ();
    
        // Update Device parameters
        environs::UpdateDeviceParams ();
        
        InitNetworkNotifier ();
    }
    
    
    
    bool StartNetworkNotifier ( Instance * env )
    {
        if ( !reachabilityConnection ) {
            if ( !InitNetworkNotifier () )
                return false;
        }
        
        SCNetworkReachabilityContext context = {0, (void *)env, NULL, NULL, NULL};
        
        if (SCNetworkReachabilitySetCallback(reachabilityConnection, ReachabilityCallback, &context))
        {
            if (SCNetworkReachabilityScheduleWithRunLoop(reachabilityConnection, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode))
            {
                CVerb ( "StartNetworkNotifier: Success" );
                return true;
            }
        }
        
        CErr ( "StartNetworkNotifier: Failed" );
        return false;
    }
    
    
    void StopNetworkNotifier ()
    {
        if (reachabilityConnection != NULL)
        {
            CVerb ( "StopNetworkNotifier: Stopped." );
            
            SCNetworkReachabilityUnscheduleFromRunLoop(reachabilityConnection, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        }
        
    }
    
    
    void DoInvokeNetworkNotifier ( Instance * env, bool enable )
    {
        if ( enable ) {
            monitorReach = StartNetworkNotifier ( env );
            
            RegisterMainThreadN ( 1 );
        }
        else {
            if ( monitorReach ) {
                StopNetworkNotifier ();
                
                monitorReach = false;
            }
        }
    }
    
    
    void InvokeNetworkNotifier ( int hInst, bool enable )
    {
        Instance * env = instances [ hInst ];
        if ( !env )
            return;
        
        if ( native.useHeadless )
            DoInvokeNetworkNotifier ( env, enable );
        else
        {
            dispatch_sync(dispatch_get_main_queue(), ^{
                DoInvokeNetworkNotifier ( env, enable );
            });
        }
    }
    
    
    bool IsUIThread ()
    {
        return [NSThread isMainThread] ;
    }
    
    
    namespace API
    {
        NSString * GetNSSID ( bool desc );
        
        void onLocationChanged ( int hInst, CLLocation * sensorData );
        
        
        void Environs_LoginDialog ( int hInst, const char * userName )
        {
            NSString * uname = userName ? [[NSString alloc ] initWithUTF8String: userName] : @"User Name";
            
            LoginDialog * dlg = [LoginDialog SingletonInstance:@"Please enter login credentials:" Title:@"Mediator Login" UserName:uname];
            if ( dlg ) {
                dlg->env = native.GetInstanceAPI ( hInst );
                [dlg ShowResult];
            }
        }
        
        
        bool RenderSurfaceCallback ( int type, void * surface, void * decoderOrByteBuffer )
        {
            if ( type == RENDER_CALLBACK_TYPE_DECODER )
            {
                if ( !surface || !decoderOrByteBuffer )
                    return false;
                
                RenderDecoderToSurface ( surface, decoderOrByteBuffer );
            }
            else if ( type == RENDER_CALLBACK_TYPE_IMAGE )
            {
                if ( !surface || !decoderOrByteBuffer )
                    return false;
                
                RenderImageToSurface ( surface, decoderOrByteBuffer );
            }
            else if ( type == RENDER_CALLBACK_TYPE_INIT )
            {
                if ( !decoderOrByteBuffer ) {
                    CErr ( "RenderSurfaceCallback: invalid buffer!" );
                    return false;
                }
                
                char * buffer = (BYTEBUFFER_DATA_POINTER_START ( decoderOrByteBuffer )) + 4;
                
                int width = *((int *)buffer);
                int height = *((int *)(((char *)buffer) + 4));
                
                CLogArg ( "RenderSurfaceCallback: got [%s]stream init with width [%i] and height [%i]",
                         (((ByteBuffer *)decoderOrByteBuffer)->type & DATA_STREAM_IMAGE) == DATA_STREAM_IMAGE ? "image" : "h264", width, height );
            }
            
            return true;
        }
        
        
        bool opt ( int hInst, const char * key, const char * value )
        {
            CVerbArg ( "opt: storing key=%s, value=%s", key ? key : "NULL", value ? value : "NULL" );
            
            NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
            if ( !prefs ) {
                CErr ( "User defaults not accessible!" );
                return false;
            }
            
            [prefs setValue:[[NSString alloc ] initWithUTF8String:value] forKey:[[NSString alloc ] initWithUTF8String:key]];
            [prefs synchronize];
            CVerbArg ( "opt: stored key=%s, value=%s", key, value );
            return true;
        }
        
        
        const char * optString ( int hInst, const char * key )
        {
            CVerbArg ( "optString: loading key=%s", key ? key : "NULL" );
            
            NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
            if ( !prefs ) {
                CErr ( "optString: User defaults not accessible!" );
                return 0;
            }
            
            static char buffer [ 128 ];
            
            NSString * value = [prefs stringForKey:[[NSString alloc ] initWithUTF8String:key]];
            if ( value ) {
                if ( [value getCString:buffer maxLength:sizeof(buffer)/sizeof(*buffer) encoding:NSUTF8StringEncoding] ) {
                    CVerbArg ( "optString: loading key=%s, value=%s", key, buffer );
                    return buffer;
                }
                
            }
            //            else if ( !strcmp ( key, "optInitialSettings" ) )
            //                return "1";
            
            return 0;
        }
        
        
        const char * opt ( int hInst, const char * key )
        {
            CVerbArg ( "opt: loading key=%s", key ? key : "NULL" );
            
            const char * value = optString ( hInst, key );
            if ( value )
                return value;
            
            return "0";
        }
        
        
        void UpdateNetworkStatus ()
        {
            CVerb ( "UpdateNetworkStatus" );
            
            SCNetworkReachabilityFlags flags;
            
            if ( reachabilityConnection == 0 ) {
                CWarn ( "UpdateNetworkStatus: no reachability ref available!" );
                return;
            }
            
            if ( SCNetworkReachabilityGetFlags(reachabilityConnection, &flags) )
            {
                ReachabilityCallback ( 0, flags, 0 );
            }
        }
    }
    
    
    namespace lib
    {
        class EnvironsProxy
        {
        public:
            static void SetHandle ( environs::lib::Environs * inst, int hInst )
            {
                inst->SetInstance ( hInst );
                inst->isUIAdapter = true;
            }
            static Environs * Create ()
            {
                return new Environs ();
            }
        };
        
        class EObserver : public EnvironsObserver
        {
        public:
            /** Constructor */
            EObserver ( ::Environs * p ) : parent ( p ) {};
            
            ~EObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::Environs * parent;
            
            void OnStatus ( environs::Status_t status );
            
            void OnNotify ( environs::ObserverNotifyContext * context );
            
            void OnNotifyExt ( environs::ObserverNotifyContext * context );

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
            void OnPortalRequestOrProvidedBase ( environs::PortalInstance * portal );
#endif
        };
        
        void EObserver::OnStatus ( environs::Status_t status )
        {
            if ( parent )
                [parent OnStatus:status];
        }
        
        void EObserver::OnNotify ( environs::ObserverNotifyContext * context )
        {
            if ( parent )
                [parent OnNotify:context];
        }
        
        void EObserver::OnNotifyExt ( environs::ObserverNotifyContext * context )
        {
            if ( parent )
                [parent OnNotifyExt:context];
        }

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
        void EObserver::OnPortalRequestOrProvidedBase ( environs::PortalInstance * portal )
        {
            if ( !parent )
                return;
            ::PortalInstance * p = GetPortalInstanceInterface ( portal );
            if ( p ) {
                [parent OnPortalRequestOrProvided:p];
            }
        }
#endif
        
        class EMessageObserver : public EnvironsMessageObserver
        {
        public:
            /** Constructor */
            EMessageObserver ( ::Environs * p ) : parent ( p ) {};
            
            ~EMessageObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::Environs * parent;
            
            void OnMessage ( environs::ObserverMessageContext * context );
            
            void OnMessageExt ( environs::ObserverMessageContext * context );
            
            void OnStatusMessage ( const char * message );
        };
        
        void EMessageObserver::OnMessage ( environs::ObserverMessageContext * context )
        {
            if ( parent )
                [parent OnMessage:context];
        }
        
        void EMessageObserver::OnMessageExt ( environs::ObserverMessageContext * context )
        {
            if ( parent )
                [parent OnMessageExt:context];
        }
        
        void EMessageObserver::OnStatusMessage ( const char * message )
        {
            if ( parent )
                [parent OnStatusMessage:message];
        }
        
        
        class EDataObserver : public EnvironsDataObserver
        {
        public:
            /** Constructor */
            EDataObserver ( ::Environs * p ) : parent ( p ) {};
            
            ~EDataObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::Environs * parent;
            
            void OnData ( environs::ObserverDataContext * context );
        };
        
        void EDataObserver::OnData ( environs::ObserverDataContext * context )
        {
            if ( parent )
                [parent OnData:context];
        }
        
        
        class ESensorDataObserver : public EnvironsSensorObserver
        {
        public:
            /** Constructor */
            ESensorDataObserver ( ::Environs * p ) : parent ( p ) {};
            
            ~ESensorDataObserver () {}

#ifndef OSX_USE_MANUAL_REF_COUNT
            __weak
#endif
            ::Environs * parent;
            
            void OnSensorData ( int objID, environs::SensorFrame * sensorFrame );
        };
        
        void ESensorDataObserver::OnSensorData ( int objID, environs::SensorFrame * sensorFrame )
        {
            if ( parent )
                [parent OnSensorData:objID Frame:sensorFrame];
        }
        
        
        bool DeviceList::DeviceListUpdateDispatchSync ( const sp ( DeviceListUpdatePack ) &updatePacks )
        {
            static bool updated = false;
            
            if ( IsUIThread () ) {
                return DeviceList::DeviceListUpdateDataSourceSync ( updatePacks );
            }
            dispatch_sync(dispatch_get_main_queue(), ^{
                updated = DeviceList::DeviceListUpdateDataSourceSync ( updatePacks );
            });
            
            return updated;
        }
    }
}


/********************************************
 * iOSX Implementation of Environs (internal)
 ********************************************
 */
@implementation Environs (internal)


- (void) OnStatus:(environs::Status_t) status
{
    for ( int i=0; i < [environsObservers count]; i++ )
    {
        id<EnvironsObserver> obs = [environsObservers objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnStatus:status];
            }
            catch(...) {
                CErr ( "OnStatus: Exception!" );
            }
        }
    }
}

- (void) OnNotify:(environs::ObserverNotifyContext *) context
{
    for ( int i=0; i < [environsObservers count]; i++ )
    {
        id<EnvironsObserver> obs = [environsObservers objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnNotify:context];
            }
            catch(...) {
                CErr ( "OnNotify: Exception!" );
            }
        }
    }
}

- (void) OnNotifyExt:(environs::ObserverNotifyContext *) context
{
    for ( int i=0; i < [environsObservers count]; i++ )
    {
        id<EnvironsObserver> obs = [environsObservers objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnNotifyExt:context];
            }
            catch(...) {
                CErr ( "OnNotifyExt: Exception!" );
            }
        }
    }
}


#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
- (void) OnPortalRequestOrProvided:(id) portalInstance
{
    
    for ( int i=0; i < [environsObservers count]; i++ )
    {
        id<EnvironsObserver> obs = [environsObservers objectAtIndex:i];
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

- (void) OnMessage:(environs::ObserverMessageContext *) context
{
    for ( int i=0; i < [environsObserversForMessages count]; i++ )
    {
        id<EnvironsMessageObserver> obs = [environsObserversForMessages objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnMessage:context];
            }
            catch(...) {
                CErr ( "OnMessage: Exception!" );
            }
        }
    }
}

- (void) OnMessageExt:(environs::ObserverMessageContext *) context
{
    for ( int i=0; i < [environsObserversForMessages count]; i++ )
    {
        id<EnvironsMessageObserver> obs = [environsObserversForMessages objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnMessageExt:context];
            }
            catch(...) {
                CErr ( "OnMessageExt: Exception!" );
            }
        }
    }
}

- (void) OnStatusMessage:(const char *)message
{
    for ( int i=0; i < [environsObserversForMessages count]; i++ )
    {
        id<EnvironsMessageObserver> obs = [environsObserversForMessages objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnStatusMessage:message];
            }
            catch(...) {
                CErr ( "OnStatusMessage: Exception!" );
            }
        }
    }
}

- (void) OnData:(environs::ObserverDataContext *) context
{
    for ( int i=0; i < [environsObserversForData count]; i++ )
    {
        id<EnvironsDataObserver> obs = [environsObserversForData objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnData:context];
            }
            catch(...) {
                CErr ( "OnData: Exception!" );
            }
        }
    }
}


- (void) OnSensorData:(int) objID Frame:(environs::SensorFrame *)sensorFrame
{
    for ( int i=0; i < [environsObserversForSensor count]; i++ )
    {
        id<EnvironsSensorObserver> obs = [environsObserversForSensor objectAtIndex:i];
        if ( obs ) {
            try {
                [obs OnSensorData:objID Frame:sensorFrame];
            }
            catch(...) {
                CErr ( "OnSensorData: Exception!" );
            }
        }
    }
}


@end


/********************************************
 * iOSX Implementation of Environs
 ********************************************
 */
@implementation Environs



/** Perform calls to the Environs object asynchronously. If set to Environs.CALL_WAIT, then all commands will block until the call finishes.
 * If set to Environs.CALL_NOWAIT, then certain calls (which may take longer) will be performed asynchronously. */

- (environs::Call_t) 	async { return inst->async; }

- (void)    setAsync : (environs::Call_t) value { inst->async = value; }



/**
 * Instructs the framework to perform a quick shutdown (with minimal wait times)
 *
 * @param enable      true / false
 */
- (void) SetAppShutdown : (bool) enable
{
    environs::native.isAppShutdown = enable;
}


static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info)
{
    int netStat = NETWORK_CONNECTION_NO_NETWORK;
    
    CVerb ( "got a reachability notification" );
    
    if ( flags & kSCNetworkReachabilityFlagsReachable )
    {
        if ( flags & kSCNetworkReachabilityFlagsIsDirect )
        {
            netStat = NETWORK_CONNECTION_WIFI;
        }
        else
            netStat = NETWORK_CONNECTION_MOBILE_DATA;
    }
    
    SetNetworkStatusN ( netStat );
}


bool InitNetworkNotifier ()
{
    // const char * reachHost = 0;
    
    /*    if ( environs::DefaultMediatorIP[0] )
     reachHost = environs::DefaultMediatorIP;
     else if ( environs::CustomMediatorIP )
     reachHost = inet_ntoa ( environs::CustomMediatorIP );
     else */
    //       reachHost = "hcm-lab.de";
    //    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName ( NULL, reachHost );
    
    /// Reachability for internet connection
    struct sockaddr_in addr;
    Zero ( addr );
    addr.sin_len = sizeof(addr);
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = htonl(IN_LINKLOCALNETNUM);
    
    reachabilityConnection = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr *)&addr);
    
    if ( reachabilityConnection == NULL ) {
        CErr ( "InitNetworkNotifier: Failed." );
        return false;
    }
    
    CVerb ( "InitNetworkNotifier: Success" );
    return true;
}


void DisposeNetworkNotifier ()
{
    if ( !reachabilityConnection )
        return;
    
    CFRelease ( reachabilityConnection );
    reachabilityConnection = 0;
}


- (bool) StartNetworkNotifier
{
    if ( !reachabilityConnection ) {
        if ( !InitNetworkNotifier () )
            return false;
    }
    
    SCNetworkReachabilityContext context = {0, (__bridge void *)(self), NULL, NULL, NULL};
    
    if (SCNetworkReachabilitySetCallback(reachabilityConnection, ReachabilityCallback, &context))
    {
        if (SCNetworkReachabilityScheduleWithRunLoop(reachabilityConnection, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode))
        {
            CVerb ( "StartNetworkNotifier: Success" );
            return true;
        }
    }
    
    CErr ( "StartNetworkNotifier: Failed" );
    return false;
}


- (void) StopNetworkNotifier
{
    if (reachabilityConnection != NULL)
    {
        CVerb ( "StopNetworkNotifier: Stopped." );
        
        SCNetworkReachabilityUnscheduleFromRunLoop(reachabilityConnection, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
    
}


+ (void) ShowMessage:(NSString *) sender Message:(const char *) message Length:(int)length
{
    if ( !message || !length )
        return;
    
    for (unsigned int i=0; i<length; i++) {
        CLogArg ( "C:%c", message[i] );
    }
    
#ifdef ENVIRONS_IOS
    UIApplication* app = [UIApplication sharedApplication];
    UILocalNotification* messageNotifier = [[UILocalNotification alloc] init];
    if (messageNotifier)
    {
        NSString * rawMsg = [[NSString alloc ] initWithUTF8String:message];
        /*
         NSString* s = [[NSString alloc] initWithBytes:message length:msgLength  encoding:NSUTF8StringEncoding];
         
         NSLog(@"%@", s);
         */
        NSString * msg = [[NSString alloc ] initWithFormat:@"Message from Device %@\r\n%@", sender, rawMsg];
        
        messageNotifier.timeZone = [NSTimeZone defaultTimeZone];
        messageNotifier.repeatInterval = 0;
        messageNotifier.alertBody = msg;
        [app presentLocalNotificationNow:messageNotifier];
    }
#else
#endif
    
}


- (void) ShowMessage:(NSString *) sender Message:(const char *) message Length:(int)length
{
    [Environs ShowMessage:sender Message:message Length:length];
}


+ (void) initialize {
    memset ( instancesPlt, 0, sizeof(instancesPlt) );
}


/**
 * Create an Environs object.
 *
 * @return   An Environs object
 */
+ (Environs *) CreateInstance
{
    int hEnvirons = environs::API::CreateEnvironsN ();
    if ( hEnvirons <= 0 )
        return nil;
    
    Environs * env = nil;
    
    do
    {
        env = [[Environs alloc] init];
        if ( !env )
            break;
        
        /// Initialize platform layer if necessary
        environs::lib::EnvironsProxy::SetHandle ( env->inst.get (), hEnvirons );
        
        env->hEnvirons = hEnvirons;
        
        if ( !env->inst->Init() )
            break;
        
        instancesPlt [ hEnvirons ] = env;
        return env;
    }
    while ( 0 );
    
    if ( hEnvirons < ENVIRONS_MAX_ENVIRONS_INSTANCES ) {
        environs::API::DisposeN ( hEnvirons );
    }
    return nil;
}


/**
 * Create an Environs object.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   An Environs object
 */
+ (Environs *) CreateInstance: (const char *) appName Area:(const char *) areaName
{
    Environs * env = [Environs CreateInstance];
    if ( env ) {
        [env LoadSettings:appName Area:areaName];
    }
    return env;
}


/**
 * Create an Environs object.
 *
 * @return   An Environs object
 */
+ (Environs *) New
{
    return [Environs CreateInstance];
}


/**
 * Create an Environs object.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   An Environs object
 */
+ (Environs *) New: (const char *) appName Area:(const char *) areaName
{
    return [Environs CreateInstance: appName Area:areaName];
}


/**
 * Load settings for the given application environment from settings storage,
 * if any have been saved previously.
 *
 * @param 	appName		The application name for the application environment.
 * @param  	areaName	The area name for the application environment.
 *
 * @return   success
 */
- (bool) LoadSettings: (const char *) appName Area:(const char *) areaName
{
    return environs::API::LoadSettingsN(hEnvirons, appName, areaName);
}


/**
 * Load settings. Prior to this call, an application environment MUST be given
 * using SetApplicationName and SetAreaName.
 *
 * @return   success
 */
- (bool) LoadSettings
{
    return environs::API::LoadSettingsN(hEnvirons, 0, 0);
}


/**
 * Dispose the storage, that is remove all data and messages received in the data store.
 *
 */
- (void) ClearStorage {
    environs::API::ClearStorageN ();
}


- (id) init
{
    CVerb ( "init" );
    
    self = [super init];
    
    if ( self ) {
        hEnvirons = 0;
        
        observer = std::make_shared<environs::lib::EObserver> ( self );
        if ( !observer )
            return nil;
        
        observerForMessages = std::make_shared<environs::lib::EMessageObserver> ( self );
        if ( !observerForMessages )
            return nil;
        
        observerForData = std::make_shared<environs::lib::EDataObserver> ( self );
        if ( !observerForData )
            return nil;
        
        observerForSensors = std::make_shared<environs::lib::ESensorDataObserver> ( self );
        if ( !observerForSensors )
            return nil;
        
        inst = sp ( environs::lib::Environs ) ( environs::lib::EnvironsProxy::Create () );
        if ( !inst )
            return nil;
        inst->Retain (); // Not really necessary here.. just for completeness
        
        inst->platformRef = (__bridge void *) self;        
    }
    
    return self;
}


- (bool) opt:(NSString *) key
{
    NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
    if ( !prefs ) {
        return false;
    }
    
    return [prefs integerForKey:[[NSString alloc ] initWithFormat:@"%i_%@", environs::API::GetAppAreaIDN(hEnvirons), key]];
}


+ (const char *) optString:(NSString *) key
{
    NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
    if ( !prefs ) {
        return 0;
    }
    
    return [[prefs stringForKey:key] UTF8String];
}


- (void) dealloc
{
    CLog ( "dealloc" );
    
    inst->RemoveObserver ( observer.get () );
    inst->RemoveObserverForData ( observerForData.get () );
    inst->RemoveObserverForMessages ( observerForMessages.get () );
    inst->RemoveObserverForSensorData ( observerForSensors.get () );
    
    if ( monitorReach ) {
        [self StopNetworkNotifier];
        monitorReach = false;
    }
    
    DisposeNetworkNotifier ();
    
    [self Dispose];

#ifdef OSX_USE_MANUAL_REF_COUNT
    [envObserverLock release];

    [environsObservers release];
    [environsObserversForMessages release];
    [environsObserversForData release];
    [environsObserversForSensorData release];
#endif

    IOSX_SUPER_DEALLOC ();
}


- (bool) Init
{
    CVerb("Init");
    
    return true;
}



/**
 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
 *
 * @return		true = yes, false = no
 */
- (bool) GetUseMediatorLoginDialog
{
    return inst->GetUseMediatorLoginDialog ();
}

/**
 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseMediatorLoginDialog: (bool) enable
{
    inst->SetUseMediatorLoginDialog ( enable );
}


/**
 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @return		true = yes, false = no
 */
- (bool) GetMediatorLoginDialogDismissDisable
{
    return inst->GetMediatorLoginDialogDismissDisable ();
}

/**
 * Instruct Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMediatorLoginDialogDismissDisable: (bool) enable
{
    inst->SetMediatorLoginDialogDismissDisable ( enable );
}


/**
 * Query whether the native layer was build for release (or debug).
 *
 * @return	true = Release build, false = Debug build.
 */
- (bool) GetIsReleaseBuild
{
    return GetIsReleaseBuildN ();
}


- (const char *) GetVersionString {
    return GetVersionStringN ();
}


- (int) GetVersionMajor {
    return GetVersionMajorN ();
}


- (int) GetVersionMinor {
    return GetVersionMinorN ();
}

- (int) GetVersionRevision {
    return GetVersionRevisionN ();
}


- (void) SetAppStatus: (int) status {
    SetAppStatusN ( hEnvirons, status );
}


- (void) SetDeviceID: (int)deviceID
{
    CVerbArg ( "SetDeviceID: [0x%X]", deviceID );
    
    inst->SetDeviceID( deviceID );
}


- (bool) HasDeviceUID
{
    return inst->HasDeviceUID ();
}

- (bool) SetDeviceUID: (const char *)name
{
    return inst->SetDeviceUID ( name );
}


- (void) SetMediatorFilterLevel: (environs::MediatorFilter_t) level
{
    inst->SetMediatorFilterLevel ( level );
}


- (environs::MediatorFilter_t) GetMediatorFilterLevel {
    return inst->GetMediatorFilterLevel();
}

/**
 * Reset crypt layer and all created resources. Those will be recreated if necessary.
 * This method is intended to be called directly after creation of an Environs instance.
 *
 */
- (void) ResetCryptLayer {
    inst->ResetCryptLayer ();
}


- (int) GetDeviceID {
    return inst->GetDeviceID ();
}


- (int) GetDeviceIDFromMediator {
    return inst->GetDeviceIDFromMediator ();
}


- (NSString *) GetMediatorIP {
    return [[NSString alloc ] initWithUTF8String:inst->GetMediatorIP ()];
}


- (int) GetMediatorPort
{
    return inst->GetMediatorPort ();
}


- (void) SetMediator:(NSString *)ip Port:(unsigned short) port
{
    if ( !ip || !port )
        return;
    
    inst->SetMediator ( ip ? [ip UTF8String] : 0, port );
}


- (bool) RegisterAtMediators
{
    return inst->RegisterAtMediators();
}


- (bool) GetUseH264
{
    return inst->GetUseH264 ();
}


- (void) SetUseH264:(bool)usage
{
    CVerbArg ( "SetUseH264 %i", usage );
    
    inst->SetUseH264 ( usage );
}


#ifdef ENVIRONS_IOS

- (bool) GetUseSensors
{
    return GetUseSensorsN ( hEnvirons );
}

- (void) SetUseSensors:(bool)usage
{
    CVerbArg("SetUseSensors %i", usage);
    
    SetUseSensorsN ( hEnvirons, usage );
}

#endif


- (bool) GetUseCustomMediator
{
    return inst->GetUseCustomMediator ();
}


- (void) SetUseCustomMediator:(bool)usage
{
    CVerbArg("SetUseCustomMediator %i", usage);
    
    inst->SetUseCustomMediator ( usage );
}


- (bool) GetUseDefaultMediator
{
    return inst->GetUseDefaultMediator ();
}


- (void) SetUseDefaultMediator:(bool)usage
{
    CVerbArg("SetUseDefaultMediator %i", usage);
    
    inst->SetUseDefaultMediator ( usage );
}


- (void) SetUseCLSForMediator:(bool)usage
{
    CVerbArg("SetUseCLSForMediator %i", usage);
    
    inst->SetUseCLSForMediator ( usage );
}


- (bool) GetUseCLSForMediator
{
    return inst->GetUseCLSForMediator ();
}


- (void) SetUseCLSForDevices:(bool)usage
{
    CVerbArg("SetUseCLSForDevices %i", usage);
    
    inst->SetUseCLSForDevices ( usage );
}


- (bool) GetUseCLSForDevices
{
    return inst->GetUseCLSForDevices ();
}


- (void) SetUseCLSForDevicesEnforce:(bool)usage
{
    CVerbArg("SetUseCLSForDevicesEnforce %i", usage);
    
    inst->SetUseCLSForDevicesEnforce ( usage );
}


- (bool) GetUseCLSForDevicesEnforce
{
    return inst->GetUseCLSForDevicesEnforce ();
}


- (void) SetUseCLSForAllTraffic:(bool)usage
{
    CVerbArg("SetUseCLSForAllTraffic %i", usage);
    
    inst->SetUseCLSForAllTraffic ( usage );
}


- (bool) GetUseCLSForAllTraffic
{
    return inst->GetUseCLSForAllTraffic ();
}

/**
 * Instruct Environs to show log messages in the status log.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseNotifyDebugMessage:(bool)usage {
    inst->SetUseNotifyDebugMessage(usage);
}

/**
 * Query Environs settings that show log messages in the status log.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetUseNotifyDebugMessage {
    return inst->GetUseNotifyDebugMessage();
}

/**
 * Instruct Environs to create and write a log file in the working directory.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetUseLogFile:(bool)usage {
    inst->SetUseLogFile(usage);
}

/**
 * Query Environs settings that create and write a log file in the working directory.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetUseLogFile {
    return inst->GetUseLogFile();
}


/**
* Instruct Environs to log to stdout.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseLogToStdout : ( bool ) enable {
    inst->SetUseLogToStdout(enable);
}


/**
* Query Environs settings whether to log to stdout.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseLogToStdout {
    return inst->GetUseLogToStdout();
}


/**
* Instruct Environs to create DeviceLists that are used as UIAdapter by client code.
* Any changes of those lists are made within the applications main / UI thread context.
* Only DeviceList objects that are created after this call are affected.
* DeviceList objects created before this call remain using the setting at the time they are created.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseDeviceListAsUIAdapter : ( bool ) enable {
    inst->SetUseDeviceListAsUIAdapter(enable);
}


/**
* Query Environs settings whether to create DeviceLists that are used as UIAdapter by client code.
* Any changes of those lists are made within the applications main / UI thread context.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseDeviceListAsUIAdapter {
    return inst->GetUseDeviceListAsUIAdapter();
}


#ifdef ENVIRONS_OSX

/**
* Instruct Environs to use headless mode without worrying about UI thread.
*
* @param enable      true = enable, false = disable
*/
-( void ) SetUseHeadless : ( bool ) enable {
    inst->SetUseHeadless(enable);
}


/**
* Query Environs settings whether to use headless mode without worrying about UI thread.
*
* @return enable      true = enabled, false = disabled
*/
-( bool ) GetUseHeadless {
    return inst->GetUseHeadless();
}


/**
* Check for mediator logon credentials and query on command line if necessary.
*
* @param success      true = successful, false = failed
*/
-( bool ) QueryMediatorLogonCommandLine {
    return inst->QueryMediatorLogonCommandLine();
}

#endif


#ifdef ENVIRONS_IOS

- (bool) GetUseNativeDecoder
{
    return GetUseNativeDecoderN ( hEnvirons );
}


- (void) SetUseNativeDecoder:(bool)usage
{
    CVerbArg("SetUseNativeDecoder %i", usage);
    
    SetUseNativeDecoderN ( hEnvirons, usage );
}


- (bool) GetPortalTCP
{
    return inst->GetUsePortalTCP ();
}


- (void) SetPortalTCP:(bool)usage
{
    CVerbArg("SetPortalTCP %i", usage);
    
    inst->SetUsePortalTCP ( usage );
}


- (bool) GetUsePortalTCP
{
    return inst->GetUsePortalTCP ();
}


- (void) SetUsePortalTCP:(bool)usage
{
    CVerbArg("GetUsePortalTCP %i", usage);
    
    inst->SetUsePortalTCP ( usage );
}


- (bool) GetPortalAutoStart
{
    return inst->GetPortalAutoStart ();
}


- (void) SetPortalAutoStart:(bool)enable
{
    CVerbArg ( "SetPortalAutoStart %i", enable );
    
    inst->SetPortalAutoStart ( enable );
}


- (bool) GetPortalNativeResolution
{
    return inst->GetPortalNativeResolution ();
}


- (void) SetPortalNativeResolution:(bool)enable
{
    CVerbArg("SetPortalNativeResolution %i", enable);
    
    inst->SetPortalNativeResolution ( enable );
}

#endif


/**
 * Instruct Environs to output verbose debug logging.
 *
 * @param level      debug level 0 ... 16
 */
- (void) SetDebug:(int)value
{
    CLog("SetDebug");
    
    inst->SetDebug ( value );
}


/**
 * Get output debug level.
 *
 * @return level      debug level 0 ... 16
 */
- (int) GetDebug
{
    return inst->GetDebug ();
}

/**
* Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
* Increasing this value may help to handle worse networks which suffer from large latencies.
*
* @param   timeout
*/
- (void) SetNetworkConnectTimeout:(int)timeout
{
    CVerb("SetNetworkConnectTimeout");
    
    inst->SetNetworkConnectTimeout ( timeout );
}


/**
 * Get platform that the app is running on.
 *
 * @return 	enum of type Environs.Platforms
 */
- (int) GetPlatform
{
    return inst->GetPlatform ();
}

/**
 * Set the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Valid type are enumerated in Environs.Platforms.*
 *
 * @param	platform	Environs.Platforms.*
 */
- (void) SetPlatform: (int) platform
{
    inst->SetPlatform ( platform );
}

/**
 * Set/Remove the location-node flag to the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Flag: Environs.Platforms.LocationNode_Flag
 *
 * @param	isLocationNode	true or false
 */
- (void) SetIsLocationNode: (bool) isLocationNode
{
    inst->SetIsLocationNode ( isLocationNode );
}


- (void) SetUseAuthentication: (bool) enable
{
    inst->SetUseAuthentication ( enable );
}


+ (NSMutableDictionary *)GetSecItemFormat:(NSDictionary *)inDict
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary:inDict];
    
    [dict setObject:(__bridge id)kSecClassGenericPassword forKey:(__bridge id)kSecClass];
    
    NSString *passwordString = [inDict objectForKey:(__bridge id)kSecValueData];
    [dict setObject:[passwordString dataUsingEncoding:NSUTF8StringEncoding] forKey:(__bridge id)kSecValueData];
    
    return dict;
}


+ (bool) SetKeychainItem:(NSString *) value forKey:(id)key
{
    while ( value ) {
        const char * appName = GetApplicationNameN ( 1 );
        if ( !appName )
            break;
        
        NSMutableDictionary * query = [[NSMutableDictionary alloc] init];
        NSMutableDictionary * keyChain = [[NSMutableDictionary alloc] init];
        if ( !query || !keyChain )
            break;
        
        [query setObject:(__bridge id)kSecClassGenericPassword forKey:(__bridge id)kSecClass];
        [query setObject:[[NSString alloc ] initWithUTF8String:appName] forKey:(__bridge id)kSecAttrGeneric];
        
        [query setObject:(__bridge id)kSecMatchLimitOne forKey:(__bridge id)kSecMatchLimit];
        [query setObject:(id)kCFBooleanTrue forKey:(__bridge id)kSecReturnAttributes];
        
        NSDictionary * tmp = [NSDictionary dictionaryWithDictionary:query];
        if ( !tmp )
            break;
        
        NSMutableDictionary * outDict = NULL;
        CFDictionaryRef cfoutDicst = NULL;
        
        if ( SecItemCopyMatching ( (__bridge CFDictionaryRef)tmp, (CFTypeRef *)&cfoutDicst) == noErr )
        {
            outDict = (__bridge NSMutableDictionary *) cfoutDicst;
            
            [keyChain setObject:[[NSString alloc ] initWithUTF8String:appName] forKey:(__bridge id)kSecAttrGeneric];
        }
        else
        {
            keyChain = [NSMutableDictionary dictionaryWithDictionary:outDict];
            
            [keyChain setObject:(id)kCFBooleanTrue forKey:(id)CFBridgingRelease(kSecReturnData)];
            [keyChain setObject:(__bridge id)kSecClassGenericPassword forKey:(__bridge id)kSecClass];
            
            NSData *dictData = NULL;
            CFDataRef cfoutDicst = NULL;
            
            if ( SecItemCopyMatching((__bridge CFDictionaryRef)keyChain, (CFTypeRef *)&cfoutDicst) == noErr )
            {
                dictData = (__bridge NSData *)cfoutDicst;
                
                [keyChain removeObjectForKey:(__bridge id)kSecReturnData];
                
                NSString *password = [[NSString alloc] initWithBytes:[dictData bytes] length:[dictData length] encoding:NSUTF8StringEncoding];
                [keyChain setObject:password forKey:(__bridge id)kSecValueData];
            }
            else
                break;
        }
        
        id obj = [keyChain objectForKey:key];
        if (![obj isEqual:value])
        {
            [keyChain setObject:obj forKey:key];
            
            NSDictionary *attr = NULL;
            CFDictionaryRef cfoutDict = NULL;
            NSMutableDictionary *updateItem = NULL;
            //OSStatus result;
            
            if ( SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef *)&cfoutDict) == noErr )
            {
                attr = (__bridge NSDictionary *)cfoutDict;
                updateItem = [NSMutableDictionary dictionaryWithDictionary:attr];
                
                [updateItem setObject:[query objectForKey:(__bridge id)kSecClass] forKey:(__bridge id)kSecClass];
                
                NSMutableDictionary *tmp = [self GetSecItemFormat:keyChain];
                [tmp removeObjectForKey:(__bridge id)kSecClass];
                
#if TARGET_IPHONE_SIMULATOR == 1
                [tmp removeObjectForKey:(__bridge id)kSecAttrAccessGroup];
#endif
                //result =
                SecItemUpdate((__bridge CFDictionaryRef)updateItem, (__bridge CFDictionaryRef)tmp);
            }
            else
            {
                //result =
                SecItemAdd((__bridge CFDictionaryRef)[self GetSecItemFormat:keyChain], NULL);
            }
            
        }
        
    }
    
    return false;
}


/**
 * Enable or disable anonymous logon to the Mediator.
 *
 * @param 	enable A boolean that determines the target state.
 */
- (void) SetUseMediatorAnonymousLogon: (bool) enable
{
    inst->SetUseMediatorAnonymousLogon ( enable );
}

/**
 * Retrieve a boolean that determines whether Environs makes use of anonymous logon to Mediator services.
 *
 * @return		true = yes, false = no
 */
- (bool) GetUseMediatorAnonymousLogon
{
    return inst->GetUseMediatorAnonymousLogon ();
}


- (bool) SetUserName:(const char *) username
{
    return inst->SetMediatorUserName ( username );
}


- (bool) SetMediatorUserName:(NSString *) userName
{
    if ( !userName )
        return false;
    
    return inst->SetMediatorUserName ( [[userName lowercaseString] UTF8String] );
}


- (NSString *) GetMediatorUserName {
    return [[NSString alloc ] initWithUTF8String: inst->GetMediatorUserName ()];
}


- (bool) SetUserPassword:(const char * )password
{
    return inst->SetMediatorPassword ( password );
}


- (bool) SetMediatorPassword:(NSString * )password
{
    if ( !password )
        password = @"";
    
    return inst->SetMediatorPassword ( [password UTF8String] );
}



/**
 * Enable or disable device list update notifications from Mediator layer.
 * In particular, mobile devices should disable notifications if the devicelist is not
 * visible to users or the app transitioned to background.
 * This helps recuding cpu load and network traffic when not required.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMediatorNotificationSubscription : (bool) enable
{
    inst->SetMediatorNotificationSubscription ( enable );
}


/**
 * Get subscription status of device list update notifications from Mediator layer.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetMediatorNotificationSubscription
{
    return inst->GetMediatorNotificationSubscription ();
}


/**
 * Enable or disable short messages from Mediator layer.
 * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
 * This helps recuding cpu load and network traffic when not necessary.
 *
 * @param enable      true = enable, false = disable
 */
- (void) SetMessagesSubscription : (bool) enable
{
    inst->SetMessagesSubscription ( enable );
}


/**
 * Get subscription status of short messages from Mediator layer.
 *
 * @return enable      true = enable, false = disable
 */
- (bool) GetMessagesSubscription
{
    return inst->GetMessagesSubscription ();
}



/**
 * Add an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
- (void) AddObserver:(id<EnvironsObserver>) obs
{
    CVerb ( "AddObserver" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( ![environsObservers containsObject:obs] ) {
        [environsObservers addObject:obs];
        
        if ( [environsObservers count] <= 1 ) {
            inst->AddObserver ( observer.get () );
        }
    }
    
    [envObserverLock unlock];
}



/**
 * Remove an observer for communication with Environs and devices within the environment.
 *
 * @param observer Your implementation of EnvironsObserver.
 */
- (void) RemoveObserver:(id<EnvironsObserver>) obs
{
    CVerb ( "RemoveObserver" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( [environsObservers containsObject:obs] ) {
        [environsObservers removeObject:obs];
        
        if ( [environsObservers count] <= 0 ) {
            inst->RemoveObserver ( observer.get () );
        }
    }
    
    [envObserverLock unlock];
}


/**
 * Add an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
- (void) AddObserverForMessages:(id<EnvironsMessageObserver>) obs
{
    CVerb ( "AddObserverForMessages" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( ![environsObserversForMessages containsObject:obs] ) {
        [environsObserversForMessages addObject:obs];
        
        if ( [environsObserversForMessages count] <= 1 ) {
            inst->AddObserverForMessages ( observerForMessages.get () );
        }
    }
    
    [envObserverLock unlock];
}

/**
 * Remove an observer for receiving messages.
 *
 * @param observer Your implementation of EnvironsMessageObserver.
 */
- (void) RemoveObserverForMessages:(id<EnvironsMessageObserver>) obs
{
    CVerb ( "RemoveObserverForMessages" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( [environsObserversForMessages containsObject:obs] ) {
        [environsObserversForMessages removeObject:obs];
        
        if ( [environsObserversForMessages count] <= 0 ) {
            inst->RemoveObserverForMessages ( observerForMessages.get () );
        }
    }
    
    [envObserverLock unlock];
}


/**
 * Add an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) AddObserverForData:(id<EnvironsDataObserver>) obs
{
    CVerb ( "AddObserverForData" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( ![environsObserversForData containsObject:obs] ) {
        [environsObserversForData addObject:obs];
        
        if ( [environsObserversForData count] <= 1 ) {
            inst->AddObserverForData ( observerForData.get () );
        }
    }
    
    [envObserverLock unlock];
}

/**
 * Remove an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) RemoveObserverForData:(id<EnvironsDataObserver>) obs
{
    CVerb ( "RemoveObserverForData" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( [environsObserversForData containsObject:obs] ) {
        [environsObserversForData removeObject:obs];
        
        if ( [environsObserversForData count] <= 0 ) {
            inst->RemoveObserverForData ( observerForData.get () );
        }
    }
    
    [envObserverLock unlock];
}


/**
 * Add an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) AddObserverForSensorData:(id<EnvironsSensorObserver>) obs
{
    CVerb ( "AddObserverForSensorData" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( ![environsObserversForSensor containsObject:obs] ) {
        [environsObserversForSensor addObject:obs];
        
        if ( [environsObserversForSensor count] <= 1 ) {
            inst->AddObserverForSensorData ( observerForSensors.get () );
        }
    }
    
    [envObserverLock unlock];
}

/**
 * Remove an observer for receiving data buffers and files.
 *
 * @param observer Your implementation of EnvironsDataObserver.
 */
- (void) RemoveObserverForSensorData:(id<EnvironsSensorObserver>) obs
{
    CVerb ( "RemoveObserverForSensorData" );
    
    if ( obs == nil ) return;
    
    [envObserverLock lock];
    
    if ( [environsObserversForSensor containsObject:obs] ) {
        [environsObserversForSensor removeObject:obs];
        
        if ( [environsObserversForSensor count] <= 0 ) {
            inst->RemoveObserverForSensorData ( observerForSensors.get () );
        }
    }
    
    [envObserverLock unlock];
}


- (void) SetPorts:(int)comPort DataPort:(int)dataPort
{
    inst->SetPorts ( comPort, dataPort );
}


- (environs::Status_t) GetStatus
{
    return inst->GetStatus ();
}


- (void) SetApplication: (const char *) name
{
    inst->SetApplicationName ( name );
}

- (void) SetApplicationName: (const char *) name
{
    inst->SetApplicationName ( name );
}


- (const char *) GetApplicationName
{
    return inst->GetApplicationName();
}


- (void) SetAreaName: (const char *) name
{
    inst->SetAreaName ( name );
}


- (const char *) GetAreaName
{
    return inst->GetAreaName();
}

- (void) SetDeviceName: (const char *) name
{
    inst->SetDeviceName ( name );
}


- (void) Stop
{
    CVerb("Stop");
    
    inst->Stop ();
}


- (void) Dispose
{
    if ( inst )
    {
        inst->Stop ();
        
        inst->DisposeInstance();
    }
    
    instancesPlt [ hEnvirons ] = nil;
}


- (void) Start
{
    CVerb ( "Start" );
    
    inst->Start ();
}



- (unsigned int) GetIPAddress {
    return inst->GetIPAddress ();
}

- (unsigned int) GetSubnetMask {
    return inst->GetSubnetMask();
}



- (NSString *) GetSSID {
    return environs::API::GetNSSID ( false );
}

- (NSString *) GetSSIDDesc {
    return environs::API::GetNSSID ( true );
}


/**
 * Called by Reachability whenever status changes.
 */
- (void) reachabilityChanged:(NSNotification *)note
{
}


#ifdef ENVIRONS_IOS


#else


- (int) SetUseTracker:(environs::Call_t) async module:(const char *) moduleName
{
    return inst->SetUseTracker ( async, moduleName );
}


- (int) GetUseTracker:(const char *) moduleName
{
    return inst->GetUseTracker ( moduleName );
}

- (EBOOL) DisposeTracker:(environs::Call_t) async module:(const char *) moduleName
{
    return inst->DisposeTracker ( async, moduleName );
}

- (EBOOL) PushTrackerCommand:(environs::Call_t) async module:(int) index cmd:(int) command
{
    return inst->PushTrackerCommand ( async, index, command );
}

#endif





/**
 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
- (bool) SetUseTouchRecognizer: (const char *) moduleName Status:(bool)enable
{
    return inst->SetUseTouchRecognizer ( moduleName, enable);
}


/**
 * Use default encoder, decoder, capture, render modules.
 *
 * @return  success
 */
- (bool) SetUsePortalDefaultModules
{
    return inst->SetUsePortalDefaultModules ( );
}



/**
 * Use encoder module with the name moduleName. (libEnv-Enc...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
- (bool) SetUseEncoder: (const char *) moduleName
{
    return inst->SetUseEncoder ( moduleName );
}


/**
 * Use decoder module with the name moduleName. (libEnv-Dec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
- (bool) SetUseDecoder: (const char *) moduleName
{
    return inst->SetUseDecoder ( moduleName );
}

/**
 * Use capture module with the name moduleName. (libEnv-Cap...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
- (bool) SetUseCapturer: (const char *) moduleName
{
    return inst->SetUseCapturer ( moduleName );
}


/**
 * Use render module with the name moduleName. (libEnv-Rend...).
 *
 * @param	moduleName	the name of the module
 * @return  success
 */
- (bool) SetUseRenderer: (const char *) moduleName
{
    return inst->SetUseRenderer ( moduleName );
}


/**
 * Create a new collection that holds all devices of given list type. This list ist updated dynamically by Environs.
 * After client code is done with the list, the list->Release () method MUST be called by the client code,
 * in order to release the resource (give ownership) back to Environs.
 *
 * @return Collection of IDeviceInstance objects
 */
- (DeviceList *) CreateDeviceList : (environs::DeviceClass_t) MEDIATOR_DEVICE_CLASS_
{
    DeviceList * iosList = [[DeviceList alloc] init];
    if ( !iosList )
        return 0;
    
    environs::lib::DeviceList * list = inst->CreateDeviceList ( MEDIATOR_DEVICE_CLASS_ );
    if ( !list )
        return 0;
    
    sp ( environs::lib::DeviceList ) sp1 ( list );
    
    if ( ![iosList SetInst:sp1] )
        return 0;
    
    return iosList;
}




-(environs::DeviceDisplay *) GetDeviceDisplayProps:(int) nativeID
{
    return inst->GetDeviceDisplayProps ( nativeID );
}

/** Default value for each DeviceInstance after object creation. */
- (bool) GetAllowConnectDefault
{
    return inst->GetAllowConnectDefault ();
}

/** Default value for each DeviceInstance after object creation. */
- (void) SetAllowConnectDefault:(bool) value
{
    inst->SetAllowConnectDefault ( value );
}


/**
 * Connect to device with the given ID and a particular application environment.
 *
 * @param deviceID	Destination device ID
 * @param areaName Project name of the application environment
 * @param appName	Application name of the application environment
 * @param async	    Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @return status	0: Connection can't be conducted (maybe environs is stopped or the device ID is invalid) &nbsp;
 * 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					2: A new connection has been triggered and is in progress
 */
-(int) DeviceConnect:(int) deviceID areaName:(const char *) areaName appName:(const char *) appName async:(environs::Call_t) async
{
    return inst->DeviceConnect ( deviceID, areaName, appName, async );
}


/**
 * Set render callback.
 *
 * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
 * @param callback		The pointer to the callback.
 * @param callbackType	A value of type RENDER_CALLBACK_TYPE_* that tells the portal receiver what we actually can render..
 * @return				true = success, false = failed.
 */
-(bool) SetRenderCallback:(environs::Call_t) async portalID:(int) portalID callback:(void *)callback callbackType:(environs::RenderCallbackType_t) callbackType
{
    return inst->SetRenderCallback ( async, portalID, callback, callbackType );
}


/**
 * Release render callback delegate or pointer
 *
 * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
 * @param callback		A delegate that manages the callback.
 * @return				true = success, false = failed.
 */
-(bool) ReleaseRenderCallback:(environs::Call_t) async portalID:(int) portalID
{
    return inst->ReleaseRenderCallback ( async, portalID );
}


/**
 * Start streaming of portal to or from the portal identifier (received in notification).
 *
 * @param async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
 * @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
 *
 * @return success
 */
-(bool) StartPortalStream:(environs::Call_t) async portalID:(int) portalID
{
    return inst->StartPortalStream ( async, portalID );
}


/**
 * Stop streaming of portal to or from the portal identifier (received in notification).
 *
 * @param 	async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
 * @param 	nativeID    The native device id of the target device.
 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 					    Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
-(bool) StopPortalStream:(environs::Call_t) async nativeID:(int) nativeID portalID:(int) portalID
{
    return inst->StopPortalStream ( async, nativeID, portalID );
}


/**
 * Get the status, whether the device (id) has established an active portal
 *
 * @param 	nativeID    The device id of the target device.
 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
 * @return	success 	true = yes, false = no
 */
-(bool) GetPortalEnabled:(int) nativeID portalType:(int) portalType
{
    return inst->GetPortalEnabled ( nativeID, portalType );
}


/**
 * Get the number of devices that are currently connected to our device.
 *
 * @return	Count of connected devices
 */
-(int) GetConnectedDevicesCount
{
    return inst->GetConnectedDevicesCount ( );
}


/**
 * Get enabled status for stream encoding.
 *
 * @return	enabled
 */
-(bool) GetUseStream
{
    return inst->GetUseStream ( );
}


#ifdef DISPLAYDEVICE
/**
 * Get platform support for OpenCL.
 *
 * @return	enabled
 */
-(bool) GetUseOpenCL
{
    return inst->GetUseOpenCL ( );
}


/**
 * Switch platform support for OpenCL rendering.
 *
 * @param enable
 */
-(void) SetUseOpenCL:(bool) enable
{
    inst->SetUseOpenCL ( enable );
}

#endif


-(const char *) GetFilePathNative:(int) nativeID fileID:(int)fileID
{
    return inst->GetFilePathNative ( nativeID, fileID );
}


-(char *) GetFilePath:(int) nativeID  fileID:(int) fileID
{
    return inst->GetFilePath ( nativeID, fileID );
}



/**
 * Load the file that is assigned to the fileID into a byte array.
 *
 * @param nativeIDOK		Indicates that the nativeOrDeviceID represents a nativeID.
 * @param nativeID		The native id of the device.
 * @param deviceID		The device id of the device.
 * @param areaName		Area name of the application environment.
 * @param appName		Application name of the application environment.
 * @param fileID			The id of the file to load (given in the onData receiver).
 * @param size			An int pointer, that receives the size of the returned buffer.
 * @return byte-array
 */
-(NSData *) GetFile: (bool) nativeIDOK nativeID:(int)nativeID deviceID:(int)deviceID areaName:(const char *)areaName appName:(const char *)appName fileID:(int)fileID size:(int *)size
{
    CVerb ( "GetFile" );
    
    int capacity_ = 0;
    
    if ( nativeIDOK )
        environs::API::GetFileNativeN ( hEnvirons, nativeID, fileID, 0, &capacity_ );
    else
        environs::API::GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, 0, &capacity_ );
    
    if ( !capacity_ )
        return nill;
    
    NSMutableData * data = [NSMutableData dataWithCapacity: 30];
    if ( !data || !data.mutableBytes )
        return nill;
    
    *size = capacity_;
    
    if ( nativeIDOK )
        environs::API::GetFileNativeN ( hEnvirons, nativeID, fileID, ( void * ) data.mutableBytes, &capacity_ );
    else
        environs::API::GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, ( void * ) data.mutableBytes, &capacity_ );
    
    *size = capacity_;
    return data;
}



/**
 * Enable dispatching of sensor events from ourself.
 * Events are send if Environs instance is started stopped if the Environs instance has stopped.
 *
 * Note: If you request GPS locations, then you must add the following keys to your plist
    <key>NSLocationWhenInUseUsageDescription</key>
    <string>Reason as text</string>
 *
 * @param sensorType            A value of type environs.SensorType.
 * @param enable 				true = enable, false = disable.
 *
 * @return success true = enabled, false = failed.
 */
- (bool) SetSensorEvent: (environs::SensorType_t) sensorType enable:(bool) enable
{
    return inst->SetSensorEvent ( sensorType, enable );
}


/**
 * Determine whether the given sensorType is available.
 *
 * @param sensorType A value of type environs::SensorType_t.
 *
 * @return success true = available, false = not available.
 */
- (bool) IsSensorAvailable: (environs::SensorType_t) sensorType
{
    return inst->IsSensorAvailable ( sensorType );
}

@end









