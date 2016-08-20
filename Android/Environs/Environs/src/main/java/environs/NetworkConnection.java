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
 * Network connection enumeration.
 * */
public abstract class NetworkConnection {

	@IntDef(flag=true, value={TriggerUpdate, Unknown, NoNetwork, NoInternet, MobileData, WiFi, LAN, 
	})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int TriggerUpdate       	=	Types.NETWORK_CONNECTION_TRIGGER_UPDATE;
	public static final int Unknown             	=	Types.NETWORK_CONNECTION_UNKNOWN;
	public static final int NoNetwork           	=	Types.NETWORK_CONNECTION_NO_NETWORK;
	public static final int NoInternet          	=	Types.NETWORK_CONNECTION_NO_INTERNET;
	public static final int MobileData          	=	Types.NETWORK_CONNECTION_MOBILE_DATA;
	public static final int WiFi                	=	Types.NETWORK_CONNECTION_WIFI;
	public static final int LAN                 	=	Types.NETWORK_CONNECTION_LAN;
		
}





