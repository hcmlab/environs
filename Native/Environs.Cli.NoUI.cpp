/**
 * Environs CLI Windows No UI part
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

#if (defined( CLI_CPP) && defined(CLI_NOUI))

#include "Environs.Types.h.cli.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Cli.NoUI.h"
#include "Message.Instance.h"
#include "File.Instance.h"
#include "Portal.Instance.h"
#include "Device.Instance.h"
#include "Device.List.h"
#include "Interop/Stat.h"
#include "Environs.h"

#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>

#using <system.dll>

namespace environs
{
	void InvokeNetworkNotifier ( int hInst, bool enable )
	{

	}

	void EnvironsPlatformInit ( int hInst )
	{

	}

	void Environs::StartPlatformHandlers ()
	{

	}


	void Environs::ReleasePlatformLayer ()
	{
	}


	bool Environs::InitPlatformLayer ()
	{
		environs::API::SetUseHeadlessN ( 1 );
		return true;
	}


	/// <summary>
	/// A static method of the surface specific handling of injection of contact down events.
	/// </summary>
	/// <param name="id"></param>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void Environs::InjectTouch ( int nativeID, InputPack ^ pack )
	{
	}


	namespace lib
	{
	}

	namespace API
	{

		void Environs_LoginDialog ( int hInst, CString_ptr userName )
		{

		}


		bool RenderSurfaceCallback ( int type, void * surface, void * decoderOrByteBuffer )
		{
			return false;
		}
	}

}


#endif