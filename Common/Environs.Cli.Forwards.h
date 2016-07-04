/**
* Environs CLI forward declarations
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

#ifdef CLI_CPP

#include "Interop.h"
#include "Interop/Smart.Pointer.h"
#include "Environs.Platforms.h"
#include "Environs.Platform.Decls.h"

using namespace System;
using namespace System::Runtime::InteropServices;

#if (defined(CLI_PS) || defined(CLI_STT))

#	define ENV_CLIENTWINDOW		Microsoft::Surface::Presentation::Controls::SurfaceWindow

#else

#	define ENV_CLIENTWINDOW		System::Windows::Window

#endif

namespace environs
{
	CLASS Environs;
	CLASS StaticDisposer;

	CLASS PortalInfo;
	CLASS PortalInfoBase;
	CLASS SensorFrame;

	CLASS DeviceInfo;
	CLASS DeviceList;
	CLASS DeviceInstance;
	CLASS FileInstance;
	CLASS MessageInstance;
	CLASS PortalInstance;

	CLASS InputPack;
	CLASS ApplicationEnvironment;

	namespace lib
	{
		CLASS Environs;
		CLASS EnvironsBase;
		CLASS DeviceList;
		CLASS DeviceInstance;
		CLASS PortalInstance;
		CLASS FileInstance;

		CLASS ListCommandContext;
		CLASS DeviceNotifierContext;
		
		STRUCT UdpDataContext;

		typedef System::Object 		IIEnvironsObserver;
	}

}

#endif