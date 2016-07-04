using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Surface;
using Microsoft.Surface.Presentation;
using Microsoft.Surface.Presentation.Controls;
using Microsoft.Surface.Presentation.Input;
using System.Diagnostics;
using System.Runtime.InteropServices;

using environs;
using System.Threading;

namespace environs.apps.SurfaceExample
{
    /// <summary>
    /// Interaction logic for SurfaceWindow1.xaml
    /// </summary>
    public partial class SurfaceWindow1 : SurfaceWindow
    {
        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;

        /// <summary>
        /// Default constructor.
        /// </summary>
        public SurfaceWindow1()
        {
            InitializeComponent();

            // Add handlers for window availability events
            AddWindowAvailabilityHandlers();

            InitEnvirons();
        }


        /// <summary>
        /// Initialization of Environs
        /// </summary>
        #region Initialization of Environs

        /// <summary>
        /// Initialization of Environs within a thread.
        /// </summary>
        private void InitEnvirons()
        {
            Thread thread = new Thread(InitThread);
            if (thread == null)
                throw new InvalidProgramException("[ERROR_HALT] SurfaceExample: Initialization Error - Failed to create initialization thread!");
             
            thread.Start();
        }

        /// <summary>
        /// Initialization thread for Environs
        /// </summary>
        public void InitThread()
        {
            Utils.Log(2, "SurfaceExample: threaded initialization started...");

            environs = Environs.CreateInstance(this, InitializedEvent, "SurfaceExample", "Environs");

            // Use default mediator for device management
            environs.SetUseDefaultMediator(true);

            // Identify us as a surface 2
            environs.SetPlatform(Platforms.SAMSUR40);

            // Identify us as the id 176
            environs.SetDeviceID(176);

            environs.SetUserName("T@T.t");
            environs.SetUseAuthentication(true);

            environs.Start();

            Utils.Log(1, "SurfaceExample: Initialization thread done.");
        }


        private void InitializedEvent()
        {
            /*
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */

            Debug.WriteLine("[VERB] SurfaceExample: Environs is now initialized");
            /*
             * Now environs is initialized and ready to use. Furthermore the ui window is visible and fullscreen if requested to do so.
             */
        }
        #endregion


        /// <summary>
        /// Occurs when the window is about to close. 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);

            // Remove handlers for window availability events
            RemoveWindowAvailabilityHandlers();
        }


        #region Surface application/shell handling
        /// <summary>
        /// Adds handlers for window availability events.
        /// </summary>
        private void AddWindowAvailabilityHandlers()
        {
            // Subscribe to surface window availability events
            ApplicationServices.WindowInteractive += OnWindowInteractive;
            ApplicationServices.WindowNoninteractive += OnWindowNoninteractive;
            ApplicationServices.WindowUnavailable += OnWindowUnavailable;
        }

        /// <summary>
        /// Removes handlers for window availability events.
        /// </summary>
        private void RemoveWindowAvailabilityHandlers()
        {
            // Unsubscribe from surface window availability events
            ApplicationServices.WindowInteractive -= OnWindowInteractive;
            ApplicationServices.WindowNoninteractive -= OnWindowNoninteractive;
            ApplicationServices.WindowUnavailable -= OnWindowUnavailable;
        }

        /// <summary>
        /// This is called when the user can interact with the application's window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnWindowInteractive(object sender, EventArgs e)
        {
        }

        /// <summary>
        /// This is called when the user can see but not interact with the application's window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnWindowNoninteractive(object sender, EventArgs e)
        {
        }

        /// <summary>
        /// This is called when the application's window is not visible or interactive.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnWindowUnavailable(object sender, EventArgs e)
        {
        }
        #endregion

    }
}