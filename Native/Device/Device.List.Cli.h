/**
 * DeviceLists CLI
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_CLI_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_CLI_H

#ifdef CLI_CPP

#include "Device.List.h"


/**
 *	DeviceLists CLI
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	public ref class DeviceList : public environs::lib::DeviceList
	{
	public:
		DeviceList ();
		~DeviceList ();

		property bool disposed { bool get () { return disposed_; } };

		property int Count { int get () { return lib::DeviceList::GetCount (); } };

	};


#ifndef CLI_NOUI
	generic <class T>
	ref class EnvObservableCollection : public ObservableCollection<T>
	{
	public:
		bool notificationsHappend;
		bool notificationsActive;

		EnvObservableCollection<T> () : notificationsHappend ( false ), notificationsActive ( true )
		{ }

		EnvObservableCollection ( System::Collections::Generic::List<T> ^ list ) : ObservableCollection ( list )
		{ }

		property bool notificationEnabled { 
			bool get () { return notificationsActive; } 
			void set ( bool value )
			{
				notificationsActive = value;

				if ( notificationsActive && notificationsHappend )
				{
					ObservableCollection::OnCollectionChanged ( 
						gcnew System::Collections::Specialized::NotifyCollectionChangedEventArgs ( 
							System::Collections::Specialized::NotifyCollectionChangedAction::Reset ) );
					notificationsHappend = false;
				}
			}
		};

		virtual void OnCollectionChanged ( System::Collections::Specialized::NotifyCollectionChangedEventArgs ^ e ) override
		{
			if ( notificationsActive )
				ObservableCollection::OnCollectionChanged ( e );
			else
				notificationsHappend = true;
		}
	};
#endif
}

#endif // CLI_CPP

#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_CLI_H


