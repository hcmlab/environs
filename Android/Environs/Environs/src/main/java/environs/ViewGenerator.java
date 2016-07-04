package environs;
/**
 * ViewGenerator
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

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

/**
 *	ViewGenerator
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface ViewGenerator
{
    //* @param inflater      The LayoutInflater used by the ListView.
    void init(LayoutInflater inflater, int layout_id, int view_id);


    /**
     * getView is called by the DeviceList instance in order to build a View for the BaseAdater.
     * This interface is intended to be used for application context objects of DeviceInstance objects.
     * If the appContext1 object reference implements this interface, then application can customize
     * the row views within a ListView.
     *
     * @param position      The position of the corresponding BaseAdapter call.
     * @param device        The DeviceInstance object at the requested position.
     * @param convertView   The convertView of the corresponding BaseAdapter call.
     * @param parent        The parent of the corresponding BaseAdapter call.
     *
     * @return  The generated view in case of success. null in case of errors.
     */
    View getView(int position, DeviceInstance device, View convertView, ViewGroup parent);
}
