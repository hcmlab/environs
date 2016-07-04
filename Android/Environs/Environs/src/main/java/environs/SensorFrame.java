package environs;
/**
 *	SensorFrame
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
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
/**
 *	SensorFrame
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public class SensorFrame
{
    public int objID   = 0;
    public DeviceInstance device;

    public @SensorType.Value int type = SensorType.Accelerometer;

    // Increase with each frame
    public int         seqNumber;

    // Location: f1 = accuracy (latitude/longitude)
    // Light: f1 = light in Lux
    public float		f1;
    // Location: f2 = accuracy (longitude)
    public float		f2;
    // Location: f3 = speed
    public float		f3;

    // Location: d1 = latitude
    public double		d1;
    // Location: d2 = longitude
    public double		d2;
    // Location: d3 = altitude
    public double		d3;
}
