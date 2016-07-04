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
#include "stdafx.h"

#ifdef CLI_CPP

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Environs.h"
#include "Device.Instance.Cli.h"
#include "Environs.Cli.h"
#include "Environs.Native.h"

#define CLASS_NAME	"Device.Instance.Cli. . ."


namespace environs
{
	DeviceInstance::DeviceInstance ()
	{
		CVerbVerb ( "Construct" );

		appContext0_ = 0;
		appContext1_ = nullptr;
		appContext2_ = nullptr;
		appContext3_ = nullptr;		
	}


	DeviceInstance::~DeviceInstance ()
	{
		CVerbVerbArg1 ( "Destruct", "", "i", objID_ );
	}


	String ^ DeviceInstance::ToString ()
	{
		CVerbVerb ( "ToString" );

		return lib::DeviceInstance::toString ();
	}


	ref class OnPropertyChangedClass
	{
		String ^ prop;
		DeviceInstance ^ device;

	public:
		bool updated;

		OnPropertyChangedClass ( DeviceInstance ^ dev, String ^ name ) : device ( dev ), prop ( name ) { }

		void Run ()
		{
			CVerbVerb ( "Run" );

			device->OnPropertyChangedDo ( prop );
		}
	};


	void DeviceInstance::OnPropertyChangedDo ( String ^ name )
	{
		PropertyChanged ( this, gcnew System::ComponentModel::PropertyChangedEventArgs ( name ) );
		PropertyChanged ( this, gcnew System::ComponentModel::PropertyChangedEventArgs ( "toString" ) );
	}


	void DeviceInstance::OnPropertyChanged ( String ^ name, bool ignoreDefaultSetting )
	{
		if ( !ignoreDefaultSetting && !notifyPropertyChanged )
			return;

		if ( name == nill )
			return;

		OnPropertyChangedClass ^ act = gcnew OnPropertyChangedClass ( this, name );

		Action ^ action = gcnew Action ( act, &OnPropertyChangedClass::Run );

		environs::Environs::dispatch ( action );
	}


	EPSPACE DeviceInstance ^ DeviceInstance::GetPlatformObj ()
	{
		return this;
	}

	/**
	* Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
	*
	* @return Collection with objects of type MessageInstance
	*/
	NLayerVecType ( EPSPACE MessageInstance ) DeviceInstance::GetMessages ()
	{
		// Prevent garbage collection from deleting us while this operation may take longer
		DeviceInstance ^ device = this;

		NLayerVecType ( EPSPACE MessageInstance ) list = environs::lib::DeviceInstance::GetMessages();
		if ( device->disposed ) {
			CWarn ( "GetMessages: Disposed while we load messages from storage." );
		}
		return list;
	}

	/**
	* Get a list with all messages that this device has received (and sent) from the storage.
	*
	* @return Collection with objects of type MessageInstance
	*/
	NLayerVecType(EPSPACE MessageInstance) DeviceInstance::GetMessagesInStorage()
	{
		// Prevent garbage collection from deleting us while this operation may take longer
		DeviceInstance ^ device = this;

		NLayerVecType(EPSPACE MessageInstance) list = environs::lib::DeviceInstance::GetMessagesInStorage();
		if (device->disposed) {
			CWarn("GetMessagesInStorage: Disposed while we load messages from storage.");
		}
		return list;
	}

	/**
	* Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
	*
	* @return Collection with objects of type MessageInstance
	*/
	NLayerMapType ( int, EPSPACE FileInstance ) DeviceInstance::GetFiles ()
	{
		// Prevent garbage collection from deleting us while this operation may take longer
		DeviceInstance ^ device = this;

		NLayerMapType ( int, EPSPACE FileInstance ) list = environs::lib::DeviceInstance::GetFiles();
		if ( device->disposed ) {
			CWarn ( "GetFiles: Disposed while we load files from storage." );
		}
		return list;
	}

	/**
	* Get a list with all messages that this device has received (and sent) from the storage.
	*
	* @return Collection with objects of type MessageInstance
	*/
	NLayerMapType(int, EPSPACE FileInstance) DeviceInstance::GetFilesInStorage()
	{
		// Prevent garbage collection from deleting us while this operation may take longer
		DeviceInstance ^ device = this;

		NLayerMapType(int, EPSPACE FileInstance) list = environs::lib::DeviceInstance::GetFilesInStorage();
		if (device->disposed) {
			CWarn("GetFilesInStorage: Disposed while we load files from storage.");
		}
		return list;
	}
}


#endif // CLI_CPP





