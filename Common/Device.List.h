/**
 * DeviceLists
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_H

#include "Environs.Observer.CLI.h"
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Notify.Context.h"
#include "Environs.Msg.Types.h"

#ifndef CLI_CPP
#	include "Interfaces/IDevice.List.h"
#	include "Queue.List.h"
#endif

/**
 *	DeviceLists
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{

	namespace lib
	{
		CLASS Environs;

		CLASS DeviceInstance;

#ifndef DeviceInstancePtr
#	define	DeviceInstancePtr	 DeviceInstance OBJ_ptr
#endif
        CLASS ListCommandContext;
        
        CLASS ListContext;
        
#ifndef EnvironsPtr
#	define	EnvironsPtr Environs OBJ_ptr
#endif
        
#ifndef ListCommandContextPtr
#	define	ListCommandContextPtr ListCommandContext OBJ_ptr
#endif        

        STRUCT UdpDataContext;
        
		PUBLIC_CLASS DeviceInstanceUpdateContext
		{
		public:
            DeviceInstanceESP device;
            DeviceInstanceESP deviceSrc;

			environs::DeviceInfo OBJ_ptr deviceInfo;
        };
        
#ifndef DeviceInstanceUpdateContextPtr
#	define	DeviceInstanceUpdateContextPtr DeviceInstanceUpdateContext OBJ_ptr
#endif


		PUBLIC_CLASS DeviceListQueueItem
		{
		public:
			int     cmd;
			int     pos;

			DeviceInstanceESP device;
		};

		PUBLIC_CLASS DeviceListUpdatePack
		{
		public:
			DeviceListUpdatePack ( ) : lock ( nill ), updates ( nill ), api ( nill ), listType ( 0 ), devices ( nill ), devicesCount ( 0 ) 
			{ }
            
			pthread_mutex_t_ptr											lock;
            
			devList ( DeviceInstanceEP )								deviceList;
			NLayerVecTypeObj ( DeviceListQueueItem )                    items;

			NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr 	updates;

			NLayerVecType ( DeviceInstanceEP )		vanished;
			NLayerVecType ( DeviceInstanceEP )		appeared;

			EnvironsPtr			api;
			int					listType;
			DeviceListItems		devices;
			int					devicesCount;
        };


		PUBLIC_CLASS DeviceList DERIVE_c_only ( environs::DeviceList ) DERIVE_DISPOSABLE
		{
        
            MAKE_FRIEND_CLASS ( Environs );
            MAKE_FRIEND_CLASS ( DeviceListProxy );

        public:
			ENVIRONS_LIB_API DeviceList ();
			ENVIRONS_LIB_API ~DeviceList ();
        
            /**
             * Dispose device list.
             * This method is intended to be used by client code to enforce disposal.
             */
            void					DisposeList ();
        
			ENVIRONS_LIB_API bool	disposed ();

            ENVIRONS_LIB_API void	SetListType ( environs::DeviceClass_t MEDIATOR_DEVICE_CLASS_ );
        
            ENVIRONS_LIB_API void	SetIsUIAdapter ( bool enable );

            /**
             * Enable caching of the list returned by GetDevices() and update on call of GetDevices() for single threaded usagge.
             *
             * @param enable        true = enable, false = disable (default).
             * Note for C++/Obj-C API: The cached list returned by GetDevices() is intended for single thread applications.
             *          A call to GetDevices() while the cached list is still held by a thread is not allowed and might end up in invalid memory access.
         */
            ENVIRONS_LIB_API void   SetEnableListCache ( bool enable );

			DeviceInstanceESP		GetItem ( int position );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetItemRetained ( int position );
#endif

			ENVIRONS_LIB_API int	GetCount ();

			ENVIRONS_LIB_API void	AddObserver ( environs::ListObserverPtr observer );
			ENVIRONS_LIB_API void	RemoveObserver ( environs::ListObserverPtr observer );

			DeviceInstanceESP		RefreshItem ( DeviceInstanceEPtr source, DeviceObserverPtr observer );

			/**
			* Lock access to the devicelist returned by GetDevicesSource methods.
			*
			* @return success
			*/
			bool Lock ();

			/**
			* Unlock access to the devicelist returned by GetDevicesSource methods.
			*
			* @return success
			*/
			bool Unlock ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstancePtr RefreshItemRetained ( environs::DeviceInstancePtr source, environs::DeviceObserverPtr observer );
#endif
			/**
			* Get a copy of the collection that holds the devices according to the specified listtype.
			*
			* @return Collection with DeviceInstance objects
			*/
			devList ( DeviceInstanceEP ) GetDevices ();

			/**
			* Get the collection that holds the devices according to the specified listtype. 
			* This list is updated dynamically by Environs (even when user code access the list).
			*
			* @return Collection with DeviceInstance objects
			*/
			c_const devList ( DeviceInstanceEP ) c_ref GetDevicesSource ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesRetained ();
#endif


			/**
			 * Query the number of the devices according to the specified listtype within the environment.
			 *
			 * @return numberOfDevices
			 */
			ENVIRONS_LIB_API int GetDevicesCount ();


			/**
			* Query a DeviceInstance object from the devices according to the specified listtype within the environment.
			*
			* @param deviceID      The device id of the target device.
			* @param areaName      Area name of the application environment
			* @param appName		Application name of the application environment
			* @return DeviceInstance-object
			*/
			DeviceInstanceESP GetDevice ( int deviceID, CString_ptr areaName, CString_ptr appName, int * pos );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceRetained ( int deviceID, const char * areaName, const char * appName, int * pos );
#endif


			/**
			 * Query a DeviceInstance object from the devices according to the specified listtype devices within the environment.
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			DeviceInstanceESP GetDevice ( OBJIDType objID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceRetained ( OBJIDType objID );
#endif


			/**
			* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator).
			*
			* @param nativeID      The device id of the target device.
			* @return DeviceInstance-object
			*/
			DeviceInstanceESP GetDeviceAll ( OBJIDType objID );


			/**
			 * Query a DeviceInstance object that best match the deviceID only.
			 * Usually the one that is in the same app environment is picked up.
			 * If there is no matching in the app environment,
			 * then the areas are searched for a matchint deviceID.
			 *
			 * @param deviceID      The portalID that identifies an active portal.
			 * @return DeviceInstance-object
			 */
			DeviceInstanceESP GetDeviceBestMatch ( int deviceID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceBestMatchRetained ( int deviceID );
#endif


			/**
			 * Query a DeviceInstance object that best match the deviceID only from native layer.
			 * Usually the one that is in the same app environment is picked up.
			 * If there is no matching in the app environment,
			 * then the areas are searched for a matchint deviceID.
			 *
			 * @param deviceID      The portalID that identifies an active portal.
			 * @return DeviceInstance-object
			 */
			DeviceInstanceESP GetDeviceBestMatchNative ( int deviceID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceBestMatchNativeRetained ( int deviceID );
#endif


			/**
			 * Get a copy of the collection that holds the nearby devices.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			devList ( DeviceInstanceEP ) GetDevicesNearby ();

			/**
			* Get the collection that holds the nearby devices.
			* This list is updated dynamically by Environs (even when user code access the list).
			*
			* @return Collection with DeviceInstance objects
			*/
			c_const devList ( DeviceInstanceEP ) c_ref GetDevicesNearbySource ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesNearbyRetained ();
#endif


			/**
			 * Query the number of nearby (broadcast visible) devices within the environment.
			 *
			 * @return numberOfDevices
			 */
			ENVIRONS_LIB_API int GetDevicesNearbyCount ();


			/**
			 * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			DeviceInstanceESP GetDeviceNearby ( OBJIDType objID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceNearbyRetained ( OBJIDType objID );
#endif


			/**
			 * Release the ArrayList that holds the nearby devices.
			 */
			ENVIRONS_LIB_API void ReleaseDevicesNearby ();


			/**
			 * Get a copy of the collection that holds the Mediator server devices.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			devList ( DeviceInstanceEP ) GetDevicesFromMediator ();

			/**
			* Get the collection that holds the Mediator server devices.
			* This list is updated dynamically by Environs (even when user code access the list).
			*
			* @return Collection with DeviceInstance objects
			*/
			c_const devList ( DeviceInstanceEP ) c_ref GetDevicesFromMediatorSource ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesFromMediatorRetained ();
#endif


			/**
			 * Query a DeviceInstance object of Mediator managed devices within the environment.
			 *
			 * @param objID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			DeviceInstanceESP GetDeviceFromMediator ( OBJIDType objID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceFromMediatorRetained ( OBJIDType objID );
#endif


			/**
			 * Query the number of Mediator managed devices within the environment.
			 *
			 * @return numberOfDevices (or -1 for error)
			 */
			ENVIRONS_LIB_API int GetDevicesFromMediatorCount ();

			/**
			 * Release the ArrayList that holds the Mediator server devices.
			 */
			ENVIRONS_LIB_API void ReleaseDevicesMediator ();

			/**
			 * Reload all device lists. Applications may call this if they manually stopped and started Environs again.
			 * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
			 */
			ENVIRONS_LIB_API void ReloadLists ();

			/**
			* Reload the device list.
			*/
			ENVIRONS_LIB_API void Reload ();


			/**
			* Release ownership on this interface and mark it disposable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		INTERNAL:

			int                             hEnvirons;
            bool                            isUIAdapter;

#ifndef CLI_CPP
			Instance           *			env;
            environs::ArrayList *           listCached;
#endif
			bool                            listCacheEnabled;

			environs::lib::EnvironsPtr		envObj;

			bool                            disposed_;
			environs::DeviceClass_t         listType;

			pthread_mutex_t_ptr				listDevicesLock;

			devList ( DeviceInstanceEP )	listDevices;

#ifdef CLI_CPP
			spv ( lib::ListObserverPtr )	listDevicesObservers;
#else
			spv ( lib::IIListObserver * )	listDevicesObservers;
#endif
			void                            PlatformDispose ();

			void		GetItem ( DeviceInstanceESP OBJ_ref device, int position );

			c_const devList ( DeviceInstanceEP ) c_ref GetDevices ( environs::DeviceClass_t type );

			static void GetDevice ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, int deviceID, CString_ptr areaName, CString_ptr appName, int * pos );

			static void GetDevice ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, OBJIDType objID, int * pos );

			static void GetDeviceByNativeID ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, int nativeID );

			void GetDevice ( DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID );

			void GetDeviceAll ( DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID );

			static void GetDeviceSeeker ( c_const devList ( DeviceInstanceEP ) c_ref list, pthread_mutex_t_ptr lock, DeviceInstanceESP OBJ_ref device, OBJIDType objOrDeviceID, bool isObjID );

			void GetDeviceBestMatchNative ( DeviceInstanceESP OBJ_ref device, int deviceID );

			/**
			 * Release the ArrayList that holds the available devices.
			 */
			void ReleaseDevices ();

			void DisposeLists ();

			static void DisposeList ( bool isUIAdapter, c_const devList ( DeviceInstanceEP ) list, pthread_mutex_t OBJ_ptr lock );

			static void DisposeListDo ( c_const devList ( DeviceInstanceEP ) c_ref list );

			static void DeviceListUpdater ( environs::lib::EnvironsPtr api, int listType );

			static bool DeviceListUpdaterDo ( EnvironsPtr api, int listType, devListRef ( DeviceInstanceEP ) deviceList,
				DeviceListItems devices, int devicesCount,
				NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished, NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr appeared,
				NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );

			static bool DeviceListUpdateDispatchSync ( c_const sp ( DeviceListUpdatePack ) c_ref updatePacks );

			static bool DeviceListUpdateDataSourceSync ( c_const sp ( DeviceListUpdatePack ) c_ref updatePacks );

			static void TakeOverToOtherLists ( EnvironsPtr api, int listType, NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished );

			static void TakeOverToList ( environs::lib::EnvironsPtr api, c_const devList ( DeviceInstanceEP ) c_ref list, bool getMediator,
				NLayerVecTypeObj ( DeviceInstanceEP ) OBJ_ptr vanished );

			static void c_OBJ_ptr CommandThread ( pthread_param_t pack );

			static bool DeviceInstanceUpdatesApply ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );


            static void OnDeviceListNotification ( int hInst, environs::ObserverNotifyContext OBJ_ptr ctx );
        
            static void RemoveDevice ( ListCommandContextPtr pack );

			static DeviceInstanceESP RemoveDevice ( c_const devList ( DeviceInstanceEP ) c_ref list, pthread_mutex_t_ptr lock, ListCommandContextPtr pack );
        
            static bool RemoveDeviceNotifyEnqueue ( EnvironsPtr envObj, ListContext OBJ_ref ctx, DeviceInstanceReferenceESP device );

            static void UpdateDevice ( ListCommandContextPtr pack );
        
            static void InsertDevice ( int hInst, c_const devList ( DeviceInstanceEP ) c_ref deviceList, pthread_mutex_t_ptr listLock,
				DeviceInstanceReferenceESP deviceNew, ListContext OBJ_ref ctx );
        
            static void InsertDeviceDo ( c_const devList ( DeviceInstanceEP ) c_ref deviceList, DeviceInstanceReferenceESP deviceNew,
				NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );

			static void EnqueueCommand ( ListCommandContextPtr ctx );

			static void NotifyListObservers ( int hInst, environs::DeviceClass_t listType, bool enqueue );

			
			static void UpdateConnectProgress ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, OBJIDType objID, int progress );

			static void UpdateMessage ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, environs::ObserverMessageContext OBJ_ptr ctx );

			static void UpdateData ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, environs::ObserverDataContext OBJ_ptr ctx );

            static void UpdateUdpData ( pthread_mutex_t_ptr lock, c_const devList ( DeviceInstanceEP ) c_ref list, UdpDataContext OBJ_ptr udpData );

		};
	}
}


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_H


