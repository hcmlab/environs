package environs.ChatApp;
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
import android.app.Activity;
import android.provider.ContactsContract;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import java.io.InputStream;
import java.lang.reflect.Array;
import java.util.ArrayList;

import environs.*;

/**
 * ChatActivity show a list of available ChatUsers.
 * On click of one of them, we create a new Activity to handle a chat session.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class ChatActivity extends Activity implements ListObserver, DeviceObserver, ViewGenerator {

	private static String className = "ChatActivity . . .";


	static String loginUserName = "Unknown";
	static String statusMessage = "Hi, there!";
	static String userImageBase64;

	static Environs environs;
	static ChatActivity instance;
	static DeviceList adapter;

	static boolean enabled 		= false;
	boolean 	userListEnabled = false;
	Thread		userListThread;

	final ArrayList	chatsToInit		= new ArrayList (  );
	boolean 	initThreadEnabled 	= false;
	Thread		initThread;

	static Menu menu;

	ListView listView;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView( R.layout.user_list);

		instance = this;

		/// Create an Environs instance so as to access the application environment
		environs = Environs.CreateInstance(this, "ChatApp", "Environs");

		if (environs == null) {
			Log.e("ChatApp", "Failed to create an Environs object!");
			return;
		}

		environs.ClearStorage ();
		environs.SetUseCLSForDevicesEnforce ( false );

		loginUserName = GetUserName();
		LoadUserImage ( );
        
        // Add our observer
		environs.AddObserver ( new Observer ( ) );

		AddListAdapterAndStart ( );
	}


	public void AddListAdapterAndStart ( )
	{
		listView = (ListView) findViewById( R.id.UserList);
		if (listView == null)
			return;

		enabled = true;

		// For UI performance improvements. Notify the adapter about device detail changes
		// only when we call NotifyAppContextChanged() on the device instance
		DeviceInstance.notifyPropertyChangedDefault = false;

		// Create the device list
		adapter = environs.CreateDeviceList ( this, this,
				DeviceClass.All, // for all available devices and
				this, 								// add ourselves as an observer
										   R.layout.user_list_item, R.id.text1 );

		// You may use default layout items and ids as for regular BaseAdapter
		// For that case, the following prototype arguments could be used
				//R.layout.user_list_item, android.R.id.text1);

		// Use the devicelist as an adapter for the ListView
		listView.setAdapter ( adapter );


		// ListView Item Click Listener
		listView.setOnItemClickListener ( new AdapterView.OnItemClickListener ( ) {
			@Override
			public void onItemClick ( AdapterView< ? > parent, View view, int position, long id ) {

				// Click on item (DeviceInstance)
				DeviceInstance device = ( DeviceInstance ) listView.getItemAtPosition ( position );
				if ( device == null || device.appContext1 == null )
					return;

				MessagesActivity.chatUser = ( ChatUser ) device.appContext1;

				Intent intent = new Intent ( ChatActivity.this, MessagesActivity.class );
				startActivity ( intent );
			}
		} );

		StartInitThread ();

		StartUserListThread ();

		ChatUser.StartMessageThread ();

		// Start Environs
		environs.Start ( );
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate ( R.menu.main, menu );

		ChatActivity.menu = menu;
		return true;
	}


	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case R.id.actionSettings:
				return true;

			case R.id.actionEnvStart:
				if (environs == null)
					break;

				if (environs.GetStatus() >= Status.Started )
				{
					enabled = false;

					environs.Stop();

					//ChatUser.DisposeChatUsers ();
				}
				else
					AddListAdapterAndStart ( );

				return true;

			case R.id.actionToggleChatTest:
				ChatUser.showDeviceInfo = !ChatUser.showDeviceInfo;
				break;

			case R.id.actionNotifications:
				if (environs == null)
					break;

				boolean state = environs.GetMediatorNotificationSubscription ();

				environs.SetMediatorNotificationSubscription ( !state );

				item.setTitle ( (state ? "Enable" : "Disable") + " Notifications" );
				return true;

			default:
				return super.onOptionsItemSelected(item);
		}
		return false;
	}


	@SuppressWarnings ( "all" )
	public static void UpdateList()
	{
		if ( instance == null || instance.userListThread == null )
			return;

		synchronized ( instance.userListThread ) {
			instance.userListThread.notify ();
		}
	}


	public void StartUserListThread ()
	{
		if (instance == null || instance.userListEnabled)
			return;

		userListThread = new Thread(new Runnable() {
			public void run(){
				 UserListThread ( );
			}
		});
		userListThread.start ( );
	}


	@SuppressWarnings ( "all" )
	public void UserListThread ()
	{
		userListEnabled = true;

		while (userListEnabled)
		{
			try {
				synchronized ( userListThread ) {
					userListThread.wait ();
				}
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
				break;
			}

			final DeviceList list = adapter;
			if ( list != null ) {
				instance.runOnUiThread ( new Runnable ( ) {
					@Override
					public void run ( ) {
						list.notifyDataSetChanged ( );
					}
				});
			}
			//ChatUser.CheckChatUsers ();
		}

		//ChatUser.DisposeChatUsers ();
	}


	public static void StartInitThread()
	{
		if (instance == null || instance.initThreadEnabled)
			return;

		instance.initThread = new Thread(new Runnable() {
			public void run(){
				instance.InitThread ( );
			}
		});
		instance.initThread.start ( );
	}


	@SuppressWarnings ( "all" )
	public void InitThread()
	{
		initThreadEnabled = true;
		int count;

		while ( initThreadEnabled )
		{
			int needsPingCount = 0;

			ArrayList chats;

			synchronized ( initThread )
			{
				chats = (ArrayList) chatsToInit.clone ();
			}

			if ( chats != null && chats.size () > 0 )
			{
				ArrayList chatsToRemove = new ArrayList ();

				for ( Object obj : chats )
				{
					ChatUser chat = (ChatUser) obj;

					if ( chat.Init () )
						chatsToRemove.add ( obj );
					else {
						if ( chat.initState == 1 )
							needsPingCount++;
					}
				}

				synchronized ( initThread )
				{
					if ( chatsToRemove.size () > 0 )
						chatsToInit.removeAll ( chatsToRemove );

					count = chatsToInit.size ();
				}
			}
			else
				count = 0;

			int timeout = -1;

			if (enabled) {
				if ( count > 0 )
				{
					if ( needsPingCount > 0 )
						timeout = 1000;
					else
						timeout = 10000;
				}
			}
			else {
				synchronized ( initThread ) {
					chatsToInit.clear ();
				}
			}

			//CLogArgN ( "OnListChanged: timeout [ %i ]", timeout );

			try {
				synchronized ( initThread ) {
					if ( timeout > 0 )
						initThread.wait ( timeout );
					else
						initThread.wait ();
				}
			} catch ( InterruptedException e ) {
				e.printStackTrace ( );
			}
		}

		synchronized ( initThread ) {
			chatsToInit.clear ();
		}
	}


	@SuppressWarnings ( "all" )
	@Override
	public void OnDeviceChanged(DeviceInstance device, int flags)
	{
		if ( flags == DeviceInfoFlag.Disposed && device != null ) {
			synchronized ( device ) {
				Object chatObj = device.appContext1;

				if (chatObj != null)
					((ChatUser) chatObj).DeInit ( false );
			}
		}
	}


	@SuppressWarnings ( "all" )
	@Override
	public void OnListChanged(ArrayList<DeviceInstance> vanished, ArrayList<DeviceInstance> appeared)
	{
		if (vanished != null) {
			for (int i=0; i<vanished.size(); i++)
			{
				// Get the device that appeared in the list
				DeviceInstance device = vanished.get(i);

				synchronized ( device ) {
					device.RemoveObserver(this);

					Object chatObj = device.appContext1;

					if (chatObj != null)
						((ChatUser) chatObj).DeInit ( false );
				}
			}
		}

		if (appeared != null) {
			for (int i=0; i<appeared.size(); i++)
			{
				// Get the device that appeared in the list
				DeviceInstance device = appeared.get(i);
				if ( device == null )
					continue;

				synchronized ( device ) {
					if ( device.appContext1 != null )
						continue;

					device.AddObserver(this);

					// Initialize a ChatUser and attach it to the device
					ChatUser chat = ChatUser.InitWithDevice(device);

					if ( chat != null )
					{
						synchronized ( chatsToInit ) {
							if (enabled)
								chatsToInit.add ( chat );
						}
					}
				}
			}

			if ( initThread != null ) {
				synchronized ( initThread ) {
					initThread.notify ( );
				}
			}
		}

		if (userListEnabled)
			UpdateList ();
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
			if (c != null && c.moveToFirst()) {
				username = c.getString(c.getColumnIndex("display_name"));
				c.close();
				Utils.Log(1, className, "GetUserName: " + username);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return username;
	}


	@SuppressWarnings ( "ResultOfMethodCallIgnored" )
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

			Utils.Log(6, className, userImageBase64);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}



	LayoutInflater inflater;
	int layout_id;
	int view_id;

	public void init(LayoutInflater inflater, int layout_id, int view_id)
	{
		//Log.d ( "ChatApp", "ChatUser.init" );

		this.inflater = inflater;
		this.layout_id = layout_id;
		this.view_id = view_id;
	}


	@SuppressWarnings ( "all" )
	public View getView(int position, DeviceInstance device, View convertView, ViewGroup parent) {
		do {
			try {
				//Log.d ( "ChatApp", "ChatUser.getView" );

				ChatUser chat = null;

				if (device != null && device.appContext1 != null && device.appContext1.getClass () == ChatUser.class )
					chat = (ChatUser) device.appContext1;

				if ( convertView == null ) {
					if (inflater == null)
						break;

					convertView = inflater.inflate( R.layout.user_list_item, null);
					if (convertView == null)
						break;
				}

				TextView tv = (TextView) convertView.findViewById(R.id.text1);
				if (tv == null)
					break;

				ImageView iv = (ImageView) convertView.findViewById( R.id.image1);

				if ( chat == null ) {
					tv.setText("Loading ...");
					if (iv != null)
						iv.setImageBitmap(null);
					break;
				}
				tv.setText(chat.GetUserText ( ));

				if (iv != null)
					iv.setImageBitmap ( chat.profilePic );

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		while (false);

		return convertView;
	}
}
