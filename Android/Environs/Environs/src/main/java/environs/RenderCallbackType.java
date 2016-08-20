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
 * Environs RENDER_CALLBACK_TYPES enumeration.
 * */
public abstract class RenderCallbackType {

	@IntDef(flag=true, value={All, Init, AvContext, Decoder, Image})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int All                 	=	Types.RENDER_CALLBACK_TYPE_ALL;
	public static final int Init                	=	Types.RENDER_CALLBACK_TYPE_INIT;
	public static final int AvContext           	=	Types.RENDER_CALLBACK_TYPE_AVCONTEXT;
	public static final int Decoder             	=	Types.RENDER_CALLBACK_TYPE_DECODER;
	public static final int Image               	=	Types.RENDER_CALLBACK_TYPE_IMAGE;
		
}





