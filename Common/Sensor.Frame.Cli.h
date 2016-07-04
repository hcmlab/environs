/**
* SensorFrame CLI Declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_SENSOR_FRAME_CLI_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_SENSOR_FRAME_CLI_DECLARATIONS_H_

#ifdef CLI_CPP
#	include "Environs.Msg.Types.h"
#	include "Environs.Types.h.Cli.h"

	using namespace System;


namespace environs
{

	public ref class SensorFrame
	{
	public:
		DeviceInstancePtr device;

		// An id that identifies this input entity for its whole lifecycle
		int			id;

		// A value of type environs.SensorType
		environs::SensorType	type;

		// Increase with each frame
		int         seqNumber;

		// Location: x = latitude
		double		x;
		// Location: y = longitude
		double		y;
		// Location: z = altitude
		double		z;


		// Location: f1 = accuracy (latitude/longitude)
		// Light: f1 = light in Lux
		float		f1;
		// Location: f2 = accuracy (longitude)
		float		f2;
		// Location: f3 = speed
		float		f3;
	};
}

#endif

#endif /* INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_ */
