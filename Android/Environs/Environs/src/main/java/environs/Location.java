package environs;
/**
 * PortalReceiver
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
public class Location
{
    public double latitude;
    public double longitude;
    public double altitude;
    public float accuracyLatitude;
    public float accuracyLongitude;
    public float speed;


    Location (android.location.Location loc)
    {
        if (loc != null)
        {
            latitude = loc.getLatitude ();
            longitude = loc.getLongitude ( );
            altitude = loc.getAltitude ( );
            accuracyLatitude = loc.getAccuracy ( );
            accuracyLongitude = accuracyLatitude;
            speed = loc.getSpeed ();
        }
    }
}
