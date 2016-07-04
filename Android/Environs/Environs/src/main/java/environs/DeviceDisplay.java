package environs;
/**
 *	DeviceDisplay
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
import android.annotation.SuppressLint;

/** Description of a device screen
 * 
 * @author Chi-Tai Dang
 *
 */
public class DeviceDisplay
{
	public int		width;
	public int		width_mm;
	public int		height;
	public int		height_mm;
	public int		orientation;
	public float	dpi;
	
	@SuppressLint("DefaultLocale")
	public String toString() {
		
        return ( "Width [" + width + "] height ["
        		+ height + "] width_mm [" + width_mm + "] height_mm [" + height_mm + "]");
    }
}
