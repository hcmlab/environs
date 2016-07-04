package environs.ChatApp.Watch;
/**
 *	ChatUser
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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Base64;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;

import environs.*;
import environs.ChatApp.Watch.R;

/**
 * ChatUser represents a particular user in the application environment,
 * that interacts with us.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class ChatUser implements MessageObserver, DeviceObserver, ViewGenerator
{
    private static String className = "ChatUser";

    DeviceInstance device;

    String userName;
    Bitmap profilePic;
    String lastStatus;
    String lastMessage;

    ArrayList<MessageInstance> messages = new ArrayList<MessageInstance>();


    boolean IsChatCommand(String msg) {
        if (msg.length() < 5)
            return false;

        return msg.startsWith("$ca$");
    }


    boolean HandleChatCommand(String msg, boolean processIncoming) {
        char type = msg.charAt(4);
        if (type == '1') { // Update the username if required
            if (userName == null || processIncoming)
                userName = msg.substring(6);
            return true;
        }
        else if (type == '3') { // Update the profile picture if required
            if (profilePic == null || processIncoming)
            {
                String profilePicStr = msg.substring(6);
                try {
                    if ( profilePicStr != null && profilePicStr.length() > 0 )
                    {
                    // Decode the base64 sting to a byte array
                        byte[] imageBytes = Base64.decode(profilePicStr, Base64.DEFAULT);
                        if ( imageBytes != null ) {
                            // Let's use the BitmapFactory to create a bitmap
                            Bitmap bitmap = BitmapFactory.decodeByteArray(imageBytes, 0, imageBytes.length);
                            if (bitmap != null)
                                profilePic = bitmap;
                        }
                    }
                }
                catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            return true;
        }
        else if (type == '5') { // Update the status message if required
            if (lastStatus == null || processIncoming)
                lastStatus = msg.substring(6);
            return true;
        }

        if (processIncoming) // If this is an incoming request, then reply accordingly
        {
            if (type == '0') { // Resonse with our username
                device.SendMessage("$ca$1$" + ChatActivity.loginUserName);
                return true;
            }
            else if (type == '2') { // Resonse with a base64 encoded string of our profile picture
                device.SendMessage("$ca$3$" + ChatActivity.userImageBase64);
                return true;
            }
            else if (type == '4') { // Resonse with our status message
                device.SendMessage("$ca$5$" + ChatActivity.statusMessage);
                return true;
            }
        }
        return false;
    }


    public static boolean InitWithDevice(DeviceInstance userDevice) {
        if (userDevice == null)
            return false;

        // Create a ChatUser object and attach it as the appContext to the DeviceInstance
        final ChatUser chatUser = new ChatUser();
        chatUser.device = userDevice;
        userDevice.appContext1 = chatUser;

        // Let's observe device changes, e.g. disposal, connections, etc..
        userDevice.AddObserver(chatUser);

        // Let's listen to messages from this device
        userDevice.AddObserverForMessages(chatUser);

        new Thread(new Runnable() {
            public void run(){
                chatUser.Init();
            }
        }).start();

        return true;
    }


    public boolean Init() {
        ArrayList msgs = device.GetMessages ();
        if (msgs != null)
        {
            try
            {
                for (int i = msgs.size() - 1; i >= 0; i--) {
                    // Iterate through the messagelist from the end to the start
                    // to get the most recent information at first
                    MessageInstance msgInst = (MessageInstance)msgs.get(i);
                    String msg = msgInst.text;

                    if (msg == null)
                        continue;

                    if (!IsChatCommand(msg))
                    {
                        if (lastMessage == null && !msgInst.sent)
                            lastMessage = msg;

                        messages.add(0, msgInst);
                        continue;
                    }
                    if (msgInst.sent)
                        continue;
                    HandleChatCommand(msg, false);
                }
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }

        if (userName == null) {
            // Username not found. Let's request that.
            device.SendMessage("$ca$0");
            userName = "Unknown";
        }

        if (profilePic == null) {
            // Profile picture not found. Let's request that.
            device.SendMessage("$ca$2");
        }

        if (lastStatus == null) {
            // Status message not found. Let's request that.
            device.SendMessage("$ca$4");
            lastStatus = "No Status";
        }

        device.NotifyAppContextChanged(0);
        return true;
    }


    public void OnDeviceChanged(DeviceInstance sender, int Environs_NOTIFY_) {
    }


    /**
     * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
     *
     * @param portal 		The PortalInstance object.
     */
    public void OnPortalRequestOrProvided(PortalInstance portal)
    {

    }


    /**
     * OnMessage is called whenever a text message has been received from a device.
     *
     * @param msg                           The corresponding message object of type MessageInstance
     * @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
     */
    public void OnMessage(MessageInstance msg, int MESSAGE_INFO_ATTR_changed) {
        if ( IsChatCommand(msg.text) ) {
            if ( !msg.sent && HandleChatCommand(msg.text, true) ) {
                device.NotifyAppContextChanged(0);
            }
            return;
        }

        if (!msg.sent) {
            lastMessage = msg.text;
            device.NotifyAppContextChanged(0);
        }

        messages.add(msg);

        if (MessagesActivity.instance != null) {
            MessagesActivity.instance.UpdateMessages(this);
        }
    }


    public String GetUserText()
    {
        return userName + " (" + lastStatus + ")\n" + (lastMessage == null ? "" : lastMessage);
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
