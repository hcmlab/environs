package environs;
/**
 * Utility class that provides easier access
 * to Android toast messages or progress bar dialogs.
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
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Random;
import java.util.UUID;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import android.Manifest;
import android.accounts.Account;
import android.accounts.AccountManager;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.SparseArray;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

/** Utility class that provides easier access to Android toast messages or progress bar dialogs.
 * 
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 */
@SuppressWarnings("unused")
public class Utils
{
	/**
	 * isDebug Should be set to false for release builds in order to remove verbose logging code.
	 */
	static final boolean isDebug = true;

    //
    // Definitions of app configuration
    //

    //
    // Definitions of constants
    //
	
    /**
     * The application name is automatically retrieved as the application/window title of the main activity
     */
	private static final String className = "Utils. . . . . . . . . .";
    private static final String AppTag = "Environs";
    private static boolean initialized = false;

    public static int dataPort = 5901;
    public static int controlPort = 5901;
    public static String surfaceIP = "137.250.171.200";
    public static int recBufSize = 131071; //2000000;

	/**
	 * logLevel Determines the max. level of messages that will be logged.
	 *          Default max logLevel is 1
	 */
	public static int logLevel = 1;
    
	public static final int APILevel = android.os.Build.VERSION.SDK_INT;
	

	public static float density;
	public static boolean bigScreen;
	public static int imageSize;
	public static int width;
	public static int height;
	public static int width_mm;
	public static int height_mm;
	
	public static int cpus = 1;
	public static long memsize = -1L;
	public static boolean extMem = false;
	
    /**
     * Progressbar dialog managing
     */
    public static ProgressDialog progressDialog;
    public static int progressBarStatus;
    public static String progressBarMessage;  


	@SuppressWarnings ( "all" )
	static void Init(Context context)
	{
		if (initialized)
			return;

		String verb = "";
		
		DisplayMetrics metrics = context.getResources().getDisplayMetrics();
		density = metrics.density;
		verb += "Device density: " + density;
		
		int dpi;
		width = metrics.widthPixels;
		height = metrics.heightPixels;
		float xdpi = metrics.xdpi;
		float ydpi = metrics.ydpi;
		
	    if (width > height) {
	        dpi = (int) (width / metrics.densityDpi);
	        // We switch width/height here
	        
	        width = height;
	        height = metrics.widthPixels;
	        
	        xdpi = ydpi;
	        ydpi = metrics.xdpi;
	    }
	    else{
	        dpi = (height / metrics.densityDpi);
	    }
	    bigScreen = (dpi > 5);
	    
	    double x_in = width/xdpi;
	    width_mm = (int)(x_in * 25.4);
	    double y_in = height/ydpi;
	    height_mm = (int)(y_in * 25.4);

		verb += " | width [" + width + "] height [" + height + "] width_mm [" + width_mm + " mm]";
	    
	    if (bigScreen)
	    	imageSize = toPixels(64);
	    else
	    	imageSize = toPixels(48);

		verb += " | Device dpi: " + dpi;
	    
	    cpus = Runtime.getRuntime().availableProcessors();
		verb += " | We have " + cpus + " cpus!";
	    
	    try {
            Runtime info = Runtime.getRuntime();
            memsize = info.totalMemory();
        } catch (Exception e) {
            e.printStackTrace();
        }
		verb += " | Memory size: " + memsize;

		if (Utils.isDebug) Utils.Log ( 3, className, verb );
	    
	    extMem = memsize > 500000;
	    
	    try {
			/*if (Environs.GetIsReleaseBuildN ( ))
				logLevel = 1;
				*/

			DatagramSocket dataSocket = new DatagramSocket();

			//dataSocket.setReceiveBufferSize(2000000);
			dataSocket.setReceiveBufferSize(131071);
			recBufSize = dataSocket.getReceiveBufferSize();
			dataSocket.close();

			initialized = true;
		} catch (SocketException e) {
			e.printStackTrace();
		}
	}
	
	/*
	public static String getParam(String key)
	{
		Utils.Log("[TRACE] Utils.getParam key=" + key);

		String value = "";

		try {
			// Trust any certificate
			SSLContext ctx = SSLContext.getInstance("TLS");
    		ctx.Init(null, new TrustManager[] {
          		  new X509TrustManager() {
          		    public void checkClientTrusted(X509Certificate[] chain, String authType) {}
          		    public void checkServerTrusted(X509Certificate[] chain, String authType) {}
          		    public X509Certificate[] getAcceptedIssuers() { return new X509Certificate[]{}; }
          		  }
          		}, null);
    		HttpsURLConnection.setDefaultSSLSocketFactory(ctx.getSocketFactory());

			// Verify any host
    		HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
    			  public boolean verify(String hostname, SSLSession session) {
    			    return true;
    			  }
    			});

    		// Get value for key from parameter server
    		String ParamServer = "https://hcm-lab.de/projects/touch/index.php?p=surfacetile&t=g&k=";
    		URL url = new URL(ParamServer + key);

    		HttpsURLConnection con = (HttpsURLConnection)url.openConnection();

    		BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream()));

			String input;
			while ((input = br.readLine()) != null){
				value += input;
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}

		Utils.Log("[TRACE] Utils.getParam value=" + value);
		return value;
	}
	


	public static String setParam(String key, String value)
	{
		String ret = "";

		try {
			// Trust any certificate
			SSLContext ctx = SSLContext.getInstance("TLS");
    		ctx.Init(null, new TrustManager[] {
          		  new X509TrustManager() {
          		    public void checkClientTrusted(X509Certificate[] chain, String authType) {}
          		    public void checkServerTrusted(X509Certificate[] chain, String authType) {}
          		    public X509Certificate[] getAcceptedIssuers() { return new X509Certificate[]{}; }
          		  }
          		}, null);
    		HttpsURLConnection.setDefaultSSLSocketFactory(ctx.getSocketFactory());

			// Verify any host
    		HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
    			  public boolean verify(String hostname, SSLSession session) {
    			    return true;
    			  }
    			});
    		
    		//HttpGet httpGet = new HttpGet(uri.);
    		
    		// Get value for key from parameter server    		
    		String ParamServer = "https://hcm-lab.de/projects/touch/index.php?p=surfacetile&t=s&k=" + URLEncoder.encode(key, "UTF-8") 
    				+ "&v=" + URLEncoder.encode(value, "UTF-8");
    		
    		URL url = new URL(ParamServer);
    		HttpsURLConnection con = (HttpsURLConnection)url.openConnection();
    		BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream()));

			String input;
			while ((input = br.readLine()) != null){
				ret += input;
			}


		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		return ret;
	}
	*/
	

/*
	public static String customParam(String type, String key, String value)
	{
		String ret = "";

		try {
			// Trust any certificate
			SSLContext ctx = SSLContext.getInstance("TLS");
    		ctx.Init(null, new TrustManager[] {
          		  new X509TrustManager() {
          		    public void checkClientTrusted(X509Certificate[] chain, String authType) {}
          		    public void checkServerTrusted(X509Certificate[] chain, String authType) {}
          		    public X509Certificate[] getAcceptedIssuers() { return new X509Certificate[]{}; }
          		  }
          		}, null);
    		HttpsURLConnection.setDefaultSSLSocketFactory(ctx.getSocketFactory());

			// Verify any host
    		HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
    			  public boolean verify(String hostname, SSLSession session) {
    			    return true;
    			  }
    			});
    		
    		
    		// Get value for key from parameter server    		
    		String ParamServer = "https://hcm-lab.de/projects/touch/index.php?p=surfacetile&t=" + type + "&k=" + key;
    		if (value != null && !value.equals(""))
    			ParamServer += "&v=" + value;
    		
    		URL url = new URL(ParamServer);
    		HttpsURLConnection con = (HttpsURLConnection)url.openConnection();
    		BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream()));

			String input;
			while ((input = br.readLine()) != null){
				ret += input;
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}		
		return ret;
	}
	*/

	/*
	public static String getLocalIpAddressA()
    {
	      try {
	          for (Enumeration<NetworkInterface> en = 
	             NetworkInterface.getNetworkInterfaces();  
	              en.hasMoreElements();) {
	              NetworkInterface intf = en.nextElement();
	              for (Enumeration<InetAddress> enumIpAddr =
	              intf.getInetAddresses(); 
	               enumIpAddr.hasMoreElements();) {
	                  InetAddress inetAddress = enumIpAddr.nextElement();
	                  if (!inetAddress.isLoopbackAddress()) {
	                      return inetAddress.getHostAddress().toString();
	                  }
	              }
	          }
	      } catch (Exception ex) {
	         Log("[ERROR] ip Address " + ex.toString());
	      }
	      return null;
    }
	*/
	/*
	public static String getLocalIpAddress() {
	    try {
	        for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
	            NetworkInterface intf = en.nextElement();
	            for (Enumeration<InetAddress> enumIpAddr = intf
	                    .getInetAddresses(); enumIpAddr.hasMoreElements();)
	            {
	                InetAddress inetAddress = enumIpAddr.nextElement();
	                Log("[TRACE] ip:" + inetAddress);
	                Log("[TRACE] host:" + inetAddress.getHostAddress());

	                // for getting IPV4 format
	                String ipv4;
	                if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(ipv4 = inetAddress.getHostAddress())) {

	                    String ip = inetAddress.getHostAddress().toString();
	                    Log("[INFO] ip found " + ip);
	                    // return inetAddress.getHostAddress().toString();
	                    return ipv4;
	                }
	            }
	        }
	    } catch (Exception ex) {
	          Log("[ERROR] ip Address: " + ex.toString());
	    }
	    return null;
	}
	*/
	
	public static int toPixels(int dp)
	{
		//TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, sizeInDip, getResources().getDisplayMetrics());
		return Math.round((float)dp * density);
	}

    /**
     * Output a message through the android log debug output.
     *
     * @param msg A message to be logged.
     */
	public static void Log(String msg) { Log(1, msg); }

    /**
     * Output a message through the android log debug output.
     *  
     * @param className The className that the log message is related to.
     * @param msg       A message to be logged.
     */
	public static void Log(String className, String msg) {
		Log(1, className, msg);
	}

	
    /**
     * Output a message through the android log debug output if the current logLevel is lower than the given level.&nbsp;
     * The default logLevel is 3.
     *  
     * @param level The loglevel that this message is ordered to.
     * @param msg   A message to be logged.
     */
	public static void Log ( int level, String msg )
	{
        if (level == Environs.ERR_LEVEL) {
        	LogE ( msg );
        }
        else if (level == Environs.WARN_LEVEL) {
        	LogW ( msg );
        }
        else if (level <= logLevel)
        {
			if ( level < 2 )
				LogI(msg);
			else if (level < 3)
				LogT(msg);
			else
				LogV(msg);
        }
	}
	
    /**
     * Output a message through the android log debug output if the current logLevel is lower than the given level.&nbsp;
     * The default logLevel is 3.
	 * Note: No debug output through this method will be output in release builds.
     *  
     * @param level     The loglevel that this message is ordered to.
     * @param className The className that the log message is related to.
     * @param msg       A message to be logged.
     */
	public static void Log ( int level, String className, String msg )
	{
		if (level == Environs.ERR_LEVEL) {
			LogE(className + "." + msg );
		}
		else if (level == Environs.WARN_LEVEL) {
			LogW ( className + "." + msg );
		}
		else if (level <= logLevel)
			{
				if ( level < 2 )
					android.util.Log.i ( AppTag, "[INFO]   " + className + " " + msg );
				else if ( level < 3 )
					android.util.Log.d ( AppTag, "[TRACE]  " + className + " " + msg );
				else
					android.util.Log.d ( AppTag, "[VERB]   " + className + " " + msg);
			}
	}

	public static void Log1 ( String className, String msg ) {
		Log ( 1, className, msg );
	}

	public static void Log2 ( String className, String msg ) {
		Log ( 2, className, msg );
	}


	/*
	static String GetTimeString()
	{
		final String format = "EEE MMM dd kk:mm:ss: ";

		SimpleDateFormat sdf = new SimpleDateFormat(format);

		return sdf.format(new Date ());
	}*/


    /**
     * Output an error message through the android log debug output.
     *  
     * @param msg An error message to be logged.
     */
	public static void LogE ( String msg ) {
		android.util.Log.e ( AppTag, "[ERROR] -- E --> " + msg);
	}

    /**
     * Output an error message through the android log debug output.
     *  
     * @param className The className that the error message is related to.
     * @param msg       An error message to be logged.
     */
	public static void LogE ( String className, String msg ) {
		Utils.LogE ( className + " " + msg );
	}

    /**
     * Output a warning message through the android log debug output.
     *  
     * @param msg   A warning to be logged.
     */
	public static void LogW ( String msg ) {
		android.util.Log.w ( AppTag, "[WARN]   " + msg );
	}

    /**
     * Output a warning message through the android log debug output.
     *  
     * @param className The className that the warning is related to.
     * @param msg       A warning to be logged.
     */
	public static void LogW ( String className, String msg ) {
		Utils.LogW ( className + " " + msg );
	}

	/**
	 * Output message through the android log debug output (every).
	 * logLevel 0
	 *
	 * @param msg   A message to be logged.
	 */
	/*public static void LogA ( String msg ) {
		android.util.Log.i ( AppTag, "[INFO]   " + msg );
	}*/

	/**
	 * Output an info level message through the android log debug output.
	 * logLevel 1
	 *
	 * @param msg   An info message to be logged.
	 */
	public static void LogI ( String msg ) {
		android.util.Log.i ( AppTag, "[INFO]   " + msg );
	}

	/**
	 * Output a trace level message through the android log debug output.
	 * logLevel 2
	 *
	 * @param msg   A trace message to be logged.
	 */
	public static void LogT ( String msg ) {
		android.util.Log.d ( AppTag, "[TRACE]   " + msg );
	}

	/**
	 * Output a verbose level message through the android log debug output.
	 *
	 */
	public static void LogV ( String msg ) {
		android.util.Log.d(AppTag, "[VERB]   " + msg);
	}


	static String buildDeviceName ( Context ctx ) {
		try {
			String deviceName = android.os.Build.MODEL;

			if (!Environs.useAccounts) {
				return deviceName;
			}

			AccountManager manager = AccountManager.get ( ctx );
			Account[] accs = manager.getAccountsByType ( "com.google" );

			for ( Account account : accs ) {
				String[] splits = account.name.split("@");
				if ( splits.length < 2 )
					continue;

				deviceName = deviceName + "." + splits [ 0 ];
				if (Utils.isDebug) Utils.Log ( 4, className, "buildDeviceName: DeviceName = " + deviceName );

				return deviceName;
			}
		} catch (Exception e) {
			Utils.LogW ( className, "buildDeviceName: Failed to build device name!");
		}

		return null;
	}


    /**
     * Queries whether the application runs on a device that supports API &gt;= 14 (ICS) .
     *
     * @return APIisLGT14
     */
	public static boolean isNewApi()
	{
		return (APILevel >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH);
	}


	private static boolean isNetTypeConnected(Context context, int type)
	{
		ConnectivityManager manager = ((ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE));

		if (manager != null) {
			NetworkInfo ni = manager.getNetworkInfo(type);
			if ( ni != null ) {
				return ni.isAvailable();
			}
		}
		return false;
	}

    /**
     * Determines whether the device is currently connected to the internet over WiFi network.
     *
     * @param context The current activity/context.
     * @return connectStatus
     */
	public static boolean isWifiConnected (Context context) {
        return isNetTypeConnected ( context, ConnectivityManager.TYPE_WIFI );
	}


    /**
     * Determines whether the device is currently connected to the internet over mobile data.
     *
     * @param context The current activity/context.
     * @return connectStatus
     */
	public static boolean isMobileConnected (Context context) {
		return isNetTypeConnected ( context, ConnectivityManager.TYPE_MOBILE );
	}


    /**
     * Determines whether the device is currently connected to the internet.
     *
     * @param context The current activity/context.
     * @return connectStatus
     */
	 public static boolean isInternetConnected(Context context)
	 {
        ConnectivityManager manager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
          if (manager != null)
          {
              NetworkInfo[] info = manager.getAllNetworkInfo();
              if (info != null)
                  for (NetworkInfo i : info)
                      if (i.getState() == NetworkInfo.State.CONNECTED)
                      {
                          return true;
                      }
 
          }
          return false;
	}

    // Google project id
    static final String PROJECT_ID = "1234567890";
 
    static final String EXTRA_MESSAGE = "msg";
    /*
    private static final int MAX_ATTEMPTS = 5;
    private static final int BACKOFF_MILLI_SECONDS = 2000;
    private static final Random random = new Random();
 */

    
    /**
     * Register this account/device pair within the server.
     *
     */
    /*
    static void register(final Context context, final String regId) {
        Log("[TRACE] registering device (regId = " + regId + ")");
               
 
        long backoff = BACKOFF_MILLI_SECONDS + random.nextInt(1000);
        
        // Once GCM returns a registration id, we need to register on our server
        // As the server might be down, we will retry it a couple
        // times.
        for (int i = 1; i <= MAX_ATTEMPTS; i++)
        {
            Log("[TRACE] Utils.register: Attempt #" + i + " to register");
            
            String ret = setParam ( Environs._tagID + "_pn", "gcm" + regId);
            if (!ret.equals("0")) {
            	Log(ret);
            	return;
            }
            // Here we are simplifying and retrying on any error; in a real
            // application, it should retry only on unrecoverable errors
            // (like HTTP error code 503).
            Log("[ERROR] Utils.register:Failed to register on attempt " + i);
            if (i == MAX_ATTEMPTS) {
                break;
            }
            
            try {
                Log("[TRACE] Utils.register: Sleeping for " + backoff + " ms before retry");
                Thread.sleep(backoff);
            } catch (InterruptedException e1) {
                // Activity finished before we complete - exit.
                Log("[WARNING] Utils.register: Thread interrupted: abort remaining retries!");
                Thread.currentThread().interrupt();
                return;
            }
            // increase backoff exponentially
            backoff *= 2;        	
        }
    }
    */

    static String configFileName = "environs.conf";
    
    static boolean SaveIDToStorage(int deviceID)
    {
    	// Check whether we are allowed to save to external storage?
    	try 
    	{
    		String storageDir = Environment.getExternalStorageDirectory().getAbsolutePath();

    		File file = new File(storageDir + File.separator + configFileName);
    		FileOutputStream fos = new FileOutputStream ( file );

    		fos.write(("id: 0x" + Integer.toHexString(deviceID)).getBytes());
    		fos.flush();
    		fos.close();
    	}
    	catch (Exception e) {
    		return false;
    	}
    	return true;
    }


	@SuppressWarnings ( "ResultOfMethodCallIgnored" )
    static int LoadIDFromStorage()
    {
    	try 
    	{
    		String storageDir = Environment.getExternalStorageDirectory().getAbsolutePath();

    		File file = new File(storageDir + File.separator + configFileName);    		
    		FileInputStream fis = new FileInputStream ( file );
    		
    		byte [] buffer = new byte[32];
    		fis.read ( buffer );
    		fis.close ( );
    		
    		String line = new String(buffer);
			if (line.startsWith("id:")) {
				String deviceID = line.substring(6).trim();
				return Integer.parseInt(deviceID, 16);
			}
    	}
    	catch (Exception e) {
    		return 0;
    	}
    	return 0;
    }


	static boolean CheckManifestWarning(String option, String perm)
	{
		Utils.LogW ( className, "CheckManifestWarning: " + (option != null ? option + " is enabled, but " : "")
				+ "AndroidManifest.xml is missing the permission " + perm);
		return false;
	}
    
    
    static boolean CheckManifest(Environs env, Context context)
    {
		if (Utils.isDebug) Utils.Log ( 6, className, "CheckManifest");

    	PackageManager pm = context.getPackageManager();
    	int hasPerm;
    	String name = context.getPackageName();

		String perm = android.Manifest.permission.INTERNET;

    	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			return CheckManifestWarning(null, perm);

		if (Environs.useAccounts) {
			perm = Manifest.permission.GET_ACCOUNTS;

			if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.useAccounts = CheckManifestWarning("useAccounts", perm);
		}

		perm = android.Manifest.permission.ACCESS_NETWORK_STATE;

    	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			return CheckManifestWarning(null, perm);

    	if (Environs.useWiFiCheck) {
			perm = android.Manifest.permission.ACCESS_WIFI_STATE;

        	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.useWiFiCheck = CheckManifestWarning("useWiFiCheck", perm);
    	}

		perm = android.Manifest.permission.ACCESS_FINE_LOCATION;

		if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			Environs.useLocation = CheckManifestWarning("useLocation", perm);

		if (Environs.useWiFiCheck) {
			perm = android.Manifest.permission.CHANGE_WIFI_STATE;

			if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.useWiFiCheck = CheckManifestWarning("useWiFiCheck", perm);
		}
    	
    	if (Environs.GetUsePushNotificationsN ( env.hEnvirons )) {
			perm = android.Manifest.permission.VIBRATE;

        	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.SetUsePushNotificationsN ( env.hEnvirons, CheckManifestWarning("usePushNotifications", perm) );
    	}    	
    	
    	if (Environs.useWakup) {
			perm = android.Manifest.permission.WAKE_LOCK;

        	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.useWakup = CheckManifestWarning("useWakup or GCM", perm);
    	}

		if (Environs.useWakup) {
			perm = android.Manifest.permission.DISABLE_KEYGUARD;

			if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
				Environs.useWakup = CheckManifestWarning("useWakup or GCM", perm);
		}

		if (Utils.isDebug) Utils.Log ( 6, className, "CheckManifest: done");
    	return true;
    }
    
    
    static String GetAppID(Context ctx)
    {
    	String appID = null;
    	 
        File file = new File(ctx.getFilesDir(), "Environs.appID");
        try {
            if (!file.exists())
            	CreateAppID(file);

            RandomAccessFile f = new RandomAccessFile(file, "r");

    		//Log ( 4, className, "GetAppID: length " + f.length() );
    		
            byte[] bytes = new byte[(int) f.length()];
            f.readFully(bytes);
            f.close();
            
            appID = new String(bytes);
    		//Log ( 4, className, "GetAppID: appID " + appID );
            
        } catch (Exception e) {
        	e.printStackTrace();
        }
        if ( appID != null )
        	return appID;
        
        return "Environs-Empty-Template-UID-..-" + new Random().nextInt();
    }
    

    static void CreateAppID(File file) throws IOException
    {
		//Log ( 4, "CreateAppID" );
		
        FileOutputStream out = new FileOutputStream(file);
        String id = UUID.randomUUID().toString();
		if (Utils.isDebug) Utils.Log ( 4, className, "CreateAppID: " + id );
        out.write(id.getBytes());
        out.close();
    }


	@SuppressWarnings ( "SimplifiableIfStatement" )
    static boolean CheckManifestGCM(Context context)
    {
		if (!Environs.useWakup)
			return CheckManifestWarning("GCM", "android.permission.WAKE_LOCK");

    	PackageManager pm = context.getPackageManager();
    	int hasPerm;
    	String name = context.getPackageName();

		String perm = android.Manifest.permission.VIBRATE;

    	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			return CheckManifestWarning("GCM", perm);

    	perm = name + ".permission.C2D_MESSAGE";

    	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			return CheckManifestWarning("GCM", perm);

		perm = "com.google.android.c2dm.permission.RECEIVE";

    	if (pm.checkPermission ( perm, name ) != PackageManager.PERMISSION_GRANTED)
			return CheckManifestWarning("GCM", perm);
    	
    	return true;
    }


	@SuppressWarnings ( "all" )
	static void RunSync(Activity act, Runnable work)
	{
		synchronized ( work )
		{
			act.runOnUiThread ( work );

			try {
				work.wait ();
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
			}
		}
	}

	static Toast lastToast;

 	/** 
 	 * Show a text message via Android toast message / notification system. 
 	 * 
     * @param msg A message to be shown in a toast dialog.
     */
	static void Message ( int hInst, final String msg )
	{
		final Activity act = Environs.GetActivity ( hInst );
		if (act == null )
			return;

		if (lastToast != null)
			lastToast.cancel ();

		act.runOnUiThread(new Runnable() {        	
            public void run() {
				lastToast = Toast.makeText(act, msg, Toast.LENGTH_LONG);

				if (Utils.isDebug) Utils.Log ( 8, className, "Message: showing toast - " + msg );
            	lastToast.show();
            }
        });
	}


	public static void Message ( final Activity act, final String msg )
	{
		if (Utils.isDebug) Utils.Log ( 8, className, "Message: " + msg );

		if (act == null )
			return;

		if (lastToast != null)
			lastToast.cancel ();

		act.runOnUiThread(new Runnable() {
			public void run() {
				lastToast = Toast.makeText(act, msg, Toast.LENGTH_LONG);

				if (Utils.isDebug) Utils.Log ( 8, className, "Message: showing toast - " + msg );
				lastToast.show();
			}
		});
	}

	
	static boolean EnableWiFi(int hInst)
	{
		if (Utils.isDebug) Utils.Log ( 8, className, "EnableWiFi" );

		Activity act = Environs.GetActivity ( hInst );
		if (act == null){
			return false;
		}
				
        WifiManager wifiManager = (WifiManager)act.getSystemService(Context.WIFI_SERVICE);
		if ( wifiManager == null )
			return false;

        int state = wifiManager.getWifiState();
    	if (!wifiManager.isWifiEnabled() || state != WifiManager.WIFI_STATE_ENABLED)
    	{
			if (Utils.isDebug) Utils.Log ( 8, className, "EnableWiFi: Enabling ..." );

    		if (Environs.showConnectionToasts)
    			Utils.Message(act, "Enabling WiFi...");
    		
            if (!wifiManager.setWifiEnabled(true))
            {
            	String msg = "ERROR! Failed to enable WiFi!";
        		if (Environs.showConnectionToasts)
        			Utils.Message(act, msg);
        		
        		Utils.LogE ( className, msg );
            	return false;                	
            }
    	}
    	
    	int trials = 0;
    	while (trials < 5) {
        	if (Utils.isWifiConnected(act))
        		break;

			String msg = "Waiting (" + trials + ") for WiFi to connect...";

			if (Utils.isDebug) Utils.Log ( 10, className, "EnableWiFi: " + msg );

    		if (Environs.showConnectionToasts)
    			Utils.Message(act, msg);
    		
        	try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
        	trials++;        		
    	}
    	
    	if (trials >= 5) {
        	String msg = "ERROR! WiFi connecting to a network lasted too long!";
    		if (Environs.showConnectionToasts)
    			Utils.Message(act, msg);
    		
    		Utils.LogE ( className, msg );
        	return false;        		
    	}
    	
    	trials = 0;
    	while (trials < 5) {
        	if (Utils.isInternetConnected(act))
        		break;

			String msg = "Waiting (" + trials + ") for Internet to connect...";

			if (Utils.isDebug) Utils.Log ( 10, className, "EnableWiFi: " + msg );

    		if (Environs.showConnectionToasts)
    			Utils.Message(act, msg);
    		
        	try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
        	trials++;        		
    	}
    	if (trials >= 5) {
        	String msg = "ERROR! Connecting to the internet lasted too long!";
    		if (Environs.showConnectionToasts)
    			Utils.Message(act, msg);
    		
    		Utils.LogE ( className, msg );
        	return false;        		
    	}
    	
		return true;
	}


	static String GetSSID(int hInst, boolean desc)
	{
		Activity act = Environs.GetActivity ( hInst );
		if (act == null){
			return "Error";
		}

		WifiManager wifiManager = (WifiManager)act.getSystemService(Context.WIFI_SERVICE);
		WifiInfo info = wifiManager.getConnectionInfo();
		if (info == null)
			return "Error";

		if ( desc )
			return info.getSSID() + " (" + info.getRssi() + " dB)";
		return info.getSSID();
	}

    /**
     * Request fullscreen UI feature and always on display - IMPORTANT This method must be called BEFORE calling setContentView()!!!
     *  
     * @param activity The activity for which the fullscreen feature should be enabled.
     */
	public static void RequestFullscreenAlwaysOn(Activity activity)
	{
		if (activity == null) {
			Utils.LogW("RequestFullscreenAlwaysOn: invalid Activity argument.");
			return;
		}
		RequestFullscreen(activity);

        // Request navigation bar to hide		
		RequestHiddenUIBars(activity.getWindow().getDecorView());
		
		// Request always on window
		activity.getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}
	

    /**
     * Request fullscreen UI feature - IMPORTANT This method must be called BEFORE calling setContentView()!!!
     *  
     * @param activity The activity for which the fullscreen feature should be enabled.
     */
	public static void RequestFullscreen(Activity activity)
	{
		if (activity == null) {
			Utils.LogW("RequestFullscreen: invalid Activity argument.");
			return;
		}

		activity.requestWindowFeature(Window.FEATURE_NO_TITLE);
		activity.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // Request navigation bar to hide		
		RequestHiddenUIBars(activity.getWindow().getDecorView());
	}
	

    /**
     * Request to hide the UI bars if API level allows this feature.
     *  
     * @param view The view for which the feature should be enabled.
     */
	@SuppressWarnings("deprecation")
	@SuppressLint("NewApi")
	public static void RequestHiddenUIBars(View view)
	{
		if (view == null) {
			Utils.LogW("RequestHiddenUIBars: invalid View argument.");
			return;
		}

        // Request navigation bar to hide		
		
		if (APILevel >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
			view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);			
		}

		// deprecated as of API 11
		view.setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
        
	}
    
	
    private static Context tempContext;
    private static int progressMax;
    
    public static void createProgress(Context context, int max) {
    	tempContext = context;
    	progressMax = max;
    	
    	if (progressDialog != null)
    		progressDialog.dismiss();
    	
    	/*Activity act = Environs.GetClient();
    	if (act == null)
    		return;
		*/
		Activity act = (Activity)context;

    	act.runOnUiThread(new Runnable() {
			public void run() {
				progressDialog = new ProgressDialog(tempContext);
				progressDialog.setMessage(" ");
				progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
				progressDialog.setProgress(0);
				progressDialog.setMax(progressMax);
				progressDialog.setCanceledOnTouchOutside(false);
				progressDialog.setCancelable(false);
				progressDialog.show();
			}
		});
    }
    
    public static void updateProgress(int status) {
    	if (progressDialog == null)
    		return;   	

    	Message msg = progressBarHandler.obtainMessage();
        Bundle b = new Bundle();
        b.putInt("i", status);
        msg.setData(b);
        progressBarHandler.sendMessage(msg);        
    }
    
    public static void updateProgress(String text) {
    	if (progressDialog == null)
    		return;

    	Message msg = progressBarHandler.obtainMessage();
        Bundle b = new Bundle();
        b.putString("t", text);
        msg.setData(b);
        progressBarHandler.sendMessage(msg);   
    }
    
    public static void updateProgressMax(int status) {
    	if (progressDialog == null)
    		return;

    	Message msg = progressBarHandler.obtainMessage();
        Bundle b = new Bundle();
        b.putInt("m", status);
        msg.setData(b);
        progressBarHandler.sendMessage(msg);
    }


	@SuppressWarnings ( "all" )
    public static final Handler progressBarHandler = new Handler() {
        public void handleMessage(Message msg) {
            // Get the current value of the variable total from the message data
            // and update the progress bar.
        	Bundle b = msg.getData();
        	String text = b.getString("t");
        	if (text != null) {
        		if (progressDialog != null)
        			progressDialog.setMessage(text);
        		return;
        	}
        	        	
            int v = b.getInt("m");
            if (v > 0){
        		if (progressDialog != null)
        			progressDialog.setMax(v);
        		return;
        	}
         	
            v = b.getInt("i");
    		if (progressDialog != null)
    			progressDialog.setProgress(v);
        }
    };


    public static void dismissProgress(Activity act)
    {
    	if (act == null)
    		return;
		
    	act.runOnUiThread(new Runnable() {
			public void run() {
				if (progressDialog != null)
					progressDialog.dismiss();
				progressDialog = null;
			}
		});
    }

    
	private static int hNB = 0;
	
    static int GetNaviBarHeigth(Activity act)
    {
    	if (hNB == 0) {
    		if ( act == null )
    			return 96;
    		
    		Resources resources = act.getResources();
    		if ( resources == null )
    			return 96;
    		
    		int resourceId = resources.getIdentifier("navigation_bar_height", "dimen", "android");
    		if (resourceId > 0) {
    			hNB = resources.getDimensionPixelSize(resourceId);
				if (Utils.isDebug) Utils.Log ( 5, className, "GetNaviBarHeigth: Height of navigation bar [" + hNB + "]");
    		}    		
    	}
    	return hNB;
    }

    @SuppressLint("NewApi")
	static void CheckForCodecs()
    {
		if (Utils.isDebug) Utils.Log ( 6, className, "CheckForCodecs");

		Encoder.colorFormatsPreferred = -1;
		Encoder.colorFormatsSupported.clear();

    	if (APILevel < 16) {
			if (Utils.isDebug) Utils.Log ( 3, className, "CheckForCodecs: API level [" + APILevel + "] does not support createEncoderByType.");
    		return;
    	}
 		
    	MediaCodec codec = null;
		try {
			codec = MediaCodec.createEncoderByType("video/avc");
		} catch (Exception e) {
			e.printStackTrace();
		}
		if ( codec == null ) {
    		Utils.Log2 ( className, "CheckForCodecs: No h264 encoder available!");
    		return;
    	}

		if (APILevel < 18) {
			if (Utils.isDebug) Utils.Log ( 3, className, "CheckForCodecs: API level [" + APILevel + "] does not support getCodecInfo.");
			return;
		}

		try {
			MediaCodecInfo info = codec.getCodecInfo();

			String[] types = info.getSupportedTypes ( );
			for (String type : types) {
				if (Utils.isDebug) Utils.Log ( 5, className, "CheckForCodecs: support [" + type + "]");

				CodecCapabilities caps = info.getCapabilitiesForType (type);
				if (caps == null)
					continue;

				for (int k=0; k<caps.colorFormats.length; k++) {
					if (Utils.isDebug) Utils.Log ( 7, className, "CheckForCodecs: support color format [" + caps.colorFormats[k] + "]");

					Encoder.colorFormatsSupported.add(caps.colorFormats[k]);

					/// 19 = COLOR_FormatYUV420Planar - preferred
					if ( caps.colorFormats[k] == 19 ) {
						Encoder.colorFormatsPreferred = caps.colorFormats[k];
						if (Utils.isDebug) Utils.Log ( 5, className, "CheckForCodecs: Encoder seems to support [" + type + "]");
					}

					/// 21 = COLOR_FormatYUV420SemiPlanar
					if ( Encoder.colorFormatsPreferred < 0 && caps.colorFormats[k] == 21 ) {
						Encoder.colorFormatsPreferred = caps.colorFormats[k];
						if (Utils.isDebug) Utils.Log ( 5, className, "CheckForCodecs: Encoder seems to support [" + type + "]");
					}

					/// 2130708361 = COLOR_FormatSurface
					//if (encoderColorFormatsPreferred != 0)
					//	continue;
				}
			}
		}
		catch ( Exception ex ) {
			ex.printStackTrace ();
		}

		if (Utils.isDebug) Utils.Log ( 6, className, "CheckForCodecs: done");
    }
    
    
    static ByteBuffer SHAHashCreate ( byte[] msg )
    {                
    	try
    	{
            MessageDigest digest = MessageDigest.getInstance("SHA-512");
            if (digest == null)
            	return null;
            
            digest.reset();

            byte[] blobData = digest.digest(msg);
            if (blobData == null)
            	return null;

            ByteBuffer blob = Environs.Java_malloc(blobData.length, false);
            if ( blob == null )
            	return null;
            blob.put(blobData, 0, blobData.length);
            /*
       		StringBuffer sb = new StringBuffer();
            for (int i = 0; i < blobData.length; i++){
              sb.append(Integer.toString((blobData[i] & 0xff) + 0x100, 16).substring(1));
            }
            Utils.Log (5, className, "SHAHashCreate: blob [" + sb.toString() + "]" );
            */
            
    		return blob;
    	}
    	catch(Exception ex)
    	{
            Utils.LogE ( className, "SHAHashCreate: Failed with exception [" + ex.getMessage() + "]");
    	}
    	return null;
    }

    
    static String toString (byte [] a) {
    	if (a == null)
    		return "null";
    	String r = "";
    	for (byte b : a)
    		r += (char)b;
    	return r;
    }

    
    static String toHexString (byte [] a) {
    	if (a == null)
    		return "null";
    	String r = "";
    	for (byte b : a)
    		r += String.format("%02x", b & 0xff) + " ";
    	return r;
    }


	@SuppressWarnings ( "all" )
	private static SparseArray<Cipher> pubCerts = new SparseArray<Cipher>();

	static void ReleaseCert ( int deviceID )
    {    	
		pubCerts.remove(deviceID);
    }


	@SuppressWarnings ( "all" )
	@SuppressLint("TrulyRandom")
	static ByteBuffer EncryptMessage ( int deviceID, int certProp, byte[] cert, byte[] msg )
    {
		if (Utils.isDebug) Utils.Log ( 5, className, "EncryptMessage");

    	int format = (certProp >> 16) & 0xFF;

    	Exception p1 = null, p2 = null;
    	ByteBuffer cipherText = null;
    	byte[] cipherData = null;
    	Cipher cipher = pubCerts.get(deviceID);
    	
    	try
    	{
        	do
        	{
        		if (cipher == null) {
            		//Utils.Log ( 4, className, "EncryptMessage: try importing certificate [" + toString(cert) + "]");
                    
                    //KeyFactory kf = KeyFactory.getInstance ( "RSA", "BC" );
                    KeyFactory kf = KeyFactory.getInstance ( "RSA" );
                    if ( kf == null ) {
                    	Utils.LogE ( className, "EncryptMessage: Failed to get RSA keyFactory." ); break;
                    }
                    PublicKey pubKey = null;
            		
                    if ( format == 'd' ) {
                    	/// Let's try binary DER certificate...
                    	try {
                			X509EncodedKeySpec spec = new X509EncodedKeySpec(cert);   
                            pubKey = kf.generatePublic(spec);                		
                    	}
                    	catch (Exception e1) {
                        	//Utils.Log ( 0, className, "EncryptMessage: Binary parse Exception [" + e1.getMessage() + "]");
                        	p1 = e1;
                    	}
                    	
                    	/*if ( pubKey == null ) {
                        	/// Let's try DER pub key
                        	 /// http://goldenpackagebyanuj.blogspot.de/2013/10/RSA-Encryption-Descryption-algorithm-in-java.html
                        	  /// there is a modulus/exponent parser...
                        	try {
                        		RSAPublicKeySpec spec = new RSAPublicKeySpec();
                                pubKey = kf.generatePublic(spec);                		
                        	}
                        	catch (Exception e1) {
                            	Utils.Log ( 0, "Utils.EncryptMessage: Binary parse Exception [" + e1.getMessage() + "]");
                            	//e1.printStackTrace();
                        	}                		
                    	}
                    	*/
                    }
                                    
                    if ( pubKey == null ) {
                    	/// Let's try cert file...
                    	try {
                            ByteArrayInputStream bis = new ByteArrayInputStream(cert);
                            CertificateFactory cf = CertificateFactory.getInstance("X.509");
                            X509Certificate certs = 
                                (X509Certificate) cf.generateCertificate (bis);
                            pubKey = certs.getPublicKey();                 		
                    	}
                    	catch (Exception e1) {
                        	//Utils.LogE ( className, "EncryptMessage: File parse Exception [" + e1.getMessage() + "]");
                        	p2 = e1;
                    	}
                    }        		

                    if ( pubKey == null ) {
                    	Utils.LogE("Utils.EncryptMessage: Failed to generate public key." );
                    	if ( p1 != null ) p1.printStackTrace();
                    	if ( p2 != null ) p2.printStackTrace();
                    	break;
                    }
                	//Utils.Log ( 5, "EncryptMessageCert: pubKey [" + pubKey.toString() + "]" );
                	//Utils.Log ( 5, "EncryptMessageCert: getAlgorithm [" + pubKey.getAlgorithm() + "]" );
                	//Utils.Log ( 5, "EncryptMessageCert: getFormat [" + pubKey.getFormat() + "]" );
                	//Utils.Log ( 5, "EncryptMessageCert: getEncoded [" + toString(pubKey.getEncoded()) + "]" );
                	//Utils.Log ( 5, "EncryptMessageCert: getEncodedLength [" + pubKey.getEncoded().length + "]" );
          
                	String pad = "";
                	if ( (certProp & Environs.ENVIRONS_CRYPT_PAD_OAEP) != 0 )
                		pad = "OAEPWithSHA-1AndMGF1Padding";
                	else if ( (certProp & Environs.ENVIRONS_CRYPT_PAD_PKCS1SHA1) != 0 )
                		pad = "OAEPWithSHA-256AndMGF1Padding";
                	else if ( (certProp & Environs.ENVIRONS_CRYPT_PAD_PKCS1) != 0 )
                		pad = "PKCS1Padding";
                	else
                		pad = "NoPadding";
					if (Utils.isDebug) Utils.Log ( 5, className, "EncryptMessage: Using padding [" + pad + "]");
                	
                    //Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA256AndMGF1Padding");
                	
                    cipher = Cipher.getInstance("RSA/ECB/" + pad);
                    if ( cipher != null ) {

                        cipher.init(Cipher.ENCRYPT_MODE, pubKey);            

                		//Utils.Log ( 0, className, "EncryptMessage: try encrypting message [" + toString(msg) + "]");
                		//Utils.Log ( 5, className, "EncryptMessage: Cipher block size is [" + cipher.getBlockSize() + "]");
                        
                    	pubCerts.put(deviceID, cipher);
                    }
        		}
        		
                if ( cipher == null ) {
                	Utils.LogE ( className, "EncryptMessage: Failed to get RSA cipher." ); break;
                }
        		
                synchronized ( cipher ) {
                    cipher.update(msg);
                    cipherData = cipher.doFinal();                  	
                }
                
                //byte[] cipherData = cipher.doFinal(msg);  		
                if ( cipherData == null ) {
                	Utils.LogE ( className, "EncryptMessage: Failed to encrypt message." ); break;
                }
                //Utils.Log ( 0, className, "EncryptMessage: Encrypted message size [" + cipherData.length + "] msg [" + toHexString(cipherData) + "]" );
                
                /*byte[] cd = Base64.decode(cipherData, Base64.DEFAULT );
                if ( cd == null ) {
                	Utils.Log ( 0, className, "EncryptMessage: Failed to encrypt message." ); break;
                }
                else
                	Utils.Log ( 0, className, "EncryptMessage: Encrypted message size [" + cd.length + "] msg [" + toString(cd) + "]" );
                */
                
                cipherText = ByteBuffer.allocateDirect ( cipherData.length );
                if ( cipherText == null ) {
                	Utils.LogE ( className, "EncryptMessage: Failed to allocate ByteBuffer for cipher." ); break;
                }
        		
                ByteOrder nativeOrder = ByteOrder.nativeOrder();
        		if ( cipherText.order() != nativeOrder )
        			cipherText.order ( nativeOrder );
        		
                cipherText.put(cipherData);                
        	}
        	while ( false );
    	}
    	catch (Exception ex)
    	{
    		Utils.LogE ( className, "EncryptMessage: Exception [" + ex.getMessage() + "]");
        	ex.printStackTrace();
    	}
        
        return cipherText;      	
    }

	
	static ByteBuffer GenerateCertificate ()
	{		
		try {
	          KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
	          SecureRandom random = SecureRandom.getInstance("SHA1PRNG");
	          keyGen.initialize(Environs.ENVIRONS_DEVICES_KEYSIZE, random);

	          KeyPair pair = keyGen.generateKeyPair();
	          PrivateKey priv = pair.getPrivate();
	          //Utils.Log ( 0, className, "GenerateCertificate: PrivateKey [" + priv.toString() + "]");
	          
	          PublicKey pub = pair.getPublic();
	          //Utils.Log ( 0, className, "GenerateCertificate: PublicKey [" + pub.toString() + "]");

	          byte [] privEnc = priv.getEncoded();
	          byte [] pubEnc = pub.getEncoded();
	          int reqLength = privEnc.length + pubEnc.length + 8;
	          
	          ByteBuffer output = Environs.Java_malloc(reqLength, false);
	          if ( output == null )
	        	  return null;
	          
	          output.putInt(privEnc.length);
	          output.put(privEnc);

	          output.putInt(pubEnc.length | ('d' << 16));
	          output.put(pubEnc);
	          return output;

		} catch (Exception e) {
	         e.printStackTrace();
	    }		
		return null;
	}


	@SuppressWarnings ( "all" )
	static ByteBuffer DecryptMessage ( byte[] key, byte[] msg )
    {
		if (Utils.isDebug) Utils.Log ( 5, className, "DecryptMessage");
    	Exception p1 = null, p2 = null;
    	ByteBuffer plainText = null;
    	char padding = 'o';
    	try
    	{
        	do
        	{
        		//Utils.Log ( 0, "Utils.DecryptMessage: try importing private key [" + toString(key) + "]");
                
                KeyFactory kf = KeyFactory.getInstance ( "RSA" );
                if ( kf == null ) {
                	LogE ( className, "DecryptMessage: Failed to get RSA keyFactory." ); break;
                }
                PrivateKey privKey = null;

            	try {
        			X509EncodedKeySpec spec = new X509EncodedKeySpec(key);   
        			privKey = kf.generatePrivate(spec);                		
            	}
            	catch (Exception e1) {
                	//Utils.LogE ( className, "DecryptMessage: Exception [" + e1.getMessage() + "]");
                	//e1.printStackTrace();
                	p1 = e1;
            	}

            	if ( privKey == null) {
                	try {
                        PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(key);
                        privKey = kf.generatePrivate(spec);               		
                	}
                	catch (Exception e1) {
                    	//Utils.LogE ( className, "DecryptMessage: Exception [" + e1.getMessage() + "]");
                    	p2 = e1;
                	}            		
            	}

                if ( privKey == null ) {
                	Utils.LogE(className, "DecryptMessage: Failed to generate private key.");
                	if ( p1 != null ) p1.printStackTrace();
                	if ( p2 != null ) p2.printStackTrace();
                	break;
                }
      
            	String pad = "";
            	if ( padding == 'o' )
            		pad = "OAEPWithSHA-1AndMGF1Padding";
            	else if ( padding == 'r' )
            		pad = "OAEPWithSHA-256AndMGF1Padding";
            	else if ( padding == 'p' )
            		pad = "PKCS1Padding";
            	else
            		pad = "NoPadding";
				if (Utils.isDebug) Utils.Log ( 4, className, "DecryptMessage: Using padding [" + pad + "]");
            	
                Cipher cipher = Cipher.getInstance("RSA/ECB/" + pad);
                if ( cipher == null ) {
                	Utils.LogE(className, "DecryptMessage: Failed to get RSA cipher." ); break;
                }

                cipher.init(Cipher.DECRYPT_MODE, privKey);            
        		
                byte[] cipherData = cipher.doFinal(msg);  		
                if ( cipherData == null ) {
                	Utils.LogE(className, "DecryptMessage: Failed to encrypt message."); break;
                }
                      
                plainText = ByteBuffer.allocateDirect ( cipherData.length );
                if ( plainText == null ) {
                	Utils.LogE(className, "DecryptMessage: Failed to allocate ByteBuffer for cipher."); break;
                }
        		
                ByteOrder nativeOrder = ByteOrder.nativeOrder();
        		if ( plainText.order() != nativeOrder )
        			plainText.order ( nativeOrder );
        		
        		plainText.put(cipherData);                
        	}
        	while ( false );
    	}
    	catch (Exception ex)
    	{
    		Utils.LogE ( className, "DecryptMessage: Exception [" + ex.getMessage() + "]");
        	ex.printStackTrace();
    	}
        
        return plainText;      	
    }


	@SuppressWarnings ( "all" )
	private static SparseArray<byte[]> aesKeys = new SparseArray<byte[]>();
	
	
	static void AESDisposeKeyContext ( int deviceID )
	{
		if (Utils.isDebug) Utils.Log ( 5, className, "AESDisposeKeyContext: [" + deviceID + "]");
    	
		byte[] key = aesKeys.get(deviceID);
		if ( key != null ) {
            for (int i=0; i<key.length; i++)
            	key[i] = 0;
            aesKeys.remove(deviceID);
		}
	}
	
	
	static void AESUpdateKeyContext ( int deviceIDTemp, int deviceID )
	{
		if (Utils.isDebug) Utils.Log ( 5, className, "AESUpdateKeyContext: " + deviceIDTemp + " -> " + deviceID );
    	
		byte[] keyData = aesKeys.get(deviceIDTemp);
		if ( keyData == null ) {
			Utils.LogW(className, "AESUpdateKeyContext: No keys found for temporary deviceID [" + deviceIDTemp + "]");
			return;
		}
		
        aesKeys.put(deviceID, keyData);
	}
	

	static ByteBuffer AESDeriveKeyContext ( int deviceID, byte[] key, int keyLen, int aesSize )
	{
		if (Utils.isDebug) Utils.Log ( 5, className, "AESDeriveKeyContext");
    	
    	ByteBuffer keyBuffer = null;
    	try
    	{
        	do
        	{
                MessageDigest digest = MessageDigest.getInstance("SHA-256");
                if (digest == null)
                	break;
                
                digest.reset();

                byte[] keySource = new byte[aesSize];
                System.arraycopy(key, 0, keySource, 0, aesSize);

                /*for (int i=0; i<aesSize; i++)
                	keySource[i] = key[i];
                	*/
                
                byte[] keyData = digest.digest(keySource);
                if (keyData == null)
                	break;
                                
                ByteBuffer output = Environs.Java_malloc(aesSize, false);
                if ( output == null )
                	break;
                
                output.put(keyData, 0, keyData.length);
                
           		/*StringBuffer sb = new StringBuffer();
                for (int i = 0; i < keyData.length; i++){
                  sb.append(Integer.toString((keyData[i] & 0xff) + 0x100, 16).substring(1) + " ");
                }
                Utils.LogE ( className, "AESDeriveKeyContexts: key [" + sb.toString().toUpperCase() + "]" );
                */
                
                aesKeys.put(deviceID, keyData);
                keyBuffer = output;
        	}
        	while ( false );
    	}
    	catch (Exception ex)
    	{
    		Utils.LogE ( className, "AESDeriveKeyContext: Exception [" + ex.getMessage() + "]");
        	ex.printStackTrace();
    	}
    	
		return keyBuffer;
	}
	

	@SuppressWarnings ( "LoopStatementThatDoesntLoop" )
	static ByteBuffer AESTransform ( int deviceID, boolean encrypt, byte[] msg, byte [] keyIV )
    {
    	//Utils.Log ( 2, "[TRACE] Environs.AESTransform: " + (encrypt ? "Encrypt" : "Decrypt"));

    	try
    	{
        	do
        	{
        		byte[] key = aesKeys.get(deviceID);
        		if ( key == null || keyIV == null ) {
        			Utils.LogE ( className, "AESTransform: No keys found for deviceID [" + deviceID + "]" );
        			break;
        		}
        		
        		//Utils.Log ( 0, "Utils.AESTransform: Message sized [" + msg.length + "]");
        		
        		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        		
                SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
                IvParameterSpec ivParameterSpec = new IvParameterSpec(keyIV);
                
                cipher.init(encrypt ? Cipher.ENCRYPT_MODE : Cipher.DECRYPT_MODE, secretKeySpec, ivParameterSpec);

                byte[] out = cipher.doFinal(msg);
        		
        		ByteBuffer output = ByteBuffer.allocateDirect(out.length); //Environs.Java_malloc ( (out.length * 2), false );//
        		output.put(out);
        		return output;
        	}
        	while ( false );
    	}
    	catch (Exception ex)
    	{
    		Utils.LogE ( className, "AESTransform: Exception [" + ex.getMessage() + "]");
        	ex.printStackTrace();
    	}
        
        return null;    	
    }


	@SuppressWarnings ( "ResultOfMethodCallIgnored" )
	public static boolean CopyFile(File source, File dest) {
		try
		{
			if(!dest.exists()) {
				dest.createNewFile();
			}

			FileChannel schannel = null;
			FileChannel dchannel = null;

			try {
				schannel = new FileInputStream(source).getChannel();
				dchannel = new FileOutputStream(dest).getChannel();
				dchannel.transferFrom(schannel, 0, schannel.size());
			}
			finally {
				if(schannel != null) {
					schannel.close();
				}
				if(dchannel != null) {
					dchannel.close();
				}
			}
			return true;
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return false;
	}
}
