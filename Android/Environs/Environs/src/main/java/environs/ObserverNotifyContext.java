package environs;
/**
 * ObserverNotifyContext
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
public class ObserverNotifyContext
{
    /** The native/device identifier that targets the device */
    public int              destID = 0;
    /** Area name of the application environment */
    public String			areaName;
    /** Area name of the application environment */
    public String			appName;
    /** The notification */
    public @Notify.Value int notification;
    /** A value of the enumeration Types.EnvironsSource */
    public int				sourceIdent;

    public byte []          contextPtr;

    /** A value that provides additional context information (if available). */
    public int				context;
    /**  */
    public int				hEnvirons;

    public Environs         env;
}
