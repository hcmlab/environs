package environs;

/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar */

/**
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
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
import android.support.annotation.IntDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Sensor type enumeration.
 * */
public abstract class SensorType {

	@IntDef(flag=true, value={All, Accelerometer, MagneticField, Gyroscope, Orientation, Light, Location, 
			Heading, Altimeter, MotionAttitudeRotation, MotionGravityAcceleration, MotionMagneticField, Heartrate, Proximity, 
			VOC, CO2, Humidity, Temperature, Custom, Max})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int All                 	=	-1;
	public static final int Accelerometer       	=	Types.ENVIRONS_SENSOR_TYPE_ACCELEROMETER;
	public static final int MagneticField       	=	Types.ENVIRONS_SENSOR_TYPE_MAGNETICFIELD;
	public static final int Gyroscope           	=	Types.ENVIRONS_SENSOR_TYPE_GYROSCOPE;
	public static final int Orientation         	=	Types.ENVIRONS_SENSOR_TYPE_ORIENTATION;
	public static final int Light               	=	Types.ENVIRONS_SENSOR_TYPE_LIGHT;
	public static final int Location            	=	Types.ENVIRONS_SENSOR_TYPE_LOCATION;
	public static final int Heading             	=	Types.ENVIRONS_SENSOR_TYPE_HEADING;
	public static final int Altimeter           	=	Types.ENVIRONS_SENSOR_TYPE_ALTIMETER;
	public static final int MotionAttitudeRotation                    	=	Types.ENVIRONS_SENSOR_TYPE_MOTION_ATTITUTDE_ROTATION;
	public static final int MotionGravityAcceleration                                                  	=	Types.ENVIRONS_SENSOR_TYPE_MOTION_GRAVITY_ACCELERATION;
	public static final int MotionMagneticField 	=	Types.ENVIRONS_SENSOR_TYPE_MOTION_MAGNETICFIELD;
	public static final int Heartrate           	=	Types.ENVIRONS_SENSOR_TYPE_HEARTRATE;
	public static final int Proximity           	=	Types.ENVIRONS_SENSOR_TYPE_PROXIMITY;
	public static final int VOC                 	=	Types.ENVIRONS_SENSOR_TYPE_VOC;
	public static final int CO2                 	=	Types.ENVIRONS_SENSOR_TYPE_CO2;
	public static final int Humidity            	=	Types.ENVIRONS_SENSOR_TYPE_HUMIDITY;
	public static final int Temperature         	=	Types.ENVIRONS_SENSOR_TYPE_TEMPERATURE;
	public static final int Custom              	=	Types.ENVIRONS_SENSOR_TYPE_CUSTOM;
	public static final int Max                 	=	Types.ENVIRONS_SENSOR_TYPE_MAX;
		
}




