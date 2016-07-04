package environs;
/**
 * GCM
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
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;

import com.google.android.gcm.GCMBaseIntentService;
/*
import android.app.Notification;
import android.app.NotificationManager;
*/
import environs.R;

/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 * GCM class, 
 */
@SuppressWarnings("deprecation")
public class GCM extends GCMBaseIntentService 
{ 
	private static String className = "GCM";
	
    public GCM() {
        super( Utils.PROJECT_ID);
    }
 
    /**
     * Method called on device registered
     * @param context           The context.
     * @param registrationId    The registration id.
     **/
    @Override
    protected void onRegistered(Context context, String registrationId)
    {
        Utils.Log2 ( className, "onRegistered: Device registered: regId = " + registrationId );
        
        // Save id to our environs
        Environs.GCMRegID(registrationId);
        
//        Utils.register(context, registrationId);
    }
 
    /**
     * Method called on device un registred
     * @param context           The context.
     * @param registrationId    The registration id.
     * */
    @Override
    protected void onUnregistered(Context context, String registrationId)
    {
    	Utils.Log2 ( className, "onUnregistered: Device unregistered" );

        //C.unregister(context, registrationId);
    }
 
    /**
     * Method called on Receiving a new message
     * @param context           The context.
     * @param intent           The intent.
     * */
    @Override
    protected void onMessage(Context context, Intent intent)
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "onMessage: Received message" );

        String message = intent.getExtras().getString("msg");
        
    	Utils.Log1 ( className, "onMessage: Received message=" + message);

    	// Check whether the device is in standby or not
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        boolean isScreenOn = pm.isScreenOn();
    	
        if (!isScreenOn)
            WakeLocker.acquire(getApplicationContext());
        
    	// Check wether we need to unlock        
        KeyguardManager km = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        boolean locked = km.inKeyguardRestrictedInputMode();
        
        if (locked) {
            KeyguardLock mLock = km.newKeyguardLock("Environs");
            mLock.disableKeyguard();
        }
        
        /// TODO
        /*if (AndroidEnvirons.instance == null) {
        	Intent sintent = new Intent(context, AndroidEnvirons.instance.mainActivity.class);
        	sintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        	sintent.putExtra("msg", message);
        	startActivity(sintent);
        }
        else
        	AndroidEnvirons.instance.onPushMessage(message);
        */
        
        // notifies user
        generateNotification(0, context, message);
        
        if (!isScreenOn)
            WakeLocker.release();
    }
 
    /**
     * Method called on receiving a deleted message
     * @param context           The context.
     * @param total           The total count.
     * */
    @Override
    protected void onDeletedMessages(Context context, int total)
    {
    	Utils.Log2 ( className, "onDeletedMessages: Received deleted messages notification");
        
        String message = "Deleted " + total + " messages";
        
        // notifies user
        generateNotification(0, context, message);
    }
 
    /**
     * Method called on Error
     * @param context           The context.
     * @param errorId           The error id.
     * */
    @Override
    public void onError(Context context, String errorId)
    {
    	Utils.LogE ( className, "onError: Received error: " + errorId);

    }

    /**
     * Method called on recoverable Error
     * @param context           The context.
     * @param errorId           The error id.
     * @return success
     * */
    @Override
    protected boolean onRecoverableError(Context context, String errorId)
    {
    	Utils.LogW ( className, "onRecoverableError: Received recoverable error: " + errorId);


        return super.onRecoverableError(context, errorId);
    }
 
    /**
     * Issues a notification to inform the user that server has sent a message.
     * @param context           The context.
     * @param message           The message to send.
     */
    private static void generateNotification(int hInst, Context context, String message) {
        int icon = R.drawable.hcm;
        long when = System.currentTimeMillis();
        
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notification = new Notification(icon, message, when);
 
        String title = context.getString(R.string.app_name);
 
    	if ( Environs.GetInstance(hInst).GetClient() != null ) {
            Intent notificationIntent = new Intent(context, Environs.GetInstance(hInst).GetClient().getClass());
            // set intent so it does not Start a new activity
            notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP |
                    Intent.FLAG_ACTIVITY_SINGLE_TOP);
            PendingIntent intent =
                    PendingIntent.getActivity(context, 0, notificationIntent, 0);
            //notification.setLatestEventInfo(context, title, message, intent);
    		
    	}
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
 
        // Play default notification sound
        //notification.defaults |= Notification.DEFAULT_SOUND;
 
        // Vibrate if vibrate is enabled
        //notification.defaults |= Notification.DEFAULT_VIBRATE;
        notificationManager.notify(0, notification);      
 
    }
 
}
