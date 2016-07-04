/**
 * Environs CLI declarations common
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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_EXTENSTION_CLI_DECL_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_EXTENSTION_CLI_DECL_H

#ifdef CLI_CPP

#include "Interop.h"
#include "Interop/Smart.Pointer.h"
#include "Environs.Platforms.h"
#include "Environs.Msg.Types.h"
#include "Device.Display.Decl.h"
#include "Environs.h"
#include "Environs.Cli.Utils.h"
#include "Environs.Lib.h"
#include "Utils/Device.Handler.h"
#include "Utils/Latency.Indicator.h"


using namespace System;
using namespace System::IO;
using namespace System::Windows;
using namespace System::Windows::Input;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Interop;
using namespace System::Threading;
using namespace System::Text;

#ifndef CLI_NOUI
	using namespace System::Windows::Forms;
#endif

namespace environs
{
#if !WINDOWS_PHONE
	[System::Security::SuppressUnmanagedCodeSecurityAttribute ()]
#endif
	public ref class Environs : lib::Environs
	{

	public:
		~Environs ();

#ifndef CLI_NOUI
		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object
		*/
		static Environs ^ CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler, String ^appName, String ^areaName );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object
		*/
		static Environs ^ CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, String ^appName, String ^areaName );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
		*
		* @return   An Environs object
		*/
		static Environs ^ CreateInstance ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		*
		* @return   An Environs object
		*/
		static Environs ^ CreateInstance ( ENV_CLIENTWINDOW ^clientWindow );
#endif

		/**
		* Create an Environs object.
		*
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object
		*/
		static Environs ^ CreateInstance ( String ^appName, String ^areaName );


#ifndef CLI_NOUI
		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object
		*/
		static Environs ^ New ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler, String ^appName, String ^areaName );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   An Environs object
		*/
		static Environs ^ New ( ENV_CLIENTWINDOW ^clientWindow, String ^appName, String ^areaName );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		* @param 	handler		A callback method of type EnvironsInitializedHandler, which is executed when Environs has initialized all required resources.
		*
		* @return   An Environs object
		*/
		static Environs ^ New ( ENV_CLIENTWINDOW ^clientWindow, environs::EnvironsInitializedHandler ^ handler );


		/**
		* Create an Environs object.
		*
		* @param 	clientWindow    A reference to the application window.
		*
		* @return   An Environs object
		*/
		static Environs ^ New ( ENV_CLIENTWINDOW ^clientWindow );
#endif

		/**
		* Create a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
		*
		* @param 	clientWindow    A reference to the application window.
		*
		* @return A DeviceList object
		*/
		DeviceList ^ CreateDeviceList ( environs::DeviceClass MEDIATOR_DEVICE_CLASS_ );

		/**
		* Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
		* 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
		* 		this call executes the start tasks within a thread.&nbsp;
		* 		In order to get the status, catch the onNotify handler of your EnvironsListener.
		*
		*/
		void Start ();
		void Stop ();

		static bool dispatchSync ( Action ^ action );
		static bool dispatch ( Action ^ action );
		static void SafeExit ();

		/**
		* OnStatus is called whenever the framework status changes.&nbsp;
		*
		* @param status      A status constant of type Status
		*/
		delegate void EnvironsStatusObserver ( Status status );
		EnvironsStatusObserver ^ OnStatus;

		/**
		* Add an observer for status changes.
		*
		* @param   observer Your implementation of EnvironsStatusObserver.
		*
		* @return	success
		*/
		bool AddObserverForStatus ( EnvironsStatusObserver ^ observer );

		/**
		* Remove an observer for status changes.
		*
		* @param   observer Your implementation of EnvironsStatusObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForStatus ( EnvironsStatusObserver ^ observer );

		/**
		* OnNotify is called by Environs in order to notify about events,<br\>
		* 		such as when a connection has been established or closed.
		*
		* @param context		An object reference of type environs.ObserverNotifyContext.
		*/
		delegate void EnvironsNotificationObserver ( environs::ObserverNotifyContext ^ context );
		EnvironsNotificationObserver ^ OnNotify;

		/**
		* Add an observer for notifications.
		*
		* @param   observer Your implementation of EnvironsNotificationObserver.
		*
		* @return	success
		*/
		bool AddObserverForNotify ( EnvironsNotificationObserver ^ observer );

		/**
		* Remove an observer for notifications.
		*
		* @param   observer Your implementation of EnvironsNotificationObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForNotify ( EnvironsNotificationObserver ^ observer );

		/**
		* OnNotifyExt is called by Environs in order to notify about events,<br\>
		* 		such as when a connection has been established or closed.
		*
		* @param context		An object reference of type environs.ObserverNotifyContext.
		*/
		delegate void EnvironsNotificationObserverExt ( environs::ObserverNotifyContext ^ context );
		EnvironsNotificationObserverExt ^ OnNotifyExt;

		/**
		* Add an observer for extended notifications.
		*
		* @param   observer Your implementation of EnvironsNotificationObserverExt.
		*
		* @return	success
		*/
		bool AddObserverForNotifyExt ( EnvironsNotificationObserverExt ^ observer );

		/**
		* Remove an observer for extended notifications.
		*
		* @param   observer Your implementation of EnvironsNotificationObserverExt.
		*
		* @return	success
		*/
		bool RemoveObserverForNotifyExt ( EnvironsNotificationObserverExt ^ observer );

		/**
		* OnMessage is called by native layer in order to notify about incoming messages.
		*
		* @param context		An object reference of type environs.ObserverMessageContext.
		*/
		delegate void EnvironsMessageObserver ( environs::ObserverMessageContext ^ context );
		EnvironsMessageObserver ^ OnMessage;

		/**
		* Add an observer for messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserver.
		*
		* @return	success
		*/
		bool AddObserverForMessages ( EnvironsMessageObserver ^ observer );

		/**
		* Remove an observer for messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForMessages ( EnvironsMessageObserver ^ observer );

		/**
		* OnMessageExt is called by native layer in order to notify about incoming messages.
		*
		* @param context		An object reference of type environs.ObserverMessageContext.
		*/
		delegate void EnvironsMessageObserverExt ( environs::ObserverMessageContext ^ context );
		EnvironsMessageObserverExt ^ OnMessageExt;

		/**
		* Add an observer for extended messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserverExt.
		*
		* @return	success
		*/
		bool AddObserverForMessagesExt ( EnvironsMessageObserverExt ^ observer );

		/**
		* Remove an observer for extended messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserverExt.
		*
		* @return	success
		*/
		bool RemoveObserverForMessagesExt ( EnvironsMessageObserverExt ^ observer );

		/**
		* OnStatusMessage is called by native layer in order to drop a status messages.
		*
		* @param msg           A status message of Environs.
		*/
		delegate void EnvironsStatusMessageObserver ( CString_ptr msg );
		EnvironsStatusMessageObserver ^ OnStatusMessage;

		/**
		* Add an observer for status messages.
		*
		* @param   observer Your implementation of EnvironsStatusMessageObserver.
		*
		* @return	success
		*/
		bool AddObserverForStatusMessages ( EnvironsStatusMessageObserver ^ observer );

		/**
		* Remove an observer for status messages.
		*
		* @param   observer Your implementation of EnvironsStatusMessageObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForStatusMessages ( EnvironsStatusMessageObserver ^ observer );

		/**
		* OnData is called by native layer in order to notify about data received from a device.
		*
		* @param context		An object reference of type environs.ObserverDataContext.
		*/
		delegate void EnvironsDataObserver ( environs::ObserverDataContext ^ context );
		EnvironsDataObserver ^ OnData;

		/**
		* Add an observer for data.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		bool AddObserverForData ( EnvironsDataObserver ^ observer );

		/**
		* Remove an observer for data.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForData ( EnvironsDataObserver ^ observer );

#ifndef MOVE_PORTALOBSERVER_TO_DEVICEINSTANCE
		/**
		* OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
		*
		* @param portal 		The PortalInstance object.
		*/
		delegate void PortalRequestOrProvided ( PortalInstance ^ portal );
		PortalRequestOrProvided ^ OnPortalRequestOrProvided;
#endif

		delegate void PortalRenderer ( System::Windows::Media::Imaging::WriteableBitmap ^ bitmap );

		/**
		* This handler type is called (by the native layer) when a human input event has been received from a connected device.
		*
		* @param nativeID 		The nativeID
		* @param hInput 		An input frame
		*/
		delegate void EnvironsHumanInputObserver ( int nativeID, environs::InputPack ^ hInput );

		/**
		* The delegate instance for external human input events that handlers can attach to.
		* This handler should be used only internally from the platform_ layer implementation.
		* Applications that are interested in these events should attach to the application surface window delegates,
		* where external events can be distinguished by type <c>EnvironsTouchDevice</c>.
		*
		*/
		EnvironsHumanInputObserver	^ OnHumanInput;

		/**
		* Add an observer for human input, such as touch.
		*
		* @param   observer Your implementation of EnvironsHumanInputObserver.
		*
		* @return	success
		*/
		bool AddObserverForHumanInput ( EnvironsHumanInputObserver ^ observer );

		/**
		* Remove an observer for human input, such as touch.
		*
		* @param   observer Your implementation of EnvironsHumanInputObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForHumanInput ( EnvironsHumanInputObserver ^ observer );

		/**
		* OnSensorInput is called (by the native layer) when a sensor input event has been received from a connected device.
		*
		* @param objID			The object id of the sender device.
		* @param sInput			The sensor data frame.
		*/
		delegate void EnvironsSensorDataObserver ( OBJIDType objID, environs::SensorFrame OBJ_ptr sInput );
		EnvironsSensorDataObserver ^ OnSensorInput;

		/**
		* Add an observer for sensor data.
		*
		* @param   observer Your implementation of EnvironsSensorDataObserver.
		*
		* @return	success
		*/
		bool AddObserverForSensorData ( EnvironsSensorDataObserver ^ observer );

		/**
		* Remove an observer for sensor data.
		*
		* @param   observer Your implementation of EnvironsSensorDataObserver.
		*
		* @return	success
		*/
		bool RemoveObserverForSensorData ( EnvironsSensorDataObserver ^ observer );

#ifndef CLI_NOUI
		/**
		* Instruct Environs to switch the app window to fullscreen after successful initialization and Start of Environs.
		*
		* @param value      
		*/
		void SetUseFullscreen ( bool value );

		/**
		* This is an Environs option to use the app window for portal streams or not. This must be set before starting Environs.
		* Not using the app window automatically uses the desktop window for portal streams.
		*
		* @param value
		*/
		void SetPortalFromAppWindow ( bool value );
#endif

		/**
		* Try enabling stream encoder or disable stream encoding.
		*
		*/
		void SetUseStream ( bool enable );


#if (defined(CLI_PS) || defined(CLI_STT))
		bool				isSurface;
		bool				updateDeviceTagPosition;

		LatencyIndicator ^	latencyIndicator;

		void InitLatencyIndicator ( System::Windows::Controls::TextBlock ^ tb );
#endif

		static String ^ resolveName ( int notification );

		static String ^ ParseCString ( IntPtr cString );


		property environs::Status status { Environs::Status get () { return ( environs::Status) environs::API::GetStatusN (hEnvirons); }};
		property bool isInitialized { bool get () { return initialized; }};
		
		bool disposeOnClose;

	internal:
		/**
		* This handler type is called (by the native layer) when data from devices or environs is available.
		*
		* @param objID
		* @param notification
		* @param sourceIdent
		* @param context
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl )]
		delegate void NotificationSourceInt ( int hInst, OBJIDType objID, int notification, int sourceIdent, IntPtr contextPtr, int context );

		NotificationSourceInt ^ onNotify;
		IntPtr onNotify_ptr;

		/**
		* This handler type is called (by the native layer) when data from devices or environs is available.
		*
		* @param hInst			A handle to the Environs instance
		* @param deviceID
		* @param areaName
		* @param appName
		* @param notification
		* @param sourceIdent
		* @param context
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl, CharSet = CharSet::Ansi )]
		delegate void NotificationSourceExtInt ( int hInst, int deviceID, 
			[ MarshalAs ( UnmanagedType::LPStr ) ] String ^ areaName, 
			[ MarshalAs ( UnmanagedType::LPStr ) ] String ^ appName, 
			int notification, int sourceIdent, IntPtr context );

		NotificationSourceExtInt ^ onNotifyExt;
		IntPtr onNotifyExt_ptr;

		/**
		* This handler type is called (by the native layer) when important short messages from devices, status, progress, or error messages are available.
		*
		* @param hInst			A handle to the Environs instance
		* @param objID
		* @param sourceIdent
		* @param msg
		* @param length
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl, CharSet = CharSet::Ansi )]
		delegate void MessageSourceInt ( int hInst, OBJIDType objID, int sourceIdent, [ MarshalAs ( UnmanagedType::LPStr ) ] String ^ msg, int length );

		MessageSourceInt ^ onMessage;
		IntPtr onMessage_ptr;

		/**
		* This handler type is called (by the native layer) when important short messages from devices, status, progress, or error messages are available.
		*
		* @param hInst			A handle to the Environs instance
		* @param deviceID
		* @param areaName
		* @param appName
		* @param sourceType
		* @param msg
		* @param length
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl, CharSet = CharSet::Ansi )]
		delegate void MessageSourceExtInt ( int hInst, int deviceID, 
			[ MarshalAs ( UnmanagedType::LPStr ) ] String ^ areaName, 
			[ MarshalAs ( UnmanagedType::LPStr ) ] String ^ appName, int sourceIdent,
			[ MarshalAs ( UnmanagedType::LPStr ) ] String ^ msg, int length );

		MessageSourceExtInt ^ onMessageExt;
		IntPtr onMessageExt_ptr;

		/**
		* This handler type is called (by the native layer) when a status message is available.
		*
		* @param msg
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl, CharSet = CharSet::Ansi )]
		delegate void StatusMessageSourceInt ( int hInst, [ MarshalAs ( UnmanagedType::LPStr ) ] String ^ msg );

		StatusMessageSourceInt ^ onStatusMessage;
		IntPtr onStatusMessage_ptr;

		/**
		* This handler type is called (by the native layer) when data from devices or environs is available.
		*
		* @param hInst			A handle to the Environs instance
		* @param objID
		* @param nativeID
		* @param sourceType
		* @param fileID
		* @param fileDescriptor
		* @param size
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl, CharSet = CharSet::Ansi )]
		delegate void DataSourceInt ( int hInst, OBJIDType objID, int nativeID, int sourceIdent, int fileID, [ MarshalAs ( UnmanagedType::LPStr ) ] String ^ fileDescriptor, int size );

		DataSourceInt ^ onData;
		IntPtr onData_ptr;

		/**
		* This handler type is internally called with the memory block of the received input packet when a human input event has been received from a connected device.
		*
		* @param hInst			A handle to the Environs instance
		* @param nativeID
		* @param data
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl )]
		delegate void EnvironsHumanInputHandlerRaw ( int hInst, int nativeID, IntPtr data );

		EnvironsHumanInputHandlerRaw ^ onHumanInput;
		IntPtr onHumanInput_ptr;

        
        /**
         * BridgeForUdpData static method to be called by native layer in order to notify about udp data received from a device.
         *
         * @param hInst			A handle to the Environs instance
         * @param objID         The native device id of the sender device.
         * @param pack          A udp data structure containing the received udp or sensor data.
         */
		CLI_NO_STATIC void BridgeForInputData ( int hInst, int nativeID, IntPtr pack );

		void InjectTouch ( int nativeID, InputPack ^ pack );

		/**
		* This handler type is internally called with the memory block of the received sensor input packet when a sensor input event has been received from a connected device.
		*
		* @param hInst			A handle to the Environs instance
		* @param objID
		* @param data
		* @param packSize      The size of the data buffer in number of bytes.
		*/
		[UnmanagedFunctionPointer ( CallingConvention::Cdecl )]
		delegate void EnvironsUdpDataHandlerRaw ( int hInst, OBJIDType objID, IntPtr data, int dataSize );

		EnvironsUdpDataHandlerRaw ^ onUdpData;
		IntPtr onUdpData_ptr;

	internal:
		Environs ();

		static	Environs ();

		static cli::array<Environs ^> ^					instancesAPI    = gcnew cli::array<Environs ^> ( ENVIRONS_MAX_ENVIRONS_INSTANCES );
        
		/// <summary>
		/// A static status that determines whether native layer is initialized or not
		/// 0 means not initialized
		/// 1 means available, but not initialized
		/// 2 means initialized, but platform layer not
		/// 3 means fully initialized
		/// </summary>
		static int										nativeStatus = 0;

		/// <summary>
		/// A static status that determines whether environs is initialized or not
		/// </summary>
		bool											initialized;

#ifndef CLI_NOUI
		/// <summary>
		/// A reference to the application window given when creating this object instance.
		/// </summary>
		ENV_CLIENTWINDOW							^	appWindow;

		bool											onCloseAttached;
#endif

		static  StaticDisposer ^						staticDisposer = gcnew StaticDisposer ();
		static	pthread_mutex_t							platformLock;
		static  IntPtr								^	EnvironsModule = nullptr;
		static	String ^								nativeLib = nill;
		static	int										EnvironsModuleCounter = 0;

		static int										DeviceInstanceSize;
		static int										DevicesHeaderSize;

		static String ^									libDir = "libs/";
		static array<String ^> ^						runtimes = { "env", "v100", "v120", "v140" };


		bool InitInstance ( environs::EnvironsInitializedHandler ^ handler );

		System::Object ^ environsObservers;
		System::Object ^ environsObserversForMessages;
		System::Object ^ environsObserversForData;
		System::Object ^ environsObserversForSensorData;

		static bool SetLibsDirectory ( String ^newLibDir );
		static bool PrepareNativeRuntime ( int rtIndex );

		void StartPlatformHandlers ();

#ifndef CLI_NOUI
		/// <summary>
		/// The delegate instance for signaling that environs has been initialized and is ready to be used by applications.
		/// </summary>
		environs::EnvironsInitializedHandler					^	Initialized;

		/// <summary>
		/// An event object for signaling whether the window has been fully loaded or not
		/// </summary>
		System::Threading::ManualResetEvent						^	windowStartStopEvent;

		bool														do_FullScreen;
		bool														do_PortalFromAppWindow;
#endif
		static bool isShutDown = false;

		static bool debug_keys = true;
		String ^ keyInput;
		int debug_device_tagID;
		int trackerIndex;
		virtual bool	InitPlatform () override;

		void	ReleasePlatformLayer ();
		bool	InitPlatformLayer ();

		void InitThread ();
		void InitDispatched ();

		static void SafeExitThreaded ();
		static void SafeExitThreadedAction ();

#ifndef CLI_NOUI
		void WaitUIFinished ( System::Threading::ManualResetEvent ^ waitEvent );

		// This is called by specific constructors (in common code for Surface 2 and Surface 1
		void AttachEventHandlers ();
		void DetachEventHandlers ( bool invoke );

		// This is called by EnvironsInstance constructor (located in Common.cs code for specific event handler initialization)
		void AttachSpecificEventHandlers ();
		void DetachSpecificEventHandlers ();

		void SwitchToFullScreen ();

		void DetermineWindowHandle ( System::Windows::Window ^window );

		/// <summary>
		/// Handler for invoking the environment init thread. Called when the (client) application window has been loaded, rendered and shown.
		/// </summary>
		/// <param name="obj"></param>
		/// <param name="args"></param>
		void OnWindowLoaded ( Object ^ obj, System::Windows::RoutedEventArgs ^ args );
		void OnSourceInitialized ( Object ^ obj, EventArgs ^ e );
		void OnSizeChanged ( Object ^ obj, System::Windows::SizeChangedEventArgs ^ args );
		void OnClosing ( Object ^ sender, System::ComponentModel::CancelEventArgs ^ e );

		void AppStylusDown ( Object ^ sender, StylusDownEventArgs ^ e );
		void AppStylusMove ( Object ^ sender, StylusEventArgs ^ e );
		void AppStylusUp ( Object ^ sender, StylusEventArgs ^ e );
#endif

		void AppPreviewKeyDown ( Object ^ sender, System::Windows::Input::KeyEventArgs ^ e );
		void MessageEvent ( environs::Source_t source, String ^ msg );

	private:
		void MessageEvent ( String ^ msg );

	internal:
#if (defined(CLI_PS) || defined(CLI_STT))
		DeviceHandler	^	deviceHandler;
		
		System::Collections::Generic::Dictionary<int, ApplicationEnvironment ^ > ^ applicationEnvironments;

		bool InitSurfacePlatformLayer ();

		void appMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e );
		void appMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e );
		void appMouseMove ( Object ^ sender, System::Windows::Input::MouseEventArgs ^ e );
		bool appHandleRightClickBothButtons ( int x, int y );
		bool appHandleRightClick ( int x, int y );
		void appRightMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e );
		void appLeftMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e );
		void appLeftMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e );
		void itemRightMouseUp ( Object ^ sender, MouseButtonEventArgs ^ e );

		void appTouchUp ( Object ^ sender, TouchEventArgs ^ e );
		void appTouchMove ( Object ^ sender, TouchEventArgs ^ e );
		void appTouchDown ( Object ^ sender, TouchEventArgs ^ e );
#endif
	};


	/// <summary>
	/// A TouchData object serves as container for details of a touch contact.
	/// Environs usually provides a InputPack object within the callback handler of type EnvironsTouchHandler.
	/// </summary>
	public ref class InputPack
	{
	public:
		// An id that identifies this input entity for its whole lifecycle
		int id;

		// A state declared by INPUT_STATE_* or INPUTSOURCE_COMMAND_*
		short state;

		// A type declared by INPUT_TYPE_*
		char type;

		int x;
		int y;
		int value;

		float angle;
		float size;

		float axisx;
		float axisy;
	};


	/// <summary>
	/// A sensor input object serves as container for raw sensor data.
	/// Environs usually provides a SensorInputPack object within the callback handler of type EnvironsSensorHandler.
	/// </summary>
	public ref class SensorInputPack
	{
	public:
		DeviceInstance ^ device;

		// An id that identifies this input entity for its whole lifecycle
		int id;

		// A type according to Environs.ENVIRONS_SENSOR_TYPE_*
		int type;

		float x;
		float y;
		float z;
	};

	/// <summary>
	/// An AvContext object serves as container for details of a portal frame (as rgb image)
	/// </summary>
	public ref class AvContext
	{
	public:
		int width;
		int height;
		int stride;
		IntPtr ^ data;
	};
}

#endif

#endif