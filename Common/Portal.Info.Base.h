/**
 * Base Portal Information
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINFO_BASE_H
#define INCLUDE_HCM_ENVIRONS_PORTALINFO_BASE_H


/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

/* Namespace: environs -> */
namespace environs
{
#endif
	/**
	*	A PortalInfoBase object serves as container for raw portal information.
    *   Environs makes use of such objects to get/set portal details.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	current size is (4 + 4 + 4 + 4 + 4) = 20 bytes
	* ****************************************************************************************
	*/
#ifndef CLI_CPP

	typedef struct _PortalInfoBase
	{
		int portalID;
		int flags;

		int centerX;
		int centerY;
		int width;
		int height;
		float orientation;
	}
	PortalInfoBase;

#else

	PUBLIC_CLASS PortalInfoBase
	{
	public:
		int portalID;
		int flags;

		int centerX;
		int centerY;
		int width;
		int height;
		float orientation;
	};

#endif

    
#ifdef __cplusplus
} /* namepace Environs */
#endif

#endif // INCLUDE_HCM_ENVIRONS_PORTALINFO_BASE_H
