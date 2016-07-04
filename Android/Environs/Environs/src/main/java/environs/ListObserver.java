package environs;
/**
 * ListObserver
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
import java.util.ArrayList;

/**
 *	ListObserver
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface ListObserver {
    /**
     * Attachable to **DeviceList** objects in order to receive list changes of a particular DeviceList.
     *
     * Change notifications of object properties are conducted through the implementation of
     * the environs.ListObserver interface.
     *
     * The execution context is not guaranteed to be the UI thread context!
     * List changes are notified through onListChanged()
     * Device instance changes are notified through onItemChanged()
     *
     */

    /**
     * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
     *
     * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
     * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
     */
    void OnListChanged ( @Nullable ArrayList<DeviceInstance> vanished, @Nullable ArrayList<DeviceInstance> appeared );
}
