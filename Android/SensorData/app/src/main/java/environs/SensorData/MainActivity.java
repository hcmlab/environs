package environs.SensorData;
/**
 *	MainActivity
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
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.Switch;

import java.util.ArrayList;
import environs.*;

/**
 * MainActivity.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class MainActivity extends Activity implements ListObserver, DeviceObserver, EnvironsSensorObserver
{
    private static final String className = "MainActivity";

    static MainActivity instance;

    static Switch switchAccel;
    static Switch switchAccelLinear;
    static Switch switchMagnetic;
    static Switch switchGyroscope;
    static Switch switchOrientation;
    static Switch switchLocation;
    static Switch switchLight;

    static Switch switchRotation;
    static Switch switchPressure;
    static Switch switchHumidity;
    static Switch switchProximity;

    ListView listView;
    static DeviceList adapter;
    static DeviceInstance currentDevice;
    static Environs environs;

    static boolean enableSensorAccelerometer = true;
    static boolean enableSensorAccelerometerLinear = false;
    static boolean enableSensorMagneticField = true;
    static boolean enableSensorGyroscope = true;
    static boolean enableSensorOrientation = false;
    static boolean enableSensorLocation = false;
    static boolean enableSensorLight = false;

    static boolean enableSensorRotation = false;
    static boolean enableSensorPressure = false;
    static boolean enableSensorHumidity = false;
    static boolean enableSensorProximity = false;


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        instance = this;

        switchAccel = (Switch) findViewById(R.id.switchAccel);
        if (switchAccel == null)
            return;
        switchAccel.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                enableSensorAccelerometer = ChangeSensorEventState ( SensorType.Accelerometer, isChecked );
            }
        });

        switchAccelLinear = (Switch) findViewById(R.id.switchAccelLinear);
        if (switchAccelLinear == null)
            return;
        switchAccelLinear.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                enableSensorAccelerometerLinear = ChangeSensorEventState ( SensorType.MotionGravityAcceleration, isChecked );
            }
        });

        switchMagnetic = (Switch) findViewById(R.id.switchMagnetic);
        if (switchMagnetic == null)
            return;
        switchMagnetic.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                enableSensorMagneticField = ChangeSensorEventState ( SensorType.MagneticField, isChecked );
            }
        });

        switchGyroscope = (Switch) findViewById(R.id.switchGyroscope);
        if (switchGyroscope == null)
            return;
        switchGyroscope.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                enableSensorGyroscope = ChangeSensorEventState ( SensorType.Gyroscope, isChecked );
            }
        });

        switchOrientation = (Switch) findViewById(R.id.switchOrientation);
        if (switchOrientation == null)
            return;
        switchOrientation.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorOrientation = ChangeSensorEventState ( SensorType.Orientation, isChecked );
            }
        } );

        switchLocation = (Switch) findViewById(R.id.switchLocation);
        if (switchLocation == null)
            return;
        switchLocation.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorLocation = ChangeSensorEventState ( SensorType.Location, isChecked );
            }
        } );

        switchLight = (Switch) findViewById(R.id.switchLight);
        if (switchLight == null)
            return;
        switchLight.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorLight = ChangeSensorEventState ( SensorType.Light, isChecked );
            }
        } );

        switchRotation = (Switch) findViewById(R.id.switchRotation);
        if (switchRotation == null)
            return;
        switchRotation.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorRotation = ChangeSensorEventState ( SensorType.MotionAttitudeRotation, isChecked );
            }
        } );

        switchPressure = (Switch) findViewById(R.id.switchPressure);
        if (switchPressure == null)
            return;
        switchPressure.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorPressure = ChangeSensorEventState ( SensorType.Altimeter, isChecked );
            }
        } );

        switchHumidity = (Switch) findViewById(R.id.switchHumidity);
        if (switchHumidity == null)
            return;
        switchHumidity.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorHumidity = ChangeSensorEventState ( SensorType.Humidity, isChecked );
            }
        } );

        switchProximity = (Switch) findViewById(R.id.switchProximity);
        if (switchProximity == null)
            return;
        switchProximity.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorProximity = ChangeSensorEventState ( SensorType.Proximity, isChecked );
            }
        } );


        /// Create the Environs instance and take part in the given application environment
        environs = Environs.CreateInstance(this, "SensorData", "Environs" );

        if (environs == null) {
            Log.e("SensorData", "Failed to create an Environs object!");
            return;
        }

        environs.AddObserver ( new Observer ( ) );
        environs.AddObserverForSensorData ( this );

        /// Start Environs
        environs.Start ( );

        if ( !environs.IsSensorAvailable ( SensorType.Accelerometer ) ) {
            switchAccel.setEnabled ( false ); switchAccel.setChecked ( false ); enableSensorAccelerometer = false;
        }
        else {
            switchAccel.setChecked ( enableSensorAccelerometer );

            environs.SetSensorEvent ( SensorType.Accelerometer, enableSensorAccelerometer );
        }

        if ( !environs.IsSensorAvailable ( SensorType.MotionGravityAcceleration ) ) {
            switchAccelLinear.setEnabled ( false ); switchAccelLinear.setChecked ( false ); enableSensorAccelerometerLinear = false;
        }
        else {
            switchAccelLinear.setChecked ( enableSensorAccelerometerLinear );

            environs.SetSensorEvent ( SensorType.MotionGravityAcceleration, enableSensorAccelerometerLinear );
        }

        if ( !environs.IsSensorAvailable ( SensorType.MagneticField ) ) {
            switchMagnetic.setEnabled ( false ); switchMagnetic.setChecked ( false ); enableSensorMagneticField = false;
        }
        else {
            switchMagnetic.setChecked ( enableSensorMagneticField );

            environs.SetSensorEvent ( SensorType.MagneticField, enableSensorMagneticField );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Gyroscope ) ) {
            switchGyroscope.setEnabled ( false ); switchGyroscope.setChecked ( false ); enableSensorGyroscope = false;
        }
        else {
            switchGyroscope.setChecked ( enableSensorGyroscope );

            environs.SetSensorEvent ( SensorType.Gyroscope, enableSensorGyroscope );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Orientation ) ) {
            switchOrientation.setEnabled ( false ); switchOrientation.setChecked ( false ); enableSensorOrientation = false;
        }
        else {
            switchOrientation.setChecked ( enableSensorOrientation );

            environs.SetSensorEvent ( SensorType.Orientation, enableSensorOrientation );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Location ) ) {
            switchLocation.setEnabled ( false ); switchLocation.setChecked ( false ); enableSensorLocation = false;
        }
        else {
            switchLocation.setChecked ( enableSensorLocation );

            environs.SetSensorEvent ( SensorType.Location, enableSensorLocation );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Light ) ) {
            switchLight.setEnabled ( false ); switchLight.setChecked ( false ); enableSensorLight = false;
        }
        else {
            switchLight.setChecked ( enableSensorLight );

            environs.SetSensorEvent ( SensorType.Light, enableSensorLight );
        }

        if ( !environs.IsSensorAvailable ( SensorType.MotionAttitudeRotation ) ) {
            switchRotation.setEnabled ( false ); switchRotation.setChecked ( false ); enableSensorRotation = false;
        }
        else {
            switchRotation.setChecked ( enableSensorRotation );

            environs.SetSensorEvent ( SensorType.MotionAttitudeRotation, enableSensorRotation );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Altimeter ) ) {
            switchPressure.setEnabled ( false ); switchPressure.setChecked ( false ); enableSensorPressure = false;
        }
        else {
            switchPressure.setChecked ( enableSensorPressure );

            environs.SetSensorEvent ( SensorType.Altimeter, enableSensorPressure );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Humidity ) ) {
            switchHumidity.setEnabled ( false ); switchHumidity.setChecked ( false ); enableSensorHumidity = false;
        }
        else {
            switchHumidity.setChecked ( enableSensorHumidity );

            environs.SetSensorEvent ( SensorType.Humidity, enableSensorHumidity );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Proximity ) ) {
            switchProximity.setEnabled ( false ); switchProximity.setChecked ( false ); enableSensorProximity = false;
        }
        else {
            switchProximity.setChecked ( enableSensorProximity );

            environs.SetSensorEvent ( SensorType.Proximity, enableSensorProximity );
        }

        AddListAdapter();
    }


    private boolean ChangeSensorEventState(final @SensorType.Value int sensorType, final boolean state)
    {
        new Thread(new Runnable() {
            public void run(){
                environs.SetSensorEvent ( sensorType, state );

                if (adapter == null)
                    return;

                ArrayList<DeviceInstance> devices = adapter.devices();
                if ( devices == null )
                    return;

                for (DeviceInstance device : devices)
                    device.SetSensorEventSending(sensorType, state);
            }
        }).start();

        return state;
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
        listView.setAdapter(adapter);

        // ListView Item Click Listener
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                // Clicked on item (DeviceInstance)
                DeviceInstance device = (DeviceInstance) listView.getItemAtPosition(position);
                if (device == null)
                    return;

                if (device != currentDevice) {
                    if (currentDevice != null && currentDevice.isConnected) {
                        currentDevice.Disconnect ();
                    }
                }

                if (device.isConnected)
                    device.Disconnect ();
                else
                    device.Connect ( );
            }
        });
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    /**
     * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
     * The notification parameter is an integer value which represents one of the values as listed in Types.*
     * The string representation can be retrieved through TypesResolver.get(notification).
     *
     * @param device                     The DeviceInstance object that sends this notification.
     * @param DeviceInfoFlag_changed     The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
     */
    @Override
    public void OnDeviceChanged(DeviceInstance device, int DeviceInfoFlag_changed)
    {
        if ((DeviceInfoFlag_changed & DeviceInfoFlag.IsConnected) != 0 && device.isConnected)
        {
            device.SetSensorEventSending ( SensorType.Accelerometer, enableSensorAccelerometer );

            device.SetSensorEventSending ( SensorType.MotionGravityAcceleration, enableSensorAccelerometerLinear );

            device.SetSensorEventSending ( SensorType.MagneticField, enableSensorMagneticField );

            device.SetSensorEventSending ( SensorType.Gyroscope, enableSensorGyroscope );

            device.SetSensorEventSending ( SensorType.Orientation, enableSensorOrientation );

            device.SetSensorEventSending ( SensorType.Location, enableSensorLocation );

            device.SetSensorEventSending ( SensorType.Light, enableSensorLight );
        }

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
                Utils.Log(1, className, "appeared: " + device.toString());

                device.AddObserver ( this );

                Utils.Log ( 1, className, "isLocationNode: " + device.isLocationNode () );
            }
        }
    }


    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The PortalInstance object.
     */
    public void OnPortalRequestOrProvided ( PortalInstance portal )
    {

    }


    @Override
    public void OnSensorData ( @NonNull SensorFrame sensorFrame )
    {
        int type = sensorFrame.type;
        String name = "Unknown";

        if ( type >= 0 && type < SensorType.Max )
            name = Environs.sensorFlagDescriptions [ type ];

        Utils.Log(1, className, "OnSensorData: type [ " + type + " / " + name + " ]");
    }
}
