package environs;

/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar */

/**
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
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
import android.support.annotation.IntDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Environs mediator filter enumeration.
 * */
public abstract class MediatorFilter {

	@IntDef(flag=true, value={None, Area, AreaAndApp, All})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int None                	=	Types.MEDIATOR_FILTER_NONE;
	public static final int Area                	=	Types.MEDIATOR_FILTER_AREA;
	public static final int AreaAndApp          	=	Types.MEDIATOR_FILTER_AREA_AND_APP;
	public static final int All                 	=	Types.MEDIATOR_FILTER_ALL;
		
}





