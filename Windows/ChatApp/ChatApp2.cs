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
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Diagnostics;

using System.IO;
using System.Threading;
using System.Collections.ObjectModel;

namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic for ChatAppWindow.xaml
    /// </summary>
    public partial class ChatAppWindow : Window
    {

        #region Initialization

        private void InitEnvironsChat()
        {

            // For UI performance improvements. Notify the adapter about device detail changes
            // only when we call NotifyAppContextChanged(0) on the device instance
            DeviceInstance.notifyPropertyChangedDefault = false;
        }

        public void StartUpdaterThread()
        { }
        #endregion



        #region Environs Observers        

        public ObservableCollection<DeviceInstance> userCollection
        {
            get
            {
                if (deviceList != null)
                    return (ObservableCollection<DeviceInstance>)deviceList.GetDevicesSource();
                return null;
            }
        }


        void OnListChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.OldItems != null)
            {
                foreach (DeviceInstance device in args.OldItems)
                {
                    Debug.WriteLine("Vanished: " + device.ToString());

                    Object chat = device.appContext1;
                    if (chat != null && chat.GetType() == typeof(ChatUser))
                    {
                        if (currentMessages == ((ChatUser)chat).messages)
                        {
                            Environs.dispatch(new Action(delegate ()
                            {
                                BindingOperations.SetBinding(messagesList, DataGrid.ItemsSourceProperty, new Binding("emptyCollection"));

                                currentMessages = emptyMessages;
                            }));
                        }

                        ((ChatUser)chat).Release();
                    }
                }
            }

            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    //Debug.WriteLine("Appeared: " + device.ToString());

                    // Initialize a ChatUser and attach it to the device
                    ChatUser chat = ChatUser.InitWithDevice(device);
                    if (ChatAppWindow.useInitThread && chat != null)
                    {
                        lock (chatsToInit)
                        {
                            if (enabled)
                                chatsToInit.Add(chat);
                        }
                    }
                }
                if (ChatAppWindow.useInitThread)
                    initThreadEvent.Set();
            }
        }

        void OnEnvironsStarted(Environs env)
        {
            Title = "ChatApp2 0x" + env.GetDeviceID().ToString("X") + " | " + ChatUser.loginUserName;
        }

        void OnEnvironsStopped()
        {
        }

        #endregion




    }
}
