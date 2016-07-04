package environs;
/**
 * WakeLocker
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
import android.content.Context;
import android.os.PowerManager;

/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 * WakeLocker abstract class, 
 */
abstract class WakeLocker {
	private static PowerManager.WakeLock wakeLock = null;
 
	@SuppressLint("Wakelock")
	@SuppressWarnings("deprecation")
	public static void acquire(Context context) {
        if (wakeLock != null)
        	wakeLock.release();
 
        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        wakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK |
                PowerManager.ACQUIRE_CAUSES_WAKEUP |
                PowerManager.ON_AFTER_RELEASE, "WakeLock");
        wakeLock.acquire();
    }
 
    public static void release() {
        if (wakeLock != null)
        	wakeLock.release();
        wakeLock = null;
    }
}
