package environs;
/**
 *	BroadcastReceiver
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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;

class Broadcasts extends BroadcastReceiver
{
	private static final String className = "Broadcasts . . . . . . .";

	static @NetworkConnection.Value int networkStatus = NetworkConnection.NoNetwork;

	@NetworkConnection.Value
 	static int getNetStat ( NetworkInfo networkInfo )  
 	{
		int netStat = NetworkConnection.Unknown;
		
		if (networkInfo != null) {
			boolean isConnected = networkInfo.getState() == State.CONNECTED;
			if (!isConnected)
				return NetworkConnection.NoNetwork;

			int netType = networkInfo.getType();

			switch (netType) {
			case ConnectivityManager.TYPE_ETHERNET:
	       		 netStat = NetworkConnection.LAN;
	       		 break;
			case ConnectivityManager.TYPE_WIFI:
	       		 netStat = NetworkConnection.WiFi;
	       		 break;
			default:
	       		 netStat = NetworkConnection.MobileData;
	       		 break;
			}
		}
		return netStat;
 	}
 	
	@Override
	public void onReceive( Context context, Intent intent )
	{
		int netStat = NetworkConnection.Unknown;
		String action = intent.getAction();

        if ( !action.equals(ConnectivityManager.CONNECTIVITY_ACTION) ) {
        	 Utils.LogW ( className, "onReceive: called with " + intent);
             return;
        }

        //boolean noConnectivity = intent.getBooleanExtra(ConnectivityManager.EXTRA_NO_CONNECTIVITY, false);

         /*if (noConnectivity) {
             mState = State.NOT_CONNECTED;
         } else {
             mState = State.CONNECTED;
         }
         */
         @SuppressWarnings("deprecation")
		NetworkInfo networkInfo = intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
         if (networkInfo != null) { 
             netStat = getNetStat ( networkInfo );

        	 //Utils.Log( className, "onReceive: by static flag - networkInfo = [" + networkInfo
             //            + "] noConn = [" + noConnectivity + "]");
         }
         
         networkStatus = netStat;

         Environs.UpdateNetworkStatus(netStat);
	}
}
