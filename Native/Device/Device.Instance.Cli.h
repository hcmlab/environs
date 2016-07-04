/**
 * Device Instance CLI Objects
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEISNTANCE_CLI_H
#define INCLUDE_HCM_ENVIRONS_DEVICEISNTANCE_CLI_H

#ifdef CLI_CPP

#include "Device.Instance.h"


/**
 *	Device Instance CLI Objects
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	public ref class DeviceInstance : public environs::lib::DeviceInstance

#ifndef CLI_NOUI
		, System::ComponentModel::INotifyPropertyChanged
#endif

	{
	public:
		DeviceInstance ();
		~DeviceInstance ();

		virtual event System::ComponentModel::PropertyChangedEventHandler ^ PropertyChanged;

		virtual void OnPropertyChanged ( String ^ name, bool ignoreDefaultSetting ) override;

		void OnPropertyChangedDo ( String ^ name );

		/**
		* Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
		*
		* @return Collection with objects of type MessageInstance
		*/
		NLayerVecType ( environs::MessageInstance ) GetMessages ();

		/**
		* Get a list with all messages that this device has received (and sent) from the storage.
		*
		* @return Collection with objects of type MessageInstance
		*/
		NLayerVecType(environs::MessageInstance) GetMessagesInStorage();

		/**
		* Get a dictionary with all files that this device instance has received since the Device instance has appeared.
		*
		* @return Collection with objects of type FileInstance with the fileID as the key.
		*/
		NLayerMapType ( int, environs::FileInstance ) GetFiles ();

		/**
		* Get a dictionary with all files that this device instance has received from the storage.
		*
		* @return Collection with objects of type FileInstance with the fileID as the key.
		*/
		NLayerMapType(int, environs::FileInstance) GetFilesInStorage();
        
        /** Application defined context 0 for arbitrary use. */
		property int appContext0 {
			int get () { return appContext0_; }
			void set ( int value ) {
				appContext0_ = value;
				NotifyObservers ( DEVICE_INFO_ATTR_APP_CONTEXT, true );
				DeviceInstancePropertyNotify ( "appContext0", false );
			}  };
        
        /** Application defined context 1 for arbitrary use. */
		property Object ^ appContext1 {
			Object ^ get () { return appContext1_; }
			void set ( Object ^ value )
			{
				appContext1_ = value;
				NotifyObservers ( DEVICE_INFO_ATTR_APP_CONTEXT, true );
				DeviceInstancePropertyNotify ( "appContext1", false );
			}  };
        
        /** Application defined context 2 for arbitrary use. */
		property Object ^ appContext2 {
			Object ^ get () { return appContext2_; }
			void set ( Object ^ value )
			{
				appContext2_ = value;
				NotifyObservers ( DEVICE_INFO_ATTR_APP_CONTEXT, true );
				DeviceInstancePropertyNotify ( "appContext2", false );
			}  };
        
        /** Application defined context 3 for arbitrary use. */
		property Object ^ appContext3 {
			Object ^ get () { return appContext3_; }
			void set ( Object ^ value )
			{
				appContext3_ = value;
				NotifyObservers ( DEVICE_INFO_ATTR_APP_CONTEXT, true );
				DeviceInstancePropertyNotify ( "appContext3", false );
			}  };
        
        /** The ID that is used by the native layer to identify this particular device within the environment.
         A value of 0 means that this device is not connected and therefore not actively managed. */
		property int nativeID { int get () { return info_->nativeID; } };


		/** The device properties structure into a DeviceInfo object. */
		property environs::DeviceInfo ^ info { environs::DeviceInfo ^ get () { return info_; } };
        
        /** IP from device. The IP address reported by the device which it has read from network configuration. */
		property String ^ ips { String ^ get () { return ips_; } };
        
        /** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
         * This address could be different from IP due to NAT, Router, Gateways behind the device.
         */
		property String ^ ipes { String ^ get () { return ipes_; } };
        
        /** isConnected is true if the device is currently in the connected state. */
		property bool isConnected { bool get () { return info_->isConnected; } };
        
        /** Determines whether the remote device has attached an observer. */
		property bool isObserverReady { bool get () { return lib::DeviceInstance::isObserverReady(); } };
        
        /** Determines whether the remote device has attached a message observer. */
		property bool isMessageObserverReady { bool get () { return lib::DeviceInstance::isMessageObserverReady (); } };
        
        /** Determines whether the remote device has attached a data observer. */
		property bool isDataObserverReady { bool get () { return lib::DeviceInstance::isDataObserverReady (); } };
        
        /** Determines whether the remote device has attached a sesnor observer. */
		property bool isSensorObserverReady { bool get () { return lib::DeviceInstance::isSensorObserverReady (); } };
        
        /** sourceType is a value of environs::DeviceSourceType and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
		property environs::DeviceSourceType sourceType { environs::DeviceSourceType get () { return ( environs::DeviceSourceType )  info_->broadcastFound; } };

		property int hEnvirons { int get () { return lib::DeviceInstance::hEnvirons; } };

		property int connectProgress { int get () { return lib::DeviceInstance::connectProgress; } };
        
        /** The device ID within the environment */
		property int deviceID { int get () { return info_->deviceID; } };
        
        /** A value that describes the device platform. */
		property int platform { int get () { return info_->platform; } };
        
        /** The area name of the appliction environment. */
		property String ^ areaName { String ^ get () { return info_->areaName; } };
        
        /** The application name of the appliction environment. */
		property String ^ appName { String ^ get () { return info_->appName; } };
        
        /** The device name. */
		property String ^ deviceName { String ^ get () { return info_->deviceName; } };
        
        /** A descriptive string with the most important details. */
		property String ^ toString { String ^ get () { return ToString(); } };
        
        /** A DeviceDisplay structure that describes the device's display properties. */
		property DeviceDisplay display { DeviceDisplay get () { return lib::DeviceInstance::display; } };

		/**
		* disposed is true if the object is no longer valid. Nothing will be updated anymore.
		* disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
		* */
		property bool disposed { bool get () { return ( disposed_ == 1 ); } };


		property int directStatus { int get () { return directStatus_; } };

		virtual String ^ ToString () override;

#ifdef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
		/**
		* OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
		*
		* @param portal 		The PortalInstance object.
		*/
		delegate void PortalRequestOrProvided ( PortalInstance ^ portal );
		PortalRequestOrProvided ^ OnPortalRequestOrProvided;
#endif

	internal:
		/** Application defined contexts for arbitrary use. */
		int                     appContext0_;
		Object                ^ appContext1_;
		Object                ^ appContext2_;
		Object                ^ appContext3_;

		virtual environs::DeviceInstance ^ GetPlatformObj () override;
	};
}

#endif // CLI_CPP

#endif	/// INCLUDE_HCM_ENVIRONS_DEVICEISNTANCE_CLI_H


