/**
* PortalInstance CPP Object
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

#ifndef CLI_CPP
#include "Environs.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"
#include "Environs.Lib.h"
#include "Environs.Obj.h"
#include "Environs.iOSX.Imp.h"
#include "Device.Instance.h"
#include <map>
#else
#include "Environs.Cli.Forwards.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"

#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Environs.h"
#include "Device/Device.Instance.Cli.h"
#include "Portal.Instance.Cli.h"
#endif

using namespace environs;
using namespace std;

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Instance. . . . ."


/* Namespace: environs -> */
namespace environs
{
	namespace lib
	{

#define PORTAL_OUTGOING_DIALOG  1
#define PORTAL_INCOMING_DIALOG  2
#define PORTAL_TRAFFIC_DIALOG   3

#ifndef CLI_CPP
		pthread_mutex_t					PortalInstance::portalInstancelock;
		pthread_mutex_t					PortalInstance::portalsLock;

		msp ( int, PortalInstance )	*	PortalInstance::portals		= nill;

		bool							PortalInstance::globalsInit = false;
#endif
		ENVIRONS_OUTPUT_ALLOC ( PortalInstance );


		bool PortalInstance::GlobalsInit ()
		{
			CVerb ( "GlobalsInit" );

			if ( globalsInit )
				return true;

			if ( portals == nill ) {
				portals = new__obj ( NLayerMapTypeObj ( int, PortalInstanceEP ) );
				if ( portals == nill )
					return false;

				ContainerClear ( portals );
			}

			if ( !LockInitA ( portalInstancelock ) )
				return false;
			if ( !LockInitA ( portalsLock ) )
				return false;

			globalsInit = true;
			return true;
		}


		void PortalInstance::GlobalsDispose ()
		{
			CVerb ( "GlobalsDispose" );

			if ( !globalsInit )
				return;

			if ( portals != nill ) {
				ContainerClear ( portals );
				delete__obj ( portals );
				portals = nill;
			}

			LockDisposeA ( portalsLock );
			LockDisposeA ( portalInstancelock );

			globalsInit = false;
		}


		PortalInstance::PortalInstance ()
		{
			CVerb ( "Construct" );

			disposed_   = 0;

			hEnvirons_  = 0;
			portalID_   = -1;
			dialogID    = 0;

			async       = environs::Call::NoWait;
			portalKey   = 0;
			status_     = PortalStatus::Created;
			networkOK    = false;
			outgoing_    = false;

			C_Only ( info_.portal	= this; )

			establishCalled     = false;
			renderActivated     = false;
			startIfPossible     = false;
			surface             = nill;
			askForTypeValue     = true;
			disposeOngoing_     = false;
			createdTickCount    = GetEnvironsTickCount ();

			ENVIRONS_OUTPUT_ALLOC_INIT_WSP ();
		}


		/**
		 * Release ownership on this interface and mark it disposable.
		 * Release must be called once for each Interface that the Environs framework returns to client code.
		 * Environs will dispose the underlying object if no more ownership is hold by anyone.
		 *
		 */
		void PortalInstance::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE_SP ( PortalInstance );
		}


		PortalInstance::~PortalInstance ()
		{
			CVerbArg1 ( "Destruct", "", "i", objID_ );

			DisposeInstance ( true );
		}


		void PortalInstance::DisposeInstance ( bool removeFromGlobal )
		{
			CVerbArg1 ( "DisposeInstance", "", "i", objID_ );

			if ( ___sync_val_compare_and_swap ( c_Addr_of ( disposed_ ), 0, 1 ) != 0 ) 
			{
				CVerbArg1 ( "DisposeInstance: Already disposed.", "portalID", "i", portalID_ );
				return;
			}

			LockAcquireVA ( portalsLock, "Dispose" );
						 
			CVerbArg2 ( "DisposeInstance: Removing portal from global portals.", "portalID", "i", portalID_, "portalKey", "i", portalKey );

			ContainerRemove ( portals, portalKey );

			LockReleaseA ( portalsLock, "DisposeInstance" );

			if ( removeFromGlobal ) {
				/// Remove ourself from the portals of the particular device
				LockAcquireA ( device_->devicePortalsLock, "Dispose" );

				CVerbArg1 ( "DisposeInstance: Removing portal from device portals.", "portalID", "i", portalID_ );

				for ( int i=0; i < (int) vd_size ( device_->devicePortals ); ++i )
				{
					if ( sp_get ( vd_at ( device_->devicePortals, i ) ) == CPP_CLI ( this, GetPlatformObj () ) )
					{
						ContainerdRemoveAt ( device_->devicePortals, i );
						break;
					}
				}

				LockReleaseA ( device_->devicePortalsLock, "DisposeInstance" );
			}

			if ( status_ >= PortalStatus::CreatedAskRequest )
				NotifyObservers ( Notify::Portale::StreamStopped );

			status_ = PortalStatus::Disposed;

			NotifyObservers ( Notify::Portale::Disposed );

			PlatformDispose ();

            C_Only ( observers.clear () );
            
            info_.portal = nill;
            
            C_Only ( myself = nill );
		}


		void PortalInstance::Dispose ( int nativeID, int portalID )
		{
			CVerbArg1 ( "Dispose", "portalID", "i", portalID );

			PortalInstanceSP portal;

			LockAcquireVA ( portalsLock, "Dispose" );

			ContainerIfContains ( portals, GetKey ( nativeID, portalID ) )

			portal = ( *portals ) [ GetKey ( nativeID, portalID ) ];

			LockReleaseA ( portalsLock, "Dispose" );

			if ( portal != nill )
				portal->DisposeInstance ( true );
		}


#ifndef CLI_CPP
		/** An ID that identifies this portal across all available portals. */
		int PortalInstance::portalID ()
		{
			return portalID_;
		}


		/** true = Object is disposed and not updated anymore. */
		bool PortalInstance::disposed ()
		{
			return ( disposed_ == 1 );
		}


		/**
		 * Get an Interface to the DeviceInstance that this PortalInstance is attached to.
		 * */
		environs::DeviceInstance * PortalInstance::deviceRetained ()
		{
			if ( device_ == nill )
				return nill;

			if ( device_->Retain () )
                return device_.get ();
            return nill;
		}


		environs::PortalInfo OBJ_ptr PortalInstance::info ()
		{
			return Addr_of ( info_ );
		}


		environs::PortalStatus_t PortalInstance::status ()
		{
			return status_;
		}


		bool PortalInstance::disposeOngoing ()
		{
			return disposeOngoing_;
		}


		/** true = outgoing (Generator), false = incoming (Receiver). */
		bool PortalInstance::outgoing ()
		{
			return outgoing_;
		}

		/** true = outgoing (Generator), false = incoming (Receiver). */
		bool PortalInstance::isOutgoing ()
		{
			return outgoing_;
		}

		/** true = outgoing (Generator), false = incoming (Receiver). */
		bool PortalInstance::isIncoming ()
		{
			return !outgoing_;
		}


		environs::PortalType_t PortalInstance::portalType ()
		{
			return portalType_;
		}
#endif


		void PortalInstance::AddObserver ( environs::PortalObserver OBJ_ptr observer )
		{
			CVerb ( "AddObserver" );

			if ( observer == nill ) return;

#ifdef CLI_CPP
			observers += observer;
#else
			size_t i = 0;
			size_t size = observers.size ();

			lib::IIPortalObserver * obsc = ( lib::IIPortalObserver * ) observer;

			for ( ; i < size; i++ )
			{
				lib::IIPortalObserver * obs = observers.at ( i );

				if ( obsc == obs )
					break;
			}

			if ( i >= size )
				observers.push_back ( obsc );
#endif
		}


		void PortalInstance::RemoveObserver ( environs::PortalObserver OBJ_ptr observer )
		{
			CVerb ( "RemoveObserver" );

			if ( observer == nill ) return;

#ifdef CLI_CPP
			observers -= observer;
#else
			int size = ( int ) observers.size ();

			lib::IIPortalObserver * obsc = ( lib::IIPortalObserver * ) observer;

			for ( int i=0; i < size; i++ )
			{
				lib::IIPortalObserver * obs = observers.at ( i );

				if ( obsc == obs ) {
					observers.erase ( observers.begin () + i );
					return;
				}
			}
#endif
		}


#ifndef CLI_CPP
		int PortalInstance::GetObserverCount ()
		{
			return ( int ) observers.size ();
		}
#endif


		bool PortalInstance::Establish ( bool askForType )
		{
			CVerb ( "Establish" );

			if ( disposed_ || device_->disposed_ || !device_->info_->isConnected )
				return false;

			establishCalled = true;
			askForTypeValue = askForType;

			if ( !networkOK && !CheckNetworkConnection () )
				return true;
			
			bool success = false;

			if ( outgoing_ ) {
				if ( askForType ) {
					success = ShowDialogOutgoingPortal ();
				}
				else {
					CVerbArg1 ( "Establish", "portalID", "i", portalID_ );

					if ( status_ == PortalStatus::CreatedFromRequest ) {
						return environs::API::ProvidePortalStreamN ( hEnvirons_, device_->info_->nativeID, ( int ) async, portalID_ | ( int ) portalType_ ) == 1;
					}
					else if ( status_ == PortalStatus::CreatedAskRequest ) {
						return environs::API::ProvideRequestPortalStreamN ( hEnvirons_, device_->info_->nativeID, ( int ) async, portalID_ | ( int ) portalType_ ) == 1;
					}
				}
			}
			else {
				CVerbArg1 ( "Establish: Request", "portalID", "i", portalID_ );

				return environs::API::RequestPortalStreamN ( hEnvirons_, device_->info_->nativeID, ( int ) async, portalID_ | ( int ) portalType_, info_.base.width, info_.base.height ) == 1;
			}
			return success;
		}


		bool PortalInstance::CheckNetworkConnection ()
		{
			CVerb ( "CheckNetworkConnection" );

			dialogID = PORTAL_TRAFFIC_DIALOG;

			if ( API::GetNetworkStatusN () >= NETWORK_CONNECTION_WIFI ) {
				networkOK = true;
			}
			else {
				networkOK = false;
			}

			return networkOK;
		}


		bool PortalInstance::Start ()
		{
			CVerb ( "Start" );

			if ( disposed_ || device_->disposed_ || !device_->info_->isConnected || portalID_ <= 0 )
				return false;

			return environs::API::StartPortalStreamN ( hEnvirons_, ( int ) async, portalID_ ) == 1;
		}


		bool PortalInstance::Stop ()
		{
			CVerb ( "Stop" );

			if ( disposed_ )
				return false;

			DisposeInstance ( true );

			if ( !device_->info_->isConnected )
				return false;

			return environs::API::StopPortalStreamN ( hEnvirons_, ( int ) async, device_->info_->nativeID, portalID_ ) == 1;
		}


		PortalInstanceSP PortalInstance::GetPortal ( int nativeID, int portalID )
		{
			PortalInstanceSP portal;

			if ( LockAcquireA ( portalsLock, "GetPortal" ) )
			{
				ContainerIfContains ( portals, GetKey ( nativeID, portalID ) )

					portal = ( *portals ) [ GetKey ( nativeID, portalID ) ];

				LockReleaseVA ( portalsLock, "GetPortal" );
			}
			return portal;
		}


		bool PortalInstance::SetRenderSurface ( void * surface_ )
		{
			return SetRenderSurface ( surface_, 0, 0 );
		}


		bool PortalInstance::SetRenderSurface ( void * surface_, int width, int height )
		{
			CVerb ( "SetRenderSurface" );


#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )
			if ( !width || !height ) {
				float scale = 1.0;

#   ifdef ENVIRONS_IOS
				UIScreen * screen = [ UIScreen mainScreen ];
				if ( screen ) {
					scale = screen.scale;
					if ( [ screen respondsToSelector : @selector(nativeScale ) ])
					scale = screen.nativeScale;
				}
#   else
				NSScreen * screen = [ NSScreen mainScreen ];
				if ( screen ) {
					scale = screen.backingScaleFactor;
				}
#   endif

				UIView * view = ( __bridge UIView * )surface_;

				CGRect rect = [ view frame ];
				width = rect.size.width * scale;
				height = rect.size.height * scale;
			}
#endif

			if ( width && height && !info_.base.width && !info_.base.height ) {
				info_.base.width = width;
				info_.base.height = height;
			}

			this->surface = surface_;

			renderActivated = environs::API::SetRenderSurfaceN ( hEnvirons_, portalID_, surface_, width, height ) == 1;
			return renderActivated;
		}


		bool PortalInstance::ReleaseRenderSurface ()
		{
			CVerb ( "ReleaseRenderSurface" );

			return environs::API::ReleaseRenderSurfaceN ( hEnvirons_, CALL_NOWAIT, portalID_ ) == 1;
		}


		bool PortalInstance::ReleaseRenderCallback ()
		{
			return environs::API::ReleaseRenderSurfaceN ( hEnvirons_, ( int ) async, portalID_ ) != 0;
		}


#ifdef DISPLAYDEVICE
		bool PortalInstance::SetPortalOverlayARGB ( int layerID, int left, int top, int width, int height, int stride, Addr_obj renderData, int alpha, bool positionDevice )
		{
			return environs::API::SetPortalOverlayARGBN ( hEnvirons_, device_->info_->nativeID, portalID_,
				layerID, left, top, width, height, stride, renderData, alpha, positionDevice ) != 0;
		}
#endif


#ifndef CLI_CPP
		PortalInfoBaseSP PortalInstance::GetPortalInfo ( int hInst, int portalID )
		{
			PortalInfoBaseSP info ( new PortalInfoBase );

			if ( info != nill ) {
				ZeroStruct ( *( info.get ( ) ), PortalInfoBase );

				info->portalID = portalID;
				environs::API::GetPortalInfoN ( hInst, info.get () );
			}

			return info;
		}
#else
		PortalInfoBaseSP PortalInstance::GetPortalInfo ( int hInst, int portalID )
		{
			if ( portalID == 0 )
				return nill;

			PortalInfoBase ^ info = nill;			
			
			char * buffer = new char [ 36 ];
			char * bufferOrg = buffer;
			if ( buffer )
			{
				int* pInt = ( int* ) buffer;
				*pInt = portalID;

				if ( environs::API::GetPortalInfoN ( hInst, ( void* ) buffer ) )
				{
					info = gcnew PortalInfoBase ();
					if ( info != nill )
					{
						info->portalID = portalID;

						pInt = ( int* ) buffer;
						pInt += 1;

						info->flags = *pInt++;
						info->centerX = *pInt++;
						info->centerY = *pInt++;
						info->width = *pInt++;
						info->height = *pInt++;
						info->orientation = *( ( float* ) pInt );
					}
				}

				delete[] bufferOrg;
			}
			return info;
		}
#endif


		PortalInfoBase OBJ_ptr PortalInstance::GetIPortalInfo ()
		{
			return Addr_of ( info_.base );
		}


		bool PortalInstance::SetPortalInfo ( int hInst, PortalInfoBase OBJ_ptr infoBase )
		{
#ifndef CLI_CPP
			return environs::API::SetPortalInfoN ( hInst, infoBase ) == 1;
#else
			char * pInfo = BuildNativePortalInfo ( infoBase );
			if ( !pInfo )
				return false;

			bool success = environs::API::SetPortalInfoN ( hInst, ( void* ) pInfo );

			delete pInfo;
			return success;
#endif
		}


		bool PortalInstance::SetIPortalInfo ( PortalInfoBase OBJ_ptr infoBase )
		{
			return SetPortalInfo ( hEnvirons_, infoBase );
		}


		void PortalInstance::NotifyObservers ( environs::Notify::Portale_t notification )
		{
			CVerb ( "NotifyObservers" );

#ifdef CLI_CPP
			observers ( GetPlatformObj (), notification );
#else
			for ( unsigned int i=0; i < observers.size (); i++ )
			{
				lib::IIPortalObserver * observer = observers.at ( i );
				try {
					if ( observer->OnPortalChangedInterface_ ) {
						observer->OnPortalChangedInterface ( this, notification );
					}
					if ( observer->OnPortalChanged_ ) {
						observer->OnPortalChangedBase ( this, notification );
					}
				}
                catch ( ... ) {
                    CErr ( "NotifyObservers: Exception!" );
				}
			}
#endif
		}


		bool PortalInstance::HasPortalRenderer ()
		{
			CVerb ( "HasPortalRenderer" );

			if ( surface ) {
				if ( renderActivated )
					return true;
				renderActivated = environs::API::SetRenderSurfaceN ( hEnvirons_, portalID_, surface, info_.base.width, info_.base.height ) == 1;
			}
			return renderActivated;
		}


		int PortalInstance::GetKey ( int nativeID, int portalID_ )
		{
			return ( ( nativeID << 16 ) | ( portalID_ & 0xFFF ) );
		}


		bool PortalInstance::CreateInstance ( c_const DeviceInstanceESP c_ref device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot )
		{
			CVerb ( "InitInstance" );

			if ( device == nill )
				return false;

			outgoing_ = ( ( Environs_PORTAL_DIR_ & PORTAL_DIR_OUTGOING ) == PORTAL_DIR_OUTGOING );
			portalType_ = ( PortalType_t ) type;
			hEnvirons_ = device->hEnvirons;
			
			info_.hEnvirons	= hEnvirons_;

#ifndef NDEBUG
			if ( hEnvirons_ <= 0 || hEnvirons_ > ENVIRONS_MAX_ENVIRONS_INSTANCES ) {
				CErrArg1 ( "CreateInstance: INVALID instance handle.", "hEnvirons", "i", hEnvirons_ );
				return false;
			}
#endif

			bool success = false;

			if ( LockAcquireA ( device->devicePortalsLock, "InitInstance" ) )
			{
				this->device_ = device;

#if (!defined(ENVIRONS_IOS) && !defined(ENVIRONS_OSX))
				//info.portal = myself;
#endif

				if ( slot < 0 ) {
					slot = environs::API::GetPortalIDFreeSlotN ( hEnvirons_, device_->info_->nativeID, Environs_PORTAL_DIR_ );
					if ( slot < 0 ) {
						CErr ( "InitInstance: No free portal slot available." );
						goto Finish;
					}
				}

				/// Add ourself to the portals of the particular device

				portalKey = GetKey ( device_->info_->nativeID, ( slot | Environs_PORTAL_DIR_ ) );

				portalID_ = Environs_PORTAL_DIR_ | slot | (int)type;

				CVerbArg2 ( "InitInstance: Adding portal to device portals.", "portalID", "i", portalID_, "portalKey", "i", portalKey );

                ContainerdAppend ( device->devicePortals, CPP_CLI ( myself, GetPlatformObj () ) );

				success = true;
			}

		Finish:
			LockReleaseVA ( device->devicePortalsLock, "InitInstance" );

			return success;
		}


		bool PortalInstance::Create ( c_const DeviceInstanceESP c_ref device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot )
		{
			CVerb ( "Init" );

			if ( CreateInstance ( device, Environs_PORTAL_DIR_, type, slot ) )
			{
				if ( LockAcquireA ( portalsLock, "Init" ) )
				{
					CVerbArg2 ( "Init: Adding portal to global portals.", "portalID", "i", portalID_, "portalKey", "i", portalKey );
#ifndef CLI_CPP
					( *portals ) [ portalKey ] = myself;
#else
					portals[ portalKey ] = GetPlatformObj ( );

					//this will throw "ArgumentException: An element with the same key already exists""
					//portals->Add ( portalKey, GetPlatformObj () );
#endif
					LockReleaseVA ( portalsLock, "Init" );

					return true;
				}
			}
			return false;
		}


		bool PortalInstance::Create ( c_const DeviceInstanceESP c_ref device, int destID )
		{
			CVerb ( "Init" );

			if ( CreateInstance ( device, ( destID & PORTAL_DIR_MASK ), ( environs::PortalType_t ) ( destID & PORTAL_TYPE_MASK ), ( destID & 0xFF ) ) )
			{
				if ( LockAcquireA ( portalsLock, "Init" ) )
				{
					CVerbArg2 ( "Init: Adding portal to global portals.", "portalID", "i", portalID_, "portalKey", "i", portalKey );

					portalID_ = destID;
#ifndef CLI_CPP
					( *portals ) [ portalKey ] = myself;
#else
					portals[ portalKey ] = GetPlatformObj ( );

					//this will throw "ArgumentException: An element with the same key already exists""
					//portals->Add ( portalKey, GetPlatformObj () );
#endif

					LockReleaseVA ( portalsLock, "Init" );

					return true;
				}

			}
			return false;
		}


		void c_OBJ_ptr PortalInstance::Destroyer ( pthread_param_t _targets )
		{
			NLayerVecTypeObj ( PortalInstance ) OBJ_ptr targets = ( NLayerVecTypeObj ( PortalInstance ) OBJ_ptr ) _targets;
//			vector < sp ( PortalInstance ) > * targets = ( vector < sp ( PortalInstance ) > * ) _targets;

			for ( size_t i=0; i < vp_size ( targets ); i++ )
			{
				PortalInstanceSP portal = vp_at ( targets,  i );
				portal->DisposeInstance ( true );
			}

			ContainerClear ( targets );
			delete__obj ( targets );

			return C_Only ( 0 );
		}


		void PortalInstance::KillZombies ()
		{
			NLayerVecTypeObj ( PortalInstance ) OBJ_ptr targets = new__obj ( NLayerVecTypeObj ( PortalInstance ) );

			if ( LockAcquireA ( portalsLock, "KillZombies" ) )
			{
				pthread_t thread;

				INTEROPTIMEVAL tick = GetEnvironsTickCount ();
#ifndef CLI_CPP
				typedef map < int, PortalInstanceESP >::const_iterator it_def;

				for ( it_def iterator = portals->begin (); iterator != portals->end (); ++iterator )
				{
					c_const PortalInstanceESP c_ref portal = iterator->second;

					if ( portal != nill && !portal->disposeOngoing_ && portal->portalID_ < 0 && ( tick - portal->createdTickCount ) > MAX_PORTAL_REQUEST_WAIT_TIME_MS )
					{
						portal->disposeOngoing_ = true;
						targets->push_back ( portal );
					}
				}
#else
				for each (KeyValuePair<int, PortalInstanceEP ^>^ inst in portals)
				{
					PortalInstanceEP ^portal = inst->Value;

					if ( portal != nill && !portal->disposeOngoing_ && portal->portalID_ < 0 && ( tick - portal->createdTickCount ) > MAX_PORTAL_REQUEST_WAIT_TIME_MS )
					{
						portal->disposeOngoing_ = true;
						targets->Add ( portal );
					}
				}
#endif
				int ret = 0;

				LockReleaseVA ( portalsLock, "KillZombies" );

				if ( vp_size ( targets ) <= 0 )
					goto Return;

                // Alright, lets call the Destroyer
                ret = pthread_create ( c_Addr_of ( thread ), NULL, &Destroyer, ( pthread_param_t ) targets );
				if ( ret != 0 ) {
					CErr ( "KillZombies: Failed to create worker thread." );
				}
                else
                {
#ifndef CLI_CPP
                    DetachThread ( nill, nill, thread, "PortalInstance::KillZombies" );
#endif
                    return;
                }
			}

		Return:
			if ( targets != nill ) {
				ContainerClear ( targets );
				delete__obj ( targets );
			}
		}


		void PortalInstance::Update ()
		{
			CVerb ( "Update" );

			portalType_ = ( PortalType_t ) ( portalID_ & PORTAL_TYPE_MASK );

			int Environs_PORTAL_DIR_ = portalID_ & PORTAL_DIR_MASK;

			outgoing_ = ( ( Environs_PORTAL_DIR_ & PORTAL_DIR_OUTGOING ) == PORTAL_DIR_OUTGOING );
		}


		/**
		 * Internal portal management logic.
		 *
		 */
		void PortalInstance::Update ( int hInst, environs::ObserverNotifyContext OBJ_ptr ctx )
		{
			int         portalID    = (int) ctx->sourceIdent;
			int         notification = ctx->notification;

			CVerbArg2 ( "Update", "portalID", "i", portalID, "", "s", CPP_CLI ( resolveName ( ctx->notification ), CCharToString ( environs::API::ResolveNameN ( ctx->notification ) ) ) );

			PortalInstanceESP portal;
			PortalInfoBaseSP info;
            
            Environs OBJ_ptr env = EnvironsAPI ( hInst );
			if ( !env )
				return;

            int nativeID = ctx->destID;

			int portalKey  = GetKey ( nativeID, portalID );

			if ( !LockAcquireA ( portalsLock, "Update" ) )
				return;

			ContainerIfContains ( portals, portalKey )
				portal = ( *portals ) [ portalKey ];

			LockReleaseVA ( portalsLock, "Update" );

			switch ( notification )
			{
			case NOTIFY_PORTAL_REQUEST_FAIL:
			case NOTIFY_PORTAL_PROVIDE_FAIL:
				KillZombies ();

			case NOTIFY_PORTAL_STREAM_STOPPED:
				Dispose ( nativeID, portalID );
				return;

			case NOTIFY_PORTAL_ASK_REQUEST:
			case NOTIFY_PORTAL_REQUEST: // Make sure that the requested slot is free and if so, then create a portal instance
			case NOTIFY_PORTAL_INCOMING_ESTABLISHED: // An incoming portal has been established. Create a portal instance and let app decide
			{
                C_Only ( if ( Environs::instancesAPI [ hInst ]->environsObservers == 0 ) return );
                
				DeviceInstanceESP device = nill;  env->GetDeviceByNativeID ( device, nativeID );
                
				if ( device == nill ) return;

				if ( notification == NOTIFY_PORTAL_REQUEST )
				{
					if ( portal == nill ) {
						portal = device->PortalCreate ( portalID );

						if ( portal == nill )
							return;
						portal->status_ = PortalStatus::CreatedFromRequest;
					}
					else {
						portal->status_ = PortalStatus::CreatedFromRequest;
						environs::API::ProvidePortalStreamN ( hInst, device->info_->nativeID, CALL_NOWAIT, portalID );
						return;
					}
				}
				else if ( notification == NOTIFY_PORTAL_INCOMING_ESTABLISHED ) {
					if ( portal == nill )
						return;
					portal->status_ = PortalStatus::Established;
				}
				else //if ( notification == NOTIFY_PORTAL_ASK_REQUEST )
				{
					portal = device->PortalCreateID ( portalID );
					if ( portal == nill )
						return;
					portal->status_ = PortalStatus::CreatedAskRequest;
				}

				if ( portal->portalID_ != portalID )
					portal->portalID_ = portalID;

				if ( portal->status_ < PortalStatus::Established ) {
					PresentPortalToObservers ( portal, notification );
					return;
				}

				HandleSuccessfulPortal ( portal, notification );
				return;
			}

			case NOTIFY_PORTAL_PROVIDER_READY:
			case NOTIFY_PORTAL_PROVIDE_STREAM_ACK:
			case NOTIFY_PORTAL_PROVIDE_IMAGES_ACK:
			{
				if ( portal != nill )
				{
					if ( portal->portalID_ != portalID )
						portal->portalID_ = portalID;

					//if ( portal->status_ < PortalStatus::Established )
						portal->status_ = PortalStatus::Established;

					info = PortalInstance::GetPortalInfo ( hInst, portalID );
					if ( info )
						portal->info_.Update ( Notify::Portale::Zero, sp_get ( info ) );

					portal->NotifyObservers ( ( Notify::Portale_t ) notification );
				}
				return;
			}
			default: break;
			}

			if ( portal == nill )
			{
				if ( notification == NOTIFY_PORTAL_STREAM_INCOMING || notification == NOTIFY_PORTAL_IMAGES_INCOMING ) {
					CLog ( "NOTIFY_PORTAL_STREAM_INCOMING" );
				}
				return;
			}

			if ( notification == NOTIFY_PORTAL_STREAM_INCOMING || notification == NOTIFY_PORTAL_IMAGES_INCOMING ) 
			{
				portal->UpdateCallbacks ( notification );
				portal->status_ = PortalStatus::Started;
			}
			else if ( notification == NOTIFY_PORTAL_STREAM_PAUSED )
				portal->status_ = PortalStatus::Established;
			else if ( notification == NOTIFY_PORTAL_ESTABLISHED_RESOLUTION ) {
				if ( !UpdateWidthHeight ( portal, ctx->contextPtr ) )
					return;
			}

			portal->NotifyObservers ( ( Notify::Portale_t ) notification );
		}


		void PortalInstance::UpdateOptions ( environs::ObserverNotifyContext OBJ_ptr ctx )
		{
			CVerbArg1 ( "UpdateOptions", "", "s", CPP_CLI ( resolveName ( ctx->notification ), CCharToString ( environs::API::ResolveNameN ( ctx->notification ) ) ) );

			PortalInstanceESP portal;
			PortalInfoBaseSP info;

            int portalKey = 0;
            
            Environs OBJ_ptr env = EnvironsAPI ( ctx->hEnvirons );
			if ( !env )
				return;
            
            int nativeID = ctx->destID;

			switch ( ctx->notification )
			{
			case NOTIFY_PORTAL_LOCATION_CHANGED:
			case NOTIFY_PORTAL_SIZE_CHANGED:
			case NOTIFY_DEVICE_ON_SURFACE:
			case NOTIFY_DEVICE_NOT_ON_SURFACE:
				portalKey = GetKey ( nativeID, (int) ctx->sourceIdent );

				if ( LockAcquireA ( portalsLock, "UpdateOptions" ) )
				{
#ifdef CLI_CPP
					if ( portals->ContainsKey ( portalKey ) )
#endif
						portal = ( *portals ) [ portalKey ];

					LockReleaseVA ( portalsLock, "UpdateOptions" );

					if ( portal == nill )
						return;

					if ( ( ctx->notification & NOTIFY_PAIRING ) == NOTIFY_PAIRING )
					{
						if ( portal->device_->SetDirectContact ( ctx->notification == NOTIFY_DEVICE_ON_SURFACE ? 1 : 0 ) )
							portal->NotifyObservers ( Notify::Portale::ContactChanged );
					}
					else {
						if ( ctx->contextPtr != nill && ctx->context ) {
							portal->info_.Update ( ( Notify::Portale_t ) ctx->notification, (PortalInfoBase OBJ_ptr) ParsePortalInfo ( ctx->contextPtr ) );
						}
						else {
							info = PortalInstance::GetPortalInfo ( ctx->hEnvirons, ( int ) ctx->sourceIdent );
							if ( info == nill )
								return;
							portal->info_.Update ( ( Notify::Portale_t ) ctx->notification, sp_get ( info ) );
						}
					}
				}

				return;

			default:
				return;
			}

			/*if (portal == 0)
			return;
			[portal NotifyObservers:notification];
			*/
		}


		bool PortalInstance::UpdateWidthHeight ( c_const PortalInstanceSP c_ref portal, Addr_ptr context )
		{
			if ( portal == nill || context == nill )
				return false;

			CVerbArg1 ( "UpdateWidthHeight", "portalID", "i", portal->portalID_ );

			int * sizes = ( int * ) Addr_pvalue ( context );
			int width = *sizes; sizes++;
			int height = *sizes;

			if ( width > 0 && height > 0 ) {
				portal->info_.base.width = width; portal->info_.base.height = height;
				return true;
			}
			return false;
		}


		void PortalInstance::HandleSuccessfulPortal ( c_const PortalInstanceSP c_ref portal, int notification )
		{
			if ( portal == nill )
				return;

			PortalInfoBaseSP info = PortalInstance::GetPortalInfo ( portal->hEnvirons_, portal->portalID_ );
			if ( info != nill )
				portal->info_.Update ( Notify::Portale::Zero, sp_get ( info ) );

			if ( portal->status_ < PortalStatus::Established )
				portal->device_->NotifyObservers ( DEVICE_INFO_ATTR_PORTAL_CREATED, portal->startIfPossible ? false : true );
			else
				portal->NotifyObservers ( ( Notify::Portale_t ) notification );

			if ( portal->startIfPossible && notification == NOTIFY_PORTAL_INCOMING_ESTABLISHED && portal->HasPortalRenderer () )
				portal->Start ();
		}


		CLASS ThreadPackPortalPresenter
		{
		public:
			int notification;
			PortalInstanceSP portal;
		};


		void c_OBJ_ptr PortalInstance::PortalPresenterThread ( pthread_param_t pack )
		{
			ThreadPackPortalPresenterPtr thread = ( ThreadPackPortalPresenterPtr ) pack;

			PortalInstancePtr portal = sp_get ( thread->portal );

			CVerbArg1 ( "PresentPortalToObservers", "portalID", "i", portal->portalID_ );

            bool success = false;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
#   ifndef CLI_CPP
            sp ( DeviceInstance ) deviceSP = portal->device_;
            if ( deviceSP )
            {
                DeviceInstance * device = deviceSP.get ();

                std::vector < lib::IIDeviceObserver * > * observers = &device->observers;

                vct ( lib::IIDeviceObserver * ) obss;

                LockAcquireVA ( device->changeEventLock, "PresentPortalToObservers" );

                size_t size = observers->size ();

                for ( size_t i = 0; i < size; ++i )
                {
                    lib::IIDeviceObserver OBJ_ptr obs = observers->at ( vctSize i );
                    if ( obs != nill ) {
                        obss.push_back ( obs );
                    }
                }

                LockReleaseVA ( device->changeEventLock, "PresentPortalToObservers" );

                size = obss.size ();

                for ( size_t i=0; i < size; i++ )
                {
                    lib::IIDeviceObserver * obs =  obss.at ( i );
                    if ( obs ) {
                        try {
                            if ( !obs->OnEnvironsPortalRequestOrProvided_ )
                                continue;

                            obs->OnPortalRequestOrProvidedBase ( portal );

                            if ( ( portal->observers.size () > 0 ) && portal->establishCalled ) {
                                success = true;
                                break;
                            }
                        }
                        catch ( ... ) {
                            CErr ( "PresentPortalToObservers: Exception!" );
                        }
                    }
                }
            }
#   else
			environs::DeviceInstance ^ device = portal->device_;
			if ( device != nill ) {
				device->OnPortalRequestOrProvided ( portal->GetPlatformObj () );
			}

            //environs::Environs::instancesAPI [portal->hEnvirons_]->OnPortalRequestOrProvided ( portal->GetPlatformObj () );

            if ( portal->establishCalled ) {
                success = true;
            }
#   endif
            
#else

#   ifndef CLI_CPP
            Environs * api = Environs::instancesAPI [ portal->hEnvirons_ ];
            if ( api )
            {
                std::vector < lib::IIEnvironsObserver * > * observers = api->environsObservers.get ();
                
                vct ( lib::IIEnvironsObserver * ) obss;
                
                LockAcquireVA ( api->queryLock, "PresentPortalToObservers" );
                
                size_t size = observers->size ();
                
                for ( size_t i = 0; i < size; ++i )
                {
                    lib::IIEnvironsObserver OBJ_ptr obs = observers->at ( vctSize i );
                    if ( obs != nill ) {
                        obss.push_back ( obs );
                    }
                }
                
                LockReleaseVA ( api->queryLock, "PresentPortalToObservers" );
                
                size = obss.size ();
                
                for ( size_t i=0; i < size; i++ )
                {
                    lib::IIEnvironsObserver * obs =  obss.at ( i );
                    if ( obs ) {
                        try {
                            if ( !obs->OnEnvironsPortalRequestOrProvided_ )
                                continue;
                            
                            obs->OnPortalRequestOrProvidedBase ( portal );
                            
                            if ( ( portal->observers.size () > 0 ) && portal->establishCalled ) {
                                success = true;
                                break;
                            }
                        }
                        catch ( ... ) {
                            CErr ( "PresentPortalToObservers: Exception!" );
                        }
                    }
                }              
            }            
#   else
			environs::Environs::instancesAPI [portal->hEnvirons_]->OnPortalRequestOrProvided ( portal->GetPlatformObj () );

			if ( portal->establishCalled ) {
				success = true;
			}
#   endif
#endif
			if ( success ) {
				HandleSuccessfulPortal ( thread->portal, thread->notification );
			}
			else {
				portal->DisposeInstance ( true );
			}

			delete__obj ( thread );
			return C_Only ( 0 );
		}


		void PortalInstance::PresentPortalToObservers ( c_const PortalInstanceSP c_ref portal, int notification )
		{
            if ( portal == nill )
                return;
#ifndef CLI_CPP
            std::vector < lib::IIEnvironsObserver * > * observers = Environs::instancesAPI [ portal->hEnvirons_ ]->environsObservers.get ();

			if ( observers == nill || !observers->size () )
				return;
#endif
			ThreadPackPortalPresenterPtr thread = new__obj ( ThreadPackPortalPresenter );
			if ( thread == nill )
				return;

			thread->portal = portal;
			thread->notification = notification;

            
			pthread_t threadID;
            
            int ret = pthread_create ( c_Addr_of ( threadID ), NULL, &PortalPresenterThread, ( pthread_param_t ) thread );
            if ( ret != 0 ) {
                CErr ( "PresentPortalToObservers: Failed to create handler thread." );
                delete__obj ( thread );
                return;
            }
            
#ifndef CLI_CPP
            DetachThread ( nill, nill, threadID, "PortalInstance::PresentPortalToObservers" );
#endif
		}


		void PortalInstance::ShowDialogNoWiFiWarn ()
		{
			CVerb ( "ShowDialogNoWiFiWarn" );

			dialogID = PORTAL_TRAFFIC_DIALOG;
		}


		void PortalInstance::ShowDialogOutgoingPortalView ()
		{
			CVerb ( "ShowDialogOutgoingPortalView" );

			dialogID = PORTAL_OUTGOING_DIALOG;
		}
	}

} /* namespace environs */



