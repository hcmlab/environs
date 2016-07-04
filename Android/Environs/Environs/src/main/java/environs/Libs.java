package environs;
/**
 * Libs
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

import org.itadaki.bzip2.BZip2InputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.security.cert.X509Certificate;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

/**
*	Library helper class for native libraries
*	---------------------------------------------------------
*	Copyright (C) 2012-2014 Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class Libs {
	private static final String className = "Libs . . . . . . . . . .";

	public Libs()
	{
	}
	
	static String libDir = null;
	static String workDir = null;
	static String architecture = "armeabi-v7a";
	static String revision = "5100";
	static boolean isJAR = false;

	/**
	 * This flag requests that the native layer libraries shall be reinitialized.
	 * Reinitializing first removes cached libraries and install them cleanly.
	 */
	static boolean nativeLayerReinit = false;


	/**
	 * Forces the native layer libraries to be cleanly reinitialized.
	 * Reinitializing first removes cached libraries and install them cleanly.
	 */
	@SuppressWarnings( "unused" )
	public static void setReinitNativeLayer () {
		if (Utils.isDebug) Utils.Log ( 5, className, "setReinitNativeLayer" );
		Libs.nativeLayerReinit = true;
		Libs.libDir = null;
	}


	static void Init_libEnvirons () throws Exception
	{
		/// Prepare folder structure
		/// /data/data/application-name/files

		/// libstlport_shared.so
		//final String stlType = "libc++";
		//final String stlType = "stlport";
		final String stlType = "gnustl";

		Environs.nativeLayerInstalled = LoadLibrary(stlType + "_shared", true, true );

		/// libEnvirons.so
		if ( Environs.nativeLayerInstalled )
			Environs.nativeLayerInstalled = LoadLibrary ( "Environs", true, true );

		if ( !LoadLibrary ( "crypto", true, false ) || !LoadLibrary ( "ssl", true, false ) ) {
			Utils.LogE ( className, "libcrypto failed");
		} else
			Utils.Log1 ( className, "libcrypto ok" );

		if ( !Environs.nativeLayerInstalled ) {
			Utils.LogE ( className, "libEnvirons.so or lib" + stlType + "_shared is not installed correctly. Please visit http://hcm-lab.de/environs for an updated version of Environs!" );
		}
		else {
			if (Environs.class.getResourceAsStream("/environs.jar.sig" ) != null )
			{
				Libs.isJAR = true;

				Libs.extract ( "Env-DecAndroid" );
				Libs.extract ( "openh264" );
				Libs.extract ( "Env-EncOpenH264" );
				Libs.extract ( "Env-DecOpenH264" );

				Libs.libDir = null;
				Libs.workDir = null;
			}
			else {
				LoadLibrary("Env-DecAndroid", false, false );
				LoadLibrary("openh264", false, false );
				LoadLibrary("Env-EncOpenH264", false, false );
				LoadLibrary("Env-DecOpenH264", false, false );
			}
		}
	}


	static boolean LoadLibrary ( String lib, boolean mandatory, boolean isJar ) {
		try {
			System.loadLibrary ( lib );
		}
		catch ( UnsatisfiedLinkError e ) {
			if ( mandatory )
				if (Utils.isDebug) Utils.Log ( 4, className, "LoadLibrary: Failed to load native lib" + lib + " !!!" );

			if (!isJar)
				return false;

			if ( Libs.libDir == null ) {
				if ( !Libs.initDirs () ) {
					return false;
				}
			}
			try {
				System.loadLibrary ( Libs.libDir + "/lib" + lib + ".so" );
				if (Utils.isDebug) Utils.Log ( 5, className, "LoadLibrary: load native lib" + lib + ".so from [" + Libs.libDir + "] ok" );
			}
			catch ( UnsatisfiedLinkError e1 ) {
				try {
					if ( Libs.load ( lib ) ) {
						if ( Utils.isDebug )
							Utils.Log ( 4, className, "LoadLibrary: lib" + lib + ".so extracted from jar." );
					}
					else {
						if (Utils.isDebug) {
							Utils.Log ( 4, className, "LoadLibrary: lib" + lib + ".so not shipped with the jar." );
						}
						return false;
					}
				} catch ( Exception e2 ) {
					//throw new RuntimeException(e1);
					if ( mandatory )
						Utils.LogE ( className, "LoadLibrary: Failed to load native lib" + lib + " from jar!!!" );
					else
						if (Utils.isDebug) Utils.Log ( 4, className, "LoadLibrary: Native lib" + lib + " not shipped with the jar." );
					return false;
				}

			}
		}
		return true;
	}


	@SuppressWarnings("deprecation")
	@SuppressLint("NewApi")
	static boolean initDirs ()
	{
		try
		{
			String firstArch = null;
			if ( android.os.Build.VERSION.SDK_INT < 21 ) {
				firstArch = android.os.Build.CPU_ABI;
				if ( firstArch == null )
					firstArch = android.os.Build.CPU_ABI2;
			}
			else {
				String [] archs = android.os.Build.SUPPORTED_ABIS;
				for ( String arch : archs )
					if ( arch != null ) {
						firstArch = arch;
						break;
					}

			}
			if (firstArch == null) {
				Utils.LogW(className, "initDirs: Could not detect CPU architecture.");
			}
			else {
				if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: CPU architecture [" + firstArch + "]" );

				// This has to be extended if we support more achitectures
				if (!firstArch.contains("7"))
					architecture = "armeabi";
			}

			InputStream ins = Environs.class.getResourceAsStream("/Environs.revision.txt");
			if ( ins != null ) {
				byte[] buffer = new byte[10];
				for ( int j=0; j<buffer.length; j++ )
					buffer [ j ] = 0;

				int readBytes  = ins.read(buffer);
				if (readBytes > 2 ) {
					try
					{
						revision = new String(buffer, "UTF-8").trim ();
					}
					catch ( Exception ex ) {
						ex.printStackTrace ();
					}
				}
			}
			if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Revision [" + revision + "]" );

            File libFile = File.createTempFile("libCheck", ".so");
            libFile.deleteOnExit();
            
            if ( !libFile.exists() ) {
            	Utils.LogE ( className, "initDirs: Failed to create temporary file for parsing lib root directory " + libFile.getAbsolutePath() );
            	return false;
            }

            String path = libFile.getAbsolutePath();
            int lastSep = path.lastIndexOf('/');
            if ( lastSep == -1 ) {
            	Utils.LogE ( className, "initDirs: Failed to find last / in " + path );
            	return false;
            }

            path = path.substring(0, lastSep); /// /data/data/app-name/cache
			if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Cache directory [" + path + "]" );
        	
        	/// Clear cache directory
        	File cacheFileDir = new File(path);        
        	File cacheFiles[] = cacheFileDir.listFiles();
        	if (cacheFiles != null && cacheFiles.length > 0) {
            	for ( File fi :  cacheFiles ) {
            		if (fi.getName().startsWith("lib"))
            			if (!fi.delete() ) {
                            Utils.LogE ( className, "initDirs: Failed to delete cached file: [" + fi.getName() + "]" );
                        }
            	}
        		
        	}
        	
        	lastSep = path.lastIndexOf ( '/' );
        	if ( lastSep == -1 ) {
            	Utils.LogE ( className, "initDirs: Failed to find last / in " + path );
            	return false;
            }
        	
        	workDir = path.substring(0, lastSep);
			if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Working directory [" + workDir + "]" );


			String libDirBase = workDir + "/libs";
			String libDirTry = libDirBase;

			for ( int i=0; i<5; i++ ) {
				if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Library directory [" + libDirTry + "]" );

				File libFolder = new File ( libDirTry );
				if ( !libFolder.isDirectory () )
				{
					if (Utils.isDebug) Utils.Log ( 6, className, "initDirs: Creating [" + libDirTry + "]" );
					if ( !libFolder.mkdir () && !libFolder.isDirectory () )
					{
						if (Utils.isDebug) Utils.Log ( 6, className, "initDirs: Failed to create [" + libDirTry + "]" );
						libDirTry = libDirBase + i;
						continue;
					}
					if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Successfully created [" + libDirTry + "]" );
				}

				if ( i == 4 && !libFolder.isDirectory () ) {
					Utils.LogE ( className, "initDirs: Failed to create [" + libDirBase + " 0-4]" );
					return false;
				}

				libDir = libDirTry;

				File revFile = new File ( libDir + "/" + revision );

				if (nativeLayerReinit) {
					nativeLayerReinit = false;
					if (revFile.exists())
						if (!revFile.delete())
						{
							Utils.LogE ( className, "initDirs: Failed to clear revision file [" + libDir + "/" + revision );
						}
				}

				if ( !revFile.exists() ) {
					// Clear the libfolder
					File cacheLibDir = new File(libDir);
					File cacheLibFiles[] = cacheLibDir.listFiles();
					if (cacheLibFiles != null && cacheLibFiles.length > 0) {
						for ( File fi :  cacheLibFiles ) {
							if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Delete cached file [" + fi.getName ( ) + "]" );
							if (!fi.delete() ) {
								Utils.LogE ( className, "initDirs: Failed to delete cached file [" + fi.getName() + "]" );
							}
						}
					}
					if (Utils.isDebug) Utils.Log ( 5, className, "initDirs: Creating revision notice [" + revFile.getName ( ) + "]" );
					if ( !revFile.createNewFile() ) {
						Utils.LogE ( className, "initDirs: Failed to create revision notice [" + revFile.getName() + "]" );
					}
				}
				break;
			}
			return true;
		}
		catch ( Exception e )
		{			
        	Utils.LogE ( className, "initDirs: Failed with exception [" + e.getMessage() + "]" );
		}
    	return false;
	}

	
	static String extract ( String lib )
	{
		String path = ExtractFromJar ( lib );
		if ( path == null ) {
			if (Utils.isDebug) Utils.Log ( 4, className, "extract: : Extract of lib" + lib + ".so from jar failed." );
		}
		return path;
	}

	
	static String ExtractFromJar ( String lib )
	{
		if ( lib == null )
			return null;

    	if ( libDir == null || workDir == null ) {	
    		if ( !initDirs () )
    			return null;
    	}
    	
        try {
        	String libName = "lib" + lib + ".so";
        	String libPath = libDir + "/" + libName;
        	
        	File libFile = new File ( libPath );    
        	
        	if ( libFile.exists() ) {
				if (Utils.isDebug) Utils.Log ( 5, className, "ExtractFromJar: Library found in [" + libPath + "]" );
        		return libPath;
        	}
        	
            // Prepare temporary file
        	/// /data/data/app-name/cache
            libFile = File.createTempFile("lib" + lib, ".so");
     
            if ( !libFile.exists() ) {
            	Utils.LogE ( className, "ExtractFromJar: Failed to create temporary file for library " + libFile.getAbsolutePath() );
            	return null;
            }
     
            byte[] buffer = new byte[1024];
            int readBytes;

            InputStream ins = Environs.class.getResourceAsStream("/jni/" + libName);
            if ( ins == null ) {
            	//Utils.LogE ( className, "extract: Failed to load " + libName + " from jar!" );
				ins = Environs.class.getResourceAsStream("/jni/" + architecture + "/" + libName);
				if (ins == null) {
					if ( !libFile.delete() )
						Utils.LogE ( className, "ExtractFromJar: Failed to delete temporary file [" + libFile.getName() + "]!" );
					return null;
				}
            }
     
            OutputStream ous = null;
            try {
                ous = new FileOutputStream(libFile);
                
                while ((readBytes = ins.read(buffer)) != -1) {
                	ous.write(buffer, 0, readBytes);
                }
            } 
            catch(Exception e) 
            {
                Utils.LogE ( className, "ExtractFromJar: Failed with exception [" + e.getMessage() + "]");
            }
            finally {
            	if (ous != null)
            		ous.close();
            	ins.close();
            }
                 
        	String destPath = libDir + "/" + libName;

			if (Utils.isDebug) Utils.Log ( 5, className, "ExtractFromJar: Try renaming " + libFile.getAbsolutePath ( ) + " to " + destPath );
        	 
        	File destFile = new File (destPath);

            if (destFile.exists()) {
				if (Utils.isDebug) Utils.Log ( 6, className, "ExtractFromJar: Library ok [" + destPath + "]" );
                if (!destFile.delete()) {
                    Utils.LogE ( className, "ExtractFromJar: Failed to delete " + destPath + ".");
                    return null;
                }
            }

            if ( !libFile.renameTo(destFile) ) {
                Utils.LogE ( className, "ExtractFromJar: Renaming to " + destPath + " failed.");
            }
            else {
				if (Utils.isDebug) Utils.Log ( 5, className, "ExtractFromJar: " + libName + " extracted from jar to " + destPath );
                return destPath;
            }
        } 
        catch(Exception e) 
        {
            Utils.LogE ( className, "ExtractFromJar: Failed with exception [" + e.getMessage() + "]");
        }
        return null;
	}
	
	
 	/**
     * Loads library from jar-archive
     * 
     */
    static boolean load ( String lib ) throws Exception
    {
    	String libPath = extract ( lib );
 
        if ( libPath == null ) {
        	Utils.LogE ( className, "load: Failed to extract library lib" + lib + ".so from jar." );
        	return false;
        }
        System.load(libPath);
        return true;
    }


	/**
	 * Search cache for given library name
	 *
	 */
	static boolean isLibAvailable ( String libName )
	{
		if ( libName == null )
			return false;

		if ( !libName.endsWith(".so") )
			libName += ".so";

		File file = new File(workDir + "/" + libName);
		return file.exists ( );

	}


	/**
	 * Delete 3rd party library from cache
	 *
	 */
	static void delete3rd ( String libName ) throws Exception
	{
		if ( libName == null )
			return;

		if ( !libName.endsWith(".so") )
			libName += ".so";

		File file = new File(workDir + "/" + libName);
		if ( file.exists() )
			if (!file.delete()) {
				Utils.LogE ( className, "delete3rd: Failed to remove " + workDir + "/" + libName );
			}

		file = new File(workDir + "/" + libName + ".txt");
		if ( file.exists() )
			if (!file.delete()) {
				Utils.LogE ( className, "delete3rd: Failed to remove " + workDir + "/" + libName + ".txt" );
			}
	}


	/**
	 * Loads 3rd party library from network
	 *
	 */
	static boolean load3rd ( String libName, String archived, String libURl,  String licenseUrl ) throws Exception
	{
		HttpsURLConnection scon;
		HttpURLConnection con;
		InputStream inStream;

		// Trust any certificate
		SSLContext ctx = SSLContext.getInstance("TLS");
		ctx.init(null, new TrustManager[] {
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

		URL url = new URL(libURl);

		if (libURl.contains("https://")) {
			scon = (HttpsURLConnection)url.openConnection();
			inStream = scon.getInputStream();
		}
		else {
			con = (HttpURLConnection)url.openConnection();
			inStream = con.getInputStream();
		}

		if (!libName.endsWith(".so"))
			libName += ".so";

		String fileName = libName;
		if (archived != null)
			fileName += archived;

		FileOutputStream outStream = new FileOutputStream(workDir + "/" + fileName);

		int bytesRead;
		byte[] buffer = new byte[100 * 1024];
		while ((bytesRead = inStream.read(buffer)) != -1) {
			outStream.write(buffer, 0, bytesRead);
		}

		outStream.close();
		inStream.close();

		if (archived != null) {
			if (archived.equals("bz2")) {
				FileInputStream inCompressed = new FileInputStream(workDir + "/" + fileName);

				File testOutput = new File(workDir + "/" + libName + ".tmp");
				if (testOutput.exists())
					if (!testOutput.delete()) {
						Utils.LogE ( className, "delete3rd: Failed to remove " + workDir + "/" + libName + ".tmp" );
					}

				FileOutputStream decompressed = new FileOutputStream(workDir + "/" + libName + ".tmp");

				InputStream fileInputStream = new BufferedInputStream(inCompressed);
				BZip2InputStream inputStream = new BZip2InputStream (fileInputStream, false);
				OutputStream decompressedStream = new BufferedOutputStream(decompressed, 2 * 1024 * 1024);

				byte[] compBytes = new byte [2 * 1024 * 1024];
				while ((bytesRead = inputStream.read (compBytes)) != -1) {
					decompressedStream.write (compBytes, 0, bytesRead) ;
				}
				decompressedStream.close();

				testOutput = new File(workDir + "/" + libName + ".tmp");
				if ( !testOutput.exists() )
					return false;

				if (!testOutput.renameTo ( new File ( workDir + "/" + libName ) )) {
					Utils.LogE ( className, "delete3rd: Failed to remove " + workDir + "/" + libName + ".tmp" );
				}

				if (Utils.isDebug) Utils.Log ( 3, className, "load3rd: Decompressed format " + archived );
			}
			else {
				Utils.LogE ( className, "load3rd: Unsupported archive format "  + archived);
			}
		}

		// Load license and present
		url = new URL(licenseUrl);

		if (licenseUrl.contains("https://")) {
			scon = (HttpsURLConnection)url.openConnection();
			inStream = scon.getInputStream();
		}
		else {
			con = (HttpURLConnection)url.openConnection();
			inStream = con.getInputStream();
		}

		outStream = new FileOutputStream(workDir + "/" + libName + ".txt");

		while ((bytesRead = inStream.read(buffer)) != -1) {
			outStream.write(buffer, 0, bytesRead);
		}

		outStream.close();
		inStream.close();

		return true;
	}


	/**
	 * @param libName	Name of the library
	 * @return			Content of the licence file
	 */
	static String getLicense ( String libName ) throws Exception
	{
		if ( !libName.endsWith(".so") )
			libName += ".so";

		libName += ".txt";

		File file = new File(workDir + "/" + libName);
		if ( !file.exists() ) {
			if (Utils.isDebug) Utils.Log ( 4, className, "getLicense: License for " + libName + " not found." );
			return null;
		}

		long length = file.length();
		if (length < 1 || length > Integer.MAX_VALUE) {
			if (Utils.isDebug) Utils.Log ( 4, className, "getLicense: License for " + libName + " has an invalid size: " + length );
			return null;
		}

		FileReader fread = new FileReader(file);

		char[] license = new char[(int)length];
		int readSize = fread.read(license);
		if (readSize < 0) {
			if (Utils.isDebug) Utils.Log ( 4, className, "getLicense: License for " + libName + " could not be read: " + readSize );
			return null;
		}
		return new String(license, 0, readSize);
	}
}
