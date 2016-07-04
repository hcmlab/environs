package environs;
/**
 * PortalObserver
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
import android.graphics.Bitmap;
import android.support.annotation.Nullable;

/**
 *	PortalObserver
 *  Attachable to **PortalInstance** objects in order to receive changes of a particular interactive portal.
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface PortalObserver {
    /**
     * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
     *
     * @param portal        The PortalInstance object.
     * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
     */
    void OnPortalChanged ( @Nullable PortalInstance portal, @Notify.Portal.Value int Environs_NOTIFY_PORTAL_);


    /**
     * OnImage is called whenever new binary data (files, buffers) has been received.
     * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
     * @param bitmap	One bitmap from an ongoing bitmap stream.
     */
    void OnImage ( Bitmap bitmap );
}
