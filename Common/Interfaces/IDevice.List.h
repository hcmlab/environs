/**
 * DeviceLists Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H

#include "Interfaces/IArray.List.h"
#include "Interfaces/IDevice.Instance.List.h"


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
	class ArrayList;

	class ListObserver;
#ifndef ListObserverPtr
#	define	ListObserverPtr	 ListObserver OBJ_ptr
#endif

	namespace lib
	{
		class IIListObserver;
		class IIDeviceObserver;
	}


	class DeviceList : public lib::IEnvironsDispose
	{
	public:
		DeviceList () { };
		virtual ~DeviceList () { };


		/**
		* Release ownership on this interface and mark it disposable.
		* Release must be called once for each Interface that the Environs framework returns to client code.
		* Environs will dispose the underlying object if no more ownership is hold by anyone.
		*
		*/
		virtual void                    Release () = 0;

		virtual bool					disposed () = 0;

        virtual void					SetListType ( environs::DeviceClass_t MEDIATOR_DEVICE_CLASS_ ) = 0;
        
        virtual void					SetIsUIAdapter ( bool enable ) = 0;
        
        /**
         * Enable caching of the list returned by GetDevices() and update on call of GetDevices() for single threaded usagge.
         *
         * @param enable        true = enable, false = disable (default).
         * Note for C++/Obj-C API: The cached list returned by GetDevices() is intended for single thread applications.
         *          A call to GetDevices() while the cached list is still held by a thread is not allowed and might end up in invalid memory access.
         */
        virtual void					SetEnableListCache ( bool enable ) = 0;

		virtual void					AddObserver ( ListObserver * observer ) = 0;
		virtual void					RemoveObserver ( ListObserver * observer ) = 0;


		sp ( DeviceInstance )			GetItem ( int pos )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetItemRetained ( pos ) );
		}

        
        virtual void					DisposeList () = 0;
        
		virtual int						GetCount () = 0;

		virtual DeviceInstance *        GetItemRetained ( int pos ) = 0;

		virtual DeviceInstance *        RefreshItemRetained ( DeviceInstance * source, DeviceObserver * observer ) = 0;

		/**
		* Get a collection that holds all available devices. This list ist updated dynamically by Environs.
		*
		* @return ArrayList with DeviceInstance objects
		*/
		sp ( DeviceInstanceList )       GetDevices ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstanceList, GetDevicesRetained () );
		}

		virtual ArrayList *             GetDevicesRetained () = 0;


		/**
		* Query the number of all available devices within the environment (including those of the Mediator)
		*
		* @return numberOfDevices
		*/
		virtual int GetDevicesCount () = 0;


		/**
		* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
		*
		* @param deviceID      The device id of the target device.
		* @param areaName      Area name of the application environment
		* @param appName		Application name of the application environment
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )		GetDevice ( int deviceID, const char * areaName, const char * appName, int * pos )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceRetained ( deviceID, areaName, appName, pos ) );
		}

		virtual DeviceInstance *	GetDeviceRetained ( int deviceID, const char * areaName, const char * appName, int * pos ) = 0;


		/**
		* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
		*
		* @param nativeID      The device id of the target device.
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )  GetDevice ( OBJIDType objID )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceRetained ( objID ) );
		}

		virtual DeviceInstance * GetDeviceRetained ( OBJIDType objID ) = 0;

		/**
		* Query a DeviceInstance object that best match the deviceID only.
		* Usually the one that is in the same app environment is picked up.
		* If there is no matching in the app environment,
		* then the areas are searched for a matchint deviceID.
		*
		* @param deviceID      The portalID that identifies an active portal.
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )  GetDeviceBestMatch ( int deviceID )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceBestMatchRetained ( deviceID ) );
		}

		virtual DeviceInstance * GetDeviceBestMatchRetained ( int deviceID ) = 0;

		/**
		* Query a DeviceInstance object that best match the deviceID only from native layer.
		* Usually the one that is in the same app environment is picked up.
		* If there is no matching in the app environment,
		* then the areas are searched for a matchint deviceID.
		*
		* @param deviceID      The portalID that identifies an active portal.
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )  GetDeviceBestMatchNative ( int deviceID )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceBestMatchNativeRetained ( deviceID ) );
		}

		virtual DeviceInstance * GetDeviceBestMatchNativeRetained ( int deviceID ) = 0;


		/**
		* Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
		*
		* @return ArrayList with DeviceInstance objects
		*/
		sp ( DeviceInstanceList )  GetDevicesNearby ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstanceList, GetDevicesNearbyRetained () );
		}

		virtual ArrayList * GetDevicesNearbyRetained () = 0;


		/**
		* Query the number of nearby (broadcast visible) devices within the environment.
		*
		* @return numberOfDevices
		*/
		virtual int GetDevicesNearbyCount () = 0;

		/**
		* Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
		*
		* @param nativeID      The device id of the target device.
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )  GetDeviceNearby ( OBJIDType objID )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceNearbyRetained ( objID ) );
		}

		virtual DeviceInstance * GetDeviceNearbyRetained ( OBJIDType objID ) = 0;

		/**
		* Release the ArrayList that holds the nearby devices.
		*/
		virtual void ReleaseDevicesNearby () = 0;

		/**
		* Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
		*
		* @return ArrayList with DeviceInstance objects
		*/
		sp ( DeviceInstanceList )  GetDevicesFromMediator ()
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstanceList, GetDevicesFromMediatorRetained () );
		}

		virtual ArrayList * GetDevicesFromMediatorRetained () = 0;


		/**
		* Query a DeviceInstance object of Mediator managed devices within the environment.
		*
		* @param nativeID      The device id of the target device.
		* @return DeviceInstance-object
		*/
		sp ( DeviceInstance )  GetDeviceFromMediator ( OBJIDType objID )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceInstance, GetDeviceFromMediatorRetained ( objID ) );
		}

		virtual DeviceInstance * GetDeviceFromMediatorRetained ( OBJIDType objID ) = 0;

		/**
		* Query the number of Mediator managed devices within the environment.
		*
		* @return numberOfDevices (or -1 for error)
		*/
		virtual int GetDevicesFromMediatorCount () = 0;

		/**
		* Release the ArrayList that holds the Mediator server devices.
		*/
		virtual void ReleaseDevicesMediator () = 0;

		/**
		* Refresh all device lists. Applications may call this if they manually stopped and started Environs again.
		* Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
		*/
		virtual void ReloadLists () = 0;

		/**
		* Reload the device list.
		*/
		virtual void Reload () = 0;
	};
}


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H

