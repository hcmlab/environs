package environs.MediaBrowser;
/**
 *	MainTab
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
import android.content.Context;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import com.actionbarsherlock.app.SherlockFragment;

import environs.*;


/**
 * MainTab class, implements the fragment which provides the main user controls to:
 * - Submit the tagID of the device's tag
 * - Start the environment with that id
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
*   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public class MainTab extends SherlockFragment
{
	private static String className = "MainTab";
	
	public static MainTab instance = null;

	public static ViewGroup mainTabView = null;
	private static Button btnStart = null;

	@SuppressWarnings ( "all" )
	private static Button button_test = null;

	public static EditText tagIDText = null;
	private static TextView textLog = null;
	private static TextView textWifi = null;

	public static int logTextLines = 0;
	public static String logText = "";


	/**
     * When creating, retrieve this instance's number from its arguments.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Utils.Log(4, className, "onCreate");

        super.onCreate(savedInstanceState);
        
        instance = this;
    }
    
    
    /**
     * Create the Fragment's UI
     */
    @SuppressLint("NewApi") @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
    {
    	Utils.Log ( 4, className, "onCreateView" );

		if (!Environs.IsInstalled()) {
			Utils.LogE("Environs library is not installed correctly!");
			return null;
		}

        View v = inflater.inflate( R.layout.main, container, false);
        if (v == null) {
        	Utils.LogE ( className, "onCreateView: Failed to get main layout!" );   
    		return null;
        }
        
        ViewGroup layout = (ViewGroup) v.findViewById(R.id.main_layout);
        if (layout == null) {
        	Utils.LogE( className, "onCreateView: Failed to find viewgroup of main layout!" );   
    		return v;
        }
        mainTabView = layout;
        
        // Start / Stop button
        btnStart = (Button) layout.findViewById(R.id.button_start);
        if (btnStart == null) {
        	Utils.LogE( className, "onCreateView: Failed to find start button!" );   
    		return v;
        }

        btnStart.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
            	if (tagIDText != null) tagIDText.clearFocus();
            	mainTabView.requestFocus();

				Environs env = TabActivity.env;
				if (env == null)
					return;

				env.SetUseTouchRecognizer("Env-RecGestureBezelTouch", true);
				env.SetUseTouchRecognizer("Env-RecGestureThreeTouch", true);

				env.Start();
            }
        });

        // Textbox for tagID
        tagIDText = (EditText) layout.findViewById(R.id.tagID);
        if (tagIDText == null) {
        	Utils.LogE( className, "onCreateView: Failed to find EditText for tagID!" );   
    		return v;
        }

		if (TabActivity.env != null) {
			String hex = Integer.toHexString(TabActivity.env.GetDeviceID());
			tagIDText.setText(hex);
		}
        
        tagIDText.addTextChangedListener(new TextWatcher() {
			public void afterTextChanged(Editable s) {
				if (s == null || s.length() < 1)
					return;
				try {
					int intValue = Integer.parseInt(s.toString().trim(), 16);
					if (intValue >= 0) {

						if (TabActivity.env != null)
							TabActivity.env.SetDeviceID(intValue);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}

			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}

			public void onTextChanged(CharSequence s, int start, int before, int count) {
			}
		});
        
        tagIDText.setOnEditorActionListener(new OnEditorActionListener() {
			@Override
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if (actionId == EditorInfo.IME_ACTION_DONE || (event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))) {
					InputMethodManager in = (InputMethodManager) getSherlockActivity().getSystemService(Context.INPUT_METHOD_SERVICE);

					in.hideSoftInputFromWindow(v.getApplicationWindowToken(), 0);
					Utils.Log(5, className, "onCreateView: Hiding keyboard");

					v.clearFocus();
					btnStart.requestFocus();
					return true;

				}
				return false;
			}
		});
        

        textLog = (TextView) layout.findViewById(R.id.textViewLog);
        if (textLog == null) {
        	Utils.LogE ( className, "onCreateView: Failed to find TextView for log messages!" );   
    		return v;
        }


		textWifi = (TextView) layout.findViewById(R.id.textViewWiFi);
		if (textLog == null) {
			Utils.LogE ( className, "onCreateView: Failed to find TextView for wifi status!" );
			return v;
		}


        button_test = (Button) layout.findViewById(R.id.button_test);
        if (button_test == null) {
        	Utils.LogE ( className, "onCreateView: Failed to find test button!" );   
    		return v;
        }


		button_test.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
            	doTestRun = !doTestRun;
            	/*if (doTestRun)
            		TestConnect();
            		*/
            }
        });
        
        return v;
    }
    
    public static boolean doTestRun = false;

	@SuppressWarnings ( "all" )
    public static void updateUI ()
    {
		final Environs env = TabActivity.env;
		if (env == null)
			return;

		final int status = env.GetStatus();
		if (status <= Status.Disposed)
			return;

		TabActivity.instance.runOnUiThread(new Runnable() {
			@Override
            public void run() {	
	            textLog.setText(logText);

            	if (TabActivity.instance.g_tabHost != null) {
    	            if ( status == Status.Started ) {
                		if (TabActivity.instance.g_tabHost.getCurrentTab() == 0)
                			TabActivity.instance.g_tabHost.setCurrentTab(1);
    	            }
    	            else if ( status == Status.Stopped ) {
                    		TabActivity.instance.g_tabHost.setCurrentTab(0);
    	            }            		
            	}
	            
		        if ( status < Status.Started )
		        {
		            btnStart.setText("Start");
		        }
		        else {
		            btnStart.setText("Stop");		        	
		        }

				if (textWifi != null)
					textWifi.setText ( "Network: " + env.GetSSID(true) );
            }
        });
    }
    

    @Override
    public void onResume() {    
    	Utils.Log ( 4, className, "onResume");
        super.onResume();

        updateUI ();
    }

    @Override
	public void onPause() {
    	Utils.Log ( 4, className, "onPause" );
        super.onPause();   
    }  
    
    @Override
	public void onDestroy () {
    	Utils.Log ( 4, className, "onDestroy" );
    	super.onDestroy();     	
    }


    public static void showEnvironsMessage(String msg) {
    	if (TabActivity.instance == null)
    		return;
    	
    	if (textLog == null)
    		return;
    	 	
    	final String log = msg;
    	TabActivity.instance.runOnUiThread(new Runnable() {

            @Override
            public void run() {
            	if ( logTextLines > 50 ) {
            		int i = logText.indexOf('\n');
            		if ( i >= 0 )
            			logText = logText.substring(i + 1);
            	}
            	logText += log + "\n"; 
            	logTextLines += 1;
            	textLog.setText(logText);
            }
        });
    	
    }
	

}
