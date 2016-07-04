package environs.MediaBrowser;
/**
 *	DeviceActivity
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
import java.io.IOException;
import java.util.ArrayList;

import com.actionbarsherlock.app.SherlockFragmentActivity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

// Code for file chooser
import ar.com.daidalos.afiledialog.FileChooserActivity;
import environs.*;


/**
*	Device activity
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class DeviceActivity extends SherlockFragmentActivity implements DeviceObserver, ListObserver, PortalObserver
{
    private static String className = "DeviceActivity";

    /**
     * A static instance of the "application"
     */
    public static DeviceActivity instance = null;
    public static DeviceInstance device = null;

    private static RadioButton btn_IsConnected = null;
    private static RadioButton btn_IsNearby = null;
    private static TextView text_DeviceName = null;
    private static Button btn_connect = null;
    private static Button btnPortalIn = null;
    private static Button btnPortalOut = null;

    @SuppressWarnings ( "all" )
    private static Button btnTest = null;
    private static Button btnSend = null;
    private static Button button_sendFile = null;
    private static Button button_takePicture = null;
    private static Button button_gallery = null;
    private static EditText messageText = null;
    private static TextView textIP = null;
    private static TextView textIPExt = null;
    private static TextView textApp = null;
    private static TextView textArea = null;
    private ProgressBar progressBar = null;

    private static int deviceConnectTrys = 0;
    private static int TAKE_PHOTO_REQUEST = 10;
    private static int PHOTO_GALLERY_REQUEST = 11;
    private static int photoID = 1;
    private static File photoFile = null;
    private static String photoName = null;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Utils.Log ( 4, className, "onCreate");

        instance = this;

        if (!Environs.IsInstalled()) {
            Utils.LogE("Environs library is not installed correctly!");
            return;
        }

        setTheme(TabActivity.THEME);

        setContentView( R.layout.device);

        View instanceView = findViewById(android.R.id.content);
        if (instanceView == null) {
        	Utils.LogW(className, "onCreate: view not found!!!");
        	return;
        }

        progressBar = (ProgressBar) findViewById(R.id.progressBar1);
        if (progressBar == null) {
        	Utils.LogW ( className, "onCreate: progressBar not found!!!" );
        	return;
        }


        text_DeviceName = (TextView) findViewById(R.id.textDeviceName);
        if (text_DeviceName == null) {
        	Utils.LogW ( className, "onCreate: textView devicename not found!!!" );
        	return;
        }


        btn_IsConnected = (RadioButton) findViewById(R.id.IsConnected);
        if (btn_IsConnected == null) {
        	Utils.LogW ( className, "onCreate: connectStatus RadioButton not found!!!" );
        	return;
        }
        btn_IsConnected.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
            	UpdateUI();
            }
        });


        btn_IsNearby = (RadioButton) findViewById(R.id.IsNearby);
        if (btn_IsNearby == null) {
        	Utils.LogW ( className, "onCreate: nearby RadioButton not found!!!" );
        	return;
        }
        btn_IsNearby.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
            	UpdateUI();
            }
        });


        btn_connect = (Button) findViewById(R.id.btn_connect);
        if (btn_connect == null) {
        	Utils.LogW ( className, "onCreate: connect Button not found!!!" );
        	return;
        }
        btn_connect.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                doConnect();
            }
        });
        btn_connect.requestFocus();


        btnPortalIn = (Button) findViewById(R.id.button_portal);
        if (btnPortalIn == null) {
        	Utils.LogE ( className, "onCreate: Failed to find portal button!" );
    		return;
        }
        btnPortalIn.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                if (device == null) return;

                PortalInstance portal = device.PortalGetIncoming();
                if ( portal == null ) {
                    portal = device.PortalRequest(PortalType.Any);
                    if (portal != null) {
                        portal.AddObserver(DeviceActivity.this);
                        portal.Establish(true);
                    }
                }
                else portal.Stop();
            }
        });


        btnPortalOut = (Button) findViewById(R.id.button_portalOut);
        if (btnPortalOut == null) {
            Utils.LogE ( className, "onCreate: Failed to find portal out button!" );
            return;
        }
        btnPortalOut.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                if (device == null) return;

                PortalInstance portal = device.PortalGetOutgoing();
                if ( portal == null ) {
                    portal = device.PortalProvide(PortalType.FrontCam);
                    if (portal != null) {
                        portal.AddObserver(DeviceActivity.this);
                        portal.Establish(true);
                    }
                }
                else portal.Stop();
            }
        });


        btnTest = (Button) findViewById(R.id.btn_test);
        if (btnTest == null) {
        	Utils.LogE ( className, "onCreate: Failed to find test button!" );
    		return;
        }
        btnTest.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                EnvironsTests.invokeTest(device);
            }
        });
        btnTest.setEnabled(true);


        // Textbox for message
        messageText = (EditText) findViewById(R.id.MessageToSend);
        if (messageText == null) {
        	Utils.LogE ( className, "onCreate: Failed to find EditText for message!" );
    		return;
        }

        messageText.setOnEditorActionListener(new OnEditorActionListener()
        {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event)
            {
                if (actionId == EditorInfo.IME_ACTION_DONE || (event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))) {
                    InputMethodManager in = (InputMethodManager) instance.getSystemService(Context.INPUT_METHOD_SERVICE);

                    in.hideSoftInputFromWindow(v.getApplicationWindowToken(), 0);
                    Utils.Log ( 2, className, "onCreateView: Hiding keyboard" );

            		v.clearFocus();
            		btnSend.requestFocus();
                   return true;

                }
                return false;
            }
        });


        // Send button
        btnSend = (Button) findViewById(R.id.button_send);
        if (btnSend == null) {
        	Utils.LogE ( className, "onCreate: Failed to find send button!" );
    		return;
        }
        btnSend.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                sendTextMessage();
            }
        });


        // Send file button
        button_sendFile = (Button) findViewById(R.id.button_sendFile);
        if (button_sendFile == null) {
        	Utils.LogE ( className, "onCreate: Failed to find send file button!" );
    		return;
        }
        button_sendFile.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                sendFile();
            }
        });


        // Take picture button
        button_takePicture = (Button) findViewById(R.id.button_takePicture);
        if (button_takePicture == null) {
        	Utils.LogE ( className, "onCreate: Failed to find take picture button!" );
    		return;
        }
        button_takePicture.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                takePhotoWithCamera();
            }
        });


        // Gallery button
        button_gallery = (Button) findViewById(R.id.button_gallery);
        if (button_gallery == null) {
        	Utils.LogE ( className, "onCreate: Failed to find gallery button!" );
    		return;
        }
        button_gallery.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                sendGalleryPicture();
            }
        });


        textIP = (TextView) findViewById(R.id.textIP);
        if (textIP == null) {
        	Utils.LogE ( className, "onCreate: Failed to find IP text!" );
    		return;
        }

        textIPExt = (TextView) findViewById(R.id.textIPExt);
        if (textIPExt == null) {
        	Utils.LogE ( className, "onCreate: Failed to find IP ext text!" );
    		return;
        }

        textApp = (TextView) findViewById(R.id.textApp);
        if (textApp == null) {
        	Utils.LogE ( className, "onCreate: Failed to find app text!" );
    		return;
        }

        textArea = (TextView) findViewById(R.id.textArea);
        if (textArea == null) {
        	Utils.LogE ( className, "onCreate: Failed to find area text!" );
        }
    }


    public void OnDeviceChanged ( DeviceInstance device, int flagsChanged )
    {
        if ( flagsChanged == DeviceInfoFlag.ConnectProgress )
        {
            SetConnectProgress(device.connectProgress);
            return;
        }

        if ( flagsChanged == DeviceInfoFlag.Disposed ) {
            OnListChanged(null, null);
        }
        else if ( flagsChanged == DeviceInfoFlag.IsConnected )
        {
            DeviceDisplay props = device.GetDisplayProps();
            if (props != null)
                Utils.Log ( 5, className, props.toString());
            else
                Utils.LogE ( className, "DeviceProps failed" );
        }

        UpdateUI();
    }


    public void OnListChanged ( ArrayList<DeviceInstance> oldDevices, ArrayList<DeviceInstance> newDevices )
    {
        if (device != null && device.disposed && newDevices != null) {
            Utils.Log ( 4, className, "onListChanged" );

            for (int i=0; i<newDevices.size(); i++)
            {
                DeviceInstance newDevice = newDevices.get(i);

                Utils.Log ( 1, className, newDevice.toString());

                if (device.EqualsID(newDevice)) {
                    device = newDevice;
                    device.AddObserver(this);

                    new Thread(new Runnable() {
                        public void run(){
                            UpdateUI();
                        }
                    }).start();
                    break;
                }
            }
        }
    }


    static boolean isVideoStream = true;

    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 	The PortalInstance object.
     */
    public void OnPortalRequestOrProvided(PortalInstance portal)
    {
        if ( portal == null )
            return;

        // If we don't add an observer, then the portal will be discarded
        portal.AddObserver(this);

        portal.startIfPossible = true;

        portal.Establish(true);
    }


    /**
     * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
     *
     * @param portal	The PortalInstance object.
     * @param notify	The notification (Notify.Portal) that indicates the change.
     */
    public void OnPortalChanged(PortalInstance portal, int notify)
    {
        if ( portal != null && portal.outgoing ) {
            if ( portal.status < 2 ) {
                portal.Start();
            }
            return;
        }

        if (portal == null)
            return;

        if (notify == Notify.Portal.StreamIncoming)
        {
            isVideoStream = true;
            StartFullscreen ( portal );
        }
        else if (notify == Notify.Portal.ImagesIncoming)
        {
            isVideoStream = false;
            StartFullscreen(portal);
        }
        else if ( notify == Notify.Portal.StreamStopped || notify == Notify.Portal.Disposed )
            {
                if ((portal.portalID & Environs.PORTAL_DIR_INCOMING) != 0) {
                    TabActivity.instance.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (Fullscreen.instance != null)
                                Fullscreen.instance.finish();
                            MainTab.updateUI();
                            if (DeviceActivity.instance != null)
                                DeviceActivity.instance.UpdateUI();
                        }
                    });
                }
            }
/*		else if ( notify == Environs.NOTIFY_CONTACT_DIRECT_CHANGED ) {

			if ( Environs.GetPortalAutoStart() )
			{
				if ( Fullscreen.instance == null ) {
					synchronized ( Fullscreen.className ) {
						if ( !Fullscreen.initiated ) {
							MainTab.showEnvironsMessage ( "Starting fullscreen." );

							StartFullscreen ( portal );
						}
					}
				}
				else {
					if (portal.status < 2)
						portal.Start();
				}
			}
		}
			*/
    }


    private void StartFullscreen ( PortalInstance portal )
    {
        if (TabActivity.instance == null)
            return;

        // Lets start the Fullscreen view
        synchronized (Fullscreen.className)
        {
            if (Fullscreen.initiated || Fullscreen.instance != null)
                return;

            Fullscreen.portal = portal;

            Intent intent = new Intent(TabActivity.instance, Fullscreen.class);

            TabActivity.instance.startActivity(intent);
            Fullscreen.initiated = true;
        }
    }


    public void OnImage ( Bitmap bitmap ) {
        if (Fullscreen.instance != null) {
            final Bitmap tbitmap = bitmap;
            Fullscreen.instance.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Fullscreen.background = tbitmap;
                    Fullscreen.instance.UpdateBackground();
                }
            });
        }
    }
    

    private void SetConnectProgress(final int progress) {
    	if (progressBar == null)
    		return;

		runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int value = progress;
                if (value > 1000) {
                    progressBar.getProgressDrawable().setColorFilter(Color.RED, Mode.SRC_IN);

                    value -= 1000;
                } else
                    progressBar.getProgressDrawable().setColorFilter(Color.CYAN, Mode.SRC_IN);
                progressBar.setProgress(value);
            }
        });
    }


    @SuppressWarnings ( "all" )
    public void UpdateUI() {
        final Environs env = TabActivity.env;
        if (env == null)
            return;

        if ( env.GetStatus() < Environs.STATUS_STARTED || device == null )
            return;

        runOnUiThread(new Runnable() {
            @SuppressLint("DefaultLocale")
            @Override
            public void run() {
                if (device.disposed) {
                    btn_connect.setText("---");

                    text_DeviceName.setText("---");
                    textIP.setText("---");
                    textIPExt.setText("---");

                    textApp.setText("---");
                    textArea.setText("---");
                    btn_IsConnected.setChecked(false);

                    progressBar.getProgressDrawable().setColorFilter(Color.CYAN, Mode.SRC_IN);
                    progressBar.setProgress(0);
                    return;
                }

                boolean connected = device.isConnected;

                btn_IsConnected.setChecked(connected);
                if (connected) {
                    btn_connect.setText("Disconnect");
                    deviceConnectTrys = 0;
                } else
                    btn_connect.setText("Connect");

                button_takePicture.setEnabled(connected);
                button_takePicture.setVisibility(connected ? View.VISIBLE : View.INVISIBLE);
                button_sendFile.setEnabled(connected);
                button_sendFile.setVisibility(connected ? View.VISIBLE : View.INVISIBLE);
                button_gallery.setEnabled(connected);
                button_gallery.setVisibility(connected ? View.VISIBLE : View.INVISIBLE);

                btnPortalIn.setEnabled(connected);
                btnPortalIn.setVisibility(connected ? View.VISIBLE : View.INVISIBLE);
                btnPortalOut.setEnabled(connected);
                btnPortalOut.setVisibility(connected ? View.VISIBLE : View.INVISIBLE);

                btn_IsNearby.setChecked(device.sourceType == DeviceSourceType.Broadcast);
                text_DeviceName.setText(device.DeviceTypeString() + " 0x" + Integer.toHexString(device.deviceID).toUpperCase() + ": " + device.deviceName);

                textIP.setText(device.ip);
                textIPExt.setText(device.ipe);

                textApp.setText(device.appName);
                textArea.setText(device.areaName);
            }
        });
    }

    public static String lastSelectedFile;
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Utils.Log (4 ,className, "onActivityResult");
    	    	
        if (resultCode == Activity.RESULT_OK) {
        	if (requestCode == TAKE_PHOTO_REQUEST) {
        		if (photoFile != null && device != null) {
                    device.SendFile(1000 + photoID, photoFile.getName(), photoFile.getAbsolutePath());
        		}
            }
        	else if (requestCode == PHOTO_GALLERY_REQUEST) {
        		if (data == null) {
        			return;
        		}
        		
        		Uri selected = data.getData();
        		if (selected == null) {
        			return;
        		}

        		String[] photoPaths = { MediaStore.Images.Media.DATA };

                Cursor cur = getContentResolver().query(selected, photoPaths, null, null, null);
        		if (cur == null) {
        			return;
        		}

        		cur.moveToFirst();

                int colIndex = cur.getColumnIndex(photoPaths[0]);
                if (colIndex < 0) {
                	return;
                }

                String photoPath = cur.getString(colIndex);
                if (photoPath == null) {
                	return;
                }

                cur.close();

                device.SendFile(1000 + photoID, photoName, photoPath);
        	}
        	else {
                Bundle bundle = data.getExtras();
                if(bundle != null)
                {
                	// Code for file chooser            
                    if(!bundle.containsKey(FileChooserActivity.OUTPUT_NEW_FILE_NAME))
                    {
                        File file = (File) bundle.get(FileChooserActivity.OUTPUT_FILE_OBJECT);
                        if (file != null) {
                            String filePath = file.getAbsolutePath ( );
                            lastSelectedFile = filePath;

                            device.SendFile ( 1000, file.getName ( ), filePath );
                        }
                    }                  
                }
        	}
        }
    }


    private void doConnect()
    {
        if (device == null) return;

        String btn_text = btn_connect.getText().toString();

        if (btn_text.equals("Connect")) {
            deviceConnectTrys++;
            if ( deviceConnectTrys > 2 ) {
                device.Connect();
                deviceConnectTrys = 0;
            }
            else
                device.Connect();
        }
        else {
            device.Disconnect();
        }
    }


    private void sendFile()
    {
        // Get device id
        if (device == null) return;

        // Code for file chooser
        Intent intent = new Intent(DeviceActivity.instance, FileChooserActivity.class);
        intent.putExtra(FileChooserActivity.INPUT_REGEX_FILTER, ".*");

        startActivityForResult(intent, 0);
    }


    private void sendGalleryPicture()
    {
        // Get device id
        if (device == null) return;

        photoID++;
        photoName = "environs" + photoID + ".jpg";

        Intent intent = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);

        startActivityForResult(intent, PHOTO_GALLERY_REQUEST);
    }


    private void sendTextMessage()
    {
        // Get device id
        if (device == null || messageText == null)
            return;

        String msg = messageText.getText().toString();
        device.SendMessage(msg);
    }


    @SuppressWarnings ( "all" )
    private void takePhotoWithCamera()
    {
        // Get device id
        if (device == null) return;

        photoID++;
        photoName = "environs" + photoID + ".jpg";
        String photoPath = photoName;

        File storageFile = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        if (storageFile != null) {

            String storageDir = storageFile.toString() + "/environs/";
            File newdir = new File(storageDir);
            if (!newdir.exists()) {
                newdir.mkdirs();
            }
            photoPath = storageDir + photoPath;
        }

        photoFile = new File(photoPath);
        try {
            if (photoFile.exists())
                photoFile.delete();

            photoFile.createNewFile();
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        Uri photoUri = Uri.fromFile(photoFile);

        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, photoUri);

        startActivityForResult(intent, TAKE_PHOTO_REQUEST);
    }


    @Override
    public void onResume() {
        Utils.Log(4, className, "onResume");
        super.onResume();

        TabActivity.currentActivity = this;

        UpdateUI();

        if (device != null)
            device.AddObserver(this);

        if (DevicelistTab.adapter != null)
            DevicelistTab.adapter.AddObserver(this);
    }


    @Override
    protected void onPause() {
        Utils.Log(4, className, "onPause");

        super.onPause();

        if (device != null)
            device.RemoveObserver(this);

        if (DevicelistTab.adapter != null)
            DevicelistTab.adapter.RemoveObserver(this);
    }


    @Override
    protected void onDestroy () {
        Utils.Log(4, className, "onDestroy");

        instance = null;

        super.onDestroy();
    }

    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU) {
            // ........
        	Utils.Log( 5, className, "onKeyUp Menu");
        }
        else if (keyCode == KeyEvent.KEYCODE_BACK) {
            // ........
        	Utils.Log( 5, className, "onKeyUp Back");
        	if (Fullscreen.instance != null)
        		Fullscreen.instance.finish();

        	if (instance != null)
        		instance.finish();
        }
        
        return super.onKeyUp(keyCode, event);
    }

}
