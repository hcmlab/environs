/**
 * Environs CLI implementation base
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

#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Message.Instance.h"
#include "File.Instance.h"
#include "Portal/Portal.Instance.Cli.h"
#include "Device/Device.Instance.Cli.h"
#include "Device.List.h"
#include "Environs.Native.h"

#define CLASS_NAME	"Environs.Cli.Init. . . ."


namespace environs
{

	[DllImport ( "kernel32.dll" )]
	extern IntPtr LoadLibrary ( String ^ dllToLoad );


	bool Environs::SetLibsDirectory ( String ^newLibDir )
	{
		// Check whether folder exists
		if ( Directory::Exists ( newLibDir ) )
		{
			libDir = newLibDir;
			return PrepareNativeRuntime ( 0 );
		}
		return false;
	}


	String ^ getEnvironsMissingMsg ( String ^ reason )
	{
		return "Environs: Environs.dll is missing in the working directory! Environs will not work without the native component. [" + reason + "]";
	}


	static Environs::Environs ()
	{
		array< Environs ^>::Clear ( instancesAPI, 0, ENVIRONS_MAX_ENVIRONS_INSTANCES );

		if ( !Environs::ObjectAPIInit () ) {
			CErr ( "ObjectAPIInit: Failed to initialize Object API." );
		}

		PrepareNativeRuntime ( -1 );

		LockAcquireA ( platformLock, "Environs" );

		try
		{
			// Check for DLL
			if ( EnvironsModule == nullptr )
			{
				EnvironsModule = LoadLibrary ( nativeLib );
				if ( EnvironsModule == IntPtr::Zero || EnvironsModule  == nullptr )
				{
					System::Windows::MessageBox::Show ( getEnvironsMissingMsg ( "" ) );
					return;
				}

				DeviceInstanceSize = environs::API::GetDeviceInstanceSizeN ();
				DevicesHeaderSize = environs::API::GetDevicesHeaderSizeN ();

				nativeStatus = 2;
			}
			EnvironsModuleCounter++;
		}
		catch ( Exception ^ex )
		{
			System::Windows::MessageBox::Show ( getEnvironsMissingMsg ( ex->Message ) );
		}

		LockReleaseA ( platformLock, "Environs" );
	}


	bool Environs::PrepareNativeRuntime ( int rtIndex )
	{
		String ^rt = nullptr;

		if ( rtIndex < 0 ) 
		{
			// Try the platform toolset that we were build at first.
			rt = ENVIRONS_TOSTRING(ENVIRONS_PROJECT_CRT);
		}

		// Check whether a runtime is specified
		else if ( rtIndex == 0 )
		{

			array<String ^> ^files = Directory::GetFiles ( ".\\", ".env.*" );
			if ( files != nullptr && files->Length > 0 )
			{
				String ^file = files [ 0 ];
				if ( file->Length > 7 )
				{
					int pos = file->IndexOf ( ".env." );
					if ( pos >= 0 )
						rt = file->Substring ( pos + 5 );
				}
			}
			if ( rt == nullptr )
				rtIndex = 1;
		}

		if ( rtIndex >= runtimes->Length )
		{
			nativeLib = "./Environs.dll";

			if ( File::Exists ( nativeLib ) ) {
				nativeStatus = 1;
				return true;
			}

			System::Windows::MessageBox::Show ( getEnvironsMissingMsg ( "Environs.dll" ) );
			return false;
		}

		if ( rt == nullptr )
			rt = runtimes [ rtIndex ];

		nativeLib = libDir + rt + "/Environs.dll";

		if ( !File::Exists ( nativeLib ) )
		{
			return PrepareNativeRuntime ( rtIndex + 1 );
		}
		return true;
	}


	namespace lib
	{

		bool Environs::InitPlatform ()
		{
			listAllObservers = gcnew ArrayList ();
			if ( !listAllObservers )
				return false;

			listNearbyObservers = gcnew ArrayList ();
			if ( !listNearbyObservers )
				return false;

			listMediatorObservers = gcnew ArrayList ();
			if ( !listMediatorObservers )
				return false;
			return true;
		}

		void MessageInstance::PlatformDispose ()
		{
		}

		void FileInstance::PlatformDispose ()
		{
		}

		void DeviceInstance::PlatformDispose ()
		{
		}

		void DeviceList::PlatformDispose ()
		{
		}

		void PortalInstance::PlatformDispose ()
		{
		}


		bool PortalInstance::ShowDialogOutgoingPortal ()
		{
#ifndef CLI_NOUI
			String ^ message = "Establish outgoing portal to " + device_->info_->deviceID + " " + device_->info_->appName + "/" + device_->info_->areaName;
			String ^ caption = "Outgoing portal";

			System::Windows::Forms::DialogResult  msgboxRes = System::Windows::Forms::MessageBox::Show (
				message,
				caption,
				System::Windows::Forms::MessageBoxButtons::YesNo,
				System::Windows::Forms::MessageBoxIcon::Question
				);

			if ( msgboxRes == System::Windows::Forms::DialogResult::Yes )
			{
				Establish ( false );
				return true;
			}
#endif
			return false;
		}
	}

}

#endif



