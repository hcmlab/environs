package environs;
/**
 *	FileInstance
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

import android.support.annotation.Nullable;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 *	FileInstance
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
@SuppressWarnings ( "unused" )
public class FileInstance
{
    private static final String className = "FileInstance . . . . . .";

    /**
     * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
     * A value of 0 indicates an invalid fileID.
     * */
    public int fileID = 0;

    /**
     * Used internally.
     * */
    public int type = 0;

    /**
     * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
     * */
    public String descriptor = null;

    /**
     * sent is true if this FileInstance is data that was sent or received (false).
     * */
    public boolean sent = false;

    /**
     * created is a posix timestamp that determines the time and date that this FileInstance
     * has been received or sent.
     * */
    public long created;

    /**
     * The size in bytes of a buffer to send or data received.
     * */
    public long size = 0;

    /**
     * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
     * */
    public String path = null;

    /**
     * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
     * file or buffer that has already been sent.
     * If this value changes, then the corresponding device's DeviceObserver is notified
     * with this FileInstance object as the sender
     * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
     * */
    public int sendProgress = 0;

    /**
     * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
     * file or buffer that has already been received.
     * If this value changes, then the corresponding device's DeviceObserver is notified
     * with this FileInstance object as the sender
     * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
     * */
    public int receiveProgress = 0;

    /**
     * A reference to the DeviceInstance that is responsible for this FileInstance.
     * */
    public DeviceInstance device = null;

    FileInstance(int fileID)
    {
        this.fileID = fileID;
    }


    boolean Init(DeviceInstance device, File file)
    {
        if (device == null)
            return false;

        this.device = device;

        try
        {
            // Read descriptor
            if (file.exists())
            {
                InputStream ins = new FileInputStream(file.getAbsolutePath());

                BufferedReader reader = new BufferedReader(new InputStreamReader(ins));

                descriptor = reader.readLine();

                reader.close();
                ins.close();
            }

            size = file.length();
            created = file.lastModified();

            //Utils.Log(1, className, "Init: Added new file. " + toString());
            return true;
        }
        catch (Exception ex) {
            ex.printStackTrace ();
        }
        return false;
    }

    @Nullable
    public byte[] data()
    {
        return device.GetFile(fileID);
    }


    @Override
    public String toString()
    {
        return "FileInstance fileID [ " + fileID + " ]  desc [ " + descriptor + " ]  size [ " + size + " bytes] created at [ " + new java.util.Date(created*1000) + " ]";
    }

    public String ToString() {
        return toString();
    }

    public String GetPath()
    {
        if (path == null)
            return device.StoragePath + fileID + ".bin";
        return path;
    }
}
