package environs;
/**
 * Wifi Observer.
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
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.SystemClock;

import java.util.List;

public class WifiObserver
{
    private static final String className = "WifiObserver . . . . . .";

    WifiManager wifi;
    Context ctx;
    boolean wifiThreadRun = false;
    Thread wifiThread;

    long lastScan	= 0;

    BroadcastReceiver scanResultsReceiver;


    public WifiObserver ()
    {
    }


    public boolean Init ( Context ctx )
    {
        if (Utils.isDebug) Utils.Log ( 1, className, "Init");

        if (ctx == null) {
            Utils.LogE ( className, "Init: Invalid context!" );
            return false;
        }

        this.ctx = ctx;

        wifi = (WifiManager) ctx.getSystemService ( Context.WIFI_SERVICE );
        if (wifi == null) {
            Utils.LogE ( className, "Init: Wifi service retrieval failed!" );
            return false;
        }

        return true;
    }


    public boolean Start ()
    {
        if (Utils.isDebug) Utils.Log ( 1, className, "Start");

        try
        {
            if ( !wifi.isWifiEnabled () )
            {
                if (Utils.isDebug) Utils.Log ( 1, className, "Start: Trying to turn on Wifi ...");

                wifi.setWifiEnabled ( true );
            }

            synchronized ( this )
            {
                if (wifiThread != null)
                    return true;

                wifiThreadRun = true;

                wifiThread = new Thread ( new Runnable ( ) {
                    public void run ( ) {
                        Thread_WifiObserver ();
                    }
                } );
                wifiThread.start ( );

                scanResultsReceiver = new BroadcastReceiver ()
                {
                    @Override
                    public void onReceive(Context c, Intent intent)
                    {
                        synchronized ( this ) {
                            this.notify ();
                        }
                        //results = wifi.getScanResults();
                        //size = results.size();
                    }
                };

                if (Utils.isDebug) Utils.Log ( 1, className, "Start: Registering broadcast receiver ...");

                ctx.registerReceiver ( scanResultsReceiver, new IntentFilter ( WifiManager.SCAN_RESULTS_AVAILABLE_ACTION ) );
            }

            return true;
        }
        catch ( Exception ex ) {
            ex.printStackTrace ();
        }

        Utils.LogE ( className, "Start: Failed!" );
        return false;
    }


    public void Stop ()
    {
        if (Utils.isDebug) Utils.Log ( 1, className, "Stop");

        wifiThreadRun = false;

        Thread thread = wifiThread;

        if (thread != null)
        {
            synchronized ( this ) {
                this.notify ();
            }

            try {
                if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for thread ...");

                thread.join ();
            } catch ( InterruptedException e ) {
                e.printStackTrace ( );
            }
        }

        if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for thread done.");

        synchronized ( this ) {
            wifiThread = null;

            if ( scanResultsReceiver != null ) {
                if (Utils.isDebug) Utils.Log ( 1, className, "Start: Unregistering broadcast receiver ...");

                ctx.unregisterReceiver ( scanResultsReceiver );
                scanResultsReceiver = null;
            }
        }
    }


    int GetWiFiChannel ( long centerFreq )
    {
        long f1 = ( centerFreq % 2412000 ) / 1000;
        return ( int ) ( ( f1 / 5 ) + 1 );
    }


    void Thread_WifiObserver ()
    {
        long lastCheck = 0;
        boolean doScan = true;

        while ( wifiThreadRun )
        {
            List<ScanResult > networks;

            if ( doScan ) {
                wifi.startScan();
                doScan = false;
            }

            networks = wifi.getScanResults();

            int size = networks.size ();
            if ( size > 0 )
            {
                for ( int i = 0; i < size; i++ )
                {
                    ScanResult network = networks.get ( i );

                    int encrypt = 0;
                    if ( network.capabilities.contains ( "WEP" ) )
                        encrypt = 1;
                    else if ( network.capabilities.contains ( "WPA" ) )
                        encrypt = 2;
                    int channel = GetWiFiChannel ( network.frequency );

                    int state = (i + 1);
                    if ( state == size )
                        state = 0;

                    Environs.WiFiUpdateWithColonMacN ( network.BSSID, network.SSID, network.level, channel, encrypt, state );

                    /*if (Utils.isDebug) Utils.Log ( 1, className, "WifiObserver: SSID [ " +
                            network.SSID + " ]\tBSSID [ " + network.BSSID +
                            " ]\trssi [ " + network.level + " ]\tchannel [ " + channel + " ]\tencrypt [ " + encrypt + " ]" + network.capabilities );*/
                }
            }

            long waitTime = Environs.ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_MIN;

            while ( wifiThreadRun ) {
                synchronized ( this ) {
                    try {
                        this.wait ( waitTime );
                    } catch ( InterruptedException e ) {
                        e.printStackTrace ( );
                    }
                }

                long now = SystemClock.uptimeMillis();
                long diff = now - lastCheck;

                if ( diff < Environs.ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_CHECK_MIN ) {
                    waitTime = ( Environs.ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_CHECK_MIN + 30 ) - diff;
                    continue;
                }

                if ( ( now - lastScan ) > Environs.ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_MIN )
                {
                    lastScan = SystemClock.uptimeMillis();
                    doScan = true;
                }
                break;
            }
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "Thread_WifiObserver: done" );

        synchronized ( this ) {
            wifiThread = null;
        }
    }
}
