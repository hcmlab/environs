package environs;
/**
 *	Environs
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

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Vector;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Looper;
import android.util.SparseArray;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import android.annotation.SuppressLint;
import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.google.android.gcm.GCMRegistrar;


/** Environs API class, This class and instance of this class provides access and control over the Environs framework.
 * 	
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 * 
 */
@SuppressWarnings("unused")
public class Environs extends TypesResolver
{
	static final String className = "Environs . . . . . . . .";


	/**
	 * The native object handle returned by CreateEnvirons.
	 */
	int hEnvirons = 0;

	public int GetHandle () { return hEnvirons; }


	/**
	 * All instances of Environs that were created by applications
	 */
	final static Environs[] 	instances = new Environs [ ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE ];


	/** Perform calls to the Environs object asynchronously. If set to Environs.CALL_WAIT, then all commands will block until the call finishes.
	 * If set to Environs.CALL_NOWAIT, then certain calls (which may take longer) will be performed asynchronously. */
	public @Call.Value int		async = Call.NoWait;


	static Broadcasts broadcasts = new Broadcasts();
	
	/**
	 * This is the main instance for handling the actual Environs environment.&nbsp; 
	 * It makes use of static access without modifier so as to be accessed by internal framework objects within the same package.
	 * 
	 *  @return	Activity	The client activity that the appliction registered during initialization of Environs.
	 */
	public Activity GetClient() { return instances[hEnvirons].clientActivity; }

	boolean started = false;

	static boolean useNativeDecoder = false;
	static boolean useSurfaceEncoding = true;

	@SuppressWarnings ( "all" )
	final static HashMap<String, PortalReceiver> portalReceivers = new HashMap<String, PortalReceiver>();

	@SuppressWarnings ( "all" )
	final static HashMap<String, PortalGenerator> portalGenerators = new HashMap<String, PortalGenerator>();

	/*static String buildPortalKey(int deviceID, int portalID, String area, String app) {
		if ( area == null )
			area = areaName;
		if ( app == null )
			app = appName;

		portalKeyCounter++;
		return area + "_" + app + "_" + deviceID + "_" + portalID;
	}
	*/
	static String buildPortalKey(int portalID) {
		return "_" + portalID;
	}

	/**
	 * Defines whether the native layer libraries are available and working.
	 * This flag is set by the static initializer of Environs.
	 */
	static boolean nativeLayerInstalled = false;

	/**
	 * Native memory mangement.&nbsp;
	 * A container that keeps a reference to ByteBuffers within the virtual machine&nbsp;
	 * to be accessed in a shared manner with the native layer.
	 */
	@SuppressWarnings ( "all" )
	private static Vector<ByteBuffer> jniMemory = new Vector<ByteBuffer>();


	/** Default value for each DeviceInstance after object creation. */
	private static boolean allowConnectDefault = false;


	/** A global wifi observer. */
	static WifiObserver wifiObserver = new WifiObserver ();


	/** A global bluetooth observer. */
	static BtObserver btObserver = new BtObserver ();


	// Load library
	static {
		/// Prepare folder structure
		/// /data/data/application-name/files
		try {
			Libs.Init_libEnvirons ( );

			if ( nativeLayerInstalled ) {
				AllocPlatformN ( );
				jniMemory.clear();
			}
		}
		catch (Exception ex) {
			Utils.LogE ( className, "Environs: " + ex.getMessage() );

			ex.printStackTrace();

			nativeLayerInstalled = false;
		}
	}


	/**
	 * Log a message through environs
	 *
	 * @param   msg	The message to log
	 */
	private static native void LogN ( @NonNull String msg, int length );

	/**
	 * Set timeout for LAN/WiFi connects. Default ( 2 seconds ).
	 * Increasing this value may help to handle worse networks which suffer from large latencies.
	 *
	 * @param   timeout	A timemout value in seconds
	 */
	public void SetNetworkConnectTimeout ( int timeout ) {
		SetNetworkConnectTimeoutN ( timeout );
	}

	private static native void SetNetworkConnectTimeoutN ( int timeout );

	/**
	 * Construction of Environs objects have to be done using Environs.CreateInstance() or Environs.New()
	 */
	private Environs ()
	{
	}


	/**
	 * Create a native Environs object and return a handle to the object.
	 * A return value of 0 means Error
	 *
	 * @return   An instance handle that refers to the created Environs object
	 */
	private static native int CreateEnvironsN ();


	/**
	 * Load settings for the given application environment from settings storage,
	 * if any have been saved previously.
	 *
	 * @param	hInst		The handle to a particular native Environs instance.
	 * @param 	appName		The application name for the application environment.
	 * @param  	areaName	The area name for the application environment.
	 *
	 * @return   success
	 */
	private static native boolean LoadSettingsN ( int hInst, String appName, String areaName );


	/**
	 * Retrieve a boolean that determines whether the native layer libraries are available and working.
	 *
	 * @return		true = yes, false = no
	 */
	public static boolean IsInstalled()
	{
		if (!nativeLayerInstalled ) {
			Utils.LogE ( className, "IsInstalled: native layer libraries are not installed correctly!" );
			return false;
		}
		return true;
	}


	@Nullable
	static Environs GetInstance ( @IntRange (from=1, to=ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE) int hInst ) {
		return instances[hInst];
	}


	/**
	 * Create an Environs object.
	 *
	 * @param	client		An application context of the client application.
	 *
	 * @return   An Environs object
	 */
	@Nullable
	public static Environs New ( @NonNull Context client )
	{
		return CreateInstance(client);
	}


	/**
	 * Create an Environs object.
	 *
	 * @param	client		An application context of the client application.
	 * @param 	appName		The application name for the application environment.
	 * @param  	areaName	The area name for the application environment.
	 *
	 * @return   An Environs object
	 */
	@Nullable
	public static Environs New ( @NonNull Context client, String appName, String areaName)
	{
		return CreateInstance(client);
	}


	/**
	 * Create an Environs object.
	 *
	 * @param	client		An application context of the client application.
	 *
	 * @return   An Environs object
	 */
	@Nullable
	public static Environs CreateInstance ( @NonNull Context client )
	{
		return CreateInstance ( client, null, null);
	}


	/**
	 * Create an Environs object.
	 *
	 * @param	client		An application context of the client application.
	 * @param 	appName		The application name for the application environment.
	 * @param  	areaName	The area name for the application environment.
	 *
	 * @return   An Environs object
	 */
	@SuppressWarnings ( {"ConstantConditions", "LoopStatementThatDoesntLoop" } )
	@Nullable
	public static Environs CreateInstance ( @NonNull Context client, String appName, String areaName)
	{
		if ( client == null ) {
			Utils.LogE(className, "CreateInstance: Invalid application context (null).");
			return null;
		}

		if ( !IsInstalled () ) {
			Utils.LogE(className, "CreateInstance: Native layer is not installed correctly.");
			return null;
		}

		int hInst = CreateEnvironsN ();
		if ( hInst == 0 ) {
			Utils.LogE(className, "CreateInstance: Failed to create native Environs object.");
			return null;
		}

		if ( hInst >= ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE ) {
			Utils.LogE(className, "CreateInstance: Invalid handle to native Environs object returned by native layer.");
			return null;
		}

		do {
			Environs env = new Environs();

			env.sensors = new Sensors ( env, hInst );

			if ( !env.Create ( client, hInst ) )
				break;

			synchronized ( instances ) {
				instances [ hInst ] = env;
			}

			if ( appName != null && areaName != null )  {
				env.appName = appName;
				env.areaName = areaName;
				env.LoadSettings ( appName, areaName );
			}

			env.appAreaID = GetAppAreaIDN ( hInst );

			if ( !env.InitInstance () )
				break;

			return env;
		}
		while ( false );

		synchronized ( instances ) {
			instances [ hInst ] = null;
		}

		DisposeN ( hInst );
		return null;
	}


	/**
	 * Load settings for the given application environment from settings storage,
	 * if any have been saved previously.
	 *
	 * @param 	appName		The application name for the application environment.
	 * @param  	areaName	The area name for the application environment.
	 *
	 * @return   success
	 */
	@SuppressWarnings ( "all" )
	public boolean LoadSettings ( @NonNull String appName, @NonNull String areaName ) {

		if ( appName != null && areaName != null )  {
			this.appName = appName;
			this.areaName = areaName;
		}
		return LoadSettingsN ( hEnvirons, appName, areaName );
	}


	/**
	 * Load settings. Prior to this call, an application environment MUST be given
	 * using SetApplicationName and SetAreaName.
	 *
	 * @return   success
	 */
	public boolean LoadSettings ( ) {
		return LoadSettingsN ( hEnvirons, null, null );
	}


	public void finalize() {
		try {
			super.finalize();

			Dispose();

		} catch (Throwable throwable) {
			throwable.printStackTrace();
		}
	}


	/**
	 * Creates the environment.
	 * Tasks:
	 * 	- Verify and take over the application object
	 *  - Load preferences (_tagID)
	 *  - Create the native layer
	 *
	 * @param client     	The client context.
	 * @param hInst			A handle to the Environs instance
	 */
	@SuppressLint("NewApi")
	private boolean Create ( @NonNull Context client, @IntRange (from=1, to=ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE) int hInst )
	{
		if (Utils.isDebug) Utils.Log ( 4, className, "Create" );

		hEnvirons = hInst;

		// Make sure that we were given an Activity context and not an application context
		try {
			clientActivity = (Activity) client;
			currentActivity = clientActivity;
		}
		catch (Exception e) {
			Utils.LogE ( className, "Create: Environment must be initialized with an Activity context! (instead of an application context)!");
			e.printStackTrace();
			return false;
		}

		if ( Utils.APILevel >= 14 ) {
			((android.app.Application) client.getApplicationContext() ).registerActivityLifecycleCallbacks ( new ActivityTracker() );
		}

		if ( !InitNative(client) )
			return false;

		if ( GetStatus() > Status.Uninitialized )
			DisposeN ( hEnvirons );

		Utils.Init(clientActivity);

		SetDeviceDimsN ( Utils.width, Utils.height, Utils.width_mm, Utils.height_mm, 0, 0 );

		DeviceInstanceSize = GetDeviceInstanceSizeN ( );
		DevicesHeaderSize = GetDevicesHeaderSizeN ( );
		return true;
	}


	private boolean InitInstance()
	{
		if (Utils.isDebug) Utils.Log ( 4, className, "InitInstance" );

		if ( clientActivity == null ) {
			Utils.LogE(className, "InitInstance: Environment must be initialized with a valid Activity context!");
			return false;
		}

		sizeOfInputPack = GetSizeOfInputPackN ( );
		if (Utils.isDebug) Utils.Log ( 6, className, "InitInstance: Size of InputPack: " + sizeOfInputPack);

		if ( sizeOfInputPack <= 0 )
			return false;

		/// Register the main UI thread to Environs
		if (Looper.getMainLooper().getThread() == Thread.currentThread()) {
			RegisterMainThreadN ( hEnvirons );
		} else {
			clientActivity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					RegisterMainThreadN ( hEnvirons );
				}
			});
		}

		//initialized = true;

		if ( !Utils.CheckManifest(this, clientActivity) ) {
			Utils.LogE ( className, "InitInstance: Application permissions in AndroidManifest.xml is incomplete!");
			return false;
		}

		// Request always on display
		if (Utils.isDebug) Utils.Log ( 6, className, "InitInstance: Requesting display always on");
		clientActivity.getWindow().setFlags (
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

		// Register for sensor services updates
		Context ctx = clientActivity.getApplicationContext();

		if (Utils.isDebug) Utils.Log ( 6, className, "InitInstance: Register sensor service updates" );

		sensors.Init ( clientActivity );

		// Check whether our client provides
		if ( GetUsePushNotificationsN ( hEnvirons ) ) {
			if ( Utils.CheckManifestGCM(clientActivity) ) {
				GCMRegistrar.checkDevice ( clientActivity );
				GCMRegistrar.checkManifest ( clientActivity );

				gcm_reg_id = GCMRegistrar.getRegistrationId ( clientActivity );

				// Check if regid already presents
				if ( gcm_reg_id.equals("") ) {
					// Registration is not present, register now with GCM
					GCMRegistrar.register ( clientActivity, Utils.PROJECT_ID );
				} else {
					// Device is already registered on GCM
					if ( GCMRegistrar.isRegisteredOnServer ( clientActivity ) ) {
						// Skips registration.
						Utils.LogW ( className, "Device is already registered with GCM" );
					} else {
						//final Context fCtx = ctx;
						final String regId = gcm_reg_id;
						mRegisterTask = new AsyncTask<Void, Void, Void>(){

							@Override
							protected Void doInBackground(Void... params) {
								// Register on our server
								// On server creates a new user
								//Utils.register(fCtx, regId);
								GCMRegID(regId);
								return null;
							}

							@Override
							protected void onPostExecute(Void result) {
								mRegisterTask = null;
							}

						};
						mRegisterTask.execute(null, null, null);
					}
				}

				Bundle intentExtras = clientActivity.getIntent().getExtras();
				if (intentExtras != null) {
					String msg = intentExtras.getString("msg");
					if (msg != null && !msg.equals(""))
						OnPushMessage(msg);
				}
			}
		}

		SetUsePlatformPortalGenerator(true);

		return true;
	}


	/**
	 * Dispose the storage, that is remove all data and messages received in the data store.
	 *
	 */
	public void ClearStorage () {
		ClearStorageN ();
	}
	private static native void ClearStorageN ();


	/**
	 * Parameters that influences environment behaviour
	 */
	
	/** Show Environs messages and status as toast messages 	*/
	public static boolean showConnectionToasts = true;

	/** Make sure that WiFi is enabled and working on Start of Environs. 	*/
	public static boolean useWiFiCheck = true;

	/** Make sure that access to user accounts is granted. 	*/
	static boolean useAccounts = true;
	
	/** Wake up, turn on the display, and Start your application on receive of a push notification. 	*/
	public static boolean useWakup = true;
	
	/** If true, then extended TouchDispatch data (physical size, axis lengths, etc) are included in TouchDispatch events. 	*/
	public static boolean useExtendedTouch = true;

	/** Use lastLocation services, GPS. 	*/
	public static boolean useLocation = true;

	/** Enables bluetooth observer. Requires permission android.permission.BLUETOOTH */
	public static boolean useBluetooth = true;

	/** Enables bluetooth observer. Requires permission android.permission.BLUETOOTH_ADMIN */
	public static boolean useBluetoothAdmin = true;


    /**
     * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
     *
     * @return		true = yes, false = no
     */
	public boolean GetUseMediatorLoginDialog() {
		return GetUseMediatorLoginDialogN ( hEnvirons );
	}

	static native boolean GetUseMediatorLoginDialogN(int hInst);


    /**
     * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
     *
     * @param enable      true = enable, false = disable
     */
	public void SetUseMediatorLoginDialog(boolean enable) {
		SetUseMediatorLoginDialogN ( hEnvirons, enable );
	}
	static native void SetUseMediatorLoginDialogN(int hInst, boolean enable);


    /**
     * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
     *
     * @return		true = yes, false = no
     */
	public boolean GetMediatorLoginDialogDismissDisable() {
		return GetMediatorLoginDialogDismissDisableN ( hEnvirons );
	}
	static native boolean GetMediatorLoginDialogDismissDisableN(int hInst);


	/**
	 * Enable or disable device list update notifications from Mediator layer.
	 * In particular, mobile devices should disable notifications if the devicelist is not
	 * visible to users or the app transitioned to background.
	 * This helps recuding cpu load and network traffic when not required.
	 *
	 * @param enable      true = enable, false = disable
	 */
	public void SetMediatorNotificationSubscription(boolean enable) {

		boolean stateBefore = GetMediatorNotificationSubscription ();
		if ( stateBefore == enable )
			return;

		SetMediatorNotificationSubscriptionN ( hEnvirons, enable ? 1 : 0 );

		if ( !stateBefore )
			ReloadLists ();

	}
	static native void SetMediatorNotificationSubscriptionN(int hInst, int enable);


	/**
	 * Get subscription status of device list update notifications from Mediator layer.
	 *
	 * @return enable      true = enable, false = disable
	 */
	public boolean GetMediatorNotificationSubscription() {
		return (GetMediatorNotificationSubscriptionN ( hEnvirons ) == 1);
	}
	static native int GetMediatorNotificationSubscriptionN(int hInst);


	/**
	 * Enable or disable short messages from Mediator layer.
	 * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
	 * This helps recuding cpu load and network traffic when not necessary.
	 *
	 * @param enable      true = enable, false = disable
	 */
	public void SetMessagesSubscription(boolean enable) {

		boolean stateBefore = GetMessagesSubscription ( );
		if ( stateBefore == enable )
			return;

		SetMessagesSubscriptionN ( hEnvirons, enable ? 0 : 1 );
	}
	static native void SetMessagesSubscriptionN(int hInst, int enable);


	/**
	 * Get subscription status of short messages from Mediator layer.
	 *
	 * @return enable      true = enable, false = disable
	 */
	public boolean GetMessagesSubscription() {
		return (GetMessagesSubscriptionN ( hEnvirons ) == 1);
	}

	static native int GetMessagesSubscriptionN(int hInst);


    /**
     * Instruct Environs disable Mediator settings on dismiss of the login dialog.
     *
     * @param enable      true = enable, false = disable
     */
	public void SetMediatorLoginDialogDismissDisable(boolean enable) {
		SetMediatorLoginDialogDismissDisableN ( hEnvirons, enable );
	}
	static native void SetMediatorLoginDialogDismissDisableN(int hInst, boolean enable);


	/**
	 * Instruct Environs to show log messages in the status log.
	 *
	 * @param enable      true = enable, false = disable
	 */
	public static native void SetUseNotifyDebugMessageN(boolean enable);

	/**
	 * Query Environs settings that show log messages in the status log.
	 *
	 * @return enable      true = enable, false = disable
	 */
	public static native boolean GetUseNotifyDebugMessageN();


	/**
	 * Instruct Environs to create and write a log file in the working directory.
	 *
	 * @param enable      true = enable, false = disable
	 */
	public void SetUseLogFile(boolean enable) {
		SetUseLogFileN ( enable );
	}
	static native void SetUseLogFileN(boolean enable);

	/**
	 * Query Environs settings that create and write a log file in the working directory.
	 *
	 * @return enable      true = enable, false = disable
	 */
	public boolean GetUseLogFile() {
		return GetUseLogFileN ( );
	}
	static native boolean GetUseLogFileN();



	/**
	 * Instruct Environs to create DeviceLists that are used as UIAdapter by client code.
	 * Any changes of those lists are made within the applications main / UI thread context.
	 * Changing this option is allowed only before the Start of the very first Environs instance
	 * (or after stopping of all Environs instances).
	 *
	 * @param enable      true = enable, false = disable
	 */
	public void SetUseDeviceListAsUIAdapter ( boolean enable )
	{
		if ( GetStatus () > Status.Stopped ) {
			Utils.LogE ( className, "SetUseDeviceListAsUIAdapter: Option cannot be changed when at least on instance has started." );
			return;
		}
		isUIAdapter = enable;
	}


	/**
	 * Query Environs settings whether to create DeviceLists that are used as UIAdapter by client code.
	 * Any changes of those lists are made within the applications main / UI thread context.
	 *
	 * @return enable      true = enabled, false = disabled
	 */
	boolean GetUseDeviceListAsUIAdapter ()
	{
		return isUIAdapter;
	}

	
	/**
	 * The application instance that we're serving for. 
	 * It has no access modifier so as to be accessed by internal framework objects within the same package. 
	 */
	private Activity clientActivity;

	static Activity currentActivity;

	@SuppressWarnings ( "all" )
	final ArrayList<EnvironsObserver>			observers 			= new ArrayList<EnvironsObserver>();

	@SuppressWarnings ( "all" )
	final ArrayList<EnvironsMessageObserver>	observersForMessages = new ArrayList<EnvironsMessageObserver>();

	@SuppressWarnings ( "all" )
	final ArrayList<EnvironsDataObserver>		observersForData 	= new ArrayList<EnvironsDataObserver>();

	@SuppressWarnings ( "all" )
	final ArrayList< EnvironsSensorObserver >	observersForSensorsData 	= new ArrayList< EnvironsSensorObserver >();

	/**
	 * A collection of device lists that this instance of environs manages
	 */
	@SuppressWarnings ( "all" )
	ArrayList<DeviceList>		deviceLists = new ArrayList<DeviceList>();

	ArrayList<DeviceInstance> 	listAll;

	ArrayList<DeviceInstance> 	listNearby;

	ArrayList<DeviceInstance> 	listMediator;

	@SuppressWarnings ( "all" )
	final ArrayList<ListObserver>		listAllObservers 	= new ArrayList<ListObserver> ( );

	@SuppressWarnings ( "all" )
	ArrayList<ListObserver>		listNearbyObservers = new ArrayList<ListObserver>();

	@SuppressWarnings ( "all" )
	ArrayList<ListObserver>		listMediatorObservers = new ArrayList<ListObserver> ( );

	int listAllUpdate = 0;

	int listNearbyUpdate = 0;

	int listMediatorUpdate = 0;

	/*
	 * DeviceInstance notifier thread resources for an instance of Environs
	 */
	final LinkedList deviceInstanceNotifierQueue = new LinkedList();

	boolean		deviceInstanceNotifierRun = false;

	Thread		deviceInstanceNotifier;


	/*
	 * DeviceList notifier thread resources for an instance of Environs
	 */
	boolean listNotifierThreadRun = false;

	Thread listNotifierThread;

	final LinkedList listNotifierQueue = new LinkedList();


	/*
	 * DeviceList notifier thread resources for an instance of Environs
	 */
	boolean listUpdateThreadRun = false;

	Thread listUpdateThread;

	final LinkedList listUpdateQueue = new LinkedList();


	/**
	 * Get the type of notification that matches one of the values of Environs.NOTIFY_TYPE_*
	 * 
	 * @param	event       The notification.
	 * @return	notifyType
	 */
 	public static int GetNotifyType(int event) {
 		return (event & Types.MSG_NOTIFY_CLASS);
 	}
	
 	
	/**
	 * Get the native version of Environs.
	 * 
	 * @return		version string
	 */
 	public static native String GetVersionStringN();
	
	/**
	 * Get the native major version of Environs.
	 * 
	 * @return		major version
	 */
 	public static native int GetVersionMajorN();
	
	/**
	 * Get the native minor version of Environs.
	 * 
	 * @return		minor version
	 */
 	public static native int GetVersionMinorN();
	
	/**
	 * Get the native revision of Environs.
	 * 
	 * @return		revision
	 */
 	public static native int GetVersionRevisionN();
 	
 	
	/**
	 * Get the device id that the application has assigned to the instance of Environs.
	 * 
	 * @return		deviceID
	 */
 	public int GetDeviceID() {
		return GetDeviceIDN ( hEnvirons );
	}
	static native int GetDeviceIDN ( int hInst );
	

	/**
	 * Set the device id that is assigned to the instance of Environs.
	 * 
	 * @param		deviceID    The device id of the target device.
	 */
	static native void SetDeviceIDN ( int hInst, int deviceID );

	/**
	 * Set the device id that is assigned to the instance of Environs.
	 * 
	 * @param		deviceID    The device id of the target device.
	 */
	public void SetDeviceID(int deviceID) {
		if (!IsInstalled() ) {
			return;
		}

		SetDeviceIDN ( hEnvirons, deviceID );
		
		if ( deviceID != 0 )
			Utils.SaveIDToStorage(deviceID);
	}


	/**
	 * Enable extended TouchDispatch protocol that includes angle, size, etc..
	 *
	 * @param		enable	Enable or disable extended TouchDispatch packets.
	 */
	public void SetUseExtendedtouch( boolean enable) {
		SetUseExtendedtouchN ( hEnvirons, enable );
	}
	static native void SetUseExtendedtouchN( int hInst, boolean enable);
	
 	
	/**
	 * Request a device id from mediator server instances that have been provided before this call.
	 * Prior to this call, the area and application name must be provided as well,
	 * in order to get an available device id for the specified application environment.
	 * If the application has already set a deviceID (using SetDeviceID), this call returns the previously set value.
	 *
	 * @return		deviceID	The device id returned by a Mediator service.
	 */
	public int GetDeviceIDFromMediator() {
		return GetDeviceIDFromMediatorN ( hEnvirons );
	}
 	static native int GetDeviceIDFromMediatorN( int hInst );


	/**
	 * Get the current wireless name.
	 *
	 * @param		desc    If true, then the wifi name contains additional infos such as signal strenght, channel, etc.
	 * @return		Our 	current wifi name (ssid)
	 */
	public String GetSSID(boolean desc) {
		return Utils.GetSSID(hEnvirons, desc);
	}

	/**
	 * Get the current wireless name.
	 *
	 * @return		Our 	current wifi name (bssid)
	 */
	public String GetBSSID() {
		return Utils.GetBSSID(hEnvirons);
	}

	/**
	 * Get the current wireless name.
	 *
	 * @return		Our 	current wifi name (rssi)
	 */
	public int GetRSSI() {
		return Utils.GetRSSI(hEnvirons);
	}
	
 	
	/**
	 * Get the current active ip address.
	 * 
	 * @return		Our current IP address
	 */
 	public static native int GetIPAddressN();
	
 	
	/**
	 * Get the subnet mask of the current active ip address.
	 * 
	 * @return		Subnet mask
	 */
 	public static native int GetSubnetMaskN();
 	

	/**
	 * Set whether Environs shall ask back (through dialogs) whether portals should established also using mobile data.
	 * 
	 * @param		enable A boolean that determines the target state.
	 */
	public void SetUseTrafficCheck(boolean enable) {
		opt ( "useTrafficCheck", enable ? "1" : "0" );
	}

	/**
	 * Get setting whether Environs shall ask back (through dialogs) whether portals should established also using mobile data.
	 * 
	 * @return		enable
	 */
	public boolean GetUseTrafficCheck() {
		return opt ( "useTrafficCheck" ).equals ( "1" );
	}

	
	/**
	 * Get the status, whether the device (id) has direct physical contact, such as lying on the surface.
	 *
	 * @param 	nativeID	Destination native device id
     * @return	success		true = yes, false = no
	 */
	public boolean GetDirectContactStatus( int nativeID) {
		return GetDirectContactStatusN ( hEnvirons, nativeID );
	}
	static native boolean GetDirectContactStatusN ( int hInst, int nativeID );

	/**
	 * Determines whether the portalID refers to an incoming portal
	 *
	 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
	 * @return				true = success, false = failed.
	 */
	public static boolean GetIsPortalIncoming(int portalID)
	{
		return ((portalID & Types.PORTAL_DIR_INCOMING) == Types.PORTAL_DIR_INCOMING);
	}

	/**
	 * Determines whether the portalID refers to an outgoing portal
	 *
	 * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
	 * @return				true = success, false = failed.
	 */
	public static boolean GetIsPortalOutgoing(int portalID)
	{
		return ((portalID & Types.PORTAL_DIR_OUTGOING) == Types.PORTAL_DIR_OUTGOING);
	}

	
	/**
	 * Get the status, whether the device (id) has established an active portal
	 * 
	 * @param 	nativeID    The native device id of the target device.
	 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
     * @return	success 	true = yes, false = no
	 */
	public boolean GetPortalEnabled(int nativeID, int portalType) {
		return GetPortalEnabledN ( hEnvirons, nativeID, portalType );
	}
	static native boolean GetPortalEnabledN(int hInst, int nativeID, int portalType);


	/**
	 * Get the portalID of the first active portal
	 *
	 * @param 	nativeID    The native device id of the target device.
	 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
	 * @return	portalID 	The portal ID.
	 */
	public int GetPortalID(int nativeID, int portalType) {
		return GetPortalIDN ( hEnvirons, nativeID, portalType );
	}
	static native int GetPortalIDN(int hInst, int nativeID, int portalType);

	
	/**
	 * Delete public/private keys and certificates
	 * 
     * @return	success
	 */
	public boolean DeleteKeys() {

		if (instances[hEnvirons] == null || instances[hEnvirons].clientActivity == null) {
			Utils.LogW ( className, "opt: No instance or activity available.");
			return false;
		}

        SharedPreferences prefs = instances[hEnvirons].clientActivity.getPreferences(Context.MODE_PRIVATE);
        if (prefs == null) {
			Utils.LogW ( className, "opt: Failed to get shared preferences.");
			return false;
        }

    	SharedPreferences.Editor editor = prefs.edit();
    	if (editor == null) {
			Utils.LogW ( className, "opt: Failed to get editor for shared preferences. ");
			return false;
        }

        //Utils.Log ( className, "Environs.opt: Saving " + key + " value:" + value );
    	editor.remove("optPrivCert");  
    	editor.remove("optPubCert");  
		return editor.commit ( );
	}


	static Activity GetActivity ( int hInst )
	{
		Activity act = currentActivity;

		if ( act == null ) {
			if ( instances[hInst] != null )
				act = instances[hInst].clientActivity;
		}
		return act;
	}

	
	/**
	 * Persistently store an option identified by a keyname with a concrete value.
	 * 
	 * @param	key     The key of the option to store.
	 * @param	value   The value for the option.
	 * @return	success
	 */
	public boolean opt ( String key, String value ) {
		return opt ( hEnvirons, key, value );
	}

	static boolean opt ( int hInst, String key, String value )
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "opt [" + hInst + "]: Saving " + key  + " [" + value + "]");

		Activity act = GetActivity ( hInst );
		if ( act == null ) {
			Utils.LogE ( className, "opt: no instance or clientActivity!" );
			return false;
		}

		String err = key + " will not be stored to preferences.";

		SharedPreferences prefs = act.getPreferences(Context.MODE_PRIVATE);
		if (prefs == null) {
			Utils.LogW ( className, "opt: Failed to get shared preferences. " + err );
			return false;
		}

		SharedPreferences.Editor editor = prefs.edit();
		if (editor == null) {
			Utils.LogW ( className, "opt: Failed to get editor for shared preferences. " + err );
			return false;
		}

		//Utils.Log ( 1, "[INFO]  Environs.opt: Saving " + key + " value:" + value );
		editor.putString(key, value);
		return editor.commit ( );
	}


	static boolean optBytes ( int hInst, byte [] key, byte [] value )
	{
		String jKey = "";
		String jValue = "";

		try {
			if ( key != null )
				jKey = new String ( key, "UTF-8" );
			if ( value != null )
				jValue = new String ( value, "UTF-8" );

			return opt ( hInst, jKey, jValue );
		}
		catch ( Exception ex ) {
			ex.printStackTrace ();
		}
		return false;
	}


	int appAreaID = 0;

	String BuildOptKey ( String key )
	{
		return String.valueOf(appAreaID) + "_" + key;
	}

	static native int GetAppAreaIDN(int hInst);

	/**
	 * Query the persistent option identified by a keyname
	 * 
	 * @param	key     The key of the option to query.
	 * @return	value
	 */
	public String opt ( String key ) {
		return opt ( hEnvirons, BuildOptKey ( key ) );
	}

	static String opt ( int hInst, String key )
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "opt [" + hInst + "]: Loading String " + key );

		Activity act = GetActivity ( hInst );
		if ( act == null ) {
			Utils.LogE ( className, "opt: no instance or clientActivity!" );
			return "0";
		}

		SharedPreferences prefs = act.getPreferences(Context.MODE_PRIVATE);
		if ( prefs == null )
			return "0";

		return prefs.getString ( key, "1" );
	}

	static String optBytes ( int hInst, byte [] key )
	{
		String jKey = "";

		try {
			if ( key != null )
				jKey = new String ( key, "UTF-8" );

			return opt ( hInst, jKey );
		}
		catch ( Exception ex ) {
			ex.printStackTrace ();
		}
		return "1";
	}


	public boolean optBool (String key ) {
		return optBool ( hEnvirons, key );
	}


	static boolean optBool ( int hInst, String key )
	{
		if (Utils.isDebug) Utils.Log ( 6, className, "opt [" + hInst + "]: Loading Bool " + key );

		Activity act = GetActivity ( hInst );
		if ( act == null ) {
			Utils.LogE ( className, "opt: no instance or clientActivity!" );
			return false;
		}

		SharedPreferences prefs = act.getPreferences ( Context.MODE_PRIVATE );

		if ( prefs != null ) {
			String value = prefs.getString ( key, "1" );
			try
			{
				return value.equalsIgnoreCase ( "1" );
			}
			catch ( Exception ex ) {
				ex.printStackTrace ();
			}
		}

		return false;
	}

	@SuppressWarnings ( "all" )
	private static String gcmRegID = null;
	
	static void GCMRegID (String regID) {
		gcmRegID = regID;
		
		SetGCMN ( gcmRegID );
	}


    /**
     * GCM preferencs
     */
    String gcm_reg_id = null;
    
	private static AsyncTask<Void, Void, Void> mRegisterTask = null;

    /** 
     * Query whether the name of the current device has been set before.
     * 
     * @return	has DeviceUID
     */
    static native boolean HasDeviceUIDN();

	/**
	 * Query whether the native layer was build for release (or debug).
	 *
	 * @return	true = Release build, false = Debug build.
	 */
	static native boolean GetIsReleaseBuildN();


	/**
     * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
     * 
     * @param 	deviceUID A unique identifier to identify this device.
     * @return	success
     */
    static native boolean SetDeviceUIDN(String deviceUID);
    
    
    /** 
     * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
     * 
     * @param 	deviceName  The device name.
     * @return	success
     */
	public boolean SetDeviceName(String deviceName) {
		return SetDeviceNameN ( deviceName );
	}
    static native boolean SetDeviceNameN(String deviceName);
    
    
    /** 
     * Set the user name for authentication with a mediator service.&nbsp;Usually the user's email address is used as the user name.
     * 
     * @param 	username    The user name for authentication at the Mediator.
     * @return	success
     */
    @SuppressLint("DefaultLocale")
	public boolean SetMediatorUserName(String username) {
    	return (username != null && SetMediatorUserNameN ( hEnvirons, username.toLowerCase ( ) ) );
    }
	public boolean SetUserName(String username) {
    	return SetMediatorUserName(username);
    }    
    
    /** 
     * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
     * 
     * @param 	username    The user name for authentication at the Mediator.
     * @return	success
     */
    static native boolean SetMediatorUserNameN(int hInst, String username);
 	

 	/**
     * Query UserName used to authenticate with a Mediator.
     * 
     * @return UserName The user name for authentication at the Mediator.
     */
 	public String GetMediatorUserName() {
 		return GetMediatorUserNameN ( hEnvirons );
 	}

	static native String GetMediatorUserNameN(int hInst);


	/**
	 * Enable or disable anonymous logon to the Mediator.
	 *
	 * @param 	enable A boolean that determines the target state.
	 */
	public void SetUseMediatorAnonymousLogon(boolean enable) {
		SetUseMediatorAnonymousLogonN ( hEnvirons, enable );
	}
	static native void SetUseMediatorAnonymousLogonN(int hInst, boolean enable);


	/**
	 * Get setting of anonymous logon to the Mediator.
	 *
	 * @return	enabled A boolean that determines the target state.
	 */
	public boolean GetUseMediatorAnonymousLogon() {
		return GetUseMediatorAnonymousLogonN ( hEnvirons );
	}
	static native boolean GetUseMediatorAnonymousLogonN(int hInst);
 	
    
    /** 
     * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
     * 
     * @param 	password    The mediator password.
     * @return	success
     */
    public boolean SetMediatorPassword(String password) {
    	return (password != null && SetMediatorPasswordN ( hEnvirons, password ));
    }

	public boolean SetUserPassword(String password) {
		return SetMediatorPassword(password);
	}

	static native boolean SetMediatorPasswordN(int hInst, String password);


	/**
	 * Enable or disable authentication with the Mediator using username/password.
	 *
	 * @param 	enable A boolean that determines the target state.
	 */
	public void SetUseAuthentication(boolean enable) {
		SetUseAuthenticationN ( hEnvirons, enable );
	}
	static native void SetUseAuthenticationN(int hInst, boolean enable);


    /** 
     * Set app status in order to enable or disable sleep mode, e.g. if the app runs in background. 
     * In APP_STATUS_SLEEPING, all network activities (except heartbeat packets) are disabled.
     * 
     * @param 	status	One of the APP_STATUS_* values. 
     */
	public void SetAppStatus(int status) {
		SetAppStatusN ( hEnvirons, status );
	}
	static native void SetAppStatusN(int hInst, int status);


	/**
	 * Add an observer for communication with Environs and devices within the environment.
	 *
	 * @param observer Your implementation of EnvironsObserver.
	 */
	public void AddObserver(EnvironsObserver observer) {
		//Utils.Log(className, "AddObserver");
		if (!IsInstalled() ) {
			return;
		}

		synchronized ( observers )
		{
			if (observer == null) {
				Utils.LogE ( className, "AddObserver: called with NULL Listener argument! The application will not be able to interact with the environment!");
			}

			if (observers.contains(observer))
				return;

			observers.add(observer);
		}
	}

	/**
	 * Remove an observer for communication with Environs and devices within the environment.
	 *
	 * @param observer Your implementation of EnvironsObserver.
	 */
	public void RemoveObserver(EnvironsObserver observer) {
		//Utils.Log(className, "RemoveObserver");

		if (observer == null)
			return;

		synchronized ( observers )
		{
			if (observers.contains(observer))
				observers.remove(observer);
		}
	}


	/**
	 * Add an observer for receiving messages.
	 *
	 * @param observer Your implementation of EnvironsMessageObserver.
	 */
	public void AddObserverForMessages(EnvironsMessageObserver observer) {
		//Utils.Log(className, "AddObserverForMessages");
		if (!IsInstalled() ) {
			return;
		}

		synchronized ( observersForMessages )
		{
			if (observer == null) {
				Utils.LogE ( className, "AddObserverForMessages: called with NULL Listener argument!");
			}

			if (observersForMessages.contains(observer))
				return;

			observersForMessages.add(observer);
		}
	}

	/**
	 * Remove an observer for receiving messages.
	 *
	 * @param observer Your implementation of EnvironsMessageObserver.
	 */
	public void RemoveObserverForMessages(EnvironsMessageObserver observer) {
		//Utils.Log(className, "RemoveObserverForMessages");

		if (observer == null)
			return;

		synchronized ( observersForMessages )
		{
			if (observersForMessages.contains(observer))
				observersForMessages.remove(observer);
		}
	}


	/**
	 * Add an observer for receiving data buffers and files.
	 *
	 * @param observer Your implementation of EnvironsDataObserver.
	 */
	public void AddObserverForData(EnvironsDataObserver observer) {
		//Utils.Log(className, "AddObserverForData");
		if (!IsInstalled() ) {
			return;
		}

		synchronized ( observersForData )
		{
			if (observer == null) {
				Utils.LogE ( className, "AddObserverForData: called with NULL Listener argument!");
			}

			if (observersForData.contains(observer))
				return;

			observersForData.add(observer);
		}
	}

	/**
	 * Remove an observer for receiving data buffers and files.
	 *
	 * @param observer Your implementation of EnvironsDataObserver.
	 */
	public void RemoveObserverForData(EnvironsDataObserver observer) {
		//Utils.Log(className, "RemoveObserverForData");

		if (observer == null)
			return;

		synchronized ( observersForData )
		{
			if (observersForData.contains(observer))
				observersForData.remove(observer);
		}
	}


	/**
	 * Add an observer for receiving sensor data of all devices.
	 * Please note: This observer reports sensor data of all devices that are connected and send to us.
	 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
	 *
	 * @param   observer Your implementation of EnvironsSensorObserver.
	 *
	 * @return	success
	 */
	public void AddObserverForSensorData(EnvironsSensorObserver observer) {
		//Utils.Log(className, "AddObserverForSensorData");
		if (!IsInstalled() ) {
			return;
		}

		synchronized ( observersForSensorsData )
		{
			if (observer == null) {
				Utils.LogE ( className, "AddObserverForSensorData: called with NULL Listener argument!");
			}

			if (observersForSensorsData.contains(observer))
				return;

			observersForSensorsData.add(observer);
		}
	}


	/**
	 * Remove an observer for receiving data buffers and files.
	 * Please note: This observer reports sensor data of all devices that are connected and send to us.
	 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
	 *
	 * @param   observer Your implementation of EnvironsSensorObserver.
	 *
	 * @return	success
	 */
	public void RemoveObserverForSensorData(EnvironsSensorObserver observer) {
		//Utils.Log(className, "RemoveObserverForSensorData");

		if (observer == null)
			return;

		synchronized ( observersForSensorsData )
		{
			if (observersForSensorsData.contains(observer))
				observersForSensorsData.remove(observer);
		}
	}
	
    
	/** 
	 * Add a native recognizer module to the recognizer chain.
	 *  
	 * @param moduleName 	This must be the name of the recognizer module that is packed within your application package, e.g. libEnv-RecDummy.so
	 * @param enable		Enable or disable the recognizer.
	 * @return success returns true if successful
	 */
    public boolean SetUseTouchRecognizer(String moduleName, boolean enable) {

		if ( moduleName == null )
			return false;

		if ( enable ) {
			if ( moduleName.length() < 6 )
				return false;

			if (Libs.isJAR) {
				String path = Libs.extract ( moduleName );
				if ( path == null ) {
					return false;
				}
			}
			else {
				if ( !Libs.LoadLibrary ( moduleName, false, false ) )
					return false;
			}
		}

		/// If the modulename does not end with .so, we assume it to be Environs -> libEnvirons.so
		/// .so will be appended by native layer
		/// lib has to be prepended by platform layer
		if ( !moduleName.endsWith(".so") && !moduleName.startsWith("lib") )
			moduleName = "lib" + moduleName;

		return SetUseTouchRecognizerN ( hEnvirons, moduleName, enable );
		/*Libs.extract ( "Env-RecDummy" );
		Libs.extract ( "Env-RecGestureBezelTouch" );
		Libs.extract ( "Env-RecGestureThreeTouch" );
		*/
	}

	static native boolean SetUseTouchRecognizerN(int hInst, String moduleName, boolean enable);



	/**
	 * Enable or disable all touch recognizers.
	 *
	 * @param enable		Enable or disable.
	 */
	public void SetUseTouchRecognizerEnable ( boolean enable ) {
		SetUseTouchRecognizerEnableN ( hEnvirons, enable );
	}

	static native void SetUseTouchRecognizerEnableN ( int hInst, boolean enable );


	static native boolean AllocPlatformN();

    
	// Attention: current_deviceID is only for testing purposes, keep track of the connected devices
	// in your App state
	private static int current_deviceID = 0;
	private static String current_deviceType = null;
	private static String current_deviceAppName = null;


	/**
	 * Determine whether the event of Environs status type (NATIVE_STATUS_*) or not.
	 *
	 * @param notification	The event to check if it is of status type.
	 * @return true if it is of status type.
	 */
	public static boolean IsStatus(int notification) {
		return (notification < 0 && (notification & 0xF) != 0);
	}

	public static boolean IsNotStatus(int notification)
	{
		return (notification > 0 || (notification & 0xF) == 0);
	}

	@SuppressWarnings ( "all" )
	@Status.Value
	public static int GetStatus ( int notification )
	{
		return (0 - notification);
	}


	static void CreatePortalRequest(final int hInst, final int objID)
	{
		new Thread(new Runnable() {
			public void run(){
				Environs env = GetInstance(hInst);

				if (env == null || !env.GetPortalAutoStart())
					return;

				DeviceInstance device = null;

				for (DeviceList list : env.deviceLists) {
					device = list.GetDevice(objID);
					if ( device != null )
						break;
				}

				if ( device == null )
					return;

				if (!device.isConnected && !device.Connect(Call.Wait))
					return;

				PortalInstance portal = device.PortalRequest(PortalType.Any);
				if (portal == null )
					return;
				portal.status = PortalStatus.CreatedAskRequest;

				Utils.Log1 ( className, "CreatePortalRequest");

				PortalInstance.PresentPortalToObservers ( hInst, portal, Notify.Portal.AskRequest );
			}
		}).start();
	}


	static void OnPlatformNotify( final int hInst, final int objID, final int notification, final int source) {

		new Thread(new Runnable() {
			public void run(){
				BridgeForNotify(hInst, objID, notification, source, null, 0);
			}
		}).start();
	}


	int exceptionsStatus = 0;
	int exceptionsNotify = 0;

    /**
     * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
     * 		such as when a connection has been established or closed.
     *
	 * @param hInst			A handle to the Environs instance
     * @param objID      The native device id of the sender device.
     * @param notification  The received notification.
     * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
	 * @param context       A value that provides additional context information (if available).
     */
	@SuppressWarnings("unchecked")
    static void BridgeForNotify ( int hInst, int objID, int notification, int sourceIdent, byte[] contextPtr, int context )
    {
		if (Utils.isDebug) Utils.Log ( 5, className, "BridgeForNotify sender [" + objID + "] source [" + sourceIdent + "] [" + TypesResolver.resolveName(notification) + "]");

		Environs env = instances[hInst];
		if ( env == null ) return;

		ArrayList<EnvironsObserver> obs;

		synchronized ( env.observers )
		{
			obs = (ArrayList<EnvironsObserver>) env.observers.clone ();
		}

		int size = obs.size ();

		if (IsStatus(notification))
		{
			int status = GetStatus(notification);

			if ( status == Status.Started ) {
				env.ReloadLists ();
			}

			if (size > 0)
			{
				for (int i=0; i<size; i++)
				{
					EnvironsObserver observer = obs.get(i);

					try {
						observer.OnStatus ( status );
					} catch (Exception e) {
						e.printStackTrace ( );
					}
				}
			}
			return;
		}

		ObserverNotifyContext ctx = new ObserverNotifyContext();
		ctx.hEnvirons = hInst;
		ctx.destID = objID;
		ctx.notification = notification;
		ctx.sourceIdent = sourceIdent;
		ctx.contextPtr = contextPtr;
		ctx.context = context;
		ctx.env = env;

		int notifType = notification & Types.MSG_NOTIFY_CLASS;

		if ( notifType == Notify.Portal.type)
		{
			if (notification == Notify.Portal.StreamIncoming || notification == Notify.Portal.ImagesIncoming )
			{
				if (!GetUseNativeDecoderN ( hInst ))
				{
					PortalInstance portal = PortalInstance.GetPortal ( objID, sourceIdent );
					if (portal == null) {
						Utils.LogE(className, "BridgeForNotify: PortalInstance not found!");
						return;
					}

					String key = buildPortalKey(sourceIdent);

					synchronized (portalReceivers)
					{
						PortalReceiver receiver = portalReceivers.get(key);
						if (receiver == null) {
							// Create the listener thread and listen for portal stream packets
							receiver = new PortalReceiver();
							receiver.nativeID = objID;
							receiver.portal = portal;
							receiver.key = key;
							receiver.env = env;
							receiver.hEnvirons = hInst;

							portal.AddObserver(receiver);

							portalReceivers.put ( receiver.key, receiver );
							receiver.start ( );
						}
					}
				}
			}
			/*else if (notification == Types.NOTIFY_PORTAL_STREAM_STARTED)
			{
			}*/
			else if (notification == Notify.Portal.StreamStopped || notification == Notify.Connection.Closed ) {

				if ( notification == Notify.Portal.StreamStopped && (sourceIdent & Environs.PORTAL_DIR_OUTGOING) != 0 ) {

					String key = buildPortalKey(sourceIdent);

					synchronized (portalGenerators)
					{
						PortalGenerator portal = portalGenerators.get(key);
						if ( portal != null ) {
							portalGenerators.remove(key);
							portal.release();
						}
					}
				}
			}
			else if (notification == Notify.Portal.ProviderReady) {

				String key = buildPortalKey(sourceIdent);

				synchronized (portalGenerators)
				{
					PortalGenerator portal = portalGenerators.get(key);

					if (portal != null) {
						Utils.LogE(className, "Notify: Max count of active outgoing portals slots " + Environs.MAX_PORTAL_STREAMS_A_DEVICE + " occupied.");
					}
					else {
						// Create the portal generator
						/*if (useSurfaceEncoding)
							portal = new PortalGeneratorEGL();
						else
						*/
						portal = new PortalGenerator ();

						portal.destID = objID;
						portal.portalID = sourceIdent;

						String area = null;
						String app = null;

						if (env.areaName != null && env.appName != null) {
							if (!env.areaName.equalsIgnoreCase(env.GetAreaName()) || !env.appName.equalsIgnoreCase(env.GetApplicationName())) {
								area = env.areaName;
								app = env.appName;
							}
						}
						portal.areaName = area;
						portal.appName = app;
						portal.key = key;
						portal.env = env;
						portal.hEnvirons = hInst;

						portalGenerators.put(portal.key, portal);

						boolean success = false;

						if ( portal.init ( sourceIdent & Types.PORTAL_TYPE_MASK, 0, 0) )
						{
							if ( SendPortalInitN ( hInst, Types.CALL_WAIT, sourceIdent, portal.width, portal.height ) )
							{
								StartPortalStreamN ( hInst, Types.CALL_NOWAIT, sourceIdent );

								portal.start();

								success = true;
							}
						}

						if (!success) {

							StopPortalStreamN ( hInst, Types.CALL_NOWAIT, 0, sourceIdent );
						}
					}
				}
			}
			PortalInstance.Update ( env, objID, notification, sourceIdent );
		}
		else if ( notifType == Notify.Connection.type )
		{
			switch (notification)
			{
				case Notify.Connection.Closed:
					env.sensors.StopAllSensors ( );

				/*case NOTIFY_CONNECTION_ESTABLISHED:
					DeviceList.OnDeviceListNotification();
					break;
				*/

				case Notify.Connection.Progress:
					DeviceList.UpdateConnectProgress(env, objID, sourceIdent);
					break;

				case Notify.Connection.EstablishedAck:
					if (env.GetPortalAutoStart())
						CreatePortalRequest(hInst, objID);
					break;
			}
		}
		else if ( notifType == Notify.Environs.type )
		{
			if ((notification & Notify.Mediator.type) == Notify.Mediator.type)
			{
				if (notification == Notify.Mediator.DeviceListUpdateAvailable) {
					DeviceList.OnDeviceListNotification(env);
				}
				else
				{
					switch(notification) {
						case Notify.Mediator.ServerPasswordFail:
						case Notify.Mediator.ServerPasswordMissing:

							if ( env.GetUseMediatorLoginDialog () && !env.GetUseMediatorAnonymousLogon() && env.GetStatus () >= Status.Started )
							{
								LoginDialog dlg = LoginDialog.SingletonInstance(hInst,
										"", "Mediator Login", env.opt ( Types.APPENV_SETTING_TOKEN_MEDIATOR_USERNAME ));

								if (dlg != null)
									dlg.ShowResult();
							}
							break;

						default:
							DeviceList.OnDeviceListNotification ( ctx );
							break;
					}
				}
			}
			else if ( notification == Notify.Environs.DeviceOnSurface || notification == Notify.Environs.DeviceNotOnSurface )
			{
				PortalInstance.UpdateOptions(ctx, notification, sourceIdent);
			}
			else if ( notification == Notify.Environs.DeviceFlagsUpdate )
			{
				/**
				 * Update flags from mediator daemon by AsyncWorker which changed them to CPTypes of DeviceFlagsInternal
				 *
				 */
				DeviceInstance device = DeviceList.GetDevice ( env, objID );
				if (device != null) {
					if ( context != 0 )
						device.flags |= sourceIdent;
					else
						device.flags &= ~sourceIdent;

					device.NotifyObservers ( DeviceInfoFlag.Flags, true );
				}
			}
		}
		else if ( notifType == Notify.File.type )
		{
			DeviceList.OnDeviceListNotification(env, ctx);
		}
		else if ( notifType == Notify.Options.type ) {
			if (notification == Notify.Options.DirectContactChanged ) {
				DeviceList.OnDeviceListNotification(env, ctx);
			}
			else
				PortalInstance.UpdateOptions(ctx, notification, sourceIdent);
		}

		if (size > 0) {
			for (int i=0; i<size; i++)
			{
				EnvironsObserver observer = obs.get(i);

				try {
					observer.OnNotify(ctx);
				} catch (Exception e) {
					e.printStackTrace ( );
				}
			}
		}
    }


	/**
	 * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
	 * 		such as when a connection has been established or closed.
	 *
	 * @param hInst			A handle to the Environs instance
	 * @param deviceID      The device id of the sender device.
	 * @param areaName		Area name of the application environment.
	 * @param appName		Application name of the application environment.
	 * @param notification  The received notification.
	 * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
	 * @param context       A value that provides additional context information (if available).
	 */
	@SuppressWarnings("unchecked")
	static void BridgeForNotifyExt ( int hInst, int deviceID, byte[] areaName, byte[] appName, int notification, int sourceIdent, int context )
	{
		String area = "";
		String app = "";

		// Unfortunatelly, Android heavily crashes when using NewStringUTF in native layer on some utf8 strings.
		// We can't be sure when and on which API/ndk version this is fixed. Even though it may crash on
		// on older devices which (most probably) not (or never) got updated.
		// Therfore, we use the safe method through byte arrays.
		// Change of this in future and taking the risk of crashes is up to you ...
		try {
			if ( areaName != null )
				area = new String ( areaName, "UTF-8" );
			if ( appName != null )
				app = new String ( appName, "UTF-8" );

			Environs env = instances[hInst];
			if ( env == null ) return;

			ObserverNotifyContext ctx = new ObserverNotifyContext();
			ctx.hEnvirons = hInst;
			ctx.destID = deviceID;
			ctx.areaName = area;
			ctx.appName = app;
			ctx.notification = notification;
			ctx.sourceIdent = sourceIdent;
			ctx.context = context;
			ctx.env = env;

			int notifType = notification & Types.MSG_NOTIFY_CLASS;

			if ( notifType == Types.NOTIFY_TYPE_ENVIRONS )
			{
				DeviceList.OnDeviceListNotification ( ctx );
			}

		/*
		if ( notifType == NOTIFY_TYPE_CONNECTION )
		{
			if ( notification == NOTIFY_CONNECTION_MAIN_FAILED ) {
				DeviceList.OnDeviceListNotification();
			}
		}*/

			ArrayList<EnvironsObserver> obs;

			synchronized ( env.observers )
			{
				if ( env.observers.size () <= 0 )
					return;
				obs = (ArrayList<EnvironsObserver>) env.observers.clone ();
			}

			int size = obs.size ();

			for (int i=0; i<size; i++)
			{
				EnvironsObserver observer = obs.get(i);

				try {
					observer.OnNotifyExt ( ctx );
				} catch (Exception e) {
					e.printStackTrace ( );
				}
			}
		}
		catch ( Exception e ) {
			e.printStackTrace ();
		}
	}

    
    /**
     * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
     *
	 * @param hInst			A handle to the Environs instance
     * @param objID      The native device id of the sender device.
     * @param type          The type of this message.
     * @param buffer           The message.
	 * @param length        The length of the message.
     */
	@SuppressWarnings("unchecked")
    private static void BridgeForMessage ( int hInst, int objID, int type, byte[] buffer, int length )
    {
    	//Utils.Log(className, "BridgeForMessage from jni, sender " + senderID + ", msg: " + msg);
        if ( buffer == null )
        	return;

		// Unfortunatelly, Android heavily crashes when using NewStringUTF in native layer on some utf8 strings.
		// We can't be sure when and on which API/ndk version this is fixed. Even though it may crash on
		// on older devices which (most probably) not (or never) got updated.
		// Therfore, we use the safe method through byte arrays.
		// Change of this in future and taking the risk of crashes is up to you ...
		String msg;
		try {
			msg = new String ( buffer, "UTF-8" );

			Environs env = instances[hInst];
			if ( env == null ) return;

			ObserverMessageContext ctx = new ObserverMessageContext ();
			ctx.destID = objID;
			ctx.sourceType = type;
			ctx.message = msg;
			ctx.length = length;

			if (objID != 0 && type == Source.Device)
				DeviceList.UpdateMessage(env, ctx);

			ArrayList<EnvironsMessageObserver> obs;

			synchronized ( env.observersForMessages )
			{
				if ( env.observersForMessages.size () <= 0 )
					return;
				obs = (ArrayList<EnvironsMessageObserver>) env.observersForMessages.clone ();
			}

			int size = obs.size ();

			for (int i=0; i<size; i++)
			{
				EnvironsMessageObserver observer = obs.get(i);

				try {
					observer.OnMessage ( ctx );
				} catch (Exception e) {
					e.printStackTrace ( );
					if ( obs.size () > i )
						obs.remove ( i );
					if ( i > 0 ) i--;
				}
			}
		}
		catch ( Exception e ) {
			e.printStackTrace ();
		}
    }


	/**
	 * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
	 *
	 * @param hInst			A handle to the Environs instance
	 * @param deviceID      The device id of the sender device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
	 * @param type          The type of this message.
	 * @param buffer           The message.
	 * @param length        The length of the message.
	 */
	@SuppressWarnings("unchecked")
	private static void BridgeForMessageExt ( int hInst, int deviceID, byte[]  areaName, byte[]  appName, int type, byte[] buffer, int length )
	{
		if ( buffer == null )
			return;

		// Unfortunatelly, Android heavily crashes when using NewStringUTF in native layer on some utf8 strings.
		// We can't be sure when and on which API/ndk version this is fixed. Even though it may crash on
		// on older devices which (most probably) not (or never) got updated.
		// Therfore, we use the safe method through byte arrays.
		// Change of this in future and taking the risk of crashes is up to you ...
		String msg;
		String area = "";
		String app = "";
		try {
			msg = new String ( buffer, "UTF-8" );
			if ( areaName != null )
				area = new String ( areaName, "UTF-8" );
			if ( appName != null )
				app = new String ( appName, "UTF-8" );

			Environs env = instances[hInst];
			if ( env == null ) return;

			ObserverMessageContext ctx = new ObserverMessageContext ();
			ctx.destID = deviceID;
			ctx.areaName = area;
			ctx.appName = app;
			ctx.sourceType = type;
			ctx.message = msg;
			ctx.length = length;

			DeviceList.UpdateMessageExt ( env, ctx );

			ArrayList<EnvironsMessageObserver> obs;

			synchronized ( env.observersForMessages )
			{
				if ( env.observersForMessages.size () <= 0 )
					return;
				obs = (ArrayList<EnvironsMessageObserver>) env.observersForMessages.clone ();
			}

			int size = obs.size ();

			for (int i=0; i<size; i++)
			{
				EnvironsMessageObserver observer = obs.get(i);

				try {
					observer.OnMessageExt ( ctx );
				} catch (Exception e) {
					e.printStackTrace ( );
					if ( obs.size () > i )
						obs.remove ( i );
					if ( i > 0 ) i--;
				}
			}
		}
		catch ( Exception e ) {
			e.printStackTrace ();
		}
	}

    
    /**
     * BridgeForStatusMessage static method to be called by native layer in order to drop a status messages.
     *
	 * @param hInst			A handle to the Environs instance
     * @param msg A status message of Environs.
     */
	@SuppressWarnings("unchecked")
    private static void BridgeForStatusMessage ( int hInst, byte[] msg )
    {
    	//Utils.Log(className, "BridgeForStatusMessage msg: " + msg);

		// Unfortunatelly, Android heavily crashes when using NewStringUTF in native layer on some utf8 strings.
		// We can't be sure when and on which API/ndk version this is fixed. Even though it may crash on
		// on older devices which (most probably) not (or never) got updated.
		// Therfore, we use the safe method through byte arrays.
		// Change of this in future and taking the risk of crashes is up to you ...
		String jMsg;

		try {
			jMsg = new String ( msg, "UTF-8" );

			Environs env = instances[hInst];
			if ( env == null ) return;

			ArrayList<EnvironsMessageObserver> obs;

			synchronized ( env.observersForMessages )
			{
				if ( env.observersForMessages.size () <= 0 )
					return;
				obs = (ArrayList<EnvironsMessageObserver>) env.observersForMessages.clone ();
			}

			int size = obs.size ();

			for (int i=0; i<size; i++)
			{
				EnvironsMessageObserver observer = obs.get(i);

				try {
					observer.OnStatusMessage ( jMsg );
				} catch (Exception e) {
					e.printStackTrace ( );
					if ( obs.size () > i )
						obs.remove ( i );
					if ( i > 0 ) i--;
				}
			}
		}
		catch ( Exception e ) {
			e.printStackTrace ();
		}
    }


	/**
	 * BridgeForData static method to be called by native layer in order to notify about data received from a device.
	 *
	 * @param hInst			A handle to the Environs instance
	 * @param objID      	The object id of the sender device.
	 * @param nativeID      The native device id of the sender device.
	 * @param type          The type of this message.
	 * @param fileID        A fileID that was attached to the buffer.
	 * @param descriptor    A descriptor that was attached to the buffer.
	 * @param size          The size of the data buffer.
	 */
	@SuppressWarnings("unchecked")
	private static void BridgeForData ( int hInst, int objID, int nativeID, int type, int fileID, byte[] descriptor, int size)
    {
		// Unfortunatelly, Android heavily crashes when using NewStringUTF in native layer on some utf8 strings.
		// We can't be sure when and on which API/ndk version this is fixed. Even though it may crash on
		// on older devices which (most probably) not (or never) got updated.
		// Therfore, we use the safe method through byte arrays.
		// Change of this in future and taking the risk of crashes is up to you ...
		String desc = "";

		try {
			if ( descriptor != null )
				desc = new String ( descriptor, "UTF-8" );

			//Utils.Log ( className, "BridgeForData from " + deviceID + " " + size + " bytes and desc: " + fileDescriptor );

			ObserverDataContext ctx = new ObserverDataContext ();
			ctx.objID = objID;
			ctx.nativeID = nativeID;
			ctx.type = type;
			ctx.fileID = fileID;
			ctx.descriptor = desc;
			ctx.size = size;

			Environs env = instances[hInst];
			if ( env == null ) return;

			DeviceList.UpdateData(env, ctx);

			ArrayList<EnvironsDataObserver> obs;

			synchronized ( env.observersForData )
			{
				if ( env.observersForData.size () <= 0 )
					return;

				obs = (ArrayList<EnvironsDataObserver>) env.observersForData.clone ();
			}

			int count = obs.size ();

			for (int i=0; i<count; i++)
			{
				EnvironsDataObserver observer = obs.get(i);

				try {
					observer.OnData ( ctx );
				} catch (Exception e) {
					e.printStackTrace ( );
				}
			}
		}
		catch ( Exception e ) {
			e.printStackTrace ();
		}
    }


	@SuppressWarnings ( "all" )
	static SensorFrame GetSensorInputPack ( int objID, byte [] pack )
	{
		if ( pack == null )
			return null;

		ByteBuffer buffer = ByteBuffer.wrap ( pack );

		ByteOrder nativeOrder = ByteOrder.nativeOrder ( );
		if ( buffer.order() != nativeOrder )
			buffer.order ( nativeOrder );


		SensorFrame frame = new SensorFrame ();
		frame.objID = objID;
		frame.type = buffer.getInt ( 4 );

		frame.seqNumber = buffer.getInt ( 8 );

		if ( (frame.type & ENVIRONS_SENSOR_PACK_TYPE_DOUBLES) != 0  )
		{
			frame.d1 = buffer.getDouble ( 12 );
			frame.d2 = buffer.getDouble ( 20 );
			frame.d3 = buffer.getDouble ( 28 );
		}
		else
		{
			frame.f1 = buffer.getFloat ( 12 );
			frame.f2 = buffer.getFloat ( 16 );
			frame.f3 = buffer.getFloat ( 20 );

			if ( pack.length > 24 ) {
				frame.d1 = buffer.getDouble ( 24 );
				frame.d2 = buffer.getDouble ( 32 );
				frame.d3 = buffer.getDouble ( 40 );
			}
		}

		frame.type &= ~(ENVIRONS_SENSOR_PACK_TYPE_EXT | ENVIRONS_SENSOR_PACK_TYPE_DOUBLES);

		return frame;
	}


	/**
	 * BridgeForUdpData static method to be called by native layer in order to notify about udp data received from a device.
	 *
	 * @param hInst			A handle to the Environs instance
	 * @param objID         The native device id of the sender device.
	 * @param pack          A udp data structure containing the received udp or sensor data.
	 * @param packSize      The size of the data buffer in number of bytes.
	 */
	@SuppressWarnings ( "all" )
	private static void BridgeForUdpData ( int hInst, int objID, byte[] pack, int packSize )
	{
		if ( pack == null || packSize <= 0 || pack.length <= 3 )
			return;

		try
		{
			Environs env = instances[hInst];
			if ( env == null ) return;

			UdpDataContext ctx = new UdpDataContext();

			if ( pack [ 0 ] == 's' && pack [ 1 ] == 'f' && pack [ 2 ] == ':' )
			{
				ctx.sensorFrame = GetSensorInputPack ( objID, pack );

				if ( ctx.sensorFrame == null )
					return;
			}
			else {
				ctx.data     = pack;
			}
			ctx.objID = objID;

			BridgeForUdpData ( env, ctx );
		}
		catch ( Exception e ) {
			e.printStackTrace ();
			return;
		}
	}


	@SuppressWarnings ( "all" )
	static void BridgeForUdpData ( Environs env, UdpDataContext ctx )
	{
		if ( env == null ) return;

		if (ctx.objID > 0)
			DeviceList.UpdateSensorData ( env, ctx );

		if (ctx.sensorFrame == null)
			return;

		ArrayList< EnvironsSensorObserver > obs;

		synchronized ( env.observersForSensorsData )
		{
			if ( env.observersForSensorsData.size () <= 0 )
				return;

			obs = (ArrayList< EnvironsSensorObserver >) env.observersForSensorsData.clone ();
		}

		int count = obs.size ();

		for (int i=0; i<count; i++)
		{
			EnvironsSensorObserver observer = obs.get( i);

			try {
				observer.OnSensorData ( ctx.sensorFrame );
			} catch (Exception e) {
				e.printStackTrace ( );
			}
		}
	}


	static Integer	pluginObjectsIndexer	= 0;

	@SuppressWarnings ( "all" )
	static SparseArray<Object> pluginObjects = new SparseArray<Object>();

	/**
	 * Plugin_CreateInstance static method to be called by native layer in order to create an instance of a java plugin class.
	 *
	 * @param deviceID		The deviceID that the class instance is designated for.
	 * @param classType 	A type that identifies the class to be instantiated
	 * @param index 		A class specific index that is supplied to the instance.
	 * @return id 			An environs determined id that identifies this object during its lifecycle. (-1 means error)
	 */
	private static int Plugin_CreateInstance ( int hInst, int deviceID, int classType, int index )
	{
		Utils.Log1 ( className, "Plugin_CreateInstance for " + deviceID + " of type: [" + classType + "] with index [" + index + "]" );

		int	objectIndex = pluginObjectsIndexer++;

		switch(classType) {
			case InterfaceType.Encoder:
				Encoder encoder = new Encoder();
				pluginObjects.put(objectIndex, encoder);
				return objectIndex;

			default:
				break;
		}
		return -1;
	}


	/**
	 * Plugin_Encoder_Init static method to be called by native layer in order to initialize an instance of a java plugin class.
	 *
	 * @param deviceID		The deviceID that the class instance is designated for.
	 * @param objectIndex 	An environs determined id that identifies this object during its lifecycle.
	 * @param props 		Properties for the media.
	 * @param width 		Width of the media.
	 * @param height 		Height of the media.
	 * @param frameRate 	Framerate of the media.
	 */
	private static int Plugin_Encoder_Init ( int hInst, int deviceID, int objectIndex, int props, int width, int height, int frameRate )
	{
		Utils.Log1 ( className, "Plugin_Encoder_Init for " + deviceID + " of objectIndex: [" + objectIndex + "] with props [" + props + "] with usePNG [" + width + "]"
		+ " with height [" + height + "] with frameRate [" + frameRate + "]" );

		Encoder encoder = (Encoder) pluginObjects.get(objectIndex);
		if (encoder != null) {
			encoder.width = width;
			encoder.height = height;
			encoder.frameRate = frameRate;
			encoder.codecType = props;
			return (encoder.init() ? 1 : 0);
		}

		return 0;
	}


	/**
	 * Plugin_DestroyInstance static method to be called by native layer in order to destroy an instance of a java plugin class.
	 *
	 * @param objectIndex 	An environs determined id that identifies this object during its lifecycle.
	 * @return objectIndex	The objectIndex means success. (-1 means error).
	 */
	private static int Plugin_DestroyInstance ( int hInst, int objectIndex )
	{
		Utils.Log1 ( className, "Plugin_DestroyInstance objectIndex: [" + objectIndex + "]" );

		pluginObjects.remove ( objectIndex );

		return objectIndex;
	}


    /**
     * Register at known Mediator server instances.
     * 
     * @return success state 
     */
	public boolean RegisterAtMediators () {
		return RegisterAtMediatorsN ( hEnvirons );
	}
	static native boolean RegisterAtMediatorsN ( int hInst );


	/**
	 * Reset crypt layer and all created resources. Those will be recreated if necessary.
	 * This method is intended to be called directly after creation of an Environs instance.
	 *
	 */
	public void ResetCryptLayer() {
		ResetCryptLayerN ();
	}

	static native void ResetCryptLayerN();


	private static ByteBuffer SHAHashCreate ( byte[] msg )
    {
		return Utils.SHAHashCreate ( msg );
	}

	private static ByteBuffer GenerateCertificate()
	{
		return Utils.GenerateCertificate ( );
	}
	
	private static ByteBuffer EncryptMessage ( int deviceID, int certProp, byte[] cert, byte[] msg )
    {
		return Utils.EncryptMessage ( deviceID, certProp, cert, msg );
    }
	
	private static ByteBuffer DecryptMessage ( byte[] cert, byte[] msg )
    {
		return Utils.DecryptMessage ( cert, msg );
    }
	
	private static void ReleaseCert ( int deviceID )
    {    	
        Utils.ReleaseCert ( deviceID );
    }
	
	private static ByteBuffer AESDeriveKeyContext ( int deviceID, byte[] key, int keyLen, int aesSize )
	{
		return Utils.AESDeriveKeyContext ( deviceID, key, keyLen, aesSize );
	}

	private static void AESDisposeKeyContext ( int deviceID ) {
		Utils.AESDisposeKeyContext ( deviceID );
	}

	private static void AESUpdateKeyContext ( int deviceIDTemp, int deviceID )
	{
		Utils.AESUpdateKeyContext ( deviceIDTemp, deviceID );
	}
	
	private static ByteBuffer AESTransform ( int deviceID, boolean encrypt, byte[] msg, byte [] keyIV )
	{
		return Utils.AESTransform ( deviceID, encrypt, msg, keyIV );
	}
	
    /**
     * malloc static method to be called by native layer in order to allocate a ByteBuffer of size.
     * 
     * @param size      The size in bytes.
     * @param	manage	if true then keep a reference within the java VM until a call to Java_free()
     * @return buffer 
     */
	@Nullable
    static ByteBuffer Java_malloc(int size, boolean manage)
    {
		if (Utils.isDebug) Utils.Log ( 5, className, "Java_malloc from jni: " + size);
    	
    	if ( size <= 0 )
    		return null;

		ByteBuffer buffer;

		try
		{
			buffer = ByteBuffer.allocateDirect(size);

		}
		catch ( Exception ex ) {
			Utils.LogE ( className, "Java_malloc FAILED size of " + size);
			return null;
		}

    	if ( manage )
    		jniMemory.add(buffer);
		
        ByteOrder nativeOrder = ByteOrder.nativeOrder();
		if ( buffer.order() != nativeOrder )
			buffer.order ( nativeOrder );

    	return buffer;
    }   

    /**
     * Java_free static method to be called by native layer in order to release a ByteBuffer.
     * 
     * @param buffer    The ByteBuffer object to be released.
     * @return success 
     */
    private static boolean Java_free(ByteBuffer buffer)
    {
    	//Utils.Log(className, "Java_free from jni");
    	
    	if ( buffer == null )
    		return false;
    	//int capacity = buffer.capacity();
    	
    	//Utils.Log(className, "Java_free from jni of size " + capacity);
    	
    	return jniMemory.remove ( buffer );
    }

    
	/**
	 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
	 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
	 * 		this call executes the Start tasks within a thread.&nbsp;
	 * 		In order to get the status, catch the onNotify handler of your EnvironsObserver.
	 * 
	 */
	public void Start() {
		Utils.Log1 ( className, "Start" );

		if (observers.size() <= 0) {
			Utils.LogW ( className, "Start: No listener given! The application will not be able to interact with the environment!" );
		}

		StartStoper thread = new StartStoper ( );
		thread.env = this;
		thread.hEnvirons = hEnvirons;

		if ( async == Call.NoWait ) {
			// Create a thread that connects to the surface
			thread.start ( );
			return;
		}

		instances[hEnvirons] = this;
		thread.run ();
	}


	private void StopSync()
	{
		Environs.StopSensorListeningAllN ( hEnvirons );

		wifiObserver.Stop ();

		btObserver.Stop ();

		StopNetLayerN ( hEnvirons );

		synchronized (portalReceivers)
		{
			for ( PortalReceiver thread : portalReceivers.values () )
			{
				Utils.Log1 ( className, "Stop: Interrupting stream thread for device " + thread.nativeID);

				thread.interrupt();

				if (thread.decoder != null) {
					thread.decoder.release();
					thread.decoder = null;
				}
			}

			portalReceivers.clear ( );
		}


		synchronized (portalGenerators)
		{
			for ( PortalGenerator generator : portalGenerators.values () )
			{
				if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Releasing portal generator for device " + generator.destID);

				generator.release();
			}

			portalGenerators.clear();
		}

		DisposeLists ( );

		Thread thread = listNotifierThread;
		if (thread != null) {
			listNotifierThreadRun = false;

			synchronized ( listNotifierQueue ) {
				listNotifierQueue.notify ();
			}

			try {
				if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device list notifier thread ...");

				thread.join ();
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
			}

			listNotifierThread = null;
			if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device list notifier thread done.");
		}

		thread = listUpdateThread;
		if (thread != null) {
			listUpdateThreadRun = false;

			synchronized ( listUpdateQueue ) {
				listUpdateQueue.notify ();
			}

			try {
				if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device list update thread ...");

				thread.join ();
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
			}

			listUpdateThread = null;
			if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device list update thread done.");
		}

		thread = deviceInstanceNotifier;
		if (thread != null) {
			deviceInstanceNotifierRun = false;

			synchronized ( deviceInstanceNotifierQueue ) {
				deviceInstanceNotifierQueue.notify ();
			}

			try {
				if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device instance notifier thread ...");

				thread.join ();
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
			}

			deviceInstanceNotifier = null;
			if (Utils.isDebug) Utils.Log ( 4, className, "Stop: Waiting for device instance notifier thread done.");
		}

		DisposeListNotifier ( );

		DisposeListCommandContext ( contextAll );
		DisposeListCommandContext ( contextMediator );
		DisposeListCommandContext ( contextNearby );

		DisposeDeviceInstanceNotifier ();

		// Make sure that the platform layer has shut down before shutting down native layer
		StopN ( hEnvirons );

		if ( touchesCache != null ) {
			Java_free(touchesCache);
			touchesCache = null;
		}
	}


	void DisposeDeviceInstanceNotifier ()
	{
		synchronized ( deviceInstanceNotifierQueue )
		{
			while ( !deviceInstanceNotifierQueue.isEmpty () )
			{
				DeviceInstance.NotifierContext ctx = (DeviceInstance.NotifierContext) deviceInstanceNotifierQueue.remove();
				if (ctx != null)
				{
					if (ctx.device != null) {
						ctx.device.Dispose ();
						ctx.device.NotifyObservers ( Environs.ENVIRONS_OBJECT_DISPOSED, false );
					}
				}
			}
		}
	}


	void DisposeListNotifier ()
	{
		synchronized ( listNotifierQueue )
		{
			while ( !listNotifierQueue.isEmpty () )
			{
				DeviceList.NotifierContext ctx = (DeviceList.NotifierContext) listNotifierQueue.remove();
				if (ctx != null)
				{
					if (ctx.device != null) {
						ctx.device.Dispose ();
						ctx.device.NotifyObservers ( Environs.ENVIRONS_OBJECT_DISPOSED, false );
					}
				}
			}
		}
	}


	void DisposeListContainer ( ArrayList<DeviceInstance> list )
	{
		if (list != null) {
			for (int i=0; i<list.size(); i++)
			{
				DeviceInstance device = list.get(i);

				if (device == null)
					continue;

				device.Dispose ();
				device.NotifyObservers ( Environs.ENVIRONS_OBJECT_DISPOSED, false );
			}
		}
	}


	void DisposeListCommandContext ( ListContext ctx )
	{
		if (ctx.vanished != null && ctx.vanished.size () > 0)
			DisposeListContainer ( ctx.vanished );

		if (ctx.appeared != null && ctx.appeared.size () > 0)
			DisposeListContainer ( ctx.appeared );
	}


	/**
	 * Stop Environs and release all acquired resources.
	 *
	 * @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 **/
	public void Stop ( @Call.Value int async )
	{
		started = false;

		if ( async == Call.NoWait ) {
			new Thread(new Runnable() {
				public void run(){
					StopSync();
				}
			}).start();
		}
		else
			StopSync();
	}
	
    /**
     * Stop Environs asynchronously.
     */
    public void Stop () {
		Stop ( async );
    }

	/**
	 * Dispose Environs and release all acquired resources asynchronously.
	 */
	public void Dispose ( ) {
		if (started)
			Stop ( Call.Wait );

		DisposeN ( hEnvirons );

		synchronized ( instances ) {
			/// This should no be necessary. JVM should call us only if the reference was cleared before
			instances[hEnvirons] = null;
		}
	}
    
    
    /**
     * This method is called by GCM when a push notification has arrived and is targeted at our client application.
     * 
     * @param msg   A message that shall be pushed.
     */
    private void OnPushMessage(String msg) {
    	Utils.Log1 ( className, "OnPushMessage: " + msg );

        // ehlo - Wakeup the device and Start the application
        /*if (msg.equals("ehlo")) {
            // Check whether we have started the network layer
        	int status = GetStatus();
            if (status < Types.STATUS_STOPPED) {

            	// Create a thread that connects to the surface
            	//if (Environs.instance.clientActivity != null)
            	//	StartN(hEnvirons);
            }
        }*/
    }
    

    /**
     * Receiving push messages
     * 
    private final BroadcastReceiver mHandleMessageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String newMessage = intent.getExtras().GetString(C.EXTRA_MESSAGE);
            // Waking up mobile if it is sleeping
            WakeLocker.acquire(getApplicationContext());

 
            // Showing received message
            //lblMessage.append(newMessage + "\n");
            //Toast.makeText(getApplicationContext(), "New Message: " + newMessage, Toast.LENGTH_LONG).show();
 
            // Releasing wake lock
            WakeLocker.release();
        }
    };*/

    /**
     * OnPause must be called by the activities, that needs the sensor data participating in the environment.
     * This call pauses sensor data to be sent, when no activity of the application is actually using it. 
     */
    public void OnPause() {
    	//Utils.Log ( className, "OnPause");
    }

    
    /**
     * OnResume must be called by the activities, that needs the sensor data participating in the environment.
     * This call resumes sending of sensor data to the environment.
	 *
	 * @param    activity    The calling activity.
	 */
	public void OnResume(Activity activity)
    {    
    	//Utils.Log( className, "OnResume w. context");

        // Request always on display
		activity.getWindow().setFlags(
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		currentActivity = activity;
    }


	static native void PushSensorDataN ( int ENVIRONS_SENSOR_TYPE_, float x, float y, float z );


	static native void PushSensorDataDoublesN ( int ENVIRONS_SENSOR_TYPE_, double x, double y, double z );


	static native void PushSensorDataExtN ( int ENVIRONS_SENSOR_TYPE_, double x, double y, double z, float m, float n, float o );


	/**
	 * Set use of Tcp transport channel of the given sensorType.
	 *
	 * @param sensorType    A value of type environs::SensorType_t.
	 * @param enable        true = TCP, false = UDP.
	 *
	 */
	public void SetUseSensorChannelTcp ( @SensorType.Value int sensorType, boolean enable ) {
		SetUseSensorChannelTcpN ( hEnvirons, sensorType, enable );
	}
	static native void SetUseSensorChannelTcpN ( int hInst, @SensorType.Value int sensorType, boolean enable );


	/**
	 * Get use of Tcp transport channel of the given sensorType.
	 *
	 * @param sensorType    A value of type environs::SensorType_t.
	 * @return success      1 = TCP, 0 = UDP, -1 = error.
	 *
	 */
	public int GetUseSensorChannelTcp ( @SensorType.Value int sensorType ) {
		return GetUseSensorChannelTcpN ( hEnvirons, sensorType );
	}
	static native int GetUseSensorChannelTcpN ( @SensorType.Value int hInst, int sensorType );


	/**
	 * Set sample rate of the given sensorType in microseconds.
	 *
	 * @param sensorType        A value of type environs::SensorType_t.
	 * @param microseconds      The sensor sample rate in microseconds.
	 *
	 */
	public void SetUseSensorRate ( @SensorType.Value int sensorType, int microseconds ) {
		SetUseSensorRateN ( hEnvirons, sensorType, microseconds );
	}
	static native void SetUseSensorRateN ( int hInst, @SensorType.Value int sensorType, int microseconds );


	/**
	 * Get sample rate of the given sensorType in microseconds.
	 *
	 * @param sensorType        A value of type environs::SensorType_t.
	 *
	 * @return microseconds     The sensor sample rate in microseconds. -1 means error.
	 */
	public int GetUseSensorRate ( @SensorType.Value int sensorType ) {
		return GetUseSensorRateN ( hEnvirons, sensorType );
	}
	static native int GetUseSensorRateN ( @SensorType.Value int hInst, int sensorType );


	/**
	 * Objects for handling sensor services
	 */
	Sensors sensors = null;


	public Location GetLocation ()
	{
		if (sensors == null)
			return null;
		return new Location ( sensors.GetLocation () );
	}



	/**
	 * Enable sending of sensor events to this DeviceInstance.
	 * Events are send if the device is connected and stopped if the device is disconnected.
	 *
	 * @param nativeID 				Destination native id
	 * @param objID 				Destination object id
	 * @param sensorType 			A value of type SensorType.Value.
	 * @param enable 				true = enable, false = disable.
	 *
	 * @return success true = enabled, false = failed.
	 */
	static native boolean SetSensorEventSenderN(int hInst, int nativeID, int objID, @SensorType.Value int sensorType, boolean enable);


	/**
	 * Register to sensor events and listen to sensor data events.
	 * This implementation is platform specific and needs to be implemented
	 * in the particular platform layer.
	 *
	 * @param sensorType A value of type environs::SensorType.
	 *
	 * @return success 1 = enabled, 0 = failed.
	 */
	static native int StartSensorListeningN ( int hInst, @SensorType.Value int sensorType );


	/**
	 * Deregister to sensor events and stop listen to sensor data events.
	 * This implementation is platform specific and needs to be implemented
	 * in the particular platform layer.
	 *
	 * @param sensorType A value of type environs::SensorType.
	 *
	 * @return success 1 = enabled, 0 = failed.
	 */
	static native int StopSensorListeningN ( int hInst, @SensorType.Value int sensorType );


	/**
	 * Stop all sensors that the given Environs instance have subscribed to.
	 *
	 * @param hInst 				The Environs instance identifier.
	 *
	 */
	static native void StopSensorListeningAllN ( int hInst );

	/**
	 * Start all sensors that the given Environs instance have subscribed to.
	 *
	 * @param hInst 				The Environs instance identifier.
	 *
	 */
	static native void StartSensorListeningAllN ( int hInst );


	/**
	 * Get registered DeviceInstance objects for sending of sensor events.
	 *
	 * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
	 *
	 * @return success true = enabled, false = failed.
	 */
	static native int GetSensorEventSenderCountN(int hInst, int ENVIRONS_SENSOR_TYPE_);

	/**
	 * Determine whether the given sensorType is available.
	 *
	 * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
	 *
	 * @return success true = enabled, false = failed.
	 */
	private static native int IsSensorAvailableN(int hInst, int ENVIRONS_SENSOR_TYPE_);

	/**
	 * Determine whether the given sensorType is enabled.
	 *
	 * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
	 *
	 * @return success 1 = enabled, 0 = failed, -1 = error.
	 */
	static native int IsSensorEnabledN(int hInst, int ENVIRONS_SENSOR_TYPE_);


	/**
	 * Enable dispatching of sensor events from ourself.
	 * Events are send if Environs instance is started stopped if the Environs instance has stopped.
	 *
	 * @param sensorType            A value of type environs.SensorType.
	 * @param enable 				true = enable, false = disable.
	 *
	 * @return success true = enabled, false = failed.
	 */
	public boolean SetSensorEvent ( @SensorType.Value int sensorType, boolean enable )
	{
		Utils.Log2 ( className, "SetSensorEvent:\t\t\tType [ " + sensorType + " ]  enable [ " + enable + " ]" );

		if ( sensorType == SensorType.Location )
		{
			if (sensors == null || sensors.locationManager == null) {
				return false;
			}
		}

		boolean success = SetSensorEventSenderN ( hEnvirons, 0, 0, sensorType, enable );

		if ( enable ) {
			StartSensorListeningN ( hEnvirons, sensorType );

			if ( sensorType == SensorType.Location ) {
				int enb = Environs.IsSensorEnabledN ( hEnvirons, SensorType.Location );
				if ( enb == 1 )
					sensors.StartLocation ( );
			}
		}
		else {
			StopSensorListeningN ( hEnvirons, sensorType );

			if ( sensorType == SensorType.Location ) {
				int enb = Environs.IsSensorEnabledN ( hEnvirons, SensorType.Location );
				if ( enb == 0 )
					sensors.StopLocation ( );
			}
		}

		return success;
	}


	/**
	 * Determine whether the given sensorType is available.
	 *
	 * @param sensorType A value of type environs::SensorType_t.
	 *
	 * @return success true = available, false = not available.
	 */
	public boolean IsSensorAvailable ( @SensorType.Value int sensorType )
	{
		if ( sensorType == SensorType.Location )
		{
			if (sensors.locationManager == null)
				return false;
		}

		return ( IsSensorAvailableN ( hEnvirons, sensorType ) != 0);
	}


	/**
	 * Enable sending of sensor events to this DeviceInstance.
	 * Events are send if the device is connected and stopped if the device is disconnected.
	 *
	 * @param hInst                 The Environs instance identifier.
	 * @param nativeID 				Destination native device id
	 * @param objID 				Destination object device id
	 * @param flags            		A bitfield with values of type SensorType
	 * @param enable 				true = enable, false = disable.
	 *
	 * @return success 1 = enabled, 0 = failed.
	 */
	static native int SetSensorEventSenderFlagsN ( int hInst, int nativeID, int objID, int flags, int enable );


    /**
	 * OnDestroy must be called by the activities, that needs the sensor data participating in the environment.
     */
    public void OnDestroy() {
    	//Utils.Log( className, "OnDestroy");
        
        if (mRegisterTask != null) {
        	mRegisterTask.cancel(true);
        }
        
        // Dismiss any progress dialogs if there is one active
        Utils.dismissProgress(GetClient());
        
        /*
        try {
            unregisterReceiver(mHandleMessageReceiver);
            GCMRegistrar.OnDestroy(this);
        } catch (Exception e) {
            Utils.Log("UnRegister Receiver Error> " + e.getMessage());
        }
        */
    }
	
    
    /*
     * Environs native layer section
     */
    // Prepare the native layer with default values
 	//private static native boolean allocNative ();
	private static native boolean IsNativeAllocatedN ();

 	private static native void RegisterMainThreadN ( int hInst );
 	
 	private static native void InitStorageN ( String path );
 	private static native void InitWorkDirN ( String path );

 	private static native void SetNetworkStatusN ( int netStat );
 	
 	private static boolean InitNative(Context client)
 	{
		Utils.Log1 ( className, "InitNative" );

 		if (client != null) {
 			File file = client.getFilesDir ();
 			if (file != null) {
 				String absPath = file.getAbsolutePath();
 				InitStorageN ( absPath );

 				int p = absPath.lastIndexOf("/files");
 				if ( p != -1 ) {
 					absPath = absPath.substring(0, p) +  "/";
 				}
 				InitWorkDirN ( absPath );
 				Utils.Log1 ( className, "Storage path: " + absPath );
 			}
 		}

		if ( !IsNativeAllocatedN ( ) )
			return false;
 		
 		SetOSLevelN(Utils.APILevel);

		return true;
 	}

 	//private static native void deallocNative ();
 	
 	// Set Status to 1
 	static native int InitN ( int hInst );
 	
 	static native int StartN ( int hInst );

	/**
	* Query the filter level for device management within Environs.
	*
	* @return level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_PROJECT, Environs.MEDIATOR_FILTER_PROJECT_AND_APP
	*/
	@MediatorFilter.Value
 	public int GetMediatorFilterLevel () {
		//noinspection ResourceType
		return GetMediatorFilterLevelN ( hEnvirons );
	}
	static native int GetMediatorFilterLevelN ( int hInst );
 	

	/**
	* Set the filter level for device management within Environs.
	*
	* @param level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_PROJECT, Environs.MEDIATOR_FILTER_PROJECT_AND_APP
	*/
 	public void SetMediatorFilterLevel ( @MediatorFilter.Value int level ) {
		SetMediatorFilterLevelN ( hEnvirons, level );
	}
	static native void SetMediatorFilterLevelN ( int hInst, int level );


	/**
	 * Get platform that the app is running on.
	 *
	 * @return 	enum of type Environs.Platforms
	 */
	@Platforms.Value
	public int GetPlatform () {
		//noinspection ResourceType
		return GetPlatformN ( );
	}
	static native int GetPlatformN ();

	/**
	 * Set the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
	 * Valid type are enumerated in Environs.Platforms.*
	 *
	 * @param	platform	Environs.Platforms.*
	 */
	public void SetPlatform(@Platforms.Value int platform) {
		SetPlatformN ( platform );
	}

	static native void SetPlatformN(int platform);

	/**
	 * Set/Remove the lastLocation-node flag to the platform type that the local instance of Environs shall use for identification within the environment.&nbsp;
	 * Flag: Environs.Platforms.LocationNode_Flag
	 *
	 * @param	isLocationNode	true or false
	 */
	public void SetIsLocationNode ( boolean isLocationNode ) {
		SetIsLocationNodeN ( isLocationNode );
	}

	static native void SetIsLocationNodeN ( boolean isLocationNode );


	/**
	 * Option whether to allow connections by every device.
	 *
	 * @param enable  true = enabled, false = failed.
	 */
	public void SetConnectAllowFromAll ( boolean enable )
	{
		SetConnectAllowFromAllN ( hEnvirons, enable );
	}

	static native void SetConnectAllowFromAllN ( int hInst, boolean enable );

	/**
	 * Option whether to allow connections by every device.
	 *
	 * @return  true = enabled, false = failed.
	 */
	public boolean GetConnectAllowFromAll ( )
	{
		return (GetConnectAllowFromAllN ( hEnvirons ) != 0);
	}

	static native int GetConnectAllowFromAllN ( int hInst );


	/** Allow connects by this device. The default value of for this property is determined by GetAllowConnectDefault() / SetAllowConnectDefault ().
	 Changes to this property or the allowConnectDefault has only effect on subsequent instructions. */
	static native int AllowConnectN ( int hInst, int objID, int value );

	/** Default value for each DeviceInstance after object creation. */
	static native int AllowConnectDefaultN ( int hInst, int value );


	/** Default value for each DeviceInstance after object creation. */
	public boolean GetAllowConnectDefault ()
	{
		return allowConnectDefault;
	}

	/** Default value for each DeviceInstance after object creation. */
	public void SetAllowConnectDefault ( boolean value )
	{
		allowConnectDefault = value;

		Environs.AllowConnectDefaultN ( hEnvirons, value ? 1 : 0 );
	}

	/**
	* Connect to device with the given id.
	*
	* @param deviceID	Destination device id
	* @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
	* @return status	0: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
	* 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
	* 					2: A new connection has been triggered and is in progress
	*/
 	public int DeviceConnect(int deviceID, @Call.Value int async)
 	{
 		return DeviceConnectN ( hEnvirons, deviceID, null, null, async );
 	}
	
	/**
	* Connect to device with the given id and a particular application environment.
	*
	* @param deviceID	Destination device id
	* @param areaName	Area name of the application environment
	* @param appName	Application name of the application environment
	* @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
	* @return status	0: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
	* 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
	* 					2: A new connection has been triggered and is in progress
	*/
	public int DeviceConnect(int deviceID, String areaName, String appName, @Call.Value int async) {
		return DeviceConnectN ( hEnvirons, deviceID, areaName, appName, async );
	}

	static native int DeviceConnectN(int hInst, int deviceID, String areaName, String appName, @Call.Value int async);


	/**
	 * Disconnect the device with the given id and a particular application environment.
	 *
	 * @param 	nativeID	The native device id that targets the device.
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
	 */
	public boolean DeviceDisconnect ( int nativeID, @Call.Value int async )
	{
		return DeviceDisconnectN ( hEnvirons, nativeID, async );
	}


 	/** 
 	 * Native call to disconnect the device with the given id and a particular application environment.
 	 * 
 	 * @param 	nativeID	Destination native device id
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
 	 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected. 
     */ 	
 	static native boolean DeviceDisconnectN(int hInst, int nativeID, @Call.Value int async);


	/**
	 * Get status whether the device with the given id is connected to the local instance of Environs.&nbsp;
     * The return value is of enumeration type Types.DeviceStatus.*&nbsp;
     * That is e.g. Types.DeviceStatus.ConnectInProgress
     * 
 	 * @param nativeID	Destination native device id
 	 * @return connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
     */
 	public int GetDeviceConnected ( int nativeID ) {
 		return GetDeviceConnectStatusN ( hEnvirons, nativeID );
 	}

    /** 
     * Get status whether the device with the given id is connected to the local instance of Environs.&nbsp;
     * The return value is of enumeration type Types.DeviceStatus.*&nbsp;
     * That is e.g. Types.DeviceStatus.ConnectInProgress
     * 
 	 * @param nativeID	Destination native device id
 	 * @return connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
     */
	static native int GetDeviceConnectStatusN(int hInst, int nativeID);

    /** 
     * Query the number of devices that are connected to the local instance of Environs.
     * 
 	 * @return numberOfConnectedDevices
     */
 	public int GetConnectedDevicesCount() {
		return GetConnectedDevicesCountN ( hEnvirons );
	}
	static native int GetConnectedDevicesCountN(int hInst);


	/**
	 * Dispose a native Environs object with the given object handle.
	 *
	 * @param hInst			A handle to the Environs instance
	 */
 	private static native void DisposeN(int hInst);

 	private static native int StopN(int hInst);

	private static native void StopNetLayerN(int hInst);

    /**
     * Query the status of Environs. Valid values are Types.NATIVE_STATUS_*
     * 
     * @return Environs.NATIVE_STATUS_* 
     */
	@Status.Value
 	public int GetStatus() {
		if (!nativeLayerInstalled)
			return Types.ENVIRONS_OBJECT_DISPOSED;

		//noinspection ResourceType
		return GetStatusN ( hEnvirons );
	}

	static native int GetStatusN(int hInst);

    /**
     * Set the ports that the local instance of Environs shall use for listening on connections.
     * 
     * @param	tcpPort The tcp port.
     * @param	udpPort The udp port.
     * @return success
     */
 	public boolean SetPorts(int tcpPort, int udpPort) {
 		return SetPortsN ( hEnvirons, tcpPort, udpPort );
 	} 	
 	static native boolean SetPortsN(int hInst, int tcpPort, int udpPort);


	/**
	 * Set the base port that the local instance of Environs shall use for communication with other instances.
	 * This option enables spanning of multiple multi surface environsments separated by the network stacks.
	 *
	 * @param	port The base port.
	 * @return success
	 */
	public boolean SetBasePort ( int port ) {
		return SetBasePortN ( hEnvirons, port );
	}
	static native boolean SetBasePortN ( int hInst, int port );


 	static native void SetOSLevelN(int level);


    /**
     * Set the debug mode of Environs to influence the amount of debug log messages.
     * 
     * @param	level     0 = debug off (only error log); 1 ... 16 = debug verbose level only available in debug builds.
	 *                    (dont use levels &gt; 12 for long durations. it will produce huge log files)
     */
	public static void SetDebug ( int level ) {
		Utils.logLevel = level;

		if (IsInstalled ())
			SetDebugN ( level );
	}

	public static native void SetDebugN(int level);


 	private static native void SetDeviceDimsN(int width, int height, int width_mm, int height_mm, int leftpos, int toppos);
 	

 	String areaName = null;
    /**
     * Set the area name that the local instance of Environs shall use for identification within the environment.
     * It must be set before creating the Environs instance.
     * 
     * @param	name    The name of the Environs area.
     * @return	success
     */
 	public boolean SetAreaName(String name) {
		if (name == null || name.length() < 1)
			return false;
 		areaName = name;
 		return SetAreaNameN ( hEnvirons, name );
 	}
 	static native boolean SetAreaNameN(int hInst, String name);

    /**
     * Get the area name that the local instance of Environs use for identification within the environment.
     * It must be set before creating the Environs instance.
     * 
     * @return	areaName
     */
 	public String GetAreaName() {
		if (areaName == null || areaName.length() < 1)
			return Types.ENVIRONS_DEFAULT_APP_NAME;
 		return areaName;
 	}

 	String appName = null;
    /**
     * Set the application name of that the local instance of Environs shall use for identification within the environment.
     * It must be set before creating the Environs instance.
     * 
     * @param	name    The name of the Environs app.
     * @return	success
     */
 	public boolean SetApplicationName(String name) {
		if (name == null || name.length() < 1)
			return false;
 		appName = name;
 		return SetApplicationNameN ( hEnvirons, name );
 	}
 	static native boolean SetApplicationNameN(int hInst, String name);

    /**
     * Get the application name that the local instance of Environs use for identification within the environment.
     * It must be set before creating the Environs instance.
     * 
     * @return	appName
     */
 	public String GetApplicationName() {
		if (appName == null || appName.length() < 1)
			return Types.ENVIRONS_DEFAULT_APP_NAME;
 		return appName;
 	}
 	
	
    /**
     * Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
     * Valid type are enumerated in Environs.DEVICE_TYPE_*
     * 
     * @param	type	Environs.DEVICE_TYPE_*
     */
 	public static native void SetDeviceTypeN(char type);

 	
    /**
     * Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
     * Valid type are enumerated in Types.DEVICE_TYPE_*
     * 
     * @return	type	Environs.DEVICE_TYPE_*
     */
 	//public static native char GetDeviceFlagseN();


	/**
	 * Update device flags to native layer and populate them to the environment.
	 *
	 * @param	hInst    The handle to the environs instance.
	 * @param 	async    Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param	objID    The identifier for the native device object.
	 * @param	flags    The internal flags to set or clear.
	 * @param	set    	 true = set, false = clear.
	 */
	static native void SetDeviceFlagsN(int hInst, @Call.Value int async, int objID, int flags, boolean set);


	/**
     * Register the view as the portal touch view (and also a TouchDispatch source for that view ) for a given deviceID.&nbsp;
     * All TouchDispatch event that arrives at that view will then be passed through to the sink of an Environs portal.
     * 
     * @param	portal    The PortalInstance object that generates the touch events.
     * @param	view        A view that belongs to the portal.
     */
 	@SuppressLint("NewApi")
	public void SetPortalTouchView(final PortalInstance portal, View view) {
		if (view == null)
			return;

		//Utils.Log( className, "SetPortalTouchView: set portal view for device " + deviceID);

		view.setOnTouchListener ( new View.OnTouchListener ( ) {
			@Override
			public boolean onTouch ( View view, MotionEvent event ) {
				return DispatchTouch ( portal.portalID, event );
			}
		} );

		if (GetPortalViewDimsAuto())
 		{
 	 		if ( Utils.APILevel >= 11 ) {
 	 	 		view.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {			
 	 	 			@Override
 	 	 	        public void onLayoutChange(View view, int left, int top, int right, int bottom, 
 	 	 	        		int oldLeft, int oldTop, int oldRight, int oldBottom)
 	 	 			{ 	 	 				
 	 	 	            if (left == 0 && top == 0 && right == 0 && bottom == 0)
 	 	 	                return; 	 	 	            

 	 	 	            bottom -= (Utils.GetNaviBarHeigth(GetClient()) / 2);

						SetPortalViewDims(portal.portalID, left, top, right, bottom);
 	 	 	        }
 	 			});
 	 		}
 	 		else {
 	 			Utils.LogW ( className, "SetPortalTouchView: API level is less than 11." +
 	 						"We are not able to observe the view's size for portal TouchDispatch. Only fullscreen is supported for TouchDispatch!" );
 	 		} 			
 		}		
 	}

	static void SetPortalTouchView(int hInst, PortalInstance portal, View view) {
		instances[hInst].SetPortalTouchView(portal, view);
	}


	/**
	 * Set the lastLocation (and its size) of the portal that belongs to the nativeID.
	 * Such values are usually provided within the onLayoutChangeListener of a View.
	 *
	 * @param portalID      The portal device id of the target device.
	 * @param left          The left coordinate
	 * @param top           The top coordinate
	 * @param right         The right coordinate
	 * @param bottom        The bottom coordinate
	 *
	 * @return success		This call will fail, if the touchsource (and portal resources) have not been initialized.
	 */
	public boolean SetPortalViewDims(int portalID, int left, int top, int right, int bottom) {
		return SetPortalViewDimsN ( hEnvirons, portalID, left, top, right, bottom );
	}
	static native boolean SetPortalViewDimsN(int hInst, int portalID, int left, int top, int right, int bottom);
 	

 	static native void SetGCMN(String regID);
 	
 	// Options

    /**
     * Query whether to use Crypto Layer Security for Mediator connections.
     * 
     * @return	enabled
     */
 	public boolean GetUseCLSForMediator() {
		return GetUseCLSForMediatorN ( hEnvirons );
	}
	static native boolean GetUseCLSForMediatorN(int hInst);

    /**
     * Option for whether to use Crypto Layer Security for Mediator connections.
     * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseCLSForMediator(boolean enable) {
		SetUseCLSForMediatorN ( hEnvirons, enable );
	}
	static native void SetUseCLSForMediatorN(int hInst, boolean enable);

    /**
     * Query whether to use Crypto Layer Security for device-to-device connections.
     * 
     * @return	enabled
     */
	public boolean GetUseCLSForDevices() {
		return GetUseCLSForDevicesN ( hEnvirons );
	}
	static native boolean GetUseCLSForDevicesN(int hInst);

    /**
     * Determines whether to use Crypto Layer Security for device-to-device connections.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseCLSForDevices(boolean enable) {
		SetUseCLSForDevicesN ( hEnvirons, enable );
	}
	static native void SetUseCLSForDevicesN(int hInst, boolean enable);


    /**
     * Query whether to enforce Crypto Layer Security for device-to-device connections.
     * 
     * @return	enabled
     */
 	public boolean GetUseCLSForDevicesEnforce() {
		return GetUseCLSForDevicesEnforceN ( hEnvirons );
	}
	static native boolean GetUseCLSForDevicesEnforceN(int hInst);

    /**
     * Determines whether to enforce Crypto Layer Security for device-to-device connections.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseCLSForDevicesEnforce(boolean enable) {
		SetUseCLSForDevicesEnforceN ( hEnvirons, enable );
	}
	static native void SetUseCLSForDevicesEnforceN(int hInst, boolean enable);


    /**
     * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
     * 
     * @return	enabled
     */
 	public boolean GetUseCLSForAllTraffic() {
		return GetUseCLSForAllTrafficN ( hEnvirons );
	}
	static native boolean GetUseCLSForAllTrafficN(int hInst);

    /**
     * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseCLSForAllTraffic(boolean enable) {
		SetUseCLSForAllTrafficN ( hEnvirons, enable );
	}
	static native void SetUseCLSForAllTrafficN(int hInst, boolean enable);
 	

    /**
     * Determines whether to use video codec for portal stream encoding/decoding or not.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseStream(boolean enable) {
		SetUseStreamN ( hEnvirons, enable );
	}
	static native void SetUseStreamN(int hInst, boolean enable);

    /**
     * Query whether to use video codec for portal stream encoding/decoding or not.
     * 
     * @return	enabled
     */
 	public boolean GetUseStream() {
		return GetUseStreamN ( hEnvirons );
	}
	static native boolean GetUseStreamN(int hInst);


	static long nativeDecoderLastcheck = 0;

    /**
     * Determines whether to use native C++ for portal stream encoding/decoding or not.
     * 
     * @param	enable  A boolean that determines the target state.
	 * @return  success
     */
 	public boolean SetUseNativeDecoder(boolean enable)
	{
		try
		{
			if ( enable ) {
				final String libName = "libopenh264";

				// Determine whether we have the binaries and license of the encoder installed.
				if ( !Libs.isLibAvailable(libName) )
				{
					final Activity act = Environs.currentActivity;
					if ( act == null ) {
						Utils.LogE ( className, "SetUseNativeDecoder: We need to ask for permission, but no activity is available.");
						return false;
					}

					act.runOnUiThread(new Runnable() {
						@Override
						public void run() {
							AlertDialog.Builder alertDialog = new AlertDialog.Builder(act);

							String modName = libName;
							if (modName.startsWith("lib"))
								modName = modName.substring(3);

							alertDialog.setTitle("Please Confirm");

							String msg = "Download and install 3rd party module " + modName;
							alertDialog.setMessage(msg);

							alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int which)
								{
									LibLicThread thread = new LibLicThread();
									thread.libName = libName;
									thread.libUrl = "https://github.com/cisco/openh264/releases/download/v1.4.0/libopenh264-1.4.0-android19.so.bz2";
									thread.licenseUrl = "http://www.openh264.org/BINARY_LICENSE.txt";

									thread.setActionListener(new LibLicThread.OnActionListener() {
										@Override
										public void onAction() {
											SetUseNativeDecoderN ( hEnvirons, true );

											Utils.Log1 ( className, "SetUseNativeDecoder: successfully installed " + libName );

											BridgeForNotify(hEnvirons, -1, Notify.Environs.SettingsChanged, Source.Platform, null, 0);
										}
									});
									thread.start();
									dialog.dismiss();
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
				else {
					if ( nativeDecoderLastcheck == 0 )
						nativeDecoderLastcheck = System.currentTimeMillis();
					else {
						long millis = System.currentTimeMillis();
						if ( millis - nativeDecoderLastcheck < 500 ) {
							Libs.delete3rd(libName);
						}
						else
							nativeDecoderLastcheck = millis;
					}
				}
			}

			SetUseNativeDecoderN ( hEnvirons, enable );
		}
		catch (Exception ex) {
			Utils.LogE ( className, "LibLicThread: " + ex.getMessage() );

			ex.printStackTrace();
		}
		return true;
	}


	/**
	 * Determines whether to use native C++ for portal stream encoding/decoding or not.
	 *
	 * @param	enable  A boolean that determines the target state.
	 */
	static native void SetUseNativeDecoderN(int hInst, boolean enable);


    /**
     * Query whether to use native C++ for portal stream encoding/decoding or not.
     * 
     * @return	enabled
     */
 	public boolean GetUseNativeDecoder() {
		return GetUseNativeDecoderN ( hEnvirons );
	}

	/**
	 * Query whether to use native C++ for portal stream encoding/decoding or not.
	 * Native call.
	 *
	 * @return	enabled
	 */
	static native boolean GetUseNativeDecoderN(int hInst);

    /**
     * Determines whether to use native C++ for portal stream encoding/decoding or not.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseHardwareEncoder(boolean enable)
 	{ 		
 		if ( enable ) {
 			SetUseHardwareEncoderN ( hEnvirons, true );
 			return;
 		}

		SetUseHardwareEncoderN ( hEnvirons, false );
 	}
 	
 	static native void SetUseHardwareEncoderN(int hInst, boolean enable);

    /**
     * Query whether to use native C++ for portal stream encoding/decoding or not.
     * 
     * @return	enabled
     */
 	public boolean GetUseHardwareEncoder() {
		return GetUseHardwareEncoderN ( hEnvirons );
	}
	static native boolean GetUseHardwareEncoderN(int hInst);


	/**
	 * Use default encoder, decoder, capture, render modules.
	 *
	 * @return  success
	 */
	public boolean SetUsePortalDefaultModules() {
		return SetUsePortalDefaultModulesN ( hEnvirons );
	}
	static native boolean SetUsePortalDefaultModulesN(int hInst);
 	

    /**
	 * Use encoder module with the name moduleName. (libEnv-Enc...).
     * 
     * @param	moduleName	the name of the module
     * @return  success
     */
 	public boolean SetUseEncoder(String moduleName) {
		return SetUseEncoderN ( hEnvirons, moduleName );
	}
	static native boolean SetUseEncoderN(int hInst, String moduleName);
 	
    /**
	 * Use decoder module with the name moduleName. (libEnv-Dec...).
     * 
     * @param	moduleName	the name of the module
     * @return  success
     */
	public boolean SetUseDecoder(String moduleName) {
		return SetUseDecoderN ( hEnvirons, moduleName );
	}
	static native boolean SetUseDecoderN(int hInst, String moduleName);
 	
    /**
	 * Use render module with the name moduleName. (libEnv-Rend...).
     * 
     * @param	moduleName	the name of the module
     * @return  success
     */
	public boolean SetUseRenderer(String moduleName) {
		return SetUseRendererN ( hEnvirons, moduleName );
	}
	static native boolean SetUseRendererN(int hInst, String moduleName);
 	
    /**
     * Use capture module with the name moduleName. (libEnv-Cap...).
     * 
     * @param	moduleName	the name of the module
     * @return  success
     */
	public boolean SetUseCapturer(String moduleName) {
		return SetUseCapturerN ( hEnvirons, moduleName );
	}
	static native boolean SetUseCapturerN(int hInst, String moduleName);

 	
    /**
     * Determines whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUseSensors(boolean enable) {
		SetUseSensorsN ( hEnvirons, enable );
	}
	static native void SetUseSensorsN(int hInst, boolean enable);

    /**
     * Query whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
     * 
     * @return	enabled
     */
 	public boolean GetUseSensors() {
		return GetUseSensorsN ( hEnvirons );
	}
	static native boolean GetUseSensorsN(int hInst);

 	
    /**
     * Determines whether to support push notifications via GCM or not.
     * 
     * @param	enable  A boolean that determines the target state.
     */
 	public void SetUsePushNotifications ( boolean enable ) {
		SetUsePushNotificationsN ( hEnvirons, enable );
	}
	static native void SetUsePushNotificationsN ( int hInst, boolean enable );

    /**
     * Query whether to support push notifications via GCM or not.
     * 
     * @return	enabled
     */
 	public boolean GetUsePushNotifications () {
		return GetUsePushNotificationsN ( hEnvirons );
	}
	static native boolean GetUsePushNotificationsN ( int hInst );
 	

    /**
     * Query whether to use environs default Mediator predefined by framework developers or not.
     * 
     * @return enabled
     */
	public boolean GetUseDefaultMediator() {
		return GetUseDefaultMediatorN ( hEnvirons );
	}
	static native boolean GetUseDefaultMediatorN(int hInst);


    /** 
     * Determines whether to use environs default Mediator predefined by framework developers or not.
     * 
     * @param enable 	true = use the default Mediator
     */
	public void SetUseDefaultMediator(boolean enable) {
		SetUseDefaultMediatorN ( hEnvirons, enable );
	}
	static native void SetUseDefaultMediatorN(int hInst, boolean enable);
 	
 	
 	/** 
 	 * Query whether to use the Mediator previously supplied by SetMediator()
 	 * 
     * @return enabled
     */
	public boolean GetUseCustomMediator() {
		return GetUseCustomMediatorN ( hEnvirons );
	}
	static native boolean GetUseCustomMediatorN(int hInst);

 	
    /** 
     * Determines whether to use the Mediator previously supplied by SetMediator()
     * 
     * @param enable 	true = enable, false = disable
     */
	public void SetUseCustomMediator(boolean enable) {
		SetUseCustomMediatorN ( hEnvirons, enable );
	}
	static native void SetUseCustomMediatorN(int hInst, boolean enable);
 	 	

    /** 
     * Set custom Mediator to use.
     * 
     * @param mediatorIP    The IP address (as string) of the Mediator.
     * @param port          The listening port of the Mediator.
     * @return success
     */
 	public boolean SetMediator(String mediatorIP, int port) {
		return SetMediatorN ( hEnvirons, mediatorIP, port );
	}
	static native boolean SetMediatorN(int hInst, String mediatorIP, int port);
 	

 	/**
     * Query port of custom Mediator.
     * 
     * @return Port
     */
 	public int GetMediatorPort() {
		return GetMediatorPortN ( hEnvirons );
	}
	static native int GetMediatorPortN(int hInst);
 	

 	/**
     * Query ip of custom Mediator.
     * 
     * @return ip
     */
	public String GetMediatorIP() {
		return GetMediatorIPN ( hEnvirons );
	}
	static native String GetMediatorIPN(int hInst);


	static native void WiFiUpdateWithColonMacN ( String bssid, String ssid, int rssi, int channel, int encrypt, int updateState );

	/**
	 * Option for whether to observe wifi networks to help location based services.
	 *
	 * @param	enable  A boolean that determines the target state.
	 */
	public void SetUseWifiObserver(boolean enable) {
		SetUseWifiObserverN ( enable );
	}
	static native void SetUseWifiObserverN(boolean enable);


	/**
	 * Query option for whether to observe wifi networks to help location based services.
	 *
	 * @return enabled.
	 */
	public boolean GetUseWifiObserver() {
		return GetUseWifiObserverN ( );
	}
	static native boolean GetUseWifiObserverN();

	/**
	 * Determines the interval for scanning of wifi networks.
	 *
	 * @param	interval  A millisecond value for scan intervals.
	 */
	public void SetUseWifiInterval(int interval) {
		SetUseWifiIntervalN ( interval );
	}
	static native void SetUseWifiIntervalN(int interval);

	/**
	 * Query interval for scanning of wifi networks.
	 *
	 * @return interval in milliseconds.
	 */
	public int GetUseWifiInterval() {
		return GetUseWifiIntervalN ( );
	}
	static native int GetUseWifiIntervalN();


	public WifiItem[] GetWifis() {
		return BuildWifiItemList(GetWifisN ());
	}
	static native ByteBuffer GetWifisN();


	@SuppressWarnings ( "all" )
	private static WifiItem[] BuildWifiItemList(ByteBuffer buffer)
	{
		if ( buffer == null )
			return null;
        final int wifiItemSize = 16;

		int itemCount = buffer.getInt(0);
		if ( itemCount <= 0 )
			return null;
		if ( itemCount > 256)
			itemCount = 256;

		int remainSize = buffer.capacity ();
        if ( remainSize < wifiItemSize )
            return null;

		//Utils.Log(0, "BuildWifiItemList: Parsing " + itemCount + " items. Buffersize " + remainSize );

		int start = 8;

		WifiItem[] items = new WifiItem [ itemCount ];
		if (items == null)
			return null;

		for ( int i=0; i<itemCount; i++ ) {
			if (remainSize < wifiItemSize)
				break;
			//Utils.Log(0, "BuildWifiItemList: Parsing " + i + ". Buffersize " + remainSize );

			WifiItem item =  new WifiItem();

			item.bssid = buffer.getLong ( start ); start += 8;
			item.rssi = buffer.getShort ( start ); start += 2;
			item.signal = buffer.getShort ( start ); start += 2;
			item.channel = buffer.getChar ( start ); start++;
			item.encrypt = buffer.getChar ( start ); start++;
			item.isConnected = (buffer.getChar ( start ) == 1); start++;
			item.sizeOfssid = (short) buffer.get ( start ); start++;

			remainSize -= wifiItemSize;

			if ( item.sizeOfssid > 0 ) {
                if (item.sizeOfssid > 32)
                    break;
				//Utils.Log(0, "BuildWifiItemList: Parsing " + i + ". ssid " + item.sizeOfssid );
				if (remainSize < item.sizeOfssid)
					break;
				item.ssid = GetString ( buffer, start, 32 );
				start += item.sizeOfssid;

				remainSize -= item.sizeOfssid;
			}
			items [ i ] = item;
		}

		//for(int i=0; i<items.length; i++)
		//	if (items[i] != null) Utils.Log(0, items[i].toString());

		return items;
	}


	static native void BtUpdateWithColonMacN ( String bssid, String ssid, int rssi, int cod, long uuid1, long uuid2, int updateState );

	/**
	 * Option for whether to observe blueooth to help location based services.
	 *
	 * @param	enable  A boolean that determines the target state.
	 */
	public void SetUseBtObserver(boolean enable) {
		SetUseWifiObserverN ( enable );
	}
	static native void SetUseBtObserverN(boolean enable);


	/**
	 * Query option for whether to observe blueooth to help location based services.
	 *
	 * @return enabled.
	 */
	public boolean GetUseBtObserver() {
		return GetUseBtObserverN ( );
	}
	static native boolean GetUseBtObserverN();

	/**
	 * Determines the interval for scanning of bluetooth devices.
	 *
	 * @param	interval  A millisecond value for scan intervals.
	 */
	public void SetUseBtInterval(int interval) {
		SetUseBtIntervalN ( interval );
	}
	static native void SetUseBtIntervalN(int interval);

	/**
	 * Query interval for scanning of bluetooth devices.
	 *
	 * @return interval in milliseconds.
	 */
	public int GetUseBtInterval() {
		return GetUseBtIntervalN ( );
	}
	static native int GetUseBtIntervalN();


	public static BtItem[] GetBts() {
		return BuildBtItemList(GetBtsN ());
	}
	static native ByteBuffer GetBtsN();


	@SuppressWarnings ( "all" )
	private static BtItem[] BuildBtItemList(ByteBuffer buffer)
	{
		if ( buffer == null )
			return null;

        final int btItemSize = 32;

		int itemCount = buffer.getInt(0);
		if ( itemCount <= 0 )
			return null;
		if ( itemCount > 256)
			itemCount = 256;

		int remainSize = buffer.capacity ();
        if ( remainSize < btItemSize )
            return null;

		//Utils.Log(0, "BuildBtItemList: Parsing " + itemCount + " items. Buffersize " + remainSize );

		int start = 8;

		BtItem[] items = new BtItem [ itemCount ];
		if (items == null)
			return null;

		for ( int i=0; i<itemCount; i++ ) {
			if (remainSize < btItemSize)
				break;
			//Utils.Log(0, "BuildBtItemList: Parsing " + i + ". Buffersize " + remainSize );

			BtItem item =  new BtItem();

			item.bssid = buffer.getLong ( start ); start += 8;
			item.rssi = buffer.getShort ( start ); start += 2;
			item.isConnected = (buffer.getChar ( start ) == 1); start++;
			item.sizeOfssid = (short) buffer.get ( start ); start++;
			item.uuid1 = buffer.getLong ( start ); start += 8;
			item.uuid2 = buffer.getLong ( start ); start += 8;

			remainSize -= btItemSize;

			if ( item.sizeOfssid > 0 ) {
                if ( item.sizeOfssid > 32 )
                    break;

				//Utils.Log(0, "BuildBtItemList: Parsing " + i + ". ssid " + item.sizeOfssid );
				if (remainSize < item.sizeOfssid)
					break;
				item.ssid = GetString ( buffer, start, 32 );
				start += item.sizeOfssid;

				remainSize -= item.sizeOfssid;
			}
			items [ i ] = item;
		}

		for(int i=0; i<items.length; i++)
			if (items[i] != null) Utils.Log(0, items[i].toString());

		return items;
	}
 	

    /** 
     * Determines whether to use native display resolution for the portal stream.
     * 
     * @param enable 	true = enable, false = disable
     */
	public void SetPortalNativeResolution(boolean enable) {
		SetPortalNativeResolutionN ( hEnvirons, enable );
	}
	static native void SetPortalNativeResolutionN(int hInst, boolean enable);


    /** 
     * Query whether to use native display resolution for the portal stream.
     * 
     * @return enabled
     */
	public boolean GetPortalNativeResolution() {
		return GetPortalNativeResolutionN ( hEnvirons );
	}
	static native boolean GetPortalNativeResolutionN(int hInst);
 	

    /** 
     * Determine whether to automatically Start a portal stream on successful device connection or not.
     * 
     * @param enable 	true = enable, false = disable
     */
	public void SetPortalAutoStart(boolean enable) {
		SetPortalAutoStartN ( hEnvirons, enable );
	}
	static native void SetPortalAutoStartN(int hInst, boolean enable);


    /** 
     * Query whether to automatically Start a portal stream on successful device connection or not.
     * 
     * @return enabled 	true = enable, false = disable
     */
	public boolean GetPortalAutoStart() {
		return GetPortalAutoStartN ( hEnvirons );
	}
	static native boolean GetPortalAutoStartN(int hInst);

 	
 	/** 
 	 * Query whether to use TCP for portal streaming (UDP otherwise)
 	 * 
     * @return enabled
     */
	public boolean GetPortalTCP() {
		return GetPortalTCPN ( hEnvirons );
	}
	static native boolean GetPortalTCPN(int hInst);
 	

 	/** 
 	 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
 	 * 
     * @param enable    A boolean that determines the target state.
     */
	public void SetPortalTCP(boolean enable) {
		SetPortalTCPN ( hEnvirons, enable );
	}
	static native void SetPortalTCPN(int hInst, boolean enable);

 	
 	/** 
 	 * Determine whether Environs shall automatically adapt the layout dimensions of
 	 * the View provided for the portal with the deviceID.
 	 * The layout dimensions are in particular important for proper mapping of TouchDispatch contact points
 	 * on the remote portal.
 	 * If enable is set to false, then custom applications must adapt the layout parameters
 	 * by means of calling SetPortalViewDims().
 	 * 
     * @param enable    A boolean that determines the target state.
     */
	public void SetPortalViewDimsAuto(boolean enable) {
		SetPortalViewDimsAutoN ( hEnvirons, enable );
	}
	static native void SetPortalViewDimsAutoN(int hInst, boolean enable);


 	/** 
 	 * Query the option whether Environs adapts the portal according to the size/lastLocation
 	 * of its view within the layout.
 	 * 
     * @return enabled
     */
	public boolean GetPortalViewDimsAuto() {
		return GetPortalViewDimsAutoN ( hEnvirons );
	}
	static native boolean GetPortalViewDimsAutoN(int hInst);


 	static native int GetBufferHeaderSizeN();
	static native int GetBufferHeaderBytesToSizeN();
	static native int GetBufferHeaderBytesToTypeN();
	static native int GetBufferHeaderBytesToStartValueN();

 	
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
     * @param deviceID	The device id which the message should be send to.
	 * @param async     Execute asynchronous. Non-async means that this call blocks until the command has finished.
     * @param message 	Message to send
     * @return success
     */ 
 	public boolean SendMessage(int deviceID, @Call.Value int async, String message) {
		return SendMessage(deviceID, null, null, async, message);
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
     * @param deviceID	    The device id which the message should be send to.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
	 * @param async        	Execute asynchronous. Non-async means that this call blocks until the command has finished.
     * @param message       A message to be send.
     * @return success
     */ 
 	public boolean SendMessage(int deviceID, String areaName, String appName, @Call.Value int async, String message)
 	{
 		if ( message == null || message.length() == 0 ) {
	 	    Utils.LogE ( className, "SendMessage: message to send is null!");
 			return false;
 		}
 		
 		return SendMessageN ( hEnvirons, deviceID, areaName, appName, async, message.getBytes (), message.length ( ) );
 	}
 	
 	static native boolean SendMessageN(int hInst, int deviceID, String areaName, String appName, @Call.Value int async, byte [] message, int length);

 	
    /**
     * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call. 
     * 
     * @param nativeID      The native device id of the target device.
	 * @param async        	Execute asynchronous. Non-async means that this call blocks until the command has finished.
     * @param fileID        A user-customizable id that identifies the file to be send.
     * @param fileDescriptor (e.g. filename)
     * @param buffer        A buffer to be send.
     * @param bytesToSend number of bytes in the buffer to send 
     * @return success
     */ 	
 	public boolean SendBuffer(int nativeID, @Call.Value int async, int fileID, String fileDescriptor, byte[] buffer, int bytesToSend)
	{
		return SendBufferN ( hEnvirons, nativeID, async, fileID, fileDescriptor, buffer, bytesToSend );
	}

	static native boolean SendBufferN(int hInst, int nativeID, @Call.Value int async, int fileID, String fileDescriptor, byte[] buffer, int bytesToSend);



	/**
	 * Send a buffer with bytes via udp to a device.&nbsp;The devices must be connected before for this call.
	 *
	 * @param async			(environs.Call.NoWait) Perform asynchronous. (environs.Call.Wait) Non-async means that this call blocks until the call finished.
	 * @param buffer        A buffer to be send.
	 * @param offset        A user-customizable id that identifies the file to be send.
	 * @param bytesToSend number of bytes in the buffer to send
	 * @return success
	 */
	public boolean SendDataUdp(int nativeID, @Call.Value int async, byte[] buffer, int offset, int bytesToSend)
	{
		return SendDataUdpN ( hEnvirons, nativeID, async, buffer, offset, bytesToSend );
	}

	static native boolean SendDataUdpN(int hInst, int nativeID, @Call.Value int async, byte[] buffer, int offset, int bytesToSend);


	/**
	 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
	 *
	 * @param sendID      		The send id of the target portal.
	 * @param portalUnitFlags	Flags that will be or-ed with the portalUnitType
	 * @param prefixBuffer     	A prefix that prepend the actual buffer.
	 * @param prefixSize   		The size of the content within the prefixbuffer.
	 * @param buffer        	A buffer to be send.
	 * @param offset        	An offset into the buffer.
	 * @param contentSize   	The size of the content within the buffer.
	 * @return success
	 */
	static native boolean SendTcpPortalN(int sendID, int portalUnitFlags, ByteBuffer prefixBuffer, int prefixSize, ByteBuffer buffer, int offset, int contentSize);

	/**
	 * Acquire a native layer send identifier (and increase instanceLock on device) to be used in SendTcpPortal. This resource must be released on disposal of the portal generator.
	 *
	 * @param portalUnitType	e.g. MSG_TYPE_STREAM
	 * @param portalID    		Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 * @return sendID
	 */
	static native int AcquirePortalSendIDN(int portalUnitType, int portalID);

	/**
	 * Release a native layer send identifier that was acquired by a call to AcquirePortalSendID.
	 *
	 * @param sendID      The portal send id resource to be released.
	 */
	static native void ReleasePortalSendIDN(int sendID);


	/**
	 * Acquire a native layer receive identifier (and increase instanceLock on device) to be used in receivePortalUni. This resource must be released on disposal of the portal generator.
	 *
	 * @param portalID    		Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 * @return receiveID
	 */
	static native int AcquirePortalReceiveIDN(int portalID);

	/**
	 * Release a native layer receive identifier that was acquired by a call to AcquirePortalReceiveID.
	 *
	 * @param receiveID      The portal receive id resource to be released.
	 */
	static native void ReleasePortalReceiveIDN(int receiveID);

	static native ByteBuffer ReceivePortalUnitN(int receiveID);

	/**
 	 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call. 
 	 * 
     * @param nativeID      The native device id of the target device.
	 * @param async        	Execute asynchronous. Non-async means that this call blocks until the command has finished.
     * @param fileID        A user-customizable id that identifies the file to be send.
     * @param fileDescriptor (e.g. filename)
     * @param filePath      The path to the file to be send.
     * @return success
     */
	public boolean SendFile(int nativeID, @Call.Value int async, int fileID, String fileDescriptor, String filePath) {
		return SendFileN ( hEnvirons, nativeID, async, fileID, fileDescriptor, filePath );
	}
	static native boolean SendFileN(int hInst, int nativeID, int async, int fileID, String fileDescriptor, String filePath);


	/**
	 * Instruct Environs native layer to prepare required portal resources to base on generation within the platform layer.
	 *
	 * @param enable      true = enable, false = disable
	 */
	public void SetUsePlatformPortalGenerator(boolean enable) {
		SetUsePlatformPortalGeneratorN ( hEnvirons, enable );
	}
	static native void SetUsePlatformPortalGeneratorN(int hInst, boolean enable);


	/**
	 * Find a free portalID slot for the direction encoded into the given portalDetails.
	 *
	 * @param 	nativeID    	The native device id of the target device.
	 * @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
	 *
	 * @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
	 */
	int GetPortalIDFreeSlot ( int nativeID, int portalDetails ) {
		return GetPortalIDFreeSlotN ( hEnvirons, nativeID, portalDetails );
	}
	static native int GetPortalIDFreeSlotN ( int hInst, int nativeID, int portalDetails );


	/**
	 * Request a portal stream from the device with the given id.&nbsp;The device must be connected before with DeviceConnect ().
	 *
	 * @param 	nativeID    	The native device id of the target device.
	 * @param 	async       	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalDetails 	An application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 * @param 	width       	The width of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
	 *                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
	 * @param 	height      	The height of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
	 *                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
	 *
	 * @return 	success
	 */
	public boolean RequestPortalStream(int nativeID, @Call.Value int async, int portalDetails, int width, int height) {
		return RequestPortalStreamN ( hEnvirons, nativeID, async, portalDetails, width, height );
	}

	static native boolean RequestPortalStreamN(int hInst, int nativeID, int async, int portalDetails, int width, int height);


	/**
	 * Provide a portal stream to the device with the given id.&nbsp;
	 * The device must be in a connected state by means of prior call to DeviceConnect ().
	 *
	 * @param 	nativeID    	The native device id of the target device.
	 * @param 	async       	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 *
	 * @return  success
	 */
	public boolean ProvidePortalStream(int nativeID, @Call.Value int async, int portalDetails)
	{
		return ProvidePortalStreamN ( hEnvirons, nativeID, async, portalDetails );
	}


	/**
	 * Native call: Provide a portal stream to the device with the given id.
	 * The device must be in a connected state by means of prior call to DeviceConnect ().
	 *
	 * @param 	nativeID    The native device id of the target device.
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalID	Values should be of type PortalType. This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 * @return  success
	 */
	static native boolean ProvidePortalStreamN(int hInst, int nativeID, int async, int portalID);


	/**
	 * Provide a portal stream to the device with the given id.
	 * The device must be in a connected state by means of prior call to DeviceConnect ().
	 *
	 * @param 	nativeID    	The native device id of the target device.
	 * @param 	async       	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalDetails	Values should be of type PortalType. This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 *
	 * @return  success
	 */
	public boolean ProvideRequestPortalStream(int nativeID, @Call.Value int async, int portalDetails)
	{
		return ProvideRequestPortalStreamN ( hEnvirons, nativeID, async, portalDetails );
	}


	/**
	 * Native call: Provide a portal stream to the device with the given id.
	 * The device must be in a connected state by means of prior call to DeviceConnect ().
	 *
	 * @param 	nativeID    The native device id of the target device.
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalDetails	Values should be of type PortalType. This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
	 *
	 * @return success
	 */
	static native boolean ProvideRequestPortalStreamN(int hInst, int nativeID, int async, int portalDetails);


	/**
	 * Start streaming of portal to or from the portal identifier (received in notification).
	 *
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
	 * @return 	success
	 */
	public boolean StartPortalStream(@Call.Value int async, int portalID)
	{
		return StartPortalStreamN ( hEnvirons, async, portalID );
	}

	/**
	 * Start streaming of portal to or from the portal identifier (received in notification).
	 *
	 * @param async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
	 * @return success
	 */
	static native boolean StartPortalStreamN(int hInst, int async, int portalID);


	/**
	 * Pause streaming of portal to or from the portal identifier (received in notification).
	 *
	 * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
	 * 						It is provided within the notification listener as sourceIdent.&nbsp;
	 * 						Applications should store them in order to address the correct portal within Environs.
	 * @return success
	 */
	public boolean PausePortalStream(@Call.Value int async, int portalID) {
		return PausePortalStreamN ( hEnvirons, async, portalID );
	}


	/**
	 * Pause streaming of portal to or from the portal identifier (received in notification).
	 *
	 * @param 	async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
	 * 						It is provided within the notification listener as sourceIdent.&nbsp;
	 * 						Applications should store them in order to address the correct portal within Environs.
	 * @return success
	 */
	static native boolean PausePortalStreamN(int hInst, int async, int portalID);


    /**
     * Stop streaming of portal to or from the portal identifier (received in notification).
     *
	 * @param 	async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param 	nativeID    The native device id of the target device.
	 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
	 * 						It is provided within the notification listener as sourceIdent.&nbsp;
     * 					    Applications should store them in order to address the correct portal within Environs.
	 * @return success
     */
 	public boolean StopPortalStream(@Call.Value int async, int nativeID, int portalID)
	{
		/*
		if ((portalID & Environs.PORTAL_DIR_OUTGOING) != 0) {
			final String key = buildPortalKey(deviceID, portalID, areaName, appName);

			if (async == Environs.CALL_NOWAIT) {
				new Thread(new Runnable() {
					public void run(){
						stopPortalGenerator ( key );
					}
				}).start();
			}
			else
				stopPortalGenerator ( key );
		}
		else {
			//String key = buildPortalKey(deviceID, portalID, areaName, appName);
		}
		*/


		return StopPortalStreamN ( hEnvirons, async, nativeID, portalID );
	}

	/*
	private static void stopPortalGenerator(String key)
	{
		synchronized (portalGenerators)
		{
			PortalGenerator portal = portalGenerators.get(key);
			if (portal != null) {
				portalGenerators.remove(key);
				portal.release();
			}
		}
	}
	*/


	/**
	 * Stop streaming of portal from this device.
	 *
	 * @param async     Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param nativeID  The native device id of the target device.
	 * @param portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
	 * 					It is provided within the notification listener as sourceIdent.&nbsp;
	 * 					Applications should store them in order to address the correct portal within Environs.
	 * @return success
	 */
	static native boolean StopPortalStreamN(int hInst, int async, int nativeID, int portalID);



	/**
	 * Set render surface.
	 *
	 * @param portalID		The portal id.
	 * @param renderSurface	A surface on which the portal shall be rendered.
	 * @param width		    The width in of the surface in pixel.
	 * @param height		The height of the surface in pixel.
	 * @return success		true = yes, false = no
	 */
	public boolean SetRenderSurface(int portalID, Surface renderSurface, int width, int height) {
		return ( !GetUseNativeDecoder ( ) || SetRenderSurfaceN ( hEnvirons, portalID, renderSurface, width, height ) );

	}

	static boolean SetRenderSurface(int hInst, int portalID, Surface renderSurface, int width, int height)
	{
		return instances[hInst].SetRenderSurface(portalID, renderSurface, width, height);
	}


	/**
	 * Set render surface.
	 *
	 * @param portalID		The portal id.
	 * @param renderSurface	The surface on which the surface shall be rendered.
	 * @param width		    The width in of the surface in pixel.
	 * @param height		    The height of the surface in pixel.
	 * @return		true = yes, false = no
	 */
	static native boolean SetRenderSurfaceN(int hInst, int portalID, Surface renderSurface, int width, int height);


	/**
	 * Release render surface.
	 *
	 * @param async        	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param portalID		The portal id.
	 * @return		true = yes, false = no
	 */
	public boolean ReleaseRenderSurface(@Call.Value int async, int portalID)
	{
		String key = buildPortalKey(portalID);

		if (GetUseNativeDecoder()) {

			synchronized (portalReceivers)
			{
				PortalReceiver stream = portalReceivers.get(key);
				if ( stream != null ) {
					if ( stream.decoder != null )
						stream.decoder.release();
				}
			}
		}
		return ReleaseRenderSurfaceN ( hEnvirons, async, portalID );
	}

	static boolean ReleaseRenderSurface(int hInst, int async, int portalID) {
		return instances[hInst].ReleaseRenderSurface (async, portalID);
	}

	static native boolean ReleaseRenderSurfaceN(int hInst, int async, int portalID);


	static native void RequestPortalIntraFrameN(int async, int portalID);


	/**
	 * Query the portal id that we have requested a portal before.
	 *
	 * @return deviceID
	 */
	public static native int GetRequestedPortalIDN();



	static native boolean GetPortalInfoN(int hInst, ByteBuffer buffer);

 	static native boolean SetPortalInfoN(int hInst, ByteBuffer buffer);



	/**
	 * Send portal Init packet with dimensions of the portal.
	 *
	 * @param async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
	 * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
	 * 						It is provided within the notification listener as sourceIdent.&nbsp;
	 * 						Applications should store them in order to address the correct portal within Environs.
	 * @param width		 	The portal width in pixel.
	 * @param height		The portal height in pixel.
	 * @return success
	 */
	boolean SendPortalInit(int async, int portalID, int width, int height) {
		return SendPortalInitN ( hEnvirons, async, portalID, width, height );
	}
	static native boolean SendPortalInitN(int hInst, int async, int portalID, int width, int height);



 	static native void TouchDispatchN(int hInst, int portalID, ByteBuffer touches, int count, boolean init);


	private static boolean showVerboseTouchOutput = false;
    private static ByteBuffer touchesCache = null;

 	static native int GetSizeOfInputPackN();
 	private static int sizeOfInputPack = 0;

 	static short[] touchEvents = { 
 		Types.INPUT_STATE_ADD, // 0, MotionEvent.ACTION_DOWN
 		Types.INPUT_STATE_DROP, // 1, MotionEvent.ACTION_UP
 		Types.INPUT_STATE_CHANGE, // 2, MotionEvent.ACTION_MOVE
 		Types.INPUT_STATE_DROP, // 3, MotionEvent.ACTION_CANCEL
 		Types.INPUT_STATE_DROP, // 4, ACTION_OUTSIDE
 		Types.INPUT_STATE_ADD, 	// 5, MotionEvent.ACTION_POINTER_1_DOWN
 		Types.INPUT_STATE_DROP, // 6, MotionEvent.ACTION_POINTER_1_UP
 		};

 	
 	
    @SuppressLint("NewApi")
	private boolean DispatchTouch(int portalID, MotionEvent event)
    {
        int action = event.getActionMasked();

        if ( action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL ) {
            TouchDispatchN ( hEnvirons, portalID, null, 0, false );
        }
        else {      
            int pointerCount = event.getPointerCount();

            int touchesReqSize = sizeOfInputPack * pointerCount;

        	if ( touchesCache == null || touchesCache.capacity() < touchesReqSize )
			{
				if ( touchesCache != null ) {
					if (Utils.isDebug) Utils.Log ( 5, className, "DispatchTouch: touchesCache.capacity: " + touchesCache.capacity() );
					Java_free(touchesCache);
				}

        		touchesCache = Java_malloc(touchesReqSize, true);
        		if ( touchesCache == null )
        			return false;
        	}
        	
            int actionIndex = event.getActionIndex();
            
        	touchesCache.position(0);
        	
            for ( int pointer = 0; pointer < pointerCount; pointer ++ )
			{
            	//touchesCache.putInt(0); // frame_id
				touchesCache.putInt(event.getPointerId(pointer)); //id

				short state;

            	if ( pointer == actionIndex )
				{
            		if ( action < touchEvents.length )
						state = touchEvents[action];
            		else
						state = (short)InputState.Add;
            	}
            	else
					state = (short)InputState.Change;

				touchesCache.putShort(state); // state

				touchesCache.put((byte) InputType.Finger); //type
				touchesCache.put((byte) 1); //valid

				touchesCache.putInt((int) event.getX(pointer)); //x
            	touchesCache.putInt((int) event.getY(pointer)); //y

				touchesCache.putInt(0); //value

            	touchesCache.putFloat(event.getOrientation(pointer)); //angle
            	touchesCache.putFloat(event.getSize(pointer)); //size

            	if ( Utils.APILevel >= 12 ) {
                	touchesCache.putFloat(event.getAxisValue(MotionEvent.AXIS_X, pointer)); //axisx
                	touchesCache.putFloat(event.getAxisValue(MotionEvent.AXIS_Y, pointer)); //axisy
            	}
            	else {
                	touchesCache.putFloat(0);
                	touchesCache.putFloat(0);        		
            	}
            }

	        /*if (true) {
				for (int p = 0; p < pointerCount; p++) {
					float x = event.getX(p);
					float y = event.getY(p);
					float o = event.getOrientation(p);
					float size = event.getSize(p);
					int id = event.getPointerId(p);
					if (p == actionIndex) {
						Utils.Log("[TRACE] Touch: Events(" + p + "/" + pointerCount + ") - id(" + id + "), x(" + x + "), y(" + y + "), action(" + eventActionToString(action) + "), size(" + size + "), o(" + o + ")");
					} else {
						Utils.Log("[TRACE] Touch: Events(" + p + "/" + pointerCount + ") - id(" + id + "), x(" + x + "), y(" + y + "), size(" + size + "), o(" + o + ")");
					}
				}
			}*/

			touchesCache.position(0);
			TouchDispatchN ( hEnvirons, portalID, touchesCache, pointerCount, action == MotionEvent.ACTION_DOWN );
        }
        
        return true;
    }

    
    private static String eventActionToString(int eventAction) {
        switch (eventAction) {
            case MotionEvent.ACTION_CANCEL: return "Cancel";
            case MotionEvent.ACTION_DOWN: return "Down";
            case MotionEvent.ACTION_MOVE: return "Move";
            case MotionEvent.ACTION_OUTSIDE: return "Outside";
            case MotionEvent.ACTION_UP: return "Up";
            case MotionEvent.ACTION_POINTER_DOWN: return "Pointer Down";
            case MotionEvent.ACTION_POINTER_UP: return "Pointer Up";
        }
        return "";
    }

    
    /** 
     * Dispatch an asynchronous notification to registered EnvironsListeners.&nbsp;
     * This dispatcher is only available to and should be used within the platform code.
     * 
     * @param objID      The native device id of the sender device.
     * @param notification  A notification value that may be one of the NOTIFY_* values.
     * @param source        An integer which value must match one of TYPES.ENVIRONS_SOURCE_*
	 * @param context        An additional context to attach to the notification.
     */
 	static native void BridgeForNotifier ( int hInst, int objID, int notification, int source, int context );

 	
    /** 
     * Dispatch an asynchronous notification to registered EnvironsListeners.&nbsp;
     * This dispatcher is only available to and should be used within the platform code.
     *
	 * @param objID      The native device id of the sender device.
     * @param notification  A notification value that may be one of the NOTIFY_* values.
     * @param source        An enumeration value of type environs.Source
	 * @param context        An additional context to attach to the notification.
     */
 	/*static void BridgeForNotifier ( int hInst, int objID, int notification, int source, int context) {
		BridgeForNotifier(hInst, objID, notification, source, context);
 	}
	*/

 	
    /** 
     * Dispatch an asynchronous notification to registered EnvironsListeners
     *
	 * @param objID      The native device id of the sender device.
     * @param notification  A notification value that must not be one of the NOTIFY_* values.
	 * @param context        An additional context to attach to the notification.
     */
 	public static void BridgeForNotifier ( int hInst, int objID, int notification, int context) {
		BridgeForNotifier(hInst, objID, notification, Source.Application, context);
 	}


	static Environs GetLatestInstance() {
		for (int i=1; i< ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE; i++)
			if (instances[i] != null)
				return instances[i];
		return null;
	}


	boolean isUIAdapter = false;


	/**
	 * Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
	 *
	 * @return A DeviceList object
	 */
	public DeviceList CreateDeviceList () {
		return CreateDeviceList ( DeviceClass.All );
	}


	/**
	 * Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
	 *
	 * @param deviceClass	A value of type Environs.MEDIATOR_DEVICE_CLASS_* that determines the list type
	 *
	 * @return A DeviceList object
	 */
	public DeviceList CreateDeviceList ( int deviceClass ) {
		return new DeviceList ( this, null, deviceClass, null, 0, 0 );
	}


	/**
	 * Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
	 * In addition to that, the DeviceList serves as an Adaptor for ListViews using the given details.
	 *
	 * @param context		An application context that manages the ListView
	 * @param deviceClass	A value of type environs.DeviceClass that determines the list type
	 * @param layout_id		An android layout resource id
	 * @param view_id		An android layout resource id
	 *
	 * @return A DeviceList object
	 */
	public DeviceList CreateDeviceList(Activity context, int deviceClass, int layout_id, int view_id) {
		return CreateDeviceList ( context, deviceClass, null, layout_id, view_id );
	}


	/**
	 * Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
	 * In addition to that, the DeviceList serves as an Adaptor for ListViews using the given details.
	 *
	 * @param context		An application context that manages the ListView
	 * @param deviceClass	A value of type environs.DeviceClass that determines the list type
	 * @param observer		An observer that gets notified about list changes
	 * @param layout_id		An android layout resource id
	 * @param view_id		An android layout resource id
	 *
	 * @return A DeviceList object
	 */
	public DeviceList CreateDeviceList(Activity context, int deviceClass, ListObserver observer, int layout_id, int view_id) {

		return new DeviceList(this, context, deviceClass, observer, layout_id, view_id);
	}


	/**
	 * Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
	 * In addition to that, the DeviceList serves as an Adaptor for ListViews using the given details.
	 *
	 * @param viewGenerator 	An application implemented ViewGenerator object that provides the getView() method
	 * @param context	 		An application context that manages the ListView
	 * @param deviceClass		A value of type environs.DeviceClass that determines the list type
	 * @param observer		 	An observer that gets notified about list changes
	 * @param layout_id			An android layout resource id
	 * @param view_id			An android layout resource id
	 *
	 * @return A DeviceList object
	 */
	public DeviceList CreateDeviceList(ViewGenerator viewGenerator, Activity context, int deviceClass, ListObserver observer, int layout_id, int view_id) {

		DeviceList list = new DeviceList(this, context, deviceClass, observer, layout_id, view_id);
		list.SetGenerator ( viewGenerator );
		return list;
	}


	void DisposeDeviceList (DeviceList list)
	{
		if (deviceLists.contains(list))
		{
			deviceLists.remove ( list );
			if (list.isUIAdapter )
				DeviceList.UpdateNotifyAdapterStatus ( deviceLists );
		}
	}


 	static native int GetDevicesCountN(int hInst, int fromType);

    /**
     * Query the number of available devices within the environment (including those of the Mediator)
     *
     * @return numberOfDevices
     */
	int GetDevicesCount0() {
		return GetDevicesCountN ( hEnvirons, DeviceClass.All );
	}


	/**
	 * Query the number of nearby (broadcast visible) devices within the environment.
     * 
     * @return numberOfDevices
     */
	int GetDevicesNearbyCount0() {
		return GetDevicesCountN ( hEnvirons, DeviceClass.Nearby );
	}


	int GetDevicesFromMediatorCount0() {
		return GetDevicesCountN ( hEnvirons, DeviceClass.Mediator );
	}
 	
 	
 	static native ByteBuffer GetDevicesN(int hInst, int fromType);
 	
 	static native ByteBuffer GetDeviceN(int hInst, int deviceID, String areaName, String appName, int fromType);


	/**
	 * Query a collection of DeviceInstance objects of nearby (broadcast visible) devices within the environment.
	 *
	 * @return DeviceInstance-objects
	 */
	DeviceInstance[] GetDevicesNearby0() {
		return BuildDeviceInfoList(hEnvirons, GetDevicesN ( hEnvirons, DeviceClass.Nearby ), false);
	}


	/**
	 * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
     * 
     * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
     * @return DeviceInstance-object
     */
	DeviceInstance GetDeviceNearby0(int deviceID, String areaName, String appName)
	{
		DeviceInstance[] infos = BuildDeviceInfoList(hEnvirons, GetDeviceN ( hEnvirons, deviceID, areaName, appName, DeviceClass.Nearby ), false);
		if (infos != null && infos[0] != null)
			return infos[0];
		return null;
	}
 	 	

    /**
     * Query a collection of DeviceInstance objects of Mediator managed devices within the environment.
     *
     * @return DeviceInstance collection
     */
	DeviceInstance[] GetDevicesFromMediator0()
	{
		return BuildDeviceInfoList(hEnvirons, GetDevicesN ( hEnvirons, DeviceClass.Mediator ), true);
	}


	/**
	 * Query a DeviceInstance object of Mediator managed devices within the environment.
	 * 
	 * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
	 * @return DeviceInstance-object
	 */
	DeviceInstance GetDeviceFromMediator0(int deviceID, String areaName, String appName)
	{
		DeviceInstance[] infos = BuildDeviceInfoList(hEnvirons, GetDeviceN ( hEnvirons, deviceID, areaName, appName, DeviceClass.Mediator ), true);
		if (infos != null && infos[0] != null)
			return infos[0];
		return null;
	}


	DeviceInstance[] GetDevices0() {
		return BuildDeviceInfoList ( hEnvirons, GetDevicesN ( hEnvirons, DeviceClass.All ), false );
	}


	/**
	 * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
	 *
	 * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
	 * @return DeviceInstance-object
	 */
	DeviceInstance GetDevice0(int deviceID, String areaName, String appName)
	{
		return GetDevice0 ( hEnvirons, deviceID, areaName, appName );
	}

	static DeviceInstance GetDevice0(int hInst, int deviceID, String areaName, String appName) {
		DeviceInstance[] infos = BuildDeviceInfoList(hInst, GetDeviceN ( hInst, deviceID, areaName, appName, DeviceClass.All ), false);
		if (infos != null && infos[0] != null)
			return infos[0];
		return null;
	}


	/**
	 * Query a DeviceInstance object for the active portal identified by the portalID.
	 *
	 * @param portalID      The portalID that identifies an active portal.
	 * @return DeviceInstance-object
	 */
	public DeviceInstance GetDeviceForPortal(int portalID)
	{
		DeviceInstance[] infos = BuildDeviceInfoList(hEnvirons, GetDeviceForPortalN ( hEnvirons, portalID ), false);
		if (infos.length > 0)
			return infos[0];
		return null;
	}

	/**
	 * Query a DeviceInstance object for the active portal identified by the portalID.
	 *
	 * @param portalID      The portalID that identifies an active portal.
	 * @return DeviceInstance-object
	 */
	static native ByteBuffer GetDeviceForPortalN(int hInst, int portalID);




	/**
	 * Query a DeviceInstance object that best match the deviceID only.
	 * Usually the one that is in the same app environment is picked up.
	 * If there is no matching in the app environment,
	 * then the areas are searched for a matching deviceID.
	 *
	 * @param deviceID      The portalID that identifies an active portal.
	 * @return DeviceInstance-object
	 */
	DeviceInstance GetDevice0(int deviceID)
	{
		return GetDevice0(hEnvirons, deviceID);
	}

	static DeviceInstance GetDevice0(int hInst, int deviceID)
	{
		DeviceInstance[] infos = BuildDeviceInfoList(hInst, GetDeviceBestMatchN ( hInst, deviceID ), false);
		if (infos != null && infos.length > 0) {
			return infos[0];
		}
		return null;
	}

	/**
	 * Query a DeviceInstance object that best match the deviceID only.
	 * Usually the one that is in the same app environment is picked up.
	 * If there is no matching in the app environment,
	 * then the areas are searched for a matching deviceID.
	 *
	 * @param objID      The objID of the device.
	 * @return DeviceInstance-object
	 */
	DeviceInstance GetDeviceByObjID0 ( int objID )
	{
		return GetDeviceByObjID0 ( hEnvirons, objID );
	}

	static DeviceInstance GetDeviceByObjID0 ( int hInst, int objID )
	{
		DeviceInstance[] infos = BuildDeviceInfoList ( hInst, GetDeviceByObjIDN ( hInst, objID ), false);
		if (infos != null && infos.length > 0) {
			return infos[0];
		}
		return null;
	}

	/**
	 * Query a DeviceInstance object that best match the deviceID only.
	 * Usually the one that is in the same app environment is picked up.
	 * If there is no matching in the app environment,
	 * then the areas are searched for a matching deviceID.
	 * Note: IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
	 *
	 * @param objID      The objID of the device.
	 * @return DeviceInstance-ByteBuffer
	 */
	static native ByteBuffer GetDeviceByObjIDN(int hInst, int objID);

	/**
	 * Query a DeviceInstance object that best match the deviceID only.
	 * Usually the one that is in the same app environment is picked up.
	 * If there is no matching in the app environment,
	 * then the areas are searched for a matching deviceID.
	 * Note: IMPORTANT!!! On NON-ANDROID -> the requestor has to free the memory!!!
	 *
	 * @param deviceID      The portalID that identifies an active portal.
	 * @return DeviceInstance-ByteBuffer
	 */
	static native ByteBuffer GetDeviceBestMatchN(int hInst, int deviceID);


	private static int DeviceInstanceSize = 38;
 	private static native int GetDeviceInstanceSizeN();
 	private static int DevicesHeaderSize = 16;
 	private static native int GetDevicesHeaderSizeN();


	@SuppressWarnings ( "all" )
 	private static DeviceInstance[] BuildDeviceInfoList(int hInst, ByteBuffer buffer, boolean mediator)
 	{
 		if ( buffer == null )
 			return null;

    	int deviceCount = buffer.getInt(8);
 		if ( deviceCount <= 0 )
 			return null;

    	int DeviceInstSize = Environs.DeviceInstanceSize;
    	int DeviceStart = Environs.DevicesHeaderSize;

    	DeviceInstance[] devices = new DeviceInstance[ deviceCount ];

    	for ( int i=0; i<deviceCount; i++ ) {
    		DeviceInstance device =  new DeviceInstance();
    		devices [ i ] = device;
			device.hEnvirons = hInst;
			device.env = instances[hInst];

    		device.deviceID = buffer.getInt ( DeviceStart );
			device.nativeID = buffer.getInt ( DeviceStart + DEVICEINFO_NATIVE_ID_START );

    		int IP = buffer.getInt ( DeviceStart + DEVICEINFO_IP_START );
    		device.ip = String.format ( "%d.%d.%d.%d",
					( IP & 0xff ),
					( IP >> 8 & 0xff ),
					( IP >> 16 & 0xff ),
					( IP >> 24 & 0xff ) );

			IP = buffer.getInt ( DeviceStart + DEVICEINFO_IPe_START );
    		device.ipe = String.format ( "%d.%d.%d.%d",
					( IP & 0xff ),
					( IP >> 8 & 0xff ),
					( IP >> 16 & 0xff ),
					( IP >> 24 & 0xff ) );

			device.tcpPort = buffer.getShort ( DeviceStart + DEVICEINFO_TCP_PORT_START );
			device.udpPort = buffer.getShort ( DeviceStart + DEVICEINFO_UDP_PORT_START );
			device.updates = buffer.getInt ( DeviceStart + DEVICEINFO_UPDATES_START );
			device.platform = buffer.getInt ( DeviceStart + DEVICEINFO_PLATFORM_START );
			device.sourceType = buffer.get ( DeviceStart + DEVICEINFO_BROADCAST_START );
			if (mediator)
				device.sourceType = 0;
			device.unavailable = (buffer.get ( DeviceStart + DEVICEINFO_UNAVAILABLE_START ) == 1);
			device.isConnected = (buffer.get ( DeviceStart + DEVICEINFO_ISCONNECTED_START ) == 1);
			//device.flags = (int) buffer.get ( DeviceStart + DEVICEINFO_INTERNAL_FLAGS_START );

			device.isSameAppArea = (buffer.get ( DeviceStart + DEVICEINFO_HASAPPAREA_START ) == 0);
			//if (Utils.isDebug) Utils.Log ( 4, className, "BuildDeviceInfoList: isSameAppArea [ " + device.isSameAppArea + " ].");

			device.deviceName = GetString ( buffer, DeviceStart + DEVICEINFO_DEVICENAME_START, MAX_LENGTH_DEVICE_NAME);
    		device.areaName = GetString ( buffer, DeviceStart + DEVICEINFO_AREANAME_START, MAX_LENGTH_AREA_NAME );
    		device.appName = GetString ( buffer, DeviceStart + DEVICEINFO_APPNAME_START, MAX_LENGTH_APP_NAME );
			device.objID = buffer.getInt ( DeviceStart + DEVICEINFO_OBJID_START );

			/*
			if (device.env != null)
				device.isSameAppArea = device.EqualsAppEnv ( null, null );
			boolean isSameAppArea = (buffer.get ( DEVICEINFO_HASAPPAREA_START ) != 0);

			if (device.isSameAppArea != isSameAppArea) {
				Utils.LogE(className, "BuildDeviceInfo: APPAREA not EQUAL !!! " + device.toString());
			}*/

        	//Utils.Log( className, "BuildDeviceInfoList: Device id=" + device.ID + " Type=" + device.deviceType + " Name=" + device.deviceName);
        	DeviceStart += DeviceInstSize;
    	}

    	//for(int i=0; i<devices.length; i++)
    		//Utils.Log(0, devices[i].toString());
    		
    	return devices;
 	}


	@SuppressWarnings ( "all" )
	static DeviceInstance BuildDeviceInfo ( int hInst, byte [] bytes )
	{
		if ( bytes == null )
			return null;

		ByteBuffer buffer = ByteBuffer.wrap ( bytes );

		ByteOrder nativeOrder = ByteOrder.nativeOrder ( );
		if ( buffer.order() != nativeOrder )
			buffer.order ( nativeOrder );

		DeviceInstance device = new DeviceInstance ();

		device.hEnvirons = hInst;
		device.env = instances[hInst];

		device.deviceID = buffer.getInt ( 0 );
		device.nativeID = buffer.getInt ( DEVICEINFO_NATIVE_ID_START );

		int IP = buffer.getInt ( DEVICEINFO_IP_START );
		device.ip = String.format ( "%d.%d.%d.%d",
				( IP & 0xff ),
				( IP >> 8 & 0xff ),
				( IP >> 16 & 0xff ),
				( IP >> 24 & 0xff ) );

		IP = buffer.getInt ( DEVICEINFO_IPe_START );
		device.ipe = String.format ( "%d.%d.%d.%d",
				( IP & 0xff ),
				( IP >> 8 & 0xff ),
				( IP >> 16 & 0xff ),
				( IP >> 24 & 0xff ) );

		device.tcpPort = buffer.getShort ( DEVICEINFO_TCP_PORT_START );
		device.udpPort = buffer.getShort ( DEVICEINFO_UDP_PORT_START );
		device.updates = buffer.getInt ( DEVICEINFO_UPDATES_START );
		device.platform = buffer.getInt ( DEVICEINFO_PLATFORM_START );
		device.sourceType = buffer.get ( DEVICEINFO_BROADCAST_START );
		device.unavailable = (buffer.get ( DEVICEINFO_UNAVAILABLE_START ) == 1);
		device.isConnected = (buffer.get ( DEVICEINFO_ISCONNECTED_START ) == 1);
		device.isSameAppArea = (buffer.get ( DEVICEINFO_HASAPPAREA_START ) == 0);

		device.deviceName = GetString ( buffer, DEVICEINFO_DEVICENAME_START, MAX_LENGTH_DEVICE_NAME );
		device.areaName = GetString ( buffer, DEVICEINFO_AREANAME_START, MAX_LENGTH_AREA_NAME );
		device.appName = GetString ( buffer, DEVICEINFO_APPNAME_START, MAX_LENGTH_APP_NAME );
		device.objID = buffer.getInt ( DEVICEINFO_OBJID_START );

		/*if (device.env != null)
			device.isSameAppArea = device.EqualsAppEnv ( null, null );
		boolean isSameAppArea = (buffer.get ( DEVICEINFO_HASAPPAREA_START ) == 0);

		if (device.isSameAppArea != isSameAppArea) {
			Utils.LogE(className, "BuildDeviceInfo: APPAREA not EQUAL !!! " + device.toString());
		}*/
		//Utils.Log(0, "BuildDeviceInfo: " + device.toString());
		return device;
	}


    private static String GetString(ByteBuffer buffer, int start, int maxSize ) {
    	String s = "";
    	int i = start;
        int cap = buffer.capacity ();
    	do
    	{
            if ( maxSize <= 0)
                break;
            maxSize--;

    		byte c = buffer.get ( i );
    		if ( c == 0 )
    			break;
    		s += String.valueOf((char)c);
    		i++;
            if ( i >= cap )
                break;
    	}
    	while ( true );
    	
    	return s;
    }


    /** 
     * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
     * 
     * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
     * @param fileID        The id of the file to load.
     * @return absolutePath
     */
    public String GetFilePath(int deviceID, String areaName, String appName, int fileID) {
		return GetFilePathN ( hEnvirons, deviceID, areaName, appName, fileID );
	}
	static native String GetFilePathN(int hInst, int deviceID, String areaName, String appName, int fileID);


	/**
	 * Query the absolute path for the local storage of a particular device.
	 *
	 * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
	 * @return absolutePath
	 */
	static native String GetFilePathForStorageN(int hInst, int deviceID, String areaName, String appName);

	public String GetFilePathForStorage(int deviceID, String areaName, String appName)
	{
		return GetFilePathForStorageN ( hEnvirons, deviceID, areaName, appName );
	}


	/**
	 * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
	 *
	 * @param nativeID      The native device id of the sender device.
	 * @param fileID        The id of the file to load.
	 * @return absolutePath
	 */
	public String GetFilePathNative(int nativeID, int fileID) {
		return GetFilePathNativeN ( hEnvirons, nativeID, fileID );
	}
	static native String GetFilePathNativeN(int hInst, int nativeID, int fileID);
    

    /** 
     * Load the file that is assigned to the fileID received by deviceID into an byte array.
     *
	 * @param nativeID      The native device id of the sender device.
     * @param fileID        The id of the file to load.
     * @return byte-array
     */
    public byte[] GetFileNative(int nativeID, int fileID)
    {
		return GetFileNativeN ( hEnvirons, nativeID, fileID, null, 0 );
    }

    /** 
     * Load the file that is assigned to the fileID received by deviceID into an byte array.
     * 
     * @param deviceID      The device id of the target device.
	 * @param areaName		Area name of the application environment
	 * @param appName		Application name of the application environment
     * @param fileID        The id of the file to load.
     * @return byte-array
     */
    public byte[] GetFile(int deviceID, String areaName, String appName, int fileID)
    {
    	return GetFileN ( hEnvirons, deviceID, areaName, appName, fileID, null, 0 );
    }

    private static native byte[] GetFileN(int hInst, int deviceID, String areaName, String appName, int fileID, byte[] buffer, int capacity);

	private static native byte[] GetFileNativeN(int hInst, int nativeID, int fileID, byte[] buffer, int capacity);


    /**
     * Query a DeviceDisplay object of the device with the deviceID.&nbsp;
     * Note: The device must be connected in order to query the information.
     * 
     * @param nativeID      The native device id of the target device.
     * @return DeviceDisplay
     */
 	public DeviceDisplay GetDeviceDisplayProps(int nativeID)
 	{		
		return BuildDeviceScreen(GetDeviceDisplayPropsN ( hEnvirons, nativeID ));
 	}
 	
 	static native ByteBuffer GetDeviceDisplayPropsN(int hInst, int nativeID);

 	private static DeviceDisplay BuildDeviceScreen(ByteBuffer buffer)
 	{
 		if ( buffer == null )
 			return null; 		
 		
 		DeviceDisplay props = new DeviceDisplay();

 		props.width = buffer.getInt ( 0 );
 		props.height = buffer.getInt ( 4 );
 		props.width_mm = buffer.getInt(8);
 		props.height_mm = buffer.getInt(12);
		props.orientation = buffer.getInt ( 16 );
		props.dpi = buffer.getFloat ( 20 );

		return props;
 	}


	class ListContext
	{
		@DeviceClass.Value int   type;

		ArrayList < DeviceInstance >    vanished;
		ArrayList < DeviceInstance >    appeared;
	}

	final ListContext                 contextAll = new ListContext ();
	final ListContext                 contextMediator = new ListContext ();
	final ListContext                 contextNearby= new ListContext ();

	void DisposeLists()
	{
		if (listAll != null) {
			DeviceList.DisposeList ( this, isUIAdapter, listAll );

			if (listNearby != null)
				listNearby.clear();
			if (listMediator != null)
				listMediator.clear();
		}
		else {
			DeviceList.DisposeList ( this, isUIAdapter, listNearby );
			DeviceList.DisposeList ( this, isUIAdapter, listMediator );
		}

		listAll = null;
		listNearby = null;
		listMediator = null;
	}


	/**
	 * Reload all device lists. Applications may call this if they manually stopped and started Environs again.
	 * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
	 */
	void ReloadLists()
	{
		if (listAll != null) {
			DeviceListUpdate ( DeviceClass.All );
			return;
		}

		if (listNearby != null)
			DeviceListUpdate ( DeviceClass.Nearby );

		if (listMediator != null)
			DeviceListUpdate ( DeviceClass.Mediator );
	}


	void DeviceListUpdate ( final @DeviceClass.Value int listType )
	{
		synchronized (listAllObservers) {
			switch (listType) {
				case DeviceClass.All:
					if (listAllUpdate == 0) {
						listAllUpdate = 1;
					} else {
						if (listAllUpdate == 1)
							listAllUpdate = 2;
						return;
					}
					break;

				case DeviceClass.Nearby:
					if (listNearbyUpdate == 0) {
						listNearbyUpdate = 1;
					} else {
						if (listNearbyUpdate == 1)
							listNearbyUpdate = 2;
						return;
					}
					break;

				case DeviceClass.Mediator:
					if (listMediatorUpdate == 0) {
						listMediatorUpdate = 1;
					} else {
						if (listMediatorUpdate == 1)
							listMediatorUpdate = 2;
						return;
					}
					break;
				default:
					return;
			}
		}

		final Environs env = this;

		new Thread(new Runnable() {
			public void run(){
				boolean queryAgain;
				do {
					DeviceList.DeviceListUpdater(env, listType);

					queryAgain = false;

					synchronized (listAllObservers) {
						switch(listType) {
							case DeviceClass.All:
								listAllUpdate--;
								if (listAllUpdate > 0)
									queryAgain = true;
								break;

							case DeviceClass.Nearby:
								listNearbyUpdate--;
								if (listNearbyUpdate > 0)
									queryAgain = true;
								break;

							case DeviceClass.Mediator:
								listMediatorUpdate--;
								if (listMediatorUpdate > 0)
									queryAgain = true;
								break;
							default:
								return;
						}
					}
				}
				while (queryAgain);
			}
		}).start();
	}


	@SuppressWarnings ( "all" )
	void AddToListContainer ( @DeviceClass.Value int listType, ArrayList < DeviceInstance > vanished, ArrayList < DeviceInstance > appeared )
	{
		if (Utils.isDebug) Utils.Log ( 10, className, "AddToListContainer: listType [ " + listType + " ], vanished [ " + (vanished == null ? "0" : vanished.size ())
				+ " ], appeared [ " + (appeared == null ? "0" : appeared.size ()) + " ]" );

		Environs.ListContext listContext;

		switch ( listType ) {
			case DeviceClass.All :
				listContext  = contextAll;
				break;

			case DeviceClass.Nearby :
				listContext  = contextNearby;
				break;

			default:
				listContext  = contextMediator;
				break;
		}

		synchronized ( listContext )
		{
			if ( vanished != null ) {
				if ( listContext.vanished == null )
					listContext.vanished = vanished;
				else {
					listContext.vanished.addAll ( vanished );
				}
			}

			if ( appeared != null ) {
				if ( listContext.appeared == null ) {
					if (Utils.isDebug) Utils.Log ( 10, className, "AddToListContainer: Assigning appeared" );
					listContext.appeared = appeared;
				}
				else {
					if (Utils.isDebug) Utils.Log ( 10, className, "AddToListContainer: Appending appeared" );
					listContext.appeared.addAll ( appeared );
				}
			}
		}
	}


 	public static native int GetNetworkStatusN();
 	
 	static void UpdateNetworkStatus ( @NetworkConnection.Value int netStat )
 	{
        if  (netStat == NetworkConnection.Unknown ) {
			for (int i=0; i< ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE; i++) {
				Environs env = instances[i];
				if (env == null)
					continue;

				Activity activity = env.GetClient();
				if (activity != null) {
					ConnectivityManager cm = (ConnectivityManager) activity.getSystemService(Context.CONNECTIVITY_SERVICE);
					NetworkInfo networkInfo = cm.getActiveNetworkInfo();

					if (networkInfo != null) {
						netStat = Broadcasts.getNetStat ( networkInfo );

						//boolean noConnectivity = networkInfo.isConnected();
						//Utils.Log("[TRACE] UpdateNetworkStatus: by connman - networkInfo = [" + networkInfo
						//          + "] noConn = [" + noConnectivity + "]");
					}
				}
			}
        }
   	 
   	 	if ( netStat == NetworkConnection.Unknown )
   	 		netStat = NetworkConnection.NoInternet;

		SetNetworkStatusN ( netStat);
 	}

}
