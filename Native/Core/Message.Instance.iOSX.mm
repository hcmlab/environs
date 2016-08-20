/**
 * MessageInstance for iOSX
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
#include "Environs.Native.h"

#import "Message.Instance.iOSX.h"
#import "Device.Instance.iOSX.IAPI.h"

#ifdef DEBUG_TRACK_ARC_MESSAGE_INSTANCE
#include <vector>
#endif
#include "Tracer.h"

#define	CLASS_NAME 	"Message.Instance.iOSX. ."


namespace environs
{
    namespace lib
    {
        class MessageInstanceProxy
        {
        public:
            MessageInstanceProxy () : inst ( 0 ) {};
            
            environs::lib::MessageInstance      *    inst;
            
            long GetObjID () { if ( inst ) return inst->objID_; return 0; }
                        
            void DisposeInstance () {
                if ( inst && !inst->disposed_ ) {
                    CVerb ( "DisposeInstance: instance is available and not disposed yet." );
                    inst->DisposeInstance ();
                }
            }
        };
        
        
        void MessageInstance::PlatformDispose ()
        {
            CVerbVerb ( "PlatformDispose" );
            
            CVerbArg ( "PlatformDispose: ObjID [ %i ]\tRemaining [ %i ]", objID_,
#ifdef DEBUG_TRACK_PLATFORM_MESSAGE_INSTANCE
                      debugMessagePlatformObjCount
#else
                      0
#endif
                      );

#ifdef OSX_USE_MANUAL_REF_COUNT
            void * p = 0;

            if ( platformRef ) {
                p = platformRef;
                platformRef = 0;
            }
/*
            if ( p ) {
                ::MessageInstance * fNil = (::MessageInstance *)p;
                [fNil release];
            }
            */
#endif
        }
        
        
#ifdef DEBUG_TRACK_ARC_MESSAGE_INSTANCE
        LONGSYNC    debugArcMessageObjCount = 0;
        
        pthread_mutex_t debugArcMessageObjLock = PTHREAD_MUTEX_INITIALIZER;
        
        std::vector<void *> debugArcMessages;
        
        void CheckDebugArcMessages ( void * inst, bool remove )
        {
#ifdef DEBUG_TRACK_ARC_MESSAGE_INSTANCE1
            
            pthread_mutex_lock ( &debugArcMessageObjLock );
            
            if ( remove )
            {
                for (size_t i = 0; i < debugArcMessages.size(); ++i)
                {
                    void * fi = debugArcMessages[i];
                    
                    if (fi == inst)
                    {
                        debugArcMessages.erase(debugArcMessages.begin() + i);
                        break;
                    }
                }
            }
            else {
                if ( debugArcMessages.size() > 100000 ) {
                    CErr ( "CheckDebugArcMessages: Vector exceeded max check sizes!!!" );
                }
                else {
                    debugArcMessages.push_back(inst);
                }
            }
            
            pthread_mutex_unlock ( &debugArcMessageObjLock );
#endif
        }
#endif
    }
}


@interface MessageInstance ()
{
    environs::lib::MessageInstanceProxy    p;
    
    sp ( environs::lib::MessageInstance )  instSP;
}
@end


@implementation MessageInstance


- (id) init
{
    CVerb ( "init" );
    
    self = [super init];
    if ( self ) {
        p.inst = 0;

        TracePlatformMessageInstanceAdd ();

#ifdef DEBUG_TRACK_ARC_MESSAGE_INSTANCE
        //LONGSYNC alives =
        __sync_add_and_fetch (  &environs::lib::debugArcMessageObjCount, 1 );
        
        environs::lib::CheckDebugArcMessages ( (__bridge void *) self, false );
        //CLogArg ( "Construct: Alive [ %i ]", alives );
#endif
    }
    return self;
}


- (void) dealloc
{
    TracePlatformMessageInstanceRemove ();

#ifdef DEBUG_TRACK_ARC_MESSAGE_INSTANCE
    __sync_sub_and_fetch ( &environs::lib::debugArcMessageObjCount, 1 );
    
    environs::lib::CheckDebugArcMessages ( (__bridge void *) self, true );
#endif

    CVerbArg ( "dealloc ObjID [ %i ]\tRemaining [ %i ]", p.GetObjID (),
#ifdef DEBUG_TRACK_PLATFORM_MESSAGE_INSTANCE
              environs::debugMessagePlatformObjCount
#else
              0
#endif
              );
    
    if ( p.inst ) {
        p.inst->platformRef = 0;
        
        p.DisposeInstance ();
        p.inst = 0;
    }
    
    instSP = 0;

    IOSX_SUPER_DEALLOC ();
}


- (bool) SetInst : ( const sp ( environs::lib::MessageInstance ) & ) messageSP
{
    instSP = messageSP;
    if ( !instSP )
        return false;
    
    p.inst = instSP.get ();
    
    CVerbArg ( "SetInst ObjID [ %i ]", p.GetObjID() );

    if ( p.inst->platformRef ) {
        CVerbArg ( "SetInst: Error platform ref for ObjID [ %i ] already exists.", p.GetObjID() );
        instSP.reset ();
        p.inst = 0;
        return false;
    }

    p.inst->platformRef = (__bridge void *) self;

    return true;
}


- (sp ( environs::lib::MessageInstance ) &) GetInst
{
    return instSP;
}


/**
 * sent is true if this MessageInstance is data that was sent or received (false).
 * */
- (bool) 	sent { return p.inst->sent (); }

/**
 * created is a posix timestamp that determines the time and date that this MessageInstance
 * has been received or sent.
 * */
- (unsigned long long) 	created { return p.inst->created (); }

/**
 * The length of the text message in bytes (characters).
 * */
- (int) 	length { return p.inst->length (); }

/**
 * The text message.
 * */
- (NSString *) 	text { return [ [NSString alloc ] initWithUTF8String : p.inst->text () ]; }

/**
 * Determins the type of connection (channel type) used to exchange this message.
 * c = in connected state
 * d = in not connected state through a direct device to device channel.
 * m = in not connected state by means of a Mediator service.
 * */
- (char) 	connection { return p.inst->connection (); }

/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
//@property (readonly, nonatomic) id device;

- (NSString *) 	toString { return [ [NSString alloc ] initWithUTF8String : p.inst->toString () ]; }

- (NSString *) 	shortText { return [ [NSString alloc ] initWithUTF8String : p.inst->shortText () ]; }



/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
- (id) 	device
{
    sp ( environs::DeviceInstance ) idev = p.inst->device();
    if ( !idev )
        return nil;
    
    environs::lib::DeviceInstance * dev = (environs::lib::DeviceInstance *) idev.get();
    if ( !dev->platformRef )
        return nil;
    
    return (__bridge id) dev->platformRef;
}


@end








