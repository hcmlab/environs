package environs;
/**
 * ObserverMessageContext
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
public class ObserverDataContext
{
    /** The objID of the sender device. */
    public int              objID;
    /** The nativeID of the sender device. */
    public int              nativeID;
    /** The type of this message. */
    public int              type;
    /** A fileID that was attached to the buffer. */
    public int              fileID;
    /** A descriptor that was attached to the buffer. */
    public String			descriptor;
    /** The size of the data buffer. */
    public int              size;
}
