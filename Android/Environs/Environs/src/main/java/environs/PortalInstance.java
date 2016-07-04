package environs;
/**
 *	PortalInstance
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
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Rect;
import android.util.SparseArray;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

/**
 *	PortalInstance management and event provider
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public class PortalInstance implements SurfaceHolder.Callback
{
    private static final String className = "PortalInstance . . . . .";

    int                         hEnvirons = 1;

    /** An ID that identifies this portal across all available portals. */
    public int                  portalID = -1;

    int                         portalKey = -1;

    /** true = Object is disposed and not updated anymore. */
    public boolean              disposed = false;

    /** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
    public @Call.Value   int    async = Call.NoWait;

    public boolean              disposeOngoing = false;
    boolean                     establishCalled = false;

    long                        createdTickCount = System.currentTimeMillis();

    /** A DeviceInstance object that this portal relates to. */
    public DeviceInstance       device = null;

    public @PortalStatus.Value int status = PortalStatus.Created;

    /** true = outgoing (Generator), false = incoming (Receiver). */
    public boolean              outgoing = false;

    @SuppressWarnings ( "unused" )
    public boolean isIncoming() { return !outgoing; }

    @SuppressWarnings ( "unused" )
    public boolean isOutgoing() { return outgoing; }

    public int                  portalType = PortalType.Any;

    /** A PortalInfo object that holds the lastLocation and size of the portal. */
    public PortalInfo           info    = new PortalInfo();

    boolean                     networkOK = false;

    boolean                     askForTypeValue = true;

    public boolean              startIfPossible = false;
    boolean                     renderActivated = false;

    @SuppressWarnings ( "all" )
    final ArrayList<PortalObserver>   observers = new ArrayList<PortalObserver>();

    @SuppressWarnings ( "all" )
    /** A global collection of PortalInstances that all devices have established or are managing. */
    final static SparseArray<PortalInstance> portals = new SparseArray<PortalInstance>();

    static int GetKey ( int nativeID, int portalID ) {
        return ((nativeID << 16) | (portalID & 0xFFF));
    }

    boolean InitInstance ( int hInst, DeviceInstance deviceInstance, int Environs_PORTAL_DIR_, int portalType, int slot )
    {
        if (deviceInstance == null)
            return false;

        device = deviceInstance;
        info.portal = this;
        hEnvirons = hInst;

        outgoing = ((Environs_PORTAL_DIR_ & Environs.PORTAL_DIR_OUTGOING) == Environs.PORTAL_DIR_OUTGOING);
        this.portalType = portalType;

        synchronized (device.devicePortals) {
            if (slot < 0) {
                slot = Environs.GetPortalIDFreeSlotN ( hEnvirons, device.nativeID, Environs_PORTAL_DIR_ );
                if (slot < 0) {
                    Utils.LogE(className, "InitInstance: No free portal slot available.");
                    return false;
                }
            }
            portalKey = GetKey(deviceInstance.nativeID, slot | Environs_PORTAL_DIR_);

            portalID = Environs_PORTAL_DIR_ | slot | portalType;

            if (Utils.isDebug) Utils.Log ( 6, className, "InitInstance: Adding [" + Integer.toHexString(portalID).toUpperCase() + " / K:" + portalKey + "] to device portals.");
            device.devicePortals.add ( this );
        }
        return true;
    }


    boolean Init ( int hInst, DeviceInstance deviceInstance, int Environs_PORTAL_DIR_, int portalType, int slot )
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Init");

        if (!InitInstance(hInst, deviceInstance, Environs_PORTAL_DIR_, portalType, slot))
            return false;

        synchronized (portals) {
            if (Utils.isDebug) Utils.Log ( 6, className, "Init: Adding " + portalKey + " to global portals.");
            portals.put(portalKey, this);
        }

        return true;
    }


    boolean Init( int hInst, DeviceInstance deviceInstance, int portalID)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Init");

        if (!InitInstance(hInst, deviceInstance, portalID & Environs.PORTAL_DIR_MASK, portalID & Environs.PORTAL_TYPE_MASK, portalID & 0xFF))
            return false;

        synchronized (portals) {
            if (Utils.isDebug) Utils.Log ( 6, className, "Init: Adding portal [" + Integer.toHexString(this.portalID).toUpperCase() + " / K:" +  portalKey + "] to global portals." );

            this.portalID = portalID;

            portals.put(portalKey, this);
        }
        return true;
    }


    public void Dispose()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Dispose");

        synchronized (portals)
        {
            if (disposed) {
                if (Utils.isDebug) Utils.Log ( 6, className, "Dispose: [" + Integer.toHexString(portalID).toUpperCase() + "] already disposed.");
                return;
            }
            disposed = true;

            if (portals.get(portalKey) != null) {
                if (Utils.isDebug) Utils.Log ( 6, className, "Dispose: Removing portal " + Integer.toHexString(portalKey).toUpperCase() + " from global portals.");
                portals.remove(portalKey);
            }
        }

        if (Utils.isDebug) Utils.Log ( 6, className, "Dispose: Removing [" + Integer.toHexString(portalID).toUpperCase() + "] from device portals.");
        synchronized (device.devicePortals) {
            device.devicePortals.remove(this);
        }

        if (status >= 2)
            NotifyObservers(Environs.NOTIFY_PORTAL_STREAM_STOPPED);
        status = PortalStatus.Disposed;

        NotifyObservers(Environs.ENVIRONS_OBJECT_DISPOSED);
        observers.clear ( );
    }


    static void Dispose(int nativeID, int portalID)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "DisposeS: [" + Integer.toHexString(portalID).toUpperCase() + "]");

        int portalKey = GetKey(nativeID, portalID);

        PortalInstance portal;

        synchronized (portals) {
            portal = portals.get(portalKey);
        }

        if (portal != null)
            portal.Dispose();

        if (Utils.isDebug) Utils.Log ( 6, className, "DisposeS: Done [" + Integer.toHexString ( portalID ).toUpperCase ( ) + "]" );
    }


    public void AddObserver(PortalObserver observer)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "AddObserver");

        if (observer == null)
            return;

        synchronized ( observers )
        {
            if (observers.contains(observer))
                return;

            observers.add ( observer );
        }
    }

    @SuppressWarnings("unused")
    public void RemoveObserver(PortalObserver observer) {
        if (Utils.isDebug) Utils.Log ( 6, className, "RemoveObserver");

        if (observer == null)
            return;

        synchronized ( observers )
        {
            if (observers.contains(observer))
                return;

            observers.remove(observer);
        }
    }


    void NotifyObservers(int notification)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "NotifyObservers");

        int size;
        synchronized ( observers ) { size = observers.size(); }

        if (size > 0) {
            for (int i=0; i<size; i++)
            {
                PortalObserver observer;
                synchronized ( observers ) {
                    observer = observers.get(i);
                }

                if ( observer != null ) {
                    try {
                        observer.OnPortalChanged(this, notification);
                    } catch (Exception e) {
                        e.printStackTrace ( );
                        synchronized ( observers ) { observers.remove(i); }
                        i--;
                    }
                }
                synchronized ( observers ) { size = observers.size(); }
            }
        }
    }


    @SuppressWarnings ( "SimplifiableIfStatement" )
    public boolean Start()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Start");

        if ( disposed || device.disposed || !device.isConnected || portalID <= 0 )
            return false;

        return Environs.StartPortalStreamN ( hEnvirons, async, portalID );
    }


    @SuppressWarnings ( "SimplifiableIfStatement" )
    public boolean Stop()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Stop");

        if ( disposed )
            return false;

        Dispose();

        if ( !device.isConnected )
            return false;

        return Environs.StopPortalStreamN ( hEnvirons, async, device.nativeID, portalID );
    }


    Surface                     surface;
    SurfaceView                 surfaceView;
    SurfaceHolder               surfaceHolder;

    @SuppressWarnings ( "unused" )
    public void SetRenderTouchView(View view)
    {
        Environs.SetPortalTouchView(hEnvirons, this, view);
    }


    @SuppressWarnings ( "unused" )
    public boolean SetRenderSurface(SurfaceView surfaceView, boolean addAsTouchView) {
        return SetRenderSurface(surfaceView, 1.0f, addAsTouchView);
    }


    public boolean SetRenderSurface(SurfaceView surfaceView, float resolutionDivider, boolean addAsTouchView)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "SetRenderSurface");

        if (surfaceView == null)
            return false;

        this.surfaceView = surfaceView;
        surfaceHolder = surfaceView.getHolder();
        if ( surfaceHolder == null )
            return false;

        if (addAsTouchView)
            Environs.SetPortalTouchView(hEnvirons, this, surfaceView);

        return SetRenderSurface(surfaceHolder, resolutionDivider);
    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (Utils.isDebug) Utils.Log ( 4, className, "surfaceChanged");
        surfaceHolder = holder;
        SetRenderSurface ( holder );

        NotifyObservers(Environs.PORTAL_INSTANCE_FLAG_SURFACE_CHANGED);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (Utils.isDebug) Utils.Log ( 4, className, "surfaceCreated");
        surfaceHolder = holder;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (Utils.isDebug) Utils.Log ( 4, className, "surfaceDestroyed");
        surfaceHolder = null;
    }

    public boolean SetRenderSurface(SurfaceHolder surfaceHolder) {
        return SetRenderSurface(surfaceHolder, 1.0f);
    }

    public boolean SetRenderSurface(SurfaceHolder surfaceHolder, float resolutionDivider)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "SetRenderSurface");

        if (surfaceHolder == null)
            return false;

        surfaceHolder.addCallback(this);

        int width = info.width;
        int height = info.height;

        if (width == 0 && height == 0) {
            Rect rect = surfaceHolder.getSurfaceFrame();

            width = rect.width(); height = rect.height();
        }

        surface = surfaceHolder.getSurface();
        if (surface == null)
            return false;

        renderActivated = SetRenderSurface(surface, (int)(width / resolutionDivider), (int)(height / resolutionDivider));
        return renderActivated;
    }


    public boolean SetRenderSurface(Surface surface, int width, int height)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "SetRenderSurface");

        if (width != 0 && height != 0 && info.width == 0 && info.height == 0) {
            info.width = width; info.height = height;
        }

        this.surface = surface;
        return Environs.SetRenderSurface ( hEnvirons, portalID, surface, width, height );
    }


    @SuppressWarnings ( "unused" )
    public boolean ReleaseRenderSurface()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "ReleaseRenderSurface");

        return Environs.ReleaseRenderSurface(hEnvirons, async, portalID);
    }


    boolean HasPortalRenderer()
    {
        if ( surface != null ) {
            if ( renderActivated )
                return true;
            renderActivated = Environs.SetRenderSurface(hEnvirons, portalID, surface, info.width, info.height);
        }
        return renderActivated;
    }


    public static PortalInstance GetPortal(int nativeID, int portalID) {

        int key = GetKey(nativeID, portalID);

        synchronized (portals) {
            return portals.get(key);
        }
    }


    @SuppressWarnings ( "unused" )
    public PortalInfo GetPortalInfo() {
        return info;
    }


    final static int PortalInfoSize = 36;

    /**
     * Get details about portal associated with the portalID.
     *
     * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
     * 						It is provided within the notification listener as sourceIdent.&nbsp;
     * 						Applications should store them in order to address the correct portal within Environs.
     * @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
     */
    public PortalInfo GetPortalInfo(int portalID)
    {
        ByteBuffer buffer;

        try {
            buffer = ByteBuffer.allocateDirect ( PortalInfoSize );
        }
        catch ( Exception ex ) {
            Utils.LogE ( className, "GetPortalInfo: FAILED to allocate memory of size " + PortalInfoSize);
            ex.printStackTrace ( );
            return null;
        }

        ByteOrder nativeOrder = ByteOrder.nativeOrder();
        if ( buffer.order() != nativeOrder )
            buffer.order ( nativeOrder );

        buffer.position(0);
        buffer.putInt(portalID);

        if ( !Environs.GetPortalInfoN ( hEnvirons, buffer) )
            return null;

        buffer.position(0);
        PortalInfo info = new PortalInfo();
        info.portalID = buffer.getInt();
        info.flags = buffer.getInt();

        info.centerX = buffer.getInt();
        info.centerY = buffer.getInt();
        info.width = buffer.getInt();
        info.height = buffer.getInt();
        info.orientation = buffer.getFloat();

        //Utils.Log ( info.toString() );
        return info;
    }


    static PortalInfo BuildPortalInfo(byte [] bytes)
    {
        //if (Utils.isDebug) Utils.Log ( 6, className, "BuildPortalInfo: length [" + bytes.length + "] ");

        ByteBuffer buffer;

        try {
            buffer = ByteBuffer.wrap ( bytes );
        }
        catch ( Exception ex ) {
            Utils.LogE ( className, "BuildPortalInfo: FAILED to wrap bytes of size " + bytes.length );
            ex.printStackTrace ();
            return null;
        }

        ByteOrder nativeOrder = ByteOrder.nativeOrder();
        if ( buffer.order() != nativeOrder )
            buffer.order ( nativeOrder );

        buffer.position(4);
        PortalInfo info = new PortalInfo();
        // Do not take over the portalID of source as its cache-index is different almost for sure
        //info.portalID = buffer.getInt();
        info.flags = buffer.getInt();

        info.centerX = buffer.getInt();
        info.centerY = buffer.getInt();
        info.width = buffer.getInt();
        info.height = buffer.getInt();
        info.orientation = buffer.getFloat();

        //Utils.Log ( info.toString() );
        return info;
    }

    /**
     * Set details for the portal associated with the portalID.
     *
     * @param   info        A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
     * 			            The portalID member of the PortalInfo object must have valid values.
     * 						It is provided within the notification listener as sourceIdent.&nbsp;
     * 						Applications should store them in order to address the correct portal within Environs.
     * @return success
     */
    public boolean SetPortalInfo(PortalInfo info)
    {
        ByteBuffer buffer;

        try {
            buffer = ByteBuffer.allocateDirect ( PortalInfoSize );
        }
        catch ( Exception ex ) {
            Utils.LogE ( className, "SetPortalInfo: FAILED to allocate memory of size " + PortalInfoSize);
            ex.printStackTrace ();
            return false;
        }

        ByteOrder nativeOrder = ByteOrder.nativeOrder();
        if ( buffer.order() != nativeOrder )
            buffer.order ( nativeOrder );

        buffer.position(0);
        buffer.putInt(info.portalID);

        if (info.flags == 0)
            info.flags = 0xF;
        buffer.putInt(info.flags);

        buffer.putInt(info.centerX);
        buffer.putInt(info.centerY);
        buffer.putInt ( info.width );
        buffer.putInt(info.height);
        buffer.putFloat(info.orientation);

        return Environs.SetPortalInfoN ( hEnvirons, buffer );
    }


    @SuppressWarnings ( "unused" )
    void Update()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Update");

        portalType = (portalID & Environs.PORTAL_TYPE_MASK);

        int Environs_PORTAL_DIR_ = portalID & Environs.PORTAL_DIR_MASK;

        outgoing = ((Environs_PORTAL_DIR_ & Environs.PORTAL_DIR_OUTGOING) == Environs.PORTAL_DIR_OUTGOING);
    }


    public boolean Establish(boolean askForType)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "Establish");

        if ( device == null || device.disposed || !device.isConnected )
            return false;

        establishCalled = true;
        askForTypeValue = askForType;

        if ( !networkOK && !CheckNetworkConnection() )
            return true;

        if (outgoing) {
            if ( askForType ) {
                ShowDialogOutgoingPortal();
            }
            else {
                if (status == PortalStatus.CreatedFromRequest) {
                    return Environs.ProvidePortalStreamN ( hEnvirons, device.nativeID, async, portalID );
                }
                else if (status == PortalStatus.CreatedAskRequest ) {
                    return Environs.ProvideRequestPortalStreamN ( hEnvirons, device.nativeID, async, portalID );
                }
            }
        }
        else {
            return Environs.RequestPortalStreamN ( hEnvirons, device.nativeID, async, portalID, info.width, info.height );
        }
        return false;
    }


    /**
     * Internal portal management logic.
     *
     */
    static void Update ( Environs env, int nativeID, int notification, int portalID )
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "PortalUpdate: [" + Integer.toHexString(portalID).toUpperCase() + "] "  + Environs.resolveName(notification));

        PortalInfo info;

        int portalKey = GetKey(nativeID, portalID);

        PortalInstance portal;

        synchronized (portals) {
            portal = portals.get(portalKey);
        }

        switch (notification)
        {
            case Environs.NOTIFY_PORTAL_REQUEST_FAIL:
            case Environs.NOTIFY_PORTAL_PROVIDE_FAIL:
                KillZombies();

            case Environs.NOTIFY_PORTAL_STREAM_STOPPED:
                Dispose(nativeID, portalID);
                return;

            case Environs.NOTIFY_PORTAL_ASK_REQUEST:
            case Environs.NOTIFY_PORTAL_REQUEST: // Make sure that the requested slot is free and if so, then create a portal instance
            case Environs.NOTIFY_PORTAL_INCOMING_ESTABLISHED: // An incoming portal has been established. Create a portal instance and let app decide
            {
                if (env.observers.size() <= 0) return;

                DeviceInstance device = null;
                if (portal != null)
                    device = portal.device;
                else {

                    for (DeviceList list : env.deviceLists) {
                        device = list.GetDeviceByNativeID ( nativeID );
                        if (device != null)
                            break;
                    }
                }
                if (device == null)
                    return;
                if (notification == Environs.NOTIFY_PORTAL_REQUEST)
                {
                    if (portal == null) {
                        portal = device.PortalCreate(portalID);
                        if (portal == null)
                            return;
                        portal.status = PortalStatus.CreatedFromRequest;
                    }
                    else {
                        portal.status = PortalStatus.CreatedFromRequest;
                        Environs.ProvidePortalStreamN ( env.hEnvirons, device.nativeID, Types.CALL_NOWAIT, portalID);
                        return;
                    }
                }
                else if (notification == Notify.Portal.IncomingEstablished ) {
                    if (portal == null)
                        return;
                    portal.status = PortalStatus.Established;
                }
                else //if (notification == Environs.NOTIFY_PORTAL_ASK_REQUEST)
                {
                    portal = device.PortalCreateID(portalID);
                    if (portal == null)
                        return;
                    portal.status = PortalStatus.CreatedAskRequest;
                }

                if (portal.portalID != portalID)
                    portal.portalID = portalID;

                if (portal.status < Environs.PORTAL_STATUS_ESTABLISHED) {
                    PresentPortalToObservers(env.hEnvirons, portal, notification);
                    return;
                }
                HandleSuccessfulPortal(portal, notification);
                return;
            }

            case Environs.NOTIFY_PORTAL_PROVIDER_READY:
            case Environs.NOTIFY_PORTAL_PROVIDE_STREAM_ACK:
            case Environs.NOTIFY_PORTAL_PROVIDE_IMAGES_ACK:
            {
                if (portal != null)
                {
                    if (portal.portalID != portalID)
                        portal.portalID = portalID;

                    //if (portal.status < PortalStatus.Established)
                        portal.status = PortalStatus.Established;

                    info = portal.GetPortalInfo(portalID);
                    if (info != null)
                        portal.info.Update(0, info);
                    portal.NotifyObservers(notification);
                }
                return;
            }
            default: break;
        }

        if (portal == null)
            return;

        if (notification == Environs.NOTIFY_PORTAL_STREAM_INCOMING || notification == Environs.NOTIFY_PORTAL_IMAGES_INCOMING)
            portal.status = PortalStatus.Started;
        else if (notification == Environs.NOTIFY_PORTAL_STREAM_PAUSED)
            portal.status = PortalStatus.Established;

        portal.NotifyObservers(notification);
    }


    static void UpdateOptions(ObserverNotifyContext ctx, int notification, int portalID)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "UpdateOptions: [" + Integer.toHexString(portalID).toUpperCase() + "] " + Environs.resolveName(notification));

        PortalInstance portal;

        switch(notification)
        {
            case Environs.NOTIFY_PORTAL_LOCATION_CHANGED:
            case Environs.NOTIFY_PORTAL_SIZE_CHANGED:
            case Environs.NOTIFY_DEVICE_ON_SURFACE:
            case Environs.NOTIFY_DEVICE_NOT_ON_SURFACE:

                DeviceInstance dev = DeviceList.GetDeviceByNativeID ( ctx.env, ctx.destID );
                if ( dev == null )
                    return;

                int nativeID = dev.nativeID;

                synchronized (portals)
                {
                    int key = GetKey ( nativeID, portalID);

                    portal = portals.get(key);
                }
                if (portal == null) {
                    Utils.LogW ( className, "UpdateOptions: Portal [" + Integer.toHexString(portalID).toUpperCase() + "] not found." );
                    return;
                }

                PortalInfo info;

                if (ctx.contextPtr != null )
                    info = BuildPortalInfo ( ctx.contextPtr );
                else
                    info = portal.GetPortalInfo(portalID);

                if (info == null) {
                    Utils.LogW ( className, "UpdateOptions: Get PortalInfo [" + Integer.toHexString(portalID).toUpperCase() + "] failed." );
                    return;
                }

                if (portal.device.SetDirectContact ( notification == Environs.NOTIFY_DEVICE_ON_SURFACE ? 1 : 0 ) )
                    portal.NotifyObservers(Environs.NOTIFY_CONTACT_DIRECT_CHANGED);

                if (!portal.info.Update(notification, info))
                    return;
                break;

            default:
                return;
        }

        portal.NotifyObservers(notification);
    }


    static void HandleSuccessfulPortal(PortalInstance portal, int notification)
    {
        PortalInfo info = portal.GetPortalInfo(portal.portalID);
        if (info != null)
            portal.info.Update(0, info);

        if (portal.status < Environs.PORTAL_STATUS_ESTABLISHED)
            portal.device.NotifyObservers(Environs.DEVICE_INFO_ATTR_PORTAL_CREATED, true);
        //portal.device.NotifyObservers(portal, Environs.DEVICE_INFO_ATTR_PORTAL_CREATED);
        else
            portal.NotifyObservers(notification);

        if (portal.startIfPossible && notification == Environs.NOTIFY_PORTAL_INCOMING_ESTABLISHED && portal.HasPortalRenderer())
            portal.Start();
    }


    static void PresentPortalToObservers(final int hInst, final PortalInstance portal, final int notification)
    {
        new Thread(new Runnable() {
            @SuppressWarnings ( "SynchronizationOnLocalVariableOrMethodParameter" )
            public void run(){
                boolean success = false;

                if (portal != null) {
                    DeviceInstance device = portal.device;
                    if ( device != null )
                    {
                        ArrayList<DeviceObserver>	obs = device.observers;

                        int size;
                        synchronized ( obs ) { size = obs.size(); }

                        if (size > 0) {
                            for (int i=0; i<size; i++)
                            {
                                DeviceObserver observer;
                                synchronized ( obs ) {
                                    observer = obs.get(i);
                                }

                                try {
                                    observer.OnPortalRequestOrProvided(portal);
                                    if (portal.observers.size() > 0 && portal.establishCalled) {
                                        success = true;
                                        break;
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace ( );
                                    synchronized ( obs ) { obs.remove(i); }
                                    i--;
                                }

                                synchronized ( obs ) { size = obs.size(); }
                            }
                        }

                        if (!success) {
                            portal.Dispose();
                            return;
                        }

                        HandleSuccessfulPortal(portal, notification);
                    }
                }
            }
        }).start();
    }



    private boolean CheckNetworkConnection ()
    {
        if ( Broadcasts.networkStatus >= Environs.NETWORK_CONNECTION_WIFI ) {
            networkOK = true;
            return true;
        }

        Environs env = Environs.GetInstance(hEnvirons);
        if ( env == null )
            return false;

        if ( !env.GetUseTrafficCheck()) {
            networkOK = true;
            return true;
        }

        final Activity act = Environs.currentActivity;
        if ( act == null ) {
            Utils.LogE ( className, "CheckNetworkConnection: We need to ask for permission, but no activity is available.");
            return false;
        }

        act.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder alertDialog = new AlertDialog.Builder(act);

                final PortalInstance portal = PortalInstance.this;

                alertDialog.setTitle("High Traffic Info" + Integer.toHexString(portal.device.deviceID));

                String msg = "You are not connected by WiFi! A portal will create high data traffic. Continue?";
                alertDialog.setMessage(msg);

                alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        portal.networkOK = true;
                        portal.Establish(true);
                    }
                });

                alertDialog.setNegativeButton("No, Cancel!", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });

                alertDialog.show();
            }
        });

        return false;
    }


    /**
     * Request a portal stream from the device with the given id.&nbsp;The device must be connected before with DeviceConnect ().
     *
     */
    private void ShowDialogOutgoingPortal ()
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "ShowDialogOutgoingPortal");

        Activity act;

        synchronized (Environs.className) {
            act = Environs.currentActivity;
        }
        if ( act == null ) {
            Utils.LogE ( className, "ShowDialogOutgoingPortal: No activity available.");
            return;
        }

        act.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final PortalInstance portal = PortalInstance.this;

                boolean isFront = portalType == PortalType.FrontCam;
                boolean isBack = portalType == PortalType.BackCam;

                AlertDialog.Builder alertDialog = new AlertDialog.Builder(Environs.currentActivity);

                alertDialog.setTitle("Incoming portal request by 0x" + Integer.toHexString(device.deviceID));

                String msg;

                if (isFront || isBack)
                    msg = "A request for the " + (isBack ? "back facing" : "front facing")
                            + " camera.\r\n"
                            + "Do you want to send such a camera portal?";
                else
                    msg = "Do you want to share a camera portal?";
                alertDialog.setMessage(msg);

                alertDialog.setPositiveButton("Back camera", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        portal.portalType = PortalType.BackCam;
                        portal.portalID = (portal.portalID & 0xFFFF0FFF) | portal.portalType;
                        portal.Establish(false);
                    }
                });

                alertDialog.setNeutralButton("Front camera", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        portal.portalType = PortalType.FrontCam;
                        portal.portalID = (portal.portalID & 0xFFFF0FFF) | portal.portalType;
                        portal.Establish(false);
                    }
                });

                alertDialog.setNegativeButton("No thanks!", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        portal.Dispose();
                    }
                });

                alertDialog.show();
            }
        });
    }


    static void KillZombies()
    {
        @SuppressWarnings ( "all" )
        final ArrayList<PortalInstance> targets = new ArrayList<PortalInstance>();

        synchronized (portals)
        {
            long tick = System.currentTimeMillis ( );

            for ( int i = 0; i < portals.size (); ++i )
            {
                PortalInstance portal = portals.valueAt ( i );

                if (!portal.disposeOngoing && portal.portalID < 0 && (tick - portal.createdTickCount) > Environs.MAX_PORTAL_REQUEST_WAIT_TIME_MS)
                {
                    portal.disposeOngoing = true;
                    targets.add(portal);
                }
            }
        }
        if (targets.size() <= 0)
            return;

        // Alright, lets call the Destroyer
        new Thread(new Runnable() {
            public void run(){
                Destroyer(targets);
            }
        }).start();
    }

    static void Destroyer(ArrayList<PortalInstance> targets)
    {
        for (int i=0; i<targets.size(); i++) {
            targets.get(i).Dispose();
        }
    }
}
