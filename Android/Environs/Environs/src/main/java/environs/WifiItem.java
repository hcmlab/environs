package environs;
/**
 * Wifi Observer Item.
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
public class WifiItem
{
    public long         bssid;			// 8
    public short        rssi;			// 2
    public short		signal;			// 2
    public char         channel;		// 1
    public char         encrypt;		// 1
    public boolean      isConnected;	// 1
    public short		sizeOfssid;		// 1
    public String       ssid;

    public String toString ()
    {
        return "Wifi: " + ssid + " [ " + GetMac() + " : " + rssi + " ]";
    }

    public String GetMac()
    {
        return Long.toHexString ( bssid );
    }
}
