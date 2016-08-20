/**
 * DeviceList Object
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
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#	ifndef CLI_CPP
//#		define DEBUGVERBList
#	endif
#endif

#ifdef CLI_CPP
#	include "Environs.Cli.Forwards.h"
#	include "Sensor.Frame.Cli.h"
#else
#	include "Environs.Msg.Types.h"
#endif

#include "Environs.Native.h"
#include "Interop/Threads.h"

#ifdef CLI_CPP
#	include "Environs.h"
#endif

#include "Device.List.h"
#include "Portal.Instance.h"

#ifdef CLI_CPP
#	include "Device/Device.List.Cli.h"
#	include "Device/Device.Instance.Cli.h"
#	include "Utils/Application.Environment.h"
#	include "Environs.Cli.h"
#	include "Environs.Lib.h"

#else
#	include "Device.Instance.h"
#	include "Environs.h"
#	include "Environs.Lib.h"
#	include "Environs.Obj.h"
#	include "Interop/jni.h"
#	include "Core/Array.List.h"
#	include <queue>
#endif

#include "Tracer.h"

using namespace std;
using namespace environs;


#define CLASS_NAME	"Device.List. . . . . . ."


namespace environs 
{
	namespace lib
    {
        sp ( DeviceListUpdatePack ) BuildDeviceListUpdatePack ( c_const devList ( DeviceInstanceEP ) c_ref deviceList )
        {
            sp ( DeviceListUpdatePack ) uiQueue = sp_make ( DeviceListUpdatePack );
            if ( uiQueue == nill ) {
                CErr ( "BuildDeviceListUpdatePack: Failed to create update queue for UI thread." );
                return nill;
            }
            
            uiQueue->appeared = nill;
            uiQueue->vanished = nill;
            
            uiQueue->deviceList = deviceList;
            return uiQueue;
        }
        
        
        ENVIRONS_OUTPUT_ALLOC ( DeviceList );
        

		DeviceList::DeviceList ()
		{
            CVerb ( "Construct" );
            
			disposed_       = false;
			listType        = DeviceClass::All;

            listDevicesLock = nill;
            hEnvirons       = 0;
            isUIAdapter     = false;

            C_Only ( env    = nill );
            
            envObj          = nill;
            
            ENVIRONS_OUTPUT_ALLOC_INIT ();

#ifndef CLI_CPP
			listCached      = nill;
			listCacheEnabled = false;
#else
			listCacheEnabled = true;
#endif
		}


		DeviceList::~DeviceList ()
        {
			CVerbArg1 ( "Destruct", "", "i", objID_ );
            
            listDevicesLock = nill;
            
#ifndef CLI_CPP
            if ( listCached ) {
                listCached->Release ();
                listCached = nill;
            }
#endif
        }
        
        
        /**
         * Release ownership on this interface and mark it disposable.
         * Release must be called once for each Interface that the Environs framework returns to client code.
         * Environs will dispose the underlying object if no more ownership is hold by anyone.
         *
         */
        void DeviceList::Release ()
        {
            ENVIRONS_OUTPUT_RELEASE ();

#ifndef CLI_CPP
            if ( localRefCount == 0 ) {
                
                PlatformDispose ();
                
				delete this;
            }
#endif
        }
        

		void DeviceList::SetListType ( environs::DeviceClass_t type )
        {
			CVerbArg2 ( "SetListType", "", "i", objID_, "type", "i", type );
            
			listDevices = DeviceList::GetDevices ( type );

			if ( listDevices == nill ) {
				type = DeviceClass::All;
				listDevices = DeviceList::GetDevices ( type );
			}

			switch ( type ) {
			case DeviceClass::All:
				listDevicesLock = Addr_of ( envObj->listAllLock );
                    
				listDevicesObservers = envObj->listAllObservers;
                    
				break;

			case DeviceClass::Nearby:
				listDevicesLock = Addr_of ( envObj->listNearbyLock );
                    
				listDevicesObservers = envObj->listNearbyObservers;
                    
                break;

			case DeviceClass::Mediator:
				listDevicesLock = Addr_of ( envObj->listMediatorLock );

				listDevicesObservers = envObj->listMediatorObservers;

                break;
			}
			listType = type;
        }
        
        
        void DeviceList::SetIsUIAdapter ( bool enable )
        {
            if ( enable )
                EnvironsAPI ( hEnvirons )->isUIAdapter = enable;
            isUIAdapter = enable;
        }

        
        /**
         * Enable caching of the list returned by GetDevices() and update on call of GetDevices() for single threaded usagge.
         *
         * @param enable        true = enable, false = disable (default).
         * Note for C++/Obj-C API: The cached list returned by GetDevices() is intended for single thread applications. 
         *          A call to GetDevices() while the cached list is still held by a thread is not allowed and might end up in invalid memory access.
         */
        void DeviceList::SetEnableListCache ( bool enable )
        {
            listCacheEnabled = enable;
            
#ifndef CLI_CPP
            if ( Lock () ) {
                if ( !enable && listCached ) {
                    listCached->Release ();
                    listCached = nill;
                }
                Unlock ();
            }
#endif
        }
        
        
        bool DeviceList::disposed ()
		{
			CVerbVerb ( "disposed" );

            return disposed_;
        }


		void DeviceList::GetItem ( DeviceInstanceESP OBJ_ref device, int position )
        {
            CVerbVerb ( "GetItem" );

			if ( disposed_ || listDevices == nill )
				return;
            
			if ( listDevices == nill ) {
				SetListType ( DeviceClass::All );

				if ( listDevices == nill )
					return;
			}

            bool locked = false;

			if ( isUIAdapter && !IsUIThread () ) { // What if we're not using UIAdapter. No lock?
				if ( !LockAcquire ( listDevicesLock, "GetItem" ) )
					return;
				locked = true;
			}
            
            if ( position < ( int ) vp_size ( listDevices ) )
                device = vp_at ( listDevices, position );
            
            if ( locked )
                LockReleaseV ( listDevicesLock, "GetItem" );
        }


		DeviceInstanceESP DeviceList::GetItem ( int position )
        {
            CVerbVerb ( "GetItem" );
            
			DeviceInstanceESP device = nill; GetItem ( device, position );
            
			return device;
        }
       

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetItemRetained ( int position )
        {
            CVerbVerb ( "GetItemRetained" );
            
			DeviceInstanceESP device; // SP will be initialized by constructor (C++)

			DeviceList::GetItem ( device, position );

            if ( device != nill && device->Retain () )
				return device.get ();
            return nill;
        }
#endif

		int DeviceList::GetCount ()
        {
            CVerbVerb ( "GetCount" );

			if ( disposed_ || listDevices == nill )
				return 0;
            
			if ( listDevices == nill ) {
				SetListType ( DeviceClass::All );

				if ( listDevices == nill )
					return 0;
            }
            
            bool locked = false;

			if ( isUIAdapter && !IsUIThread () ) {
				if ( !LockAcquire ( listDevicesLock, "GetCount" ) )
					return 0;
				locked = true;
			}
            
            int size = (int) vp_size ( listDevices );
            
            if ( locked )
                LockReleaseV ( listDevicesLock, "GetCount" );
            
            return size;
		}


		void DeviceList::AddObserver ( environs::ListObserverPtr observer )
        {
            CVerb ( "AddObserver" );
            
			if ( observer == nill || disposed_ ) return;

#ifdef CLI_CPP
#	ifndef CLI_NOUI
			if ( isUIAdapter ) {
				((ObservableCollection<environs::DeviceInstance ^> ^) GetDevicesSource ())->CollectionChanged += observer;
				return;
			}
#	endif
#endif
			if ( listDevicesObservers == nill ) return;

			if ( !LockAcquireA ( envObj->listLock, "AddObserver" ) )
                return;

			int i = 0;
			int size = ( int ) vp_size ( listDevicesObservers );

			for (; i<size; i++ ) {
				environs::ListObserverPtr obs = ( environs::ListObserverPtr ) vp_at ( listDevicesObservers, i );
				if ( observer == obs )
					break;
			}

			if ( i >= size ) {
				ContainerAppend ( listDevicesObservers, ( CPP_CLI ( lib::IIListObserver, ListObserver ) OBJ_ptr ) observer );
			}

			LockReleaseVA ( envObj->listLock, "AddObserver" );
		}


		void DeviceList::RemoveObserver ( environs::ListObserverPtr observer )
        {
            CVerb ( "RemoveObserver" );

			if ( observer == nill || disposed_ ) return;

#ifdef CLI_CPP
#	ifndef CLI_NOUI
			if ( isUIAdapter ) {
				( ( ObservableCollection<environs::DeviceInstancePtr> ^ ) GetDevicesSource () )->CollectionChanged -= observer;
				return;
			}
#	endif
#endif
			if ( listDevicesObservers == nill ) return;

			LockAcquireVA ( envObj->listLock, "RemoveObserver" );

			int size = ( int ) vp_size ( listDevicesObservers );

			for ( int i=0; i<size; i++ ) {
				environs::ListObserverPtr obs = ( environs::ListObserverPtr ) vp_at ( listDevicesObservers, i );

				if ( observer == obs ) {
					ContainerRemoveAt ( listDevicesObservers, i );
					break;
				}
			}

			LockReleaseVA ( envObj->listLock, "RemoveObserver" );
		}


		DeviceInstanceESP DeviceList::RefreshItem ( DeviceInstanceEPtr source, DeviceObserverPtr observer )
        {
            CVerb ( "RefreshItem" );

			if ( disposed_ || listDevices == nill )
				return nill;

			if ( LockAcquire ( listDevicesLock, "RefreshItem" ) )
			{
				size_t count = vp_size ( listDevices );

				for ( size_t i=0; i<count; i++ )
				{
					c_const DeviceInstanceESP c_ref device = vp_at ( listDevices, i );

					if ( device != nill &&  device->EqualsID ( source ) )
					{
						if ( observer != nill )
							device->AddObserver ( observer );

						LockReleaseV ( listDevicesLock, "RefreshItem" );
						return device;
					}
				}

				LockReleaseV ( listDevicesLock, "RefreshItem" );

			}
			return nill;
        }        


#ifndef CLI_CPP
		environs::DeviceInstancePtr DeviceList::RefreshItemRetained ( environs::DeviceInstancePtr source, environs::DeviceObserverPtr observer )
        {
            CVerb ( "RefreshItemRetained" );

			if ( disposed_ || listDevices == nill )
				return nill;
            
            const DeviceInstanceSP &device = RefreshItem ( ( DeviceInstancePtr ) source, observer );
            if ( device != nill ) {
				if ( device->Retain () )
                    return device.get ();
            }
            return nill;
        }
#endif


		c_const devList ( DeviceInstanceEP ) c_ref DeviceList::GetDevices ( DeviceClass_t type )
        {
            CVerb ( "GetDevices" );

			return envObj->GetDevices ( (int) type );
		}

		
		/**
		* Get a DeviceInstance that matches the given arguments.
		*
		* @return DeviceInstance objects
		*/
		void DeviceList::GetDevice ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, int deviceID, CString_ptr areaName, CString_ptr appName, int * pos )
		{
			CVerbVerb("GetDevice");

			if ( lock && !LockAcquire ( lock, "GetDevice" ) )
				return;

			int size = ( int ) vp_size ( deviceList );

			for ( int i = 0; i < size; i++ ) {
				c_const DeviceInstanceESP c_ref tmp = vp_at ( deviceList, i );

				if ( tmp != nill && tmp->EqualsID ( deviceID, areaName, appName ) ) {
					if ( pos != nill ) *pos = i;
					device = tmp;
					break;
				}
			}

			if ( lock )
				LockRelease ( lock, "GetDevice" );
		}


		/**
		* Get a DeviceInstance that matches the given arguments.
		*
		* @return DeviceInstance objects
		*/
		void DeviceList::GetDevice ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, OBJIDType objID, int * pos )
		{
			CVerbVerb("GetDevice");

			if ( lock && !LockAcquire ( lock, "GetDevice" ) )
                return;

			int size = ( int ) vp_size ( deviceList );

			for ( int i = 0; i < size; i++ ) {
				c_const DeviceInstanceESP c_ref tmp = vp_at ( deviceList, i );

				if ( tmp != nill && ( tmp->info_->objID == objID || tmp->objIDPrevious == objID) ) {
                    if ( pos != nill ) *pos = i;
					device = tmp;
					break;
				}
			}

			if ( lock )
				LockRelease ( lock, "GetDevice" );
		}


		/**
		* Get a DeviceInstance that matches the given arguments.
		*
		* @return DeviceInstance objects
		*/
		void DeviceList::GetDeviceByNativeID ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, int nativeID )
		{
			CVerbVerb ( "GetDevice" );

			if ( lock && !LockAcquire ( lock, "GetDeviceByNativeID" ) )
				return;

			int size = ( int ) vp_size ( deviceList );

			for ( int i = 0; i < size; i++ ) {
				c_const DeviceInstanceESP c_ref tmp = vp_at ( deviceList, i );

				if ( tmp != nill && tmp->info_->nativeID == nativeID ) {
					device = tmp;
					break;
				}
			}

			if ( lock )
				LockRelease ( lock, "GetDeviceByNativeID" );
		}


		/**
		* Lock access to the devicelist returned by Source methods.
		*
		* @return success
		*/
		bool DeviceList::Lock ()
		{
			if ( listDevicesLock == nill )
				return false;
			return LockAcquire ( listDevicesLock, "DeviceList.Lock" );
		}


		/**
		* Unlock access to the devicelist returned by Source methods.
		*
		* @return success
		*/
		bool DeviceList::Unlock ()
		{
			if ( listDevicesLock == nill )
				return false;
			return LockRelease ( listDevicesLock, "DeviceList.Unlock" );
		}


		/**
		* Get a copy of the collection that holds the devices according to the specified listtype.
		*
		* @return Collection with DeviceInstance objects
		*/
		devList ( DeviceInstanceEP ) DeviceList::GetDevices ()
        {
            CVerbVerb ( "GetDevices" );
            
			c_const devList ( DeviceInstanceEP ) c_ref list = GetDevicesSource ();
			if ( list == nill )
				return nill;

			if ( !Lock () )
				return nill;

 			devList ( DeviceInstanceEP ) cloned = devListNewArg ( isUIAdapter, DeviceInstanceEP, list );

			if ( !Unlock () )
				return nill;

			return cloned;
		}


		/**
		* Get a collection that holds all available devices. This list is updated dynamically by Environs.
		*
		* @return ArrayList with DeviceInstance objects
		*/
		c_const devList ( DeviceInstanceEP ) c_ref DeviceList::GetDevicesSource ()
		{
			CVerbVerb ( "GetDevices" );

			if ( listDevices == nill ) {
				listDevices = GetDevices ( DeviceClass::All );
				listDevicesLock = OBJ_ref envObj->listAllLock;
			}
			return listDevices;
		}

#ifndef CLI_CPP
		environs::ArrayList * DeviceList::GetDevicesRetained ()
        {
            CVerbVerb ( "GetDevicesRetained" );

			if ( disposed_ || listDevices == nill )
                return nill;
            
            if ( !Lock () )
                return nill;
            
            environs::ArrayList * list = nill;
            
            if ( listCacheEnabled ) {
                if ( listCached ) {
                    ((lib::ArrayList *) listCached)->UpdateWithDevices ( listDevices );
                    
                    // Increment reference count (as the caller will decrease the count)
                    if ( listCached->Retain () )
                        list = listCached;
                }
                
                else {
                    listCached = ArrayList::CreateWithDevicesRetained ( listDevices );
                    if ( listCached ) {
                        // Increment reference count for the cache reference
                        if ( listCached->Retain () )
                            list = listCached;
                    }
                }
            }
            else {
                list = ArrayList::CreateWithDevicesRetained ( listDevices );
            }
            
            Unlock ();
            
            return list;
        }
#endif


		/**
		* Query the number of all available devices within the environment (including those of the Mediator)
		*
		* @return numberOfDevices
		*/
		int DeviceList::GetDevicesCount ()
        {
            CVerbVerb ( "GetDevicesCount" );

			if ( disposed_ || listDevices == nill )
				return 0;

			return ( int ) vp_size ( listDevices );
		}


		/**
		* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
		*
		* @param deviceID		The device id of the target device.
		* @param areaName		Area name of the application environment
		* @param appName		Application name of the application environment
		* @return DeviceInstance-object
		*/
		DeviceInstanceESP DeviceList::GetDevice ( int deviceID, CString_ptr areaName, CString_ptr appName, int * pos )
        {
            CVerbVerb ( "GetDevice" );

			if ( disposed_ || listDevices == nill )
				return nill;

			DeviceInstanceESP device = nill;
			
			GetDevice ( listDevices, listDevicesLock, device, deviceID, areaName, appName, pos );

			return device;
        }

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceRetained ( int deviceID, const char * areaName, const char * appName, int * pos )
        {
            CVerbVerb ( "GetDeviceRetained" );

			if ( disposed_ || listDevices == nill )
				return nill;
            
			DeviceInstanceESP device; // SP will be initialized by constructor (C++)

			GetDevice ( listDevices, listDevicesLock, device, deviceID, areaName, appName, pos );

            if ( device != nill && device->Retain () )
				return device.get ();
            return nill;
        }
#endif

		void DeviceList::GetDeviceBestMatchNative ( DeviceInstanceESP OBJ_ref device, int deviceID )
		{
			CVerbVerb ( "GetDeviceBestMatchNative" );

			if ( disposed_ )
				return;

			environs::lib::DeviceInfo * deviceInfo = ( environs::lib::DeviceInfo * ) environs::API::GetDeviceBestMatchN ( hEnvirons, deviceID );
			if ( deviceInfo != nill )
			{
				GetDevice ( device, deviceInfo->objID, 0 );

				free_plt ( deviceInfo );
			}
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
		DeviceInstanceESP DeviceList::GetDeviceBestMatchNative ( int deviceID )
        {
            CVerbVerb ( "GetDeviceBestMatchNative" );

			DeviceInstanceESP device = nill; GetDeviceBestMatchNative ( device, deviceID ); return device;
        }

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceBestMatchNativeRetained ( int deviceID )
        {
            CVerbVerb ( "GetDeviceBestMatchNativeRetained" );
            
			DeviceInstanceSP device; // SP will be initialized by constructor (C++)
			
			GetDeviceBestMatchNative ( device, deviceID );

            if ( device != nill && device->Retain () )
				return device.get ();
            return 0;
        }
#endif

		void DeviceList::GetDeviceSeeker ( c_const devList ( DeviceInstanceEP ) c_ref list, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID )
		{
			CVerbVerb ( "GetDeviceSeeker" );

			if ( list == nill )
				return;

			if ( !LockAcquire ( lock, "GetDeviceSeeker" ) )
				return;

			size_t count = vp_size ( list );

			for ( size_t i = 0; i < count; i++ )
			{
				c_const DeviceInstanceESP c_ref tmp = vp_at ( list, vctSize i );
				if ( tmp == nill )
					continue;

				if ( isObjID ) {
					if ( tmp->info_->objID == objOrDeviceID || tmp->objIDPrevious == objOrDeviceID ) {
						device = tmp; break;
					}
				}
				else {
					if ( tmp->info_->deviceID == ( int ) objOrDeviceID ) {
						device = tmp; break;
					}
				}

			}

			LockReleaseV ( lock, "GetDeviceSeeker" );
		}


		void DeviceList::GetDevice ( DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID )
        {
            CVerbVerb ( "GetDevice" );
            
			if ( disposed_ || listDevices == nill )
				return;
			
			GetDeviceSeeker ( listDevices, listDevicesLock, device, objOrDeviceID, isObjID );
		}


		void DeviceList::GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID )
		{
			CVerbVerb ( "GetDeviceAll" );

			if ( disposed_ )
				return;

			GetDeviceSeeker ( GetDevices ( DeviceClass::All ), Addr_of ( envObj->listAllLock ), device, objOrDeviceID, isObjID );
		}
        
        
        /**
         * Query a DeviceInstance object that first match the deviceID only.
         * Usually the one that is in the same app environment is picked up.
         * If there is no matching in the app environment,
         * then the areas are searched for a matchint deviceID.
         *
         * @param deviceID      The portalID that identifies an active portal.
         * @return DeviceInstance-object
         */
		DeviceInstanceESP DeviceList::GetDeviceBestMatch ( int deviceID )
        {
            CVerbVerb ( "GetDeviceBestMatch" );

			DeviceInstanceESP device = nill;
            
			DeviceList::GetDevice ( device, deviceID, false );

			return device;
		}

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceBestMatchRetained ( int deviceID )
        {
            CVerbVerb ( "GetDeviceBestMatchRetained" );
            
			DeviceInstanceSP device; // SP will be initialized by constructor (C++)
			
			DeviceList::GetDevice ( device, deviceID, false );
            
			if ( device != nill && device->Retain () )
				return device.get ();
            return 0;
        }
#endif
        
        
        /**
         * Query a DeviceInstance object that first match the deviceID only.
         * Usually the one that is in the same app environment is picked up.
         * If there is no matching in the app environment,
         * then the areas are searched for a matchint deviceID.
         *
         * @param objID      The objID that identifies the device.
         * @return DeviceInstance-object
         */
		DeviceInstanceESP DeviceList::GetDevice ( OBJIDType objID )
        {
            CVerbVerb ( "GetDevice" );

			DeviceInstanceESP device = nill;
			
			DeviceList::GetDevice ( device, objID, true );

			return device;
        }

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceRetained ( OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceRetained" );
            
			DeviceInstanceSP device; // SP will be initialized by constructor (C++)

			DeviceList::GetDevice ( device, objID, true );

            if ( device != nill && device->Retain () )
				return device.get ();
            return 0;
        }
#endif


		/**
		* Query a DeviceInstance object that first match the deviceID only.
		* Usually the one that is in the same app environment is picked up.
		* If there is no matching in the app environment,
		* then the areas are searched for a matchint deviceID.
		*
		* @param deviceID      The portalID that identifies an active portal.
		* @return DeviceInstance-object
		*/
		DeviceInstanceESP DeviceList::GetDeviceAll ( OBJIDType objID )
		{
			CVerbVerb ( "GetDeviceAll" );

			DeviceInstanceESP device = nill;
			
			GetDeviceAll ( device, objID, true );

			return device;
		}
        
        /**
         * Release the ArrayList that holds the available devices.
         */
		void DeviceList::ReleaseDevices ()
        {
            CVerb ( "ReleaseDevices" );
            
			DisposeList ( isUIAdapter, envObj->listAll, OBJ_ref envObj->listAllLock );
		}
    
        
        /**
         * Get a copy of the collection that holds the nearby devices.
         *
         * @return ArrayList with DeviceInstance objects
         */
		devList ( DeviceInstanceEP ) DeviceList::GetDevicesNearby ()
        {
            CVerb ( "GetDevicesNearby" );

			c_const devList ( DeviceInstanceEP ) c_ref list = GetDevicesNearbySource ();

			if ( list == nill || !LockAcquireA ( envObj->listNearbyLock, "DeviceList.GetDevicesNearby" ) )
				return nill;

			devList ( DeviceInstanceEP ) cloned = devListNewArg ( isUIAdapter, DeviceInstanceEP, list );

			if ( !LockReleaseA ( envObj->listNearbyLock, "DeviceList.GetDevicesNearby" ) )
				return nill;

			return cloned;
		}

		/**
		* Get the collection that holds the nearby devices.
		* This list is updated dynamically by Environs (even when user code access the list).
		*
		* @return Collection with DeviceInstance objects
		*/
		c_const devList ( DeviceInstanceEP ) c_ref DeviceList::GetDevicesNearbySource ()
		{
			CVerb ( "GetDevicesNearbySource" );

			return GetDevices ( DeviceClass::Nearby );
		}

#ifndef CLI_CPP
		environs::ArrayList * DeviceList::GetDevicesNearbyRetained ()
        {
            CVerb ( "GetDevicesNearbyRetained" );

			const svsp ( DeviceInstance ) &result = DeviceList::GetDevices ( DeviceClass::Nearby );

			return ArrayList::CreateWithDevicesRetained ( result );
        }
#endif


		/**
		* Query the number of nearby (broadcast visible) devices within the environment.
		*
		* @return numberOfDevices
		*/
		int DeviceList::GetDevicesNearbyCount ()
        {
            CVerbVerb ( "GetDevicesNearbyCount" );

			return ( int ) vp_size ( GetDevicesNearbySource () );
		}


		/**
		* Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
		*
		* @param objID      The objID of the target device.
		* @return DeviceInstance-object
		*/
		DeviceInstanceESP DeviceList::GetDeviceNearby ( OBJIDType objID )
        {
            CVerbVerb ( "GetDevicesNearby" ); 
			
			DeviceInstanceESP device = nill;
            
			DeviceList::GetDevice ( GetDevicesNearbySource (), OBJ_ref envObj->listNearbyLock, device, objID, 0 );

			return device;
		}

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceNearbyRetained ( OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceNearbyRetained" );
            
            DeviceInstanceSP device;

			DeviceList::GetDevice ( GetDevicesNearbySource (), OBJ_ref envObj->listNearbyLock, device, objID, 0 );

            if ( device != nill && device->Retain () )
				return device.get ();
            return nill;
        }
#endif


		/**
		* Release the ArrayList that holds the nearby devices.
		*/
		void DeviceList::ReleaseDevicesNearby ()
        {
            CVerb ( "ReleaseDevicesNearby" );
            
			DisposeList ( isUIAdapter, envObj->listNearby, OBJ_ref envObj->listNearbyLock );
		}


		/**
		* Query a DeviceInstance object of Mediator managed devices within the environment.
		*
		* @param objID      The objID of the target device.
		* @return DeviceInstance-object
		*/
		DeviceInstanceESP DeviceList::GetDeviceFromMediator ( OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceFromMediator" );

			DeviceInstanceESP device = nill;

			GetDevice ( GetDevicesFromMediatorSource (), OBJ_ref envObj->listMediatorLock, device, objID, 0 );

			return device;
        }

#ifndef CLI_CPP
		environs::DeviceInstance * DeviceList::GetDeviceFromMediatorRetained ( OBJIDType objID )
        {
            CVerbVerb ( "GetDeviceFromMediatorRetained" );
            
            DeviceInstanceSP device;

			GetDevice ( GetDevicesFromMediatorSource (), OBJ_ref envObj->listMediatorLock, device, objID, 0 );

            if ( device != nill && device->Retain () )
				return device.get ();
            return nill;
        }
#endif

		/**
		* Get a copy of the collection that holds the Mediator server devices.
		*
		* @return ArrayList with DeviceInstance objects
		*/
		devList ( DeviceInstanceEP ) DeviceList::GetDevicesFromMediator ()
        {
            CVerb ( "GetDevicesFromMediator" );

			c_const devList ( DeviceInstanceEP ) c_ref list = GetDevicesFromMediatorSource ();

			if ( list == nill || !LockAcquireA ( envObj->listMediatorLock, "DeviceList.GetDevicesFromMediator" ) )
				return nill;

			devList ( DeviceInstanceEP ) cloned = devListNewArg ( isUIAdapter, DeviceInstanceEP, list );

			if ( !LockReleaseA ( envObj->listMediatorLock, "DeviceList.GetDevicesFromMediator" ) )
				return nill;

			return cloned;
		}

		/**
		* Get the collection that holds the Mediator server devices.
		* This list is updated dynamically by Environs (even when user code access the list).
		*
		* @return Collection with DeviceInstance objects
		*/
		c_const devList ( DeviceInstanceEP ) c_ref DeviceList::GetDevicesFromMediatorSource ()
		{
			CVerb ( "GetDevicesFromMediatorSource" );

			return GetDevices ( DeviceClass::Mediator );
		}

#ifndef CLI_CPP
		environs::ArrayList * DeviceList::GetDevicesFromMediatorRetained ()
        {
			CVerb ( "GetDevicesFromMediatorRetained" );

			const svsp ( DeviceInstance ) &result = DeviceList::GetDevices ( DeviceClass::Mediator );

			return ArrayList::CreateWithDevicesRetained ( result );
        }
#endif
        

		/**
		* Query the number of Mediator managed devices within the environment.
		*
		* @return numberOfDevices (or -1 for error)
		*/
		int DeviceList::GetDevicesFromMediatorCount ()
        {
            CVerbVerb ( "GetDevicesFromMediatorCount" );

			return ( int ) vp_size ( GetDevicesFromMediatorSource () );
		}

		/**
		* Release the ArrayList that holds the Mediator server devices.
		*/
		void DeviceList::ReleaseDevicesMediator ()
        {
            CVerb ( "ReleaseDevicesMediator" );
            
			DisposeList ( isUIAdapter, envObj->listMediator, OBJ_ref envObj->listMediatorLock );
        }
        
        
        /**
         * Dispose all device lists.
         * This method is intended to be used by the platform layer when the framework shuts down.
         */
        void DeviceList::DisposeLists ()
        {
            CVerb ( "DisposeLists" );
            
            envObj->DisposeLists ( true );
        }
        
        
        /**
         * Dispose device list.
         * This method is intended to be used by client code to enforce disposal.
         */
        void DeviceList::DisposeList ()
        {
            CVerb ( "DisposeList" );
            
            envObj->DisposeList ( (int) listType );
        }
        

#define ENABLE_UI_ADAPTER_UI_THREAD

		bool DeviceListClearQ ( c_const sp ( DeviceListUpdatePack ) c_ref uiQueue )
		{
			sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
			if ( item != nill ) {
				item->cmd = DEVICELIST_QUEUE_COMMAND_CLEAR;

				ContainerdAppend ( uiQueue->items, item );
				return true;
			}
			return false;
		}

		bool DeviceListAppendQ ( c_const sp ( DeviceListUpdatePack ) c_ref uiQueue, DeviceInstanceESP c_ref device )
		{
			sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
			if ( item != nill ) {
				item->cmd = DEVICELIST_QUEUE_COMMAND_APPEND;
				item->device = device;

				ContainerdAppend ( uiQueue->items, item );
				return true;
			}
			return false;
		}

		bool DeviceListInsertQ ( c_const sp ( DeviceListUpdatePack ) c_ref uiQueue, int pos, DeviceInstanceESP c_ref device )
		{
			sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
			if ( item != nill ) {
				item->cmd = DEVICELIST_QUEUE_COMMAND_INSERT_AT;
				item->device = device;
				item->pos = pos;

				ContainerdAppend ( uiQueue->items, item );
				return true;
			}
			return false;
		}

		bool DeviceListRemoveAtQ ( c_const sp ( DeviceListUpdatePack ) c_ref uiQueue, int pos )
		{
			sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
			if ( item != nill ) {
				item->cmd = DEVICELIST_QUEUE_COMMAND_REMOVE_AT;
				item->pos = pos;

				ContainerdAppend ( uiQueue->items, item );
				return true;
			}
			return false;
		}


		bool DeviceList::DeviceListUpdateDataSourceSync ( c_const sp ( DeviceListUpdatePack ) c_ref uiQueue )
		{
			bool updated = false;

			if ( vd_size ( uiQueue->items ) <= 0 )
			{
				DeviceList::DeviceListUpdaterDo ( uiQueue->api, uiQueue->listType, sp_get ( uiQueue->deviceList ),
					uiQueue->devices, uiQueue->devicesCount, sp_get ( uiQueue->vanished ), sp_get ( uiQueue->appeared ), uiQueue->updates );
				return true;
			}

			size_t size = vd_size ( uiQueue->items );

			for ( size_t i = 0; i < size; ++i )
			{
				c_const sp ( DeviceListQueueItem ) c_ref item = vd_at ( uiQueue->items, i );

				if ( item == nill )
					continue;

				switch ( item->cmd )
				{
				case DEVICELIST_QUEUE_COMMAND_CLEAR:
					ContainerClear ( uiQueue->deviceList );
					updated = true;
					break;

				case DEVICELIST_QUEUE_COMMAND_APPEND:
					ContainerAppend ( uiQueue->deviceList, item->device );
					updated = true;
					break;

				case DEVICELIST_QUEUE_COMMAND_INSERT_AT:
					if ( item->pos > (int) vp_size ( uiQueue->deviceList ) )
						break;
					ContainerInsert ( uiQueue->deviceList, item->pos, item->device );
					updated = true;
					break;

				case DEVICELIST_QUEUE_COMMAND_REMOVE_AT:
					if ( item->pos >= (int) vp_size ( uiQueue->deviceList ) )
						break;
					ContainerRemoveAt ( uiQueue->deviceList, item->pos );
					updated = true;
					break;

				case DEVICELIST_QUEUE_COMMAND_INSERT_CALL:
					InsertDeviceDo ( uiQueue->deviceList, item->device, uiQueue->updates );
                    updated = true;
					break;

				case DEVICELIST_QUEUE_COMMAND_DISPOSE_LIST:
					DisposeListDo ( uiQueue->deviceList );
					updated = true;
                    break;
                    
                case DEVICELIST_QUEUE_COMMAND_LOCK:
                    return LockAcquire ( uiQueue->lock, "DeviceListUpdateDataSourceSync" );
                    break;

				default:
					break;
				}
			}

			return updated;
		}
        
        
        /**
         * Dispose the DeviceInstances of the given list.
         */
		void DeviceList::DisposeList ( bool isUIAdapter, c_const devList ( DeviceInstanceEP ) list, pthread_mutex_t_ptr lock )
        {
            CVerb ( "DisposeList" );

			if ( list == nill )
				return;

            bool isUIThread     = IsUIThread ();
            bool locked         = false;
            
            if ( !isUIAdapter || !isUIThread ) {
				if ( !LockAcquire ( lock, "DisposeList" ) )
					return;
				locked = true;
            }            

			size_t count = vp_size ( list );

			if ( count > 0 ) {
				if ( isUIAdapter && !isUIThread )
				{
					sp ( DeviceListUpdatePack ) uiQueue = BuildDeviceListUpdatePack ( list );
					if ( uiQueue == nill ) {
						CErr ( "DisposeList: Failed to create update queue for UI thread." );
					}
                    else {
                        sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
                        if ( item != nill )
                        {
                            item->cmd = DEVICELIST_QUEUE_COMMAND_DISPOSE_LIST;
                            
                            ContainerdAppend ( uiQueue->items, item );
                            
                            DeviceListUpdateDispatchSync ( uiQueue );
                        }
                    }
				}
				else
                    DisposeListDo ( list );
			}

			if ( locked && !LockRelease ( lock, "DisposeList" ) )
				return;
        }


		/**
		* Dispose the DeviceInstances of the given list.
		*/
		void DeviceList::DisposeListDo ( c_const devList ( DeviceInstanceEP ) c_ref list )
		{
			CVerb ( "DisposeListDo" );

			if ( list == nill )
                return;
            
            size_t count = vp_size ( list );            
            if ( count <= 0 )
                return;

            for ( size_t i = 0; i < count; i++ )
            {
                DeviceInstanceReferenceESP device = vp_at ( list, vctSize i );
                if ( device != nill )
                {
                    TRACE_DEVICE_INSTANCE ( device->gotRemoves2++ );

                    device->DisposeInstance ();
                    device->PlatformDispose ();
                }
            }
            ContainerClear ( list );
		}


		/**
		* Refresh all device lists.
		*/
		void DeviceList::ReloadLists ()
        {
            CVerb ( "ReloadLists" );
            
            envObj->ReloadLists ();
		}


		/**
		* Refresh all device lists.
		*/
		void DeviceList::Reload ()
		{
			CVerb ( "Reload" );

			envObj->DeviceListUpdate ( ( int ) listType );
		}

        
        void c_OBJ_ptr DeviceList::CommandThread ( pthread_param_t obj )
        {
            CVerbVerb ( "CommandThread" );
            
            environs::lib::EnvironsPtr                      envObj  = ( environs::lib::EnvironsPtr ) obj;
            
            ListCommandContextPtr                           ctx     = nill;

			envQueueVector ( ListCommandContextPtr ) OBJ_ref  q       = envObj->listCommandQueue;

			pthread_mutex_t                        OBJ_ref  mutex   = envObj->listCommandThreadLock;
            
            ThreadSync                             OBJ_ref  thread  = envObj->listCommandThread;
            
            
            while ( envObj->listCommandThreadRun )
            {
                LockAcquireVA ( mutex, "CommandThread" );
                
                if ( envQueue_empty ( q ) )
                    ctx = nill;
                else {
                    CVerbVerb ( "CommandThread: Dequeue" );
                    ctx = C_Only ( ( ListCommandContextPtr ) ) envQueue_front ( q );
                    
                    CVerbVerb ( "CommandThread: pop" );
					envQueue_pop ( q );
                }                

                LockReleaseVA ( mutex , "CommandThread" );
                
                if ( ctx == nill ) {
                    if ( !thread.WaitOne ( "CommandThread", ENV_INFINITE_MS, true, false ) )
                        break;
                    continue;
                }
                
                CVerbArg1 ( "CommandThread", "Notif", "s", CPP_CLI ( resolveName ( ctx->notification ), CCharToString ( environs::API::ResolveNameN ( ctx->notification ) ) ) );
                
                if ( ctx->type == 0 )
                    RemoveDevice ( ctx );
                else if ( ctx->type == 1 )
                    UpdateDevice ( ctx );
                else if ( ctx->type == 2 ) {
                    NotifyListObservers ( ctx->hEnvirons, ctx->listType, false );
                }
                else if ( ctx->type == 3 )
				{
					pthread_mutex_t_ptr lock = nill;

					c_const devList ( DeviceInstanceEP ) c_ref  list = envObj->GetDevicesBest ( lock );
                    if ( list != nill )
                    {
                        DeviceInstanceESP dev = nill;
                        
                        if ( ctx->device )
                            DeviceList::GetDevice ( list, lock, dev, ctx->device->objID, 0 );
                        else
                            DeviceList::GetDevice ( list, lock, dev , (int) ctx->destID, STRING_get ( ctx->areaName ), STRING_get ( ctx->appName ), 0 );
                        if ( dev != nill ) {
                            /// Lock the changeEvent mutex
                            pthread_cond_mutex_lock ( &dev->changeEventLock );
                            
                            /// Signal a changeEvent
                            if ( pthread_cond_signal ( c_Addr_of ( dev->changeEvent ) ) ) {
                                CVerb ( "CommandThread: Failed to signal changeEvent!" );
                            }
                            
                            /// UnLock the changeEvent mutex
                            pthread_cond_mutex_unlock ( &dev->changeEventLock );
                        }
                    }
                }
                
#ifndef CLI_CPP
                CVerbVerb ( "CommandThread: delete" );

                CheckListCommandContexts ( &mutex, ctx, true );

                delete__obj_n ( ctx );
#endif
                CVerbVerb ( "CommandThread: next" );
            }
            
            
            //
            // Drain the queue. Clear all references to device instances and other objects
            //
            envObj->ListCommandQueueClear ();
            
            CVerbVerb ( "CommandThread: done" );

            C_Only ( return 0 );
        }
        

        void DeviceList::EnqueueCommand ( ListCommandContextPtr ctx )
        {
            EnvironsPtr envObj = ctx->envObj;
            
            if ( !envObj->listCommandThreadRun )
            {
                //envObj->DisposeListCommandContext ( ctx );
                ctx->device = nill;

                CheckListCommandContexts ( &envObj->listCommandThreadLock, ctx, true );

                delete__obj_n ( ctx );
                return;
            }

            LockAcquireVA ( envObj->listCommandThreadLock , "EnqueueCommand" );

            CheckListCommandContexts ( 0, ctx, false );
            
#ifdef CLI_CPP
            envQueue_push ( envObj->listCommandQueue, ( ListCommandContextPtr ) ctx );
            
            CVerbVerb ( "EnqueueCommand: Enqueue" );
#else
            if ( !envQueue_push ( envObj->listCommandQueue, ( ListCommandContextPtr ) ctx ) )
            {
                CVerbVerb ( "EnqueueCommand: Enqueue: Failed!" );
                delete ( ctx );
            }
            else {
                CVerbVerb ( "EnqueueCommand: Enqueue" );
            }
#endif
            envObj->listCommandThread.Notify ( "EnqueueCommand", true );
            
            LockReleaseVA ( envObj->listCommandThreadLock , "EnqueueCommand" );
        }


		sp ( environs::DeviceInfo ) BuildDeviceInfo ( Addr_ptr context, int notification )
		{
#ifdef CLI_CPP
			return BuildDeviceInfoCli ( ( unsigned char * ) context->ToPointer (), (notification & NOTIFY_MEDIATOR_SERVER) == NOTIFY_MEDIATOR_SERVER );
#else
			sp ( DeviceInfo ) device = sp ( DeviceInfo ) ( new DeviceInfo );
			if ( device != nill )
				memcpy ( device.get (), context, sizeof ( DeviceInfo ) );
			return device;
#endif
		}
        

		void DeviceList::OnDeviceListNotification ( int hInst, environs::ObserverNotifyContext OBJ_ptr ctx )
		{
			//CListLogArg ( "OnDeviceListNotification: [0x%X] [%s] [%s context]", ctx->destID, CPP_CLI ( resolveName ( ctx->notification ), CCharToString ( environs::API::ResolveNameN ( ctx->notification ) ) ), ctx->contextPtr ? "has" : "no" );

			bool valid = true;
            int type = 0;
            
			switch ( ctx->notification ) {
                case NOTIFY_MEDIATOR_DEVICE_REMOVED:
                case NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED:
                {
                    type = 0; break;
                }
                    
                case NOTIFY_MEDIATOR_DEVICE_CHANGED:
                case NOTIFY_MEDIATOR_DEVICE_ADDED:
                case NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED:
                case NOTIFY_MEDIATOR_SRV_DEVICE_ADDED:
                {
                    type = 1; break;
                }
                    
                case MSG_HANDSHAKE_MAIN_FAIL:
                {
                    type = 3; break;
                }
                    
                default:
                    valid = false; break;
            }
            
            if ( valid ) {
                EnvironsPtr api = EnvironsAPI ( hInst );
                if ( api != nill )
                {
                    ListCommandContextPtr context = new__obj ( ListCommandContext );
                    if ( context != nill )
                    {
                        if ( ctx->contextPtr != nill )
                            context->device = BuildDeviceInfo ( ctx->contextPtr, ctx->notification );
                        
                        context->destID         = ctx->destID;
						context->areaName       = ( ctx->areaName ? ctx->areaName : "" );
						context->appName        = ( ctx->appName ? ctx->appName : "" );
						context->sourceIdent    = ( int ) ctx->sourceIdent;
                        context->notification   = ctx->notification;
                        context->hEnvirons      = hInst;
                        context->type           = type;
                        context->envObj         = api;

                        EnqueueCommand ( context );
                    }
                }
            }
            CVerbVerb ( "OnDeviceListNotification: done" );
		}

        
		DeviceInstanceESP DeviceList::RemoveDevice ( c_const devList ( DeviceInstanceEP ) c_ref list, pthread_mutex_t_ptr lock, ListCommandContextPtr ctx )
		{
			CVerbVerb ( "RemoveDevice: list, lock, ctx" );

			DeviceInstanceESP device = nill;
            
			if ( list == nill )
				return nill;

			bool    dispatchToUI = ( ctx->envObj->isUIAdapter && !IsUIThread () );

			sp ( DeviceListUpdatePack )     updatePacks = nill;

			if ( dispatchToUI ) {
				updatePacks = BuildDeviceListUpdatePack ( list );
				if ( updatePacks == nill ) {
					CErr ( "RemoveDevice: Failed to create update queue for UI thread." );
					return nill;
				}
			}
            
            int pos = -1;

            if ( !LockAcquire ( lock, "RemoveDevice" ) )
                return nill;

            GetDevice ( list, nill, device, ctx->destID, &pos );

            if ( device == nill || pos < 0 ) {
                LockReleaseV ( lock, "RemoveDevice" );
                
                CVerbVerbArg1 ( "RemoveDevice: Device not found", "destID", "i", ctx->destID );
				return nill;
			}

            if ( !ctx->device ) //  ctx->notification == NOTIFY_MEDIATOR_DEVICE_REMOVED || ctx->notification == NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED ||
            {
                CVerbVerbArg1 ( "RemoveDevice: Disposing ", "destID", "i", ctx->destID );

                TRACE_DEVICE_INSTANCE ( device->gotRemoves++ );

				if ( dispatchToUI ) {
                    DeviceListRemoveAtQ ( updatePacks, pos );
                    
					DeviceListUpdateDispatchSync ( updatePacks );
                    
                    LockReleaseV ( lock, "RemoveDevice" );
				}
				else {
                    CListLogArg1 ( "RemoveDevice: Device vanished", "", "X", device->info_->deviceID );

                    TRACE_DEVICE_INSTANCE ( device->gotRemoves1++ );

                    ContainerRemoveAt ( list, pos );
                    
                    LockReleaseV ( lock, "RemoveDevice" );
				}

                if ( !device->disposed_ )
                    device->DisposeInstance ();

				CVerbVerb ( "RemoveDevice: list, lock, ctx disposed" );
            }
            else {
                LockReleaseV ( lock, "RemoveDevice" );

				CVerbVerbArg1 ( "RemoveDevice: Updating ", "destID", "i", ctx->destID );
				CListLogArg1 ( "RemoveDevice: Updating", "", "X", device->info_->deviceID );

				device->Update ( sp_get ( ctx->device ) );

				device = nill;
			}

            CVerbVerb ( "RemoveDevice: list, lock, ctx done" );
            return device;
        }
        
        
        bool DeviceList::RemoveDeviceNotifyEnqueue ( EnvironsPtr envObj, ListContext OBJ_ref ctx, c_const DeviceInstanceESP c_ref device )
        {
#ifdef CLI_CPP
            if ( envObj->isUIAdapter )
                return true; // Possible leak here?
#endif
            bool success = true;

			if ( LockAcquire ( c_Addr_of ( ctx.lock ), "RemoveDeviceNotifyEnqueue" ) )
			{
				if ( ctx.vanished == nill ) {
					ctx.vanished = sp_make ( NLayerVecTypeObj ( DeviceInstanceEP ) );

					success = ( ctx.vanished != nill );
				}

				if ( success )
					ContainerAppend ( ctx.vanished, device );

				LockReleaseV ( c_Addr_of ( ctx.lock ), "RemoveDeviceNotifyEnqueue" );
			}
            
            
            if ( success ) {
                NotifyListObservers ( envObj->hEnvirons, ctx.type, true );
            }
            else {
                // This shouldn't happen at all. However, if it happens, then we must dispose the device here and skip notifying client code
                
                device->NotifyObservers ( ENVIRONS_OBJECT_DISPOSED, false );
            }
            
            return success;
        }
        

        void DeviceList::RemoveDevice ( ListCommandContextPtr ctx )
        {
            CVerbVerbArg1 ( "RemoveDevice: ctx", "destID", "i", ctx->destID );
            
			/// Remove device from all lists

			DeviceInstanceESP device;
            
            EnvironsPtr envObj = ctx->envObj;

			if ( envObj->listNearby != nill )
            {
				device = RemoveDevice ( envObj->listNearby, Addr_of ( envObj->listNearbyLock ), ctx );
                
				if ( device != nill )
                    RemoveDeviceNotifyEnqueue ( envObj, envObj->contextNearby, device );
			}

            if ( envObj->listMediator != nill ) {
                device = RemoveDevice ( envObj->listMediator, Addr_of ( envObj->listMediatorLock ), ctx );
                
                if ( device != nill )
                    RemoveDeviceNotifyEnqueue ( envObj, envObj->contextMediator, device );
			}

            if ( envObj->listAll != nill ) {
                device = RemoveDevice ( envObj->listAll, Addr_of ( envObj->listAllLock ), ctx );
                
                if ( device != nill )
                    RemoveDeviceNotifyEnqueue ( envObj, envObj->contextAll, device );
            }

			AppEnvRemove ( envObj->hEnvirons, device )

            CVerbVerb ( "RemoveDevice: ctx done" );
		}


		void DeviceList::UpdateDevice ( ListCommandContextPtr ctx )
        {
            CVerbVerb ( "UpdateDevice" );

			environs::DeviceInfoPtr info = sp_get ( ctx->device );
            
            EnvironsPtr envObj = ctx->envObj;

			DeviceInstanceESP device;

			if ( envObj->listAll != nill ) 
			{
				device = nill;

                GetDevice ( envObj->listAll, Addr_of ( envObj->listAllLock ), device, ctx->destID, 0 );
				if ( device != nill )
                {
					if ( ctx->device != nill ) {
                        device->Update ( info );
                        CVerbArg1 ( "UpdateDevice:", "", "s", CPP_CLI ( device->toString().c_str(), device->ToString () ) );
					}
				}
				else {
					if ( info != nill ) {
						device = DeviceInstance::Create ( ctx->hEnvirons, info );

                        TRACE_DEVICE_INSTANCE ( device->gotInserts++ );

                        InsertDevice ( ctx->hEnvirons, envObj->listAll, Addr_of ( envObj->listAllLock ), device, envObj->contextAll );
					}
				}
			}

			if ( envObj->listNearby != nill && ( ( ctx->notification & NOTIFY_MEDIATOR_SERVER ) != NOTIFY_MEDIATOR_SERVER ) )
			{
				bool insert = false;

				if ( device == nill ) {
					if ( info != nill ) {
						device = nill;

						GetDevice ( envObj->listNearby, Addr_of ( envObj->listNearbyLock ), device, ctx->destID, 0 );
						if ( device != nill ) {
							device->Update ( info );
						}
						else {
							device = DeviceInstance::Create ( ctx->hEnvirons, info );
                            insert = true;

                            TRACE_DEVICE_INSTANCE ( device->gotInserts++ );
						}
					}
				}
				else insert = true;

				if ( insert && device != nill ) {
					InsertDevice ( ctx->hEnvirons, envObj->listNearby, Addr_of ( envObj->listNearbyLock ), device, envObj->contextNearby );
				}
			}

			if ( envObj->listMediator != nill && ( ( ctx->notification & NOTIFY_MEDIATOR_SERVER ) == NOTIFY_MEDIATOR_SERVER ) )
			{
				bool insert = false;

				if ( device == nill ) {
					if ( info != nill )
					{
						device = nill;

						GetDevice ( envObj->listMediator, Addr_of ( envObj->listMediatorLock ), device, ctx->destID, 0 );
						if ( device != nill )
						{
							device->Update ( info );
						}
						else {
							device = DeviceInstance::Create ( ctx->hEnvirons, info );
							insert = true;

                            TRACE_DEVICE_INSTANCE ( device->gotInserts++ );
						}
					}
				}
				else insert = true;

				if ( insert && device != nill ) {
					InsertDevice ( ctx->hEnvirons, envObj->listMediator, Addr_of ( envObj->listMediatorLock ), device, envObj->contextMediator );
				}
			}

			AppEnvUpdate ( envObj->hEnvirons, device )
		}


		bool DeviceList::DeviceInstanceUpdatesApply ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates )
		{
			bool changed = false;

			size_t size = vp_size ( updates );

			for ( size_t i = 0; i < size; ++i )
            {
				DeviceInstanceUpdateContextPtr upd = vp_at ( updates, i );
				if ( !upd )
					continue;

				if ( upd->device->Update ( upd->deviceInfo ) ) 
                    changed = true;
                //CVerbArg1 ( "DeviceInstanceUpdatesApply:", "", "s", upd->device->toString() );

                if ( upd->deviceSrc && sp_get ( upd->deviceSrc ) != sp_get ( upd->device ) )
                {
                    upd->deviceSrc->DisposeInstance ();
                    upd->deviceSrc->PlatformDispose ();
                }
				delete__obj ( upd );
			}
            
            ContainerClear ( updates );

			return changed;
        }


		void DeviceList::InsertDevice ( int hInst, c_const devList ( DeviceInstanceEP ) c_ref deviceList,

			pthread_mutex_t_ptr lock, DeviceInstanceReferenceESP deviceNew, ListContext OBJ_ref ctx )
        {
            CVerbVerb ( "InsertDevice" );
            
#ifndef NDEBUG
            if ( !deviceNew->info_->deviceID ) {
                CErr ( "InsertDevice: Invalid deviceID 0 !!!" );
            }
#endif
            bool    doNotify    = true;

			bool	isUIAdapter	= EnvironsAPI ( hInst )->isUIAdapter;

			bool    dispatchToUI = ( isUIAdapter && !IsUIThread () );

			NLayerListType ( DeviceInstanceUpdateContext ) updates = sp_make ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) );

            if ( updates == nill || deviceList == nill )
                goto Finish;
			
			if ( dispatchToUI )
			{
				sp ( DeviceListUpdatePack ) uiQueue = BuildDeviceListUpdatePack ( deviceList );
				if ( uiQueue == nill ) {
                    CErr ( "InsertDevice: Failed to create update queue for UI thread." );
                    goto Finish;
                }
				uiQueue->updates = sp_get ( updates );
                
                sp ( DeviceListQueueItem ) item = sp_make ( DeviceListQueueItem );
                if ( item == nill )
                    goto Finish;
                
                if ( !LockAcquire ( lock, "InsertDevice" ) )
                    goto Finish;
                
                item->cmd       = DEVICELIST_QUEUE_COMMAND_INSERT_CALL;
                item->device    = deviceNew;
                ContainerdAppend ( uiQueue->items, item );
                
                DeviceListUpdateDispatchSync ( uiQueue );
			}
            else {
                if ( !LockAcquire ( lock, "InsertDevice" ) )
                    goto Finish;

                InsertDeviceDo ( deviceList, deviceNew, sp_get ( updates ) );
            }

            if ( !LockRelease ( lock, "InsertDevice" ) )
                goto Finish;

			if ( vp_size ( updates ) > 0 ) {
                DeviceInstanceUpdatesApply ( sp_get ( updates ) );
                doNotify = false;
			}

#ifdef CLI_CPP
			if ( !isUIAdapter )
			{
#endif
                if ( doNotify ) {
					if ( LockAcquire ( c_Addr_of ( ctx.lock ), "InsertDevice" ) )
					{
						bool success = true;

						if ( ctx.appeared == nill ) {
							ctx.appeared = sp_make ( NLayerVecTypeObj ( DeviceInstanceEP ) );

							success = ( ctx.appeared != nill );
						}

						if ( success ) {
							ContainerAppend ( ctx.appeared, deviceNew );
						}

						LockRelease ( c_Addr_of ( ctx.lock ), "InsertDevice" );
                        
                        deviceNew->SetDeviceFlags ( DeviceFlagsInternal::PlatformReady, environs::Call::Wait, true );
                        
						//DeviceInstance::ParseAllFiles ( appeared );

						NotifyListObservers ( hInst, ctx.type, true );
					}
				}
#ifdef CLI_CPP
			}
#endif
            //CVerbArg1 ( "InsertDevice: Devicelist update thread done.", "", "s", deviceNew->toString() );
            return;

        Finish:
            deviceNew->DisposeInstance ();
            deviceNew->PlatformDispose ();
            
			CErr ( "InsertDevice: Devicelist update thread done." );
        }

		
		void DeviceInstanceUpdateAdd ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates, DeviceInstanceReferenceESP device, DeviceInstanceReferenceESP deviceSrc, environs::DeviceInfoPtr deviceInfo )
		{
			if ( !updates )
				return;

			DeviceInstanceUpdateContextPtr upd = new__obj ( DeviceInstanceUpdateContext );
			if ( upd == nill )
				return;

            TRACE_DEVICE_INSTANCE ( device->gotUpdates++ );

            upd->device     = device;
            upd->deviceSrc  = deviceSrc;
            upd->deviceInfo = deviceInfo;

			ContainerAppend ( updates, upd );
		}

        
        void DeviceList::InsertDeviceDo ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, DeviceInstanceReferenceESP deviceNew,
			NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates )
        {
            CVerbVerb ( "InsertDeviceDo" );
            
            int     listIndex   = 0;
            bool    itemHandled = false;

			int     listCount = ( int ) vp_size ( deviceList );
            
            if ( listCount <= 0 ) {
                CListLogArg1 ( "InsertDeviceDo: Device appeared (1)", "", "X", deviceNew->info_->deviceID );
                
                ContainerAppend ( deviceList, deviceNew );

                TRACE_DEVICE_INSTANCE ( deviceNew->gotInserts1++ );
                return;
            }
            
            while ( listIndex < listCount )
            {
				DeviceInstanceReferenceESP device = vp_at ( deviceList, listIndex );
                
                if ( deviceNew->info_->deviceID < device->info_->deviceID )
                {
                    CListLogArg1 ( "InsertDeviceDo: Device appeared (2)", "", "X", deviceNew->info_->deviceID );
                    
                    ContainerInsert ( deviceList, listIndex, deviceNew );

                    TRACE_DEVICE_INSTANCE ( deviceNew->gotInserts2++ );
                    itemHandled = true;
                    break;
                }
                
                if ( deviceNew->isSameAppArea )
                {
                    if ( deviceNew->info_->deviceID == device->info_->deviceID ) /// Device found
                    {
                        CListLogArg1 ( "InsertDeviceDo: Updating", "", "X", device->info_->deviceID );
                        
						DeviceInstanceUpdateAdd ( updates, device, deviceNew, deviceNew->info_ );
                        itemHandled = true;
                        break;
                    }
                    listIndex++;
                    continue;
                }
                else
                {
                    if ( device->LowerThanAppEnv ( deviceNew->info_ ) || deviceNew->info_->deviceID > device->info_->deviceID )
                    {
                        listIndex++;
                        continue;
                    }
                    CListLogArg1 ( "InsertDeviceDo: Device appeared (3)", "", "X", deviceNew->info_->deviceID );
                    
                    ContainerInsert ( deviceList, listIndex, deviceNew );

                    TRACE_DEVICE_INSTANCE ( deviceNew->gotInserts3++ );
                    itemHandled = true;
                    break;
                }
            }
            
            if ( !itemHandled ) {
                CListLogArg1 ( "InsertDeviceDo: Device appeared (4)", "", "X", deviceNew->info_->deviceID );
                
                ContainerAppend ( deviceList, deviceNew );

                TRACE_DEVICE_INSTANCE ( deviceNew->gotInserts4++ );
            }
            
            CVerb ( "InsertDeviceDo: Devicelist update thread done." );
        }
        

		void DeviceList::NotifyListObservers ( int	hInst, environs::DeviceClass_t listType, bool enqueue )
		{
			CVerbVerb ( "NotifyListObservers" );
            
			C_Only ( Instance * env = instances [ hInst ] );

			EnvironsPtr envObj = EnvironsAPI ( hInst );
			if ( envObj == nill )
                return;

			if ( enqueue && !CPP_CLI ( env->disposing, environs::API::GetDisposingN ( hInst ) ) )
			{
				ListCommandContextPtr ctx = new__obj ( ListCommandContext );
				if ( ctx != nill ) {
					ctx->envObj     = envObj;
					ctx->listType   = listType;
					ctx->type       = 2;
					ctx->hEnvirons  = hInst;

					EnqueueCommand ( ctx );
					return;
				}
				return;
			}

            ListContext OBJ_ptr listContext = nill;
            
            spv ( lib::IIListObserver * ) observerList;
            
            switch ( listType ) {
                case environs::DeviceClass::All :
                    observerList = envObj->listAllObservers;
                    listContext  = Addr_of ( envObj->contextAll );
                    break;
                    
                case environs::DeviceClass::Nearby :
                    observerList = envObj->listNearbyObservers;
                    listContext  = Addr_of ( envObj->contextNearby );
                    break;
                    
                default:
                    observerList = envObj->listMediatorObservers;
                    listContext  = Addr_of ( envObj->contextMediator );
                    break;
            }

#ifdef CLI_CPP
			if ( observerList == nill )
				return;
#endif            
			if ( !LockAcquire ( c_Addr_of ( listContext->lock ), "NotifyListObservers" ) )
				return;
            
            NLayerVecType ( DeviceInstanceEP ) vanished = listContext->vanished; listContext->vanished = nill;
            NLayerVecType ( DeviceInstanceEP ) appeared = listContext->appeared; listContext->appeared = nill;
            
            size_t sizeVanished = ( vanished != nill ? vp_size ( vanished ) : 0 );
            size_t sizeAppeared = ( appeared != nill ? vp_size ( appeared ) : 0 );

			if ( LockRelease ( c_Addr_of ( listContext->lock ), "NotifyListObservers" ) )
			{                
#ifndef CLI_CPP
                API::onEnvironsNotifier1 ( env, 0, NOTIFY_MEDIATOR_DEVICELISTS_CHANGED, SOURCE_NATIVE );
#endif
                CListLog ( "NotifyListObservers: Devicelist has changed." );
                
                size_t size = vp_size ( observerList );
                
                if ( size > 0 && ( sizeVanished > 0 || sizeAppeared > 0 ) )
                {
#ifdef CLI_CPP
#	ifndef CLI_NOUI
                    //System::Collections::Specialized::NotifyCollectionChangedEventArgs args ( System::Collections::Specialized::NotifyCollectionChangedAction::Add, appeared, vanished);
                    if ( appeared == nill )
                        appeared = gcnew List<DeviceInstanceEP ^> ();
                    if ( vanished == nill )
                        vanished = gcnew List<DeviceInstanceEP ^> ();
                    
                    System::Collections::Specialized::NotifyCollectionChangedEventArgs args ( System::Collections::Specialized::NotifyCollectionChangedAction::Replace, appeared, vanished );
#	endif
#else
                    environs::ArrayList * vanished4API = nill;
                    environs::ArrayList * appeared4API = nill;
#endif
                    vct ( CPP_CLI ( lib::IIListObserver, environs::ListObserver ) OBJ_ptr ) obss;
                    
                    LockAcquireVA ( envObj->listLock, "NotifyListObservers" );
                    
                    size = vp_size ( observerList );
                    
                    for ( size_t i = 0; i < size; ++i )
                    {
                        CPP_CLI ( lib::IIListObserver, environs::ListObserver ) OBJ_ptr obs = ( CPP_CLI ( lib::IIListObserver, environs::ListObserver ) OBJ_ptr ) vp_at ( observerList, i );
                        if ( obs != nill ) {
                            ContainerdAppend ( obss, obs );
                        }
                    }
                    
                    LockReleaseVA ( envObj->listLock, "NotifyListObservers" );
                    
                    size = vd_size ( obss );
                    
                    for ( size_t i = 0; i < size; ++i )
                    {
                        CPP_CLI ( lib::IIListObserver, environs::ListObserver ) OBJ_ptr obs = ( CPP_CLI ( lib::IIListObserver, environs::ListObserver ) OBJ_ptr ) vd_at ( obss, i );
                        
                        try {
                            CVerbVerbArg1 ( "NotifyListObservers: Notifying observer", "", "i", (int) i );
#ifdef CLI_CPP
#	ifndef CLI_NOUI
                            obs ( nill, %args );
#	else
                            obs ( vanished, appeared );
#	endif
#else
                            if ( obs->OnListChangedInternal_ )
                            {
                                obs->OnListChangedInternal ( vanished, appeared );
                            }
                            else {
                                if ( !obs->OnListChanged_ && !obs->OnListChangedInterface_ )
                                    continue;
                                
                                if ( vanished4API == nill && sizeVanished > 0 ) {
									vanished4API = ArrayList::CreateWithDevicesRetained ( vanished );
                                }
                                
                                if ( appeared4API == nill && sizeAppeared > 0 ) {
									appeared4API = ArrayList::CreateWithDevicesRetained ( appeared );
                                }
                                
                                if ( obs->OnListChangedInterface_ )
                                {
                                    obs->OnListChangedInterface ( ( environs::DeviceInstanceList *) vanished4API, ( environs::DeviceInstanceList *) appeared4API );
                                }
                                if ( obs->OnListChanged_ )
                                {
                                    // The base implementation MUST handle reference counting
                                    obs->OnListChangedBase ( vanished4API, appeared4API );
                                }
                            }
#endif
                        }
                        catch ( ... ) {
                            CErr ( "NotifyListObservers: Exception!" );
                        }
                    }
                    
                    C_Only ( if ( vanished4API != nill ) \
						vanished4API->Release (); )
                    
                    C_Only ( if ( appeared4API != nill ) \
						appeared4API->Release (); )
                }
            }
            
            if ( sizeVanished > 0 ) {
                for ( size_t i = 0; i < sizeVanished; ++i )
                {
                    c_const DeviceInstanceSP c_ref device = vp_at ( vanished, i );

                    if ( device != nill )
						device->PlatformDispose ();                    
                }
            }
		}

        
		/**
		* Devicelist update thread.
		*/
		void DeviceList::DeviceListUpdater ( EnvironsPtr api, int listType )
		{
			CListLogArg1 ( "DeviceListUpdater: Devicelist update thread started", "listType", "i", listType );

			int                             devicesCount	= 0;
			DeviceListItems					devices			= nill;
			DeviceHeader           *		devicesHeader	= nill;
			pthread_mutex_t_ptr				lock			= nill;
			bool                            doNotify		= false;
            int                             listCount       = 0;

			devList ( DeviceInstanceEP )	deviceListSP = nill;

			NLayerVecType ( DeviceInstanceEP ) appeared = sp_make ( NLayerVecTypeObj ( DeviceInstanceEP ) );
            NLayerVecType ( DeviceInstanceEP ) vanished = sp_make ( NLayerVecTypeObj ( DeviceInstanceEP ) );
            
            if ( appeared == nill || vanished == nill ) {
                CErrArg1 ( "DeviceListUpdater: Failed to create v/a lists", "Type", "i", listType );
                return;
            }

            int hEnvirons = api->hEnvirons;

			NLayerListType ( DeviceInstanceUpdateContext ) updates ( new__obj ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) ) );
			if ( updates == nill )
				return;

			switch ( listType )
			{
			case MEDIATOR_DEVICE_CLASS_ALL:
				deviceListSP    = api->listAll;
				lock			= Addr_of ( api->listAllLock );
				devicesHeader   = ( DeviceHeader * ) environs::API::GetDevicesN ( hEnvirons, MEDIATOR_DEVICE_CLASS_ALL );
				break;

			case MEDIATOR_DEVICE_CLASS_MEDIATOR:
				deviceListSP    = api->listMediator;
				lock			= Addr_of ( api->listMediatorLock );
				devicesHeader   = ( DeviceHeader * ) environs::API::GetDevicesN ( hEnvirons, MEDIATOR_DEVICE_CLASS_MEDIATOR );
				break;

			case MEDIATOR_DEVICE_CLASS_NEARBY:
				deviceListSP    = api->listNearby;
				lock			= Addr_of ( api->listNearbyLock );
				devicesHeader   = ( DeviceHeader * ) environs::API::GetDevicesN ( hEnvirons, MEDIATOR_DEVICE_CLASS_NEARBY );
				break;

			default:
				CErrArg1 ( "DeviceListUpdater: Unknown devicelist.", "Type", "i", listType );
				return;
			}

			if ( deviceListSP == nill )
			{
                CErrArg1 ( "DeviceListUpdater: Devicelist is missing.", "Type", "i", listType );
                goto Finish;
            }

			if ( devicesHeader ) {
				devices = DevicesToPlatform ( devicesHeader );
				if ( devices )
					devicesCount = DevicesPlatformCount ( devicesHeader, devices );
			}

			if ( api->isUIAdapter && !IsUIThread () ) {
				sp ( DeviceListUpdatePack ) uiQueue = BuildDeviceListUpdatePack ( deviceListSP );
				if ( uiQueue == nill ) {
					CErr ( "DeviceListUpdater: Failed to create update queue for UI thread." );
					goto Finish;
				}

				uiQueue->api		= api;
				uiQueue->devices	= devices;
				uiQueue->devicesCount = devicesCount;
				uiQueue->listType	= listType;
                uiQueue->updates	= sp_get ( updates );
                uiQueue->appeared   = appeared;
                uiQueue->vanished   = vanished;

                if ( !LockAcquire ( lock, "DeviceListUpdater" ) )
                    goto Finish;

				doNotify = DeviceListUpdateDispatchSync ( uiQueue );

			}
			else {
                if ( !LockAcquire ( lock, "DeviceListUpdater" ) )
                    goto Finish;

				doNotify = DeviceListUpdaterDo ( api, listType, sp_get ( deviceListSP ), devices, devicesCount, sp_get ( vanished ), sp_get ( appeared ), sp_get ( updates ) );
			}
            
            LockReleaseV ( lock, "DeviceListUpdater" );

			listCount = ( int ) vp_size ( vanished );

			for ( int i = 0; i < listCount; i++ ) {
				DeviceInstanceESP c_ref device = vp_at ( vanished, i );

				if ( device != nill )
					device->DisposeInstance ();
			}

            if ( vp_size ( updates ) > 0 ) {
                DeviceInstanceUpdatesApply ( sp_get ( updates ) );
			}
            
            if ( vp_size ( appeared ) > 0 ) {
                //	DeviceInstance::ParseAllFiles ( appeared );
                
                listCount = ( int ) vp_size ( appeared );
                
                for ( int i = 0; i < listCount; i++ ) {
                    DeviceInstanceESP c_ref device = vp_at ( appeared, i );
                    
                    if ( device != nill )
                        device->SetDeviceFlags ( DeviceFlagsInternal::PlatformReady, environs::Call::Wait, true );
                }
            }
            
			if ( doNotify )
			{
#ifdef CLI_CPP
				if ( !api->isUIAdapter )
				{
#endif
					api->AddToListContainer ( ( DeviceClass_t ) listType, vanished, appeared );

					NotifyListObservers ( hEnvirons, ( DeviceClass_t ) listType, true );
#ifdef CLI_CPP
				}
#endif
            }

        Finish:
            free_plt ( devicesHeader );
            
			CVerb ( "DeviceListUpdater: Devicelist update thread done." );
		}


		/**
		* Devicelist update thread.
		*/
		bool DeviceList::DeviceListUpdaterDo ( EnvironsPtr api, int listType, devListRef ( DeviceInstanceEP ) deviceList,
                                             DeviceListItems devices, int devicesCount,
                                             NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished,
                                             NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr appeared,
												NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates )
		{
			CListLogArg1 ( "DeviceListUpdaterDo: Started", "listType", "i", listType );
			
			bool    doNotify    = false;
			int     listCount;

            int     hEnvirons   = api->hEnvirons;

#ifdef USE_CLI_CUSTOM_OBSERVABLE_COLLECTION_CLASS
			EnvObservableCollection<DeviceInstanceEP ^> ^ notifDisabledList = nill;

			if ( deviceList->GetType () == EnvObservableCollection<DeviceInstanceEP ^>::typeid ) {
				notifDisabledList = ( EnvObservableCollection<DeviceInstanceEP ^> ^ ) deviceList;
				notifDisabledList->notificationEnabled = false;
			}
#endif
			listCount = ( int ) vp_size ( deviceList );

			if ( devices == nill )
			{
				if ( listCount > 0 )
				{
					CVerb ( "DeviceListUpdaterDo: Clearing deviceList." );
                    
					for ( int i = 0; i < listCount; i++ )
                    {
						DeviceInstanceReferenceESP device = vp_at ( deviceList, i );
                        
                        if ( device != nill ) {
                            TRACE_DEVICE_INSTANCE ( device->gotRemoves3++ );

                            ContainerAppend ( vanished, device );
						}
					}
					ContainerClear ( deviceList );
					doNotify = true;
				}
			}
			else
			{
				if ( listCount == 0 )
				{
					for ( int i = 0; i < devicesCount; i++ ) {
						DeviceInstanceESP created = DeviceInstance::Create ( hEnvirons, DevicesPlatformAt ( devices, i ) );
						if ( created != nill ) {
                            ContainerAppend ( deviceList, created ); ContainerAppend ( appeared, created );

                            TRACE_DEVICE_INSTANCE ( created->gotInserts++ );

                            CListLogArg1 ( "DeviceListUpdaterDo: Adding", "", "s", created-> CPP_CLI ( toString(), ToString() ) );
						}
					}
					doNotify = true;
				}
				else
				{
					CListLogArg1 ( "DeviceListUpdaterDo:", "devicesCount", "i", devicesCount );

					int listIndex = 0;

					for ( int i = 0; i < devicesCount; i++ )
					{
						bool						itemHandled = false;
						environs::DeviceInfoPtr     item        = DevicesPlatformAt ( devices, i );

#ifndef CLI_CPP
						CListLogArg ( "DeviceListUpdaterDo: [ %i ]  [ %8X / %i ] [ %s %s ]    name [ %s ]", i, item->deviceID, item->broadcastFound, item->areaName, item->appName, item->deviceName );
#endif                        
                        if ( !item->deviceID ) {
#ifndef NDEBUG
                            CErrArg1 ( "DeviceListUpdaterDo: Invalid deviceID 0 found at", "index", "i", i );
#endif
                            continue;
                        }
                        
						while ( listIndex < listCount )
						{
							DeviceInstanceReferenceESP device = vp_at ( deviceList, listIndex );

                            if ( item->deviceID < device->info_->deviceID )
							{
								/// Insert the new device
								DeviceInstanceESP created = DeviceInstance::Create ( hEnvirons, item );
								if ( created != nill ) {
									ContainerInsert ( deviceList, listIndex, created ); ContainerAppend ( appeared, created );

                                    TRACE_DEVICE_INSTANCE ( created->gotInserts++ );

									listCount = ( int ) vp_size ( deviceList ); listIndex++;									
									itemHandled = true; doNotify = true;                                    

									CListLogArg1 ( "DeviceListUpdaterDo: Device appeared (1)", "", "X", item->deviceID );
								}
								break;
							}

							if ( item->hasAppEnv == 0 )
							{
								if ( item->deviceID == device->info_->deviceID ) /// Device found
								{
									CListLogArg1 ( "DeviceListUpdaterDo: Updating", "", "X", item->deviceID );

									DeviceInstanceUpdateAdd ( updates, device, nill, item );

									listIndex++; itemHandled = true;
									break;
								}

								/// id of new item > id of current device in list and has not been handled, so the current device must be gone..
								CListLogArg1 ( "DeviceListUpdaterDo: Device vanished (1)", "", "X", device->info_->deviceID );

                                TRACE_DEVICE_INSTANCE ( device->gotRemoves41++ );

								ContainerAppend ( vanished, device );
								ContainerRemoveAt ( deviceList, listIndex ); 

								listCount = ( int ) vp_size ( deviceList ); doNotify = true;
                                
								/// Stop comparing if we reached the end of the list
								if ( listIndex >= listCount )
									break;
								else
									continue;
							}
							else
							{
								if ( device->LowerThanAppEnv ( item ) || item->deviceID > device->info_->deviceID ) /// We have found a device that is not available anymore
								{
									/// id of new item > id of current device in list and has not been handled, so the current device must be gone..
									CListLogArg1 ( "DeviceListUpdaterDo: Device vanished (2)", "", "X", item->deviceID );

                                    TRACE_DEVICE_INSTANCE ( device->gotRemoves42++ );

									ContainerAppend ( vanished, device );
									ContainerRemoveAt ( deviceList, listIndex );

									listCount = ( int ) vp_size ( deviceList );                                 
									doNotify = true; continue;
								}

								/// Add the new item to the list if we reached the end of the devicelist
								///
								DeviceInstanceESP created = DeviceInstance::Create ( hEnvirons, item );
								if ( created != nill ) {
									ContainerInsert ( deviceList, listIndex, created ); ContainerAppend ( appeared, created );
									
									listCount = ( int ) vp_size ( deviceList ); listIndex++;									
									doNotify = true; itemHandled = true;

									CListLogArg1 ( "DeviceListUpdaterDo: Device appeared (2)", "", "X", item->deviceID );
                                    
								}
								break;
							}
						}

						if ( !itemHandled && listIndex >= listCount )
						{
							/// Add the new item to the list if we reached the end of the devicelist
							///
							DeviceInstanceESP created = DeviceInstance::Create ( hEnvirons, item );
							if ( created != nill ) {
                                ContainerAppend ( deviceList, created ); ContainerAppend ( appeared, created );

                                TRACE_DEVICE_INSTANCE ( created->gotInserts++ );

								listCount = ( int ) vp_size ( deviceList ); listIndex++;
								doNotify = true;

								CListLogArg1 ( "DeviceListUpdaterDo: Device appeared (3)", "", "X", item->deviceID );                                
							}
						}
					}

					if ( devicesCount < listCount )
					{
						while ( listIndex < ( int ) vp_size ( deviceList ) )
						{
							DeviceInstanceReferenceESP device = vp_at ( deviceList, listIndex );

                            TRACE_DEVICE_INSTANCE ( device->gotRemoves5++ );

                            if ( device != nill ) {
                                ContainerAppend ( vanished, device );
							}   
							ContainerRemoveAt ( deviceList, listIndex );

							CListLogArg1 ( "DeviceListUpdaterDo: Device vanished (3)", "", "X", device->info_->deviceID );
							doNotify = true;
						}
					}
				}
			}

#ifdef USE_CLI_CUSTOM_OBSERVABLE_COLLECTION_CLASS
			if ( notifDisabledList != nill )
				notifDisabledList->notificationEnabled = true;
#endif
			TakeOverToOtherLists ( api, listType, vanished );
            
			CVerb ( "DeviceListUpdaterDo: Devicelist update thread done." );
			return doNotify;
		}
        

		void DeviceList::TakeOverToOtherLists ( EnvironsPtr api, int listType, NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished  )
		{
			if ( listType == MEDIATOR_DEVICE_CLASS_ALL )
			{
				if ( api->listNearby != nill )
				{
					TakeOverToList ( api, api->listNearby, false, vanished );
				}
				if ( api->listMediator != nill )
				{
					TakeOverToList ( api, api->listMediator, true, vanished );
				}
			}
		}
        

		void DeviceList::TakeOverToList ( EnvironsPtr api, c_const devList ( DeviceInstanceEP ) c_ref list, bool getMediator,
			NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished )
        {
            CVerbVerb ( "TakeOverToList" );

			LockAcquireVA ( ( getMediator ? api->listMediatorLock : api->listNearbyLock ), "TakeOverToList" );

#ifdef USE_CLI_CUSTOM_OBSERVABLE_COLLECTION_CLASS
			EnvObservableCollection<DeviceInstanceEP ^> ^ notifDisabledList = nill;

			if ( list->GetType () == EnvObservableCollection<DeviceInstanceEP ^>::typeid ) {
				notifDisabledList = ( EnvObservableCollection<DeviceInstanceEP ^> ^ ) list;
				notifDisabledList->notificationEnabled = false;
			}
#endif
			int listAllCount	= ( int ) vp_size ( api->listAll );
			int listCount		= ( int ) vp_size ( list );

			int listIndex		= 0;
			int listAllIndex	= 0;

			while ( listAllIndex < listAllCount && listIndex < listCount )
			{
				DeviceInstanceESP c_ref itemAll = vp_at ( api->listAll, listAllIndex );
				DeviceInstanceESP c_ref item = vp_at ( list, listIndex );

				// Check if we have the same item
				if ( item == itemAll ) {
					listAllIndex++; listIndex++;
					continue;
				}

				if ( itemAll->info_->broadcastFound == ( getMediator ? DEVICEINFO_DEVICE_BROADCAST : DEVICEINFO_DEVICE_MEDIATOR ) ) {
					listAllIndex++;
					continue;
				}

                if ( item->LowerThanAppEnv ( itemAll->info_ ) && item->info_->deviceID < itemAll->info_->deviceID )
                {
                    TRACE_DEVICE_INSTANCE ( item->gotRemoves5++ );

					ContainerAppend ( vanished, item );
					ContainerRemoveAt ( list, listIndex );
					listCount--;
					continue;
				}

				ContainerInsert ( list, listIndex, itemAll );
				listIndex++;
				listCount++;
				listAllIndex++;

                TRACE_DEVICE_INSTANCE ( itemAll->gotInserts++ );
			}

			while ( listAllIndex < listAllCount )
			{
				DeviceInstanceESP c_ref itemAll = vp_at ( api->listAll, listAllIndex );
				listAllIndex++;

				if ( itemAll->info_->broadcastFound == ( getMediator ? DEVICEINFO_DEVICE_BROADCAST : DEVICEINFO_DEVICE_MEDIATOR ) ) {
					continue;
				}

				ContainerInsert ( list, listIndex, itemAll );
				listIndex++;

                TRACE_DEVICE_INSTANCE ( itemAll->gotInserts++ );
			}

#ifdef USE_CLI_CUSTOM_OBSERVABLE_COLLECTION_CLASS
			if ( notifDisabledList != nill )
				notifDisabledList->notificationEnabled = true;
#endif
			LockReleaseVA ( ( getMediator ? api->listMediatorLock : api->listNearbyLock ), "TakeOverToList" );
		}


		void DeviceList::UpdateConnectProgress ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, OBJIDType objID, int progress )
        {
            CVerbVerb ( "UpdateConnectProgress" );
            
            LockAcquireV ( lock, "UpdateConnectProgress" );
            
			size_t listCount = vp_size ( list );

			for ( size_t i = 0; i < listCount; i++ )
			{
				DeviceInstanceESP c_ref device = vp_at ( list, i );

				if ( device != nill && ( device->info_->objID == objID || device->objIDPrevious == objID ) )
				{
					device->SetProgress ( progress );
					break;
				}
            }
            
            LockReleaseV ( lock, "UpdateConnectProgress" );
		}
        
        
		void DeviceList::UpdateMessage ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, environs::ObserverMessageContext OBJ_ptr ctx )
        {
            CVerb ( "UpdateMessage" );
            
            LockAcquireV ( lock, "UpdateMessage" );
            
            size_t size = vp_size ( list );

			size_t i = 0;
			for ( ; i < size; i++ )
			{
				DeviceInstanceESP c_ref device = vp_at ( list, i );
				if ( device == nill )
					continue;

                if ( ctx->connection == 'u' ) {
#ifndef CLI_CPP
                    CVerbVerbArg1 ( "UpdateMessage: Comparing ", "device", "s", device->toString() );
                    CVerbVerbArg1 ( "UpdateMessage: With      ", "deviceID", "X", ctx->destID );
                    CVerbVerbArg2 ( "UpdateMessage: ", "appName", "s", ctx->appName, "areaName", "s", ctx->areaName );
#endif
					if ( !device->EqualsID ( ( int ) ctx->destID, ctx->areaName, ctx->appName ) )
						continue;
				}
                else {
#ifndef CLI_CPP
                    CVerbVerbArg1 ( "UpdateMessage: Comparing ", "device", "s", device->toString() );
                    
                    CVerbVerbArg1 ( "UpdateMessage: Comparing ", "c objID", "i", ctx->destID );
#endif

					if ( device->info_->objID != ctx->destID && device->objIDPrevious != ctx->destID )
						continue;
				}

				device->AddMessage ( ctx->message, ctx->length, false, ctx->connection );
				break;
			}
            
#ifndef NDEBUG
			if ( i >= size ) {
				CErrsArg2 ( 2, "UpdateMessage: Device not found", "c", "c", ctx->connection, "id", "X", ctx->destID );
				CVerbsArg1 ( 4, "UpdateMessage: Device not found", "message", "s", ctx->message );
			}
#endif
            LockReleaseV ( lock, "UpdateMessage" );
        }


		void DeviceList::UpdateData ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, environs::ObserverDataContext OBJ_ptr ctx )
        {
            CVerbVerb ( "UpdateData" );
            
            LockAcquireV ( lock, "UpdateData" );
            
			size_t size = vp_size ( list );

			for ( size_t i = 0; i < size; i++ )
			{
				DeviceInstanceESP c_ref device = vp_at ( list, i );

				if ( device != nill && ( device->info_->objID == ctx->objID || device->objIDPrevious == ctx->objID ) )
				{
					device->AddFile ( 0, ctx->fileID, ctx->descriptor, nill, ctx->size, false );
					break;
				}
            }
            
            LockReleaseV ( lock, "UpdateData" );
		}


		void DeviceList::UpdateUdpData ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, UdpDataContext OBJ_ptr udpData )
        {
            CVerbVerb ( "UpdateUdpData" );
            
            if ( udpData == nill )
                return;
            
            OBJIDType objID = udpData->objID;
            
            LockAcquireV ( lock, "UpdateUdpData" );
            
			size_t size = vp_size ( list );

			for ( size_t i = 0; i < size; i++ )
			{
				DeviceInstanceESP c_ref device = vp_at ( list, i );

				if ( device != nill && ( device->info_->objID == objID || device->objIDPrevious == objID ) )
				{
					if ( udpData->sensorFrame != nill )
					{
						udpData->sensorFrame->device = sp_get ( device );

						device->NotifySensorObservers ( udpData->sensorFrame );
					}
					else {
						device->NotifyUdpData ( udpData );
					}
					break;
				}
            }
            
            LockReleaseV ( lock, "UpdateUdpData" );
		}

	}
}







