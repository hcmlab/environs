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
 * Deviceflags for internalFlags enumeration.
 * */
public abstract class DeviceFlagsInternal {

	@IntDef(flag=true, value={NativeReady, PlatformReady, ObserverReady, MessageReady, DataReady, SensorReady, NotifyMask, 
			CPNativeReady, CPPlatformReady, CPObserverReady, CPMessageReady, CPDataReady, CPSensorReady, CPNotifyMask, 
	})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int NativeReady         	=	Types.DEVICEFLAGS_INTERNAL_NATIVE_READY;
	public static final int PlatformReady       	=	Types.DEVICEFLAGS_INTERNAL_PLATFORM_READY;
	public static final int ObserverReady       	=	Types.DEVICEFLAGS_INTERNAL_OBSERVER_READY;
	public static final int MessageReady        	=	Types.DEVICEFLAGS_INTERNAL_MESSAGE_READY;
	public static final int DataReady           	=	Types.DEVICEFLAGS_INTERNAL_DATA_READY;
	public static final int SensorReady         	=	Types.DEVICEFLAGS_INTERNAL_SENSOR_READY;
	public static final int NotifyMask          	=	Types.DEVICEFLAGS_INTERNAL_NOTIFY_MASK;
		
	public static final int CPNativeReady       	=	Types.DEVICEFLAGS_INTERNAL_CP_NATIVE_READY;
	public static final int CPPlatformReady     	=	Types.DEVICEFLAGS_INTERNAL_CP_PLATFORM_READY;
	public static final int CPObserverReady     	=	Types.DEVICEFLAGS_INTERNAL_CP_OBSERVER_READY;
	public static final int CPMessageReady      	=	Types.DEVICEFLAGS_INTERNAL_CP_MESSAGE_READY;
	public static final int CPDataReady         	=	Types.DEVICEFLAGS_INTERNAL_CP_DATA_READY;
	public static final int CPSensorReady       	=	Types.DEVICEFLAGS_INTERNAL_CP_SENSOR_READY;
	public static final int CPNotifyMask        	=	Types.DEVICEFLAGS_INTERNAL_CP_NOTIFY_MASK;
		
}





