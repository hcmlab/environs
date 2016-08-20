package environs;
/**
 * Bluetooth Observer.
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
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.SystemClock;

import java.util.List;
import java.util.Set;

public class BtObserver
{
    private static final String className = "Bt.Observer. . . . . . .";

    BluetoothAdapter adapter;
    Context ctx;
    boolean btThreadRun = false;
    Thread btThread;

    long lastScan	= 0;
    int scanState = 0;

    BroadcastReceiver scanResultsReceiver;
    BroadcastReceiver scanFinishedReceiver;


    public BtObserver ()
    {
    }


    public boolean Init ( Context ctx )
    {
        if (Utils.isDebug) Utils.Log ( 3, className, "Init");

        if (ctx == null) {
            Utils.LogE ( className, "Init: Invalid context!" );
            return false;
        }

        if ( !Environs.useBluetooth ) {
            return false;
        }

        this.ctx = ctx;

        adapter = BluetoothAdapter.getDefaultAdapter ();
        if (adapter == null) {
            Utils.LogE ( className, "Init: Bluetooth service retrieval failed!" );
            return false;
        }

        return true;
    }


    public boolean Start ()
    {
        if (Utils.isDebug) Utils.Log ( 3, className, "Start");

        if ( !Environs.useBluetooth ) {
            return false;
        }

        try
        {
            if ( !adapter.isEnabled () )
            {
                if (Utils.isDebug) Utils.Log ( 1, className, "Start: Trying to turn on Bluetooth ...");

                if ( Environs.useBluetoothAdmin )
                    adapter.enable ();
                else if ( Environs.GetUseBtObserverN () )
                {
                    Intent enableBtIntent = new Intent ( BluetoothAdapter.ACTION_REQUEST_ENABLE );

                    Activity act = ((Activity )ctx);
                    act.startActivityForResult( enableBtIntent, 42);
                }
            }

            /*if ( adapter.getScanMode () != BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE )
            {
                Intent enableBtIntent = new Intent ( BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE );

                Activity act = ((Activity )ctx);
                act.startActivityForResult( enableBtIntent, 42);
            }*/

            synchronized ( this )
            {
                if ( btThread != null)
                    return true;

                btThreadRun = true;

                btThread = new Thread ( new Runnable ( ) {
                    public void run ( ) {
                        Thread_BtObserver ();
                    }
                } );
                btThread.start ( );

                scanResultsReceiver = new BroadcastReceiver ()
                {
                    @Override
                    public void onReceive(Context c, Intent intent)
                    {

                        String action = intent.getAction();

                        if (BluetoothDevice.ACTION_FOUND.equals(action))
                        {
                            BluetoothDevice device = intent.getParcelableExtra ( BluetoothDevice.EXTRA_DEVICE );
                            if ( device != null )
                            {
                                int rssi = intent.getShortExtra ( BluetoothDevice.EXTRA_RSSI, Short.MIN_VALUE );

                                if (Utils.isDebug) Utils.Log ( 6, className, "OnReceive: Bssid [" + device.getAddress () + "]\tName [" + device.getName () + "]\trssi [" + rssi + "]");

                                Environs.BtUpdateWithColonMacN ( device.getAddress (), device.getName (), rssi,
                                                                 device.getBluetoothClass ().getMajorDeviceClass (),
                                                                 0, 0, scanState );

                                scanState++;
                            }
                        }
                    }
                };

                if (Utils.isDebug) Utils.Log ( 3, className, "Start: Registering results receiver ...");

                ctx.registerReceiver ( scanResultsReceiver, new IntentFilter ( BluetoothDevice.ACTION_FOUND ) );


                scanFinishedReceiver = new BroadcastReceiver ()
                {
                    @Override
                    public void onReceive(Context c, Intent intent)
                    {

                        String action = intent.getAction();

                        if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action))
                        {
                            if ( scanState > 1 ) {
                                if (Utils.isDebug) Utils.Log ( 6, className, "Thread: Finish scan frame.");

                                Environs.BtUpdateWithColonMacN ( null, null, 0, 0, 0, 0, 0 );
                            }
                            scanState = 0;

                            Set<BluetoothDevice> devices = adapter.getBondedDevices ();

                            if ( devices.size () > 0 )
                            {
                                for ( BluetoothDevice device : devices ) {
                                    if (Utils.isDebug) Utils.Log ( 4, className, "Thread: Bssid [" + device.getAddress () + "]\tName [" + device.getName () + "]\trssi [" + 256 + "]");

                                    Environs.BtUpdateWithColonMacN ( device.getAddress (), device.getName (), 256, 2, device.getBluetoothClass ().getMajorDeviceClass (), 0, 0 );
                                }
                            }

                            synchronized ( this ) {
                                this.notify ();
                            }
                        }
                    }
                };

                if (Utils.isDebug) Utils.Log ( 3, className, "Start: Registering finished receiver ...");

                ctx.registerReceiver ( scanFinishedReceiver, new IntentFilter ( BluetoothAdapter.ACTION_DISCOVERY_FINISHED ) );




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

        btThreadRun = false;

        Thread thread = btThread;

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
            btThread = null;

            if ( scanResultsReceiver != null ) {
                if (Utils.isDebug) Utils.Log ( 3, className, "Stop: Unregistering results receiver ...");

                ctx.unregisterReceiver ( scanResultsReceiver );
                scanResultsReceiver = null;
            }

            if ( scanFinishedReceiver != null ) {
                if (Utils.isDebug) Utils.Log ( 3, className, "Stop: Unregistering finished receiver ...");

                ctx.unregisterReceiver ( scanFinishedReceiver );
                scanFinishedReceiver = null;
            }
        }
    }


    void Thread_BtObserver ()
    {
        long lastCheck = 0;
        boolean doScan = false;

        Environs.BtUpdateWithColonMacN ( adapter.getAddress () , adapter.getName (), 42, 1, 0, 0, 0 );

        while ( btThreadRun )
        {
            if ( doScan ) {
                if ( !adapter.isDiscovering () )
                {
                    if (Utils.isDebug) Utils.Log ( 6, className, "Thread: Not discovering.");

                    if ( scanState > 1 ) {
                        if (Utils.isDebug) Utils.Log ( 6, className, "Thread: Finish scan frame.");

                        Environs.BtUpdateWithColonMacN ( null, null, 0, 0, 0, 0, 0 );
                    }

                    scanState = 1;

                    Environs.GetBts();

                    if (Environs.useBluetoothAdmin) {
                        if (Utils.isDebug) Utils.Log ( 6, className, "Thread: Invoke scan ...");
                        adapter.startDiscovery ( );
                    }
                }
                else {
                    if (Utils.isDebug) Utils.Log ( 6, className, "Thread: Still discovering.");
                }

                doScan = false;
                lastScan = SystemClock.uptimeMillis();
            }


            long waitTime = Environs.ENVIRONS_BT_OBSERVER_INTERVAL_MOBILE_MIN;

            while ( btThreadRun ) {

                //Environs.GetBts ();

                synchronized ( this ) {
                    try {
                        this.wait ( waitTime );
                    } catch ( InterruptedException e ) {
                        e.printStackTrace ( );
                    }
                }

                long now = SystemClock.uptimeMillis();
                long diff = now - lastCheck;

                if ( diff < Environs.ENVIRONS_BT_OBSERVER_INTERVAL_MOBILE_CHECK_MIN ) {
                    waitTime = ( Environs.ENVIRONS_BT_OBSERVER_INTERVAL_MOBILE_CHECK_MIN + 30 ) - diff;
                    continue;
                }

                if ( ( now - lastScan ) > Environs.ENVIRONS_BT_OBSERVER_INTERVAL_MOBILE_MIN )
                {
                    doScan = true;
                }
                break;
            }
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "Thread_BtObserver: done" );

        synchronized ( this ) {
            btThread = null;
        }
    }
}
