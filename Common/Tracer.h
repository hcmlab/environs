/**
* Tracer
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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_TRACER_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_TRACER_H

#include "Environs.Build.Opts.h"

#ifdef __cplusplus
namespace environs
{
#endif

#ifndef NDEBUG

    extern bool InitTracerAll ();
    extern void DisposeTracerAll ();

#	define InitTracer()		if ( !InitTracerAll () ) return false;
#else
#	define InitTracer()
#	define DisposeTracerAll()
#endif

    /*
     * Core object tracer
     *
     */
#ifdef TRACE_CORE_OBJECTS
	extern LONGSYNC        debugCoreCount;

#   define TraceCoreAdd()		__sync_add_and_fetch ( &debugCoreCount, 1 )
#   define TraceCoreRemove()	__sync_sub_and_fetch ( &debugCoreCount, 1 )
#else
#   define TraceCoreAdd()
#   define TraceCoreRemove()
#endif

    /*
     * DeviceBase tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICEBASE
    class DeviceBase;
	extern void TraceDeviceBaseAdd ( DeviceBase  * ctx );
    extern void TraceDeviceBaseRemove ( DeviceBase  * ctx );
    extern void TraceDeviceBaseMapAdd ( DeviceBase  * device, int nativeID );
#else
#   define TraceDeviceBaseAdd(a)
#   define TraceDeviceBaseRemove(a)
#   define TraceDeviceBaseMapAdd(a,b)
#endif

    /*
     * Socket tracer
     *
     */
#ifdef DEBUG_TRACK_SOCKET
	extern void TraceCheckStatus ( bool withKernel );
	extern void TraceSocket ( int sock, const char * src );
	extern void TraceSocketUpdate ( int sock, const char * src );
	extern void TraceSocketRemove ( int sock, const char * src, const char * src1 );

	extern LONGSYNC    debugKernelCount;
	extern LONGSYNC    debugMediatorCount;
	extern LONGSYNC    debugDeviceBaseCount;

    extern LONGSYNC    debugDeviceInstanceNodeCount;

#   define TraceDeviceInstanceNodeAdd()		__sync_add_and_fetch ( &debugDeviceInstanceNodeCount, 1 )
#   define TraceDeviceInstanceNodeRemove()	__sync_sub_and_fetch ( &debugDeviceInstanceNodeCount, 1 )
#else
#   define TraceDeviceInstanceNodeAdd()
#   define TraceDeviceInstanceNodeRemove()
#endif


#ifdef TRACE_ALIVE_THREAD_LOCK
	extern void TraceAliveLocker ( const char * src );
	extern void TraceAliveUnlocker ( const char * src );
#else
#   define TraceAliveLocker(a)
#   define TraceAliveUnlocker(a)
#endif

    /*
     * TcpAcceptorContext tracer
     *
     */
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS
	class TcpAcceptorContext;
	extern void TraceTcpAcceptorAdd ( TcpAcceptorContext  * ctx );
	extern void TraceTcpAcceptorRemove ( TcpAcceptorContext  * ctx );
#else
#   define TraceTcpAcceptorAdd(a)
#   define TraceTcpAcceptorRemove(a)
#endif

    /*
     * DeviceInstance tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
    namespace lib { class DeviceInstance; }
    
    extern void TraceDeviceInstanceAdd ( lib::DeviceInstance * inst );
    extern void TraceDeviceInstanceRemove ( lib::DeviceInstance * inst );
    extern void TraceDeviceInstanceReset ( lib::DeviceInstance * inst );

#   define TRACE_DEVICE_INSTANCE(exp)       exp
#else
#   define TraceDeviceInstanceAdd(a)
#   define TraceDeviceInstanceRemove(a)
#   define TraceDeviceInstanceReset(a)
#   define TRACE_DEVICE_INSTANCE(exp)
#endif

    /*
     * MessageInstance tracer
     *
     */
#ifdef DEBUG_TRACK_MESSAGE_INSTANCE
    namespace lib { class MessageInstance; }

    extern void TraceMessageInstanceAdd ( lib::MessageInstance * inst );
    extern void TraceMessageInstanceRemove ( lib::MessageInstance * inst );

#   define TRACE_MESSAGE_INSTANCE(exp)       exp
#   define TRACE_MESSAGE_INSTANCE_NOT(exp)
#else
#   define TraceMessageInstanceAdd(a)
#   define TraceMessageInstanceRemove(a)
#   define TRACE_MESSAGE_INSTANCE(exp)
#   define TRACE_MESSAGE_INSTANCE_NOT(exp)   exp
#endif

    /*
     * DeviceNotifier tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT
    namespace lib { class DeviceNotifierContext; }

    void CheckNotifierContextContexts ( lib::DeviceNotifierContext * inst, bool remove );
    void CheckNotifierContextContextsWithLock ( void * lock, lib::DeviceNotifierContext * inst, bool remove );
#else
#   define CheckNotifierContextContexts(a,b)
#   define CheckNotifierContextContextsWithLock(a,b,c)
#endif

    /*
     * ListCommandContext tracer
     *
     */
#ifdef DEBUG_TRACK_LIST_NOTIFIER_CONTEXT
    namespace lib { class ListCommandContext; }

    void CheckListCommandContexts ( void * lock_, lib::ListCommandContext * inst, bool remove );
#else
#   define CheckListCommandContexts(a,b,c)
#endif


    /*
     * Platform MessageInstance tracer
     *
     */
#ifdef DEBUG_TRACK_PLATFORM_MESSAGE_INSTANCE
    extern LONGSYNC debugMessagePlatformObjCount;

    extern void TracePlatformMessageInstanceAdd1 ();
#   define TracePlatformMessageInstanceAdd()        environs::TracePlatformMessageInstanceAdd1 ()
#   define TracePlatformMessageInstanceRemove()     __sync_sub_and_fetch ( &environs::debugMessagePlatformObjCount, 1 )
#else
#   define TracePlatformMessageInstanceAdd()
#   define TracePlatformMessageInstanceRemove()
#endif


    /*
     * DeviceInstanceNode tracer
     *
     */
#ifdef TRACE_MEDIATOR_DEVICE_INSTANCE_NODES
    struct DeviceInstanceNode;

    extern void TraceDeviceInstanceNodesAdd ( DeviceInstanceNode  * ctx );
    extern void TraceDeviceInstanceNodesRemove ( DeviceInstanceNode  * ctx );
#else
#   define TraceDeviceInstanceNodesAdd(a)
#   define TraceDeviceInstanceNodesRemove(a)
#endif


    /*
     * SendContext tracer
     *
     */
#ifdef TRACE_MEDIATOR_SEND_CONTEXTS
    class SendContext;

    extern void TraceSendContextAdd ( SendContext  * ctx );
    extern void TraceSendContextRemove ( SendContext  * ctx );
#else
#   define TraceSendContextAdd(a)
#   define TraceSendContextRemove(a)
#endif


    /*
     * ThreadInstance tracer
     *
     */
#ifdef TRACE_MEDIATOR_THREAD_INSTANCES
    struct ThreadInstance;

    extern void TraceThreadInstanceAdd ( ThreadInstance  * ctx );
    extern void TraceThreadInstanceRemove ( ThreadInstance  * ctx );
#else
#   define TraceThreadInstanceAdd(a)
#   define TraceThreadInstanceRemove(a)
#endif



#ifdef __cplusplus
}
#endif

#endif