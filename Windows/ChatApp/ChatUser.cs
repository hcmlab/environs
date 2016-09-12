/**
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
#define USE_CHATUSER_IMAGESOURCE
//#define USE_UI_THREAD_IMAGESOURCE
//#define USE_CHATUSER_COPY_TO_IMAGESOURCE
//#define USE_CHATUSER_FREEZE_FLAG
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace environs.Apps
{
    class ChatUser : IDisposable
    {
        private const String className = "ChatUser . . . . . . . .";

        static ChatUser noUser = new ChatUser();
        static Collection<ChatUser> chatUsers = new Collection<ChatUser>();

        DeviceInstance device;

        String userName = @"Unknown";

#if USE_CHATUSER_IMAGESOURCE
        ImageSource profileBitmapImage = null;
#else
        BitmapImage profileBitmapImage = null;
#endif

#if USE_CHATUSER_FREEZE_FLAG
        bool profileBitmapImageFreezed = false;
#endif
        String lastStatus = "";
        String lastMessage = null;

        bool enabled = true;
        Thread chatInitThread = null;

        internal int initState = 0;
        internal int pingsSent = 0;
        internal long lastPingSent = 0;
        internal int updatesSent = 0;
        internal long lastUpdateSent = 0;
        
        String lastProfile = null;
        bool userReady = false;
        bool appContextChanged = false;
        int userReadyToSubmits = 3;
        
        int availableDetails = 0;
        int lastStatusRequested = 0;
        int profilePicRequested = 0;
        int userNameRequested = 0;

        public ObservableCollection<MessageInstance> messages = new ObservableCollection<MessageInstance>();

        internal static String loginUserName = Environment.UserName;

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                messages = null;
                profileBitmapImage = null;
            }
        }

        bool IsChatCommand(String msg)
        {
            if (msg == null || msg.Length < 5)
                return false;

            return msg.StartsWith("$ca$");
        }

        bool IsEmpty(String s)
        {
            return (s == null || s.Equals(""));
        }

        void Send(String msg)
        {
            if (!enabled)
                return;
            device.SendMessage(msg);
        }
        

        void HandleProfileImage(byte[] imageData)
        {
            Environs.dispatch(new Action(delegate ()
            {
                try
                {
                    var bs = (BitmapSource)new ImageSourceConverter().ConvertFrom(imageData);
                    if (bs != null)
                    {
#if USE_CHATUSER_IMAGESOURCE
                        profileBitmapImage = bs;
#endif
                        availableDetails |= 0x4;
                        appContextChanged = true;
                        ChatAppWindow.updaterThreadEvent.Set();
                    }
                }
                catch (Exception) { }
            }));
        }


        bool HandleChatCommand(String msg, bool processIncoming)
        {
            if (msg == null)
                return false;

            if (Utils.Log(4))
                Utils.Log(className, "HandleChatCommand: " + (msg.Length < 260 ? msg : msg.Substring(0,260)));

            bool success = false;

            char type = msg[4];
            if (type == '1') // Update the username if required
            {
                if (IsUserNameEmpty() || processIncoming)
                {
                    String name = msg.Substring(6);

                    if (userName != name)
                    {
                        userName = name;
                        success = true;
                        availableDetails |= 0x1;
                    }
                }
            }
            else if (type == '3') // Update the profile picture if required
            {
                if (profileBitmapImage == null || processIncoming)
                {
                    String profile = msg.Substring(6); // System.ExecutionEngineException (counter 1)

                    if (lastProfile == null || lastProfile.Length != profile.Length)
                    {
                        try
                        {
                            // Decode the base64 sting to a byte array
                            byte[] imageData = Convert.FromBase64String(profile);
                            if (imageData != null)
                            {
                                lastProfile = profile;

                                HandleProfileImage(imageData);
                            }
                        }
                        catch (Exception) { }
                    }
                }
            }
            else if (type == '5') // Update the status message if required
            {
                if (IsStatusEmpty() || processIncoming)
                {
                    String status = msg.Substring(6);

                    if (lastStatus != status)
                    {
                        lastStatus = status;
                        success = true;
                        availableDetails |= 0x2;
                    }
                }
            }
            else if (processIncoming) // If this is an incoming request, then reply accordingly
            {
                if (type == '0') // Resonse with our username 
                {
                    Send("$ca$1$" + loginUserName);
                }
                else if (type == '2') // Resonse with a base64 encoded string of our profile picture
                {
                    Send("$ca$3$" + ChatAppWindow.userImageBase64);
                }
                else if (type == '4') // Resonse with our status message
                {
                    Send("$ca$5$" + ChatAppWindow.statusMessage);
                }
                else if (type == '8') // Send ready
                {
                    if ( !userReady || userReadyToSubmits > 0 )
                    {
                        userReady = true;
                        userReadyToSubmits--;

                        // Commit ready status
                        Send("$ca$8");
                    }
                }

                RequestProfile(false, false);
            }
            return success;
        }

        bool IsStatusEmpty()
        {
            return (lastStatus == null || lastStatus == "No Status" || lastStatus == "");
        }

        bool IsUserNameEmpty()
        {
            return (userName == null || userName == "Unknown" || userName == "Loading ..." || userName == "");
        }


#if USE_CHATUSER_IMAGESOURCE
        public ImageSource profileImage
        {
            get { return this.GetProfileImage(); }
        }

        public ImageSource GetProfileImage()
        {
            if (profileBitmapImage != null)
            {
#if USE_CHATUSER_FREEZE_FLAG
                if (!profileBitmapImageFreezed)
                {
                    lock (noUser)
                    {
                        if (!profileBitmapImageFreezed)
                        {
                            profileBitmapImage.Freeze();
                            profileBitmapImageFreezed = true;
                        }
                    }
                }
#endif
                return profileBitmapImage;
            }
            return ChatAppWindow.defaultUserImage;
        }
#else
        public BitmapImage profileImage
        {
            get { return this.GetProfileImage(); }
        }


        public BitmapImage GetProfileImage()
        {
            if (profileBitmapImage != null)
                return profileBitmapImage;
            return ChatAppWindow.defaultUserImage;
        }
#endif


        public String profileText
        {
            get { return GetProfileText(); }
        }

        public String GetProfileText()
        {
            //RequestProfileProc();
            String connected = "";

            DeviceInstance localDevice = device;
            if (localDevice != null && localDevice.isConnected)
                connected = "* ";
            return connected + userName + " (" + lastStatus + ")\n" + lastMessage;
        }


        public void Release()
        {
            lock(this)
            {
                enabled = false;

                DeviceInstance dev = device;
                if (dev == null)
                    return;

                // Let's observe device changes, e.g. disposal, connections, etc..
                dev.RemoveObserver(OnDeviceChanged);

                // Let's listen to messages from this device
                dev.RemoveObserverForMessages(OnMessage);

                profileBitmapImage = null;

                dev.appContext1 = noUser;
            }

            if (chatInitThread != null)
            {
                try
                {
                    if (chatInitThread.ThreadState == System.Threading.ThreadState.Running)
                        chatInitThread.Join();
                }
                catch (Exception e)
                {
                    Debug.WriteLine(e.Message);
                }
            }

            lock(chatUsers)
            {
                chatUsers.Remove(this);
            }
        }


        public static ChatUser InitWithDevice(DeviceInstance userDevice)
        {
            if (userDevice == null)
                return null;

            // Create a ChatUser object and attach it to the appContext of the DeviceInstance
            ChatUser chatUser = new ChatUser();
            chatUser.device = userDevice;
            userDevice.appContext1 = chatUser;

            lock (chatUsers)
            {
                chatUsers.Add(chatUser);
            }

            if (!ChatAppWindow.useInitThread)
            {
                chatUser.chatInitThread = new Thread(() => chatUser.Init1());
                if (chatUser.chatInitThread != null)
                {
#if DEBUG
                    chatUser.chatInitThread.Name = "ChatUser.Init1";
#endif
                    chatUser.chatInitThread.Start();
                }
            }
            return chatUser;
        }

        public static void CheckChatUsers()
        {
            Collection<ChatUser> toDispose = new Collection<ChatUser>();

            lock (chatUsers)
            {
                foreach (ChatUser chat in chatUsers)
                {
                    DeviceInstance device = chat.device;

                    if ( device == null || device.disposed || !chat.enabled )
                    {
                        toDispose.Add(chat);
                    }
                    else
                    {
                        if (chat.appContextChanged)
                        {
                            chat.appContextChanged = false;
                            device.NotifyAppContextChanged(1);
                        }
                    }
                }
            }

            foreach (ChatUser chat in toDispose)
            {
                chat.Release();
            }
        }

        static int gcRunCount = 0;

        public static void DisposeChatUsers()
        {
            ChatUser[] toDispose = null;

            lock (chatUsers)
            {
                if (chatUsers.Count > 0)
                {
                    toDispose = new ChatUser[chatUsers.Count];
                    chatUsers.CopyTo(toDispose, 0);
                }
            }

            if (toDispose != null)
            {
                foreach (ChatUser chat in toDispose)
                {
                    chat.Release();
                }
            }

            gcRunCount++;

            if (gcRunCount >= 10)
            {
                gcRunCount = 0;

                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
        }


        void RequestProfileProc(bool enforce)
        {
            if (availableDetails >= 7)
                return;

            DeviceInstance localDevice = device;
            if (localDevice == null)
                return;

            bool observerReady = localDevice.isMessageObserverReady();

            if (!enforce && !userReady && !observerReady && localDevice.sourceType != DeviceSourceType.Broadcast)
                return;

            int now = Environment.TickCount & Int32.MaxValue;

            if ((enforce || userNameRequested == 0 || (now - userNameRequested > 1000)) && IsUserNameEmpty())
            {
                // Username not found. Let's request that.
                Send("$ca$0");

                userName = "Loading ...";
                //if (observerReady)
                    userNameRequested = now;
            }

            if ((enforce || lastStatusRequested == 0 || (now - lastStatusRequested > 1000)) && IsStatusEmpty())
            {
                // Status message not found. Let's request that.
                Send("$ca$4");
                //if (observerReady)
                    lastStatusRequested = now;
            }

            if (profileBitmapImage == null && (enforce || profilePicRequested == 0 || (now - profilePicRequested > 1000)))
            {
                // Profile picture not found. Let's request that.
                Send("$ca$2");
                //if (observerReady)
                    profilePicRequested = now;
            }
        }

        void RequestProfile(bool useLock, bool enforce)
        {
            if (useLock)
            {
                lock (this)
                {
                    RequestProfileProc(enforce);
                }
            }
            else
                RequestProfileProc(enforce);
        }


        public bool Init()
        {
            //Utils.Log(1, className, "Init: Started");

            DeviceInstance userDevice = device;

            if (userDevice == null || userDevice.disposed)
                return false;

            // This thread may run longer and if the native device vanishes, then Release will clear all references.
            // Garbage collection seems to dispose the device while we're still running
            ChatUser localChat = this;

            bool notify = false;

            if (initState == 0)
            {
                // Let's observe device changes, e.g. disposal, connections, etc..
                userDevice.AddObserver(OnDeviceChanged);

                // Let's listen to messages from this device
                userDevice.AddObserverForMessages(OnMessage);

                // Get or Request Username
                // Get or Request profile image
                // All chat internal messages start with $ca$type$data

                List<MessageInstance> msgs = userDevice.GetMessagesInStorage();
                if (msgs != null)
                {
                    try
                    {
                        for (int i = 0; i < msgs.Count; i++)
                        {
                            MessageInstance msgInst = msgs[i];
                            String msg = msgInst.text;

                            if (msg == null)
                                continue;

                            if (!IsChatCommand(msg))
                            {
                                if (IsEmpty(lastMessage) && !msgInst.sent)
                                {
                                    notify = true;
                                    lastMessage = msg;
                                }

                                Environs.dispatch(new Action(delegate ()
                                {
                                    messages.Insert(0, msgInst);
                                }));
                                continue;
                            }
                            if (msgInst.sent)
                                continue;

                            if (HandleChatCommand(msg, false))
                                notify = true;
                        }
                    }
                    catch (Exception)
                    {
                    }
                }

                userDevice.DisposeStorageCache();

                userDevice.ClearMessages();
                userDevice.ClearStorage();

                if (notify)
                    userDevice.NotifyAppContextChanged(1);

                if (userDevice.sourceType == DeviceSourceType.Broadcast)
                {
                    /// Ask user whether he/she is ready for commands
                    userDevice.SendMessage("$ca$8");

                    initState = 1;
                }
                else
                {
                    initState = 2; lastUpdateSent = Environment.TickCount;
                    return false;
                }
            }

            if (initState == 1)
            {
                int now = Environment.TickCount;

                if ((now - lastPingSent) < 1000)
                    return false;
                lastPingSent = now;

                if (userReady || !enabled || userDevice.disposed || userDevice.isMessageObserverReady())
                    return true;

                if (userDevice.sourceType == DeviceSourceType.Broadcast)
                {
                    /// Ask user whether he/she is ready for commands
                    userDevice.SendMessage("$ca$8");

                    pingsSent++;

                    if (pingsSent > 20)
                    {
                        initState = 2; lastUpdateSent = Environment.TickCount;
                        return false;
                    }
                    else
                        return false;
                }
                else
                {
                    initState = 2; lastUpdateSent = Environment.TickCount;
                    return false;
                }
            }

            if (initState == 2)
            {
                if (!NeedsUpdate())
                {
                    userDevice.NotifyAppContextChanged(1);
                    return true;
                }

                long now = Environment.TickCount;

                if ((now - lastUpdateSent) < 10000)
                    return false;
                lastUpdateSent = now;


                if (!enabled || userDevice.disposed)
                    return true;

                RequestProfile(true, false);
                updatesSent++;

                return updatesSent > 500;
            }

            if (Utils.Log(6))
                Utils.Log(className, "Init: Done userReady -> " + userReady);

            return true;
        }


        public bool Init1()
        {
            //Utils.Log(1, className, "Init: Started");

            // This thread may run longer and if the native device vanishes, then Release will clear all references.
            // Garbage collection seems to dispose the device while we're still running
            DeviceInstance localDevice = device;
            ChatUser localChat = this;

            bool notify = false;

            lock (this)
            {
                // Let's observe device changes, e.g. disposal, connections, etc..
                localDevice.AddObserver(OnDeviceChanged);

                // Let's listen to messages from this device
                localDevice.AddObserverForMessages(OnMessage);

                // Get or Request Username
                // Get or Request profile image
                // All chat internal messages start with $ca$type$data

                List<MessageInstance> msgs = localDevice.GetMessagesInStorage();
                if (msgs != null)
                {
                    try
                    {
                        for (int i = 0; i < msgs.Count; i++)
                        {
                            MessageInstance msgInst = msgs[i];
                            String msg = msgInst.text;

                            if (msg == null)
                                continue;

                            if (!IsChatCommand(msg))
                            {
                                if (IsEmpty(lastMessage) && !msgInst.sent)
                                {
                                    notify = true;
                                    lastMessage = msg;
                                }

                                Environs.dispatch(new Action(delegate ()
                                {
                                    messages.Insert(0, msgInst);
                                }));
                                continue;
                            }
                            if (msgInst.sent)
                                continue;

                            // System.ExecutionEngineException within next call (counter 1)
                            if (HandleChatCommand(msg, false))
                                notify = true;
                        }
                    }
                    catch (Exception)
                    {
                    }
                }

                localDevice.DisposeStorageCache();
            }

            localDevice.ClearMessages();
            localDevice.ClearStorage();

            if (notify)
                localDevice.NotifyAppContextChanged(1);

            int maxWaits = 36;

            // Must be changed to ==
            if (device.sourceType == DeviceSourceType.Broadcast)
            {
                /// Ask user whether he/she is ready for commands
                localDevice.SendMessage(Call.Wait, "$ca$8");

                Thread.Sleep(400);
                while (enabled && !userReady && maxWaits > 0 && !localDevice.disposed)
                {
                    if (device.sourceType != DeviceSourceType.Broadcast)
                        break;

                    /// Ask user whether he/she is ready for commands
                    if ((maxWaits % 2) == 0)
                        localDevice.SendMessage(Call.Wait, "$ca$8");

                    Thread.Sleep(400);
                    maxWaits--;
                }
            }

            if (Utils.Log(6))
                Utils.Log(className, "Init: Done userReady -> " + userReady + ", max: " + maxWaits);

            return true;
        }


        bool NeedsUpdate()
        {
            return (availableDetails < 3);
        }


        private void OnDeviceChanged(DeviceInstance device, DeviceInfoFlag notify)
        {
            if (device == null)
                return;
                        
            if (notify == DeviceInfoFlag.Disposed)
            {
                enabled = false;
                Release();
            }

            else if (notify == DeviceInfoFlag.Flags)
            {
                if (device.isMessageObserverReady())
                {
                    RequestProfile(false, false);
                }
            }            
            else if ((notify & DeviceInfoFlag.IsConnected) == DeviceInfoFlag.IsConnected)
            {
                device.NotifyAppContextChanged(1);

                RequestProfile(false, true);
            }
            // ...
        }


        public void OnMessage(MessageInstance msg, MessageInfoFlag changedFlags)
        {
            if (msg == null || msg.disposed)
            {
                //Utils.LogE("OnMessage: Invalid or disposed message.");
                return;
            }

            String text = msg.text;

            if (text == null || text.Length <= 0)
            {
                //Utils.LogE("OnMessage: Message text is null.");
                return;
            }

            //if (!msg.sent)
            //    Utils.Log(1, className, "OnMessage: " + text);

            lock (this)
            {
                if (IsChatCommand(text))
                {
                    if (!msg.sent && HandleChatCommand(text, true))
                    {
                        device.NotifyAppContextChanged(1);
                    }
                }
                else
                {
                    if (!msg.sent)
                    {
                        if (lastMessage != text)
                        {
                            lastMessage = text;
                            device.NotifyAppContextChanged(1);
                        }
                    }

                    Environs.dispatch(new Action(delegate()
                    {
                        messages.Insert(0, msg);
                    }));
                }
            }
        }
        
    }
    
}
