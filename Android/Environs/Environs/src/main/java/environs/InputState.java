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
 * Input state enumeration.
 * */
public abstract class InputState {

	@IntDef(flag=true, value={Add, Change, NoChange, Drop})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int Add                 	=	Types.INPUT_STATE_ADD;
	public static final int Change              	=	Types.INPUT_STATE_CHANGE;
	public static final int NoChange            	=	Types.INPUT_STATE_NOCHANGE;
	public static final int Drop                	=	Types.INPUT_STATE_DROP;
		
}





