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
#include "Environs.Types.h.cli.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Cli.Base.h"

#include "Environs.Native.h"
#include "Environs.Lib.h"

#	include <stdio.h>
#	include <stdarg.h>
#	include <stdlib.h>

#using <system.dll>
#using <mscorlib.dll>
#using <System.Windows.Forms.dll>

using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

#define CLASS_NAME	"Environs.Cli . . . . . ."


namespace environs
{
	namespace lib
	{
	}


	namespace API
	{
		/**
		* Register to sensor events and listen to sensor data events.
		* This implementation is platform specific and needs to be implemented
		* in the particular platform layer.
		*
		* @param sensorType A value of type SensorType.
		*
		*/
		void StartSensorListening ( int hInst, SensorType sensorType )
		{
			// Needs to be implemented for Winodws
		}


		/**
		* Deregister to sensor events and stop listen to sensor data events.
		* This implementation is platform specific and needs to be implemented
		* in the particular platform layer.
		*
		* @param sensorType A value of type SensorType.
		*
		*/
		void StopSensorListening ( int hInst, SensorType sensorType )
		{
			// Needs to be implemented for Winodws
		}

	}


}

#endif



