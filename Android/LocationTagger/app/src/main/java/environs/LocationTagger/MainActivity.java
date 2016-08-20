package environs.LocationTagger;
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
public class MainActivity extends Activity implements EnvironsSensorObserver
{
    private static final String className = "MainActivity";

    static MainActivity instance;

    static Switch switchLocation;

    static Switch switchRotation;
    static Switch switchPressure;

    static Environs environs;

    static boolean enableSensorLocation = false;

    static boolean enableSensorRotation = false;
    static boolean enableSensorPressure = false;


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        instance = this;

        switchLocation = (Switch) findViewById(R.id.switchLocation);
        if (switchLocation == null)
            return;
        switchLocation.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorLocation = ChangeSensorEventState ( SensorType.Location, isChecked );
            }
        } );

        switchRotation = (Switch) findViewById(R.id.switchRotation);
        if (switchRotation == null)
            return;
        switchRotation.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorRotation = ChangeSensorEventState ( SensorType.Rotation, isChecked );
            }
        } );

        switchPressure = (Switch) findViewById(R.id.switchPressure);
        if (switchPressure == null)
            return;
        switchPressure.setOnCheckedChangeListener ( new CompoundButton.OnCheckedChangeListener ( ) {
            @Override
            public void onCheckedChanged ( CompoundButton buttonView, boolean isChecked ) {
                enableSensorPressure = ChangeSensorEventState ( SensorType.Pressure, isChecked );
            }
        } );

        /// Create the Environs instance and take part in the given application environment
        environs = Environs.CreateInstance(this, "LocationTagger", "Environs" );

        if (environs == null) {
            Log.e("LocationTagger", "Failed to create an Environs object!");
            return;
        }

        environs.AddObserver ( new Observer ( ) );
        environs.AddObserverForSensorData ( this );

        /// Start Environs
        environs.Start ( );

        if ( !environs.IsSensorAvailable ( SensorType.Location ) ) {
            switchLocation.setEnabled ( false ); switchLocation.setChecked ( false ); enableSensorLocation = false;
        }
        else {
            switchLocation.setChecked ( enableSensorLocation );

            environs.SetSensorEvent ( SensorType.Location, enableSensorLocation );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Rotation ) ) {
            switchRotation.setEnabled ( false ); switchRotation.setChecked ( false ); enableSensorRotation = false;
        }
        else {
            switchRotation.setChecked ( enableSensorRotation );

            environs.SetSensorEvent ( SensorType.Rotation, enableSensorRotation );
        }

        if ( !environs.IsSensorAvailable ( SensorType.Pressure ) ) {
            switchPressure.setEnabled ( false ); switchPressure.setChecked ( false ); enableSensorPressure = false;
        }
        else {
            switchPressure.setChecked ( enableSensorPressure );

            environs.SetSensorEvent ( SensorType.Pressure, enableSensorPressure );
        }
    }


    private boolean ChangeSensorEventState(final @SensorType.Value int sensorType, final boolean state)
    {
        new Thread(new Runnable() {
            public void run(){
                environs.SetSensorEvent ( sensorType, state );
            }
        }).start();

        return state;
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
