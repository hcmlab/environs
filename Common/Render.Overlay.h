/**
* Render overlay declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_RENDEROVERLAY_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_RENDEROVERLAY_DECLARATIONS_H_

/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif
    
    
	typedef struct _RenderOverlay
	{
		int					left;
		int					top;
		unsigned int		width;
		unsigned int		height;
		unsigned int		stride;
		void			*	data;
		int					alpha;
		bool				positionDevice;
		int					errorCount;

		void			*	renderArg1;
		void			*	renderArg2;
		void			*	renderArg3;
	}
	RenderOverlay;


#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_HCM_ENVIRONS_RENDEROVERLAY_DECLARATIONS_H_ */
