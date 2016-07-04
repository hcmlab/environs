package environs;
/**
 * PortalInfo
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

/** 
 * A PortalInfo object serves as container for portal information.&nbsp;
 * Environs makes use of such objects to get/set portal details.
 * 
 * @author Chi-Tai Dang
 *
 */
public class PortalInfo 
{
	private static final String className = "PortalInfo . . . . . . .";

	public int deviceID;
	public int portalID;
	public int flags;
	
	public int centerX;
	public int centerY;
	public int width;
	public int height;
	public float orientation;

	PortalInstance	portal = null;


	void NotifyObservers(int notification)
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "NotifyObservers" );

		if ( portal != null )
			portal.NotifyObservers ( notification );
	}


	public void SetSize(int width, int height)
	{
		flags = 0;
		this.width = width;
		this.height = height;
		flags |= Environs.PORTAL_INFO_FLAG_SIZE;

		portal.SetPortalInfo(this);
	}

	public void SetOrientation(float angle)
	{
		flags = 0;
		this.orientation = angle;
		flags |= Environs.PORTAL_INFO_FLAG_ANGLE;

		portal.SetPortalInfo(this);
	}

	public void SetLocation(int centerX, int centerY)
	{
		flags = 0;
		this.centerX = centerX;
		this.centerY = centerY;
		flags |= Environs.PORTAL_INFO_FLAG_LOCATION;

		portal.SetPortalInfo(this);
	}

	public void SetLocation(int centerX, int centerY, float angle)
	{
		flags = 0;
		this.centerX = centerX;
		this.centerY = centerY;
		flags |= Environs.PORTAL_INFO_FLAG_LOCATION;
		this.orientation = angle;
		flags |= Environs.PORTAL_INFO_FLAG_ANGLE;

		portal.SetPortalInfo(this);
	}

	public void Set(int centerX, int centerY, float angle, int width, int height)
	{
		flags = 0;
		this.centerX = centerX;
		this.centerY = centerY;
		flags |= Environs.PORTAL_INFO_FLAG_LOCATION;
		this.orientation = angle;
		flags |= Environs.PORTAL_INFO_FLAG_ANGLE;
		this.width = width;
		this.height = height;
		flags |= Environs.PORTAL_INFO_FLAG_SIZE;

		portal.SetPortalInfo(this);
	}

	
	@SuppressLint("DefaultLocale")
	public String toString() {
		
        return ("Portal: center coordinates [ " + centerX + " / " + centerY + " ], size [ " 
        + width + " / " + height + " ], orientation [ " + orientation + " ]");
    }

	public String ToString() {
		return toString();
	}


	boolean Update(int notification, PortalInfo info)
	{
		//if (Utils.isDebug) Utils.Log ( 0, className, "Update: " + info.ToString () );

		boolean changedl = false;
		boolean changeds = false;

		if (portalID != info.portalID && info.portalID > 0)
			portalID = info.portalID;

		if (notification == 0 || notification == Environs.NOTIFY_PORTAL_LOCATION_CHANGED)
		{
			if (info.centerX != centerX)
			{
				changedl = true;
				centerX = info.centerX;
			}

			if (info.centerY != centerY)
			{
				changedl = true;
				centerY = info.centerY;
			}

			if (info.orientation != orientation)
			{
				changedl = true;
				orientation = info.orientation;
			}

			if (changedl)
				NotifyObservers(Environs.NOTIFY_PORTAL_LOCATION_CHANGED);
		}

		if (notification == 0 || notification == Environs.NOTIFY_PORTAL_SIZE_CHANGED)
		{
			if (info.width != width)
			{
				changeds = true;
				width = info.width;
			}

			if (info.height != height)
			{
				changeds = true;
				height = info.height;
			}

			if (changeds)
				NotifyObservers(Environs.NOTIFY_PORTAL_SIZE_CHANGED);
		}
		return (changedl | changeds);
	}
}
