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
 * Environs source values which determines the source of an event, data, or message.&nbsp;
 * Represents the same values as for ENVIRONS_SOURCE_* 
 * */
public abstract class Source {

	@IntDef(flag=true, value={Native, Platform, Device, Application})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

			/** Sent by native layer. */
	public static final int Native              	=	Types.SOURCE_NATIVE;
			/** Sent by platform specific layer. */
	public static final int Platform            	=	Types.SOURCE_PLATFORM;
			/** Sent by another device within the environment.  */
	public static final int Device              	=	Types.SOURCE_DEVICE;
			/** Sent by the app layer. */
	public static final int Application         	=	Types.SOURCE_APPLICATION;
}





