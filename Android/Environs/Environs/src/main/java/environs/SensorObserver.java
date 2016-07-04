package environs;
/**
 * SensorObserver
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
 *	SensorObserver
 *  Attachable to **DeviceInstance** objects in order to receive sensor data transmissions of a particular device.
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface SensorObserver
{
    /**
     * OnSensorData is called whenever new sensor data has been received.
     *
     * @param sensorFrame     The corresponding SensorFrame of sensor data
     */
    void OnSensorData ( @Nullable SensorFrame sensorFrame );
}
