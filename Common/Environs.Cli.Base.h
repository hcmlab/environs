/**
 * Environs CLI base class
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_CLI_BASE_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_CLI_BASE_H

#ifdef CLI_CPP

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"

#include "Environs.Types.h"
#include "Environs.Msg.Types.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Observer.CLI.h"
#include "Environs.Types.h"
#include "Device.Display.Decl.h"

#using "WindowsBase.dll"
#using "PresentationFramework.dll"
#using "PresentationCore.dll"

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Runtime::InteropServices;


namespace environs
{
	namespace lib
	{

	}
}

#endif

#endif






















