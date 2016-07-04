/**
 * Environs CLI Utilities
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_CLI_TOOLS_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_CLI_TOOLS_H

#ifdef CLI_CPP

#include "Environs.Build.Opts.h"

namespace environs
{
	ref class AvContext;

	public ref class Utils
	{
	public:
		static int logLevel =
#ifdef NDEBUG
			0;
#else
			1;
#endif
		static void Log ( int level, String ^ msg );

		static bool Log ( int level );

		static void Log ( String ^ className, String ^ msg );

		static void LogW ( String ^ className, String ^ msg );

		static void LogW ( String ^ msg );

		static void LogE ( String ^ className, String^  msg );

		static void LogE ( String ^ msg );

		static void Log ( int level, String ^ className, String ^ msg );

		static void Log1 ( String ^ className, String ^ msg );

		static String ^ GetExecPath ();

		static AvContext ^ GetAvContext ( IntPtr ^ avContext );

		static bool CreateUpdateBitmap ( System::Windows::Media::Imaging::WriteableBitmap ^% wbm, IntPtr ^ avContext );

		static bool CreateUpdateBitmap ( System::Windows::Media::Imaging::WriteableBitmap ^% wbm, AvContext ^ context );

	internal:
		static void Log ( int level, String ^ msg, bool withDateTime );
	};
	

	/**
	* Go through all created instances, stop all Environs activities and release all acquired resources.
	*/
	ref class StaticDisposer 
	{
	public:
		StaticDisposer ();
		~StaticDisposer ();
	};


	namespace lib
	{
		environs::DeviceInfo ^BuildDeviceInfoCli ( unsigned char * pDevice, bool mediator );
		
		cli::array < environs::DeviceInstance ^ > ^ BuildDeviceInfoList ( int hInst, IntPtr ^pDevices, bool mediator );

		char *  BuildNativePortalInfo ( PortalInfoBase ^ info );
		
		PortalInfoBase ^ ParsePortalInfo ( IntPtr ^ portalInfoStruct );

		environs::InputPack ^ GetInputPack ( IntPtr data );

		environs::SensorFrame ^ GetSensorInputPack ( Addr_ptr data );

		environs::DeviceDisplay ^ BuildDeviceDisplayProps ( void * pData, int nativeID );

		void DeviceListAppendFunc ( devListRef ( EPSPACE DeviceInstance ) list, EPSPACE DeviceInstance ^ device );

		void DeviceListInsertFunc ( devListRef ( EPSPACE DeviceInstance ) list, EPSPACE DeviceInstance ^ device, int pos );

		void DeviceListRemoveAtFunc ( devListRef ( EPSPACE DeviceInstance ) list, int pos );

		void DeviceListClearFunc ( devListRef ( EPSPACE DeviceInstance ) list );
	}
}

#endif

#endif






















