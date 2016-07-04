package environs;
/**
 * Sensors
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
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.IBinder;

class Sensors extends Service implements LocationListener
{
    private static final String className = "Sensors. . . . . . . . .";

    int hEnvirons = 0;
    Environs env;
    Activity act;

    /**
     * Objects for handling sensor services
     */
    int [] sensorSender = new int [Environs.ENVIRONS_SENSOR_TYPE_MAX];

    int sensorRegistered       = 0;

    Sensors ( Environs envObj, int hInst )
    {
        env = envObj;

        hEnvirons = hInst;
    }


    void Init ( Activity activity )
    {
        act = activity;

        try {
            if (Environs.useLocation) {
                locationManager = (LocationManager ) act.getSystemService(LOCATION_SERVICE);

                if (locationManager != null) {
                    gpsAvail = locationManager .isProviderEnabled ( LocationManager.GPS_PROVIDER );

                    netLocAvail = locationManager .isProviderEnabled ( LocationManager.NETWORK_PROVIDER );

                    if (!gpsAvail && !netLocAvail) {
                        locationManager = null;
                        Environs.useLocation = false;

                        Utils.LogE ( className, "Init: Location manager init failed." );
                    }
                }
                else {
                    Utils.LogE ( className, "Init: Query of location manager failed." );
                }
            }
            else {
                Utils.LogW ( className, "Init: Location sensors disabled." );
            }
        } catch (Exception e) {
            Utils.LogE(className, "InitInstance: Failed to init sensor manager.");
            e.printStackTrace();
        }
    }


    /**
     * Enable sending of sensor events to this DeviceInstance.
     * Events are send if the device is connected and stopped if the device is disconnected.
     *
     * @param nativeID 				Destination native device id
     * @param objID 				Destination object device id
     * @param sensorType            A value of type SensorType
     * @param enable 				true = enable, false = disable.
     *
     * @return success true = enabled, false = failed.
     */
    boolean SetSensorEventSender(int nativeID, int objID, @SensorType.Value int sensorType, boolean enable)
    {
        if (Utils.isDebug) Utils.Log ( 1, className, "SetSensorEventSender: " +
                Environs.sensorFlagDescriptions [ sensorType ] + " to be " + (enable ? "enabled" : "disabled") );

        boolean success = Environs.SetSensorEventSenderN ( hEnvirons, nativeID, objID, sensorType, enable );

        if (enable) {
            StartSensorListening(sensorType);
        }
        else {
            StopSensorListening(sensorType);
        }
        return success;
    }


    void SetSensorEventSenderFlags ( int nativeID, int objID, @SensorType.Value int flags, boolean enable )
    {
        for ( int i=0; i<SensorType.Max; ++i )
        {
            if ( (flags & Environs.sensorFlags [ i ]) != 0 ) {
                SetSensorEventSender ( nativeID, objID, i, enable );
            }
        }
    }


    void StopAllSensors ()
    {
        StopSensorListening ( SensorType.Location );

        Environs.StopSensorListeningAllN ( hEnvirons );
    }


    /**
     *
     */
    void StartSensorListening ( @SensorType.Value int sensorType )
    {
        // return immediately if the environment hasn't been started or using sensor data has been disabled.
        if (!env.started)
            return;

        if ( IsRegistered ( sensorType ) ) {
            if (Utils.isDebug) Utils.Log ( 1, className, "StartSensorListening: " +
                    Environs.sensorFlagDescriptions [ sensorType ] + " already  started." );
            return;
        }

        synchronized ( this ) {
            sensorRegistered |= Environs.sensorFlags [ sensorType ];

            if ( sensorType == SensorType.Location )
            {
                if (StartLocation ())
                    onLocationChanged ( lastLocation );
                return;
            }

            Environs.StartSensorListeningN ( hEnvirons, sensorType );
        }
    }


    boolean StartLocation ()
    {
        if (locationManager != null)
        {
            lastLocation = null;

            final Sensors sensorObj = this;

            Utils.RunSync ( act, new Runnable ( ) {
                @Override
                public void run ( ) {
                    if ( netLocAvail ) {
                        locationManager.requestLocationUpdates ( LocationManager.NETWORK_PROVIDER, UPDATE_MS_MIN, UPDATE_METERS_MIN, sensorObj );

                        lastLocation = locationManager.getLastKnownLocation ( LocationManager.NETWORK_PROVIDER );
                    }
                    else {
                        Utils.LogW ( className, "StartLocation: No network manager available" );
                    }

                    if ( gpsAvail ) {
                        locationManager.requestLocationUpdates ( LocationManager.GPS_PROVIDER, UPDATE_MS_MIN, UPDATE_METERS_MIN, sensorObj );

                        //if ( lastLocation == null )
                            lastLocation = locationManager.getLastKnownLocation ( LocationManager.GPS_PROVIDER );
                    }
                    else {
                        Utils.LogE ( className, "StartLocation: No location manager available" );
                    }

                    synchronized ( this ) {
                        this.notify ( );
                    }
                }
            } );

            if ( lastLocation != null ) {
                return true;
            }
        }
        else {
            Utils.LogE ( className, "StartLocation: No location manager available" );
        }
        return false;
    }


    void StopLocation()
    {
        if (locationManager != null) {
            locationManager.removeUpdates ( this );
            lastLocation = null;
        }
    }


    boolean IsRegistered ( @SensorType.Value int sensorType )
    {
        if ( sensorType < 0 || sensorType >= SensorType.Max )
            return false;
        return ((sensorRegistered & Environs.sensorFlags [ sensorType ]) != 0);
    }

    /**
     *
     */
    void StopSensorListening ( @SensorType.Value int sensorType )
    {
        if (Utils.isDebug) Utils.Log ( 1, className, "StopSensorListening: " + Environs.sensorFlagDescriptions [ sensorType ] );

        synchronized (this) {
            if ( sensorType == -1 ) {
                sensorRegistered = 0;
                if (Utils.isDebug) Utils.Log ( 1, className, "StopSensorListening: Stopping all sensors ..." );

                if ( locationManager != null ) {
                    locationManager.removeUpdates(this);
                }

                Environs.StopSensorListeningAllN ( hEnvirons );
                return;
            }

            int devs = GetSensorEventSenderCount ( sensorType );
            if ( devs < 0 )
                return;

            if ( devs > 0 ) {
                if (Utils.isDebug) Utils.Log ( 1, className, "StopSensorListening: " +
                        Environs.sensorFlagDescriptions [ sensorType ] + " is subscribed by " + devs + " other device(s)." );
                return;
            }

            sensorRegistered &= ~ (Environs.sensorFlags [ sensorType ]);

            if ( sensorType == SensorType.Location )
            {
                if ( IsRegistered ( sensorType ) ) {
                    if (Utils.isDebug) Utils.Log ( 1, className, "StopSensorListening: Stopping Location." );
                    StopLocation ();
                }
                return;
            }

            if (Utils.isDebug) Utils.Log ( 1, className, "StopSensorListening: Stopping " +
                    Environs.sensorFlagDescriptions [ sensorType ] + " at native." );

            Environs.StopSensorListeningN ( hEnvirons, sensorType );
        }
    }


    /**
     * Get registered DeviceInstance objects for sending of sensor events.
     *
     * @param sensorType A value of type SensorType.
     *
     * @return success true = enabled, false = failed.
     */
    int GetSensorEventSenderCount ( @SensorType.Value int sensorType )
    {
        if ( sensorType < 0 || sensorType >= SensorType.Max )
            return -1;

        int devs = Environs.GetSensorEventSenderCountN ( hEnvirons, sensorType );

        sensorSender [ sensorType ] = devs;
        return devs;
    }


    /**
     * @param rad   Angle in radians.
     * @return degrees for given radians
    public float GetDegrees ( double rad ) {
        double deg = ((rad * 180) / Math.PI);
        if (deg < 0)
            deg = 360 + deg;

        return (float) deg;
    }
     */


    LocationManager locationManager = null;
    boolean gpsAvail = false;
    boolean netLocAvail = false;

    Location lastLocation;
    double latitude;
    double longitude;
    double altitude = 0;
    float accuracy;
    float speed;
    int seqNr = 1;

    static final long UPDATE_METERS_MIN = 1; // 1 meter
    static final long UPDATE_MS_MIN = 1000; // 1 sec


    Location GetLocation()
    {
        Location loc = lastLocation;

        boolean disposeLocation = false;

        if (locationManager == null) {
            disposeLocation = true;
            if ( !StartLocation () ) {
                return null;
            }
            loc = lastLocation;
        }

        if (disposeLocation)
            StopLocation ();

        return loc;
    }


    @Override
    public void onLocationChanged ( Location location ) {
        if ( location == null )
            return;

        lastLocation    = location;
        latitude        = location.getLatitude ( );
        longitude       = location.getLongitude ( );

        double alt      = location.getAltitude ( );
        if ( altitude == 0 || Math.abs(alt - altitude) < 200 )
            altitude = alt;

        speed           = location.getSpeed ( );
        accuracy        = location.getAccuracy ( );

        //Utils.Log ( 0, className, "onLocationChanged: latitude [ " + latitude + "]  longitude [ " + longitude + " ] altitude [ " + altitude + " ] speed [ " + speed + " ]" );

        if (env.sensorLocationEnabled) {
            SensorFrame frame = new SensorFrame ();
            frame.type = SensorType.Location;
            frame.seqNumber = seqNr++;

            frame.d1 = latitude;
            frame.d2 = longitude;
            frame.d3 = altitude;

            frame.f1 = accuracy;
            frame.f2 = accuracy;
            frame.f3 = speed;

            UdpDataContext ctx = new UdpDataContext();
            ctx.sensorFrame = frame;

            try
            {
                Environs.BridgeForUdpData ( env, ctx );
            }
            catch ( Exception e ) {
                e.printStackTrace ();
                return;
            }
        }

        Environs.PushSensorDataExtN ( true, SensorType.Location, latitude, longitude, altitude, accuracy, 0, speed );
    }


    @Override
    public void onProviderDisabled(String provider) {
    }

    @Override
    public void onProviderEnabled(String provider) {
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }
}
