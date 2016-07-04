/**
 * Example Window Win32
 * Skeleton created by MS VS 2010
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

#ifndef NDEBUG
#	define DEBUGVERB
#	define DEBUGVERBVerb
#endif

#include "WindowWin32.h"

#include <atlstr.h>
#include <atlimage.h>
#include <commctrl.h>

#include "Environs.h"
#include "Environs.Native.h"
#include "Environs.Build.Macros.h"
#include "Environs.Build.Lnk.h"

#include "Observer.h"
#include "Device.List.h"
#include "Device.Instance.h"
#include "Message.Instance.h"

using namespace std;
using namespace environs;

#define CLASS_NAME	"WindowWin32. . . . . . ."


#ifdef USE_STATIC_OBJECT
#pragma comment ( lib, ENVIRONS_PLATFORM_TOOLSET_LIB )
#endif

HINSTANCE		hInst				= 0;
HBRUSH			hBrushBackground	= 0;
HBITMAP			hBackGround			= 0;
HWND			hDlgMain			= 0;
HWND			hWndList			= 0;
HWND			hWndLog				= 0;
HWND			hWndDisConnect		= 0;
HWND			hWndMessage			= 0;
HWND			hWndSend			= 0;
HWND			hWndClearMessages	= 0;
HWND			hWndClearFiles		= 0;

TCHAR		*	szWindowClass		= L"Environs_Win32_Example_Window";


sp ( Observer )		obs;
sp ( Environs )		env;
sp ( DeviceList )	deviceList;
spv ( string )		logs ( new vector<string> () );


INT_PTR CALLBACK WndProcMain ( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

ATOM			MyRegisterClass ( HINSTANCE hInstance );
void			OnPaint ( HWND hWnd );
bool			InitEnvirons ();


BOOL InitInstance ( HINSTANCE hInstance, int nCmdShow )
{

	HWND hWnd = CreateWindow ( szWindowClass, L"Environs Win32 Simple Example Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 
		600, 400, NULL, NULL, hInstance, NULL );

	if ( !hWnd )
		return FALSE;

	InitEnvirons ();

	ShowWindow ( hWnd, nCmdShow );
	UpdateWindow ( hWnd );
	return TRUE;
}


int APIENTRY _tWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	CVerbN ( "_tWinMain" );

	INIT_ENVIRONS_LOG ();

	MSG msg;

	UNREFERENCED_PARAMETER ( hPrevInstance );
	UNREFERENCED_PARAMETER ( lpCmdLine );

	hInst = hInstance;

	HWND hDlg = CreateDialogParam ( hInstance, MAKEINTRESOURCE ( IDD_MAINDLG ), 0, WndProcMain, 0 );
	if ( !hDlg ) {
		return 0;
	}
	hDlgMain = hDlg;

	hWndList = GetDlgItem ( hDlg, IDC_LISTDEVICES );
	if ( !hWndList )
		return 0;

	hWndLog = GetDlgItem ( hDlg, IDC_LOG1 );
	if ( !hWndLog )
		return 0;

	hWndDisConnect = GetDlgItem ( hDlg, IDC_DISCONNECT );
	if ( !hWndDisConnect )
		return 0;

	/// Disable button
	EnableWindow ( hWndDisConnect, FALSE );

	hWndMessage = GetDlgItem ( hDlg, IDC_MESSAGE );
	if ( !hWndMessage )
		return 0;

	hWndSend = GetDlgItem ( hDlg, IDC_SEND );
	if ( !hWndSend )
		return 0;

	hWndClearMessages = GetDlgItem ( hDlg, IDC_CLEARMESSAGES );
	if ( !hWndClearMessages )
		return 0;

	hWndClearFiles = GetDlgItem ( hDlg, IDC_CLEARFILES );
	if ( !hWndClearFiles )
		return 0;

	ShowWindow ( hDlg, nCmdShow );
	
	InitEnvirons ();

	while ( GetMessage ( &msg, NULL, 0, 0 ) )
	{
		if ( !IsDialogMessage ( hDlg, &msg ) ) {
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
		}
	}

	if ( deviceList && obs )
		deviceList->RemoveObserver ( obs.get () );

	if ( env && obs ) {
		env->RemoveObserver ( obs.get () );
		env->RemoveObserverForData ( obs.get () );
		env->RemoveObserverForMessages ( obs.get () );
		env->RemoveObserverForSensorData ( obs.get () );
	}

	if ( hBrushBackground )
		DeleteObject ( hBrushBackground );

	if ( hBackGround )
		DeleteObject ( hBackGround );

	return (int) msg.wParam;
}


INT_PTR CALLBACK WndProcAbout ( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER ( lParam );
	switch ( message )
	{
	case WM_INITDIALOG:
		return (INT_PTR) TRUE;

	case WM_COMMAND:
		if ( LOWORD ( wParam ) == IDOK || LOWORD ( wParam ) == IDCANCEL )
		{
			EndDialog ( hDlg, LOWORD ( wParam ) );
			return (INT_PTR) TRUE;
		}
		break;
	}
	return (INT_PTR) FALSE;
}


sp ( DeviceInstance ) GetCurrentDevice ()
{
	int index = (int) SendMessage ( hWndList, LVM_GETNEXTITEM, -1, LVNI_SELECTED );
	if ( index < 0 ) {
		return 0;
	}

	sp ( DeviceInstance ) device = deviceList->GetItem ( index );
	if ( !device ) {
		EnableWindow ( hWndDisConnect, FALSE );
	}
	return device;
}


void HandleSelectedDevice ()
{
	sp ( DeviceInstance ) device = GetCurrentDevice ();
	if ( !device )
		return;

	EnableWindow ( hWndDisConnect, TRUE );

	if ( device->isConnected () ) {
		SendMessageA ( hWndDisConnect, WM_SETTEXT, 0, (LPARAM) "Disconnect" );
	}
	else {
		SendMessageA ( hWndDisConnect, WM_SETTEXT, 0, (LPARAM) "Connect" );
	}
}


void HandleDisConnect ()
{
	sp ( DeviceInstance ) device = GetCurrentDevice ();
	if ( !device )
		return;

	if ( device->isConnected () ) {
		device->Disconnect ();
	}
	else {
		device->Connect ();
	}
}


void HandleClearMessages ()
{
	sp ( DeviceInstance ) device = GetCurrentDevice ();
	if ( !device )
		return;
	device->ClearMessages ();
}


void HandleClearStorage ()
{
	sp ( DeviceInstance ) device = GetCurrentDevice ();
	if ( !device )
		return;
	device->ClearStorage ();
}


void HandleSendMessage ()
{
	static char buffer [1024];
	int length = GetWindowTextA ( hWndMessage, (LPSTR)buffer, 1024 );
	if ( length <= 0 )
		return;

	sp ( DeviceInstance ) device = GetCurrentDevice ();
	if ( !device )
		return;
	device->SendMessage ( buffer );
}


INT_PTR CALLBACK WndProcMain ( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	RECT rc;

	UNREFERENCED_PARAMETER ( lParam );
	switch ( message )
	{
	case WM_PAINT:
		OnPaint ( hDlg );
		break;

	case WM_CTLCOLORLISTBOX:
	{
		SetBkMode ( (HDC) wParam, TRANSPARENT );
		return (INT_PTR)::GetStockObject ( NULL_PEN );
		break;
	}

	case WM_INITDIALOG:
		GetWindowRect ( hDlg, &rc );
		MoveWindow ( hDlg, 100, 100, rc.right, rc.bottom, TRUE );
		return (INT_PTR) TRUE;

	case WM_COMMAND:
		if ( LOWORD ( wParam ) == IDOK )
		{
			SendMessage ( hDlg, WM_CLOSE, 0, 0 );
			return (INT_PTR) TRUE;
		}
		if ( LOWORD ( wParam ) == IDC_LISTDEVICES )
		{
			HandleSelectedDevice ();
			return (INT_PTR) TRUE;
		}

		if ( LOWORD ( wParam ) == IDC_DISCONNECT )
		{
			HandleDisConnect ();
			return (INT_PTR) TRUE;
		}

		if ( LOWORD ( wParam ) == IDC_CLEARMESSAGES )
		{
			HandleClearMessages ();
			return (INT_PTR) TRUE;
		}

		if ( LOWORD ( wParam ) == IDC_CLEARFILES )
		{
			HandleClearStorage ();
			return (INT_PTR) TRUE;
		}

		if ( LOWORD ( wParam ) == IDC_SEND )
		{
			HandleSendMessage ();
			return (INT_PTR) TRUE;
		}
		break;

	case WM_CLOSE:
		DestroyWindow ( hDlg );
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage ( 0 );
		return TRUE;
	}

	return (INT_PTR) FALSE;
}


bool InitEnvirons ()
{
	CVerbN ( "InitEnvirons" );

#ifdef USE_STATIC_OBJECT
	env = Environs_CreateInstanceStaticLinked ();
#else
	env = Loader::CreateInstance ();
#endif
	if ( !env ) {
		CErrN ( "InitInstance: Failed to create an Environs object!" );
		return false;
	}

	hBackGround = ( HBITMAP ) env->LoadPicture ( "./data/hcmbg.jpg" );
	if ( !hBackGround )
		hBackGround = ( HBITMAP ) env->LoadPicture ( "./hcmbg.jpg" );

	if ( hBackGround )
		hBrushBackground = CreatePatternBrush ( hBackGround );

	if ( !hBrushBackground )
		CLogN ( "InitInstance: Could not load background image!" );

	env->LoadSettings ( "WindowWin32", "Environs" );

	env->SetUseMediatorAnonymousLogon ( true );

	deviceList = env->CreateDeviceList ( DeviceClass::All );
	if ( !deviceList ) {
		CErrN ( "InitInstance: Failed to create a DeviceList!" );
		return false;
	}

	obs = make_shared<Observer> ();
	if ( !obs ) {
		CErrN ( "InitInstance: Failed to create an Observer!" );
		return false;
	}

	env->AddObserver ( obs.get () );
	env->AddObserverForData ( obs.get () );
	env->AddObserverForMessages ( obs.get () );
	env->AddObserverForSensorData ( obs.get () );
	deviceList->AddObserver ( obs.get () );

	env->Start ();
	return true;
}


void OnPaint ( HWND hWnd )
{
	PAINTSTRUCT     ps;
	HDC             hDC;
	RECT			rc;

	if ( GetUpdateRect ( hWnd, &rc, 0 ) ) {
		hDC = BeginPaint ( hWnd, &ps );
		FillRect ( hDC, &ps.rcPaint, hBrushBackground );
		EndPaint ( hWnd, &ps );
	}
}


Observer::~Observer ()
{
	CVerbN ( "Observer.Destruct" );
}


/**
* OnStatus is called whenever the framework status changes.&nbsp;
*
* @param status      A status constant of type Status
*/
void Observer::OnStatus ( Status_t status )
{
	CVerbArgN ( "OnStatus: status [%i]", status );

}

/**
* OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
* The notification parameter is an integer value which represents one of the values as listed in Types.*
* The string representation can be retrieved through TypesResolver.get(notification).
*
* @param context		An object reference of type environs::NotifyContext.
*/
void Observer::OnNotify ( environs::ObserverNotifyContext * context )
{
	CVerbArgN ( "OnNotify: nativeID [%i], notification [%i], source [%i]", context->nativeID, context->notification, context->source );

}

/**
* OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
* The notification parameter is an integer value which represents one of the values as listed in Types.*
* The string representation can be retrieved through TypesResolver.get(notification).
*
* @param context		An object reference of type environs::NotifyContext.
*/
void Observer::OnNotifyExt ( environs::ObserverNotifyContext * context )
{
	CVerbArgN ( "OnNotifyExt: deviceID [%i], area [%s], app [%s], notification [%i], source [%i], context [%i]", context->nativeID, context->areaName, context->appName, context->notification, context->source, context->context );

}


/**
* OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
*
* @param portal 		The PortalInstance object.
*/
void Observer::OnPortalRequestOrProvided ( const sp ( PortalInstance ) &portal )
{
	CVerbN ( "OnPortalRequestOrProvided" );
}


void UpdateDeviceList ()
{
	CVerbN ( "UpdateDeviceList" );

	if ( !hWndList ) {
		hWndList = GetDlgItem ( hDlgMain, IDC_LISTDEVICES );
		if ( !hWndList )
			return;
	}

	SendMessage ( hWndList, LB_RESETCONTENT, 0, 0 );

	sp ( DeviceInstanceList ) devices = deviceList->GetDevices ();

	if ( devices && devices.get () ) 
	{
		size_t size = devices->size ();

		for ( size_t i = 0; i < size; ++i )
		{
			sp ( DeviceInstance ) device = devices->at ( i );

			int pos = (int) SendMessageA ( hWndList, LB_ADDSTRING, 0, (LPARAM) device->toString ().c_str () );

			SendMessage ( hWndList, LB_SETITEMDATA, pos, (LPARAM) i );
		}

		SetFocus ( hWndList );
	}

	InvalidateRect ( hDlgMain, NULL, TRUE );
}


/**
* OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
*
* @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
* @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
*/
void Observer::OnListChanged ( const sp ( DeviceInstanceList ) &vanished, const sp ( DeviceInstanceList ) &appeared )
{
	CVerbN ( "OnListChanged" );

	if ( vanished ) {
		for ( size_t i = 0; i < vanished->size (); ++i ) {
			sp ( DeviceInstance ) device = vanished->at ( i );
			if ( device )
				device->RemoveObserverForMessages ( obs.get () );
		}
	}

	if ( appeared ) {
		for ( size_t i = 0; i < appeared->size (); ++i ) {
			sp ( DeviceInstance ) device = appeared->at ( i );
			if ( device )
				device->AddObserverForMessages ( obs.get () );
		}
	}

	UpdateDeviceList ();
}


/**
* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
*
* @param device			The DeviceInstance object that sends this notification.
* @param changedFlags	The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
*/
void Observer::OnDeviceChanged ( const sp ( DeviceInstance ) &device, DeviceInfoFlag_t changedFlags )
{
	CVerbN ( "OnDeviceChanged" );

	UpdateDeviceList ();
}


/**
* OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
*
	* @param context		An object reference of type environs::ObserverMessageContext.
*/
void Observer::OnMessage ( environs::ObserverMessageContext * context )
{
	CVerbArgN ( "OnMessageE: nativeID [%i], type [%i], message [%i], size [%i]", context->nativeID, context->type, context->message, context->length );
}


/**
* OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
*
	* @param context		An object reference of type environs::ObserverMessageContext.
*/
void Observer::OnMessageExt ( environs::ObserverMessageContext * context )
{
	CVerbArgN ( "OnMessageExtE: deviceID [%i], area [%s], app [%s], type [%i], message [%s], msgLength [%i]", context->deviceID, context->areaName, context->appName, context->type, context->message, context->length );
}


/**
* OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
*
* @param message      The status as a text message.
*/
void Observer::OnStatusMessage ( const char * message )
{
	CVerbN ( "OnStatusMessage" );

	if ( !message || !hWndLog || !logs )
		return;

	if ( logs->size () > 15 ) {
		logs->erase ( logs->begin () );
	}

	string line = message;
	line += "\n";

	logs->push_back ( line );

	string log;
	for ( size_t i=0; i < logs->size (); ++i )
		log += logs->at ( i );

	SendMessageA ( hWndLog, WM_SETTEXT, 0, (LPARAM) log.c_str () );
}


/**
* OnMessage is called whenever a text message has been received from a device.
*
* @param msg			The corresponding message object of type MessageInstance
* @param changedFlags	Flags that indicate the object change.
*/
void Observer::OnMessage ( const sp ( MessageInstance ) &msg, MessageInfoFlag_t changedFlags )
{
	CVerbN ( "OnMessage" );
}


/**
* OnData is called whenever new binary data (files, buffers) has been received.
* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
*
* @param context		An object reference of type environs::ObserverDataContext.
*/
void Observer::OnData ( environs::ObserverDataContext * context )
{
	CVerbArgN ( "OnDataE: nativeID [%i], type [%i], fileID [%i], desc [%i], size [%i]", context->nativeID, context->sourceType, context->fileID, context->descriptor, context->size );

}


/**
* OnData is called whenever new binary data (files, buffers) has been received.
* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
*
* @param fileData		The corresponding file object of type FileInstance
* @param changedFlags	Flags that indicate the object change.
*/
void Observer::OnData ( const sp ( FileInstance ) &fileData, FileInfoFlag_t changedFlags )
{
	CVerbN ( "OnData" );

}


/**
* OnSensorData is called whenever new binary data (files, buffers) has been received.
* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
*
* @param nativeID      The native identifier that targets the device.
* @param pack          The frame containing the sensor data
*/
void Observer::OnSensorData ( int nativeID, environs::SensorFrame * pack )
{
	CVerbN ( "OnSensorData" );

}

/**
* OnSensorData is called whenever new sensor data has been received.
*
* @param pack     The corresponding SensorFrame of sensor data
*/
void Observer::OnSensorData ( environs::SensorFrame * pack )
{
	CVerbN ( "OnSensorData" );
}


/**
* OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
*
* @param portal		The PortalInstance object.
* @param notify		The notification (Notify::Portal) that indicates the change.
*/
void Observer::OnPortalChanged ( const sp ( PortalInstance ) &portal, Notify::Portal_t notify )
{
	CVerbN ( "OnPortalChanged" );

}



