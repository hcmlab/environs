package environs;
/**
 *	ActivityTracker
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
import android.app.Activity;
import android.app.Application.ActivityLifecycleCallbacks;
import android.os.Bundle;

/** Tracks activity states in order to support power savings through minimizing Environs activities
 * 	if the application is not visible.
 * 	Note: This feature requires API 14 ( Android 4.0 ).
 * 
 * @author chi-tai
 * 
 */
@SuppressLint("NewApi")
public class ActivityTracker implements ActivityLifecycleCallbacks {
	private static final String className = "ActivityTracker. . . . .";
	
	private int resumed = 0;
	private int paused = 0;

	@Override
	public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityCreated");
	}

	@Override
	public void onActivityStarted(Activity activity) {
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityStarted");
	}

	private void UpdateAppStatus ()
	{
		if (resumed <= paused)
			if (Utils.isDebug) Utils.Log ( 4, className, "UpdateAppStatus: NO ACTIVITY ENABLED!");
		/*if ( resumed > paused )
			Environs.SetAppStatus ( Environs.APP_STATUS_ACTIVE );
		else
			Environs.SetAppStatus ( Environs.APP_STATUS_SLEEPING );
		*/
	}
	
	@Override
	public void onActivityResumed(Activity activity) {
		resumed++;
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityResumed: " + (resumed > paused) + " " + activity.getLocalClassName() + " ACTIVE");

		synchronized (Environs.className) {
			Environs.currentActivity = activity;
		}
    	UpdateAppStatus ();
	}

	@Override
	public void onActivityPaused(Activity activity) {
		paused++;
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityPaused: " + (resumed > paused) + " " + activity.getLocalClassName() + " DISABLED");
    	
    	UpdateAppStatus ();
	}

	@Override
	public void onActivityStopped(Activity activity) {
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityStopped");
		UpdateAppStatus ();
	}

	@Override
	public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivitySaveInstanceState");
	}

	@Override
	public void onActivityDestroyed(Activity activity) {
		if (Utils.isDebug) Utils.Log ( 4, className, "onActivityDestroyed");

		synchronized (Environs.className) {
			if (Environs.currentActivity == activity)
				Environs.currentActivity = null;
		}
		UpdateAppStatus ();
	}
}
