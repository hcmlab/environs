/**
 * Environs AV context for data exchange 
 * between native and platform layer
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
#ifndef INCLUDE_HCM_ENVIRONS_AVCONTEXT_DECLS
#define	INCLUDE_HCM_ENVIRONS_AVCONTEXT_DECLS


namespace environs {
	/**
	*	Environs AV context for data exchange 
	*	between native and platform layer
	*	--------------------------------------------------------------------------------------
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	
	* ****************************************************************************************
	*/
	typedef struct _EnvironsAVContext {
		int            width;
		int            height;
		int            stride;
		int            pixelFormat; // a value of 
		char		*	data;     
	}
	EnvironsAVContext;

} /* namespace environs */


#endif /* INCLUDE_HCM_ENVIRONS_AVCONTEXT_DECLS */
