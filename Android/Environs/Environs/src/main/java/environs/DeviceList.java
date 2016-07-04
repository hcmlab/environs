package environs;
/**
 *	DeviceList
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
import java.util.ArrayList;
import android.app.Activity;
import android.content.Context;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

/**
 *	DeviceList management and event provider
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
@SuppressWarnings ( "unused" )
public class DeviceList extends BaseAdapter
{
    private static final String     className = "DeviceList . . . . . . .";

    Environs                        env;
    int                             hEnvirons   = 0;

    private final static Object     listLocker  = new Object();

    @DeviceClass.Value              int listType    = DeviceClass.All;
    protected int                   layout_id;
    protected int                   view_id;
    protected LayoutInflater        inflater;
    protected Activity              context;

    static boolean                  notifyAdapters  = false;
    boolean                         isUIAdapter     = false;
    ViewGenerator                   generator;
    boolean                         useViewAdapter  = true;
	
	final ArrayList<DeviceInstance> deviceList;

    /**
     * Device list change notifications are conducted through the implementation of
     * the environs.ListObserver interface.
     *
     * The execution context is not guaranteed to be the UI thread context!
     * List changes are notified through onListChanged()
     * Device instance changes are notified through onItemChanged()
     *
     */
    final ArrayList<ListObserver>   observers;

    public void AddObserver(ListObserver observer)
    {
        synchronized ( observers )
        {
            if (observer != null && !observers.contains(observer))
                observers.add ( observer );
        }
    }


    public void RemoveObserver(ListObserver observer)
    {
        synchronized ( observers )
        {
            if (observer != null && !observers.contains(observer))
                observers.remove ( observer );
        }
    }


	public DeviceList ( @NonNull Activity context, @DeviceClass.Value int deviceClass, int layout_id, int view_id ) {
        this ( null, context, deviceClass, null, layout_id, view_id );
	}


    public DeviceList ( @NonNull Activity context, @DeviceClass.Value int deviceClass, ListObserver observer, int layout_id, int view_id ) {
        this ( null, context, deviceClass, observer, layout_id, view_id );
    }


    DeviceList ( Environs obj, Activity context, @DeviceClass.Value int deviceClass, ListObserver observer, int layout_id, int view_id ) {
        if (env == null)
            env = Environs.GetLatestInstance();
        else
            env = obj;

        if ( env == null ) {
            throw new IllegalArgumentException (  );
        }

        //Utils.Log ( 4, className, "Construct: Adding to deviceLists container." );
        hEnvirons = env.hEnvirons;
        env.deviceLists.add ( this );

        inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        this.context = context;
        this.layout_id = layout_id;
        this.view_id = view_id;

        if (deviceClass == DeviceClass.Nearby)
            observers = env.listNearbyObservers;
        else if (deviceClass == DeviceClass.Mediator)
            observers = env.listMediatorObservers;
        else
            observers = env.listAllObservers;

        if (observer != null)
            AddObserver(observer);

        deviceList = GetDevices(env, listType );

        listType = deviceClass;

        //Utils.Log ( 4, className, "Construct: Adding notifyAdapter" );
        notifyAdapters = isUIAdapter = true;
        env.isUIAdapter = true;
    }


    void SetGenerator(ViewGenerator gen)
    {
        if (gen == null)
            return;
        if (Utils.isDebug) Utils.Log ( 4, className, "SetGenerator" );

        generator = gen;
        gen.init ( inflater, layout_id, view_id );

        useViewAdapter = true;
    }


    public void finalize() {
        try {
            if (Utils.isDebug) Utils.Log ( 4, className, "finalize");

            super.finalize();

            Release();

        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }


    static void UpdateNotifyAdapterStatus ( ArrayList<DeviceList> deviceLists )
    {
        int i=0;
        for (; i < deviceLists.size(); i++)
        {
            DeviceList adapter = deviceLists.get(i);
            if (adapter.isUIAdapter )
                break;
        }
        if (i >= deviceLists.size())
            DeviceList.notifyAdapters = false;
    }


	public void Release () {
        if (env != null && env.deviceLists != null) {
            env.deviceLists.remove ( this );
            if ( isUIAdapter )
                UpdateNotifyAdapterStatus ( env.deviceLists );
        }
    }


    static boolean IsUIThread () {
        return (Looper.myLooper() == Looper.getMainLooper ( ));
    }
 	
	@Override
    public int getCount() {
		return GetCount ( );
	}
    public int GetCount() {
        if ( deviceList == null )
            return 0;

        if (isUIAdapter && !IsUIThread () ) {
            synchronized ( deviceList ) {
                return deviceList.size();
            }
        }
        return deviceList.size ( );
    }


    @Override
    @Nullable
    public Object getItem(int position) {
        return GetItem ( position );
    }


    @Nullable
	public Object GetItem(int position) {
		if ( deviceList == null )
			return null;

        if (isUIAdapter && !IsUIThread () ) {
            synchronized ( deviceList ) {
                return GetItemDo ( position );
            }
        }
        return GetItemDo ( position );
    }


    @Nullable
    Object GetItemDo(int position) {
        if ( position < deviceList.size() )
            return deviceList.get(position);
        return null;
    }


	@Override
	public long getItemId(int position) {
		if ( deviceList == null )
			return 0;

        if (isUIAdapter && !IsUIThread () ) {
            synchronized (deviceList) {
                if ( position < deviceList.size() )
                    return deviceList.get(position).deviceID;
            }
        }

        if ( position < deviceList.size() )
            return deviceList.get(position).deviceID;
        return 0;
	}


	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
        do {
            //Utils.Log ( 6, className, "getView: " + position );

            try {
                DeviceInstance device = (DeviceInstance) getItem(position);
                if ( device == null )
                    break;

                if (useViewAdapter) {
                    if (generator != null ) {
                        View genView = generator.getView ( position, device, convertView, parent);

                        if ( genView != null )
                            return genView;
                    }
                    useViewAdapter = false;
                }

                if ( convertView == null ) {
                    if (inflater == null)
                        break;

                    convertView = inflater.inflate(layout_id, null);
                    if (convertView == null)
                        break;
                }

                TextView tv = (TextView) convertView.findViewById(view_id);
                if (tv == null)
                    break;

                tv.setText(device.toString());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        while(false);

        return convertView;
	}


    @SuppressWarnings ( "all" )
    @Nullable
    public DeviceInstance RefreshItem(DeviceInstance source, DeviceObserver observer )
    {
        ArrayList<DeviceInstance> deviceList = null;

        if (env.listAll != null) {
            deviceList = env.listAll;
        }
        else if (env.listNearby != null) {
            deviceList = env.listNearby;
        }
        else if (env.listMediator != null) {
            deviceList = env.listMediator;
        }

        if (deviceList == null) {
            deviceList = GetDevicesSource ( );
        }

        synchronized (deviceList)
        {
            int count = deviceList.size();

            for ( int i=0; i<count; i++ )
            {
                DeviceInstance device = deviceList.get(i);

                if (device != null && device.EqualsID(source)) {
                    if ( observer != null )
                        device.AddObserver(observer);
                    return device;
                }
            }
        }

        return null;
    }


    public void DisposeList ()
    {
        synchronized (listLocker) {
            ArrayList<DeviceInstance> list = null;

            switch ( listType ) {
                case DeviceClass.All:
                    list = env.listAll;
                    break;

                case DeviceClass.Nearby:
                    list = env.listNearby;
                    break;

                case DeviceClass.Mediator:
                    list = env.listMediator;
                    break;
            }

            if (list != null)
                DisposeList ( env, env.isUIAdapter, list );
        }
    }


    public static boolean isPlatformType(int src, int platform)
    {
        return ((src & platform) == platform);
    }


    @Nullable
    public ArrayList<DeviceInstance> devices () {
        return deviceList;
    }


    @SuppressWarnings ( "all" )
    @Nullable
 	static ArrayList<DeviceInstance> GetDevices(Environs env, @DeviceClass.Value int type)
 	{
        synchronized (listLocker) {
            switch(type) {
                case DeviceClass.All:
                    if ( env.listAll == null ) {
                        env.listAll = new ArrayList<DeviceInstance>();

                        if ( env.listNearby != null )
                            DisposeList ( env, env.isUIAdapter, env.listNearby );
                        if ( env.listMediator != null )
                            DisposeList ( env, env.isUIAdapter, env.listMediator );

                        env.DeviceListUpdate ( type );
                    }
                    return env.listAll;

                case DeviceClass.Nearby:
                    if ( env.listNearby == null ) {
                        env.listNearby = new ArrayList<DeviceInstance>();
                        if (env.listAll == null)
                            env.DeviceListUpdate ( type );
                        else
                            env.DeviceListUpdate ( DeviceClass.All );
                    }
                    return env.listNearby;

                case DeviceClass.Mediator:
                    if ( env.listMediator == null ) {
                        env.listMediator = new ArrayList<DeviceInstance>();
                        if (env.listAll == null)
                            env.DeviceListUpdate ( type );
                        else
                            env.DeviceListUpdate ( DeviceClass.All );
                    }
                    return env.listMediator;
            }
        }
 		return null;
 	}


    static class NotifierContext
    {
        int type;
        @DeviceClass.Value int listType;
        DeviceInstance device;
    }


    static void NotifierThread ( Environs env )
    {
        NotifierContext ctx;

        while ( env.listNotifierThreadRun )
        {
            synchronized ( env.listNotifierQueue )
            {
                if ( env.listNotifierQueue.size() == 0 )
                {
                    try {
                        env.listNotifierQueue.wait ();
                    } catch ( InterruptedException e ) {
                        e.printStackTrace ( );
                        break;
                    }
                    continue;
                }
                else {
                    ctx = (NotifierContext) env.listNotifierQueue.remove();

                    if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: Dequeue");
                }
            }

            if ( ctx != null )
            {
                switch ( ctx.type ) {
                    case 1:
                        NotifyObservers ( env, ctx.device, ctx.listType, false );
                        break;
                    case 2:
                        NotifyDatasetChanged ( env, false );
                        break;
                }

                if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: next");
            }
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "NotifierThread: done" );

        synchronized ( env.listNotifierQueue ) {
            env.listNotifierThread = null;
        }
    }


    @SuppressWarnings ( "unchecked" )
    static void EnqueueNotification ( Environs env, NotifierContext ctx )
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "EnqueueNotification");

        if ( ctx == null ) return;

        synchronized ( env.listNotifierQueue )
        {
            if ( !env.listUpdateThreadRun ) return;

            env.listNotifierQueue.add ( ctx );

            env.listNotifierQueue.notify ( );
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "EnqueueNotification: done" );
    }


    public void OnItemChanged(Object sender, int DEVICE_INFO_ATTR_changed)
    {
        context.runOnUiThread ( new Runnable ( ) {
            @Override
            public void run ( ) {
                notifyDataSetChanged ( );
            }
        } );
    }


    @SuppressWarnings ( "all" )
    static void NotifyObservers ( @NonNull Environs env, final DeviceInstance device, @DeviceClass.Value int listType, boolean enqueue )
    {
        if (enqueue) {
            NotifierContext ctx = new NotifierContext ( );
            ctx.type = 1;
            ctx.device = device;
            ctx.listType = listType;

            EnqueueNotification ( env, ctx );
            if (Utils.isDebug) Utils.Log ( 10, className, "NotifyObservers: Enqueued" );
            return;
        }

        if (Utils.isDebug) Utils.Log ( 10, className, "NotifyObservers: working ..." );
        ArrayList<ListObserver> obs = null;
        ArrayList<ListObserver> observers = null;


        Environs.ListContext listContext = null;

        switch ( listType ) {
            case DeviceClass.All :
                observers = env.listAllObservers;
                listContext  = env.contextAll;
                break;

            case DeviceClass.Nearby :
                observers = env.listNearbyObservers;
                listContext  = env.contextNearby;
                break;

            default:
                observers = env.listMediatorObservers;
                listContext  = env.contextMediator;
                break;
        }

        synchronized ( observers )
        {
            obs = (ArrayList<ListObserver>) observers.clone ();
        }

        if (obs.size () <= 0)
        {
            if (Utils.isDebug) Utils.Log ( 10, className, "NotifyObservers: no observers ..." );
        }
        else {
            ArrayList < DeviceInstance >    vanished;
            ArrayList < DeviceInstance >    appeared;

            synchronized ( listContext )
            {
                vanished = listContext.vanished; listContext.vanished = null;
                appeared = listContext.appeared; listContext.appeared = null;
            }

            if (Utils.isDebug) Utils.Log ( 10, className, "NotifyObservers: listType [ " + listType + " ], vanished [ " + (vanished == null ? "0" : vanished.size ())
                    + " ], appeared [ " + (appeared == null ? "0" : appeared.size ()) + " ]" );

            if ((vanished == null || vanished.size () <= 0) && (appeared == null || appeared.size () <= 0))
            {
                if (Utils.isDebug) Utils.Log ( 2, className, "NotifyObservers: nothing to do ..." );
                return;
            }

            for (int i=0; i<obs.size(); i++)
            {
                ListObserver observer = obs.get(i);

                try {
                    observer.OnListChanged ( vanished, appeared );
                } catch (Exception e) {
                    e.printStackTrace ( );
                }
            }
        }

        if (notifyAdapters)
            NotifyDatasetChanged ( env, false );
    }


    static void NotifyDatasetChanged ( @NonNull Environs env, boolean enqueue )
    {
        if (enqueue) {
            NotifierContext ctx = new NotifierContext();
            ctx.type    = 2;

            EnqueueNotification ( env, ctx );
            return;
        }
        if (Utils.isDebug) Utils.Log ( 4, className, "NotifyDatasetChanged" );

        for (int i=0; i < env.deviceLists.size(); i++) {

            final DeviceList adapter = env.deviceLists.get(i);
            if (Utils.isDebug) Utils.Log ( 4, className, "NotifyDatasetChanged: Checking " + i );

            if ( !adapter.isUIAdapter ) // || !adapter.deviceList.contains ( device ) )
                continue;

            if (Utils.isDebug) Utils.Log ( 4, className, "NotifyDatasetChanged: " + i );
            adapter.context.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    adapter.notifyDataSetChanged();
                }
            });
        }
    }


    static void OnDeviceListNotification ( @NonNull Environs env ) {
        if (env.listAll != null)
            env.DeviceListUpdate ( DeviceClass.All );
        else {
            if (env.listNearby != null)
                env.DeviceListUpdate ( DeviceClass.Nearby );

            if (env.listMediator != null)
                env.DeviceListUpdate ( DeviceClass.Mediator );
        }
    }


    static void ListUpdateThread ( @NonNull Environs env )
    {
        ObserverNotifyContext ctx;

        while ( env.listUpdateThreadRun )
        {
            synchronized (env.listUpdateQueue)
            {
                if ( env.listUpdateQueue.size() == 0 )
                {
                    try {
                        env.listUpdateQueue.wait ();
                    } catch ( InterruptedException e ) {
                        e.printStackTrace ( );
                        break;
                    }
                    continue;
                }
                else {
                    ctx = (ObserverNotifyContext)env.listUpdateQueue.remove();

                    if (Utils.isDebug) Utils.Log ( 6, className, "ListUpdateThread: Dequeue");
                }
            }

            if ( ctx != null )
            {
                OnDeviceListNotificationDo ( ctx );
                continue;
            }

            if (Utils.isDebug) Utils.Log ( 6, className, "ListUpdateThread: next");
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "ListUpdateThread: done" );

        synchronized ( env.listUpdateQueue ) {
            env.listUpdateThread = null;
        }
    }


    @SuppressWarnings ( "unchecked" )
    static void OnDeviceListNotification ( final ObserverNotifyContext ctx )
    {
        //Utils.Log(6, className, "OnDeviceListNotification: " + ctx.destID + " Notif: " + Environs.resolveName(ctx.notification));

        synchronized (ctx.env.listUpdateQueue) {
            ctx.env.listUpdateQueue.add ( ctx );

            ctx.env.listUpdateQueue.notify ( );
        }
    }


    static void OnDeviceListNotificationDo ( final ObserverNotifyContext ctx )
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "OnDeviceListNotification: " + ctx.destID + " Notif: " + Environs.resolveName(ctx.notification));

        DeviceInstance device;
        Environs env = Environs.instances[ctx.hEnvirons];

        switch (ctx.notification)
        {
            case Environs.NOTIFY_MEDIATOR_DEVICE_REMOVED:
            case Environs.NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED:
                RemoveDevice ( env, ctx );
                break;

            case Environs.NOTIFY_MEDIATOR_DEVICE_CHANGED:
            case Environs.NOTIFY_MEDIATOR_DEVICE_ADDED:
            case Environs.NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED:
            case Environs.NOTIFY_MEDIATOR_SRV_DEVICE_ADDED:
                if (ctx.contextPtr != null) {
                    device = Environs.BuildDeviceInfo ( ctx.hEnvirons, ctx.contextPtr );
                }
                else
                    device = env.GetDeviceByObjID0 ( ctx.destID );

                UpdateDevice ( env, ctx, device );
                break;

            default:
                break;
        }
    }


    @SuppressWarnings ( "all" )
    static DeviceInstance GetDevice ( @NonNull ArrayList<DeviceInstance> deviceList, int deviceID, String areaName, String appName)
    {
        synchronized (deviceList)
        {
            for (int i=0; i<deviceList.size(); i++) {
                DeviceInstance device = deviceList.get(i);
                if (device != null && device.EqualsID(deviceID, areaName, appName))
                    return device;
            }
        }

        return null;
    }


    static void OnDeviceListNotification ( DeviceInstance device, int notification, int sourceIdent, int context )
    {
        if (notification == Environs.NOTIFY_CONTACT_DIRECT_CHANGED)
        {
            device.SetDirectContact(sourceIdent);
        }
        else if (notification == Environs.NOTIFY_FILE_SEND_PROGRESS)
        {
            if (Utils.isDebug) Utils.Log ( 6, className, "OnDeviceListNotification: send fileID [" + sourceIdent + "] progress: [" + context + "]");

            device.SetFileProgress(sourceIdent, context, true);
        }
        else if (notification == Environs.NOTIFY_FILE_RECEIVE_PROGRESS)
        {
            if (Utils.isDebug) Utils.Log ( 6, className, "OnDeviceListNotification: receive fileID [" + sourceIdent + "] progress: [" + context + "]");

            device.SetFileProgress(sourceIdent, context, false);
        }
    }


    static void OnDeviceListNotification ( Environs env, ObserverNotifyContext ctx )
    {
        if ( ctx.notification == Notify.Options.DirectContactChanged )
        {
            if (env.listAll != null) {
                DeviceInstance device = GetDevice(env, ctx.destID);
                if ( device != null )
                    OnDeviceListNotification(device, ctx.notification, ctx.sourceIdent, ctx.context);
            }
            else
            {
                if (env.listNearby != null) {
                    DeviceInstance device = GetDeviceNearby(env, ctx.destID);
                    if ( device != null )
                        OnDeviceListNotification(device, ctx.notification, ctx.sourceIdent, ctx.context);
                }

                if (env.listMediator != null) {
                    DeviceInstance device = GetDeviceFromMediator(env, ctx.destID);
                    if (device != null)
                        OnDeviceListNotification(device, ctx.notification, ctx.sourceIdent, ctx.context);
                }
            }
        }
    }


    @SuppressWarnings ( "all" )
    @Nullable
    static DeviceInstance GetDevice ( @NonNull ArrayList<DeviceInstance> deviceList, int objID )
    {
        synchronized (deviceList)
        {
            for (int i=0; i<deviceList.size(); i++)
            {
                DeviceInstance device = deviceList.get(i);

                if (device != null && (device.objID == objID || device.objIDPrevious == objID) )
                    return device;
            }
        }

        return null;
    }

    /**
     * Get a collection that holds all available devices. This list ist updated dynamically by Environs.
     *
     * @return ArrayList with DeviceInstance objects
     */
    @SuppressWarnings ( "all" )
    public ArrayList<DeviceInstance> GetDevices () {
        ArrayList<DeviceInstance> list = GetDevices ( env, DeviceClass.All );
        if (list != null) {
            synchronized ( list ) {
                list = ( ArrayList< DeviceInstance > ) list.clone ( );
            }
        }
        return list;
    }

    public ArrayList<DeviceInstance> GetDevicesSource ( ) {
        return GetDevices ( env, DeviceClass.All );
    }
    static ArrayList<DeviceInstance> GetDevices(Environs env) {
        return GetDevices ( env, DeviceClass.All );
    }

    /**
     * Query the number of all available devices within the environment (including those of the Mediator)
     *
     * @return numberOfDevices
     */
    public int GetDevicesCount() {
        return GetDevicesSource ( ).size ( );
    }

    /**
     * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
     *
     * @param deviceID      The device id of the target device.
     * @param areaName	    Area name of the application environment
     * @param appName		Application name of the application environment
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDevice ( int deviceID, String areaName, String appName )
    {
        return GetDevice ( GetDevicesSource ( ), deviceID, areaName, appName );
    }

    static DeviceInstance GetDevice(Environs env, int deviceID, String areaName, String appName)
    {
        return GetDevice ( GetDevices ( env ), deviceID, areaName, appName );
    }

    /**
     * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
     *
     * @param objID      The native device id of the target device.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDevice ( int objID )
    {
        return GetDevice ( env, objID, true );
    }

    static DeviceInstance GetDevice ( Environs env, int objID )
    {
        return GetDevice ( env, objID, true );
    }

    /**
     * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
     *
     * @param nativeID      The native device id of the target device.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceByNativeID(int nativeID)
    {
        synchronized (deviceList)
        {
            for (int i=0; i<deviceList.size(); i++)
            {
                DeviceInstance device = deviceList.get(i);

                if (device.nativeID == nativeID)
                    return device;
            }
        }
        return null;
    }

    @SuppressWarnings ( "all" )
    static DeviceInstance GetDeviceByNativeID(Environs env, int nativeID)
    {
        ArrayList<DeviceInstance> deviceList = null;

        if (env.listAll != null)
            deviceList = env.listAll;
        else if (env.listNearby != null)
            deviceList = env.listNearby;
        else if (env.listMediator != null)
                deviceList = env.listMediator;

        if (deviceList == null)
            deviceList = GetDevices(env);

        synchronized (deviceList)
        {
            for (int i=0; i<deviceList.size(); i++)
            {
                DeviceInstance device = deviceList.get(i);

                if (device.nativeID == nativeID)
                    return device;
            }
        }
        return null;
    }


    /**
     * Query a DeviceInstance object that best match the deviceID only.
     * Usually the one that is in the same app environment is picked up.
     * If there is no matching in the app environment,
     * then the project environments are searched for a matchint deviceID.
     *
     * @param deviceID      The portalID that identifies an active portal.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceBestMatchNative(int deviceID)
    {
        DeviceInstance device = Environs.GetDevice0(hEnvirons, deviceID);
        if (device != null)
            return GetDevice(device.deviceID, device.areaName, device.appName);
        return null;
    }

    /**
     * Query a DeviceInstance object that first match the deviceID only.
     * Usually the one that is in the same app environment is picked up.
     * If there is no matching in the app environment,
     * then the project environments are searched for a matchint deviceID.
     *
     * @param deviceID      The portalID that identifies an active portal.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceBestMatch(int deviceID)
    {
        return GetDevice(env, deviceID, false);
    }

    @SuppressWarnings ( "all" )
    static public DeviceInstance GetDevice ( @NonNull Environs env, int objOrDeviceID, boolean isObjID )
    {
        ArrayList<DeviceInstance> deviceList = null;

        if (env.listAll != null)
            deviceList = env.listAll;
        else if (env.listNearby != null)
            deviceList = env.listNearby;
        else if (env.listMediator != null)
            deviceList = env.listMediator;

        if (deviceList == null)
            deviceList = GetDevices(env);

        synchronized (deviceList)
        {
            for (int i=0; i<deviceList.size(); i++)
            {
                DeviceInstance device = deviceList.get(i);

                if (isObjID) {
                    if (device.objID == objOrDeviceID || device.objIDPrevious == objOrDeviceID)
                        return device;
                }
                else {
                    if (device.deviceID == objOrDeviceID)
                        return device;
                }
            }
        }
        return null;
    }

 	/**
     * Release the ArrayList that holds the available devices.
     */
 	public void ReleaseDevices() {
        DisposeList ( env, isUIAdapter, env.listAll );
        env.listAll = null;
 	}


    /**
     * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
     *
     * @return ArrayList with DeviceInstance objects
     */
    @SuppressWarnings ( "all" )
    public ArrayList<DeviceInstance> GetDevicesNearby() {
        ArrayList<DeviceInstance> list = GetDevices ( env, DeviceClass.Nearby );
        if (list != null) {
            synchronized ( list ) {
                list = ( ArrayList< DeviceInstance > ) list.clone ( );
            }
        }
        return list;
    }
    public ArrayList<DeviceInstance> GetDevicesNearbySource ( ) {
        return GetDevices ( env, DeviceClass.Nearby );
    }

    static ArrayList<DeviceInstance> GetDevicesNearby(Environs env) {
        return GetDevices ( env, DeviceClass.Nearby );
    }

    /**
     * Query the number of nearby (broadcast visible) devices within the environment.
     *
     * @return numberOfDevices
     */
    public int GetDevicesNearbyCount() {
        return GetDevicesNearbySource ( ).size ( );
    }

    /**
     * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
     *
     * @param deviceID      The device id of the target device.
     * @param areaName	    Area name of the application environment
     * @param appName		Application name of the application environment
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceNearby(int deviceID, String areaName, String appName)
    {
        return GetDevice ( GetDevicesNearbySource ( ), deviceID, areaName, appName );
    }

    /**
     * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
     *
     * @param nativeID      The native device id of the target device.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceNearby(int nativeID)
    {
        return GetDevice ( GetDevicesNearbySource ( ), nativeID );
    }
    static DeviceInstance GetDeviceNearby(Environs env, int objID)
    {
        return GetDevice(GetDevicesNearby ( env ), objID);
    }

 	/**
     * Release the ArrayList that holds the nearby devices.
     */
 	public void ReleaseDevicesNearby() {
        DisposeList ( env, isUIAdapter, env.listNearby );
        env.listNearby = null;
 	}


 	/**
     * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
     * 
     * @return ArrayList with DeviceInstance objects
     */
    @SuppressWarnings ( "all" )
    public ArrayList<DeviceInstance> GetDevicesFromMediator() {
        ArrayList<DeviceInstance> list = GetDevices ( env, DeviceClass.Mediator );
        if (list != null) {
            synchronized ( list ) {
                list = ( ArrayList< DeviceInstance > ) list.clone ( );
            }
        }
        return list;
    }

    public ArrayList<DeviceInstance> GetDevicesFromMediatorSource ( ) {
        return GetDevices ( env, DeviceClass.Mediator );
    }

    static ArrayList<DeviceInstance> GetDevicesFromMediator(Environs env) {
        return GetDevices ( env, DeviceClass.Mediator );
    }

    /**
     * Query a DeviceInstance object of Mediator managed devices within the environment.
     *
     * @param deviceID      The device id of the target device.
     * @param areaName	    Area name of the application environment
     * @param appName		Application name of the application environment
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceFromMediator(int deviceID, String areaName, String appName)
    {
        return GetDevice ( GetDevicesFromMediatorSource ( ), deviceID, areaName, appName );
    }

    /**
     * Query a DeviceInstance object of Mediator managed devices within the environment.
     *
     * @param nativeID      The native device id of the target device.
     * @return DeviceInstance-object
     */
    @Nullable
    public DeviceInstance GetDeviceFromMediator(int nativeID)
    {
        return GetDevice ( GetDevicesFromMediatorSource ( ), nativeID );
    }
    static DeviceInstance GetDeviceFromMediator(Environs env, int objID)
    {
        return GetDevice ( GetDevicesFromMediator ( env ), objID );
    }

    /**
     * Query the number of Mediator managed devices within the environment.
     *
     * @return numberOfDevices
     */
    public int GetDevicesFromMediatorCount() {
        return GetDevicesFromMediatorSource ( ).size ( );
    }

 	/**
     * Release the ArrayList that holds the Mediator server devices.
     */
 	public void ReleaseDevicesMediator() {
        DisposeList ( env, isUIAdapter, env.listMediator );
        env.listMediator = null;
 	}


    @SuppressWarnings ( "all" )
    static void DisposeList ( Environs env, boolean isUIAdapter, @NonNull ArrayList<DeviceInstance> list )
    {
        if (list != null && list.size() > 0) {
            for (int i=0; i<list.size(); i++) {
                DeviceInstance device = list.get(i);
                device.Dispose ();
                //device.disposed = true;
                device.NotifyObservers(Environs.ENVIRONS_OBJECT_DISPOSED, true);
            }

            if (/*isUIAdapter &&*/ !IsUIThread ()) {
                synchronized ( list ) {
                    list.clear();
                }
            }
            else
                list.clear();

            if (isUIAdapter)
                NotifyDatasetChanged ( env, false );
        }
    }


    void DisposeLists()
    {
        env.DisposeLists ( );
    }


    /**
     * Reload all device lists. Applications may call this if they manually stopped and started Environs again.
     * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
     */
    public void ReloadLists()
    {
        env.ReloadLists ( );
    }

    /**
     * Reload the device list.
     */
    public void Reload()
    {
        env.DeviceListUpdate ( listType );
    }


    static boolean doNotifyDeviceListUpdater = false;

    /// <summary>
    /// Devicelist update thread.
    /// </summary>
    @SuppressWarnings ( "all" )
    static void DeviceListUpdater(final Environs env, final int listType)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "DeviceListUpdater: Devicelist update thread started");

        DeviceInstance[] devices;

        ArrayList<DeviceInstance> deviceList;
        ArrayList<ListObserver>	observers;

        final ArrayList<DeviceInstance> vanished = new ArrayList<DeviceInstance>();
        final ArrayList<DeviceInstance> appeared = new ArrayList<DeviceInstance>();

        switch (listType)
        {
            case Environs.MEDIATOR_DEVICE_CLASS_ALL:
                deviceList = env.listAll;
                devices = env.GetDevices0();
                observers = env.listAllObservers;
                break;

            case Environs.MEDIATOR_DEVICE_CLASS_MEDIATOR:
                deviceList = env.listMediator;
                devices = env.GetDevicesFromMediator0();
                observers = env.listMediatorObservers;
                break;

            case Environs.MEDIATOR_DEVICE_CLASS_NEARBY:
                deviceList = env.listNearby;
                devices = env.GetDevicesNearby0();
                observers = env.listNearbyObservers;
                break;

            default:
                Utils.LogE(className, "DeviceListUpdater: Unknown devicelist of type: [" + listType + "]");
                return;
        }

        if (deviceList == null)
        {
            Utils.LogE(className, "DeviceListUpdater: Devicelist of type: [" + listType + "] is missing.");
            return;
        }

        boolean doNotify = false;

        final ArrayList<DeviceInstanceUpdateContext> updates = new ArrayList<DeviceInstanceUpdateContext> ();

        synchronized (deviceList)
        {
            if ( env.isUIAdapter && !IsUIThread () ) {
                final ArrayList<DeviceInstance> devList = deviceList;
                final DeviceInstance[] devs = devices;

                Utils.RunSync ( env.GetClient (), new Runnable ( ) {
                    @Override
                    public void run ( ) {
                        doNotifyDeviceListUpdater = DeviceListUpdaterDo ( env, listType, devList, devs, vanished, appeared, updates );

                        synchronized ( this ) {
                            this.notify ();
                        }
                    }
                } );

                doNotify = doNotifyDeviceListUpdater;
            }
            else {
                doNotify = DeviceListUpdaterDo ( env, listType, deviceList, devices, vanished, appeared, updates );
            }
        }

        for ( int i = 0; i < vanished.size (); i++ ) {
            DeviceInstance device = vanished.get ( i );
            if ( device != null )
                device.Dispose ();
        }

        if ( updates.size () > 0 )
            if ( DeviceInstanceUpdatesApply ( updates ) )
                doNotify = true;

        if ( appeared.size () > 0 ) {
            for ( int i = 0; i < appeared.size (); i++ ) {
                DeviceInstance device = appeared.get ( i );
                if ( device != null && (device.flags & DeviceFlagsInternal.PlatformReady ) == 0 )
                {
                    device.flags |= DeviceFlagsInternal.PlatformReady;

                    Environs.SetDeviceFlagsN ( env.hEnvirons, Call.Wait, device.objID, DeviceFlagsInternal.PlatformReady, true );
                }
            }
        }

        if (doNotify) {
            if (Utils.isDebug) Utils.Log ( 6, className, "DeviceListUpdater: Notify observers.");
            //if (newDevices.size() > 0)
            //    DeviceInstance.ParseAllFiles(newDevices);

            env.AddToListContainer ( listType, vanished, appeared );

            NotifyObservers ( env, null, listType, true );

            //Environs.BridgeForNotifier(env.hEnvirons, Environs.NOTIFY_MEDIATOR_DEVICELISTS_CHANGED, Environs.SOURCE_NATIVE, 0);
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "DeviceListUpdater: Devicelist update thread done.");
    }

    /// <summary>
    /// Devicelist update thread.
    /// </summary>
    @SuppressWarnings ( "ForLoopReplaceableByForEach" )
    static boolean DeviceListUpdaterDo(Environs env, int listType, ArrayList<DeviceInstance> deviceList, DeviceInstance[] devices,
                                    ArrayList<DeviceInstance> vanished, ArrayList<DeviceInstance> appeared, ArrayList<DeviceInstanceUpdateContext> updates
                                    )
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "DeviceListUpdaterDo");

        boolean doNotify = false;

        int newCount;

        if (devices == null)
        {
            if (deviceList.size () > 0)
            {
                if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Clearing deviceList.");
                for (int i=0; i<deviceList.size(); i++) {
                    vanished.add ( deviceList.get ( i ) );
                }

                deviceList.clear();
                doNotify = true;
            }
        }
        else
        {
            newCount = devices.length;

            if (deviceList.size() == 0)
            {
                if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Adding all devices to empty deviceList.");

                for ( DeviceInstance device : devices ) {
                    if (device.deviceID == 0)
                        continue;
                    deviceList.add ( device );
                    appeared.add ( device );
                }
                doNotify = true;
            }
            else
            {
                int listIndex = 0;

                for (int i=0; i<devices.length; i++)
                {
                    boolean itemHandled = false;
                    DeviceInstance item = devices[i];

                    if (item.deviceID == 0)
                        continue;

                    while (listIndex < deviceList.size())
                    {
                        DeviceInstance device = deviceList.get(listIndex);

                        if (item.deviceID < device.deviceID)
                        {
                            /// Insert the new device
                            deviceList.add(listIndex, item);
                            doNotify = true;

                            if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Device [0x" + Integer.toHexString(item.deviceID) + "] appeared, " + item.objID);
                            listIndex++; itemHandled = true;
                            appeared.add ( item );
                            break;
                        }

                        if (item.isSameAppArea) //if (device.EqualsAppEnv(item))
                        {
                            if (item.deviceID == device.deviceID) /// Device found
                            {
                                if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Updating device [0x" + Integer.toHexString(item.deviceID) + "], " + item.objID);

                                DeviceInstanceUpdateContext upd = new DeviceInstanceUpdateContext ();
                                upd.device = device;
                                upd.deviceInfo = item;
                                updates.add ( upd );

                                listIndex++; itemHandled = true;
                                break;
                            }

                            deviceList.remove ( listIndex );

                            /// id of new item > id of current device in list and has not been handled, so the current device must be gone..
                            vanished.add ( device );

                            doNotify = true;
                            if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Device [0x" + Integer.toHexString(item.deviceID) + "] vanished, " + item.objID);

                            /// Stop comparing if we reached the end of the list
                            if (listIndex >= deviceList.size())
                                break;
                            else
                                continue;
                        }
                        else
                        {
                            if (device.LowerThanAppEnv(item) || item.deviceID > device.deviceID) /// We have found a device that is not available anymore
                            {
                                deviceList.remove ( listIndex );
                                /// id of new item > id of current device in list and has not been handled, so the current device must be gone..
                                vanished.add ( device );

                                doNotify = true;
                                if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Device [0x" + Integer.toHexString(item.deviceID) + "] vanished, " + item.objID);
                                continue;
                            }
                        }

                        /// Continue with the next
                        listIndex++;
                    }

                    if (!itemHandled && listIndex >= deviceList.size())
                    {
                        /// Add the new item to the list if we reached the end of the devicelist
                        ///
                        deviceList.add(item);
                        doNotify = true;
                        appeared.add ( item );
                        if (Utils.isDebug) Utils.Log ( 4, className, "DeviceListUpdaterDo: Device [0x" + Integer.toHexString(item.deviceID) + "] appeared, " + item.objID);
                    }
                }

                if (newCount < deviceList.size())
                {
                    while (listIndex < deviceList.size())
                    {
                        DeviceInstance item = deviceList.get(listIndex);
                        deviceList.remove ( listIndex );

                        vanished.add ( item );
                        doNotify = true;
                    }
                }
            }
        }

        TakeOverToOtherLists ( env, listType, vanished );

        if (Utils.isDebug) Utils.Log ( 6, className, "DeviceListUpdaterDo: DeviceList update thread done." );
        return doNotify;
    }


    static void TakeOverToOtherLists ( Environs api, int listType, ArrayList<DeviceInstance> vanished )
    {
        if ( listType == Environs.MEDIATOR_DEVICE_CLASS_ALL )
        {
            if (api.listNearby != null)
                TakeOverToList(api, api.listNearby, false, vanished);

            if (api.listMediator != null)
                TakeOverToList(api, api.listMediator, true, vanished);
        }
    }

    @SuppressWarnings ( "all" )
    static void TakeOverToList ( @NonNull Environs env, @NonNull ArrayList<DeviceInstance> list, boolean getMediator, ArrayList<DeviceInstance> vanished )
    {
        int listIndex = 0;
        int listAllIndex = 0;

        synchronized (list)
        {
            int listCount = list.size();

            int listAllCount = env.listAll.size();

            while (listAllIndex < listAllCount && listIndex < listCount)
            {
                DeviceInstance itemAll = env.listAll.get(listAllIndex);
                DeviceInstance item = list.get(listIndex);

                // Check if we have the same item
                if (item == itemAll)
                {
                    listAllIndex++; listIndex++;
                    continue;
                }

                if (getMediator)
                {
                    if (itemAll.sourceType == DeviceSourceType.Broadcast)
                    {
                        listAllIndex++;
                        continue;
                    }
                }
                else
                {
                    if (itemAll.sourceType == DeviceSourceType.Mediator)
                    {
                        listAllIndex++;
                        continue;
                    }
                }

                if ( item.LowerThanAppEnv(itemAll) && item.deviceID < itemAll.deviceID ) {
                    vanished.add ( item );

                    list.remove(listIndex);
                    listCount--;
                    continue;
                }

                list.add(listIndex, itemAll);
                listIndex++;
                listCount++;
                listAllIndex++;
            }

            while (listAllIndex < listAllCount)
            {
                DeviceInstance itemAll = env.listAll.get(listAllIndex);
                listAllIndex++;

                if (getMediator)
                {
                    if (itemAll.sourceType == DeviceSourceType.Broadcast)
                        continue;
                }
                else
                {
                    if (itemAll.sourceType == DeviceSourceType.Mediator)
                        continue;
                }

                list.add(listIndex, itemAll);
                listIndex++;
            }
        }
    }


    static void RemoveDevice(Environs env, ObserverNotifyContext ctx)
    {
        //Utils.Log ( 1, className, "RemoveDevice: " + ctx.destID + " Notif: " + Environs.resolveName(ctx.notification) + (deviceInfo == null ? " no" : " has") + " context");
        /// Remove device from all lists

        @SuppressWarnings ( "all" )
        ArrayList<DeviceInstance> vanished = new ArrayList<DeviceInstance>();

        if (env.listNearby != null)
        {
            DeviceInstance device = RemoveDeviceDo ( env, env.listNearby, ctx );
            if (device != null)
            {
                vanished.add ( device );
                env.AddToListContainer ( DeviceClass.Nearby, vanished, null );

                NotifyObservers ( env, null, DeviceClass.Nearby, true );
            }
            PrintDevices(env.listNearby);
        }

        if (env.listMediator != null)
        {
            DeviceInstance device = RemoveDeviceDo ( env, env.listMediator, ctx );
            if (device != null)
            {
                vanished.clear ( );
                vanished.add ( device );

                env.AddToListContainer ( DeviceClass.Mediator, vanished, null );

                NotifyObservers ( env, null, DeviceClass.Mediator, true );
            }
            PrintDevices ( env.listMediator );
        }

        if (env.listAll != null)
        {
            DeviceInstance device = RemoveDeviceDo ( env, env.listAll, ctx );
            if (device != null)
            {
                vanished.clear ( );
                vanished.add ( device );

                env.AddToListContainer ( DeviceClass.All, vanished, null );

                NotifyObservers(env, null, DeviceClass.All, true );
            }
            PrintDevices(env.listAll);
        }
    }

    @SuppressWarnings ( "all" )
    static DeviceInstance RemoveDeviceDo ( @NonNull Environs env, @NonNull final ArrayList<DeviceInstance> list, @NonNull ObserverNotifyContext ctx )
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "RemoveDeviceDo: " + ctx.destID + " Notif: " + Environs.resolveName ( ctx.notification ) );

        if ( list == null )
            return null;

        DeviceInstance device;
        DeviceInstance deviceUpd = null;
        DeviceInstance deviceInfo = null;

        boolean isUIAdapter = (env.isUIAdapter && !IsUIThread () );

        synchronized ( list )
        {
            device = GetDevice ( list, ctx.destID );
            if (device == null) {
                //Utils.Log ("RemoveDeviceDo: not found: " + ctx.destID);
                return null;
            }

            //if (ctx.notification == Environs.NOTIFY_MEDIATOR_DEVICE_REMOVED || ctx.notification == Environs.NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED)
            if ( ctx.contextPtr == null )
            {
                if ( isUIAdapter ) {
                    final DeviceInstance dev = device;

                    env.GetClient ().runOnUiThread ( new Runnable ( ) {
                        @Override
                        public void run ( ) {
                            list.remove ( dev );
                        }
                    } );
                }
                else {
                    list.remove ( device );
                }
            }
            else
            {
                deviceUpd = device;

                device = null;

                deviceInfo = Environs.BuildDeviceInfo ( env.hEnvirons, ctx.contextPtr );
            }
        }

        if ( device != null ) {
            device.Dispose();
        }

        if ( deviceUpd != null && deviceInfo != null ) {
            if (deviceUpd.Update(deviceInfo))
                NotifyDatasetChanged(env, true);
        }
        return device;
    }


    @SuppressWarnings ( "all" )
    static void UpdateDevice ( @NonNull Environs env, @NonNull ObserverNotifyContext ctx, DeviceInstance deviceInfo )
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "UpdateDevice: " + ctx.destID + " Area: " + ctx.areaName + " App: " + ctx.appName + " Notif: " + Environs.resolveName(ctx.notification)
                + (deviceInfo == null ? " no" : " has") + " context");

        DeviceInstance device = null;
        ArrayList<DeviceInstance> list = env.listAll;

        if (list != null)
        {
            //device = GetDevice(list, (int) ctx.destID, ctx.areaName, ctx.appName);
            device = GetDevice ( list, ctx.destID );

            if (device != null)
            {
                if (deviceInfo != null)
                    if (device.Update(deviceInfo))
                        NotifyDatasetChanged(env, true);
            }
            else {
                //Utils.Log ( "UpdateDevice: Device not found " + ctx.destID );
                if (deviceInfo != null)
                {
                    device = deviceInfo;

                    DeviceList.InsertDevice(env, list, deviceInfo, env.contextAll);
                }
            }

            PrintDevices(list);
        }

        list = env.listNearby;
        if (list != null && ((ctx.notification & Environs.NOTIFY_MEDIATOR_SERVER) != Environs.NOTIFY_MEDIATOR_SERVER))
        {
            boolean insert = false;

            if (device == null)
            {
                //device = GetDevice(list, (int) ctx.destID, ctx.areaName, ctx.appName);
                device = GetDevice(list, ctx.destID );
                if (device != null)
                {
                    if (deviceInfo != null)
                        if (device.Update(deviceInfo))
                            NotifyDatasetChanged(env, true);
                }
                else
                {
                    //Utils.Log ( "UpdateDevice: Device not found " + ctx.destID );
                    if (deviceInfo != null)
                    {
                        device = deviceInfo;
                        insert = true;
                    }
                }
            }
            if (insert && device != null)
                DeviceList.InsertDevice(env, list, deviceInfo, env.contextNearby);

            PrintDevices(list);
        }

        list = env.listMediator;
        if (list != null && ((ctx.notification & Environs.NOTIFY_MEDIATOR_SERVER) == Environs.NOTIFY_MEDIATOR_SERVER))
        {
            boolean insert = false;

            if (device == null)
            {
                //device = GetDevice(list, (int) ctx.destID, ctx.areaName, ctx.appName);
                device = GetDevice(list, ctx.destID );
                if (device != null)
                {
                    if (deviceInfo != null)
                        if (device.Update(deviceInfo))
                            NotifyDatasetChanged(env, true);
                }
                else
                {
                    if (deviceInfo != null)
                    {
                        device = deviceInfo;
                        insert = true;
                    }
                }
            }
            if (insert && device == null)
                DeviceList.InsertDevice(env, list, deviceInfo, env.contextMediator);

            PrintDevices(list);
        }
    }


    static class DeviceInstanceUpdateContext
    {
        public DeviceInstance device;

        public DeviceInstance deviceInfo;
    }


    static boolean DeviceInstanceUpdatesApply ( ArrayList<DeviceInstanceUpdateContext> updates )
    {
        boolean changed = false;

        int size = updates.size ();

        for ( int i = 0; i < size; ++i ) {
            DeviceInstanceUpdateContext upd = updates.get ( i );
            if ( upd == null )
                continue;

            if ( upd.device.Update ( upd.deviceInfo ) )
                changed = true;
        }

        return changed;
    }


    static boolean doNotifyInsertDevice = true;

    @SuppressWarnings ( "all" )
    static void InsertDevice ( @NonNull Environs env, @NonNull final ArrayList<DeviceInstance> deviceList, @NonNull final DeviceInstance deviceNew, Environs.ListContext ctx)
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "InsertDevice: " + deviceNew.deviceID + " Project: " + deviceNew.areaName + " App: " + deviceNew.appName
                + (deviceNew == null ? " no" : " has") + " context, " + deviceNew.objID);

        int         listIndex = 0;
        boolean     itemHandled = false;
        boolean     doNotify = doNotifyInsertDevice = true;

        final ArrayList<DeviceInstanceUpdateContext> updates = new ArrayList<DeviceInstanceUpdateContext> ();

        boolean isUIAdapter = (env.isUIAdapter && !IsUIThread ());

        if (isUIAdapter) {
            Utils.RunSync ( env.GetClient (), new Runnable ( ) {
                @Override
                public void run ( ) {
                    InsertDeviceDo ( deviceList, deviceNew, updates );
                    synchronized ( this ) {
                        this.notify ();
                    }
                }
            } );
        }
        else {
            synchronized (deviceList) {
                InsertDeviceDo ( deviceList, deviceNew, updates );
            }
        }

        if (updates.size () > 0) {
            if ( DeviceInstanceUpdatesApply ( updates ) )
                doNotify = false;
        }

        if ( doNotify ) {
            //if (newDevices.size() > 0)
            //    DeviceInstance.ParseAllFiles(newDevices);

            if ( (deviceNew.flags & DeviceFlagsInternal.PlatformReady ) == 0 ) {
                deviceNew.flags |= DeviceFlagsInternal.PlatformReady;

                Environs.SetDeviceFlagsN ( env.hEnvirons, Call.Wait, deviceNew.objID, DeviceFlagsInternal.PlatformReady, true );
            }

            @SuppressWarnings ( "all" )
            ArrayList<DeviceInstance> appeared = new ArrayList<DeviceInstance>();
            appeared.add(deviceNew);

            env.AddToListContainer ( ctx.type, null, appeared );

            NotifyObservers ( env, null, ctx.type, true );
        }
    }


    static void InsertDeviceDo(ArrayList<DeviceInstance> deviceList, DeviceInstance deviceNew, ArrayList<DeviceInstanceUpdateContext> updates)
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "InsertDeviceDo: " + deviceNew.deviceID + " Project: " + deviceNew.areaName + " App: " + deviceNew.appName
                + (deviceNew == null ? " no" : " has") + " context, " + deviceNew.objID);

        int         listIndex   = 0;
        boolean     itemHandled = false;


        int listCount = deviceList.size();

        if (listCount <= 0)
        {
            if (Utils.isDebug) Utils.Log ( 2, className, "InsertDevice: Device appeared 0 [0x" + deviceNew.deviceID + "], " + deviceNew.objID );

            /// Add the new item to the list if we reached the end of the devicelist
            ///
            deviceList.add ( deviceNew );
            return;
        }

        while (listIndex < listCount)
        {
            DeviceInstance device = deviceList.get(listIndex);

            if (deviceNew.deviceID < device.deviceID)
            {
                if (Utils.isDebug) Utils.Log ( 2, className, "InsertDevice: Device appeared 1 [0x" + deviceNew.deviceID + "], " + deviceNew.objID );

                /// Insert new item to the devicelist
                ///
                deviceList.add ( listIndex, deviceNew);
                itemHandled = true;
                break;
            }

            if (deviceNew.isSameAppArea) //if (device.EqualsAppEnv(deviceNew))
            {
                if (deviceNew.deviceID == device.deviceID) /// Device found
                {
                    if (Utils.isDebug) Utils.Log ( 2, className, "InsertDevice: Updating device 0 [0x" + deviceNew.deviceID + "], " + deviceNew.objID );

                    DeviceInstanceUpdateContext upd = new DeviceInstanceUpdateContext ();
                    upd.device = device;
                    upd.deviceInfo = deviceNew;
                    updates.add ( upd );

                    itemHandled = true;
                    break;
                }
                listIndex++;
            }
            else
            {
                if (device.LowerThanAppEnv(deviceNew) || deviceNew.deviceID > device.deviceID)
                {
                    listIndex++;
                    continue;
                }

                if (Utils.isDebug) Utils.Log ( 2, className, "InsertDevice: Device appeared 3 [0x" + deviceNew.deviceID + "], " + deviceNew.objID );

                /// Add the new item to the list if we reached the end of the devicelist
                ///
                deviceList.add(listIndex, deviceNew);
                itemHandled = true;
                break;
            }
        }

        if (!itemHandled)
        {
            if (Utils.isDebug) Utils.Log ( 2, className, "InsertDevice: Device appeared 10 [0x" + deviceNew.deviceID + "], " + deviceNew.objID);
            deviceList.add(deviceNew);
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "InsertDevice: Devicelist update thread done.");
    }


    static void PrintDevices(ArrayList<DeviceInstance> list)
    {
        /*
        synchronized (list)
        {
            for (int i = 0; i < list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                Utils.Log(1, className, "PrintDevices: Device [0x" + device.deviceID + "/" + device.broadcastFound + "]" +
                        " name [" + device.deviceName + "]  proj [" + device.areaName + "] app [" + device.appName + "]");
            }
        }
        */
    }


    static void UpdateConnectProgress (Environs env, int objID, int progress) {
        if (env.listAll != null) {
            UpdateConnectProgress ( env.listAll, objID, progress );
            return;
        }
        if (env.listNearby != null)
            UpdateConnectProgress(env.listNearby, objID, progress);

        if (env.listMediator != null)
            UpdateConnectProgress ( env.listMediator, objID, progress );
    }


    @SuppressWarnings ( "all" )
    static void UpdateConnectProgress ( @NonNull ArrayList<DeviceInstance> list, int objID, int progress )
    {
        synchronized (list)
        {
            for (int i=0; i<list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                if (device != null && (device.objID == objID || device.objIDPrevious == objID) )
                {
                    device.connectProgress = progress;
                    device.NotifyObservers(Environs.DEVICE_INFO_ATTR_CONNECT_PROGRESS, true);
                    return;
                }
            }
        }
    }


    static void UpdateMessageExt ( @NonNull Environs env, @NonNull ObserverMessageContext ctx )
    {
        if (env.listAll != null)
        {
            UpdateMessageExt ( env.listAll, ctx );
            return;
        }

        if (env.listNearby != null)
            UpdateMessageExt ( env.listNearby, ctx );

        if (env.listMediator != null)
            UpdateMessageExt ( env.listMediator, ctx );
    }

    @SuppressWarnings ( "all" )
    static void UpdateMessageExt ( @NonNull ArrayList<DeviceInstance> list, @NonNull ObserverMessageContext ctx )
    {
        synchronized (list)
        {
            for (int i=0; i<list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                if (device != null && device.EqualsID((int) ctx.destID, ctx.areaName, ctx.appName))
                {
                    device.AddMessage ( ctx.message, ctx.length, false, 'u' );
                    return;
                }
            }
        }
    }



    static void UpdateMessage ( @NonNull Environs env, @NonNull ObserverMessageContext ctx )
    {
        if (env.listAll != null)
        {
            UpdateMessage ( env.listAll, ctx );
            return;
        }

        if (env.listNearby != null)
            UpdateMessage ( env.listNearby, ctx );

        if (env.listMediator != null)
            UpdateMessage ( env.listMediator, ctx );
    }

    @SuppressWarnings ( "all" )
    static void UpdateMessage ( @NonNull ArrayList<DeviceInstance> list, @NonNull ObserverMessageContext ctx )
    {
        synchronized (list)
        {
            for (int i=0; i<list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                if (device != null && (device.objID == ctx.destID || device.objIDPrevious == ctx.destID))
                {
                    device.AddMessage(ctx.message, ctx.length, false, 'u');
                    return;
                }
            }
        }
    }


    static void UpdateData ( @NonNull Environs env, @NonNull ObserverDataContext ctx )
    {
        if (env.listAll != null)
        {
            UpdateData ( env.listAll, ctx );
            return;
        }

        if (env.listNearby != null)
            UpdateData ( env.listNearby, ctx );

        if (env.listMediator != null)
            UpdateData ( env.listMediator, ctx );
    }


    @SuppressWarnings ( "all" )
    static void UpdateData ( @NonNull ArrayList<DeviceInstance> list, @NonNull ObserverDataContext ctx )
    {
        synchronized (list)
        {
            for (int i=0; i<list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                if (device != null && (device.objID == ctx.objID || device.objIDPrevious == ctx.objID))
                {
                    device.AddFile ( ctx.type, ctx.fileID, ctx.descriptor, "", ctx.size, false );
                    return;
                }
            }
        }
    }


    static void UpdateSensorData ( @NonNull Environs env, @NonNull UdpDataContext ctx )
    {
        if (env.listAll != null)
        {
            UpdateSensorData ( env.listAll, ctx );
            return;
        }

        if (env.listNearby != null)
            UpdateSensorData ( env.listNearby,  ctx );

        if (env.listMediator != null)
            UpdateSensorData ( env.listMediator, ctx );
    }


    @SuppressWarnings ( "all" )
    static void UpdateSensorData ( @NonNull ArrayList<DeviceInstance> list, @NonNull UdpDataContext ctx )
    {
        int objID = ctx.objID;

        synchronized (list)
        {
            for (int i=0; i<list.size(); i++)
            {
                DeviceInstance device = list.get(i);

                if (device != null && (device.objID == objID || device.objIDPrevious == objID))
                {
                    if (ctx.sensorFrame == null)
                    {
                        ctx.sensorFrame.device = device;

                        device.NotifySensorObservers ( ctx.sensorFrame );
                    }
                    else
                        device.NotifyUdpData ( ctx.data );
                    return;
                }
            }
        }
    }
}
