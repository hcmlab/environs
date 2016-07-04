package environs;
/**
 * EnvironsSensorObserver
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

import android.support.annotation.NonNull;

/**
 *	EnvironsSensorObserver
 *	Attachable to **Environs** objects in order to receive all sensor data transmissions that the Environs instance received.
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public interface EnvironsSensorObserver {
    /**
     * OnSensorData is called whenever new sensor data has been received.
     *
     * @param frame		The sensor data of type SensorFrame.
     */
    void OnSensorData ( @NonNull SensorFrame frame );
}
