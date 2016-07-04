package environs;
/**
 * StartStoper
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
import android.app.Activity;
import android.content.IntentFilter;
import android.net.ConnectivityManager;

/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 * StartStoper class, 
 */
class StartStoper extends Thread implements Runnable
{
	private final static String className = "StartStoper. . . . . . .";

	Environs env;
	int hEnvirons;

	public void run() {
		{
			if ( Utils.isDebug ) Utils.Log ( 4, className, "Thread started" );

			try {
				boolean notifyApp = ( env.observers.size ( ) > 0 );

				Activity act = env.GetClient ( );

				if ( act == null ) {
					Utils.LogE ( className, "Invalid app initialization! No instance of Environs available or Environs not initialized yet!" );
					return;
				}

				int deviceID = env.GetDeviceID ( );
				if ( deviceID == 0 ) {
					// Try loading deviceID from previous installation
					int recentID = Utils.LoadIDFromStorage ( );
					if ( recentID != 0 )
						env.SetDeviceID ( recentID );
				}

				if ( !Environs.HasDeviceUIDN ( ) )
					Environs.SetDeviceUIDN ( Utils.GetAppID ( act ) );

				String deviceName = Utils.buildDeviceName ( act );
				if ( deviceName != null )
					Environs.SetDeviceNameN ( deviceName );

				if ( Utils.isDebug ) Utils.Log ( 6, className, "GetStatus" );
				int status = env.GetStatus ( );
				if ( status >= Environs.STATUS_STARTED ) {
					Activity context = env.GetClient ( );
					if ( context != null ) {
						context.unregisterReceiver ( Environs.broadcasts );
					}

					if ( Utils.isDebug ) Utils.Log ( 6, className, "Stop" );
					env.Stop ( );
					return;
				}

				if ( Utils.isDebug ) Utils.Log ( 6, className, "NOTIFY_START_ENABLING_WIFI" );
				// At first turn on wifi
				if ( notifyApp )
					Environs.BridgeForNotify ( hEnvirons, -1, Notify.Environs.StartEnablingWifi, Source.Platform, null, 0 );

				if ( !Utils.EnableWiFi ( env.hEnvirons ) ) {
					if ( notifyApp )
						Environs.BridgeForNotify ( hEnvirons, -1, Notify.Environs.WifiFailed, Source.Platform, null, 0 );
					//return "Error";
				}

				if ( Utils.isDebug ) Utils.Log ( 6, className, "Init1" );
				if ( Environs.InitN ( env.hEnvirons ) <= 0 ) {
					return;
				}

				env.SetPorts ( Utils.dataPort, Utils.controlPort );
				if ( env.GetUseCustomMediator ( ) )
					env.SetMediator ( env.GetMediatorIP ( ), env.GetMediatorPort ( ) );

				// Start device instance notifier thread
				if ( env.deviceInstanceNotifier == null ) {
					try {
						env.deviceInstanceNotifier =
								new Thread ( new Runnable ( ) {
									public void run ( ) {
										DeviceInstance.NotifierThread ( env );
									}
								} );

						env.deviceInstanceNotifierRun = true;
						env.deviceInstanceNotifier.start ( );
					} catch ( Exception ex ) {
						ex.printStackTrace ( );
						return;
					}
				}


				if ( env.listNotifierThread == null ) {
					if ( Utils.isDebug )
						Utils.Log ( 6, className, "Starting DeviceList notifier thread" );

					env.listNotifierThreadRun = true;

					env.listNotifierThread = new Thread ( new Runnable ( ) {
						public void run ( ) {
							DeviceList.NotifierThread ( env );
						}
					} );
					env.listNotifierThread.start ( );
				}


				if ( env.listUpdateThread == null ) {
					if ( Utils.isDebug )
						Utils.Log ( 6, className, "Starting DeviceList update thread" );

					env.listUpdateThreadRun = true;

					env.listUpdateThread = new Thread ( new Runnable ( ) {
						public void run ( ) {
							DeviceList.ListUpdateThread ( env );
						}
					} );
					env.listUpdateThread.start ( );
				}


				if ( Environs.StartN ( env.hEnvirons ) == 0 ) {
					Utils.LogE ( className, "Failed to Start network layer!" );
					return;
				}

				status = env.GetStatus ( );
				if ( status < Environs.STATUS_STARTED ) {
					Utils.LogE ( className, "Failed network layer stopped! status [" + status + "]" );
					return;
				}

				IntentFilter filter = new IntentFilter ( );
				filter.addAction ( ConnectivityManager.CONNECTIVITY_ACTION );

				act.registerReceiver ( Environs.broadcasts, filter );

				if ( Environs.GetUseWifiObserverN () ) {
					if ( !Environs.wifiObserver.Init ( act ) )
						Utils.LogE ( className, "InitInstance: Initialize of wifi observer failed!");
					else if ( !Environs.wifiObserver.Start () )
						Utils.LogE ( className, "InitInstance: Start of wifi observer failed!");
				}

				Environs.StartSensorListeningAllN ( hEnvirons );
				env.started = true;

				if ( Utils.isDebug ) Utils.Log ( 6, className, "successfully started." );
			} catch ( Exception e ) {
				e.printStackTrace ( );
			}
		}
	}
}
