/**
 * Dynamically accessing uuid
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
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_UUID_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_UUID_H

#include "Interop/Export.h"

#if defined(LINUX)

namespace environs
{
	typedef unsigned char uuid_t [ 16 ];

	typedef void ( CallConv * puuid_generate ) ( uuid_t );
	typedef int ( CallConv * puuid_is_null ) ( const uuid_t );

	typedef void ( CallConv * puuid_unparse_upper ) ( const uuid_t, char * );

	extern void ReleaseLibUUID ();

    extern bool InitLibUUID ( int deviceID );

	extern puuid_generate			duuid_generate;
	extern puuid_is_null			duuid_is_null;
	extern puuid_unparse_upper      duuid_unparse_upper;

} // -> namespace environs

#endif

#endif	/// INCLUDE_HCM_ENVIRONS_DYNAMIC_UUID_H
