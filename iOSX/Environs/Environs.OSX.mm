/**
 * Environs.ios.mm
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#import <Environs.iOSX.h>

#ifdef ENVIRONS_OSX

#import <Foundation/Foundation.h>
#import <CoreWLAN/CoreWLAN.h>

#include "Environs.Obj.h"
#include "Environs.AV.Context.h"
#include "Environs.Utils.h"
#include "Interfaces/IPortal.Decoder.h"
#include "Wifi.Observer.h"
using namespace environs;

#include "Environs.Lib.h"
using namespace environs::API;

#include "Core/Byte.Buffer.h"

#define	CLASS_NAME 	"Environs.OSX . . . . . ."


#ifdef NATIVE_WIFI_OBSERVER_THREAD

@interface OSXWifiListener : NSObject <NSApplicationDelegate>
{
@public
    NSString * name;
}

@property (retain) CWInterface * wifiInterface;

- ( void ) EnableNotifications;
- ( void ) DisableNotifications;

- ( void ) OnWifiNotification:(NSNotification*)note;

@end

#endif


namespace environs
{
    extern bool CreateAppID ( char * buffer, unsigned int bufSize );
    
    extern bool AllocNativePlatformIOSX ();
    
    void RemoveKeyMonitor ();
    void AddKeyMonitor ();

    
    namespace API {
        extern void BridgeForData ( int hInst, int nativeID, int type, int fileID, const char * fileDescriptor, int size );
        extern void BridgeForUdpData ( int hInst, int nativeID, SensorFrame * pack );
        
        extern void BridgeForMessage ( int hInst, int nativeID, int type, const void * message, int length );
        extern void BridgeForMessageExt ( int hInst, int deviceID, const char * areaName, const char * appName, int type, const void * message, int length );
        
        extern void BridgeForNotify ( int hInst, int nativeID, int notification, int source, void * contextPtr );
        extern void BridgeForNotifyExt ( int hInst, int deviceID, const char * areaName, const char * appName, int notification, int source, void * contextPtr );
        
        extern void BridgeForStatusMessage ( int hInst, const char * message );
        
        
        NSString * GetNSSID ( bool desc )
        {
            @autoreleasepool
            {
                CWInterface * wifi = [[CWWiFiClient sharedWiFiClient] interface];
                //CWInterface * wifi = [CWInterface interface];
                if ( !wifi )
                    return nil;
                
                if ( desc )
                    return [[NSString alloc ] initWithFormat:@"%@ (%@/%i/%i dB)", wifi.ssid == nil ? @"Unknown" : wifi.ssid, wifi.security > kCWSecurityNone && wifi.security < kCWSecurityUnknown ? @"E" : @"U", (int)wifi.wlanChannel.channelNumber, (int)wifi.rssiValue];
                
                return  wifi.ssid == nil ? @"" : [wifi.ssid copy];//[[NSString alloc ] initWithString:wifi.ssid];
            }
        }
        
        
        const char * GetSSID ( bool desc )
        {
            @autoreleasepool
            {
                NSString * ssid = GetNSSID ( desc );
                if ( ssid == nil )
                    return "";
                
                static char pssid [64];
                
                if ( snprintf ( pssid, sizeof(pssid), "%s", [ssid UTF8String] ) < 0 )
                    return "";
                
                return pssid;
            }
        }
    }

    
    /**
     * Perform SDK checks to detect ...
     *
     */
    void DetectSDKs ()
    {
        @autoreleasepool
        {
            if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber10_9) {
                native.sdks = 1010;
            }
            else if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber10_8_4 ) {
                native.sdks = 1009;
            }
            else
                native.sdks = 1000;
        }
    }

    
    /**
     * Perform platform checks to detect ...
     *
     */
    void DetectPlatform ()
    {
        if ( native.platform != Platforms::Unknown )
            return;
        
        int platform = Platforms::Display_Flag;
        
        
        native.display.orientation = DISPLAY_ORIENTATION_LANDSCAPE;
        platform |= Platforms::MacBook_Flag;
        
        native.platform = (environs::Platforms_t) platform;
    }
    
    
    bool DetermineAndInitWorkDir ()
    {
        bool success = false;
        bool addContents = false;
        
        @autoreleasepool
        {
            NSString * workDir = nil;
            
            // Get working directory
            NSFileManager * fileMan = [[NSFileManager alloc] init];
            if ( fileMan )
            {
                workDir = [fileMan currentDirectoryPath];
                if ( workDir ) {
                    if ( [workDir length] < 4 )
                        workDir = nil;
                }
            }
            
            if ( workDir == nil ) {
                workDir = NSBundle.mainBundle.bundlePath;
                addContents = true;
            }

#ifdef OSX_USE_MANUAL_REF_COUNT
            bool releaseTmp = false;
#endif
            if ( workDir )
            {
                if ( ![workDir hasSuffix:@"/"] ) {
                    workDir = [[NSString alloc ] initWithFormat:@"%@/", workDir];
#ifdef OSX_USE_MANUAL_REF_COUNT
                    releaseTmp = true;
#endif
                }
                
                if ( addContents ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
                    NSString * toDelete = workDir;
#endif
                    workDir = [[NSString alloc ] initWithFormat:@"%@Contents/", workDir];

#ifdef OSX_USE_MANUAL_REF_COUNT
                    if ( toDelete && releaseTmp ) {
                        [toDelete release];
                        toDelete = nil;
                    }
                    releaseTmp = true;
#endif
                }
                
                const char * cDir = [workDir UTF8String];
                
                CVerbArg ( "DetermineAndInitWorkDir: app working path [%s]", cDir );
                InitWorkDirN ( cDir );
                
                InitStorageN ( cDir );
                
                NSString * libDir = nil;
                
                Dl_info info;
                if ( dladdr ( &native, &info ) ) {
                    libDir = [[NSString alloc ] initWithUTF8String:info.dli_fname];
                    
                    if ( libDir ) {
                        NSRange range = [libDir rangeOfString:@"/" options:NSBackwardsSearch];
                        if ( range.length != NSNotFound ) {
                            NSString * libDir1 = [libDir substringToIndex:range.location + 1];
                            
                            InitLibDirN ( [libDir1 UTF8String] );

#ifdef OSX_USE_MANUAL_REF_COUNT
                            [libDir1 release];
#endif
                        }
#ifdef OSX_USE_MANUAL_REF_COUNT
                        [libDir release];
#endif
                    }
                }
                
                success = true;
            }
            else {
                CErr ( "DetermineAndInitWorkDir: Failed to get application directory." );
            }
#ifdef OSX_USE_MANUAL_REF_COUNT
            if ( fileMan )
                [fileMan release];
#endif
        }
        
        return success;
    }
    
    
    bool AllocNativePlatform ()
    {
        CInfo ( "AllocNativePlatform" );
        
        if ( !*native.deviceUID ) {
            CreateAppID ( native.deviceUID, sizeof ( native.deviceUID ) );
            
            environs::API::SetDeviceUIDN ( native.deviceUID );
            
            SaveConfig ();
        }
        
        return AllocNativePlatformIOSX ();
    }
    
    
    void DeallocNativePlatform ()
    {
        CVerb ( "DeallocNativePlatform" );
        
        RemoveKeyMonitor ();
    }

    
    void InitIOSX ()
    {
        CVerb ( "InitIOSX" );
        
        @autoreleasepool
        {
            NSString * computerName = CFBridgingRelease ( CSCopyMachineName() );
            if ( computerName ) {
                strlcpy ( native.deviceName, [computerName UTF8String], sizeof ( native.deviceName ) );
                
                //CFRelease ( (CFStringRef)computerName );
            }
            else
                snprintf ( native.deviceName, sizeof ( native.deviceName ), "Unknown-Computer-Name-Nr-%i", rand ( ) );
        }
    }
    
    
    void UpdateDeviceParams ()
    {
        CVerb ( "UpdateDeviceParams" );
        
        @autoreleasepool
        {
            NSScreen * screen = [NSScreen mainScreen];
            
            float scale = screen.backingScaleFactor;
            
            NSRect mainFrame = [screen frame];
            
            int screen_width = mainFrame.size.width * scale;
            int screen_height = mainFrame.size.height * scale;
            
            float dpi = 300;
            
            native.display.width = screen_width;
            native.display.height = screen_height;
            
            float width_mm = (float)(((float)screen_width / dpi) * 25.4f);
            float height_mm = (float)(((float)screen_height / dpi) * 25.4f);
            
            native.display.width_mm = width_mm;
            native.display.height_mm = height_mm;
            
            //setDeviceDims ( screen_width, screen_height, width_mm, height_mm, 0, 0 );
            
            CVerbArg ( "UpdateDeviceParams: determined width %ipx, height %ipx, width_mm=%f, height_mm=%f", native.display.width, native.display.height, width_mm, height_mm );
        }
    }
    
    
    void SetRenderSurfaceIOSX ( void * surface )
    {
        CVerb ( "SetRenderSurfaceIOSX" );
    }
    
#ifndef USE_OPENSSL
    /*bool UpdateKeyAndCert ( char * priv, char * cert )
    {
        return true;
    }*/
#endif
    
    
    OSStatus EncryptMessageX ( SecKeyRef publicKey, unsigned int certProp, char * msg, size_t msgLen, char * ciphers, size_t *ciphersLen )
    {
        CVerb ( "EncryptMessageX" );
        
        SecTransformRef encrypter   = 0;
        CFDataRef       cfMsg       = 0;
        CFErrorRef      error       = 0;
        NSData *        encrypted   = 0;
        CFDataRef empty_parameters  = 0;
        
        do
        {
            //cfMsg = CFDataCreate ( kCFAllocatorDefault, (const unsigned char *)msg, (msgLen + 1) );
            //cfMsg = CFDataCreate ( kCFAllocatorDefault, (const unsigned char *)msg, msgLen );
            cfMsg = (__bridge CFDataRef)[NSData dataWithBytes:msg length:msgLen + 1];
            if ( !cfMsg ) break;
            
            encrypter = SecEncryptTransformCreate ( publicKey, &error ) ;
            if ( error ) break;
            
            CFStringRef pad = 0;
            if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
                pad = kSecPaddingOAEPKey;
            else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1 )
                pad = kSecPaddingPKCS1Key;
            else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1SHA1 )
                pad = kSecPaddingPKCS5Key;
            else if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1SHA256 )
                pad = kSecPaddingPKCS7Key;
            else
                pad = kSecPaddingNoneKey;
            
            
            SecTransformSetAttribute ( encrypter, kSecPaddingKey, pad, &error );
            if ( error ) break;
            
            empty_parameters = CFDataCreate ( NULL, NULL, 0 );
            
            SecTransformSetAttribute ( encrypter, kSecOAEPEncodingParametersAttributeName, empty_parameters, &error );
            if ( error ) break;
            
            SecTransformSetAttribute ( encrypter, kSecTransformInputAttributeName, cfMsg, &error ) ;
            if ( error ) break;
            
            encrypted = CFBridgingRelease ( SecTransformExecute ( encrypter, &error ) );
            if ( error ) break;
            
            *ciphersLen = [encrypted length];
            
            CLogArg ( "EncryptMessage: size [%i]", [encrypted length] );
        }
        while ( 0 );
        
        if ( empty_parameters )
            CFRelease ( empty_parameters );
        
        if ( encrypter )
            CFRelease ( encrypter );
        
        if ( error ) {
            CFShow ( error );
            return 1;
        }
        
        return 0;
    }
    
    
    bool RenderDecoderToSurface ( void * _surface, void * decoderOrByteBuffer )
    {
        CVerbVerb ( "RenderDecoderToSurface" );
        
        bool success = false;
        
        IPortalDecoder * decoder = (IPortalDecoder *) decoderOrByteBuffer;
        
        char * avData = (char *) decoder->avContext;
        if ( !avData )
            avData = (char *) decoder->avContextTemp;
        if ( !avData )
            return false;
        
        unsigned int        width;
        unsigned int        height;
        char            *   rawdata;
        
        if ( decoder->avContextType == DECODER_AVCONTEXT_TYPE_PIXELS ) {
            width = decoder->width;
            height = decoder->height;
            rawdata = avData;
        }
        else
            return false;
        
        @autoreleasepool
        {
            CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
            
            CFDataRef data = CFDataCreateWithBytesNoCopy ( kCFAllocatorDefault,
                                                          (const UInt8 *)rawdata,
                                                          width * height * 3,
                                                          kCFAllocatorNull );
            
            CGDataProviderRef provider = CGDataProviderCreateWithCFData ( data );
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB ();
            
            CGImageRef cgImage = CGImageCreate ( width,
                                                height,
                                                8,
                                                24,
                                                width * 3,
                                                colorSpace,
                                                bitmapInfo,
                                                provider,
                                                NULL,
                                                NO,
                                                kCGRenderingIntentDefault );
            CGColorSpaceRelease ( colorSpace );
            
            id view = (__bridge id)_surface;
            
            CGSize cgSize = CGSizeMake ( width, height );
            
            NSImage * image = [[NSImage alloc] initWithCGImage:cgImage size:cgSize];
            /*
             NSData * imageData = [[NSData alloc] initWithBytes:rawdata length:width * height * 3 ];
             if ( !imageData )
             return false;
             
             NSBitmapImageRep * imageRep = [NSBitmapImageRep imageRepWithData:imageData];
             if ( !imageRep )
             return false;
             
             NSSize imageSize = NSMakeSize(CGImageGetWidth([imageRep CGImage]), CGImageGetHeight([imageRep CGImage]));
             
             NSImage * image = [[NSImage alloc] initWithSize:imageSize];
             [image addRepresentation:imageRep];
             
             //NSImage * image = [[NSImage alloc] initWithData:nsdata];
             */
            
            try {
                if (image)
                {
                    @autoreleasepool
                    {
                        if ( [view isMemberOfClass:[NSImageView class]] ) {
                            dispatch_sync(dispatch_get_main_queue(), ^{
                                
                                ((NSImageView *)view).image = image;
                                
                                //NSColor * backgroundColor = [NSColor colorWithPatternImage:image];
                                
                                //[image drawInRect:[view bounds]];
                                //[image drawLayer:((NSImageView *)view).layer inContext:0];
                                //[image lockFocus];
                                
                                //[image drawInRect:[((NSImageView *)view) bounds] fromRect:NSMakeRect(0, 0, width, height) operation:NSCompositeSourceOver fraction:1];
                                
                                //[image unlockFocus];
                            });
                        }
                        else {
                            if ( [view isMemberOfClass:[NSView class]] ) {
                                dispatch_sync(dispatch_get_main_queue(), ^{
                                    
                                    //NSColor *backgroundColor = [NSColor colorWithPatternImage:image];
                                    
                                    [((NSView *)view) setWantsLayer:YES];
                                    ((NSView *)view).layer.contents = image;
                                    
                                    //[image drawInRect:[surface bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1];
                                });
                            }
                        }
                    }
                }
            }
            catch ( NSException * ie ) {
                NSLog ( @"RenderDecoderToSurface: Exception: %@", ie );
            }
            
            CGImageRelease ( cgImage );
            CGDataProviderRelease ( provider );
            CFRelease ( data );
        }
        
        return success;
    }
    
    
    bool RenderImageToSurface ( void * _surface, void * byteBuffer )
    {
        CVerbVerb ( "renderImageToSurface" );
        
        @autoreleasepool
        {
            NSView * surface = (__bridge NSView *) _surface;
            if ( surface == nil )
                return false;
            
            ByteBuffer * buffer = (ByteBuffer *)byteBuffer;
            
            try {
                NSImage * image = [[NSImage alloc] initWithData:[NSData dataWithBytes: BYTEBUFFER_DATA_POINTER_START ( buffer ) length:buffer->payloadSize]];
                if (image)
                {
                    @autoreleasepool
                    {
                        dispatch_sync(dispatch_get_main_queue(), ^{
                            [image drawInRect:[surface bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1];
                        });
                    }
                }
                
            } catch (NSException * ie) {
                NSLog(@"StreamReceiver: Exception: %@", ie);
            }
        }
        
        return false;
    }
    
    bool SetUseEncoderIOSX ( const char * moduleName )
    {
        return environs::API::SetUseEncoderN ( 1, moduleName );
    }
    
    
    bool SetUseDecoderIOSX ( const char * moduleName )
    {
        return environs::API::SetUseDecoderN ( 1, moduleName );
    }
    
    
    bool SetUseCapturerIOSX ( const char * moduleName )
    {
        return environs::API::SetUseCapturerN ( 1, moduleName );
    }
    
    
    id keyEventMonitor = nil;


    void AddKeyMonitor ()
    {
        CVerb ( "AddKeyMonitor" );
        
        if ( keyEventMonitor != nil )
            return;
        
        @autoreleasepool
        {
            NSEvent * ( ^handler )( NSEvent * );
            
            handler = ^NSEvent * (NSEvent * theEvent ) {
                
                //int keyCode = [theEvent keyCode];
                switch ( [theEvent keyCode] )
                {
                    case 53:
                        [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
                        break;
                        
                    case 125:    // Down
                        if ( [theEvent modifierFlags] & NSShiftKeyMask )
                        {
                            int level = environs::API::GetDebugN ();
                            if ( level > 0 ) {
                                environs::API::SetDebugN ( level - 1 );
                                
                                NSLog ( @"KeyEvent: Decreased debug level to %i", level - 1 );
                            }
                        }
                        break;
                        
                    case 126:    // Up
                        if ( [theEvent modifierFlags] & NSShiftKeyMask )
                        {
                            int level = environs::API::GetDebugN ();
                            if ( level < 15 ) {
                                environs::API::SetDebugN ( level + 1 );
                                
                                NSLog ( @"KeyEvent: Increased debug level to %i", level + 1 );
                            }
                        }
                        break;
                    default:
                        break;
                }
                return theEvent;
            };
            
            keyEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:handler];            
        }
    }
    
    
    void RemoveKeyMonitor ()
    {
        CVerb ( "RemoveKeyMonitor" );
        
        if ( keyEventMonitor ) {
            [NSEvent removeMonitor:keyEventMonitor];
            keyEventMonitor = nil;
        }
    }


#ifdef NATIVE_WIFI_OBSERVER_THREAD

    WifiObserver * wifiObs = 0;

    extern ThreadSync	wifiThread;

    unsigned int lastScan	= 0;

    void * Thread_WifiObserver ()
    {
        CLog ( "WifiObserver: Created ..." );

        bool		 doScan		= false;
        unsigned int lastCheck	= 0;
        wifiObs = &native.wifiObserver;

        @autoreleasepool
        {
            OSXWifiListener * listener = [[OSXWifiListener alloc] init];
            if ( !listener )
            {
                CErr( "WifiObserver: Failed to create listener." );
                return 0;
            }
            listener->name = nil;
            
            NSError * err = nil;
            CWInterface * wifi = [[CWWiFiClient sharedWiFiClient] interface];

            if ( !wifi ) {
                CErr( "WifiObserver: Failed to get wifi client." );
            }
            else
            {
                listener->name = [wifi interfaceName];

                [listener EnableNotifications];

                while ( wifiObs->threadRun )
                {
                    NSSet<CWNetwork *> * networks;

                    if ( doScan ) {
                        networks = [wifi scanForNetworksWithName:nil error:&err];
                        doScan = false;
                    }
                    else
                        networks = [wifi cachedScanResults];

                    if( err ) {
                        NSLog( @"%@", err );
                    }
                    else {
                        if ( [ networks count ] > 0 )
                        {
                            native.wifiObserver.Begin ();

                            for ( CWNetwork * network in networks )
                            {
                                unsigned char encrypt;

                                if ( [network supportsSecurity:kCWSecurityNone ] )
                                    encrypt = 0;
                                else if ( [network supportsSecurity:kCWSecurityWEP ] )
                                    encrypt = 1;
                                else encrypt = 2;

                                native.wifiObserver.UpdateWithColonMac ( [[network bssid] UTF8String],
                                                                        [[network ssid] UTF8String],
                                                                        (int) [network rssiValue], 0,
                                                                        (unsigned char) [[network wlanChannel] channelNumber],
                                                                        encrypt );

                                CVerbArg ( "WifiObserver: SSID [ %s ]\tBSSID [ %s ]\trssi [ %i ]\tchannel [ %d ]\tencrypt [ %c ]",
                                          [[network ssid] UTF8String], [[network bssid] UTF8String] ,
                                          (int)[network rssiValue], (int) [[network wlanChannel] channelNumber],
                                          encrypt );
                            }

                            native.wifiObserver.Finish ();
                        }
                    }

                    lastCheck = GetEnvironsTickCount32 ();

                    unsigned int waitTime = native.useWifiInterval;

                WaitLoop:
                    if ( wifiObs->threadRun ) {
                        wifiObs->thread.WaitOne ( "WifiObserver", waitTime );

                        unsigned int now = GetEnvironsTickCount32 ();
                        unsigned int diff = now - lastCheck;

                        if ( diff < NATIVE_WIFI_OBSERVER_INTERVAL_CHECK_MIN ) {
                            waitTime = ( NATIVE_WIFI_OBSERVER_INTERVAL_CHECK_MIN + 30 ) - diff;
                            goto WaitLoop;
                        }

                        if ( ( now - lastScan ) > ( unsigned ) native.useWifiInterval )
                        {
                            lastScan = GetEnvironsTickCount32 ();
                            doScan = true;
                        }
                    }
                }

                [listener DisableNotifications];
            }
        }

        CLog ( "WifiObserver: bye bye ..." );
        return 0;
    }
#endif

}



#   ifdef NATIVE_WIFI_OBSERVER_THREAD

@implementation OSXWifiListener

@synthesize wifiInterface;

- ( void ) EnableNotifications
{
    CVerb ( "EnableNotifications" );

    //    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWScanCacheDidUpdateNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWPowerDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWBSSIDDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWLinkDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWLinkQualityDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWLinkQualityNotificationRSSIKey object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWLinkQualityNotificationTransmitRateKey object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWModeDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWPowerDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWSSIDDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnWifiNotification:) name:CWSSIDDidChangeNotification object:nil];
    //});

    self.wifiInterface = [CWInterface interfaceWithName:name];
}


- ( void ) DisableNotifications
{
    CVerb ( "DisableNotifications" );

    self.wifiInterface = nil;
}


- ( void ) OnWifiNotification : (NSNotification *) note
{
    CVerbVerb ( "OnWifiNotification" );

    lastScan = GetEnvironsTickCount32 ();

    wifiObs->thread.Notify ( "WifiObserver");
}

@end

#   endif

#endif





















