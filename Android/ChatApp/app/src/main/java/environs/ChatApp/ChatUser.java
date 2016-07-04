package environs.ChatApp;
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

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import environs.*;

/**
 * ChatUser represents a particular user in the application environment,
 * that interacts with us.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class ChatUser implements MessageObserver, DeviceObserver
{
    private static String className = "ChatUser . . . . .";

    private static final String unknownUser = "Unknown";
    private static final String unknownStatus = "No Status";

    static boolean showDeviceInfo = false;
    DeviceInstance device;

    String userName             = unknownUser;
    Bitmap profilePic;
    String lastStatus           = unknownStatus;
    String lastMessage;

    boolean enabled             = true;
    boolean userReady           = false;
    long lastStatusRequested    = 0;
    long profilePicRequested    = 0;
    long userNameRequested      = 0;

    int availableDetails        = 0;

    public int initState        = 0;
    public int pingsSent        = 0;
    public long lastPingSent    = 0;
    public int updatesSent      = 0;
    public long lastUpdateSent  = 0;

    //@SuppressWarnings ( "all" )
    //final static ArrayList chatUsers = new ArrayList (  );


    @SuppressWarnings ( "all" )
    ArrayList<MessageInstance> messages = new ArrayList<MessageInstance>();


    @SuppressWarnings ( "all" )
    boolean IsChatCommand(String msg) {
        if (msg.length() < 5)
            return false;

        return msg.startsWith("$ca$");
    }


    boolean HandleChatCommand(String msg, boolean processIncoming)
    {
        char type = msg.charAt(4);
        Utils.Log ( 6, className, "HandleChatCommand: " + type );

        boolean success = false;

        if (type == '1') { // Update the username if required
            if ( IsUserNameEmpty ( ) || processIncoming) {
                userName = msg.substring ( 6 );
                success = true;
                availableDetails |= 0x1;
            }
        }
        else if (type == '3') { // Update the profile picture if required
            if (profilePic == null || processIncoming )
            {
                try {
                    String profilePicStr = msg.substring(6);

                    if ( profilePicStr.length() > 0 )
                    {
                        // Decode the base64 sting to a byte array

                        //byte[] imageBytes = Base64.decode(profilePicStr, Base64.DEFAULT);

                        // WARNING: Call to BitmapFactory.decodeByteArray creates high cpu load which never drops
                        // when feed with data that originates from windows/ios/osx
                        // We will disable this for now and take the "Android" bitmap
                        byte[] imageBytes = Base64.decode ( ChatActivity.userImageBase64, Base64.DEFAULT );
                        if ( imageBytes != null ) {
                            // Let's use the BitmapFactory to create a bitmap
                            Bitmap bitmap = BitmapFactory.decodeByteArray(imageBytes, 0, imageBytes.length);
                            if (bitmap != null) {
                                profilePic = bitmap;
                                success = true;
                                availableDetails |= 0x4;
                            }
                        }
                    }
                }
                catch (Exception ex) {
                    ex.printStackTrace();
                    Utils.LogE ( "HandleChatCommand: String length [ " + msg.length () + " ]" );
                }
            }
        }
        else if (type == '5') { // Update the status message if required
                if ( IsStatusEmpty ( ) || processIncoming) {
                    lastStatus = msg.substring ( 6 );
                    success = true;
                    availableDetails |= 0x2;
                }
            }
            else if (processIncoming) // If this is an incoming request, then reply accordingly
                {
                    if (type == '0') { // Reply with our username
                        Send ( "$ca$1$" + ChatActivity.loginUserName );
                    }
                    else if (type == '2') { // Reply with a base64 encoded string of our profile picture
                        Send ( "$ca$3$" + ChatActivity.userImageBase64 );
                    }
                    else if (type == '4') { // Reply with our status message
                            Send("$ca$5$" + ChatActivity.statusMessage );
                        }
                        else if (type == '8') // Send ready
                            {
                                if ( !userReady ) {
                                    userReady = true;
                                    Send ( "$ca$8$" );
                                }
                            }
                    RequestProfile ();
                }
        return success;
    }


    /*public void finalize() {
        try {
            Utils.Log ( 6, className, "finalize" );

            super.finalize ( );

            DeInit ( true );

        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }
    */

/*
    public static void DisposeChatUsers () {
        int count = 1;

        while ( count > 0 ) {
            ChatUser chatUser = null;

            synchronized ( chatUsers ) {
                count = chatUsers.size ();

                if (count > 0) {
                    chatUser = (ChatUser ) chatUsers.remove ( 0 );
                }
            }

            if (chatUser != null) {
                DeviceInstance userDevice = chatUser.device;

                if (userDevice != null) {
                    synchronized ( userDevice ) {
                        chatUser.DeInit ( true );
                    }
                }
                else
                    chatUser.DeInit ( true );
            }
        }
    }


    public static void CheckChatUsers () {
        int count = 1;

        ArrayList toDispose = new ArrayList (  );

        synchronized ( chatUsers )
        {
            for ( int i = 0; i < chatUsers.size (); i++ )
            {
                ChatUser chatUser = (ChatUser) chatUsers.get ( i );
                if (!chatUser.enabled || chatUser.device == null || chatUser.device.disposed )
                {
                    toDispose.add ( chatUser );
                    chatUsers.remove ( i );
                    i--;
                }
            }
        }

        for ( int i = 0; i < toDispose.size (); i++ )
        {
            ChatUser chatUser = (ChatUser) toDispose.get ( i );

            DeviceInstance userDevice = chatUser.device;

            if (userDevice != null) {
                synchronized ( userDevice ) {
                    chatUser.DeInit ( true );
                }
            }
            else
                chatUser.DeInit ( true );
        }
    }
    */


    public void DeInit ( boolean wait )
    {
        Utils.Log ( 6, className, "DeInit" );

        enabled = false;

        DeviceInstance userDevice = this.device;

        if (userDevice != null) {
            userDevice.RemoveObserver ( this );
            userDevice.RemoveObserverForMessages ( this );
        }

        messages.clear ();

        if (userDevice != null) {
            userDevice.appContext1 = null;
        }

        this.device = null;

        /*synchronized ( chatUsers ) {
            chatUsers.remove ( this );
        }*/
    }


    public static ChatUser InitWithDevice(DeviceInstance userDevice) {
        if (userDevice == null)
            return null;

        Utils.Log ( 6, className, "InitWithDevice" );

        // Create a ChatUser object and attach it as the appContext to the DeviceInstance
        final ChatUser chatUser = new ChatUser();
        chatUser.device = userDevice;
        userDevice.appContext1 = chatUser;

        /*synchronized ( chatUsers ) {
            chatUsers.add ( chatUser );
        }*/

        return chatUser;
    }


    public boolean Init()
    {
        Utils.Log ( 6, className, "Init" );

        DeviceInstance userDevice = this.device;

        if ( userDevice == null || userDevice.disposed )
            return false;

        if ( initState == 0 )
        {
            userDevice.appContext1 = this;

            // Let's observe device changes, e.g. disposal, connections, etc..
            userDevice.AddObserver ( this );

            // Let's listen to messages from this device
            userDevice.AddObserverForMessages ( this );

            boolean doNotify = false;

            ArrayList msgs = userDevice.GetMessagesInStorage ();
            if (msgs != null)
            {
                if (userDevice.disposed)
                    return true;

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
                            if (lastMessage == null && !msgInst.sent) {
                                lastMessage = msg;
                                doNotify = true;
                            }

                            messages.add(0, msgInst);
                            continue;
                        }
                        if (msgInst.sent)
                            continue;
                        if (HandleChatCommand(msg, false))
                            doNotify = true;
                    }
                }
                catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            Utils.Log ( 6, className, "Init: Load storage done" );

            userDevice.DisposeStorageCache ();

            userDevice.ClearMessages ();
            userDevice.ClearStorage ();

            if (userDevice.disposed)
                return true;

            if (doNotify) {
                ChatActivity.UpdateList ( );
            }

            if ( userDevice.sourceType == DeviceSourceType.Broadcast )
            {
                /// Ask user whether he/she is ready for commands
                userDevice.SendMessage ( "$ca$8" );

                initState = 1;
            }
            else {
                initState = 2; lastUpdateSent = System.currentTimeMillis();
                return false;
            }
        }


        if ( initState == 1 )
        {
            long now = System.currentTimeMillis();

            if ( ( now - lastPingSent ) < 1000 )
                return false;
            lastPingSent = now;

            if ( userReady || !enabled || userDevice.disposed || userDevice.isMessageObserverReady () )
                return true;

            if ( userDevice.sourceType == DeviceSourceType.Broadcast )
            {
                /// Ask user whether he/she is ready for commands
                userDevice.SendMessage (  "$ca$8" );

                pingsSent++;

                if ( pingsSent > 20 ) {
                    initState = 2; lastUpdateSent = System.currentTimeMillis();
                    return false;
                }
                else
                    return false;
            }
            else {
                initState = 2; lastUpdateSent = System.currentTimeMillis();
                return false;
            }
        }


        if ( initState == 2 )
        {
            if ( !NeedsUpdate () ) {
                ChatActivity.UpdateList ( );
                return true;
            }

            long now = System.currentTimeMillis();

            if ( ( now - lastUpdateSent ) < 10000 )
                return false;
            lastUpdateSent = now;


            if ( !enabled || userDevice.disposed )
                return true;

            RequestProfile ();
            updatesSent++;

            return updatesSent > 500;
        }

        Utils.Log ( 6, className, "Init: done" );
        return true;
    }


    boolean NeedsUpdate ()
    {
        return (availableDetails < 3);
    }


    void Send (String msg)
    {
        if (!enabled)
            return;
        device.SendMessage(msg);
    }


    boolean IsStatusEmpty ( )
    {
        return (lastStatus == null || lastStatus.contentEquals ( unknownStatus ) || lastStatus.length () == 0 );
    }

    boolean IsUserNameEmpty ( )
    {
        return (userName == null || userName.length () == 0 || userName.contentEquals ( unknownUser ) || userName.contentEquals ( "Loading ..." ) );
    }

    public void RequestProfile ()
    {
        if ( availableDetails >= 7 )
            return;

        DeviceInstance userDevice = this.device;

        if ( userDevice == null || userDevice.disposed )
            return;

        boolean observerReady = userDevice.isMessageObserverReady ();

        boolean ready = (userReady || observerReady || userDevice.sourceType == DeviceSourceType.Broadcast);

        long now = System.currentTimeMillis();

        if ( IsUserNameEmpty ( ) && ( userNameRequested == 0 || ( now - userNameRequested > 1000 ) ) ) {
            if (ready) {
                // Username not found. Let's request that.
                Send ( "$ca$0" );
                if (observerReady)
                    userNameRequested = now;
                userName = "Loading ...";
            }
        }

        if ( IsStatusEmpty ( ) && ( lastStatusRequested == 0 || ( now - lastStatusRequested > 1000 ) )) {
            // Status message not found. Let's request that.
            if (ready) {
                Send ( "$ca$4" );
                if (observerReady)
                    lastStatusRequested = now;
            }
        }

        if (profilePic == null && ( profilePicRequested == 0 || ( now - profilePicRequested > 1000 ) ) ) {
            // Profile picture not found. Let's request that.
            if (observerReady)
                profilePicRequested = now;
            Send("$ca$2" );
        }
    }


    public void OnDeviceChanged ( DeviceInstance sender, int flags )
    {
        if ( flags == DeviceInfoFlag.Disposed ) {
            if ( sender != null ) {
                synchronized ( sender ) {
                    DeInit ( false );
                }
            }
        }
        else if ( flags == DeviceInfoFlag.Flags ) {
            RequestProfile ();
        }
        else if ( (flags & DeviceInfoFlag.IsConnected) != 0 ) {
                RequestProfile ();

                ChatActivity.UpdateList ( );
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


    static boolean messageThreadRun = true;

    static Thread messageThread = null;

    static final LinkedList messageQueue = new LinkedList();


    static class MessageContext
    {
        public ChatUser chatUser;
        public MessageInstance msg;
    }


    public static void StartMessageThread ()
    {
        if (messageThread != null)
            return;

        messageThread = new Thread(new Runnable() {
            public void run(){
                MessageThread ( );
            }
        });
        messageThread.start ( );
    }


    static void MessageThread ()
    {
        messageThreadRun = true;

        MessageContext ctx;

        while ( messageThreadRun )
        {
            synchronized (messageQueue)
            {
                if ( messageQueue.size() == 0 )
                {
                    try {
                        messageQueue.wait ();
                        continue;
                    } catch ( InterruptedException e ) {
                        e.printStackTrace ( );
                    }
                    break;
                }
                else {
                    ctx = (MessageContext) messageQueue.remove();

                    //Utils.Log(6, className, "MessageThread: Dequeue");
                }
            }

            if ( ctx != null )
            {
                ChatUser chatUser = ctx.chatUser;

                if ( chatUser.enabled ) {
                    DeviceInstance userDevice = chatUser.device;

                    if ( userDevice != null && !userDevice.disposed )
                        chatUser.HandleMessage ( ctx.msg );
                }

                //Utils.Log ( 6, className, "MessageThread: next" );
            }
        }

        messageThread = null;

        Utils.Log ( 6, className, "MessageThread: done" );
    }


    public void HandleMessage(MessageInstance msg)
    {
        if ( IsChatCommand(msg.text) ) {
            if ( !msg.sent && HandleChatCommand(msg.text, true) ) {
                ChatActivity.UpdateList ( );
            }
            return;
        }

        if (!msg.sent) {
            lastMessage = msg.text;
            ChatActivity.UpdateList ( );
        }

        messages.add(msg);

        if (MessagesActivity.instance != null) {
            MessagesActivity.instance.UpdateMessages(this);
        }
    }


    /**
     * OnMessage is called whenever a text message has been received from a device.
     *
     * @param msg                           The corresponding message object of type MessageInstance
     * @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
     */
    @SuppressWarnings ( "unchecked" )
    public void OnMessage(MessageInstance msg, int MESSAGE_INFO_ATTR_changed )
    {
        MessageContext ctx = new MessageContext ();

        ctx.chatUser = this;
        ctx.msg = msg;

        synchronized (messageQueue) {
            DeviceInstance device = this.device;

            if ( !enabled || device == null || device.disposed )
                return;

            messageQueue.add ( ctx );

            messageQueue.notify ( );
        }
    }


    public String GetUserText()
    {
        DeviceInstance userDevice = device;

        if (showDeviceInfo) {
            if (userDevice != null)
                return userDevice.toString ();
        }

        String connected = "";

        if (userDevice != null && userDevice.isConnected)
            connected = "* ";

        String text = connected + (userName != null ? userName : unknownUser);
        if (lastStatus != null)
            text += " (" + lastStatus + ")";

        text += "\n" + (lastMessage == null ? "" : lastMessage);

        return text;
    }

}
