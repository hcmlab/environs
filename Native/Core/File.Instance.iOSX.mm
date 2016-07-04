/**
 * FileInstance for iOSX
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

#import "File.Instance.iOSX.h"
#import "Device.Instance.iOSX.h"


#define	CLASS_NAME 	"File.Instance.iOSX . . ."


namespace environs
{
    namespace lib
    {
        class FileInstanceProxy
        {
        public:
            environs::lib::FileInstance      *    inst;
            
            void DisposeInstance () {
                if ( inst )
                    inst->DisposeInstance ();
            }
        };
        
        
        void FileInstance::PlatformDispose ()
        {
            CVerbVerb ( "PlatformDispose" );
            
            if ( platformKeep )
            {
                void * p = 0;
                
                LockAcquireV ( &device_->devicePortalsLock, "PlatformDispose" );
                
                p = platformKeep;
                platformKeep = 0;
                
                LockReleaseV ( &device_->devicePortalsLock, "PlatformDispose" );
                
                if ( p ) {
#ifdef OSX_USE_MANUAL_REF_COUNT
                    ::FileInstance * fNil = (::FileInstance *)p;
                    [fNil release];
#else
                    ::FileInstance * fNil = (__bridge_transfer ::FileInstance *)p;
                    if ( fNil )
                        fNil = nil;
#endif
                }
            }
        }
    }
}


@interface FileInstance ()
{
    environs::lib::FileInstanceProxy    p;
    
    sp ( environs::lib::FileInstance )  instSP;
}
@end


@implementation FileInstance (internal)


- (bool) SetInst : ( const sp ( environs::lib::FileInstance ) & ) fileSP
{
    instSP = fileSP;
    
    if ( !instSP )
        return false;
    
    p.inst = instSP.get ();

#ifdef OSX_USE_MANUAL_REF_COUNT
    [self retain];
    p.inst->platformKeep = (void *) self;
    p.inst->platformRef = p.inst->platformKeep;
#else
    p.inst->platformKeep = (__bridge_retained void *) self;
    p.inst->platformRef = p.inst->platformKeep;
#endif
    return true;
}


- (sp ( environs::lib::FileInstance ) &) GetInst
{
    return instSP;
}

@end


@implementation FileInstance

- (id) init
{
    CVerb ( "init" );
    
    self = [super init];
    if ( self ) {
        p.inst = 0;
    }
    return self;
}


- (void) dealloc
{
    CVerb ( "dealloc" );
    
    if ( p.inst ) {
        p.inst->platformRef = 0;
        
        p.DisposeInstance ();
        p.inst = 0;
    }
    
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


/**
 * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
 * A value of 0 indicates an invalid fileID.
 * */
- (int) 	fileID { return p.inst->fileID (); }

/**
 * Used internally.
 * */
- (int) 	type { return p.inst->type (); }

/**
 * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
 * */
- (NSString *) 	descriptor { return [ [NSString alloc ] initWithUTF8String : p.inst->descriptor () ]; }

/**
 * sent is true if this FileInstance is data that was sent or received (false).
 * */
- (bool) 	sent { return p.inst->sent (); }

/**
 * created is a posix timestamp that determines the time and date that this FileInstance
 * has been received or sent.
 * */
- (unsigned long long) 	created { return p.inst->created (); }

/**
 * The size in bytes of a buffer to send or data received.
 * */
- (long) 	size { return p.inst->size (); }

/**
 * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
 * */
- (NSString *) 	path { return [ [NSString alloc ] initWithUTF8String : p.inst->path () ]; }

/**
 * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
 * file or buffer that has already been sent.
 * If this value changes, then the corresponding device's DeviceObserver is notified
 * with this FileInstance object as the sender
 * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
 * */
- (int) 	sendProgress { return p.inst->sendProgress (); }

/**
 * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
 * file or buffer that has already been received.
 * If this value changes, then the corresponding device's DeviceObserver is notified
 * with this FileInstance object as the sender
 * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
 * */
- (int) 	receiveProgress { return p.inst->receiveProgress (); }

/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
//@property (readonly, nonatomic) id device;

- (NSString *) 	toString { return [ [NSString alloc ] initWithUTF8String : p.inst->toString () ]; }

- (NSString *) 	GetPath { return [ [NSString alloc ] initWithUTF8String : p.inst->GetPath () ]; }

@end







