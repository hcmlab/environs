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
 * Portal status enumeration.
 * */
public abstract class PortalStatus {

	@IntDef(flag=true, value={Disposed, Created, CreatedFromRequest, CreatedAskRequest, Established, Started})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int Disposed            	=	Types.PORTAL_STATUS_DISPOSED;
	public static final int Created             	=	Types.PORTAL_STATUS_CREATED;
	public static final int CreatedFromRequest  	=	Types.PORTAL_STATUS_CREATED_FROM_REQUEST;
	public static final int CreatedAskRequest   	=	Types.PORTAL_STATUS_CREATED_ASK_REQUEST;
	public static final int Established         	=	Types.PORTAL_STATUS_ESTABLISHED;
	public static final int Started             	=	Types.PORTAL_STATUS_STARTED;
		
}





