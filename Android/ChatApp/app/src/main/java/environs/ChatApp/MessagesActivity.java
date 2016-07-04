package environs.ChatApp;
/**
 *	MessagesActivity
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
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import environs.DeviceInstance;


/**
 * MessagesActivity represents the active chat session with a ChatUser.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class MessagesActivity extends Activity {

	//private static String className = "MessagesActivity";

	ListView	listView;
	EditText messageText;
	Button btnSend;

	public static BaseAdapter adapter;
	public static ChatUser chatUser;
	public static MessagesActivity instance;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView( R.layout.messages_list);

		instance = this;

		listView = (ListView) findViewById( R.id.MessageList);
		if (listView == null)
			return;

		messageText = (EditText) findViewById(R.id.MessageToSend);
		if (messageText == null)
			return;

		messageText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			@Override
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if (actionId == EditorInfo.IME_ACTION_DONE || (event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))) {
					InputMethodManager in = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

					in.hideSoftInputFromWindow(v.getApplicationWindowToken(), 0);

					v.clearFocus();
					btnSend.requestFocus();
					return true;

				}
				return false;
			}
		});

		// Send button
		btnSend = (Button) findViewById(R.id.buttonSend);
		if (btnSend == null)
			return;

		btnSend.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				SendMessage();
			}
		});

		// Delete button
		Button buttonDelete = (Button) findViewById(R.id.buttonDelete);
		if (buttonDelete == null)
			return;

		buttonDelete.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				if (chatUser != null && chatUser.device != null) {
					chatUser.device.ClearMessages();
					chatUser.device.ClearStorage();
				}
			}
		});

		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);

		adapter = new MessagesAdapter ( this, chatUser.messages );

		listView.setAdapter(adapter);
	}


	@Override
	protected void onDestroy () {
		instance = null;
		super.onDestroy();
	}


	void SendMessage() {
		String msg = messageText.getText().toString();
		ChatUser chat = chatUser;

		if (chat == null)
			return;

		DeviceInstance device = chat.device;
		if ( device == null )
			return;

		device.SendMessage(msg);
	}


	public void UpdateMessages(ChatUser user) {
		if (chatUser != user)
			return;

		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				adapter.notifyDataSetChanged();

				listView.post(new Runnable() {
					@Override
					public void run() {
						listView.setSelection(adapter.getCount() - 1);
					}
				});
			}
		});
	}
}
