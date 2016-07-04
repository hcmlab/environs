/**
* Windows GDI screen capture class-threaded
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#ifdef _WIN32

#include "Environs.Obj.h"
#include "Capture.Win.TGDI.h"
#include "Interfaces/IPortal.Renderer.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"CaptureWinTGDI"



namespace environs 
{	
	PortalBufferType_t	CaptureWinTGDI_outputTypeSupport[] = { PortalBufferType::ARGBHandle, PortalBufferType::ARGB };

	LONGSYNC		CaptureWinTGDI::referenceCount	= 0;

	HWND			winTGDIMainWindow				= 0;
	HDC				winTGDIMainDC					= 0;
	HANDLE			winTGDIgrabbedEvent				= 0;
	CRITICAL_SECTION winTGDIgrabbedFrameCS;

	LONGSYNC		grabberThreadIDState			= ENVIRONS_THREAD_NO_THREAD;
	pthread_t		grabberThreadID					= 0;
	HANDLE			grabberThreadEvent				= 0;
	Instance	*	grabberThreadEnv				= 0;

#ifdef USE_BACK_BUFFERING
	HDC				winTGDIgrabbedFrameDC[ USE_BACK_BUFFERING ];
	LONGSYNC		CaptureWinTGDI::classCaptureAccessed = 0;

	HBITMAP			classCaptureBitmap				[USE_BACK_BUFFERING];
	HBITMAP			classCaptureBitmapOld			[USE_BACK_BUFFERING];

	LONGSYNC		classCaptureAccessLock[ USE_BACK_BUFFERING ];
	unsigned int	classCaptureBitmapIndexNext		= 0;
#else
	HDC				winTGDIgrabbedFrameDC = 0;
	LONGSYNC 		CaptureWinTGDI::classCaptureAccessed	= 1;

	HBITMAP			classCaptureBitmap				= 0;
	HBITMAP			classCaptureBitmapOld			= 0;
#endif

	int				winTGDIScreenWidth				= 0;
	int				winTGDIScreenHeight				= 0;

	/// static lock start 
	class winTGDIInitLockClass
	{
	public:
		winTGDIInitLockClass ( )
		{
			if ( !didiInitStaticInitDelLock )
			{
				didiInitStaticInitDelLock = true;
				InitializeCriticalSection ( &staticInitDelLock );
			}
		}

		~winTGDIInitLockClass ( )
		{
			DeleteCriticalSection ( &staticInitDelLock );
		}

		static CRITICAL_SECTION		staticInitDelLock;

	private:
		static bool					didiInitStaticInitDelLock;
	};

	CRITICAL_SECTION		winTGDIInitLockClass::staticInitDelLock;
	bool					winTGDIInitLockClass::didiInitStaticInitDelLock = false;

	static winTGDIInitLockClass staticLock;
	/// static lock end

CaptureWinTGDI::CaptureWinTGDI ( )
{
	name				= "Windows GDI Threaded Grabber";

	CLogArgID ( "Construct: [%s]", name );

	captureType			= CaptureType::Screen;
	bufferType			= CaptureBufferType::PixelBuffer;

	didAddRef			= false;

	hDC					= 0;
	hBitmapCapturedOld	= 0;

	outputTypes			= CaptureWinTGDI_outputTypeSupport;
	outputTypesLength	= sizeof(CaptureWinTGDI_outputTypeSupport) / sizeof(CaptureWinTGDI_outputTypeSupport [0]);
}


int CaptureWinTGDI::Init ( )
{
	CVerbArgID ( "Init: [%s]", name );

	if ( !env->appWindowWidth || !env->appWindowHeight ) {
		CErrID ( "Init: Failed to retrieve system metrics for primary screen!" );
		return 0;
	}

	didAddRef = true;

	EnterCriticalSection ( &winTGDIInitLockClass::staticInitDelLock );

	if ( __sync_add_and_fetch ( &referenceCount, 1 ) == 1 )
	{
		winTGDIScreenWidth = env->appWindowWidth;
		winTGDIScreenHeight = env->appWindowHeight;

		env->desktopDrawLeft = 0;
		env->desktopDrawTop = 0;
		env->desktopDrawWidth = winTGDIScreenWidth;
		env->desktopDrawHeight = winTGDIScreenHeight;

		winTGDIMainWindow	= (HWND) hAppWindow;
		grabberThreadEnv	= env;

		if ( !ClassInit ( deviceID ) ) {
			LeaveCriticalSection ( &winTGDIInitLockClass::staticInitDelLock );
			CErrID ( "Init: Failed to initialize Interface resources for threading" );
			return -1;
		}
	}

	LeaveCriticalSection ( &winTGDIInitLockClass::staticInitDelLock );
	
	int ret = 1;

	if ( !hDC ) {
		hDC = CreateCompatibleDC ( winTGDIMainDC );
		if ( hDC ) {
			ret = 1;
			enabled = true;
		}
		else {
			ret = 0;
			CErrArgID ( "Init: Error [0x%X]", GetLastError ( ) );
		}
	}

	if ( ret > 0 )
		initialized = true;

	return ret;
}


bool CaptureWinTGDI::ClassInit ( int deviceID )
{
	CVerbID ( "ClassInit" );

#ifdef USE_BACK_BUFFERING
	classCaptureAccessed = USE_BACK_BUFFERING;

	if ( !winTGDIMainDC ) 
	{
		winTGDIMainDC = GetDC ( winTGDIMainWindow );
		if ( !winTGDIMainDC )
		{
			CErrID ( "ClassInit: GetDC ( winTGDIMainWindow ) failed." );
			return false;
		}
	}

	for ( unsigned int i = 0; i < USE_BACK_BUFFERING; i++ ) 
	{
		if ( !winTGDIgrabbedFrameDC[ i ] ) 
		{
			winTGDIgrabbedFrameDC[ i ] = CreateCompatibleDC ( winTGDIMainDC );
			if ( !winTGDIgrabbedFrameDC[ i ] )
			{
				CErrID ( "ClassInit: CreateCompatibleDC ( winTGDIMainDC ) failed." );
				return false;
			}
		}
	}
#else
	if ( !winTGDIgrabbedFrameDC ) {
		winTGDIMainDC = GetDC ( winTGDIMainWindow );
		if ( !winTGDIMainDC )
			goto Finish;

		winTGDIgrabbedFrameDC = CreateCompatibleDC ( winTGDIMainDC );
		if ( !winTGDIgrabbedFrameDC )
			goto Finish;
	}
#endif

#ifdef USE_BACK_BUFFERING
	Zero ( classCaptureBitmap );
	Zero ( classCaptureBitmapOld );

	for ( int i = 0; i < USE_BACK_BUFFERING; i++ )
	{
		classCaptureAccessLock[ i ] = 0;
	}

	unsigned char * classData;

	BITMAPINFO info;
	Zero ( info );

	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biSize = sizeof ( info.bmiHeader );
	info.bmiHeader.biWidth = winTGDIScreenWidth;
	info.bmiHeader.biHeight = -( ( int ) winTGDIScreenHeight );
	info.bmiHeader.biSizeImage = winTGDIScreenWidth * winTGDIScreenHeight * 4;

	for ( unsigned int i = 0; i < USE_BACK_BUFFERING; i++ )
	{
		classData = 0;

		classCaptureBitmap[ i ] = CreateDIBSection ( winTGDIgrabbedFrameDC[ i ], &info, DIB_RGB_COLORS, ( VOID** ) &classData, 0, 0 );
		if ( !classData ) {
			CErrArgID ( "ClassInit: Failed to retrieve memory pointer (CreateDIBSection) for bitmap of size [%u], Error [%i]", info.bmiHeader.biSizeImage, GetLastError ( ) );
			return false;
		}

		if ( !classCaptureBitmap[ i ] ) {
			CErrArgID ( "ClassInit: Failed to create hBitmap for capturing of [%u] bytes", info.bmiHeader.biSizeImage );
			return false;
		}

		classCaptureBitmapOld[ i ] = ( HBITMAP ) SelectObject ( winTGDIgrabbedFrameDC[ i ], classCaptureBitmap[ i ] );
	}

	classCaptureBitmapIndexNext = 0;
#else
	classCaptureBitmapOld = 0;
	classCaptureBitmap = 0;

	unsigned char * classData;

	BITMAPINFO info;
	Zero ( info );

	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biSize = sizeof ( info.bmiHeader );
	info.bmiHeader.biWidth = winTGDIScreenWidth;
	info.bmiHeader.biHeight = -( ( int ) winTGDIScreenHeight );
	info.bmiHeader.biSizeImage = winTGDIScreenWidth * winTGDIScreenHeight * 4;

	classData = 0;
	classCaptureBitmap = CreateDIBSection ( winTGDIgrabbedFrameDC, &info, DIB_RGB_COLORS, ( VOID** ) &classData, 0, 0 );
	if ( !classData ) {
		CErrArgID ( "ClassInit: Failed to retrieve memory pointer (CreateDIBSection) for bitmap of size [%u], Error [%i]", info.bmiHeader.biSizeImage, GetLastError ( ) );
		return false;
	}

	if ( !classCaptureBitmap ) {
		CErrArgID ( "ClassInit: Failed to create hBitmap for capturing of [%u] bytes", info.bmiHeader.biSizeImage );
		return false;
	}

	classCaptureBitmapOld = ( HBITMAP ) SelectObject ( winTGDIgrabbedFrameDC, classCaptureBitmap );
#endif

	InitializeCriticalSection ( &winTGDIgrabbedFrameCS );

	if ( !grabberThreadEvent ) {
		grabberThreadEvent = CreateEvent ( 0, TRUE, FALSE, 0 );
		if ( !grabberThreadEvent ) {
			CErrID ( "ClassInit: Failed to create thread event!" );
			return false;
		}
	}

	if ( !winTGDIgrabbedEvent ) {
		winTGDIgrabbedEvent = CreateEvent ( 0, TRUE, FALSE, 0 );
		if ( !winTGDIgrabbedEvent ) {
			CErrID ( "ClassInit: Failed to create thread captured event!" );
			return false;
	}
	}

	// Create capture thread
	if ( grabberThreadIDState == ENVIRONS_THREAD_NO_THREAD )
	{
		ResetEvent ( grabberThreadEvent );

		grabberThreadIDState = ENVIRONS_THREAD_DETACHEABLE;

		pthread_create ( &grabberThreadID, NULL, &CaptureWinTGDI::Thread_Grab, nill );
		if ( !pthread_valid ( grabberThreadID ) )
		{
			grabberThreadIDState = ENVIRONS_THREAD_NO_THREAD;

			CErrID ( "ClassInit: Failed to create thread for capturing!" );
			return false;
		}

		if ( WaitForSingleObject ( grabberThreadEvent, 1000 ) != WAIT_OBJECT_0 ) {
			CErrID ( "ClassInit: Failed to wait for thread to become ready!" );
			return false;
		}
	}

	return true;
}


CaptureWinTGDI::~CaptureWinTGDI ()
{
	CLogID ( "Destruct" );

	Release ();

	if ( didAddRef )
	{
		EnterCriticalSection ( &winTGDIInitLockClass::staticInitDelLock );

		if ( __sync_sub_and_fetch ( &referenceCount, 1 ) == 0 )
		{
			ClassDispose ( deviceID );
		}

		LeaveCriticalSection ( &winTGDIInitLockClass::staticInitDelLock );
	}
}


void CaptureWinTGDI::Release ( )
{
	CVerbID ( "Release" );

	initialized = false;

	ReleaseResources ( );

	if ( hDC ) {
		DeleteDC ( hDC );
		hDC = 0;
	}
}


void ClassDisposeBuffers ( int deviceID ) 
{
	CVerbID ( "ClassDisposeBuffers" );

#ifdef USE_BACK_BUFFERING
	for ( unsigned int i = 0; i < USE_BACK_BUFFERING; i++ ) {
		if ( classCaptureBitmapOld [i] ) {
			if ( winTGDIgrabbedFrameDC [i] ) {
				CVerbArgID ( "ClassDispose: selecting old bitmap for capture into screen DC %i", i );

				SelectObject ( winTGDIgrabbedFrameDC [i], classCaptureBitmapOld [i] );

				DeleteDC ( winTGDIgrabbedFrameDC [i] );
			}
			else {
				CErrArgID ( "ClassDispose: INVALID GDI STATE. An old DC bitmap nr. [%i] cannot be released due to a missing 'winTGDIgrabbedFrameDC'", i );
			}
		}

		if ( classCaptureBitmap [i] ) {
			CVerbArgID ( "ClassDispose: Deleting bitmap capture handle %i", i );

			DeleteObject ( classCaptureBitmap [i] );
		}
	}

	Zero ( classCaptureBitmap );
	Zero ( classCaptureBitmapOld );
	Zero ( winTGDIgrabbedFrameDC );
#else
	if ( classCaptureBitmapOld ) {
		if ( winTGDIgrabbedFrameDC ) {
			CVerbID ( "ClassDispose: selecting old bitmap for capture into portal DC" );

			SelectObject ( winTGDIgrabbedFrameDC, classCaptureBitmapOld );

			DeleteDC ( winTGDIgrabbedFrameDC );
		}
		else {
			CErrID ( "ClassDispose: INVALID GDI STATE. An old DC bitmap cannot be released due to a missing 'winTGDIgrabbedFrameDC'" );
		}
		classCaptureBitmapOld = 0;
	}

	if ( classCaptureBitmap ) {
		CLogID ( "ClassDispose: Deleting bitmap capture handle" );

		DeleteObject ( classCaptureBitmap );
		classCaptureBitmap = 0;
	}
#endif

	if ( winTGDIMainDC ) {
		ReleaseDC ( winTGDIMainWindow, winTGDIMainDC );
		winTGDIMainDC = 0;
	}
}


void CaptureWinTGDI::ClassDispose ( int deviceID )
{
	CVerbID ( "ClassDispose" );

	DisposeThread ( &grabberThreadIDState, grabberThreadID, 0, "CaptureWinTGDI class grabber thread", captureClassTimerSignal );
	
	ClassDisposeBuffers ( deviceID );

	if ( grabberThreadEvent ) {
		CloseHandle ( grabberThreadEvent );
		grabberThreadEvent = 0;
	}

	if ( winTGDIgrabbedEvent ) {
		CloseHandle ( winTGDIgrabbedEvent );
		winTGDIgrabbedEvent = 0;
	}

	DeleteCriticalSection ( &winTGDIgrabbedFrameCS );
}


int CaptureWinTGDI::ReleaseResources ( )
{
	CVerbID ( "ReleaseResources" );

	if ( hBitmapCapturedOld ) {
		if ( hDC ) {
			CVerbID ( "ReleaseResources: selecting old bitmap for capture into portal DC" );

			SelectObject ( hDC, hBitmapCapturedOld );
			hBitmapCapturedOld = 0;
		}
		else {
			CErrID ( "ReleaseResources: INVALID GDI STATE. An old DC bitmap cannot be released due to a missing PortalDC" );
		}
	}

	if ( dataHandle ) {
		CVerbID ( "ReleaseResources: Deleting bitmap capture handle" );

		DeleteObject ( dataHandle );
		dataHandle = 0;
	}

	buffersInitialized	= false;

	data			= 0;
	dataSize		= 0;
	squareLength	= 0;
    
    return 1;
}


int CaptureWinTGDI::AllocateResources ( RenderDimensions * dims )
{
	CVerbID ( "AllocateResources" );

	BITMAPINFO info;
	Zero ( info );

	info.bmiHeader.biBitCount		= 32;
	info.bmiHeader.biCompression	= BI_RGB;
	info.bmiHeader.biPlanes			= 1;
	info.bmiHeader.biSize			= sizeof(info.bmiHeader);
	info.bmiHeader.biWidth			= dims->square;
	info.bmiHeader.biHeight			= -((int)dims->square);
	info.bmiHeader.biSizeImage		= dims->square * dims->square * 4;

	data = 0;

	dataHandle = CreateDIBSection ( hDC, &info, DIB_RGB_COLORS, (VOID**) &data, 0, 0 );
	if ( !data ) {
		CErrArgID ( "AllocateResources: Failed to retrieve memory pointer (CreateDIBSection) for bitmap of size [%u], Error [%i]", (dims->square * dims->square * 4), GetLastError ( ) );
		return 0;
	}

	if ( !dataHandle ){
		CErrArgID ( "AllocateResources: Failed to create hBitmap for capturing of [%u] bytes", (dims->square * dims->square * 4) );
		return 0;
	}

	dataSize		= info.bmiHeader.biSizeImage;

	hBitmapCapturedOld = (HBITMAP) SelectObject ( hDC, (HBITMAP) dataHandle );

	return 1;
}


int CaptureWinTGDI::Perform ( RenderDimensions * dims, RenderContext * context )
{
	CVerbVerbID ( "Perform" );

	if ( WaitForSingleObject ( winTGDIgrabbedEvent, 500 ) != WAIT_OBJECT_0 )
		return 0;
	
	//GdiFlush();

	bool ret = 0;

	int left = 0;
	int leftc = dims->left_cap;
	int top = 0;
	int topc = dims->top_cap;
	int widtha = dims->square;
	int heighta = dims->square;

	/// grab that frame
	if ( dims->left_cap < 0 || dims->top_cap < 0 ) {
		if ( dims->left_cap < 0 ) {
			left = -dims->left_cap;
			widtha -= left;
			leftc = 0;
		}

		if ( dims->top_cap < 0 ) {
			top = -dims->top_cap;
			heighta -= top;
			topc = 0;
		}
	}

#ifdef USE_BACK_BUFFERING
	/// Get index of the next available frame
	unsigned int next = classCaptureBitmapIndexNext;

	/// Increase access counter on that frame
	InterlockedIncrement ( classCaptureAccessLock + next );

	/// grab that frame
	EnterCriticalSection ( &winTGDIgrabbedFrameCS );

	ret = BitBlt ( hDC, left, top, widtha, heighta, winTGDIgrabbedFrameDC [next], leftc, topc, SRCCOPY ) ? 1 : 0;

	LeaveCriticalSection ( &winTGDIgrabbedFrameCS );

	if ( !ret ) {
		CErrArgID ( "Perform: Error [0x%X]", GetLastError ( ) );			
	}

	/// Release access counter on that frame
	InterlockedDecrement ( classCaptureAccessLock + next );
	InterlockedIncrement ( &classCaptureAccessed );
#else
	EnterCriticalSection ( &winTGDIgrabbedFrameCS );

	ret = BitBlt ( hDC, left, top, widtha, heighta, winTGDIgrabbedFrameDC, leftc, topc, SRCCOPY ) ? 1 : 0;

	LeaveCriticalSection ( &winTGDIgrabbedFrameCS );

	if ( !ret ) {
		CErrArgID ( "Perform: Error [0x%X]", GetLastError ( ) );
	}

	InterlockedIncrement ( &classCaptureAccessed );
#endif

	return ret;
}


void * CaptureWinTGDI::Thread_Grab ( void * arg )
{
	CLog ( "Thread_Grab: CaptureWinTGDI class thread created." );

#ifdef USE_BACK_BUFFERING
	classCaptureAccessed = USE_BACK_BUFFERING;

	if ( !winTGDIMainDC ) {
		goto Finish;
	}

	for ( unsigned int i = 0; i < USE_BACK_BUFFERING; i++ ) {
		if ( !winTGDIgrabbedFrameDC [i] ) {
			goto Finish;
		}
	}
#else
	if ( !winTGDIMainDC  || !winTGDIgrabbedFrameDC ) {
		goto Finish;
	}
#endif

#ifdef USE_BACK_BUFFERING
	unsigned long clientAccessedCache = 0;
#else
	unsigned long clientAccessedCache = classCaptureAccessed + 2;
#endif

	SetEvent ( grabberThreadEvent );

	grabberThreadEnv->desktopDrawRequested = true;

	while ( referenceCount )
	{
		DWORD res = WaitForSingleObject ( captureClassTimerSignal, INFINITE );
		if ( res == WAIT_OBJECT_0 ) {
#ifdef USE_BACK_BUFFERING
			unsigned int next = classCaptureBitmapIndexNext + 1;
			if ( next >= USE_BACK_BUFFERING )
				next = 0;

			if ( InterlockedCompareExchange ( classCaptureAccessLock + next, 0, 0 ) != 0 ) {
				CVerbArg ( "Thread_Grab: Next frame [%i] is still locked by a grabber.", next );
				continue;
			}

			if ( ( LONG ) clientAccessedCache > classCaptureAccessed ) {
				//CVerbArg ( "Thread_Grab: Next frame [%i] has not been touched yet.", next );
				SetEvent ( winTGDIgrabbedEvent );
				continue;
			}

			if ( classCaptureAccessed - USE_BACK_BUFFERING > ( LONG ) clientAccessedCache )
				clientAccessedCache = classCaptureAccessed - USE_BACK_BUFFERING;
			else
				clientAccessedCache++;

			//CVerb ( "Thread_Grab: signaled." );
			ResetEvent ( winTGDIgrabbedEvent );

			if ( !BitBlt ( winTGDIgrabbedFrameDC [next], grabberThreadEnv->desktopDrawLeft, grabberThreadEnv->desktopDrawTop, grabberThreadEnv->desktopDrawWidth,
							grabberThreadEnv->desktopDrawHeight, winTGDIMainDC, grabberThreadEnv->desktopDrawLeft, grabberThreadEnv->desktopDrawTop, SRCCOPY ) ) {
				CVerb ( "ERROR ----> Thread_Grab:  Failed to grab the screen capture." );
				continue;
			}

			classCaptureBitmapIndexNext = next;

			SetEvent ( winTGDIgrabbedEvent );
#else
			//CVerb ( "Thread_Grab: signaled." );
			ResetEvent ( winTGDIgrabbedEvent );

			unsigned long incValue = InterlockedIncrement ( &classCaptureAccessed );

			clientAccessedCache++;
			if ( incValue == clientAccessedCache ) {
				SetEvent ( winTGDIgrabbedEvent );
				continue;
			}

			clientAccessedCache = incValue;

			EnterCriticalSection ( &winTGDIgrabbedFrameCS );

			if ( BitBlt ( winTGDIgrabbedFrameDC, grabberThreadEnv->desktopDrawLeft, grabberThreadEnv->desktopDrawTop, grabberThreadEnv->desktopDrawWidth,
							grabberThreadEnv->desktopDrawHeight, winTGDIMainDC, grabberThreadEnv->desktopDrawLeft, grabberThreadEnv->desktopDrawTop, SRCCOPY ) )
				SetEvent ( winTGDIgrabbedEvent );

			LeaveCriticalSection ( &winTGDIgrabbedFrameCS );
#endif
		}
		else if ( res != WAIT_TIMEOUT ) {
			break;
		}
	}

	ClassDisposeBuffers ( 0 );

Finish:
	grabberThreadEnv->desktopDrawRequested = false;

	CLog ( "Thread_Grab: CaptureWinTGDI class thread terminated." );

	return 0;
}



} /* namespace environs */

#endif

