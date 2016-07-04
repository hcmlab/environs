package environs.CamPortal;
/**
 *	DeviceListActivity
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
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;

import environs.*;
import environs.CamPortal.R;

/**
 * DeviceListActivity manages the compound view with a device list and a portal view.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class DeviceListActivity extends Activity implements ListObserver, DeviceObserver, PortalObserver
{
    private static final String className = "DeviceListActivity";

    static DeviceListActivity instance;
    static DeviceList adapter;
    static DeviceInstance currentDevice;
    static Menu menu;
    static Environs environs;

    TextView statusText;
    ListView listView;
    static SurfaceView surfaceView;

    static PortalInstance portal;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate ( savedInstanceState );
        setContentView( R.layout.activity_device_list);

        instance = this;

        surfaceView = (SurfaceView) findViewById( R.id.portalSurface);
        if (surfaceView == null)
            return;

        statusText = (TextView) findViewById(R.id.statusText);
        if (statusText == null)
            return;
        Utils.logLevel = 6;

        /// Instantiate the Environs instance to take part in the application environment
        environs = Environs.CreateInstance ( this, "CamPortal", "Environs" );

        if (environs == null) {
            Log.e("CamPortal", "Failed to create an Environs object!");
            return;
        }

        environs.SetUseAuthentication ( true );

        environs.SetUseTouchRecognizer ( "Env-RecGestureBezelTouch", true );
        environs.SetUseTouchRecognizer ( "Env-RecGestureThreeTouch", true );

        Observer observer = new Observer ();

        /// Set our listener implementation to receive callbacks and notification
        environs.AddObserver(observer);

        environs.AddObserverForMessages ( observer );

        /// Start Environs
        environs.Start ( );

        AddListAdapter();
    }


    public void AddListAdapter()
    {

        listView = (ListView) findViewById(R.id.deviceList);
        if (listView == null)
            return;

        // Create the device list
        adapter = environs.CreateDeviceList ( this,
                DeviceClass.All, // for all available devices and
                this, 								// add ourself as an observer
                R.layout.list_item_device, android.R.id.text1);

        // Use the devicelist as an adapter for the ListView
        listView.setAdapter ( adapter );

        // ListView Item Click Listener
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                // Clicked on item (DeviceInstance)
                DeviceInstance device = (DeviceInstance) listView.getItemAtPosition(position);
                if (device == null)
                    return;
                EstablishPortal(device);
            }
        });

        Button disc = (Button) findViewById(R.id.disconnect);
        if ( disc != null ) {

            disc.setOnClickListener(new Button.OnClickListener() {
                public void onClick(View v) {
                    if (adapter == null)
                        return;
                    ArrayList<DeviceInstance> devices = adapter.GetDevices ( );

                    if (devices != null) {
                        for (int i=0; i<devices.size(); i++)
                        {
                            // Get the device that appeared in the list
                            DeviceInstance device = devices.get(i);
                            if (device.isConnected)
                                device.Disconnect ();
                        }
                    }
                }
            });
        }
    }


    void EstablishPortal(final DeviceInstance device)
    {
        new Thread(new Runnable() {
            public void run() {
                if ( currentDevice != null && currentDevice != device && currentDevice.isConnected) {
                    // Call in synchronous mode as we want to wait until the device has disconnected
                    device.Disconnect(Call.Wait);
                }

                // Synchronize concurrent calls to the same DeviceInstance
                synchronized (this) {
                    currentDevice = device;

                    device.async = Call.Wait;

                    if (device.isConnected && device.PortalGetIncoming () != null)
                        return;

                    // Call in synchronous mode as we want to wait until the device has connected
                    // before establishing the portal
                    if (!device.Connect())
                        return;

                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    portal = device.PortalGetOutgoing();
                    if ( portal == null ) {
                        portal = device.PortalRequest(PortalType.Any);
                        if (portal != null) {
                            portal.AddObserver(DeviceListActivity.this);

                            portal.SetRenderSurface(surfaceView, 2.0f, true);

                            portal.startIfPossible = true;
                            portal.Establish(true);
                        }
                    }
                    else portal.Stop();

                    device.async = Call.NoWait;
                }
            }
        }).start();
    }


    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal  The PortalInstance object that represents the incoming request.
     */
    public void OnPortalRequestOrProvided(PortalInstance portal)
    {
        if (portal == null)
            return;

        // A portalInstance may be caused by
        //  (a) An incoming request by another device for an outgoing portal (portal.outgoing == true)
        //      In this case we may accept (add an observer and call Establish())
        //      Establish(true) also checks whether WiFi is available and prompt an accept dialog to the user if we're on mobile data only.
        //      Establish(true) also prompts a dialog to the user to choose a source (front/back facing camera)
        //
        //  (b) An incoming request by another device that want's to provide a portal (it asks us to request a portal from his/her)
        //      In this case we may accept (add an observer, set a render surface (if already available) and call Establish())
        //      Establish(true) also checks whether WiFi is available and prompt an accept dialog to the user if we're on mobile data only.
        //
        //  You may block this call, user your own user dialog, set the portal.portalType by yourself, and call Establish(false).
        //
        portal.AddObserver(DeviceListActivity.instance);

        if (portal.isIncoming())
            portal.SetRenderSurface(DeviceListActivity.surfaceView, 2.0f, true);

        portal.startIfPossible = true;

        portal.Establish(true);
    }


    @Override
    public void OnPortalChanged(PortalInstance portalInstance, int Environs_NOTIFY_)
    {
        if ( portalInstance == null || portalInstance.status == PortalStatus.Disposed )
            return;
    }


    @Override
    public void OnImage(Bitmap bitmap) {

    }


    @Override
    public void onResume() {
        Utils.Log(4, className, "onResume");
        super.onResume();
    }

    @Override
    public void onPause () {
        Utils.Log(4, className, "onPause");
        super.onPause();
    }

    @Override
    public void onDestroy () {
        Utils.Log(4, className, "onDestroy");
        super.onDestroy();

        if (environs != null)
            environs.Stop();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_device_list, menu);
        this.menu = menu;
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.actionSettings:
                return true;

            case R.id.actionEnvStart:
                if (environs == null)
                    break;

                if (environs.GetStatus() >= Status.Initialized)
                    environs.Stop();
                else {
                    environs.Start();
                    AddListAdapter ();
                }
                return true;

            case R.id.actionStopPortal:
                if (portal != null)
                    portal.Stop();
                return true;

            default:
                break;
        }

        return super.onOptionsItemSelected(item);
    }


    @Override
    public void OnDeviceChanged(DeviceInstance sender, int changeFlags) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                adapter.notifyDataSetChanged();
            }
        });
    }


    @Override
    public void OnListChanged(ArrayList<DeviceInstance> vanished, ArrayList<DeviceInstance> appeared)
    {
        if (appeared != null) {
            for (int i=0; i<appeared.size(); i++)
            {
                // Get the device that appeared in the list
                DeviceInstance device = appeared.get(i);
                Utils.Log(1, className, "appeared: " + device.toString() );

                device.AddObserver(this);
            }
        }
    }

    public void ShowStatusMessage(String msg) {
        if (statusText == null)
            return;
        final String text = statusText.getText().toString() + "\n" + msg;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                DeviceListActivity.this.statusText.setText(text);
            }
        });
    }
}
