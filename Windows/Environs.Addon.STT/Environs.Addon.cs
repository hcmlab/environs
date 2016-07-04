using System;
using System.Threading;
using System.Windows;

namespace environs
{
    public class EnvironsAddon
    {
        /// <summary>
        /// Helper method to dispatch instructions encapsulated into an Action to be executed in the Main- or UI-thread.
        /// </summary>
        public static bool Dispatch(Action action)
        {
#if WFORMS
            if (appWindow == null || action == null)
                return false;

            if (appWindow.InvokeRequired)
                appWindow.Invoke(action, null);
            else
                action();
#else
#if WINDOWS_PHONE
            Deployment.Current.Dispatcher.BeginInvoke(action);
#else
            if (Application.Current == null || action == null)
                return false;

            if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                action();
            else
                Application.Current.Dispatcher.BeginInvoke(action);
#endif
#endif
            return true;
        }
    }
}
