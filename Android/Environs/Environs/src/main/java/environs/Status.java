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
 * Environs Status enumeration. Represents the same values as for NATIVE_STATUS_* 
 * */
public abstract class Status {

	@IntDef(flag=true, value={Disposed, Uninitialized, Disposing, Initializing, Initialized, Stopped, StopInProgress, 
			Stopping, Starting, Started, Connected})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

			/** Disposed. */
	public static final int Disposed            	=	Types.STATUS_DISPOSED;
			/** Uninitialized. Usually after creation of an Environs object. */
	public static final int Uninitialized       	=	Types.STATUS_UNINITIALIZED;
			/** Environs is about to be disposed. */
	public static final int Disposing           	=	Types.STATUS_DISPOSING;
			/** Environs is initializing. */
	public static final int Initializing        	=	Types.STATUS_INITIALIZING;
			/** Environs is initialized. Usually after a call to Environs.Init() */
	public static final int Initialized         	=	Types.STATUS_INITIALIZED;
			/** Environs is stopped. Usually after a call to Environs.Stop() */
	public static final int Stopped             	=	Types.STATUS_STOPPED;
			/** Environs is currently stopping. Threads are being shut down and allocated resources are being released. */
	public static final int StopInProgress      	=	Types.STATUS_STOP_IN_PROGRESS;
			/** Environs is about to Stop. Threads are being shut down and allocated resources are being released. */
	public static final int Stopping            	=	Types.STATUS_STOPPING;
			/** Environs is about to Start. Thread are being started and resources are being allocated. */
	public static final int Starting            	=	Types.STATUS_STARTING;
			/** Environs is started. Usually after a call to Environs.Start() */
	public static final int Started             	=	Types.STATUS_STARTED;
			/** Environs is in connected state and connected to at least one device. */
	public static final int Connected           	=	Types.STATUS_CONNECTED;
}





