/**
 * Environs CLI Utilities
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
#include "stdafx.h"

#ifdef CLI_CPP

#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Sensor.Frame.Cli.h"

#include "Interop/Stat.h"
#include "Environs.Native.h"
#include "Environs.Lib.h"
#include "Environs.h"
#include "Device/Device.List.Cli.h"
#include "Device/Device.Instance.Cli.h"
#include "Portal.Info.h"

using namespace System::Windows::Media;

#define CLASS_NAME	"Environs.Cli.Utils . . ."

#define USE_ENV_LOG
//#define WITH_TIME_STRING

#ifdef USE_ENV_LOG
//#	define	LOG_OUT(exp)	String ^ l = exp + "\n"; environs::API::LogN ( l, l->Length )
#	define	LOG_OUT(exp)	exp->Append ( "\n" ); environs::API::LogN ( sbl->ToString(), sbl->Length )
//#	define	LOG_OUT(exp)	LogBuffer::Add ( exp + "\n" )
#else
#	define	LOG_OUT(exp)	Debug::WriteLine ( exp )
#endif

namespace environs
{

#ifndef CLI_NOUI
	void Environs::SwitchToFullScreen ()
	{
		if ( appWindow == nullptr ) {
			CWarn ( "SwitchToFullScreen: No client window!" );
			return;
		}

		appWindow->MinWidth = SystemParameters::PrimaryScreenWidth - 50;
		appWindow->MaxWidth = SystemParameters::PrimaryScreenWidth + 200;
		appWindow->MinHeight = SystemParameters::PrimaryScreenHeight - 25;
		appWindow->MaxHeight = SystemParameters::PrimaryScreenHeight + 200;
		appWindow->WindowStyle = WindowStyle::None;
		appWindow->WindowState = WindowState::Maximized;
	}

	
	bool IsUIThread ()
	{
		if ( System::Windows::Application::Current == nullptr )
			return false;

		if ( Thread::CurrentThread == System::Windows::Application::Current->Dispatcher->Thread )
			return true;
		return false;
	}


	/// <summary>
	/// Helper method to dispatch instructions encapsulated into an Action to be executed in the Main- or UI-thread.
	/// </summary>
	bool Environs::dispatch ( Action ^ action )
	{
#if defined(WFORMS)
		if ( appWindow == nullptr || action == nullptr )
			return false;

		if ( appWindow->InvokeRequired )
			appWindow->Invoke ( action, nullptr );
		else
			action ();
#elif defined(WINDOWS_PHONE)
		Deployment::Current::Dispatcher.BeginInvoke ( action );
#else
		if ( System::Windows::Application::Current == nullptr || action == nullptr )
			return false;

		if ( Thread::CurrentThread == System::Windows::Application::Current->Dispatcher->Thread )
			action ();
		else {
			System::Windows::Threading::DispatcherOperation ^ op = System::Windows::Application::Current->Dispatcher->BeginInvoke ( action );
			if ( op == nullptr || op->Status == System::Windows::Threading::DispatcherOperationStatus::Aborted )
				return false;
		}
#endif
		return true;
	}


	/// <summary>
	/// Helper method to dispatch instructions encapsulated into an Action to be executed in the Main- or UI-thread.
	/// </summary>
	bool Environs::dispatchSync ( Action ^ action )
	{
#if defined(WFORMS)
		if ( appWindow == nullptr || action == nullptr )
			return false;

		if ( appWindow->InvokeRequired )
			appWindow->Invoke ( action, nullptr );
		else
			action ();
#elif defined(WINDOWS_PHONE)
		Deployment.Current.Dispatcher.BeginInvoke ( action );
#else
		if ( System::Windows::Application::Current == nullptr || action == nullptr )
			return false;

		if ( Thread::CurrentThread == System::Windows::Application::Current->Dispatcher->Thread )
			action ();
		else
			System::Windows::Application::Current->Dispatcher->Invoke ( action, nullptr );
#endif
		return true;
	}


	void Environs::SafeExitThreaded ()
	{
		CLog ( "SafeExitThreaded" );

		//Dispose(true);
#if defined(WFORMS)
		if ( appWindow != nullptr )
		{
			Action ^ action = gcnew Action ( &Environs::SafeExitThreadedAction );

			if ( !dispatch ( action ) )
				Debug.WriteLine ( "SafeExitThreaded: No UI thread or window handle available!!" );
			appWindow = null;
		}

#elif !defined(WINDOWS_PHONE)

		if ( System::Windows::Application::Current != nullptr )
		{
			Action ^ action = gcnew Action ( &Environs::SafeExitThreadedAction );

			if ( !dispatch ( action ) )
				CWarn ( "SafeExitThreaded: No UI thread or window handle available!!" );
		}
#else
		/// Windows Phone code... to do...
#endif
	}
#else

	void Environs::SafeExitThreaded ()
	{
		CLog ( "SafeExitThreaded" );

		Environs::SafeExitThreadedAction ();
	}


	bool IsUIThread ()
	{
		return false;
	}


	ref class DispatcherClass
	{
		Action ^ action;

	public:
		DispatcherClass ( Action ^ a ) : action ( a ) { }
		void Run ()
		{
			if ( action == nullptr )
				return;

			action ();
		}
	};


	/// <summary>
	/// Helper method to dispatch instructions encapsulated into an Action to be executed in the Main- or UI-thread.
	/// </summary>
	bool Environs::dispatch ( Action ^ action )
	{
		DispatcherClass ^ act = gcnew DispatcherClass ( action );

		Thread ^ thread = gcnew Thread ( gcnew ThreadStart ( act, &DispatcherClass::Run ) );
		if ( thread != nullptr )
			thread->Start ();
		else
		{
			act->Run ();
		}

		return true;
	}


	/// <summary>
	/// Helper method to dispatch instructions encapsulated into an Action to be executed in the Main- or UI-thread.
	/// </summary>
	bool Environs::dispatchSync ( Action ^ action )
	{
		action (); 

		return true;
	}
#endif


	/// <summary>
	/// Safely close the application and shutdown Environs.
	/// </summary>
	void Environs::SafeExit ()
	{
		CLog ( "SafeExit" );

		Thread ^ thread = gcnew Thread ( gcnew ThreadStart ( &Environs::SafeExitThreaded ) );
		if ( thread != nullptr )
			thread->Start ();
		else
		{
			CErr ( "SafeExit: Failed to create exit thread!" );

			SafeExitThreaded ();
		}
	}
	

	String ^ Environs::resolveName ( int notification )
	{
		return Marshal::PtrToStringAnsi ( IntPtr ( (void *) environs::API::ResolveNameN ( notification ) ) );
	}
	
	
	String ^ Environs::ParseCString ( IntPtr cString )
	{
		if ( cString == IntPtr::Zero )
			return "";

		return CCharToString ( cString );
	}


	void Environs::SafeExitThreadedAction ()
	{
		CVerb ( "SafeExitThreadedAction" );

		if ( isShutDown )
			return;
		isShutDown = true;

#if defined(WFORMS)
		// Disposal happens within the OnClose handler
		//Dispose();
		if ( xnaWindow != nullptr )
		{
			xnaWindow->Exit ();
			xnaWindow = nullptr;
		}

#elif !defined(WINDOWS_PHONE)
		System::Windows::Application::Current->Shutdown ();
		//appWindow.Close();
#else
		/// Windows Phone code... to do...
#endif
	}

	static bool inputLine = false;
	static char keyCommand = 'n';

	int tryGetTagID ( String ^ input )
	{
		// parse tag id
		int id = 0;
		try
		{
			id = Convert::ToInt32 ( input, 16 );
		}
		catch ( Exception ^ )
		{
		}
		return id;
	}



	/// <summary>
	/// This is a wrapper used within the Environs framework
	/// </summary>
	void Environs::MessageEvent ( environs::Source_t source, String ^ msg )
	{
		//Debug.WriteLine("[INFO] MessageEvent: device [" + nativeID + "] source [" + source + "] msg [" + msg + "]");

		BridgeForMessage ( hEnvirons, 0, ( int ) source, msg, msg->Length );
	}



	/// <summary>
	/// This is a wrapper used within the Environs framework
	/// </summary>
	void Environs::MessageEvent ( String ^ msg )
	{
		//Debug.WriteLine("[INFO] MessageEvent: device [" + nativeID + "] source [" + source + "] msg [" + msg + "]");

		BridgeForMessage ( hEnvirons, 0, ( int ) Source::Platform, msg, msg->Length );
	}


	void Environs::AppPreviewKeyDown ( Object ^ sender, System::Windows::Input::KeyEventArgs ^ e )
	{
		if ( e->Key == Key::Escape )
		{
			SafeExit ();
		}
		else {
			if ( !debug_keys )
				return;

			if ( inputLine )
			{
				if ( e->Key == Key::Enter )
				{
					int id = 0;
					inputLine = false;
					if ( keyInput == nullptr || keyInput->Length == 0 )
					{
						MessageEvent ( "Input aborted." );
						return;
					}
					// analysing input
					MessageEvent ( ":: " + keyInput );

					if ( keyCommand == 'd' )
					{
						id = tryGetTagID ( keyInput );
						if ( id > 0 )
						{
							//debug_device_tagID = id;
							MessageEvent ( "New debug tagID is " + keyInput );
						}
						else
							MessageEvent ( "Debug tagID " + keyInput + " is invalid" );
					}

					else if ( keyCommand == 'f' )
					{
						id = tryGetTagID ( keyInput );
						if ( id > 0 )
						{
							MessageEvent ( "Destination TagID is 0x" + String::Format ( "{0:X}", id ) );
							if ( (DeviceStatus) environs::API::GetDeviceConnectStatusN ( hEnvirons, id ) == DeviceStatus::Connected )
							{
#if !WINDOWS_PHONE
								//environs::API::SendFileN ( hEnvirons, id );
#endif
							}
							else
							{
								MessageEvent ( "Conncecting to " + id + ". Try sending again after successful connection." );
								environs::API::DeviceConnectN ( hEnvirons, id, nullptr, nullptr, CALL_NOWAIT );
							}
						}
						else
							MessageEvent ( "TagID " + keyInput + " is invalid" );
					}

					else if ( keyCommand == 'm' || keyCommand == 'p' )
					{
						int p = keyInput->IndexOf ( ' ' );
						if (/*p < -1 ||*/ p < 2 )
						{
							MessageEvent ( "Invalid input." );
						}
						else
						{
							String ^ ids = keyInput->Substring ( 0, p );
							id = tryGetTagID ( ids );
							if ( id > 0 )
							{
								String ^ msg = keyInput->Substring ( p + 1 );
								MessageEvent ( "Sending to 0x" + String::Format ( "{0:X}", id ) + " msg: " + msg );

								if ( keyCommand == 'm' ) {
									IntPtr p = Marshal::StringToHGlobalAuto ( msg );

									environs::API::SendMessageN ( hEnvirons, id, nullptr, nullptr, 1, p.ToPointer(), msg->Length );

									Marshal::FreeHGlobal ( IntPtr ( p ) );
								}
								else
									environs::API::SendPushNotificationN ( hEnvirons, id, nullptr, nullptr, msg );
							}
							else
								MessageEvent ( "TagID " + ids + " is invalid" );
						}
					}
					else if ( keyCommand == 't' )
					{
#if SURFACE2
						if ( keyInput [ 0 ] == 'e' )
						{
							InitRawImageCapture ( new WindowInteropHelper ( appWindow ).Handle );
						}
						else if ( keyInput [ 0 ] == 'd' )
						{
							DisposeSurfaceInput ();
						}
						else
#endif
							PushTrackerCommand ( Call::NoWait, trackerIndex, keyInput [ 0 ] );
					}
				}
				else if ( keyCommand == 's' )
				{
					if ( e->Key == Key::F )
					{
						inputLine = true;
						MessageEvent ( "Enter the destination tagID." );
						keyCommand = 'f';
						keyInput = "";
					}
					else if ( e->Key == Key::M || e->Key == Key::P )
					{
						inputLine = true;
						MessageEvent ( "Enter the destination tagID and the message: tagID message..." );
						if ( e->Key == Key::P )
							keyCommand = 'p';
						else
							keyCommand = 'm';
						keyInput = "";
					}
					else
					{
						MessageEvent ( "Unrecognized command" );
						inputLine = false;
						keyInput = "";
					}
				}
				else
				{
					if ( e->Key == Key::Space )
						keyInput += ' ';
					else
					{
						// To string
						String ^ s = Convert::ToString ( e->Key );
						if ( s->Length == 1 )
						{
							//if (e.KeyboardDevice.IsKeyDown(Key.LeftShift)
							if ( Keyboard::Modifiers == ModifierKeys::Shift )
								s = s->ToUpper ();
							else
								s = s->ToLower ();
							keyInput += s;
						}
						else if ( s->Length == 2 && s [ 0 ] == 'D' ) // Numbers
						{
							keyInput += s->Substring ( 1, 1 );
						}
					}
				}
			}
			else
			{
				switch ( e->Key )
				{
				case Key::T:
					inputLine = true;
					MessageEvent ( "\r\n" );
					PushTrackerCommand ( Call::NoWait, trackerIndex, '?' );

					MessageEvent ( "Enter command:" );
					keyCommand = 't';
					keyInput = "";
					break;

#ifndef NDEBUG
				case Key::Up:
				{
					if ( !Keyboard::IsKeyDown ( Key::LeftShift ) && !Keyboard::IsKeyDown ( Key::RightShift ) )
						break;
					int level = GetDebug ();
					if ( level < 15 ) {
						CLogArg1 ( "KeyDown: Increasing", "debug level", "i", level + 1 );
						SetDebug ( level + 1 );
					}
					break;
				}

				case Key::Down:
				{
					if ( !Keyboard::IsKeyDown ( Key::LeftShift ) && !Keyboard::IsKeyDown ( Key::RightShift ) )
						break;
					int level = GetDebug ();
					if ( level > 0 ) {
						CLogArg1 ( "KeyDown: Decreasing", "debug level", "i", level - 1 );
						SetDebug ( level - 1 );
					}
					break;
				}

				case Key::P:
				{
					/// Get the last deviceID that we are connected to
					pthread_mutex_t OBJ_ptr lock = nill;

					ObservableCollection<DeviceInstance ^> ^devices = ( ObservableCollection<DeviceInstance ^> ^ ) Environs::GetDevicesBest ( lock );
					if ( devices == nullptr )
						break;

					int i = devices->Count - 1;
					for ( ; i >= 0; i-- )
					{
						if ( devices [ i ]->info_->isConnected )
						{
							environs::API::RequestPortalStreamN ( hEnvirons, devices [ i ]->nativeID, CALL_NOWAIT, ( int ) PortalType::Any, 0, 0 );
							break;
						}
					}
					//if (i < 0)
					//{
					//    devices[i].Connect();
					//}
				}
				break;

				case Key::End:
					//DeviceInstance device = DeviceList.GetDeviceBestMatchStatic(debug_device_tagID);
					//if (device != null)
					//{
					//    if (device.isConnected)
					//    {
					//        device.Disconnect();
					//    }
					//    else
					//        device.Connect();
					//}
					break;

				case Key::D:
					inputLine = true;
					MessageEvent ( "Debug tagID is 0x" + String::Format ( "{0:X}", debug_device_tagID ) );
					MessageEvent ( "Enter new debug tagID" );
					keyCommand = 'd';
					keyInput = "";
					break;

				case Key::S:
					inputLine = true;
					MessageEvent ( "Do you want to send a (f)ile or a (p)ush (m)essage?" );
					keyCommand = 's';
					keyInput = "";
					break;
#endif

#if !WINDOWS_PHONE
				case Key::B:
				{
					array<char> ^buffer = gcnew array<char> ( 2048 );
					buffer [ 0 ] = Convert::ToByte ( 'a' );
					buffer [ 1 ] = Convert::ToByte ( 'b' );
					buffer [ 2 ] = Convert::ToByte ( 'c' );
					buffer [ 3 ] = Convert::ToByte ( 'd' );

					/*environs::API::SendBuffer1 ( hEnvirons, debug_device_tagID, 1, 2048, "noname", gcnew IntPtr ( buffer), 4 );

					byte* pByte = buffer;

					unsafe
					{
						fixed ( byte* pByte = buffer )
					{
						IntPtr intPtr = new IntPtr ( ( void* ) pByte );
						SendBuffer ( debug_device_tagID, 1, 2048, "noname", intPtr, 4 );
					}
					}*/
					break;
				}
#endif

#ifndef NDEBUG
				case Key::G:
				{
					//int count = Environs.GetDevicesNearbyCount(hEnvirons);
					//ObservableCollection<DeviceInstance> devices = null;

					//count = Environs.GetDevicesCount1(hEnvirons, MEDIATOR_DEVICE_CLASS_ALL);
					//Debug.WriteLine("We have " + count + " available devices");
					//devices = Environs.GetDevices0(hEnvirons);
					//if (devices != null)
					//{
					//    for (int i = 0; i < devices.Count; i++)
					//        Debug.WriteLine(devices[i].ToString());
					//}
					break;
				}

				case Key::V:
				{
					environs::API::SetUseTouchVisualizationN ( hEnvirons, !environs::API::GetUseTouchVisualizationN (hEnvirons) );
					break;
				}
#endif
				}

				// Send key to application
				String ^ sendChar = "";

				if ( e->Key == Key::Space )
					sendChar = " ";
				else
				{
					// To string
					String ^ s = Convert::ToString ( e->Key );
					if ( s->Length == 1 )
					{
						//if (e.KeyboardDevice.IsKeyDown(Key.LeftShift)
						if ( Keyboard::Modifiers == ModifierKeys::Shift )
							s = s->ToUpper ();
						else
							s = s->ToLower ();
						sendChar = s;
					}
					else if ( s->Length == 2 && s [ 0 ] == 'D' ) // Numbers
					{
						sendChar = s->Substring ( 1, 1 );
					}
				}
				MessageEvent ( "K:" + sendChar );
			}
		}
	}


	ref class LogBuffer : public IDisposable
	{
	internal:
		static bool				logAlive;
		static List<String ^>	logs;

		//static LogBuffer ^		logBuffer = gcnew LogBuffer ();
		static LogBuffer ^		logBuffer;

		static Thread ^			logThread = nullptr;

		static System::Threading::ManualResetEvent ^ logEvent = gcnew System::Threading::ManualResetEvent ( false );

		void LogThread ()
		{
			CLog ( "LogThread" );

			while ( logAlive ) {
				logEvent->WaitOne ();
				logEvent->Reset ();

				while ( true )
				{
					String  ^ msg = nullptr;

					Monitor::Enter ( %logs );

					if ( logs.Count > 0 ) {

						msg = logs [ 0 ];
						logs.RemoveAt ( 0 );
					}
					Monitor::Exit ( %logs );

					if ( msg == nullptr )
						break;

					environs::API::LogN ( msg, msg->Length );
				}
			}
		}

	public:
		LogBuffer ()
		{
			Debug::WriteLine ( CLASS_NAME + "LogBuffer: Construct" );

			logAlive = true;
			logs.Clear ();

			logThread = gcnew Thread ( gcnew ThreadStart ( this, &LogBuffer::LogThread ) );
			if ( logThread != nullptr )
				logThread->Start ();
		}

		~LogBuffer ()
		{
			logAlive = false;

			if ( logThread != nullptr && logEvent != nullptr ) {
				logEvent->Set ();
				logThread->Join ();
			}

			logThread = nullptr;
			logEvent = nullptr;

			Debug::WriteLine ( CLASS_NAME + "LogBuffer: Destruct" );
		}

		static void Add ( String ^ msg )
		{
			Monitor::Enter ( %logs );

			logs.Add ( msg );
			logEvent->Set ();

			Monitor::Exit ( %logs );
		}
	};


	String ^ GetTimeString ()
	{
#ifdef WITH_TIME_STRING
		DateTime dt = DateTime::Now;
		return dt.ToString ( " ddd MMM dd HH:mm:ss: " );
#else
		return "";
#endif
	}


	/**
	* Go through all created instances, stop all Environs activities and release all acquired resources.
	*/
	StaticDisposer::StaticDisposer ()
	{
		CVerbVerb ( "StaticDisposer.Construct" );
	};


	StaticDisposer::~StaticDisposer ()
	{
		CLog ( "StaticDisposer.Destruct" );

		for ( int i = 1; i < ENVIRONS_MAX_ENVIRONS_INSTANCES; i++ )
		{
			if ( environs::Environs::instancesAPI [ i ] != nullptr )
				environs::Environs::instancesAPI [ i ]->DisposeInstance ();
		}

		Environs::ObjectAPIDispose ();

		CVerbVerb ( "StaticDisposer.Destruct: Done" );
	};


	/**
	* Output a log message through the diagonostics debug output.
	*
	* @param level     The severity level of the message.
	* @param msg       An error message to be logged.
	*/
	//[ Conditional ( "DEBUG" ) ]
	void Utils::Log ( int level, String ^ msg )
	{
		Log ( level, msg, false );
	}


	bool Utils::Log ( int level )
	{
		return ( level <= logLevel );
	}


	void Utils::Log ( int level, String ^ msg, bool withDateTime )
	{
		if ( level == ERR_LEVEL )
		{
			LogE ( msg );
			return;
		}
		if ( level == WARN_LEVEL )
		{
			LogW ( msg );
			return;
		}

		if ( level <= logLevel )
		{
			StringBuilder ^ sbl;

			if ( withDateTime )
				sbl = gcnew StringBuilder ( GetTimeString () );
			else
				sbl = gcnew StringBuilder ();

			if ( level < 2 )
			{
				sbl->Append ( "[INFO]   " );
			}
			else if ( level < 3 )
			{
				sbl->Append ( "[TRACE]  " );
			}
			else
			{
				sbl->Append ( "[VERB]   " );
			}

			sbl->Append ( msg );
			LOG_OUT ( sbl );
		}
	}


	/**
	* Output an error message through the system console output.
	*
	* @param msg An error message to be logged.
	*/
	void Utils::LogE ( String ^ msg )
	{
		StringBuilder ^ sbl = gcnew StringBuilder ( GetTimeString () + "[ERROR] -- E --> " ); sbl->Append ( msg );
		LOG_OUT ( sbl );
	}


	/**
	* Output an error message through the system console output.
	*
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	void Utils::LogE ( String ^ className, String^  msg )
	{
		StringBuilder ^ sbl = gcnew StringBuilder ( GetTimeString () + "[ERROR] -- E --> " ); sbl->Append ( className )->Append ( "." )->Append ( msg );
		LOG_OUT ( sbl );
	}


	/**
	* Output a warning through the system console output.
	*
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	void Utils::LogW ( String ^ msg )
	{
		StringBuilder ^ sbl = gcnew StringBuilder ( GetTimeString () + "[WARN]   " ); sbl->Append ( msg );
		LOG_OUT ( sbl );
	}


	/**
	* Output a warning through the system console output.
	*
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	void Utils::LogW ( String ^ className, String ^ msg )
	{
		StringBuilder ^ sbl = gcnew StringBuilder ( GetTimeString () + "[WARN]   " ); sbl->Append ( className )->Append ( "." )->Append ( msg );
		LOG_OUT ( sbl );
	}


	/**
	* Output a log message through the diagonostics debug output.
	*
	* @param level     The severity level of the message.
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	//[ Conditional ( "DEBUG" ) ]
	void Utils::Log ( int level, String ^ className, String ^ msg )
	{
		if ( logLevel < level )
			return;

		StringBuilder ^ sbl = gcnew StringBuilder ( className );
		sbl ->Append ( "." )->Append ( msg );

		Log ( level, sbl->ToString (), false );
	}


	/**
	* Output a log message also in release builds through the diagonostics debug output.
	*
	* @param level     The severity level of the message.
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	void Utils::Log1 ( String ^ className, String ^ msg )
	{
		Log ( className, msg );
	}


	/**
	* Output a log message also in release builds through the diagonostics debug output.
	*
	* @param level     The severity level of the message.
	* @param className The className that the error message is related to.
	* @param msg       An error message to be logged.
	*/
	void Utils::Log ( String ^ className, String ^ msg )
	{
		StringBuilder ^ sbl = gcnew StringBuilder ( className );
		sbl->Append ( "." )->Append ( msg );

		Log ( 1, sbl->ToString (), false );
	}


	String ^ Utils::GetExecPath ()
	{
		return Environment::CurrentDirectory;
	}


	bool Utils::CreateUpdateBitmap ( System::Windows::Media::Imaging::WriteableBitmap ^% wbm, AvContext ^ context )
	{
		if ( wbm == nullptr )
		{
			int channels = ( context->stride / context->width );

			PixelFormat format = PixelFormats::Default;

			if ( channels == 1 ) format = PixelFormats::Gray8;
			else if ( channels == 3 ) format = PixelFormats::Bgr24;
			else if ( channels == 4 ) format = PixelFormats::Bgr32;
			else return false;

			wbm = gcnew System::Windows::Media::Imaging::WriteableBitmap ( context->width, context->height, 96, 96, format, nullptr );
			if ( wbm == nullptr )
				return false;
		}

		memcpy ( wbm->BackBuffer.ToPointer (), context->data->ToPointer (), context->stride * context->height );

		wbm->Lock ();
		wbm->AddDirtyRect ( Int32Rect ( 0, 0, wbm->PixelWidth, wbm->PixelHeight ) );
		wbm->Unlock ();

		return true;
	}


	bool Utils::CreateUpdateBitmap ( System::Windows::Media::Imaging::WriteableBitmap ^% wbm, IntPtr ^ avContext )
	{
		AvContext ^ context = GetAvContext ( avContext );
		if ( context == nullptr )
			return false;

		return CreateUpdateBitmap ( wbm, context );
	}


	AvContext ^ Utils::GetAvContext ( IntPtr ^ avContext )
	{
		AvContext ^ context = gcnew AvContext ();

		int* pInt = ( int* ) avContext->ToPointer();
		context->width = *pInt++;
		context->height = *pInt++;
		context->stride = *pInt++;
		pInt++; /// skip the 64 bit align spare scatterItem
		context->data = gcnew IntPtr ( ( char* ) (size_t) *pInt );

		if ( context->data == nullptr )
			return nullptr;

		return context;
	}

	namespace lib
	{
		ref class DeviceListAppendClass
		{
			devListRef ( EPSPACE DeviceInstance ) list;
			EPSPACE DeviceInstance ^ device;

		public:
			DeviceListAppendClass ( devListRef ( EPSPACE DeviceInstance ) list_, EPSPACE DeviceInstance ^ device_ ) : list ( list_ ), device ( device_ ) {}
			void Run () { list->Add ( device ); }
		};

		void DeviceListAppendFunc ( devListRef ( EPSPACE DeviceInstance ) list, EPSPACE DeviceInstance ^ device )
		{
			DeviceListAppendClass ^ act = gcnew DeviceListAppendClass ( list, device );

			Action ^ action = gcnew Action ( act, &DeviceListAppendClass::Run );

			environs::Environs::dispatchSync ( action );
		}


		ref class DeviceListInsertClass
		{
			devListRef ( EPSPACE DeviceInstance ) list;
			EPSPACE DeviceInstance ^ device;
			int pos;

		public:
			DeviceListInsertClass ( devListRef ( EPSPACE DeviceInstance ) list_, EPSPACE DeviceInstance ^ device_, int pos_ ) : list ( list_ ), device ( device_ ), pos ( pos_ ) {}
			void Run () { list->Insert ( pos, device ); }
		};

		void DeviceListInsertFunc ( devListRef ( EPSPACE DeviceInstance ) list, EPSPACE DeviceInstance ^ device, int pos )
		{
			DeviceListInsertClass ^ act = gcnew DeviceListInsertClass ( list, device, pos );

			Action ^ action = gcnew Action ( act, &DeviceListInsertClass::Run );

			environs::Environs::dispatchSync ( action );
		}


		ref class DeviceListClearClass
		{
			devListRef ( EPSPACE DeviceInstance ) list;

		public:
			DeviceListClearClass ( devListRef ( EPSPACE DeviceInstance ) list_ ) : list ( list_ ) {}
			void Run () { 
//				while ( list->Count > 0 )
//					list->RemoveAt ( 0 );
				list->Clear (); 
			}
		};

		void DeviceListClearFunc ( devListRef ( EPSPACE DeviceInstance ) list )
		{
			DeviceListClearClass ^ act = gcnew DeviceListClearClass ( list );

			Action ^ action = gcnew Action ( act, &DeviceListClearClass::Run );

			environs::Environs::dispatchSync ( action );
		}


		ref class DeviceListRemoveAtClass
		{
			devListRef ( EPSPACE DeviceInstance ) list;
			int pos;

		public:
			DeviceListRemoveAtClass ( devListRef ( EPSPACE DeviceInstance ) list_, int pos_ ) : list ( list_ ), pos ( pos_ ) { }
			void Run () { list->RemoveAt ( pos ); }
		};

		void DeviceListRemoveAtFunc ( devListRef ( EPSPACE DeviceInstance ) list, int pos )
		{
			DeviceListRemoveAtClass ^ act = gcnew DeviceListRemoveAtClass ( list, pos );

			Action ^ action = gcnew Action ( act, &DeviceListRemoveAtClass::Run );

			environs::Environs::dispatchSync ( action );
		}


		environs::DeviceInfo ^ BuildDeviceInfoCli ( unsigned char * pDevice, bool mediator )
		{
			if ( pDevice == 0 )
				return nill;

			environs::DeviceInfo ^ device = gcnew environs::DeviceInfo ();


			device->deviceID = *( ( int* ) ( pDevice ) );
			device->nativeID = *( ( int* ) ( pDevice + DEVICEINFO_NATIVE_ID_START ) );

			unsigned int IP = *( ( unsigned int* ) ( pDevice + DEVICEINFO_IP_START ) );
			device->ip = IP;
			/*String.Format ( "{0:D}.{1:D}.{2:D}.{3:D}",
			( IP & 0xff ),
			( IP >> 8 & 0xff ),
			( IP >> 16 & 0xff ),
			( IP >> 24 & 0xff ) );*/

			IP = *( ( unsigned int* ) ( pDevice + DEVICEINFO_IPe_START ) );
			device->ipe = IP;
			/*String::Format ( "{0:D}.{1:D}.{2:D}.{3:D}",
			( IP & 0xff ),
			( IP >> 8 & 0xff ),
			( IP >> 16 & 0xff ),
			( IP >> 24 & 0xff ) );*/

			device->tcpPort = *( ( unsigned short* ) ( pDevice + DEVICEINFO_TCP_PORT_START ) );
			device->udpPort = *( ( unsigned short* ) ( pDevice + DEVICEINFO_UDP_PORT_START ) );
			device->updates = *( ( unsigned int* ) ( pDevice + DEVICEINFO_UPDATES_START ) );

			int platform = *( ( int* ) ( pDevice + DEVICEINFO_PLATFORM_START ) );
			device->platform = platform;

			device->broadcastFound = ( int ) *( pDevice + DEVICEINFO_BROADCAST_START );
			if ( mediator )
			{
				if ( device->broadcastFound == 0 )
					device->broadcastFound = DEVICEINFO_DEVICE_MEDIATOR;
				else
					device->broadcastFound = DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR;
			}

			device->unavailable = *( pDevice + DEVICEINFO_UNAVAILABLE_START ) == 1 ? true : false;

            device->isConnected = *( pDevice + DEVICEINFO_ISCONNECTED_START ) == 1 ? true : false;
            device->hasAppEnv = (char) *( pDevice + DEVICEINFO_HASAPPAREA_START );

			device->deviceName = CCharToString ( pDevice + DEVICEINFO_DEVICENAME_START );
			device->areaName = CCharToString ( pDevice + DEVICEINFO_AREANAME_START );
			device->appName = CCharToString ( pDevice + DEVICEINFO_APPNAME_START );

			device->flags = *( ( unsigned short* ) ( pDevice + DEVICEINFO_FLAGS_START ) );

			device->objID = *( ( OBJIDType* ) ( pDevice + DEVICEINFO_OBJID_START ) );
			return device;
		}


		cli::array < environs::DeviceInstance ^ > ^ BuildDeviceInfoList ( int hInst, IntPtr ^pDevices, bool mediator )
		{
			if ( pDevices == nill || pDevices == IntPtr::Zero )
				return nill;

			cli::array < environs::DeviceInstance ^ > ^ devices = nill;

			int DeviceInstSize = environs::Environs::DeviceInstanceSize;

			unsigned char * pByteStart = ( unsigned char * ) pDevices->ToPointer ();
			unsigned char * pByte = pByteStart;
			unsigned int * pUI = ( unsigned int * ) pDevices->ToPointer ();
			unsigned int  devicesAvailable = *pUI; pUI++;
			unsigned int  startIndex = *pUI; pUI++;
			unsigned int  devicesCount = *pUI;
			if ( devicesCount <= 0 )
				return nill;

			pByte += environs::Environs::DevicesHeaderSize;

			devices = gcnew cli::array < environs::DeviceInstance ^ > ( devicesCount );

			for ( unsigned int i = 0; i < devicesCount; i++ )
			{
				environs::DeviceInfo ^ info = BuildDeviceInfoCli ( pByte, mediator );
				if ( info != nullptr ) 
				{
					environs::DeviceInstance ^device = gcnew environs::DeviceInstance ();
					if ( device != nullptr ) 
					{
						device->info_ = info;
						devices [ i ] = device;
					}
				}

				pByte += DeviceInstSize;
			}

			return devices;
		}


		char *  BuildNativePortalInfo ( PortalInfoBase ^ info )
		{
			if ( info->portalID == 0 )
				return 0;

			char * buffer = new char [ 36 ];
			if ( !buffer )
				return 0;

			int * pInt = ( int* ) buffer;
			*pInt++ = info->portalID;

			if ( info->flags == 0 )
				info->flags = 0xFFFF;
			*pInt++ = info->flags;

			*pInt++ = info->centerX;
			*pInt++ = info->centerY;
			*pInt++ = info->width;
			*pInt++ = info->height;
			*( ( float* ) pInt ) = info->orientation;

			return buffer;
		}


		/// <summary>
		/// ParsePortalInfo
		/// </summary>
		/// <param name="deviceID"></param>
		/// <param name="nativeMem"></param>
		/// <returns>PortalInfo object</param>
		PortalInfoBase ^ ParsePortalInfo ( IntPtr ^ portalInfoStruct )
		{
			PortalInfoBase ^ info = gcnew PortalInfoBase ();
			if ( info != nullptr )
			{
				int* pInt = ( int* ) portalInfoStruct->ToPointer ();
				info->portalID = *pInt++;
				info->flags = *pInt++;
				info->centerX = *pInt++;
				info->centerY = *pInt++;
				info->width = *pInt++;
				info->height = *pInt++;
				info->orientation = *( ( float* ) pInt );
			}

			return info;
		}


		environs::InputPack ^ GetInputPack ( IntPtr data )
		{
			InputPack ^ pack = gcnew InputPack ();
			if ( pack != nullptr )
			{
				char* bytes = (char*)data.ToPointer();
				int* pIntStart = (int*)bytes;

				pack->id = *pIntStart; pIntStart += 3;

				pack->x = *pIntStart; pIntStart++;
				pack->y = *pIntStart; pIntStart++;
				pack->value = *pIntStart; pIntStart++;

				float* pFloat = (float*)pIntStart;
				pack->angle = *pFloat; pFloat++;
				pack->size = *pFloat; pFloat++;
				pack->axisx = *pFloat; pFloat++;
				pack->axisy = *pFloat; pFloat++;

				short* pShort = (short*)(bytes + 8);
				pack->state = *pShort;

				pack->type = (char)*(bytes + 10);
			}
			return pack;
		}


		environs::SensorFrame ^ GetSensorInputPack ( Addr_ptr data )
		{
			environs::SensorFrame ^ pack = gcnew environs::SensorFrame ();
			if ( pack != nill )
			{
				char * bytes = ( char* ) data->ToPointer ();
				int * pIntStart = ( int* ) bytes;

				pIntStart++;

				int type = *pIntStart; pIntStart++;
				pack->type = ( environs::SensorType ) type;
				pack->id = *pIntStart; pIntStart++;

				float* pFloat = ( float* ) pIntStart;

				if ( ( type & ENVIRONS_SENSOR_PACK_TYPE_EXT ) != 0 )
				{
					pack->type = ( environs::SensorType ) ( type & (~ENVIRONS_SENSOR_PACK_TYPE_EXT ) );

					pack->f1 = *pFloat; pFloat++;
					pack->f2 = *pFloat; pFloat++;
					pack->f3 = *pFloat; pFloat++;

					double* pDouble = ( double* ) pFloat;
					pack->x = *pDouble; pDouble++;
					pack->y = *pDouble; pDouble++;
					pack->z = *pDouble;
				}
				else
				{
					pack->x = *pFloat; pFloat++;
					pack->y = *pFloat; pFloat++;
					pack->z = *pFloat;
				}
			}
			return pack;
		}


		environs::DeviceDisplay ^ BuildDeviceDisplayProps ( void * pData, int nativeID )
		{
			environs::DeviceDisplay ^props = gcnew ( DeviceDisplay );

			if ( pData && props )
			{
				props->nativeID = nativeID;
				int* pInts = ( int* ) pData;

				props->width = *pInts;
				props->height = *( pInts + 1 );
				props->width_mm = *( pInts + 2 );
				props->height_mm = *( pInts + 3 );
				props->orientation = *( pInts + 4 );
				//props->dpi = *((float *)(pInts + 5));
			}
			return props;
		}
	}

	// Number of milliseconds since system has started
	INTEROPTIMEVAL GetEnvironsTickCount ()
	{
		return ( INTEROPTIMEVAL ) Environment::TickCount;
	}


	unsigned long long GetUnixEpoch ()
	{
		return ( unsigned long long ) ( DateTime::UtcNow - DateTime ( 1970, 1, 1, 0, 0, 0 ) ).TotalSeconds;
	}


	void COutLog ( CString_ptr className, CString_ptr prefix, CString_ptr msg )
	{
		if ( msg == nill || msg->Length <= 0 ) {
			return;
		}
		
		StringBuilder ^ sbl = gcnew StringBuilder ( GetTimeString () );
		sbl->Append ( prefix );

		Debug::WriteLine ( sbl->Append ( " " )->Append ( className )->Append ( " " )->Append ( msg ) );
	}


	void COutLog ( CString_ptr msg )
	{
		if ( msg == nill || msg->Length <= 0 ) {
			return;
		}

		Debug::WriteLine ( GetTimeString () + msg );
	}


	size_t GetSizeOfFile ( CString_ptr filePath )
	{
		try {
			if ( File::Exists ( filePath ) ) {
				FileInfo^ fi = gcnew FileInfo ( filePath );
				return ( size_t ) fi->Length;
			}
		}
		catch ( ... ) {
			CErr ( "GetSizeOfFile: Exception." );
		}
		return 0;
	}

}

#endif



