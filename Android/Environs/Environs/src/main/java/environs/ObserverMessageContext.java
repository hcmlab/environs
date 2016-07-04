package environs;
/**
 * ObserverMessageContext
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
@SuppressWarnings ( "unused" )
public class ObserverMessageContext
{
    /** The native/device identifier that targets the device */
    public int              destID = 0;
    /** Area name of the application environment */
    public String    		areaName;
    /** Area name of the application environment */
    public String    		appName;
    /** Determines the source ( either from a device, environs, or native layer ) */
    public int             sourceType;
    /** The message as string text */
    public String    		message;
    /** The length of the message */
    public int             length;
    /** A connection type */
    public char            connection;
}
