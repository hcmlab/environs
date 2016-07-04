package environs;
/**
 * Portal base class. A platform layer portal source.
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
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 * Portal base class. A platform layer portal source.
 */
@SuppressWarnings("deprecation")
@SuppressLint("NewApi")
public abstract class PortalBase {

    abstract boolean initPortal () throws Exception;


    abstract boolean completeCameraInit() throws Exception;
}
