/**
 * Interface declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTERFACES_DISPOSE_H
#define	INCLUDE_HCM_ENVIRONS_INTERFACES_DISPOSE_H

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Interop/Threads.h"



namespace environs
{

#ifndef CLI_CPP
    
#define DERIVE_DISPOSABLE
#define DERIVE_TYPES	
//#define DISPOSER_ATOMIC_COUNTER
//#define USE_INSTANCE_OBJ_LOCK

#if !defined(USE_INSTANCE_OBJ_LOCK)
    extern pthread_mutex_t objLock;
#endif
    
	namespace lib
	{
		/**
		* IEnvironsDispose
		*
		*/
		PUBLIC_CLASS ENVIRONS_LIB_API IEnvironsDispose
		{
#ifndef CLI_CPP
			friend class PortalInstance;
			friend class DeviceList;
			friend class DeviceInstance;
			friend class MessageInstance;
			friend class FileInstance;
			friend class Environs;
#endif
		public:
			/** Constructor */
			IEnvironsDispose () { };

			virtual ~IEnvironsDispose () { };


			/**
			* Increase ownership by one in order to prevent releasing of the resources behind the interface.
			* Each increase MUST be balanced by a call to Release.
			* Otherwise the resources behind can not be released by Environs.
			*
			*/
			virtual bool Retain () = 0;


			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			*/
			virtual void Release () = 0;

		protected:
			/**
			* Internal Release used by Environs to clean up internal resources while object is locked.
			*
			*/
			virtual void ReleaseLocked () { };

#ifdef USE_INSTANCE_OBJ_LOCK
            pthread_mutex_t     objLock;    bool objLockInit;
#endif
		};
	}

	void EnvironsDisposer ( lib::IEnvironsDispose * obj );
#else

	using namespace System;

#	define DERIVE_DISPOSABLE		: IDisposable	
#	define DERIVE_TYPES				, public Types

#endif
}


#if defined(USE_INSTANCE_OBJ_LOCK) && !defined(CLI_CPP)
#   define ENVIRONS_OUTPUT_INIT_OBJLOCK()       if ( !objLockInit ) { if ( LockInitA ( objLock ) ) objLockInit = true; else return false; }
#   define ENVIRONS_OUTPUT_INITR_OBJLOCK(l,r)   if ( !l->objLockInit ) { if ( LockInitA ( l->objLock ) )  l->objLockInit = true; else return r; }
#   define ENVIRONS_OUTPUT_INITB_OBJLOCK(l)     if ( !l->objLockInit ) { if ( LockInitA ( l->objLock ) )  l->objLockInit = true; else break; } 
#   define ENVIRONS_OUTPUT_INITG_OBJLOCK(l)     if ( !l->objLockInit ) { if ( LockInitA ( l->objLock ) )  l->objLockInit = true; else goto Finish; }
#   define ENVIRONS_OUTPUT_DISPOSE_OBJLOCK()    if ( objLockInit ) { LockDisposeA ( objLock ); objLockInit = false; }
#   define ENVIRONS_OUTPUT_INITD_OBJLOCK(l)     if ( !l->objLockInit ) { if ( LockInitA ( l->objLock ) )  l->objLockInit = true; else { delete l; return nill; }; }
#else
#   define ENVIRONS_OUTPUT_INIT_OBJLOCK()
#   define ENVIRONS_OUTPUT_INITV_OBJLOCK()
#   define ENVIRONS_OUTPUT_INITR_OBJLOCK(l,r)
#   define ENVIRONS_OUTPUT_INITB_OBJLOCK(l)
#   define ENVIRONS_OUTPUT_INITG_OBJLOCK(l)
#   define ENVIRONS_OUTPUT_INITD_OBJLOCK(l)
#   define ENVIRONS_OUTPUT_DISPOSE_OBJLOCK()
#endif

#ifndef CLI_CPP

/*
 * myself is a pointer to the smart pointer instance within Environs.
 * myselfAtClients will hold a smart pointer to the instance within Environs as long as a client code acquired a reference to the object.
 *
 *
 */

#define ENVIRONS_OUTPUT_ALLOC_SP_RESOURCE(type)	sp ( type ) myself; sp ( type ) myselfAtClients; bool checkSP;

#define ENVIRONS_OUTPUT_ALLOC_WP_RESOURCE(type)	wp ( type ) myself; sp ( type ) myselfAtClients; bool checkSP;


#define ENVIRONS_OUTPUT_DE_ALLOC_DECL()         ENVIRONS_LIB_API bool	Retain (); \
                                                ENVIRONS_LIB_API void	Release (); \
                                                void * platformRef; \
                                            private: \
                                                LONGSYNC				refCountSP; \
                                                LONGSYNC                objID_;

#if defined(USE_INSTANCE_OBJ_LOCK)
#   define ENVIRONS_OUTPUT_ALLOC_INIT()		refCountSP = 0; platformRef = 0; objID_ = __sync_add_and_fetch ( &objectIdentifiers, 1 ); \
											objLockInit = false; if ( LockInitA ( objLock ) ) objLockInit = true;
#   define ENVIRONS_OUTPUT_ALLOC_INIT_WSP()	ENVIRONS_OUTPUT_ALLOC_INIT() checkSP = true;
#else
#   define ENVIRONS_OUTPUT_ALLOC_INIT()		refCountSP = 0; platformRef = 0; objID_ = __sync_add_and_fetch ( &objectIdentifiers, 1 );
#   define ENVIRONS_OUTPUT_ALLOC_INIT_WSP()	ENVIRONS_OUTPUT_ALLOC_INIT() checkSP = true;
#endif



#define ENVIRONS_OUTPUT_ALLOC(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = __sync_add_and_fetch ( &refCountSP, 1 ); \
	\
	CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
	\
	return ( localRefCount > 0 ); \
}

#ifdef DEBUG_DISPOSER_WSP

#define ENVIRONS_OUTPUT_ALLOC_SP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = __sync_add_and_fetch ( &refCountSP, 1 ); \
	\
    if ( localRefCount == 1 ) { \
        pthread_mutex_lock ( &objLock ); \
        if ( myself ) { \
            CVerbVerbArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
            myselfAtClients = myself; \
		} \
        if ( !myselfAtClients ) { \
			__sync_sub_and_fetch ( &refCountSP, 1 ); localRefCount = 0; \
        } \
        pthread_mutex_unlock ( &objLock ); \
    } \
	else {  \
        pthread_mutex_lock ( &objLock ); \
		\
        if ( !myselfAtClients ) { \
			abort(); \
        } \
        pthread_mutex_unlock ( &objLock ); \
	} \
	CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
	\
	return ( localRefCount > 0 ); \
}


#define ENVIRONS_OUTPUT_ALLOC_WP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
    CLogArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
    \
    LONGSYNC refCount = refCountSP; LONGSYNC localRefCount = __sync_add_and_fetch ( &refCountSP, 1 ); \
    \
    if ( localRefCount == 1 ) { \
		\
        pthread_mutex_lock ( &objLock ); \
        if ( myself.use_count () > 0 ) { \
            CLogArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
            myselfAtClients = myself.lock (); \
        } \
        if ( !myselfAtClients ) { \
			CLogArg ( "Retain [ %i ]: Failed -> [ %i ] [ %i ]", objID_, localRefCount, refCountSP ); \
			__sync_sub_and_fetch ( &refCountSP, 1 ); localRefCount = 0; \
        } \
        pthread_mutex_unlock ( &objLock ); \
    } \
	else {  \
        pthread_mutex_lock ( &objLock ); \
		\
        if ( !myselfAtClients ) { \
			abort(); \
        } \
        pthread_mutex_unlock ( &objLock ); \
	} \
    CLogArg ( "Retain [ %i ]: -> [ %i ] [ %i ]", objID_, localRefCount, refCountSP );  \
    \
    return ( localRefCount > 0 ); \
}


#else

#	ifdef DISPOSER_ATOMIC_COUNTER

#define ENVIRONS_OUTPUT_ALLOC_SP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = __sync_add_and_fetch ( &refCountSP, 1 ); \
	\
    if ( localRefCount == 1 || checkSP ) { \
        pthread_mutex_lock ( &objLock ); \
		\
        if ( !myselfAtClients ) { \
            CVerbVerbArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
            myselfAtClients = myself; \
			if ( !myselfAtClients ) { \
				__sync_sub_and_fetch ( &refCountSP, 1 ); localRefCount = 0; \
				checkSP = true; \
			} \
			else { \
				checkSP = false;\
			} \
        } \
		else { \
			checkSP = false;\
		} \
        pthread_mutex_unlock ( &objLock ); \
    } \
	CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
	\
	return ( localRefCount > 0 ); \
}


#define ENVIRONS_OUTPUT_ALLOC_WP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
    CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
    \
    LONGSYNC localRefCount	= __sync_add_and_fetch ( &refCountSP, 1 ); \
    \
    if ( localRefCount == 1 || checkSP ) { \
        pthread_mutex_lock ( &objLock ); \
		\
        if ( !myselfAtClients ) { \
            CVerbVerbArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
            myselfAtClients = myself.lock (); \
			\
			if ( !myselfAtClients ) { \
				__sync_sub_and_fetch ( &refCountSP, 1 ); localRefCount = 0; \
				checkSP = true; \
			} \
			else { \
				checkSP = false;\
			} \
        } \
		else { \
			checkSP = false;\
		} \
        pthread_mutex_unlock ( &objLock ); \
    } \
    CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
    \
    return ( localRefCount > 0 ); \
}

#	else // NOT DISPOSER_ATOMIC_COUNTER follows

#define ENVIRONS_OUTPUT_ALLOC_SP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	\
    pthread_mutex_lock ( &objLock ); \
	\
	CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = 0; \
	\
	if ( checkSP ) { \
	\
		localRefCount = ++refCountSP; \
		\
		if ( localRefCount == 1 ) { \
			\
			if ( !myselfAtClients ) { \
				CVerbVerbArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
				\
				myselfAtClients = myself; \
				\
				if ( !myselfAtClients ) { \
					--refCountSP; localRefCount = 0; \
					checkSP = false; \
				} \
			} \
		} \
    } \
	\
	pthread_mutex_unlock ( &objLock ); \
	\
	CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
	\
	return ( localRefCount > 0 ); \
}


#define ENVIRONS_OUTPUT_ALLOC_WP(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	\
    pthread_mutex_lock ( &objLock ); \
	\
	CVerbVerbArg ( "Retain [ %i ]: [ %i ]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = 0; \
	\
	if ( checkSP ) { \
	\
		localRefCount = ++refCountSP; \
		\
		if ( localRefCount == 1 ) { \
			\
			if ( !myselfAtClients ) { \
				CVerbVerbArg ( "Retain [ %i ]: -> Allocating SP", objID_ ); \
				\
				myselfAtClients = myself.lock (); \
				\
				if ( !myselfAtClients ) { \
					--refCountSP; localRefCount = 0; \
					checkSP = false; \
				} \
			} \
		} \
    } \
	\
	pthread_mutex_unlock ( &objLock ); \
	\
	CVerbVerbArg ( "Retain [ %i ]: -> [ %i ] ", objID_, localRefCount );  \
	\
	return ( localRefCount > 0 ); \
}

#	endif

#endif

 /**
 * Release ownership on this interface and mark it disposable.
 * Release must be called once for each Interface that the Environs framework returns to client code.
 * Environs will dispose the underlying object if no more ownership is hold by anyone.
 *
 * setting localRefCount = -1 means prohibit further reference allocations and signal custom deletion of the container
 */
#define ENVIRONS_OUTPUT_RELEASE()				 CVerbVerbArg ( "Release  [ %i ]: [ %i ]", objID_, refCountSP ); \
\
    LONGSYNC localRefCount = __sync_sub_and_fetch ( &refCountSP, 1 ); \
    \
    if ( localRefCount == 0 )  { \
        CVerbVerbArg ( "Release  [ %i ]: -> Disposing SP", objID_ );  \
        \
        ReleaseLocked (); \
    }


#	ifdef DISPOSER_ATOMIC_COUNTER

#define ENVIRONS_OUTPUT_RELEASE_SP(type)			CVerbVerbArg ( "Release  [ %i ]: [ %i ]", objID_, refCountSP ); \
\
    LONGSYNC localRefCount = __sync_sub_and_fetch ( &refCountSP, 1 ); \
    \
    if ( localRefCount == 0 )  { \
        CVerbVerbArg ( "Release  [ %i ]: -> Disposing SP", objID_ );  \
        \
        sp ( type ) toDispose; bool doRelease = false; \
        \
		pthread_mutex_lock ( &objLock ); \
			\
		if ( refCountSP == 0 ) { \
			checkSP = true; \
			\
			CVerbVerbArg ( "Release  [ %i ]: -> [ %i ]", objID_, localRefCount ); \
			\
			toDispose = myselfAtClients; doRelease = true; \
			\
			myselfAtClients = 0; \
		}  \
		pthread_mutex_unlock ( &objLock ); \
        \
        if ( doRelease ) { ReleaseLocked (); } \
    } 

//else { CVerbArg ( "Release: Somebody has retained while we were trying to release [ %i ]: 0 -> [ %i ]", objID_, refCountSP ); }

#	else // NOT DISPOSER_ATOMIC_COUNTER follows

#define ENVIRONS_OUTPUT_RELEASE_SP(type)			\
	CVerbVerbArg ( "Release  [ %i ]: [ %i ]", objID_, refCountSP ); \
    \
    sp ( type ) toDispose; bool doRelease = false; \
	\
	pthread_mutex_lock ( &objLock ); \
	\
    LONGSYNC localRefCount = --refCountSP; \
    \
    if ( localRefCount == 0 )  { \
		\
        CVerbVerbArg ( "Release  [ %i ]: -> Disposing SP", objID_ );  \
        \
		CVerbVerbArg ( "Release  [ %i ]: -> [ %i ]", objID_, localRefCount ); \
		\
		toDispose = myselfAtClients; doRelease = true; \
		\
		myselfAtClients = 0; \
		\
		if ( myself.use_count () <= 0 ) checkSP = false; \
    } \
	\
	pthread_mutex_unlock ( &objLock ); \
    \
    if ( doRelease ) { ReleaseLocked (); }  

#	endif


#if ((defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX)))

#   define ENVIRONS_OUTPUT_PLATFORM_REF()          if ( platformRef && refCountSP == 1 ) { \
    platformRef = 0; \
    id transfered = (__bridge_transfer id) platformRef; \
    transfered = nil; \
}
#else
#   define ENVIRONS_OUTPUT_PLATFORM_REF()
#endif


#define ENVIRONS_I_SP1(type,call,arg1)              \
	if ( !arg1->Retain () ) return;  \
	sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer );  \
	call ( sp1 );

#define ENVIRONS_I_SP2(type,call,arg1,arg2)        \
	if ( !arg1->Retain () ) return;  \
	sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer );  \
	call ( sp1, arg2 );

#define ENVIRONS_IR_SP1_RETURN(type,arg1)            \
	sp ( type ) sp1 ( (type *)arg1, ::environs::EnvironsDisposer ); return sp1

#define ENVIRONS_I_SP1_1(type,call,arg1,arg2)       \
	if ( !arg1->Retain () ) return;  \
	sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer ); \
	call ( sp1, arg2 );

#define ENVIRONS_IR_SP2(type,call,arg1,arg2)         \
	sp ( type ) sp1 ( (type *) arg1, ::environs::EnvironsDisposer ); \
	sp ( type ) sp2 ( (type *) arg2, ::environs::EnvironsDisposer ); \
	call ( environs_local_sp1, sp2 );

#define ENVIRONS_I_SP2_SEL(type,call,arg1,arg2)         \
	sp ( type ) sp1; sp ( type ) sp2; \
	if ( arg1 ) { \
		if ( !arg1->Retain () ) return; sp1 = sp ( type ) ( (type *) arg1, ::environs::EnvironsDisposer ); } \
	if ( arg2 ) { \
		if ( !arg2->Retain () ) return; sp2 = sp ( type ) ( (type *) arg2, ::environs::EnvironsDisposer ); } \
	call ( sp1, sp2 );


#define ENVIRONS_I_BUILDSP_LIST(type,arg1)      \
    type * t = ( type * ) itemRetained ( arg1 ); \
    if ( t == nill ) \
        return nill; \
    \
    sp ( type ) ent ( t, ::environs::EnvironsDisposer ); \
    return ent;

#define ENVIRONS_I_BUILDSP_LISTITEM_ALLOC(type,arg1)  \
    type * p1 = ( type * ) itemRetained ( arg1 ); \
	\
    if ( p1 == nill ) \
        return nill; \
    sp ( type ) ent ( p1, ::environs::EnvironsDisposer ); \
        return ent;

#else

#ifdef CLI_CPP
#	define	PLATFORMREF			System::Object ^
#	define	PLATFORMREFNILL		nill
#else
#	define	PLATFORMREF			void *
#	define	PLATFORMREFNILL		nill
#endif

#	define ENVIRONS_OUTPUT_ALLOC_SP_RESOURCE(type)	
#	define ENVIRONS_OUTPUT_ALLOC_WP_RESOURCE(type) 

#	define ENVIRONS_OUTPUT_DE_ALLOC_DECL()				ENVIRONS_LIB_API void	Release (); \
														PLATFORMREF platformRef; \
													protected: \
														LONGSYNC                objID_; \
													private: 

#	define ENVIRONS_OUTPUT_ALLOC_INIT()					platformRef = PLATFORMREFNILL; __int64 %tRef = objectIdentifiers; objID_ = __sync_add_and_fetch ( tRef, 1 );
#	define ENVIRONS_OUTPUT_ALLOC_INIT_WSP()				ENVIRONS_OUTPUT_ALLOC_INIT()

#	define ENVIRONS_OUTPUT_ALLOC(type)					extern LONGSYNC objectIdentifiers;
#	define ENVIRONS_OUTPUT_ALLOC_WP(type)				extern LONGSYNC objectIdentifiers;

#	define ENVIRONS_OUTPUT_RELEASE()
#	define ENVIRONS_OUTPUT_RELEASE_SP(type)						 

#	define ENVIRONS_I_SP1(type,call,arg1)				call ( arg1 );

#	define ENVIRONS_IR_SP1_RETURN(type,arg1)			return arg1

#endif


#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H









