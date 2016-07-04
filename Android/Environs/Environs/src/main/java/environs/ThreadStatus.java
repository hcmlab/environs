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
 * Environs thread Status enumeration.
 * */
public abstract class ThreadStatus {

	@IntDef(flag=true, value={NoThread, Detacheable, Running})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

			/** Uninitialized. Usually after creation of an Environs object. */
	public static final int NoThread            	=	Types.ENVIRONS_THREAD_NO_THREAD;
			/** Thread is either created and not yet running or terminated. */
	public static final int Detacheable         	=	Types.ENVIRONS_THREAD_DETACHEABLE;
			/** Thread is running. */
	public static final int Running             	=	Types.ENVIRONS_THREAD_RUNNING;
		
}




