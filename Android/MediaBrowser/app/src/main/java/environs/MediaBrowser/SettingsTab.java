package environs.MediaBrowser;
/**
 *	Settings tab
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
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import com.actionbarsherlock.app.SherlockFragment;

import environs.*;

/**
*	Settings tab
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class SettingsTab extends SherlockFragment implements OnCheckedChangeListener, OnEditorActionListener
{	
	private static String className = "SettingsTab";
	
	public static SettingsTab instance = null;
	
	public static ViewGroup mainTabView = null;
	public static EditText MediatorUserName = null;
	public static EditText MediatorPassword = null;
	public static EditText MediatorIP = null;
	public static TextView MediatorText = null;
	public static EditText MediatorPort = null;
	public static TextView MediatorTextPort = null;
	public static CheckBox checkBoxVideo = null;
	public static CheckBox checkBoxNativeVideo = null;
	public static CheckBox checkBoxPush = null;
	public static CheckBox CheckBoxSensor = null;
	public static CheckBox CheckBoxCustomMediator = null;
	public static CheckBox CheckBoxDefaultMediator = null;
	public static CheckBox CheckBoxAutoPortal = null;
	public static CheckBox CheckBoxNativeResolution = null;
	public static CheckBox CheckBoxTcpPortal = null;
	public static CheckBox CheckBoxNavigationButtons = null;
	public static CheckBox checkBoxHardwareDecoder = null;
	public static CheckBox checkBoxTLSDevices = null;
    public static CheckBox checkBoxShowDebugStatus = null;
    public static CheckBox checkBoxLogfile = null;
	
	
    /**
     * When creating, retrieve this instance's number from its arguments.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	Utils.Log ( 4, className, "onCreate");

        super.onCreate ( savedInstanceState );
        
        instance = this;
    }

    @Override
    public void onResume() {    
    	Utils.Log ( 4, className, "onResume");
        super.onResume ( );

        updateUI ( );
    }

    public static void updateUI ()
    {
    	if (instance == null)
    		return;

        Environs env = TabActivity.env;
        if (env == null)
            return;

    	final int status = env.GetStatus();

		TabActivity.instance.runOnUiThread ( new Runnable ( ) {
            @Override
            public void run ( ) {
                if ( status >= Environs.STATUS_STARTED ) {
                    Disable ( );
                } else if ( status < Environs.STATUS_STARTED ) {
                    Enable ( );
                }
            }
        } );
    }


    /**
     * Create the Fragment's UI
     */
    @SuppressWarnings ( "all" )
    @SuppressLint("NewApi") @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
    {
    	Utils.Log ( 4, className, "onCreateView");

        if (!Environs.IsInstalled()) {
            Utils.LogE("Environs library is not installed correctly!");
            return null;
        }

        View v = inflater.inflate( R.layout.settings, container, false);
        if (v == null) {
        	Utils.LogE ( className, "onCreateView: Failed to get main layout!" );   
    		return null;
        }

        boolean success = false;
        String lastView = "";
        do
        {
            ViewGroup layout = (ViewGroup) v.findViewById(R.id.settings_layout);
            if (layout == null) {
                Utils.LogE ( className, "onCreateView: Failed to find viewgroup of main layout!" );
                return v;
            }
            mainTabView = layout;

            TextView tvVersion;
            if ((tvVersion = (TextView) findView(R.id.TextViewVersion)) == null) {
                lastView = "TextViewVersion"; break;
            }
            tvVersion.setText ( "Version: " + Environs.GetVersionStringN ( ) );

            if ((checkBoxVideo = findCheckBox(R.id.checkBoxVideo)) == null) {
                lastView = "checkBoxVideo"; break;
            }

            if ((checkBoxNativeVideo = findCheckBox(R.id.checkBoxNativeVideo)) == null) {
                lastView = "checkBoxNativeVideo"; break;
            }

            if ((checkBoxHardwareDecoder = findCheckBox(R.id.checkBoxHardwareDecoder)) == null) {
                lastView = "checkBoxHardwareDecoder"; break;
            }

            if ((checkBoxPush = findCheckBox(R.id.checkBoxPush)) == null) {
                lastView = "checkBoxPush"; break;
            }

            if ((CheckBoxSensor = findCheckBox(R.id.CheckBoxSensor)) == null) {
                lastView = "CheckBoxSensor"; break;
            }

            if ((checkBoxTLSDevices = findCheckBox(R.id.checkBoxTLSDevices)) == null) {
                lastView = "checkBoxTLSDevices"; break;
            }

            if ((MediatorUserName = (EditText) findView(R.id.MediatorUserName)) == null) {
                lastView = "MediatorUserName"; break;
            }

            if ((MediatorPassword = (EditText) findView(R.id.MediatorPassword)) == null) {
                lastView = "MediatorPassword"; break;
            }

            if ((MediatorText = (TextView) findView(R.id.TextViewMediator)) == null) {
                lastView = "MediatorText"; break;
            }

            if ((MediatorIP = (EditText) findView(R.id.MediatorIP)) == null) {
                lastView = "MediatorIP"; break;
            }
            MediatorIP.setOnEditorActionListener(this);

            if ((MediatorTextPort = (TextView) findView(R.id.TextViewMediatorPort)) == null) {
                lastView = "MediatorTextPort"; break;
            }

            if ((MediatorPort = (EditText) findView(R.id.MediatorPort)) == null) {
                lastView = "MediatorPort"; break;
            }
            MediatorPort.setOnEditorActionListener ( this );


            if ((CheckBoxCustomMediator = findCheckBox(R.id.CheckBoxUseMediator)) == null) {
                lastView = "CheckBoxCustomMediator"; break;
            }

            if ((CheckBoxDefaultMediator = findCheckBox(R.id.CheckBoxHCMMediator)) == null) {
                lastView = "CheckBoxDefaultMediator"; break;
            }

            if ((CheckBoxAutoPortal = findCheckBox(R.id.CheckBoxAutoPortal)) == null) {
                lastView = "CheckBoxAutoPortal"; break;
            }

            if ((CheckBoxNativeResolution = findCheckBox(R.id.CheckBoxNativeResolution)) == null) {
                lastView = "CheckBoxNativeResolution"; break;
            }

            if ((CheckBoxTcpPortal = findCheckBox(R.id.CheckBoxTcpPortal)) == null) {
                lastView = "CheckBoxTcpPortal"; break;
            }

            if ((CheckBoxNavigationButtons = findCheckBox(R.id.CheckBoxNavigationButtons)) == null) {
                lastView = "CheckBoxNavigationButtons"; break;
            }

            if ((checkBoxShowDebugStatus = findCheckBox(R.id.checkBoxShowDebugStatus)) == null) {
                lastView = "checkBoxShowDebugStatus"; break;
            }

            if ((checkBoxLogfile = findCheckBox(R.id.checkBoxLogfile)) == null) {
                lastView = "checkBoxLogfile"; break;
            }

            success = true;
        }
        while ( false );

        if ( !success ) {
            Utils.LogE ( className, "onCreateView: Failed to find " + lastView + " !" );
        }
        return v;
    }


    @SuppressWarnings ( "all" )
    @Override
    public void onCheckedChanged ( CompoundButton button, boolean isChecked )
    {
        Environs env = TabActivity.env;
        if ( env == null ) return;

        switch(button.getId ())
        {
            case R.id.checkBoxVideo:
                env.SetUseStream(isChecked);
                break;

            case R.id.checkBoxNativeVideo:
                if (!env.SetUseNativeDecoder(isChecked)) {
                    button.setChecked(false);
                }
                break;

            case R.id.checkBoxHardwareDecoder:
                env.SetUseHardwareEncoder(isChecked);
                break;

            case R.id.checkBoxPush:
                env.SetUsePushNotifications ( isChecked );
                break;

            case R.id.CheckBoxSensor:
                env.SetUseSensors(isChecked);
                break;

            case R.id.checkBoxTLSDevices:
                env.SetUseCLSForDevices(isChecked);
                break;

            case R.id.CheckBoxUseMediator:
                env.SetUseCustomMediator(isChecked);
                MediatorIP.setEnabled(isChecked);
                MediatorText.setEnabled(isChecked);
                MediatorPort.setEnabled(isChecked);
                MediatorTextPort.setEnabled(isChecked);

                if (isChecked) {
                    MediatorIP.setText(env.GetMediatorIP());
                    MediatorPort.setText("" + env.GetMediatorPort());
                }
                break;

            case R.id.CheckBoxHCMMediator:
                env.SetUseDefaultMediator(isChecked);
                break;

            case R.id.CheckBoxAutoPortal:
                env.SetPortalAutoStart(isChecked);
                break;

            case R.id.CheckBoxNativeResolution:
                env.SetPortalNativeResolution(isChecked);
                break;

            case R.id.CheckBoxTcpPortal:
                env.SetPortalTCP(isChecked);
                break;

            case R.id.CheckBoxNavigationButtons:
                env.opt("optNavigationButtons", isChecked ? "1" : "0");
                break;

            case R.id.checkBoxShowDebugStatus:
                Environs.SetUseNotifyDebugMessageN ( isChecked );
                break;

            case R.id.checkBoxLogfile:
                env.SetUseLogFile(isChecked);
                break;
        }
    }

    @Override
    public boolean onEditorAction ( TextView textView, int actionId, KeyEvent event )
    {
        Environs env = TabActivity.env;
        if (env == null)
            return false;

        InputMethodManager in = (InputMethodManager) getSherlockActivity().getSystemService(Context.INPUT_METHOD_SERVICE);

        boolean success = false;

        switch ( textView.getId () )
        {
            case R.id.MediatorIP:
            case R.id.MediatorPort:
            {
                String sport = MediatorPort.getText().toString();
                int port = 0;
                try {
                    port = Integer.parseInt(sport);
                }
                catch (NumberFormatException e){
                    e.printStackTrace ();
                }

                if (event != null) {
                    env.SetMediator(MediatorIP.getText().toString(), port);
                }
                if (actionId == EditorInfo.IME_ACTION_DONE || (event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))) {
                    Utils.Log ( 3, className, "onEditorAction: Hiding keyboard. value = " + textView.getText ( ).toString ( ) );

                    env.SetMediator(MediatorIP.getText().toString(), port);
                    success = true;
                }
                break;
            }
        }

        if (success) {
            in.hideSoftInputFromWindow(textView.getApplicationWindowToken(), 0);
            textView.clearFocus ( );
        }
        return success;
    }

    CheckBox findCheckBox(int viewID)
    {
        CheckBox v;

        if ((v = (CheckBox) mainTabView.findViewById(viewID)) == null) {
            return null;
        }
        v.setOnCheckedChangeListener ( this );
        return v;
    }

    View findView(int viewID)
    {
        View v;

        if ((v = mainTabView.findViewById(viewID)) == null) {
            return null;
        }
        return v;
    }


    public static void Enable()
    {
    	if (instance == null)
    		return;

        Environs env = TabActivity.env;
        if (env == null)
            return;

        Enable(mainTabView);

        UpdateCheckBox(checkBoxVideo, env.GetUseStream ( ), false);

        UpdateCheckBox ( checkBoxNativeVideo, env.GetUseNativeDecoder ( ), true );

        UpdateCheckBox ( checkBoxHardwareDecoder, env.GetUseHardwareEncoder ( ), true );

        UpdateCheckBox ( CheckBoxNativeResolution, env.GetPortalNativeResolution ( ), true );

        UpdateCheckBox ( checkBoxPush, env.GetUsePushNotifications ( ), true );

        UpdateCheckBox ( CheckBoxSensor, env.GetUseSensors ( ), true );

        UpdateCheckBox ( CheckBoxAutoPortal, env.GetPortalAutoStart ( ), false );

        UpdateCheckBox ( CheckBoxDefaultMediator, env.GetUseDefaultMediator ( ), true );

        UpdateCheckBox ( checkBoxTLSDevices, env.GetUseCLSForDevices ( ), true );

        UpdateCheckBox ( checkBoxLogfile, env.GetUseLogFile ( ), true );

        UpdateCheckBox ( checkBoxShowDebugStatus, Environs.GetUseNotifyDebugMessageN ( ), true );

    	if (CheckBoxCustomMediator != null) {
    		CheckBoxCustomMediator.setEnabled(true);
    		
            boolean useMediator = env.GetUseCustomMediator();
            CheckBoxCustomMediator.setChecked ( useMediator );
            
    		if ( useMediator ) {
                UpdateEditText(MediatorIP, env.GetMediatorIP ( ), true);

                Enable(MediatorText);

                UpdateEditText(MediatorPort, String.valueOf (  env.GetMediatorPort()), true);

                Enable(MediatorTextPort);
    		}
    	}

        UpdateCheckBox ( CheckBoxTcpPortal, env.GetPortalTCP ( ), false );

        UpdateCheckBox ( CheckBoxNavigationButtons, env.opt ( "optNavigationButtons" ).equalsIgnoreCase ( "1" ), true );

        UpdateEditText ( MediatorUserName, env.GetMediatorUserName ( ), true );

        Enable ( MediatorPassword );
    }

    public static void Disable()
    {
    	if (instance == null)
    		return;

        View views [] = {mainTabView, MediatorIP, MediatorText, MediatorPort, MediatorTextPort,
                MediatorUserName, MediatorPassword, checkBoxNativeVideo, checkBoxHardwareDecoder,
                checkBoxPush, CheckBoxSensor, CheckBoxCustomMediator, CheckBoxDefaultMediator, CheckBoxNativeResolution};

        for (View v : views) {
            Disable(v);
        }
    }


    /*
     * Helper methods
     */
    static void Enable(View v) {
        if (v != null)
            v.setEnabled (true);
    }
    static void Disable(View v) {
        if (v != null)
            v.setEnabled ( false );
    }

    static void UpdateCheckBox(CheckBox check, boolean value, boolean enable)
    {
        if (check == null)
            return;
        if (enable)
            check.setEnabled ( true );
        if (check.isChecked ( ) != value)
            check.setChecked ( value );
    }

    static void UpdateEditText(EditText check, String value, boolean enable)
    {
        if (check == null)
            return;
        if (enable)
            check.setEnabled ( true );

        try {
            if ( !check.getText ().toString ().contentEquals ( value ) )
                check.setText ( value );
        }
        catch ( Exception ex ) {
            ex.printStackTrace ();
        }
    }
}
