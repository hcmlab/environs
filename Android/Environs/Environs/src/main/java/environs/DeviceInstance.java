package environs;
/**
 *	DeviceInstance
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
import android.annotation.SuppressLint;
import android.support.annotation.Nullable;
import android.util.SparseArray;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.LinkedList;

/**
 *	DeviceInstance management and event provider
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
@SuppressWarnings ( "unused" )
public class DeviceInstance
{
	private static final String className = "DeviceInstance . . . . .";

	int                         hEnvirons = 0;

	Environs 					env;

	private int	storageLoaded = 0;

	/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
	public @Call.Value 	int async = Call.NoWait;

	/**
	 * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
	 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to ChangeObservers.
	 * */
	public boolean disposed = false;

	/** The device id within the environment */
	public int deviceID;

	/** The native id within the environment. 0 means invalid (not connected). */
	public int nativeID;

	/** The object id within the environment. 0 means invalid (not connected). */
	int objID;

	/** The previous object id within the environment. 0 means invalid (not connected).
	 * The previous object id is required if broadcast and mediator managed devices have to be merged.
	 * In this case, we keep the previous objID to manage old events that may come after the objID update.*/
	int objIDPrevious;

	/** The latest (incoming) portalID that Environs received for this DeviceInstance */
	public int portalIDIn = -1;

	/** The latest (incoming) portalID that Environs received for this DeviceInstance */
	public int portalIDOut = -1;
	
	/** ip from device. The ip address reported by the device which it has read from network configuration. */
	public String ip;
	
	/** ip external. The ip address which was recorded by external sources (such as the Mediator) during socket connections.
	 * This address could be different from ip due to NAT, Router, Gateways behind the device.
	 */
	public String ipe;
	
	/** The tcp port on which the device listens for device connections. */
	public short tcpPort;
	
	/** The udp port on which the device listens for device connections. */
	public short udpPort;
	
	/** The number of alive updates noticed by the mediator layer since its appearance within the application environment. */
	public int updates;

	/** A value that describes the device platform. */
	public int platform;

	int flags = 0;

	/** BroadcastFound is a value of DEVICEINFO_DEVICE_* and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
	public @DeviceSourceType.Value int sourceType;

	public boolean unavailable;
	
	/** isConnected is true if the device is currently in the connected state. */
	public boolean isConnected;

	public int connectProgress = 0;

	public boolean isSameAppArea = false;

	/** The device type, which match the constants DEVICE_TYPE_* . */
	//public char		deviceType;
	
	/** The device name. */
	public String deviceName;
	
	/** The area name of the appliction environment. */
	public String areaName;
	
	/** The application name of the appliction environment. */
	public String appName;

	/** Determines whether the device is in physical contact (1) or not (0) with this device. */
	public int directStatus = 0;

	/** A DeviceDisplay structure that describes the device's display properties. */
	DeviceDisplay display = null;

	@SuppressWarnings ( "all" )
	/** A collection of PortalInstances that this device has established or is managing. */
	final ArrayList<PortalInstance> devicePortals = new ArrayList<PortalInstance>();

	@SuppressWarnings ( "all" )
	/** A collection of observers that observe this device instance for changes and events. */
	final ArrayList<DeviceObserver>	observers = new ArrayList<DeviceObserver>();

	@SuppressWarnings ( "all" )
	/** A collection of observers that observe this device instance for changes and events. */
	final ArrayList<MessageObserver>	observersForMessages = new ArrayList<MessageObserver>();

	@SuppressWarnings ( "all" )
	/** A collection of observers that observe this device instance for changes and events. */
	final ArrayList<DataObserver>	observersForData = new ArrayList<DataObserver>();

	@SuppressWarnings ( "all" )
	/** A collection of observers that observe this device instance for sensor data. */
	final ArrayList<SensorObserver>	observersForSensorData = new ArrayList<SensorObserver>();

	/** Bit flag field that determines whether the device shall receive events from us. */
	int enableSensorSender = 0;

	/** Determines the default value for emitting notifications from the BaseAdapter if properties of this instance has changed. */
	public static boolean notifyPropertyChangedDefault = true;

	/** Determines whether notification shall be emmitted if properties of this instance has changed. */
	public boolean notifyPropertyChanged = notifyPropertyChangedDefault;

	/** Application defined contexts for arbitrary use. */
	public int      appContext0 = 0;
	public Object   appContext1 = null;
	public Object   appContext2 = null;


	@Override
	@SuppressLint("DefaultLocale")
	public String toString() {
		
        return GetBroadcastString(false) + " " +  (isConnected ? " *" : "") + " 0x" + Integer.toHexString(this.deviceID).toUpperCase() + " " + DeviceTypeString() + ": "
        		+ this.deviceName + ", [" + this.appName + "/" + this.areaName + "], [" + this.objID + "]";
    }

	public String ToString() {
		return toString();
	}


	public boolean isObserverReady ()
	{

		return ( ( flags & DeviceFlagsInternal.CPObserverReady ) != 0 );
	}

	public boolean isMessageObserverReady ()
	{

		return ( ( flags & DeviceFlagsInternal.CPMessageReady ) != 0 );
	}

	public boolean isDataObserverReady ()
	{
		return ( ( flags & DeviceFlagsInternal.CPDataReady ) != 0 );
	}

	public boolean isSensorObserverReady ()
	{
		return ( ( flags & DeviceFlagsInternal.CPSensorReady ) != 0 );
	}


	public String GetBroadcastString(boolean fullText)
	{
		switch (sourceType)
		{
			case Environs.DEVICEINFO_DEVICE_BROADCAST:
				return fullText ? "Nearby" : "B";
			case Environs.DEVICEINFO_DEVICE_MEDIATOR:
				return fullText ? "Mediator" : "M";
			case Environs.DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR:
				return fullText ? "Med+Near" : "MB";
		}
		return "U";
	}


	class NotifierContext
	{
		int type;
		int flags;
		Object data;
		DeviceInstance device;
	}


	static void NotifierThread ( Environs env )
	{
		NotifierContext ctx;

		while ( env.deviceInstanceNotifierRun )
		{
			synchronized (env.deviceInstanceNotifierQueue)
			{
				if ( env.deviceInstanceNotifierQueue.size() == 0 ) {
					try {
						env.deviceInstanceNotifierQueue.wait ();
					} catch ( InterruptedException e ) {
						e.printStackTrace ( );
						break;
					}
					continue;
				}
				else {
					ctx = (NotifierContext)env.deviceInstanceNotifierQueue.remove();

					if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: Dequeue");
				}
			}

			switch ( ctx.type ) {
				case 1:
					ctx.device.NotifyObservers ( ctx.flags, false );
					break;
				case 2:
					ctx.device.NotifyObserversForMessage ( ( MessageInstance ) ctx.data, ctx.flags, false );
					break;
				case 3:
					ctx.device.NotifyObserversForData ( ( FileInstance ) ctx.data, ctx.flags, false );
					break;
			}
			if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: next");
		}

		if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: done");

		synchronized ( env.deviceInstanceNotifierQueue ) {
			env.deviceInstanceNotifier = null;
		}
	}


	@SuppressWarnings ( "unchecked" )
	void EnqueueNotification ( NotifierContext ctx )
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "EnqueueNotification");

		if ( ctx == null ) return;

		synchronized (env.deviceInstanceNotifierQueue)
		{
			if ( !env.deviceInstanceNotifierRun ) return;

			env.deviceInstanceNotifierQueue.add ( ctx );

			env.deviceInstanceNotifierQueue.notify ( );
			if (Utils.isDebug) Utils.Log ( 6, className, "EnqueueNotification: Enqueue");
		}

		if (Utils.isDebug) Utils.Log ( 6, className, "EnqueueNotification: done");
	}


	/**
	 * Update device flags to native layer and populate them to the environment.
	 *
	 * @param	flags    The internal flags to set or clear.
	 * @param	set    	 true = set, false = clear.
	 */
	void SetDeviceFlags ( @DeviceFlagsInternal.Value int flags, boolean set )
	{
		if ( set ) {
			if ( (this.flags & flags ) != 0 )
				return;
			this.flags |= flags;
		}
		else {
			if ( (this.flags & flags ) == 0 )
				return;
			this.flags &= flags;
		}

		Environs.SetDeviceFlagsN ( hEnvirons, Call.NoWait, objID, set ? this.flags : flags, set );
	}


	/**
	 * Add an observer (DeviceObserver) that notifies about device property changes.
	 *
	 * @param observer A DeviceObserver
	 */
	public void AddObserver(DeviceObserver observer)
	{
		synchronized ( observers )
		{
			if (observer != null && !observers.contains(observer)) {
				observers.add ( observer );
				notifyPropertyChanged = false;

				if (observers.size () == 1)
					SetDeviceFlags ( DeviceFlagsInternal.ObserverReady, true );
			}
		}
	}

	/**
	 * Remove an observer (DeviceObserver) that was added before.
	 *
	 * @param observer A DeviceObserver
	 */
	public void RemoveObserver(DeviceObserver observer)
	{
		synchronized ( observers )
		{
			if (observer != null && !observers.contains(observer)) {
				observers.remove ( observer );
				notifyPropertyChanged = notifyPropertyChangedDefault;

				if (observers.size () == 0)
					SetDeviceFlags ( DeviceFlagsInternal.ObserverReady, false );
			}
		}
	}


	@SuppressWarnings ( "unchecked" )
	void NotifyObservers(int flags, boolean enqueue)
	{
		if (enqueue) {
			NotifierContext ctx = new NotifierContext();
			ctx.type = 1;
			ctx.flags = flags;
			ctx.device = this;

			EnqueueNotification (ctx);
			return;
		}

		ArrayList<DeviceObserver> obs;

		synchronized ( observers )
		{
			obs = (ArrayList<DeviceObserver>) observers.clone ();
		}

		for (int i=0; i<obs.size(); i++)
		{
			DeviceObserver observer = obs.get(i);

			try {
				observer.OnDeviceChanged ( this, flags );
			} catch (Exception e) {
				e.printStackTrace ( );
			}
		}

		if (flags == Environs.ENVIRONS_OBJECT_DISPOSED) {
			appContext1 = null; appContext2 = null;
			observers.clear ();
			observersForData.clear ();
			observersForMessages.clear ();
			observersForSensorData.clear ();
		}
	}


	/**
	 * Notify to all observers (DeviceObserver) that the appContext has changed.
	 *
	 * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
	 */
	public void NotifyAppContextChanged(int customFlags)
	{
		if ( customFlags == 0 )
			customFlags = Environs.DEVICE_INFO_ATTR_APP_CONTEXT;

		NotifyObservers ( customFlags, true );
	}

	/**
	 * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
	 *
	 * @param observer A MessageObserver
	 */
	public void AddObserverForMessages(MessageObserver observer)
	{
		synchronized ( observersForMessages )
		{
			if (observer != null && !observersForMessages.contains(observer)) {
				observersForMessages.add ( observer );

				messagesEnqueue = false;

				if (observersForMessages.size () == 1)
					SetDeviceFlags ( DeviceFlagsInternal.MessageReady, true );
			}
		}
	}

	/**
	 * Remove an observer (MessageObserver) that was added before.
	 *
	 * @param observer A MessageObserver
	 */
	public void RemoveObserverForMessages(MessageObserver observer)
	{
		synchronized ( observersForMessages )
		{
			if (observer != null && !observersForMessages.contains(observer)) {
				observersForMessages.remove ( observer );

				if (observersForMessages.size () == 0)
				{
					messagesEnqueue = true;

					SetDeviceFlags ( DeviceFlagsInternal.MessageReady, false );
				}
			}
		}
	}

	@SuppressWarnings ( "unchecked" )
	void NotifyObserversForMessage(MessageInstance messageInst, int MESSAGE_INFO_ATTR_changed, boolean enqueue)
	{
		if ( messageInst == null ) return;

		if (enqueue) {
			NotifierContext ctx = new NotifierContext();
			ctx.type = 2;
			ctx.flags = MESSAGE_INFO_ATTR_changed;
			ctx.data = messageInst;
			ctx.device = this;
			EnqueueNotification (ctx);
			return;
		}

		ArrayList<MessageObserver> obs;

		synchronized ( observersForMessages )
		{
			obs = (ArrayList<MessageObserver>) observersForMessages.clone ();
		}

		for (int i=0; i<obs.size(); i++)
		{
			MessageObserver observer = obs.get(i);

			try {
				observer.OnMessage ( messageInst, MESSAGE_INFO_ATTR_changed );
			} catch (Exception e) {
				e.printStackTrace ( );
			}
		}
	}

	/**
	 * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
	 *
	 * @param observer A DataObserver
	 */
	public void AddObserverForData(DataObserver observer)
	{
		synchronized ( observersForData )
		{
			if (observer != null && !observersForData.contains(observer)) {
				observersForData.add ( observer );

				filesLast = -1;

				if (observersForData.size () == 1)
					SetDeviceFlags ( DeviceFlagsInternal.DataReady, true );
			}
		}
	}

	/**
	 * Remove an observer (DataObserver) that was added before.
	 *
	 * @param observer A DataObserver
	 */
	public void RemoveObserverForData(DataObserver observer)
	{
		synchronized ( observersForData )
		{
			if (observer != null && !observersForData.contains(observer)) {
				observersForData.remove ( observer );

				if (observersForData.size () == 0)
				{
					filesLast = files.size ();

					SetDeviceFlags ( DeviceFlagsInternal.DataReady, false );
				}
			}
		}
	}


	@SuppressWarnings ( "unchecked" )
	void NotifyObserversForData(FileInstance fileInst, int FILE_INFO_ATTR_changed, boolean enqueue)
	{
		if (fileInst == null) return;

		if (enqueue) {
			NotifierContext ctx = new NotifierContext();
			ctx.type = 3;
			ctx.flags = FILE_INFO_ATTR_changed;
			ctx.data = fileInst;
			ctx.device = this;
			EnqueueNotification (ctx);
			return;
		}

		ArrayList<DataObserver> obs;

		synchronized ( observersForData )
		{
			obs = (ArrayList<DataObserver>) observersForData.clone ();
		}

		for (int i=0; i<obs.size(); i++)
		{
			DataObserver observer = obs.get ( i );

			try {
				observer.OnData(fileInst, FILE_INFO_ATTR_changed);
			} catch (Exception e) {
				e.printStackTrace ( );
			}
		}
	}


	/**
	 * Add an observer (SensorObserver) that notifies about sensor data received through the DeviceInstance.
	 *
	 * @param observer A DataObserver
	 */
	public void AddObserverForSensorData(SensorObserver observer)
	{
		synchronized ( observersForSensorData )
		{
			if (observer != null && !observersForSensorData.contains(observer)) {
				observersForSensorData.add ( observer );

				if (observersForSensorData.size () == 1)
					SetDeviceFlags ( DeviceFlagsInternal.SensorReady, true );
			}
		}
	}

	/**
	 * Remove an observer (SensorObserver) that was added before.
	 *
	 * @param observer A DataObserver
	 */
	public void RemoveObserverForSensorData(SensorObserver observer)
	{
		synchronized ( observersForSensorData )
		{
			if (observer != null && !observersForSensorData.contains(observer)) {
				observersForSensorData.remove ( observer );

				if (observersForSensorData.size () == 0)
					SetDeviceFlags ( DeviceFlagsInternal.SensorReady, false );
			}
		}
	}


	@SuppressWarnings ( "unchecked" )
	void NotifySensorObservers(SensorFrame frame)
	{
		ArrayList<SensorObserver> obs;

		synchronized ( observersForSensorData )
		{
			obs = (ArrayList<SensorObserver>) observersForSensorData.clone ();
		}

		for (int i=0; i<obs.size(); i++)
		{
			SensorObserver observer = obs.get ( i );

			try {
				observer.OnSensorData ( frame );
			} catch ( Exception e ) {
				e.printStackTrace ( );
			}
		}
	}



	/*
	 * DeviceInstance notifier thread resources for an instance of Environs
	 */
	final LinkedList udpData = new LinkedList();


	@SuppressWarnings ( "unchecked" )
	void NotifyUdpData(byte [] data)
	{
		synchronized ( udpData )
		{
			if (disposed)
				return;

			if (udpData.size () < 128){
				udpData.add ( data );
				udpData.notify ();
			}
		}
	}


	boolean SetDirectContact(int status)
	{
		if ( directStatus == status )
			return false;
		directStatus = status;

		NotifyObservers(Environs.DEVICE_INFO_ATTR_DIRECT_CONTACT, true);

		if (devicePortals.size() <= 0) {
			if (status == 1 && env.GetPortalAutoStart()) {

				PortalInstance portal = PortalGetIncoming();
				if ( portal == null ) {
					Environs.RequestPortalStreamN ( hEnvirons, nativeID, async, PortalType.Any, 0, 0);
				}
			}
		}
		else {
			for ( int i=0; i < devicePortals.size(); i++ )
			{
				PortalInstance portal = devicePortals.get(i);
				if ( portal.portalID > 0 ) {
					try {
						portal.NotifyObservers(Environs.NOTIFY_CONTACT_DIRECT_CHANGED);

						if (status == 1 && env.GetPortalAutoStart()) {
							portal.Start();
						}
					}
					catch(Exception ex) {
						devicePortals.remove(i);
						i--;
					}
				}
			}
		}
		return true;
	}


	/**
	 * Enable sending of sensor events to this DeviceInstance.
	 * Events are send if the device is connected and stopped if the device is disconnected.
	 *
	 * @param sensorType 	A value of type environs.SensorType
	 * @param enable 		true = enable, false = disable.
	 *
	 * @return success true = enabled, false = failed.
	 */
	@SuppressWarnings ( "SimplifiableIfStatement" )
	public boolean SetSensorEventSending ( @SensorType.Value int sensorType, boolean enable )
	{
		if ( sensorType < 0 || sensorType >= Environs.ENVIRONS_SENSOR_TYPE_MAX )
			return false;

		int flag = Environs.sensorFlags [ sensorType ];

		if ( enable == ((enableSensorSender & flag) != 0) ) {
			if (Utils.isDebug) Utils.Log ( 1, className, "SetSensorEventSending: " +
					Environs.sensorFlagDescriptions [ sensorType ] + " already " + (enable ? "enabled" : "disabled") );
			return true;
		}

		if ( !env.IsSensorAvailable ( sensorType ) ) {
			Utils.LogW ( className, "SetSensorEventSending: " + Environs.sensorFlagDescriptions [ sensorType ] + " unsupported." );
			return false;
		}

		if (enable)
			enableSensorSender |= flag;
		else
			enableSensorSender &= ~flag;

		if (isConnected && nativeID > 0) {
			return env.sensors.SetSensorEventSender(nativeID, objID, sensorType, enable);
		}
		return true;
	}


	/**
	 * Query whether sending of the given sensor events to this DeviceInstance is enabled or not.
	 *
	 * @param sensorType 	A value of type environs.SensorType
	 *
	 * @return success true = enabled, false = disabled.
	 */
	@SuppressWarnings ( "SimplifiableIfStatement" )
	public boolean IsSetSensorEventSending ( @SensorType.Value int sensorType )
	{
		if ( sensorType < 0 || sensorType >= Environs.ENVIRONS_SENSOR_TYPE_MAX )
			return false;

		return ((enableSensorSender & Environs.sensorFlags [ sensorType ]) != 0);
	}


	void Dispose()
	{
		synchronized (devicePortals) {
			if (disposed) {
				if (Utils.isDebug) Utils.Log ( 6, className, "Dispose: " + deviceID + " already disposed." );
				return;
			}
			disposed = true;
		}

		if (enableSensorSender != 0 ) {
			env.sensors.SetSensorEventSenderFlags ( nativeID, objID, enableSensorSender, false );
		}

		NotifyObservers(Environs.ENVIRONS_OBJECT_DISPOSED, true);
		// If we remove the observers, then no disposed notification can be delivered
//		observers.clear();
		observersForSensorData.clear ();
		observersForMessages.clear ();
		observersForData.clear ();

		udpData.clear ();
		messages.clear ();
		if (messagesCache != null) messagesCache.clear ();
		files.clear ();
		if (filesCache != null) filesCache.clear ();
	}


	public static boolean IsPlatformType(int src, int platform)
	{
		return ((src & platform) == platform);
	}


	public boolean isLocationNode()
	{
		return ((platform & Platforms.LocationNode_Flag) != 0);
	}


	public String DeviceTypeString()
	{
		if (IsPlatformType(platform, Platforms.Tablet_Flag))
			return "Tablet";
		else if (IsPlatformType(platform, Platforms.Smartphone_Flag))
			return "Smartphone";
		else if (IsPlatformType(platform, Platforms.MSSUR01))
			return "Surface 1";
		else if (IsPlatformType(platform, Platforms.SAMSUR40))
			return "Surface 2";
		else if (IsPlatformType(platform, Platforms.Tabletop_Flag))
			return "Tabletop";
		else if (IsPlatformType(platform, Platforms.Display_Flag))
			return "Display";
		return "Unknown";
	}


    boolean Update(DeviceInstance device)
    {
		int changed = 0;

		if (deviceID != device.deviceID) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			deviceID = device.deviceID;
		}
		if (nativeID != device.nativeID) {
			changed |= Environs.DEVICE_INFO_ATTR_NATIVEID;
			nativeID = device.nativeID;
		}
		if (objID != device.objID) {
			changed |= Environs.DEVICE_INFO_ATTR_OBJID;
			objIDPrevious = objID;
			objID = device.objID;
		}
		if ( ip.contentEquals ( device.ip ) ) {
			changed |= Environs.DEVICE_INFO_ATTR_IP;
			ip = device.ip;
		}
		if ( ipe.contentEquals ( device.ipe ) ) {
			changed |= Environs.DEVICE_INFO_ATTR_IPE;
			ipe = device.ipe;
		}
		if (tcpPort != device.tcpPort) {
			changed |= Environs.DEVICE_INFO_ATTR_TCP_PORT;
			tcpPort = device.tcpPort;
		}
		if (udpPort != device.udpPort) {
			changed |= Environs.DEVICE_INFO_ATTR_UDP_PORT;
			udpPort = device.udpPort;
		}
        updates = device.updates;

		if (sourceType != device.sourceType) {
			changed |= Environs.DEVICE_INFO_ATTR_BROADCAST_FOUND;
			sourceType = device.sourceType;
		}
		if (unavailable != device.unavailable) {
			changed |= Environs.DEVICE_INFO_ATTR_UNAVAILABLE;
			unavailable = device.unavailable;
		}
		if (isConnected != device.isConnected) {
			changed |= Environs.DEVICE_INFO_ATTR_ISCONNECTED;
			isConnected = device.isConnected;

			if (isConnected) {
				synchronized (this) {
					notifyAll();

					if (nativeID > 0 && enableSensorSender != 0 ) {
						env.sensors.SetSensorEventSenderFlags ( nativeID, objID, enableSensorSender, true );
					}
				}
			}
			else {
				udpData.clear ();
			}
		}
		/*
		if (deviceType != device.deviceType) {
			changed |= Environs.DEVICE_INFO_ATTR_DEVICE_PLATFORM;
			deviceType = device.deviceType;
		}
		*/
		if (platform != device.platform) {
			changed |= Environs.DEVICE_INFO_ATTR_DEVICE_PLATFORM;
			platform = device.platform;
		}
		if (deviceName == null || device.deviceName == null || !deviceName.equals(device.deviceName)) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			deviceName = device.deviceName;
		}
		if (areaName == null || device.areaName == null || !areaName.equals(device.areaName)) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			areaName = device.areaName;
		}
		if (areaName == null) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			areaName = env.areaName;
		}
		if (appName == null || device.appName == null || !appName.equals(device.appName)) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			appName = device.appName;
		}
		if (appName == null) {
			changed |= Environs.DEVICE_INFO_ATTR_IDENTITY;
			appName = env.appName;
		}

		if (changed != 0) {
			if ( (changed & Environs.DEVICE_INFO_ATTR_IDENTITY) != 0 )
				isSameAppArea = device.isSameAppArea; // EqualsAppEnv ( env.areaName, env.appName );

			NotifyObservers ( changed, true );

			if ( (changed & Environs.DEVICE_INFO_ATTR_ISCONNECTED ) == Environs.DEVICE_INFO_ATTR_ISCONNECTED && isConnected )
			{
				display = env.GetDeviceDisplayProps(nativeID);
			}
			if (notifyPropertyChanged)
				return true;
		}
		return false;
    }


	public boolean EqualsID(DeviceInstance equalTo)
	{
		if (equalTo.areaName == null)
			equalTo.areaName = env.areaName;
		if (equalTo.appName == null)
			equalTo.appName = env.appName;

		return (deviceID == equalTo.deviceID && appName.equals(equalTo.appName) && areaName.equals(equalTo.areaName));
	}


	public boolean EqualsID(int deviceID, String areaName, String appName)
	{
		if (areaName == null)
			areaName = env.areaName;
		if (appName == null)
			appName = env.appName;

		return (deviceID == this.deviceID && appName.equals(this.appName) && areaName.equals(this.areaName));
	}


	public boolean EqualsAppEnv(DeviceInstance equalTo)
	{
		if (equalTo.areaName == null)
			equalTo.areaName = env.areaName;
		if (equalTo.appName == null)
			equalTo.appName = env.appName;

		return (appName.equals(equalTo.appName) && areaName.equals(equalTo.areaName));
	}


	public boolean EqualsAppEnv(String aareaName, String aappName)
	{
		if (aareaName == null)
			aareaName = env.areaName;
		if (aappName == null)
			aappName = env.appName;

		return (appName.equals(aappName) && areaName.equals(aareaName));
	}


	public boolean LowerThanAppEnv(DeviceInstance equalTo)
	{
		return LowerThanAppEnv(equalTo.areaName, equalTo.appName);
	}


	public boolean LowerThanAppEnv(String areaName, String appName)
	{
		if (areaName == null)
			areaName = env.GetAreaName();
		if (appName == null)
			appName = env.GetApplicationName();

		return (areaName.compareTo(areaName) < 0 || appName.compareTo(appName) < 0);
	}


	/** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
	 Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
	public boolean GetAllowConnect ()
	{
		return (Environs.AllowConnectN ( hEnvirons, objID, -1 ) != 0);
	}

	/** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
	 Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
	public void SetAllowConnect ( boolean value )
	{
		Environs.AllowConnectN ( hEnvirons, objID, value ? 1 : 0 );
	}


	/**
	 * Connect to this device asynchronously.
	 *
	 * @return status	false: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
	 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
	 * 					true: A new connection has been triggered and is in progress
	 */
	public boolean Connect() {
		return Connect(async);
	}


	/**
	 * Connect to this device using the given mode.
	 *
	 * @param callType   A value of environs.Call that determines whether (only this call) is performed synchronous or asynchronous.
	 *
	 * @return status	false: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
	 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
	 * 					true: A new connection has been triggered and is in progress
	 */
	public boolean Connect(@Call.Value int callType)
	{
		boolean success = env.DeviceConnect ( deviceID, isSameAppArea ? null : areaName, isSameAppArea ? null : appName, callType ) != 0;
		if (success && callType == Call.Wait) {
			synchronized (this) {
				if (!isConnected) {
					try {
						wait(8000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
			return isConnected;
		}
		return success;
	}


	/**
	 * Disconnect the device with the given id and a particular application environment.
	 *
	 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
	 */
	public boolean Disconnect () {
		return Disconnect(async);
	}


	/**
	 * Disconnect the device  using the given mode with the given id and a particular application environment.
	 *
	 * @param callType   A value of Environs.Call that determines whether (only this call) is performed synchronous or asynchronous.
	 *
	 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
	 */
	public boolean Disconnect ( @Call.Value int callType ) {

		int count = devicePortals.size();

		while ( devicePortals.size() > 0 && count > 0 )
		{
			try {
				devicePortals.get(0).Dispose();
			}
			catch(Exception ex) {
				ex.printStackTrace ();
			}
			count--;
		}

		return env.DeviceDisconnect ( nativeID, callType );
	}


	/**
	 * Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
	 *
	 * @return PortalInstance-object
	 */
	public DeviceDisplay GetDisplayProps()
	{
		return display;
	}


	/**
	 * Creates a portal instance that requests a portal.
	 *
	 * @param 	portalType	        A value of type environs.PortalType
	 *
	 * @return 	PortalInstance-object
	 */
	@Nullable
	public PortalInstance PortalRequest ( @PortalType.Value int portalType)
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalRequest");

		return PortalCreate(Environs.PORTAL_DIR_INCOMING, portalType, -1);
	}

	/**
	 * Creates a portal instance that provides a portal.
	 *
	 * @param 	portalType	        A value of type environs.PortalType
	 *
	 * @return 	PortalInstance-object
	 */
	@Nullable
	public PortalInstance PortalProvide( @PortalType.Value int portalType)
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalProvide");

		PortalInstance portal = PortalCreate(Environs.PORTAL_DIR_OUTGOING, portalType, -1);
		if (portal != null)
			portal.status = PortalStatus.CreatedAskRequest;
		return portal;
	}

	/**
	 * Creates a portal instance.
	 *
	 * @param request   The portal request.
	 * @return PortalInstance-object
	 */
	PortalInstance PortalCreate(int request)
	{
		return PortalCreate(request & Environs.PORTAL_DIR_MASK, request & Environs.PORTAL_TYPE_MASK, request & 0xFF);
	}

	/**
	 * Creates a portal instance.
	 *
	 * @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
	 * @param portalType	        Project name of the application environment
	 * @return PortalInstance-object
	 */
	PortalInstance PortalCreate(int Environs_PORTAL_DIR, int portalType, int slot)
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalCreate");

		if ( !isConnected )
			return null;

		PortalInstance portal = new PortalInstance();
		portal.async = async;

		if ( !portal.Init(hEnvirons, this, Environs_PORTAL_DIR, portalType, slot) )
			return null;

		return portal;
	}


	/**
	 * Creates a portal instance with a given portalID.
	 *
	 * @param portalID   The portalID received from native layer.
	 * @return PortalInstance-object
	 */
	@Nullable
	public PortalInstance PortalCreateID(int portalID)
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalCreateID");

		if ( !isConnected )
			return null;

		PortalInstance portal = new PortalInstance();

		if ( !portal.Init(hEnvirons, this, portalID) )
			return null;

		portal.async = async;
		return portal;
	}


	/**
	 * Query the first PortalInstance that manages an outgoing portal.
	 *
	 * @return PortalInstance-object
	 */
	@Nullable
	public PortalInstance PortalGetOutgoing()
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalGetOutgoing");
		return PortalGet(true);
	}

	/**
	 * Query the first PortalInstance that manages an incoming portal.
	 *
	 * @return PortalInstance-object
	 */
	@Nullable
	public PortalInstance PortalGetIncoming()
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "PortalGetIncoming" );
		return PortalGet(false);
	}


	PortalInstance PortalGet(boolean outgoing)
	{
		PortalInstance.KillZombies();

		synchronized (devicePortals) {
			for ( int i=0; i < devicePortals.size(); i++ )
			{
				PortalInstance portal = devicePortals.get(i);
				if ( !portal.disposeOngoing && portal.outgoing == outgoing )
					return portal;
			}
		}
		return null;
	}


	/**
	 * Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
	 *
	 * @return PortalInstance-object
	 */
	@Nullable
	PortalInstance PortalGetWaiting(boolean outgoing)
	{
		PortalInstance.KillZombies();

		synchronized (devicePortals)
		{
			for ( int i=0; i < devicePortals.size(); i++ )
			{
				PortalInstance portal = devicePortals.get(i);
				if ( !portal.disposeOngoing && portal.portalID < 0 && portal.outgoing == outgoing )
					return portal;
			}
		}
		return null;
	}


	boolean SetFileProgress(int fileID, int progress, boolean send)
	{
		FileInstance fileInst = files.get(fileID);
		if (fileInst == null)
			return false;

		if (send)
			fileInst.sendProgress = progress;
		else
			fileInst.receiveProgress = progress;

		NotifyObserversForData(fileInst, send ? Environs.FILE_INFO_ATTR_SEND_PROGRESS : Environs.FILE_INFO_ATTR_RECEIVE_PROGRESS, true);
		return true;
	}


	boolean messagesEnqueue = true;

	@SuppressWarnings ( "all" )
	final LinkedList messages = new LinkedList();
	@SuppressWarnings ( "all" )
	ArrayList<MessageInstance> messagesCache;

	@SuppressWarnings ( "unchecked" )
	void AddMessage(String message, int length, boolean sent, char connection)
	{
		if (disposed)
			return;

		if (connection == 'u')
		{
			if (nativeID > 0)
				connection = 'c';
			else if (sourceType != DeviceSourceType.Mediator)
				connection = 'd';
			else
				connection = 'm';
		}

		MessageInstance msg = new MessageInstance();
		msg.sent = sent;
		msg.text = message;
		msg.created = System.currentTimeMillis() / 1000L;
		msg.connection = connection;
		msg.device = this;

		if ( !sent ) {
			if (Utils.isDebug) Utils.Log ( 4, className, "AddMessage: Delivering Incoming message [ " + message + " ] device [ " + toString ( ) + " ]" );

			if (Utils.isDebug) Utils.Log ( 4, className, "AddMessage: con [ " + connection + " ]" );
		}
		else {
			if (Utils.isDebug) Utils.Log ( 4, className, "AddMessage: Delivering Outgoing message [ " + message + " ] device [ " + toString ( ) + " ]" );

			if (Utils.isDebug) Utils.Log ( 4, className, "AddMessage: con [ " + connection + " ]" );
		}

		if (messagesEnqueue) {
			messages.add ( msg );

			synchronized ( messages ) {
				messages.notify ();
			}
			return;
		}

		NotifyObserversForMessage(msg, Environs.MESSAGE_INFO_ATTR_CREATED, true);
	}

	/**
	 * Get a list with all messages that this device has received (and sent) from the storage.
	 *
	 * @return Collection with objects of type MessageInstance
	 */
	public ArrayList GetMessagesInStorage ( )
	{
		if (storageLoaded < 2)
			ParseStoragePath(true);
		return messagesCache;
	}

	/**
	 * Get a list with all messages that this device has received (and sent) since the Device instance has appeared.
	 *
	 * @return Collection with objects of type MessageInstance
	 */
	@SuppressWarnings ( "unchecked" )
	public ArrayList GetMessages ( )
	{
		ArrayList list = new ArrayList ();

		synchronized ( messages )
		{
			list.addAll ( messages );
			messages.clear ();
		}
		return list;
	}


	int filesLast = -1;

	@SuppressWarnings ( "all" )
	final static SparseArray<FileInstance> files = new SparseArray<FileInstance>();

	static SparseArray<FileInstance> filesCache;

	String StoragePath = null;


	/**
	 * Get a dictionary with all files that this device has received (and sent) from the storage.
	 *
	 * @return Collection with objects of type FileInstance with the fileID as the key.
	 */
	public SparseArray<FileInstance> GetFilesInStorage()
	{
		if (storageLoaded < 2)
			ParseStoragePath(true);
		return filesCache;
	}

	/**
	 * Get a dictionary with all files that this device has received (and sent) since the Device instance has appeared.
	 *
	 * @return Collection with objects of type FileInstance with the fileID as the key.
	 */
	public SparseArray<FileInstance> GetFiles()
	{
		return files;
	}


	/**
	 * Refresh files from fileStorage.
	 *
	 */
	static void ParseAllFiles(final ArrayList devices)
	{
		new Thread(new Runnable() {
			public void run(){
				for (int i=0; i<devices.size(); i++) {
					DeviceInstance device = (DeviceInstance) devices.get(i);
					Utils.Log ( 1, className, "ParseAllFiles: " + device.toString ( ) );
					device.ParseStoragePath(false);
				}
			}
		}).start ( );
	}

	/**
	 * Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
	 *
	 * @return absolutePath
	 */
	public String GetStoragePath()
	{
		VerifyStoragePath ( );

		return StoragePath;
	}


	@SuppressWarnings ( "all" )
	boolean ParseStoragePath(boolean wait)
	{
		//Utils.Log ( 3, "ParseStoragePath" );

		VerifyStoragePath();

		if (StoragePath == null)
			return false;

		synchronized (messages) {
			if ( storageLoaded > 1 )
				return true;

			if ( storageLoaded == 1 ) {
				// Loading is ongoing
				if ( !wait )
					return true;
			}
			else
				storageLoaded = 1;
		}

		messagesCache = new ArrayList<MessageInstance>();
		filesCache = new SparseArray<FileInstance>();

		synchronized (files) {
			synchronized (messages) {
				if (storageLoaded > 1) {
					return true;
				}
			}
			try
			{
				// Read descriptor
				String name = StoragePath + "receivedMessages.txt";

				try
				{
					File test = new File (name);
					File tmp;

					if (test.exists())
					{
						String tmpName = name + ".tmp";

						//Utils.Log ( 3, "ParseStoragePath: copy temp file of messages" );

						tmp = new File(tmpName);
						if (Utils.CopyFile(test, tmp)) {
							name = tmpName;
						}

						InputStream ins = new FileInputStream(name);
						//Utils.Log ( 3, "ParseStoragePath: read messages" );

						BufferedReader reader = new BufferedReader(new InputStreamReader(ins));

						boolean lastPrefixFound = false;
						String unit = "";
						String line;
						while ((line = reader.readLine()) != null)
						{
							if ( MessageInstance.HasPrefix(line) ) {
								if (lastPrefixFound) {
									MessageInstance msg = MessageInstance.Init(unit, this);
									if (msg != null)
										messagesCache.add(msg);
									unit = "";
								}
								else lastPrefixFound = true;
							}
							unit += line;
						}
						if (unit.length() > 10) {
							MessageInstance msg = MessageInstance.Init(unit, this);
							if (msg != null)
								messagesCache.add(msg);
						}
						reader.close();

						ins.close();
						if (tmp.exists())
							tmp.delete();
					}
				}
				catch ( Exception ex ) {
					ex.printStackTrace ();
				}

				//Utils.Log ( 3, className, "ParseStoragePath: read files" );
				try
				{
					File filesInDir = new File(StoragePath);

					File[] filesList = filesInDir.listFiles();
					for (File filePath : filesList)
					{
						//Utils.Log(1, className, "ParseStoragePath: " + filePath.getName());

						if (!filePath.isFile())
							continue;

						String fileIDs = filePath.getName();
						if ( fileIDs.length() <= 4 || fileIDs.contains("receivedMessages.txt") )
							continue;

						int postPos = fileIDs.indexOf ( ".bin" );
						if ( postPos <= 0 )
							continue;

						fileIDs = fileIDs.substring ( 0, postPos );
						try {
							int fileID = Integer.parseInt(fileIDs);
							if (fileID == 0 || files.get(fileID) != null)
								continue;

							FileInstance fileInst = new FileInstance(fileID);

							if (fileInst.Init(this, filePath))
								filesCache.put(fileID, fileInst);
						}
						catch (Exception ei) {
							ei.printStackTrace();
						}
					}
				}
				catch ( Exception ex ) {
					ex.printStackTrace ();
				}

				//Utils.Log ( 3, className, "ParseStoragePath: done parsing" );

				synchronized (messages) {
					storageLoaded = 2;
				}
				return true;
			}
			catch(Exception ex) {
				ex.printStackTrace();
			}
		}
		//Utils.Log ( 3, "ParseStoragePath: done" );
		return false;
	}


	void VerifyStoragePath()
	{
		synchronized (messages) {
			if (StoragePath == null)
				StoragePath = env.GetFilePathForStorage(deviceID, isSameAppArea ? null : areaName, isSameAppArea ? null : appName);
		}
	}

	/**
	 * Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
	 *
	 */
	public void ClearStorage()
	{
		new Thread(new Runnable() {
			public void run(){
				ClearStorageThread();
			}
		}).start();
	}


	@SuppressWarnings ( "ResultOfMethodCallIgnored" )
	void ClearStorageThread()
	{
		try
		{
			VerifyStoragePath();

			synchronized(files)
			{
				files.clear();
				filesCache.clear();

				File filesInDir = new File(StoragePath);

				File[] filesList = filesInDir.listFiles();
				for (File filePath : filesList)
				{
					//Utils.Log(1, className, "ParseStoragePath: " + filePath.getName());

					if (!filePath.isFile())
						continue;

					if (filePath.getName().contains("receivedMessages"))
						continue;

					filePath.delete();
				}
			}
		}
		catch (Exception ex) {
			ex.printStackTrace ();
		}
	}


	public void DisposeStorageCache ()
	{
		messages.clear();
		messagesCache.clear();

		files.clear ();
		filesCache.clear ( );
	}


	/**
	 * Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
	 *
	 */
	public void ClearMessages()
	{
		new Thread(new Runnable() {
			public void run(){
				ClearMessagesThread();
			}
		}).start ( );
	}


	@SuppressWarnings ( "ResultOfMethodCallIgnored" )
	void ClearMessagesThread()
	{
		try
		{
			VerifyStoragePath();

			synchronized (files)
			{
				messages.clear();
				messagesCache.clear();

				String messagePath = StoragePath + "receivedMessages.txt";

				File msgs = new File(messagePath);
				if (msgs.exists())
					msgs.delete();
			}
		}
		catch (Exception ex) {
			ex.printStackTrace ();
		}
	}


	void AddFile(int type, int fileID, String fileDescriptor, String filePath, int bytesToSend, boolean sent)
	{
		if (disposed)
			return;

		FileInstance fileInst = new FileInstance(fileID);
		fileInst.descriptor = fileDescriptor;
		fileInst.path = filePath;
		fileInst.sent = true;
		fileInst.created = System.currentTimeMillis() / 1000L;
		fileInst.size = bytesToSend;
		fileInst.device = this;
		fileInst.type = type;

		files.put(fileID, fileInst );

		if (filesLast >= 0) {
			synchronized ( files ) {
				files.notify ( );
			}
			return;
		}

		NotifyObserversForData(fileInst, Environs.FILE_INFO_ATTR_CREATED, true );
	}


	/**
	 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
	 *
	 * @param fileID        A user-customizable id that identifies the file to be send.
	 * @param fileDescriptor (e.g. filename)
	 * @param filePath      The path to the file to be send.
	 * @return success
	 */
	public boolean SendFile(int fileID, String fileDescriptor, String filePath)
	{
		if ( disposed || !isConnected )
			return false;

		AddFile ( 0, fileID, fileDescriptor, filePath, 0, true );

		return env.SendFile(nativeID, async, fileID, fileDescriptor, filePath);
	}

	/**
	 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
	 *
	 * @param fileID        A user-customizable id that identifies the file to be send.
	 * @param fileDescriptor (e.g. filename)
	 * @param buffer        A buffer to be send.
	 * @param bytesToSend number of bytes in the buffer to send
	 * @return success
	 */
	public boolean SendBuffer(int fileID, String fileDescriptor, byte[] buffer, int bytesToSend)
	{
		if ( disposed || !isConnected )
			return false;

		AddFile(0, fileID, fileDescriptor, "", bytesToSend, true);

		return env.SendBuffer ( nativeID, async, fileID, fileDescriptor, buffer, bytesToSend );
	}


	/**
	 * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
	 * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
	 * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
	 *
	 * @return MessageInstance
	 */
	public FileInstance ReceiveBuffer ()
	{
		if ( filesLast < 0 || disposed || !isConnected )
			return null;

		FileInstance item = null;

		synchronized ( files ) {
			try
			{
				while (!disposed)
				{
					if ( filesLast < files.size () ) {
						item = files.valueAt ( filesLast );

						filesLast++;
						break;
					}
					else {
						files.wait ();
					}
				}
			}
			catch ( Exception ex ) {
				ex.printStackTrace ();
			}
		}

		return item;
	}


	/**
	 * Send a string message to a device through one of the following ways.&nbsp;
	 * If a connection with the destination device has been established, then use that connection.
	 * If the destination device is not already connected, then distinguish the following cases:
	 * (1) If the destination is within the same network, then try establishing a direct connection.
	 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
	 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
	 * 		a STUNT connection with the latest connection details that are available.
	 *
	 * On successful transmission, Environs returns true if the devices already had an active connection,
	 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
	 * a registered EnvironsObserver instance.
	 *
	 * @param message       A message to be send.
	 * @return success
	 */
	public boolean SendMessage(String message)
	{
		return SendMessage ( async, message );
	}

	/**
	 * Send a string message to a device through one of the following ways.&nbsp;
	 * If a connection with the destination device has been established, then use that connection.
	 * If the destination device is not already connected, then distinguish the following cases:
	 * (1) If the destination is within the same network, then try establishing a direct connection.
	 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
	 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
	 * 		a STUNT connection with the latest connection details that are available.
	 *
	 * On successful transmission, Environs returns true if the devices already had an active connection,
	 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
	 * a registered EnvironsObserver instance.
	 *
	 * @param async			(Environs.CALL_NOWAIT) Perform asynchronous. (Environs.CALL_WAIT) Non-async means that this call blocks until the call finished.
	 * @param message       A message to be send.
	 * @return success
	 */
	public boolean SendMessage ( @Call.Value int async, String message)
	{
		if (message == null) return false;

		AddMessage(message, message.length ( ), true, 'u' );

		return env.SendMessage ( deviceID, isSameAppArea ? null : areaName, isSameAppArea ? null : appName, async, message );
	}


	/**
	 * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
	 * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
	 * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
	 *
	 * @return MessageInstance
	 */
	public MessageInstance Receive ()
	{
		if ( !messagesEnqueue )
			return null;

		MessageInstance item = null;

		synchronized ( messages ) {
			try
			{
				while (!disposed)
				{
					if ( messages.size () > 0 ) {
						item = (MessageInstance) messages.remove ();

						break;
					}
					else {
						messages.wait ();
					}
				}
			}
			catch ( Exception ex ) {
				ex.printStackTrace ();
			}
		}

		return item;
	}


	/**
	 * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
	 *
	 * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
	 * @param buffer        A buffer to be send.
	 * @param offset        A user-customizable id that identifies the file to be send.
	 * @param bytesToSend number of bytes in the buffer to send
	 * @return success
	 */
	@SuppressWarnings ( "SimplifiableIfStatement" )
	public boolean SendDataUdp ( @Call.Value int async, byte[] buffer, int offset, int bytesToSend )
	{
		if ( buffer == null || bytesToSend <= 0 || disposed || !isConnected ) return false;

		return Environs.SendDataUdpN ( hEnvirons, nativeID, async, buffer, offset, bytesToSend );
	}


	/**
	 * Receives a buffer send using SendBuffer/SendFile by the DeviceInstance.
	 * This call blocks until a new data has been received or until the DeviceInstance gets disposed.
	 * Data that arrive while Receive is not called will be queued and provided with subsequent calls to Receive.
	 *
	 * @return MessageInstance
	 */
	public byte [] ReceiveData ()
	{
		if (disposed || !isConnected)
			return null;

		byte [] item = null;

		while (!disposed)
		{
			synchronized ( udpData )
			{
				if (!udpData.isEmpty ()) {
					item = ( byte[] ) udpData.remove ( );
					break;
				}

				try {
					udpData.wait ();
				} catch ( InterruptedException e ) {
					e.printStackTrace ( );
					break;
				}
			}
		}

		return item;
	}


	/**
	 * Load the file that is assigned to the fileID received by deviceID into an byte array.
	 *
	 * @param fileID        The id of the file to load (given in the onData receiver).
	 * @return byte-array
	 */
	@Nullable
	public byte[] GetFile(int fileID) {
		if (nativeID > 0)
			return env.GetFileNative(nativeID, fileID);
		return env.GetFile(deviceID, isSameAppArea ? null : areaName, isSameAppArea ? null : appName, fileID);
	}


	/**
	 * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
	 *
	 * @param fileID        The id of the file to load (given in the onData receiver).
	 * @return absolutePath
	 */
	public String GetFilePath(int fileID) {
		if (nativeID > 0)
			return env.GetFilePathNative(nativeID, fileID);
		return env.GetFilePath(deviceID, isSameAppArea ? null : areaName, isSameAppArea ? null : appName, fileID);
	}


	/**
	 * Load the file that is assigned to the fileID received by deviceID into an byte array.
	 *
	 * @param fileID        The id of the file to load (given in the onData receiver).
	 * @return byte-array
	 */
	@Nullable
	public FileInstance GetFileInstance(int fileID)
	{
		return files.get(fileID);
	}
}
