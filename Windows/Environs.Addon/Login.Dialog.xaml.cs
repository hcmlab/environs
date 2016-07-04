/**
 *	LoginDialog
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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

/**
 *	Login Dialog
 *	---------------------------------------------------------
 *	Copyright (C) 2015 Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
    /// <summary>
    /// Interactionlogic für LoginDialog.xaml
    /// </summary>
    public partial class LoginDialog : Window
    {
        private const String className = "LoginDialog";

        private static object classLock = new object();
        private static int count = 0;

        private System.Windows.Threading.DispatcherTimer noActivityTimer = null;

        public string userName = "";
        public string password = "";


        private static Thread loginDialogThread = null;


        /**
         * Default constructor of LoginDialog. This class should not be instantiated using the default constructor.
         * Use SingletonInstance to get an instance which makes sure that there is only one instance at any time.
         *
         */
        public LoginDialog()
        {
            InitializeComponent();
        }

        
        public static void CreateAndShow(Environs env)
        {
            if (!env.GetUseMediatorLoginDialog())
                return;

            lock (classLock)
            {
                if (loginDialogThread == null)
                {
                    loginDialogThread = new Thread(() => LoginDialogThread(env));
                    if (loginDialogThread == null)
                        Utils.LogE(className, "EnvironsNotification: Failed to create loginDialogThread!!!");
                    else
                        loginDialogThread.Start();
                }
            }
        }

        /**
         * The thread that displays the Mediator login dialog and handles the response.
         *
         */
        private static void LoginDialogThread(Environs env)
        {
            string userName = env.GetMediatorUserName();

            Environs.dispatch(new Action(delegate()
            {
                LoginDialog inst = LoginDialog.SingletonInstance("Please enter login credentials!", "Mediator Login", userName);
                if (inst == null)
                {
                    lock (classLock)
                    {
                        loginDialogThread = null;
                    }
                    return;
                }

                if (inst.ShowResult())
                {
                    if (inst.userName != null && inst.password != null)
                    {
                        env.SetMediatorUserName(inst.userName);
                        env.SetMediatorPassword(inst.password);

                        env.RegisterAtMediators();
                    }
                    else
                        Utils.LogE("LoginDialog: Invalid username and password for Mediator entered!");
                }
                else
                {
                    if (env.GetMediatorLoginDialogDismissDisable())
                    {
                        env.SetUseCustomMediator(false);
                        env.SetUseDefaultMediator(false);
                    }
                }

                lock (classLock)
                {
                    loginDialogThread = null;
                }
            }));
        }


        /**
         * Default constructor of LoginDialog. This class should not be instantiated using the default constructor.
         * Use SingletonInstance to get an instance which makes sure that there is only one instance at any time.
         *
         * @param message       The message shown within the dialog.
         * @param title         The title of the dialog.
         * @param userName		The username if already known. This may be null.
         */
        public LoginDialog(string message, string title, string _userName = "")
        {
            InitializeComponent();

            userName = _userName;

            tbMessage.Text = message;
            Title = title;
            if (userName == null)
                userName = "";

            if (userName.Length > 3)
            {
                tbUserName.Text = userName;
                tbPassword.Focus();
                return;
            }
            tbUserName.Focus();
        }


        /**
         * This method is called on user input when username changed.
         * In such a case, we reschedule the no activity timer.
         *
         */
        private void tb_TextChanged(object sender, TextChangedEventArgs e)
        {
            ReScheduleTimer();
        }

        /**
         * This method is called on user input when password changed.
         * In such a case, we reschedule the no activity timer.
         *
         */
        private void tb_PasswordChanged(object sender, RoutedEventArgs e)
        {
            ReScheduleTimer();
        }


        /**
         * Create an instance of the login dialog with the given parameters. 
         * The dialog has to invoked/shown using ShowResult.
         *
         * @param message       The message shown within the dialog.
         * @param title         The title of the dialog.
         * @param userName		The username if already known. This may be null.
         * @return              An instance of the login dialog.
         */
        public static LoginDialog SingletonInstance(string message, string title, string userName = "")
        {
            lock (classLock)
            {
                if (count != 0)
                    return null;
                count = 1;
            }

            return new LoginDialog(message, title, userName);
        }


        /**
         * Show the login dialog within a thread and unblock the calling thread.
         *
         * @return  returns always true
         */
        public bool ShowResult()
        {
            ReScheduleTimer();

            ShowDialog();

            return (DialogResult == true);
        }


        /**
         * The no activity timeout is fired after the seconds declared by ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT has passed.
         * In such a case, we reschedule the no activity timer.
         *
         */
        private void noActivityTimeout(object sender, EventArgs e)
        {
            btnCancel_Click(sender, null);
            Close();
        }


        /**
         * Dispose the no activity timer.
         *
         */
        private void DisposeTimer()
        {
            Utils.Log(8, className, "DisposeTimer");

            if (noActivityTimer != null)
            {
                noActivityTimer.Stop();
                noActivityTimer = null;
            }
        }


        /**
         * Schedules a no activity timer that fires after the seconds declared by ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT.
         * Dispose a timer if a timer has been invoked before.
         *
         */
        private void ReScheduleTimer()
        {
            Utils.Log(8, className, "ReScheduleTimout");

            DisposeTimer();

            noActivityTimer = new System.Windows.Threading.DispatcherTimer();
            noActivityTimer.Tick += new EventHandler(noActivityTimeout);
            noActivityTimer.Interval = new TimeSpan(0, 0, Environs.ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT);
            //noActivityTimer.Interval = new TimeSpan(0, 0, 10); // for testing
            noActivityTimer.Start();
        }


        /**
         * Clear username and password members.
         *
         */
        public void Clear()
        {
            userName = "";
            password = "";
        }


        /**
         * Reset the static login dialog instance.
         *
         */
        private void ResetClose()
        {
            lock (classLock)
            {
                count = 0;
            }
            Close();
        }


        /**
         * Create an instance of the login dialog with the given parameters. 
         * The dialog has to invoked/shown using ShowResult.
         *
         * @param sender    The sender object (our dialog).
         * @param e         The RoutedEventArgs.
         */
        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            userName = tbUserName.Text;
            password = tbPassword.Password;

            DialogResult = true;
            ResetClose();
        }


        /**
         * Create an instance of the login dialog with the given parameters. 
         * The dialog has to invoked/shown using ShowResult.
         *
         * @param sender    The sender object (our dialog).
         * @param e         The RoutedEventArgs.
         */
        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            ResetClose();
        }
    }
}
