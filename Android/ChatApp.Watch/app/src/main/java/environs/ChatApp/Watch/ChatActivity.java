package environs.ChatApp.Watch;
/**
 *	ChatActivity
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
import android.database.Cursor;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.support.wearable.activity.WearableActivity;
import android.support.wearable.view.BoxInsetLayout;
import android.util.Base64;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;

import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import environs.*;
import environs.ChatApp.Watch.R;

/**
 * ChatActivity show a list of available ChatUsers.
 * On click of one of them, we create a new Activity to handle a chat session.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class ChatActivity extends WearableActivity implements ListObserver, DeviceObserver {

    private static String className = "ChatActivity";

    static String loginUserName = "Unknown";
    static String statusMessage = "Hi, there!";
    static String userImageBase64;

    static Environs environs;
    static ChatActivity instance;
    static DeviceList adapter;

    static Menu menu;

    ListView listView;

    private static final SimpleDateFormat AMBIENT_DATE_FORMAT =
            new SimpleDateFormat("HH:mm", Locale.US);

    private BoxInsetLayout mContainerView;
    private TextView mTextView;
    private TextView mClockView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate ( savedInstanceState );
        setContentView ( R.layout.activity_chat );
        setAmbientEnabled();

        mContainerView = (BoxInsetLayout) findViewById(R.id.container);
//        mTextView = (TextView) findViewById(R.id.text);

        instance = this;

        environs = Environs.CreateInstance ( this, "ChatApp", "Environs" );

        if (environs == null) {
            Utils.LogE ( "Environs library is not installed correctly!" );
            return;
        }

        loginUserName = GetUserName();
        LoadUserImage ( );

        environs.SetMediatorFilterLevel ( Environs.MEDIATOR_FILTER_NONE );

        // Add our observer
        environs.AddObserver(new Observer());

        // Start Environs
        environs.Start();

        AddListAdapter();
    }

    @Override
    public void onEnterAmbient(Bundle ambientDetails) {
        super.onEnterAmbient(ambientDetails);
        updateDisplay();
    }

    @Override
    public void onUpdateAmbient() {
        super.onUpdateAmbient ( );
        updateDisplay();
    }

    @Override
    public void onExitAmbient() {
        updateDisplay ( );
        super.onExitAmbient ( );
    }

    private void updateDisplay() {
        if (isAmbient()) {
            mContainerView.setBackgroundColor(getResources().getColor(android.R.color.black));
            mTextView.setTextColor(getResources().getColor(android.R.color.white));
            mClockView.setVisibility(View.VISIBLE);

            mClockView.setText(AMBIENT_DATE_FORMAT.format(new Date()));
        } else {
            mContainerView.setBackground(null);
            mTextView.setTextColor(getResources().getColor(android.R.color.black));
            mClockView.setVisibility(View.GONE);
        }
    }


    public void AddListAdapter()
    {
        listView = (ListView) findViewById(R.id.UserList);
        if (listView == null)
            return;

        // Create the device list
        adapter = environs.CreateDeviceList ( this,
                Environs.MEDIATOR_DEVICE_CLASS_ALL, // for all available devices and
                this,                                // add ourself as an observer
                0, 0);

        // You may use default layout items and ids as for regular BaseAdapter
        // For that case, the following prototype arguments could be used
        //R.layout.user_list_item, android.R.id.text1);

        // Use the devicelist as an adapter for the ListView
        listView.setAdapter(adapter);


        // ListView Item Click Listener
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                // Click on item (DeviceInstance)
                DeviceInstance device = (DeviceInstance) listView.getItemAtPosition(position);
                if (device == null || device.appContext1 == null)
                    return;

                MessagesActivity.chatUser = (ChatUser) device.appContext1;

                Intent intent = new Intent(ChatActivity.this, MessagesActivity.class);
                startActivity(intent);
            }
        });
    }


    @Override
    public void OnDeviceChanged(DeviceInstance sender, int Environs_NOTIFY_) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                adapter.notifyDataSetChanged();
            }
        });
    }


    @Override
    public void OnListChanged(ArrayList<DeviceInstance> vanished, ArrayList<DeviceInstance> appeared) {

        if (appeared != null) {
            for (int i=0; i<appeared.size(); i++)
            {
                // Get the device that appeared in the list
                DeviceInstance device = appeared.get(i);

                device.AddObserver(this);

                // Initialize a ChatUser and attach it to the device
                ChatUser.InitWithDevice(device);
            }
        }
    }


    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The PortalInstance object.
     */
    public void OnPortalRequestOrProvided(PortalInstance portal)
    {

    }


    @SuppressLint("NewApi")
    public String GetUserName ()
    {
        String username = "Unknown";

        try {
            Cursor c = getApplication().getContentResolver().query(ContactsContract.Profile.CONTENT_URI, null, null, null, null);
            if (c.moveToFirst()) {
                username = c.getString(c.getColumnIndex("display_name"));
                c.close();
                Utils.Log(1, className, "GetUserName: " + username);
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        return username;
    }


    public void LoadUserImage()
    {
        try {
            InputStream stream = getAssets().open("user.png");

            int size = stream.available();
            if (size <= 0)
                return;

            byte[] buffer = new byte[size];
            stream.read(buffer);
            stream.close();

            userImageBase64 = Base64.encodeToString(buffer, Base64.DEFAULT);

            Utils.Log(1, className, userImageBase64);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
