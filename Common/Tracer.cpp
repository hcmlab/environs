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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#if ( !defined(NDEBUG) && !defined(CLI_CPP) )
//#   define DEBUGVERB
//#   define DEBUGVERBVerb

#include "Environs.Native.h"
#include "Tracer.h"
#include "Interop/Threads.h"
#include "Interop/Sock.h"
#include <map>
#include <vector>
#include <string>
#include "Environs.Utils.h"
#include "Mediator.h"

#ifdef DEBUG_TRACK_DEVICEBASE
#	include "Device/Device.Base.h"
#endif

#if defined(DEBUG_TRACK_DEVICE_INSTANCE) || defined(DEBUG_TRACK_LIST_NOTIFIER_CONTEXT)
#	include "Device.Instance.h"
#	include "Environs.h"
#endif

#if defined(DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT) || defined(DEBUG_TRACK_LIST_NOTIFIER_CONTEXT)
#   include "Queue.List.h"
#endif

#ifdef LINUX // for socket tracer
#   include <sys/types.h>
#   include <sys/syscall.h>
#endif

#ifndef _WIN32
#   include <signal.h>
#   include <unistd.h>
#endif

#define CLASS_NAME	"Tracer . . . . . . . . ."

//#define BREAK_INTO_LEAKS

namespace environs
{
    pthread_mutex_t                 tracerLock;

    /*
     * Core object tracer
     *
     */
#ifdef TRACE_CORE_OBJECTS
    LONGSYNC	debugCoreCount = 0;
#endif

    /*
     * TcpAcceptorContext tracer
     *
     */
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS
    class TcpAcceptorContext;
    std::map<TcpAcceptorContext *, TcpAcceptorContext *>  *   tcpAcceptorContexts     = 0;
#endif

    /*
     * DeviceBase tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICEBASE
    DeviceBase *                    devicesMapTrace [ MAX_CONNECTED_DEVICES ];
    std::map<DeviceBase *, DeviceBase *>  devicesMapTraceObjects;
#endif

    /*
     * Socket tracer
     *
     */
#ifdef DEBUG_TRACK_SOCKET
    std::map<int, std::string>  *   socketsLeaked     = 0;
    std::map<int, std::string>  *   socketsLog        = 0;
    std::map<int, std::string>  *   socketsHistory    = 0;
    std::vector<int>			*	socketIDHistory	  = 0;
#ifdef TRACE_ALIVE_THREAD_LOCK
    std::string                     lastAliveLocker;
    std::string                     lastAliveUnlocker;
#endif

    //#define TRACE_TCP_ACCEPTOR_CONTEXTS

    extern LONGSYNC					stuntCount;
    extern LONGSYNC					stunCount;
    LONGSYNC                        debugMediatorCount          = 0;
    LONGSYNC                        debugDeviceBaseCount        = 0;
    LONGSYNC                        debugDeviceInstanceNodeCount = 0;
#endif

    /*
     * DeviceInstance tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
    LONGSYNC                            debugDeviceObjCount     = 0;

    std::vector<lib::DeviceInstance *>  debugDevices;

    pthread_mutex_t                     debugDeviceObjLock;
#endif

    /*
     * MessageInstance tracer
     *
     */
#ifdef DEBUG_TRACK_MESSAGE_INSTANCE
    LONGSYNC                            debugMessageObjCount    = 0;

    std::vector<lib::MessageInstance *> debugMessages;

    pthread_mutex_t                     debugMessageObjLock;
#endif

    /*
     * Platform MessageInstance tracer
     *
     */
#ifdef DEBUG_TRACK_PLATFORM_MESSAGE_INSTANCE
    LONGSYNC                            debugMessagePlatformObjCount = 0;
#endif

    /*
     * DeviceNotifier tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT
    LONGSYNC                            debugDeviceNotifierObjCount = 0;

    lib::QueueList                      debugDeviceNotifiers;
#endif


    /*
     * ListCommandContext tracer
     *
     */
#ifdef DEBUG_TRACK_LIST_NOTIFIER_CONTEXT
    LONGSYNC                            debugListCommandObjCount    = 0;

    lib::QueueList                      debugListCommands;
#endif


    /*
     * DeviceInstanceNode tracer
     *
     */
#ifdef TRACE_MEDIATOR_DEVICE_INSTANCE_NODES
    pthread_mutex_t                     deviceInstancesMapLock;

    std::map<DeviceInstanceNode *, DeviceInstanceNode *> deviceInstancesMap;
#endif


    /*
     * SendContext tracer
     *
     */
#ifdef TRACE_MEDIATOR_SEND_CONTEXTS
    pthread_mutex_t                         sendContextsMapLock;

    std::map<SendContext *, SendContext *>  sendContextsMap;
#endif


    /*
     * ThreadInstance tracer
     *
     */
#ifdef TRACE_MEDIATOR_SEND_CONTEXTS
    pthread_mutex_t                         threadInstanceMapLock;

    std::map<ThreadInstance *, ThreadInstance *> clientsMap;
#endif



    bool TraceSocketInit ();
	bool TraceTcpAcceptorInit ();
    bool TraceDeviceBaseInit ();
    bool TraceDeviceInstanceInit ();
    bool TraceMessageInstanceInit ();
    bool TraceDeviceInstanceNodesInit ();
    bool TraceSendContextsInit ();
    bool TraceThreadInstancesInit ();
    

	bool InitTracerAll ()
	{
		if ( !LockInitA ( tracerLock ) )
			return false;

        if ( !TraceDeviceBaseInit () )
            return false;

        if ( !TraceSocketInit () )
			return false;

		if ( !TraceTcpAcceptorInit () )
            return false;

        if ( !TraceDeviceInstanceInit () )
            return false;

        if ( !TraceMessageInstanceInit () )
            return false;

        if ( !TraceDeviceInstanceNodesInit () )
            return false;

        if ( !TraceSendContextsInit () )
            return false;

        if ( !TraceThreadInstancesInit () )
            return false;
        
		return true;
    }


    void TraceTcpAcceptorDispose ();
    void TraceSocketDispose ();
    void TraceDeviceInstanceDispose ();
    void TraceMessageInstanceDispose ();
    void TraceDeviceInstanceNodesDispose ();
    void TraceSendContextsDispose ();
    void TraceThreadInstancesDispose ();


    void DisposeTracerAll ()
    {
        TraceTcpAcceptorDispose ();
        TraceSocketDispose ();
        TraceDeviceInstanceDispose ();
        TraceMessageInstanceDispose ();
        TraceDeviceInstanceNodesDispose ();
        TraceSendContextsDispose ();
        TraceThreadInstancesDispose ();

        LockDisposeA ( tracerLock );
    }
    
    
    void TraceCheckCoreStatus ()
	{
#ifdef TRACE_CORE_OBJECTS
		if ( debugCoreCount > 0 ) {
			CErrArg ( "TraceCheckCoreStatus: Kernel objects still alive [ %i ] ", debugCoreCount );
#ifdef BREAK_INTO_LEAKS
			_EnvDebugBreak ();
#endif
		}
#endif
	}


#ifdef TRACE_ALIVE_THREAD_LOCK
	void TraceAliveLocker ( const char * name )
	{
		LockAcquireA ( tracerLock, "TraceAliveLocker" );

		lastAliveLocker = name;

		LockReleaseA ( tracerLock, "TraceAliveLocker" );
	}

	void TraceAliveUnlocker ( const char * name )
	{
		LockAcquireA ( tracerLock, "TraceAliveUnlocker" );

		lastAliveUnlocker = name;

		LockReleaseA ( tracerLock, "TraceAliveUnlocker" );
	}
#endif


#ifdef DEBUG_TRACK_DEVICEBASE

	void TraceDeviceBaseAdd ( DeviceBase  * ctx )
	{
		__sync_add_and_fetch ( &debugDeviceBaseCount, 1 );

		LockAcquireA ( tracerLock, "TraceDeviceBaseAdd" );

		devicesMapTraceObjects [ ctx ] = ctx;

		LockReleaseA ( tracerLock, "TraceDeviceBaseAdd" );

		ctx->nativeIDTrace				= 0;
	}


	void TraceDeviceBaseRemove ( DeviceBase  * ctx )
	{
		__sync_sub_and_fetch ( &debugDeviceBaseCount, 1 );

		if ( ctx->nativeIDTrace > 0 && ctx->nativeIDTrace < MAX_CONNECTED_DEVICES )
			devicesMapTrace [ ctx->nativeIDTrace ] = 0;

		LockAcquireA ( tracerLock, "TraceDeviceBaseRemove" );

		std::map<DeviceBase *, DeviceBase *>::iterator iter = devicesMapTraceObjects.find ( ctx );

		if ( iter != devicesMapTraceObjects.end () ) {
			devicesMapTraceObjects.erase ( iter );
		}

		LockReleaseA ( tracerLock, "TraceDeviceBaseRemove" );
    }


    void TraceDeviceBaseMapAdd ( DeviceBase  * device, int nativeID )
    {
        devicesMapTrace [ nativeID ] = device; device->nativeIDTrace = nativeID;
    }
#endif
    
    
    bool TraceDeviceBaseInit ()
    {
#ifdef DEBUG_TRACK_DEVICEBASE
        Zero ( devicesMapTrace );
#endif
        return true;
    }


#ifdef DEBUG_TRACK_SOCKET

#ifndef ANDROID
#ifdef LINUX
#	define ADD_TIMESTRING
#endif
#endif

	void TraceCheckStatus ( bool noKernel )
    {
        LockAcquireA ( tracerLock, "TraceCheckStatus" );

		if ( ( !noKernel && socketsLog->size () > 1 ) || ( noKernel && socketsLog->size () > 0 ) )
		{
			std::map<int, std::string>::iterator iter = socketsLog->begin ();

			while ( iter != socketsLog->end () )
			{
				CErrArg ( "\n~TraceCheckStatus: Socket [ %i ] leaked\n------------------------------------------------\n%s\n------------------------------------------------\n", iter->first, iter->second.c_str () );

#ifdef MEDIATORDAEMON
				printf ( "\n~TraceCheckStatus: Socket [ %i ] leaked\n------------------------------------------------\n%s\n------------------------------------------------\n", iter->first, iter->second.c_str () );
#endif
				( *socketsLeaked ) [ iter->first ] = iter->second;

				++iter;
			}
		}

		socketsLog->clear ();
		socketsHistory->clear ();
        socketIDHistory->clear ();

        LockReleaseA ( tracerLock, "TraceCheckStatus" );

#ifndef MEDIATORDAEMON
		if ( debugDeviceBaseCount > 0 ) {
			CErrArg ( "~TraceSocketDispose: DeviceBase objects still alive [ %i ] ", debugDeviceBaseCount );

			// Wait a second before we break into the debugger
			int count = 5;
			while ( count > 0 ) {
				Sleep ( 600 );
				if ( debugDeviceBaseCount <= 0 )
					break;
				count--;
			}
			if ( debugDeviceBaseCount > 0 ) {
				_EnvDebugBreak ();
			}
		}

		if ( debugDeviceInstanceNodeCount > 0 ) {
			CErrArg ( "~TraceSocketDispose: DeviceInstanceNode objects still alive [ %i ] ", debugDeviceInstanceNodeCount );
#ifdef BREAK_INTO_LEAKS
			_EnvDebugBreak ();
#endif
		}

		if ( stuntCount > 0 ) {
			CErrArg ( "~TraceSocketDispose: Stunt objects still alive [ %i ] ", stuntCount );
#ifdef BREAK_INTO_LEAKS
			_EnvDebugBreak ();
#endif
		}

		if ( stunCount > 0 ) {
			CErrArg ( "~TraceSocketDispose: Stun objects still alive [ %i ] ", stunCount );
#ifdef BREAK_INTO_LEAKS
			_EnvDebugBreak ();
#endif
		}

		if ( debugMediatorCount > 0 ) {
			CErrArg ( "~TraceSocketDispose: Mediator objects still alive [ %i ] ", debugMediatorCount );
#ifdef BREAK_INTO_LEAKS
			_EnvDebugBreak ();
#endif
		}

		if ( noKernel ) {
			TraceCheckCoreStatus ();
		}
#endif
	}


#ifdef ADD_TIMESTRING
	size_t GetTimeString ( char * timeBuffer, unsigned int bufferSize );
#endif


	void TracePrefixBuild ( std::string &log )
	{
		static char timeString [ 256 ];

#ifdef ADD_TIMESTRING
		size_t timeLen = GetTimeString ( timeString, sizeof ( timeString ) );

		if ( timeLen ) {
			log += std::string ( timeString );
		}
#endif
		*timeString = 0;

#if defined(LINUX) && !defined(__APPLE__)
		sprintf ( timeString, " | %16llX |", ( long long ) syscall ( __NR_gettid ) );
#else
		sprintf ( timeString, " | %16llX | ", ( long long ) GetCurrentThreadId () );
#endif
		log.append ( timeString );

		unsigned int t = GetEnvironsTickCount32 ();

		char tb [ 64 ];
		*tb = 0;
		sprintf ( tb, "%u", t );

		log.append ( tb );
	}


	void TraceSocket ( int sock, const char * msg )
	{
		LockAcquireA ( tracerLock, "TraceSocket" );

		if ( IsValidFD ( sock ) )
		{
#ifdef ADD_TIMESTRING
			std::string log;

			TracePrefixBuild ( log );

			log.append ( "> " ).append ( msg ? msg : "Unknown" );

			( *socketsLog ) [ sock ] = log;
#else
			std::string log = std::string ( "> " ).append ( msg ? msg : "Unknown" );

			( *socketsLog ) [ sock ] = log;
#endif
		}
		else {
			CErr ( "TraceSocket: Invalid socket!" );
		}

		LockReleaseA ( tracerLock, "TraceSocket" );
	}


	void TraceSocketUpdate ( int sock, const char * msg )
	{
		if ( IsInvalidFD ( sock ) )
			return;

		LockAcquireA ( tracerLock, "TraceSocketUpdate" );

		std::map<int, std::string>::iterator iter = socketsLog->find ( sock );

		if ( iter != socketsLog->end () ) {
#ifdef ADD_TIMESTRING
			std::string log;

			TracePrefixBuild ( log );

			iter->second.append ( "\n" ).append ( log.c_str () ).append ( "> " ).append ( msg );
#else
			iter->second.append ( "\n> " ).append ( msg );
#endif
		}
		else {
#ifdef ADD_TIMESTRING
			std::string log ( "" );

			TracePrefixBuild ( log );
#endif
			const char * src = "NOT FOUND";

			std::map<int, std::string>::iterator iterHistory = socketsHistory->find ( sock );

			if ( iterHistory != socketsHistory->end () ) {
				src = iterHistory->second.c_str ();
			}

#ifdef ADD_TIMESTRING
			CErrArg ( "TraceSocketUpdate:\n%s> Socket to update [ %i ] not found!\nSource [ %s ]\nHistory [ %s ]", log.c_str (), sock, msg, src );
#else
			CErrArg ( "TraceSocketUpdate:\n> Socket to update [ %i ] not found!\nSource [ %s ]\nHistory [ %s ]", sock, msg, src );
#endif
			//_EnvDebugBreak ();
		}

		LockReleaseA ( tracerLock, "TraceSocketUpdate" );
	}


	void TraceSocketRemove ( int sock, const char * msg, const char * msg1 )
	{
		LockAcquireA ( tracerLock, "TraceSocketRemove" );

		if ( IsValidFD ( sock ) )
		{
			std::map<int, std::string>::iterator iter = socketsLog->find ( sock );

			if ( iter != socketsLog->end () ) {

				std::string & tmp = iter->second;

#ifdef ADD_TIMESTRING
				std::string log ( "" );

				TracePrefixBuild ( log );

				if ( msg1 )
					tmp.append ( "\n" ).append ( log.c_str () ).append ( "> " ).append ( msg1 );

				tmp.append ( "\n" ).append ( log.c_str () ).append ( "> Closed in " ).append ( msg );

				( *socketsHistory ) [ sock ] = tmp;
#else
				if ( msg1 )
					tmp.append ( "\n> " ).append ( msg1 );

				tmp.append ( "\n> Closed in " ).append ( msg );
#endif
				( *socketsHistory ) [ sock ] = tmp;

				socketIDHistory->push_back ( sock );

				socketsLog->erase ( iter );

				if ( socketIDHistory->size () >= 130 )
				{
					while ( socketIDHistory->size () > 100 )
					{
						sock = socketIDHistory->at ( 0 );

						std::map<int, std::string>::iterator iterHistory = socketsHistory->find ( sock );

						if ( iterHistory != socketsHistory->end () ) {
							socketsHistory->erase ( iterHistory );
						}

						socketIDHistory->erase ( socketIDHistory->begin () );
					}
				}
			}
			else {
#ifdef ADD_TIMESTRING
				std::string log ( "" );

				TracePrefixBuild ( log );
#endif
				const char * src = "NOT FOUND";

				std::map<int, std::string>::iterator iterHistory = socketsHistory->find ( sock );

				if ( iterHistory != socketsHistory->end () ) {
					src = iterHistory->second.c_str ();
				}

#ifdef ADD_TIMESTRING
				CErrArg ( "TraceSocketRemove:\n%s> Socket to close [ %i ] not found!%s%s\nSource [ %s ]\nHistory [ %s ]", log.c_str (), sock, msg1 ? "\nOrigin: " : "", msg1 ? msg1 : "", msg, src );
#else
				CErrArg ( "TraceSocketRemove:\n> Socket to close [ %i ] not found!%s%s\nSource [ %s ]\nHistory [ %s ]", sock, msg1 ? "\nOrigin: " : "", msg1 ? msg1 : "", msg, src );
#endif
				//_EnvDebugBreak ();

				CErr ( "TraceSocketRemove: ErrorBreak" );
			}
		}
		else {
			CErr ( "TraceSocketRemove: Invalid socket!" );
		}

		LockReleaseA ( tracerLock, "TraceSocketRemove" );
	}
#endif


	bool TraceSocketInit ()
	{
#ifdef DEBUG_TRACK_SOCKET
		if ( socketsHistory )
			return true;

		socketsLog = new std::map<int, std::string> ();
		if ( !socketsLog )
			return false;

		socketsHistory = new std::map<int, std::string> ();
		if ( !socketsHistory )
			return false;

		socketIDHistory = new std::vector<int> ();
		if ( !socketIDHistory )
			return false;

		socketsLeaked = new std::map<int, std::string> ();
		if ( !socketsLeaked )
			return false;
#endif
		return true;
    }


    void TraceSocketDispose ()
    {
#ifdef DEBUG_TRACK_SOCKET
        TraceCheckStatus ( true );

        if ( !socketsHistory )
            return;

        if ( socketsLog ) {
            delete socketsLog;
            socketsLog = 0;
        }

        if ( socketsHistory ) {
            delete socketsHistory;
            socketsHistory = 0;
        }

        if ( socketIDHistory ) {
            delete socketIDHistory;
            socketIDHistory = 0;
        }

        LockAcquireA ( tracerLock, "TraceSocketDispose" );

        if ( socketsLeaked )
        {
            if ( socketsLeaked->size () > 0 )
            {
                std::map<int, std::string>::iterator iter = socketsLeaked->begin ();

                while ( iter != socketsLeaked->end () )
                {
                    CErrArg ( "\n~TraceSocketDispose: Socket [ %i ] leaked\n------------------------------------------------\n%s\n------------------------------------------------\n", iter->first, iter->second.c_str () );

#ifdef MEDIATORDAEMON
                    printf ( "\n~TraceSocketDispose: Socket [ %i ] leaked\n------------------------------------------------\n%s\n------------------------------------------------\n", iter->first, iter->second.c_str () );
#endif
                    ++iter;
                }
                
                //_EnvDebugBreak ();
            }
            delete socketsLeaked;
            socketsLeaked = 0;
        }

        LockReleaseA ( tracerLock, "TraceSocketDispose" );
#endif
    }
    
    
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS

	void TraceTcpAcceptorAdd ( TcpAcceptorContext  * ctx )
	{
		LockAcquireA ( tracerLock, "TraceTcpAcceptorAdd" );

		( *tcpAcceptorContexts ) [ ctx ] = ctx;

		LockReleaseA ( tracerLock, "TraceTcpAcceptorAdd" );
	}


	void TraceTcpAcceptorRemove ( TcpAcceptorContext  * ctx )
	{
		LockAcquireA ( tracerLock, "TraceTcpAcceptorRemove" );

		std::map<TcpAcceptorContext *, TcpAcceptorContext *>::iterator iter = tcpAcceptorContexts->find ( ctx );

		if ( iter != tcpAcceptorContexts->end () ) {
			tcpAcceptorContexts->erase ( iter );
		}

		LockReleaseA ( tracerLock, "TraceTcpAcceptorRemove" );
	}
#endif


	bool TraceTcpAcceptorInit ()
	{
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS
		tcpAcceptorContexts = new std::map<TcpAcceptorContext *, TcpAcceptorContext *> ();
		if ( !tcpAcceptorContexts )
			return false;
#endif
		return true;
    }


    void TraceTcpAcceptorDispose ()
    {
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS
        if ( tcpAcceptorContexts ) {
            tcpAcceptorContexts->clear ();
            delete tcpAcceptorContexts;
            tcpAcceptorContexts = 0;
        }
#endif
    }



    /*
     * DeviceInstance tracer
     *
     */
    bool TraceDeviceInstanceInit ()
    {
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
        if ( !LockInitA ( debugDeviceObjLock ) )
            return false;
#endif
        return true;
    }

    void TraceDeviceInstanceDispose ()
    {
#ifdef DEBUG_TRACK_DEVICE_INSTANCE
        LockDisposeA ( debugDeviceObjLock );
#endif
    }


#ifdef DEBUG_TRACK_DEVICE_INSTANCE

    namespace lib
    {
        bool DeviceInstance::IsValid ()
        {
            if ( !gotRemoves && !gotRemoves1 && !gotRemoves2 && !gotRemoves3 && !gotRemoves41 && !gotRemoves42 && !gotRemoves5 ) {
                if ( myself.expired () )
                    return false;

                if ( gotInserts || gotInserts1 || gotInserts2 || gotInserts3 || gotInserts4 ) {
                    if ( !myselfAtClients && envObj )
                    {
                        for ( size_t i = 0; i < envObj->listAll->size (); ++i )
                        {
                            const sp (DeviceInstance ) &fi = envObj->listAll->at ( i );

                            if ( fi.get () == this )
                            {
                                return true;
                            }
                        }
                        return false;
                    }
                }
            }
            
            return true;
        }

    }

    
    void CheckDebugDevices ( lib::DeviceInstance * inst, bool remove )
    {
#ifdef DEBUG_TRACK_DEVICE_INSTANCE1

        pthread_mutex_lock ( &debugDeviceObjLock );

        if ( remove )
        {
            for (size_t i = 0; i < debugDevices.size(); ++i)
            {
                lib::DeviceInstance * fi = debugDevices[i];

                if ( fi == inst )
                {
                    debugDevices.erase(debugDevices.begin() + i);
                    break;
                }
                else {
                    // Check devices
                    if ( fi ) {
                        if ( !fi->IsValid () ) {
                            CErr ( "CheckDebugDevices: Invalid device!" );
                        }
                    }
                }
            }
        }
        else {
            if ( debugDevices.size() > 100000 ) {
                CErr ( "CheckDebugDevices: Vector exceeded max check sizes!!!" );
            }
            else {
                debugDevices.push_back(inst);
            }

            /*
             if ( debugDevices.size () > 1 )
             {
             Environs * envObj = debugDevices[0]->envObj;
             if ( envObj )
             {
             int a = (int) envObj->listAll->size ();
             int b = (int) debugDevices.size();
             if ( (b - a) > 1 ) {
             CErrArg ( "CheckDebugDevices: Size mismatch [ %i - %i ]", a, b );
             }
             if ( (a - b) > 1 || (b - a) > 1 ) {
             CErrArg ( "CheckDebugDevices: Size mismatch [ %i - %i ]", a, b );
             }
             }
             }
             */
        }

        pthread_mutex_unlock ( &debugDeviceObjLock );
#endif
    }


    void TraceDeviceInstanceAdd ( lib::DeviceInstance * inst )
    {
        LONGSYNC alives = __sync_add_and_fetch ( &debugDeviceObjCount, 1 );

        CLogArg ( "Construct: Alive [ %i ]", alives );

        CheckDebugDevices ( inst, false );
    }


    void TraceDeviceInstanceRemove ( lib::DeviceInstance * inst )
    {
        __sync_sub_and_fetch ( &debugDeviceObjCount, 1 );

        CheckDebugDevices ( inst, true );
    }


    void TraceDeviceInstanceReset ( lib::DeviceInstance * inst )
    {
        inst->disposalEnqueued = false;
        inst->disposalNotified = false;
        inst->lastObserversSize = 0;
        inst->gotInserts = 0;
        inst->gotUpdates = 0;
        inst->gotRemoves = 0;
        inst->gotRemoves2 = 0;
        inst->gotRemoves3 = 0;
        inst->gotRemoves41 = 0;
        inst->gotRemoves42 = 0;
        inst->gotRemoves5 = 0;
        inst->gotDisposes = 0;
        inst->gotInserts1 = 0;
        inst->gotInserts2 = 0;
        inst->gotInserts3 = 0;
        inst->gotInserts4 = 0;
        inst->gotUpdates1 = 0;
        inst->gotRemoves1 = 0;
    }
#endif



    /*
     * MessageInstance tracer
     *
     */
    bool TraceMessageInstanceInit ()
    {
#ifdef DEBUG_TRACK_MESSAGE_INSTANCE
        if ( !LockInitA ( debugMessageObjLock ) )
            return false;
#endif
        return true;
    }

    void TraceMessageInstanceDispose ()
    {
#ifdef DEBUG_TRACK_MESSAGE_INSTANCE
        LockDisposeA ( debugMessageObjLock );
#endif
    }

#ifdef DEBUG_TRACK_MESSAGE_INSTANCE
    extern void CheckDebugMessages(const sp(lib::MessageInstance) & inst );
#endif

#ifdef DEBUG_TRACK_MESSAGE_INSTANCE

    void CheckDebugMessages ( lib::MessageInstance * inst, bool remove )
    {
#ifdef DEBUG_TRACK_MESSAGE_INSTANCE1

        pthread_mutex_lock ( &debugMessageObjLock );

        if ( remove )
        {
            for (size_t i = 0; i < debugMessages.size(); ++i)
            {
                MessageInstance * fi = debugMessages[i];

                if (fi == inst)
                {
                    debugMessages.erase(debugMessages.begin() + i);
                    break;
                }
            }
        }
        else {
            if ( debugMessages.size() > 100000 ) {
                CErr ( "CheckDebugMessages: Vector exceeded max check sizes!!!" );
            }
            else {
                debugMessages.push_back(inst);
            }
        }

        pthread_mutex_unlock ( &debugMessageObjLock );
#endif
    }


    void TraceMessageInstanceAdd ( lib::MessageInstance * inst )
    {
        //LONGSYNC alives =
        __sync_add_and_fetch ( &debugMessageObjCount, 1 );

        //CLogArg ( "Construct: Alive [ %i ]", alives );

#ifdef DEBUG_TRACK_MESSAGE_INSTANCE1
        CheckDebugMessages ( inst, false );
#endif
    }


    void TraceMessageInstanceRemove ( lib::MessageInstance * inst )
    {
        __sync_sub_and_fetch ( &debugMessageObjCount, 1 );

#ifdef DEBUG_TRACK_MESSAGE_INSTANCE1
        CheckDebugMessages ( inst, true );
#endif
    }
#endif


    /*
     * Platform MessageInstance tracer
     *
     */
#ifdef DEBUG_TRACK_PLATFORM_MESSAGE_INSTANCE
    void TracePlatformMessageInstanceAdd1 ()
    {
        __sync_add_and_fetch ( &debugMessagePlatformObjCount, 1 );

        CLogArg ( "init: Remaining [ %i ]", debugMessagePlatformObjCount );
    }
#endif


    /*
     * DeviceNotifier tracer
     *
     */
#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT


#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT
    void CheckNotifierContextContexts ( lib::DeviceNotifierContext * inst, bool remove );
#endif

#undef CLASS_NAME
#define CLASS_NAME	"DeviceNotifierContext. ."

    lib::DeviceNotifierContext::DeviceNotifierContext ()
    {
        LONGSYNC alives = __sync_add_and_fetch ( c_ref debugDeviceNotifierObjCount, 1 );

        CLogArg ( "Construct: Alive [ %i ]", alives );

    }


    lib::DeviceNotifierContext::~DeviceNotifierContext ()
    {
        //LONGSYNC alives =
        __sync_sub_and_fetch ( c_ref debugDeviceNotifierObjCount, 1 );

        //CLogArg ( "Destruct: Alive [ %i ]", alives );
    }


    void CheckNotifierContextContexts ( lib::DeviceNotifierContext * inst, bool remove )
    {
#ifdef DEBUG_TRACK_DEVICE_NOTIFIER_CONTEXT1
        if ( debugDeviceNotifiers.size_ > 100000 ) {
            CErr ( "CheckNotifierContextContexts: Vector exceeded max check sizes!!!" );
        }

        if ( remove )
        {
            if ( debugDeviceNotifiers.remove ( inst ) )
                return;

            CLog ( "CheckNotifierContextContexts: Not found!" );
            return;
        }
        debugDeviceNotifiers.push ( inst );
#endif
    }


    void CheckNotifierContextContextsWithLock ( void * lock_, lib::DeviceNotifierContext * inst, bool remove )
    {
        pthread_mutex_t * lock = (pthread_mutex_t *) lock_;

        LockAcquireV ( lock, "NotifierThread" );

        CheckNotifierContextContexts ( inst, true );

        LockReleaseV ( lock , "NotifierThread" );
    }

#endif


    /*
     * ListCommandContext tracer
     *
     */
#ifdef DEBUG_TRACK_LIST_NOTIFIER_CONTEXT

#undef CLASS_NAME
#define CLASS_NAME	"ListCommandContext . . ."

    lib::ListCommandContext::ListCommandContext ()
    {
        LONGSYNC alives = __sync_add_and_fetch ( &debugListCommandObjCount, 1 );

        CLogArg ( "Construct: Alive [ %i ]", alives );

    }

    lib::ListCommandContext::~ListCommandContext ()
    {
        //LONGSYNC alives =
        __sync_sub_and_fetch ( &debugListCommandObjCount, 1 );

        //CLogArg ( "Destruct: Alive [ %i ]", alives );
    }

    void CheckListCommandContexts ( void * lock_, lib::ListCommandContext * inst, bool remove )
    {
#ifdef DEBUG_TRACK_LIST_NOTIFIER_CONTEXT1
        pthread_mutex_t * lock = (pthread_mutex_t *) lock_;

        if ( lock )
            LockAcquireV ( lock, "CommandThread" );

        if ( debugListCommands.size () > 100000 ) {
            CErr ( "CheckListCommandContexts: Vector exceeded max check sizes!!!" );
        }

        if ( remove )
        {
            if ( debugListCommands.remove ( inst ) )
                goto Finish;

            CLog ( "CheckListCommandContexts: Not found!" );
			goto Finish;
        }
        debugListCommands.push ( inst );

	Finish:
        if ( lock )
            LockReleaseV ( lock, "CommandThread" );
#endif
    }

#endif
    
    
#undef CLASS_NAME
#define CLASS_NAME	"DeviceInstanceNode . . ."


    /*
     * DeviceInstanceNode tracer
     *
     */
    bool TraceDeviceInstanceNodesInit ()
    {
#ifdef TRACE_MEDIATOR_DEVICE_INSTANCE_NODES
        if ( !LockInitA ( deviceInstancesMapLock ) )
            return false;
#endif
        return true;
    }

    void TraceDeviceInstanceNodesDispose ()
    {
#ifdef TRACE_MEDIATOR_DEVICE_INSTANCE_NODES
        LockDisposeA ( deviceInstancesMapLock );
#endif
    }


#ifdef TRACE_MEDIATOR_DEVICE_INSTANCE_NODES

    void TraceDeviceInstanceNodesAdd ( DeviceInstanceNode  * ctx )
    {
        LockAcquireA ( deviceInstancesMapLock, "SendContext" );

        deviceInstancesMap [ ctx ] = ctx;

        LockReleaseA ( deviceInstancesMapLock, "SendContext" );
    }


    void TraceDeviceInstanceNodesRemove ( DeviceInstanceNode  * ctx )
    {
        LockAcquireA ( deviceInstancesMapLock, "DeviceInstanceNode" );

        const std::map<DeviceInstanceNode *, DeviceInstanceNode *>::iterator it = deviceInstancesMap.find ( ctx );
        if ( it != deviceInstancesMap.end () ) {
            deviceInstancesMap.erase ( it );
        }

        LockReleaseA ( deviceInstancesMapLock, "DeviceInstanceNode" );
    }
#endif


    /*
     * SendContext tracer
     *
     */
    bool TraceSendContextsInit ()
    {
#ifdef TRACE_MEDIATOR_SEND_CONTEXTS
        if ( !LockInitA ( sendContextsMapLock ) )
            return false;
#endif
        return true;
    }

    void TraceSendContextsDispose ()
    {
#ifdef TRACE_MEDIATOR_SEND_CONTEXTS
        LockDisposeA ( sendContextsMapLock );
#endif
    }


#ifdef TRACE_MEDIATOR_SEND_CONTEXTS

    void TraceSendContextAdd ( SendContext  * ctx )
    {
        LockAcquireA ( sendContextsMapLock, "SendContext" );

        sendContextsMap [ ctx ] = ctx;

        LockReleaseA ( sendContextsMapLock, "SendContext" );
    }


    void TraceSendContextRemove ( SendContext  * ctx )
    {
        LockAcquireA ( sendContextsMapLock, "SendContext" );

        const std::map<SendContext *, SendContext *>::iterator it = sendContextsMap.find ( ctx );
        if ( it != sendContextsMap.end () ) {
            sendContextsMap.erase ( it );
        }

        LockReleaseA ( sendContextsMapLock, "SendContext" );
    }
#endif


    /*
     * ThreadInstance tracer
     *
     */
    bool TraceThreadInstancesInit ()
    {
#ifdef TRACE_MEDIATOR_THREAD_INSTANCES
        if ( !LockInitA ( threadInstanceMapLock ) )
            return false;
#endif
        return true;
    }

    void TraceThreadInstancesDispose ()
    {
#ifdef TRACE_MEDIATOR_THREAD_INSTANCES
        LockDisposeA ( threadInstanceMapLock );
#endif
    }


#ifdef TRACE_MEDIATOR_THREAD_INSTANCES

    void TraceThreadInstanceAdd ( ThreadInstance  * ctx )
    {
        LockAcquireA ( threadInstanceMapLock, "ThreadInstance" );

        clientsMap [ ctx ] = ctx;

        LockReleaseA ( threadInstanceMapLock, "ThreadInstance" );
    }


    void TraceThreadInstanceRemove ( ThreadInstance  * ctx )
    {
        LockAcquireA ( threadInstanceMapLock, "ThreadInstance" );

        const std::map<ThreadInstance *, ThreadInstance *>::iterator it = clientsMap.find ( ctx );
        if ( it != clientsMap.end () ) {
            clientsMap.erase ( it );
        }

        LockReleaseA ( threadInstanceMapLock, "ThreadInstance" );
    }
#endif
}

#endif