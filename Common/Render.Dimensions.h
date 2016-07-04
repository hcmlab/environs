/**
 * Render dimensions declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_RENDERDIMENSIONS_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_RENDERDIMENSIONS_DECLARATIONS_H_


/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif
    
    
	typedef struct _RenderDimensions
	{
		int				square;
		int				left;
		int				top;

		int				streamWidth;
		int				streamHeight;
		
		int				xOffset;
		int				yOffset;

		int				left_cap;
		int				top_cap;
		int				width_cap;
		int				height_cap;
		
		float			orientation;
	}
	RenderDimensions;


#ifdef __cplusplus
}
#endif



#endif /* INCLUDE_HCM_ENVIRONS_RENDERDIMENSIONS_DECLARATIONS_H_ */
