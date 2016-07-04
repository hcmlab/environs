package environs;
/**
 * DataObserver
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

/**
 *	DataObserver
 *  Attachable to **DeviceInstance** objects in order to receive data transmissions of a particular device.
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface DataObserver
{
    /**
     * OnData is called whenever new binary data (files, buffers) has been received.
     * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
     *
     * @param fileData                  The corresponding file object of type FileInstance
     * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
     */
    void OnData ( @Nullable FileInstance fileData, @FileInfoFlag.Value int FILE_INFO_ATTR_changed );
}
