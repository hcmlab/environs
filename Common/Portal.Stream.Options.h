/**
 * Portal stream options declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_STREAM_OPTIONS_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_PORTAL_STREAM_OPTIONS_DECLARATIONS_H_


namespace environs
{
	typedef struct _PortalStreamOptions
	{
		bool			usePNG;
		bool			useOpenCL;
		bool			useStream;
		bool			useNativeResolution;
		bool			streamOverCom;
		bool			limitMaxResolution;

		unsigned int	streamWidth;
		unsigned int	streamHeight;
        int             streamType;
	}
	PortalStreamOptions;
}


#endif /* INCLUDE_HCM_ENVIRONS_PORTAL_STREAM_OPTIONS_DECLARATIONS_H_ */
