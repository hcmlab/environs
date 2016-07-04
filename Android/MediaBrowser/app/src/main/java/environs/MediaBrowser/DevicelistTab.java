package environs.MediaBrowser;
/**
 *	DevicelistTab
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
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;

import com.actionbarsherlock.app.SherlockFragment;

import environs.*;

/**
*	Device list
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class DevicelistTab extends SherlockFragment
    {
        private static String className = "DevicelistTab";

        static DevicelistTab instance = null;

        static ViewGroup mainTabView = null;
        static Button btnRefreshDevices = null;
        static ListView deviceListView = null;
        static DeviceList adapter = null;
			
    /**
     * When creating, retrieve this instance's number from its arguments.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Utils.Log ( 4, className, "onCreate" );

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

        View v = inflater.inflate( R.layout.devicelist, container, false);
        if (v == null) {
        	Utils.LogE ( className, "onCreateView: Failed to get main layout!" );   
    		return null;
        }
        
        ViewGroup layout = (ViewGroup) v.findViewById(R.id.test_layout);
        if (layout == null) {
        	Utils.LogE ( className, "onCreateView: Failed to find viewgroup of main layout!" );   
    		return v;
        }
        mainTabView = layout;
        
        deviceListView = (ListView) layout.findViewById(R.id.DeviceList);
        if (deviceListView == null) {
        	Utils.LogE ( className, "onCreateView: Failed to find deviceListView!" );
    		return v;
        }
        
        // Send button
        btnRefreshDevices = (Button) layout.findViewById(R.id.button_refresh);
        if (btnRefreshDevices == null) {
        	Utils.LogE ( className, "onCreateView: Failed to find device list refresh button!" );   
    		return v;
        }

        btnRefreshDevices.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                updateUI();
            }
        });
        
		// ListView Item Click Listener        
        deviceListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                // ListView Clicked item value
                DeviceInstance device = (DeviceInstance) deviceListView.getItemAtPosition(position);
                if (device == null)
                    return;

                DeviceActivity.device = device;
                Intent intent = new Intent(TabActivity.instance, DeviceActivity.class);
                startActivity(intent);
            }
        });
        return v;
    }


    public static void InitDeviceList ()
    {
        final Environs env = TabActivity.env;
        if (env == null)
            return;

        if (instance != null) {
            instance.getSherlockActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    adapter = env.CreateDeviceList(instance.getSherlockActivity(), DeviceClass.All, null,
                            R.layout.device_list_item, android.R.id.text1);

                    deviceListView.setAdapter(adapter);
                }
            });
        }
    }


    public void updateUI ()
    {
		if (adapter == null)
			return;

    	new Thread(new Runnable() {
            public void run(){
                adapter.Reload();
            }
        }).start();
    }
    
    
    @Override
    public void onResume() {    
    	Utils.Log ( 4, className, "onResume" );
        super.onResume();

        updateUI();
    }



    @Override
    public void onDestroy () {
        Utils.Log ( 4, className, "onDestroy" );

        super.onDestroy();
    }
}
