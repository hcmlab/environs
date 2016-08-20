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
//#define REFRESH_LIST_IN_CALLABCK
#define RELEASE_CHAT_IN_LIST_CALLABCK
//#define USE_ENV_OBSERVERCOLLECTION

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Threading;
using System.Collections.Specialized;

namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic for ChatAppWindow.xaml
    /// </summary>
    public partial class ChatAppWindow : Window, IDisposable
    {
        Thread updaterThread = null;

        #region Initialization

        private void InitEnvironsChat()
        {
            environs.SetUseDeviceListAsUIAdapter(false);

            // Notify the adapter about device detail changes
            // if we call NotifyAppContextChanged(1) on the device instance
            DeviceInstance.notifyPropertyChangedDefault = false;
        }

        #endregion


        #region Suppressable ObserveableCollection

        public class EnvObservableCollection<T> : ObservableCollection<T>
        {
            bool notificationsHappend = false;
            bool notificationsActive = true;

            public bool notificationEnabled
            {
                get
                {
                    return notificationsActive;
                }
                set
                {
                    notificationsActive = value;
                    if (notificationsActive && notificationsHappend)
                    {
                        this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
                        notificationsHappend = false;
                    }
                }
            }

            protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e)
            {
                if (notificationsActive)
                    base.OnCollectionChanged(e);
                else
                    notificationsHappend = true;
            }
        }

        #endregion


        #region Environs Observers

#if USE_ENV_OBSERVERCOLLECTION
        EnvObservableCollection<DeviceInstance> devices = new EnvObservableCollection<DeviceInstance>();

        public EnvObservableCollection<DeviceInstance> userCollection
        {
            get
            {
                return devices;
            }
        }
#else
        ObservableCollection<DeviceInstance> devices = new ObservableCollection<DeviceInstance>();

        public ObservableCollection<DeviceInstance> userCollection
        {
            get
            {
                return devices;
            }
        }
#endif


        void OnListChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
#if RELEASE_CHAT_IN_LIST_CALLABCK
            if (args.OldItems != null)
            {
                foreach (DeviceInstance device in args.OldItems)
                {
                    //Debug.WriteLine("<---- Vanished: " + device.ToString());

                    Object obj = device.appContext1;
                    if (obj != null && obj.GetType() == typeof(ChatUser))
                    {
                        ChatUser chat = (ChatUser)obj;

                        if (currentMessages == chat.messages)
                        {
                            Environs.dispatch(new Action(delegate ()
                            {
                                BindingOperations.SetBinding(messagesList, DataGrid.ItemsSourceProperty, new Binding("emptyCollection"));

                                currentMessages = emptyMessages;
                            }));
                        }                        
                        chat.Release();
                    }
                }
            }
#endif
            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    // Initialize a ChatUser and attach it to the device
                    ChatUser chat = ChatUser.InitWithDevice(device);
                    
                    if (ChatAppWindow.useInitThread && chat != null)
                    {
                        lock(chatsToInit) {
                            if (enabled)
                                chatsToInit.Add(chat);
                        }
                    }
                }

                if (ChatAppWindow.useInitThread)
                    initThreadEvent.Set();
            }

            updaterThreadEvent.Set();
        }

        int lastDeviceID = 0;
                
        void OnEnvironsStarted(Environs env)
        {
            int deviceID = env.GetDeviceID();
            if (deviceID == lastDeviceID)
                return;

            lastDeviceID = deviceID;
            Title = "ChatApp 0x" + deviceID.ToString("X") + " | " + ChatUser.loginUserName;
        }

        void OnEnvironsStopped()
        {
            if (devices != null)
                devices.Clear();
        }


        public void StartUpdaterThread()
        {
            lock (this)
            {
                if (updaterThread != null)
                    return;

                updaterThread = new Thread(SynchronizeDeviceListThread);
                if (updaterThread == null)
                    Utils.LogE("StartUpdaterThread: Failed to create updaterThread!!!");
                else
                    updaterThread.Start();
            }
        }


        private void SynchronizeDeviceListThread()
        {
            updaterThreadEnabled = true;

            while (updaterThreadEnabled)
            {
                Environs.dispatchSync(new Action(delegate ()
                {
#if REFRESH_LIST_IN_CALLABCK
                    bool changed = false;
#endif
                    DeviceList list = deviceList;
                    if (list == null)
                        return;
                    
                    Collection<DeviceInstance> backList = deviceList.GetDevices();
                    if (backList == null || backList.Count <= 0)
                    {
                        devices.Clear();
                        return;
                    }
                    
#if USE_ENV_OBSERVERCOLLECTION
                    devices.notificationEnabled = false;
#endif

                    // A reaaaally simple sync approach ... there are much better ones for sure ...
                    int i = 0, j = 0;
                    for (; i < backList.Count; i++)
                    {
                        DeviceInstance deviceSrc = backList[i];
                        if (deviceSrc.disposed || deviceSrc.appContext1 == null)
                            continue;

                        if (j >= devices.Count)
                        {
                            //Debug.WriteLine("-----> ADDED: " + deviceSrc.ToString());
                            devices.Add(deviceSrc);
                            j++;
#if REFRESH_LIST_IN_CALLABCK
                            changed = true;
#endif
                            continue;
                        }

                        DeviceInstance deviceDest = devices[j];

                        if (deviceSrc != deviceDest)
                        {
                            devices[j] = deviceSrc;
#if REFRESH_LIST_IN_CALLABCK
                            changed = true;
#endif
                            //Debug.WriteLine("-----> ASSIGNED: " + deviceSrc.ToString());
                        }
                        j++;
                    }

                    if (devices.Count > i)
                    {
                        while (i < devices.Count)
                        {
                            devices.RemoveAt(i);
                        }
#if REFRESH_LIST_IN_CALLABCK
                        changed = true;
#endif
                    }
                    
#if USE_ENV_OBSERVERCOLLECTION
                    devices.notificationEnabled = true;
#endif

#if REFRESH_LIST_IN_CALLABCK
                    if (changed)
                        userList.Items.Refresh();
#endif
                }));

                ChatUser.CheckChatUsers();

                updaterThreadEvent.WaitOne();
                updaterThreadEvent.Reset();
            }

            lock(this) { updaterThread = null; }
        }

        #endregion


        #region Messages list

        #endregion

    }
}
