/**
 * DirectShow Camera Capture
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

#ifdef _WIN32

#ifndef ENVIRONS_NATIVE_MODULE
#	define ENVIRONS_NATIVE_MODULE
#endif

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
#	define DEBUGVERB
#	define DEBUGVERBVerb
#endif

#include "Interop/Export.h"
#include "Environs.Obj.h"
#include "Environs.Release.h"
#include "Capture/Cam.Win.h"
#include "Environs.Build.Lnk.h"
using namespace environs;

#include <vector>
using namespace std;

#ifdef NDEBUG
#	define	DSLIBEXT	"e.lib"
#else
#	define	DSLIBEXT	"d.lib"
#endif

#ifdef _M_X64
#	define	DSLIBARCH	"x64"
#else
#	define	DSLIBARCH	"x86"
#endif


#pragma comment (lib, "lib/" ENVIRONS_TSDIR "/" DSLIBARCH "/strmbas" DSLIBEXT)

//#pragma comment ( lib, "comsupp.lib" )
#pragma comment ( lib, "winmm.lib" )

#ifndef MEDIASUBTYPE_I420
#	include "uuids.h"

//30323449 - 0000 - 0010 - 8000 - 00AA00389B71
DEFINE_GUID ( MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 );
#endif

// The TAG for prepending in log messages
#define	CLASS_NAME 	"CaptureCamWin"


static const char		*		CaptureCamWin_extensionNames[]	= { "Camera Capture Windows", "End" };

#ifndef ENVIRONS_CORE_LIB

static const InterfaceType_t	CaptureCamWin_interfaceTypes[]	= { InterfaceType::Capture, InterfaceType::Unknown };


/**
* GetINames
*
*	@param	size	on success, this argument is filled with the count of names available in the returned array.
*
*	@return returns an array of user readable friendly names in ASCII encoding.
*
*/
BUILD_INT_GETINAMES ( CaptureCamWin_extensionNames );


/**
* GetITypes
*
*	@param	size	on success, this argument is filled with the count of types available in the returned array.
*
*	@return returns an array with values of type InterfaceType::InterfaceType that describes the type of the plugin names returned by getINames();
*
*/
BUILD_INT_GETITYPES ( CaptureCamWin_interfaceTypes );


/**
* SetEnvironsMethods
*
*	Injects environs runtime methods.
*
*/
BUILD_INT_SETENVIRONSOBJECT ();


#if !defined(ENVIRONS_MISSING_DIRECTSHOW_SDK)

/**
* CreateInstance
*
*	@param	index		the index value of one of the plugin types returned in the array through getITypes().
*	@param	deviceID	the deviceID that the created interface object should use.
*
*	@return An object that supports the requested interface. 0 in case of error.
*
*/
BUILD_INT_CREATEOBJ ( CaptureCamWin );

#endif
#endif


#if !defined(ENVIRONS_MISSING_DIRECTSHOW_SDK)

namespace environs
{
	PortalBufferType_t	CaptureCamWin_outputTypeSupport[] = { PortalBufferType::YUV420 };

	static HRESULT		QueryInterfaces ( IGraphBuilder *pGraph, IMediaControl **ppControl = NULL, IMediaEvent **ppEvent = NULL, IVideoWindow **ppVidWin = NULL );

	static IPin*		GetFirstPin ( IBaseFilter *pFilter, PIN_DIRECTION PinDir );

	static HRESULT		FilterToPin ( const GUID& clsidOfTheFilter, LPCWSTR wszName, IGraphBuilder *pGraph, IPin *outputPin );

	static HRESULT		ConnectToNullRenderer ( IPin *pPinToConnectToNullRenderer, IGraphBuilder *pGraph );


	CaptureCamWin::CaptureCamWin ()
	{
		CLogID ( "Construct" );

		name				= "Windows DShow Camera Capture";
		captureType			= CaptureType::Camera;
		bufferType			= CaptureBufferType::PixelBuffer;

		countOfCameras		= 0;
		data				= 0;
		dataSize			= 0;

		outputTypes			= CaptureCamWin_outputTypeSupport;
		outputTypesLength	= sizeof ( CaptureCamWin_outputTypeSupport ) / sizeof ( CaptureCamWin_outputTypeSupport [0] );

		*camIdentifier			= 0;
		initialized				= false;

		// references to parent
		graphBuilder			= 0;
		captureGraphBuilder		= 0;

		captureDevice			= 0;
		mediaControl			= 0;
		grabberInterface		= 0;
		grabberFilter			= 0;
		camQualInterface		= 0;
		camControlInterface		= 0;

		coInitialized			= 0;
	}


	int CaptureCamWin::PreInit ()
	{
		CVerbID ( "PreInit" );

		if ( initialized )
			return true;

		HRESULT hr = CoInitializeEx ( NULL, 0x2 );
		//HRESULT hr = CoInitializeEx ( NULL, COINIT_MULTITHREADED );
		if ( FAILED ( hr ) )
		{
			if ( hr != RPC_E_CHANGED_MODE )
			{
				CErrID ( "PreInit: FAILED to initialize COM library!" );
				return false;
			}
			else {
				CVerbID ( "PreInit: Tried to reinitialize COM with different threading model!" );
			}
		}
		else {
			if ( hr == S_FALSE ) {
				CVerbID ( "PreInit: COM was already initialized for this thread!" );
			}
		}
		coInitialized++;


		/*******************************************************/
		CVerbID ( "PreInit: Calling InitCapturegraphBuilder..." );

		hr = InitCaptureGraphBuilder ( &graphBuilder, &captureGraphBuilder );
		if ( FAILED ( hr ) )
		{
			CErrID ( "PreInit: InitCapturegraphBuilder" );
			return false;
		}

		hr = DetectCameras ( );
		if ( FAILED ( hr ) ) {
			CErrID ( "PreInit: DetectCameras" );
			return false;
		}

		hr = SelectCamera ( );
		if ( FAILED ( hr ) ) {
			CErrID ( "PreInit: SelectCamera" );
			return false;
		}

		hr = QueryInterfaces ( graphBuilder, &mediaControl );
		if ( FAILED ( hr ) ) {
			CErrID ( "PreInit: QueryInterfaces" );
			return false;
		}

		// ******************************************************
		CVerbID ( "PreInit: Calling SelectMediaTypeOfCam..." );


		hr = SelectMediaTypeOfCam ( 30, MEDIASUBTYPE_I420 );
		if ( SUCCEEDED ( hr ) ) {
			CaptureCamWin_outputTypeSupport [0] = PortalBufferType::YUV420;
		}
		else {
			CErrID ( "PreInit: SelectMediaTypeOfCam" );

			hr = SelectMediaTypeOfCam ( 30, MEDIASUBTYPE_YUY2 );
			if ( SUCCEEDED ( hr ) ) {
				CaptureCamWin_outputTypeSupport [0] = PortalBufferType::YUY2;
			}
			else {
				hr = SelectMediaTypeOfCam ( 30, MEDIASUBTYPE_RGB24 );
				if ( SUCCEEDED ( hr ) ) {
					CaptureCamWin_outputTypeSupport [0] = PortalBufferType::BGR;
				}
				else {
					CErrID ( "PreInit: SelectMediaTypeOfCam" );
					CaptureCamWin_outputTypeSupport [0] = PortalBufferType::Unknown;
					return false;
					//hr = SelectMediaTypeOfCam ( 30, MEDIASUBTYPE_RGB24, true );
				}
			}
		}

		return true;
	}


	int CaptureCamWin::Init ()
	{
		CVerbID ( "Init" );

		if ( initialized )
			return true;

		// ******************************************************
		CVerbID ( "Init: Calling GetFirstPin with captureDevice..." );

		IPin * pPin = GetFirstPin ( captureDevice, PINDIR_OUTPUT );
		if ( !pPin )
		{
			CErrID ( "Init: Get Output Pin of Capture Device" );
			return false;
		}

		// ******************************************************
		CVerbID ( "Init: Calling FilterToPin with Capture and Grabber..." );

		HRESULT hr = FilterToPin ( CLSID_CEnvironsGrabber, L"Grabber", graphBuilder, pPin );
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: FilterToPin with CEnvironsGrabber" );
			OBJ_RELEASE ( pPin );
			return false;
		}

		OBJ_RELEASE ( pPin );

		// ******************************************************
		CVerbID ( "Init: Trying to find Grabber by Name in Graph..." );

		hr = graphBuilder->FindFilterByName ( L"Grabber", &grabberFilter );
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: FindFilterByName with CEnvironsGrabber" );
			return false;
		}

		// ******************************************************
		CVerbID ( "Init: Calling GetFirstPin with Grabber..." );

		pPin = GetFirstPin ( grabberFilter, PINDIR_OUTPUT );
		if ( !pPin )
		{
			CErrID ( "Init: Get Output Pin of Grabber Device" );
			return false;
		}

		// ******************************************************
		CVerbID ( "Init: Try to Grabber-Interface..." );

		hr = grabberFilter->QueryInterface ( IID_IEnvironsGrabber, (LPVOID *) &grabberInterface );
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: IID_IEnvironsGrabber" );
			OBJ_RELEASE ( pPin );
			return false;
		}

		// ******************************************************
		CVerbID ( "Init: Calling ConnectToNullRenderer..." );

		hr = ConnectToNullRenderer ( pPin, graphBuilder );
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: ConnectToNullRenderer" );
			OBJ_RELEASE ( pPin );
			return false;
		}

		OBJ_RELEASE ( pPin );

		// ******************************************************
		CVerbID ( "Init: Activating Framebuffering in Filter..." );

		hr = grabberInterface->ToggleGrabbing ( true );
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: Activation of FrameBuffering for Grabbing" );
			return false;
		}


		grabberInterface->SetCaptureInterface ( this );
		// Set Frame Avail Event
		//grabberInterface->SetFrameEvent ( frameEvent );


		// ******************************************************
		CVerbID ( "Init: Query Interface for adjusting quality props of Capture Device..." );

		hr = captureDevice->QueryInterface ( IID_IAMVideoProcAmp, (void**) &camQualInterface );
		if ( FAILED ( hr ) )
		{
			camQualInterface = NULL;
			CErrID ( "Init: Could not get Interface for CaptureDevice Quality Props!" );
		}


		// ******************************************************
		CVerbID ( "Init: Running Graph..." );

		hr = mediaControl->Run ();
		if ( FAILED ( hr ) )
		{
			CErrID ( "Init: Running Graph" );
			return false;
		}

		grabberInterface->ToggleGrabbing ( false );

		initialized = true;
		enabled = true;

		return true;
	}


	CaptureCamWin::~CaptureCamWin ()
	{
		CLogID ( "Destruct" );

		Release ( );
	}


	HRESULT CaptureCamWin::InitCaptureGraphBuilder ( IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppCapBuild )
	{
		CVerbID ( "InitCaptureGraphBuilder" );

		if ( !ppGraph || !ppCapBuild )
			return E_POINTER;

		IGraphBuilder			* pGraph	= NULL;
		ICaptureGraphBuilder2	* pCapBuild = NULL;

		// Create the Capture Graph Builder.
		CVerbID ( "InitCaptureGraphBuilder: Creating CapureGraphBuilder ..." );

		HRESULT hr = CoCreateInstance (
			CLSID_CaptureGraphBuilder2,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ICaptureGraphBuilder2,
			(void**) &pCapBuild );

		if ( SUCCEEDED ( hr ) )
		{
			CVerbID ( "InitCaptureGraphBuilder: Creating FilterGraph ..." );

			hr = CoCreateInstance ( CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**) &pGraph );
			if ( SUCCEEDED ( hr ) )
			{
				CVerbID ( "InitCaptureGraphBuilder: Initializing Graph with CaptureGraph..." );

				hr = pCapBuild->SetFiltergraph ( pGraph );
				if ( SUCCEEDED ( hr ) )
				{
					*ppCapBuild = pCapBuild;
					*ppGraph = pGraph;

					return S_OK;
				}
			}
		}

		CErr ( "InitCaptureGraphBuilder" );

		OBJ_RELEASE ( pCapBuild );
		OBJ_RELEASE ( pGraph );

		return hr;
	}


	HRESULT QueryInterfaces (
		IGraphBuilder	*	pGraph,
		IMediaControl	**	ppControl,
		IMediaEvent		**	ppEvent,
		IVideoWindow	**	ppVidWin )
	{
		CVerb ( "QueryInterfaces" );

		IVideoWindow	*	pVidWin		= NULL;
		IMediaControl	*	pControl	= NULL;
		IMediaEvent		*	pEvent		= NULL;
		HRESULT				hr			= S_OK;

		if ( ppControl )
		{
			CVerb ( "QueryInterfaces: Getting mediaControl Interface... " );

			hr = pGraph->QueryInterface ( IID_IMediaControl, (void**) &pControl );
			if ( FAILED ( hr ) )
			{
				CErr ( "QueryInterfaces: IID_IMediaControl" );
				return hr;
			}
		}

		if ( ppVidWin )
		{
			CVerb ( "QueryInterfaces: Getting VideoWindow Interface... " );

			hr = pGraph->QueryInterface ( IID_IVideoWindow, (void**) &pVidWin );
			if ( FAILED ( hr ) )
			{
				CErr ( "QueryInterfaces: IID_IVideoWindow" );
				OBJ_RELEASE ( pControl );
				return hr;
			}
		}

		if ( ppEvent )
		{
			CVerb ( "QueryInterfaces: Getting MediaEvent Interface... " );

			hr = pGraph->QueryInterface ( IID_IMediaEvent, (void**) &pEvent );
			if ( FAILED ( hr ) )
			{
				CErr ( "QueryInterfaces: IID_IMediaEvent" );
				OBJ_RELEASE ( pControl );
				OBJ_RELEASE ( pVidWin );
				return hr;
			}
		}

		if ( ppEvent )
			*ppEvent = pEvent;
		if ( ppControl )
			*ppControl = pControl;
		if ( ppVidWin )
			*ppVidWin = pVidWin;

		CVerb ( "QueryInterfaces: done" );
		return S_OK;
	}


	HRESULT CaptureCamWin::DetectCameras ( )
	{
		CVerbID ( "DetectCameras" );

		IBaseFilter			*	pCap		= NULL;
		ICreateDevEnum		*	pDevEnum	= NULL;
		IEnumMoniker		*	pEnum		= NULL;
		IMoniker			*	pMoniker	= NULL;
		IPropertyBag		*	pPropBag;

		HRESULT					returnCode	= S_OK;
		HRESULT					hr;
		countOfCameras						= 0;

		// Create the System Device Enumerator.
		CVerbID ( "DetectCameras: Creating DeviceEnumerator..." );

		hr = CoCreateInstance ( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum) );

		if ( FAILED ( hr ) ) {
			CErrID ( "DetectCameras: CoCreateInstance " );
			return hr;
		}

		// Create an enumerator for the video capture category.
		CVerbID ( "DetectCameras: Creating ClassEnumerator..." );

		hr = pDevEnum->CreateClassEnumerator ( CLSID_VideoInputDeviceCategory, &pEnum, 0 );
		if ( hr != S_OK ) {
			CErrID ( "DetectCameras: CreateClassEnumerator" );

			returnCode = E_UNEXPECTED;
			goto End;
		}


		CVerbID ( "DetectCameras: Iterating through available cameras..." );

		while ( pEnum->Next ( 1, &pMoniker, NULL ) == S_OK )
		{
			CVerbID ( "DetectCameras: Camera" );
			pPropBag = NULL;

			hr = pMoniker->BindToStorage ( 0, 0, IID_IPropertyBag, (void**) (&pPropBag) );
			if ( FAILED ( hr ) ) {
				CErrID ( "DetectCameras: BindToStorage" );

				pMoniker->Release ( );
				continue;
			}

			// Retrieve description or friendly name of camera.
			VARIANT		varName;
			wchar_t		buffer [IDENTIFIER_LENGTH];
			*buffer = 0;

			VariantInit ( &varName );
			hr	= pPropBag->Read ( L"FriendlyName", &varName, 0 );
			if ( SUCCEEDED ( hr ) )
			{
				wcscpy_s ( buffer, IDENTIFIER_LENGTH, varName.bstrVal );
				wcscat_s ( buffer, IDENTIFIER_LENGTH, L" - " );

				VariantClear ( &varName );
				VariantInit ( &varName );
			}

			size_t length = wcslen ( buffer );
			if ( length > 0 )
			{
				countOfCameras++;
				CLogArgID ( "DetectCameras: [%ws]", buffer );
			}
			VariantClear ( &varName );

			pPropBag->Release ( );
			pMoniker->Release ( );
		}

	End:
		OBJ_RELEASE ( pEnum );
		OBJ_RELEASE ( pDevEnum );

		CVerbID ( "DetectCameras: done." );
		return returnCode;

	}


	HRESULT CaptureCamWin::SelectCamera ( )
	{
		CVerbID ( "SelectCamera" );

		IBaseFilter			*	pCap		= NULL;
		ICreateDevEnum		*	pDevEnum	= NULL;
		IEnumMoniker		*	pEnum		= NULL;
		IMoniker			*	pMoniker	= NULL;
		IPropertyBag		*	pPropBag;

		HRESULT					returnCode	= S_OK;
		HRESULT					hr;

		// Create the System Device Enumerator.
		CVerbID ( "SelectCamera: Creating DeviceEnumerator..." );

		hr = CoCreateInstance ( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum) );

		if ( FAILED ( hr ) ) {
			CErrID ( "SelectCamera: CoCreateInstance " );
			return hr;
		}

		// Create an enumerator for the video capture category.
		CVerbID ( "SelectCamera: Creating ClassEnumerator..." );

		hr = pDevEnum->CreateClassEnumerator ( CLSID_VideoInputDeviceCategory, &pEnum, 0 );
		if ( hr != S_OK ) {
			CErrID ( "SelectCamera: CreateClassEnumerator" );

			returnCode = E_UNEXPECTED;
			goto End;
		}

		CVerbID ( "SelectCamera: Iterating through available cameras..." );

		int camType = (portalID & PORTAL_TYPE_MASK) >> 12;
		if ( camType >= countOfCameras )
			camType = countOfCameras;

		int camCount = 0;

		while ( pEnum->Next ( 1, &pMoniker, NULL ) == S_OK )
		{
			camCount++;

			if ( camType != camCount )
				continue;

			CVerbID ( "SelectCamera: Camera" );
			pPropBag = NULL;

			hr = pMoniker->BindToStorage ( 0, 0, IID_IPropertyBag, (void**) (&pPropBag) );
			if ( FAILED ( hr ) ) {
				CErrID ( "SelectCamera: BindToStorage" );

				pMoniker->Release ();
				continue;
			}

			// Retrieve description or friendly name of camera.
			VARIANT		varName;
			wchar_t		buffer [IDENTIFIER_LENGTH];
			*buffer = 0;

			VariantInit ( &varName );
			hr	= pPropBag->Read ( L"FriendlyName", &varName, 0 );
			if ( SUCCEEDED ( hr ) )
			{
				wcscpy_s ( buffer, IDENTIFIER_LENGTH, varName.bstrVal );
				wcscat_s ( buffer, IDENTIFIER_LENGTH, L" - " );

				VariantClear ( &varName );
				VariantInit ( &varName );
			}

			hr = pPropBag->Read ( L"DevicePath", &varName, 0 );
			if ( SUCCEEDED ( hr ) )
				wcscat_s ( buffer, IDENTIFIER_LENGTH, varName.bstrVal );

			size_t length = wcslen ( buffer );
			if ( length > 0 )
			{
				hr = pMoniker->BindToObject ( 0, 0, IID_IBaseFilter, (void**) &pCap );
				if ( SUCCEEDED ( hr ) )
				{
					CVerbID ( "SelectCamera: Adding capture filter to graph..." );

					hr = graphBuilder->AddFilter ( pCap, L"Capture Filter" );
					if ( SUCCEEDED ( hr ) )
					{
						if ( 0 == wcscpy_s ( camIdentifier, IDENTIFIER_LENGTH, buffer ) )
						{
							captureDevice = pCap;
							break;
						}
					}
				}
			}
			VariantClear ( &varName );

			pPropBag->Release ();
			pMoniker->Release ();
		}

	End:
		OBJ_RELEASE ( pEnum );
		OBJ_RELEASE ( pDevEnum );

		CVerbID ( "SelectCamera: done." );
		return returnCode;

	}


	HRESULT CaptureCamWin::SelectMediaTypeOfCam ( int fps, GUID desiredMediaSubType, bool useFirst )
	{
		CVerb ( "SelectMediaTypeOfCam" );

		VIDEO_STREAM_CONFIG_CAPS	scc;

		IAMStreamConfig			*	pConfig			= NULL;
		HRESULT						success			= -1;
		AM_MEDIA_TYPE			*	mediaType		= NULL;
		VIDEOINFOHEADER			*	formatDetails	= NULL;

		CVerb ( "SelectMediaTypeOfCam: Looking for camera stream configuration interface." );

		HRESULT hr = captureGraphBuilder->FindInterface (
			&PIN_CATEGORY_CAPTURE,	// Preview pin.
			0,						// Any media type.
			captureDevice,				// Pointer to the capture filter.
			IID_IAMStreamConfig,
			(void**) &pConfig );

		if ( FAILED ( hr ) ) {
			CErr ( "SelectMediaTypeOfCam: FindInterface IID_IAMStreamConfig" );
			return hr;
		}

		int Count = 0, Size = 0;


		CVerb ( "SelectMediaTypeOfCam: Determining number of Capabilities..." );

		hr = pConfig->GetNumberOfCapabilities ( &Count, &Size );
		if ( FAILED ( hr ) )
		{
			CErr ( "SelectMediaTypeOfCam: GetNumberOfCapabilities" );
			OBJ_RELEASE ( pConfig );
			return hr;
		}

		int nBest = -1;
		int targetWidth = this->width;
		int targetHeight = this->height;
		int width = 0;
		int height = 0;

		if ( Size == sizeof ( VIDEO_STREAM_CONFIG_CAPS ) )
		{
			for ( int nFormat = 0; nFormat < Count; nFormat++ )
			{
				hr = pConfig->GetStreamCaps ( nFormat, &mediaType, (BYTE*) &scc );
				if ( SUCCEEDED ( hr ) && mediaType )
				{
					formatDetails = (VIDEOINFOHEADER *) mediaType->pbFormat;

					if ( (useFirst ||
						(mediaType->majortype == MEDIATYPE_Video) &&
						(mediaType->subtype == desiredMediaSubType) &&
						(mediaType->formattype == FORMAT_VideoInfo)) &&
						(mediaType->cbFormat >= sizeof ( VIDEOINFOHEADER )) &&
						(mediaType->pbFormat != NULL) )
					{
						int curWidth = formatDetails->bmiHeader.biWidth;
						int curHeight = formatDetails->bmiHeader.biHeight;

						if ( curWidth > width || curHeight > height ) {
							if ( curWidth < targetWidth && curHeight < targetHeight ) {
								nBest = nFormat;
								width = curWidth;
								height = curHeight;
							}
						}
					}
					DeleteMediaType ( mediaType );
				}
			}
		}


		if ( nBest >= 0 ) {
			hr = pConfig->GetStreamCaps ( nBest, &mediaType, (BYTE*) &scc );
			if ( SUCCEEDED ( hr ) && mediaType )
			{
				formatDetails = (VIDEOINFOHEADER *) mediaType->pbFormat;

				if ( (useFirst ||
					(mediaType->majortype == MEDIATYPE_Video) &&
					(mediaType->subtype == desiredMediaSubType) &&
					(mediaType->formattype == FORMAT_VideoInfo)) &&
					(mediaType->cbFormat >= sizeof ( VIDEOINFOHEADER )) &&
					(mediaType->pbFormat != NULL) )
				{
					CVerb ( "SelectMediaTypeOfCam: Found Desired Format, lets try." );

					formatDetails->AvgTimePerFrame = (REFERENCE_TIME) ((REFERENCE_TIME) 10000000 / (REFERENCE_TIME) fps);

					hr = pConfig->SetFormat ( mediaType );
					if ( SUCCEEDED ( hr ) )
					{
						mediaSubType = mediaType->subtype;

						this->width = formatDetails->bmiHeader.biWidth;
						this->height = formatDetails->bmiHeader.biHeight;
						success = S_OK;
						CVerb ( "ok" );
					}
				}
				DeleteMediaType ( mediaType );
			}
		}
		else {
			CErr ("SelectMediaTypeOfCam: No matching media format found!");
		}

		OBJ_RELEASE ( pConfig );

		return success;
	}


	IPin * GetFirstPin ( IBaseFilter * pFilter, PIN_DIRECTION PinDir )
	{
		CVerb ( "GetFirstPin" );

		HRESULT			hr;
		BOOL			bFound	= FALSE;
		IEnumPins	*	pEnum	= NULL;
		IPin		*	pPin	= NULL;

		if ( pFilter == NULL )
			return NULL;

		CVerb ( "GetFirstPin: Try to enumerate Pins... " );

		hr = pFilter->EnumPins ( &pEnum );
		if ( FAILED ( hr ) )
		{
			CErr ( "GetFirstPin: EnumPins" );
			return NULL;
		}

		CVerb ( "GetFirstPin: Going through Pins... " );

		while ( pEnum->Next ( 1, &pPin, 0 ) == S_OK )
		{
			PIN_DIRECTION PinDirThis;
			pPin->QueryDirection ( &PinDirThis );
			if ( PinDir == PinDirThis )
			{
				bFound = true;
				break;
			}
			OBJ_RELEASE ( pPin );
		}
		OBJ_RELEASE ( pEnum );

		return (bFound ? pPin : NULL);
	}


	HRESULT FilterToGraph
		(
		IGraphBuilder	*	pGraph,
		const GUID&			clsidOfTheFilter,
		LPCWSTR				wszName,
		IBaseFilter		**	ppFilterToBeAddedtoTheGraph
		)
	{
		CVerb ( "FilterToGraph" );

		if ( !pGraph || !ppFilterToBeAddedtoTheGraph )
			return E_POINTER;

		HRESULT hr = 0;
		*ppFilterToBeAddedtoTheGraph = NULL;
		IBaseFilter *pF = 0;
		CEnvironsGrabber * envGrabber = 0;

		if ( clsidOfTheFilter == CLSID_CEnvironsGrabber )
		{
			envGrabber = new CEnvironsGrabber ( NAME ( "CEnvironsGrabber" ), 0, &hr );
			if ( envGrabber ) {
				hr = envGrabber->QueryInterface ( IID_IBaseFilter, reinterpret_cast<void**>(&pF) );
			}
		}
		else
		{
			hr = CoCreateInstance ( clsidOfTheFilter, 0, CLSCTX_INPROC_SERVER,
				IID_IBaseFilter, reinterpret_cast<void**>(&pF) );
		}

		if ( SUCCEEDED ( hr ) )
		{
			hr = pGraph->AddFilter ( pF, wszName );
			if ( SUCCEEDED ( hr ) )
				*ppFilterToBeAddedtoTheGraph = pF;
			else
				OBJ_RELEASE ( pF );
		}

		return hr;
	}


	HRESULT FilterToPin
		(
		const GUID&			clsidOfTheFilter,
		LPCWSTR				wszName,
		IGraphBuilder	*	pGraph,
		IPin			*	outputPin
		)
	{
		CVerb ( "FilterToPin" );

		HRESULT			hr;
		IBaseFilter	*	baseFilter	= NULL;
		IPin		*	inputPin	= 0;

		hr = FilterToGraph ( pGraph, clsidOfTheFilter, wszName, &baseFilter );
		if ( FAILED ( hr ) )
			goto End;

		inputPin = GetFirstPin ( baseFilter, PINDIR_INPUT );
		if ( inputPin == 0 )
		{
			hr = E_UNEXPECTED;
			goto End;
		}

		hr = pGraph->Connect ( outputPin, inputPin );

	End:
		OBJ_RELEASE ( baseFilter );
		OBJ_RELEASE ( inputPin );

		return hr;
	}


	HRESULT ConnectToNullRenderer ( IPin *pPinToConnectToNullRenderer, IGraphBuilder *pGraph )
	{
		CVerb ( "ConnectToNullRenderer" );

		HRESULT hr = FilterToPin ( CLSID_NullRenderer, L"NullRenderer", pGraph, pPinToConnectToNullRenderer );
		if ( SUCCEEDED ( hr ) ) {
			CVerb ( "ConnectToNullRenderer" );
		}

		return hr;
	}


	void CaptureCamWin::Release ()
	{
		CVerbID ( "Release" );

		initialized = false;


		if ( grabberInterface ) {
			// Stop grabbing
			grabberInterface->ToggleGrabbing ( false );
		}

		Stop ();

		initialized = false;
		OBJ_RELEASE ( camQualInterface );
		OBJ_RELEASE ( camControlInterface );
		OBJ_RELEASE ( grabberInterface );
		OBJ_RELEASE ( grabberFilter );
		OBJ_RELEASE ( mediaControl );
		OBJ_RELEASE ( captureDevice );

		// references to parent properties
		graphBuilder		= NULL;
		captureGraphBuilder	= NULL;

		OBJ_RELEASE ( captureGraphBuilder );
		OBJ_RELEASE ( graphBuilder );

		if ( coInitialized > 0 ) {
			coInitialized--;
			CoUninitialize ();
		}


		dataHandle	= 0;
		data		= 0;
		dataSize	= 0;
		squareLength= 0;

		buffersInitialized	= false;
	}


	int CaptureCamWin::Start ()
	{
		CVerbID ( "Start" );

		CVerbID ( "Start: Starting Graph..." );

		if ( !mediaControl )
		{
			CErrID ( "Start: MediaControl-Interface missing" );
			return false;
		}

		HRESULT hr = mediaControl->Run ();
		if ( FAILED ( hr ) )
		{
			CErrID ( "Start" );
			return false;
		}

		if ( grabberInterface )
			grabberInterface->ToggleGrabbing ( true );

		return 1;
	}


	int	CaptureCamWin::Stop ()
	{
		CVerbID ( "Stop" );

		CVerbID ( "Stop: Stopping Graph..." );

		if ( grabberInterface )
			grabberInterface->ToggleGrabbing ( false );

		if ( mediaControl )
		{
			HRESULT hr = mediaControl->Stop ();
			if ( FAILED ( hr ) )
			{
				CErrID ( "Stop:" );
				return 0;
			}
		}

		return 1;
	}


#ifdef CLASS_NAME
#undef CLASS_NAME
#endif
#define	CLASS_NAME 	"CEnvironsGrabber"



	CEnvironsGrabber::CEnvironsGrabber ( TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr )
		: CTransInPlaceFilter ( tszName, punk, CLSID_CEnvironsGrabber, phr )
	{
		CVerb ( "Construct" );

		enabled					= false;
		capture					= NULL;
	}


	CEnvironsGrabber::~CEnvironsGrabber ()
	{
		CVerb ( "Destruct" );
	}


	HRESULT CEnvironsGrabber::Transform ( IMediaSample * pSample )
	{
		if ( !enabled )
			return S_OK;

		if ( !capture )
			return S_FALSE;

		BYTE			*	pData;

		HRESULT hr = pSample->GetPointer ( &pData );
		if ( FAILED ( hr ) ) {
			return hr;
		}

		AM_MEDIA_TYPE	&	pType	= m_pInput->CurrentMediaType ();
		VIDEOINFOHEADER *	pvi		= (VIDEOINFOHEADER *) pType.pbFormat;

		int width		= pvi->bmiHeader.biWidth;
		int height		= abs ( pvi->bmiHeader.biHeight );
		int stride		= (width * ((pvi->bmiHeader.biBitCount) >> 3) + 3) & ~3;

		unsigned int size = pvi->bmiHeader.biSizeImage;
		if ( size <= 0 ) {
			size = height * stride;
			if ( pType.subtype == MEDIASUBTYPE_I420 ) {
				size += size >> 1;
			}
			else size *= 3;
		}

		if ( size && enabled )
		{
			capture->dataStride = stride;

			if ( capture->PerformBase ( (const char *) pData, size ) ) {
				return S_OK;
			}
		}

		return S_FALSE;
	}


	STDMETHODIMP CEnvironsGrabber::SetCaptureInterface ( CaptureCamWin * capture )
	{
		CVerb ( "SetCaptureInterface" );
		this->capture = capture;
		return S_OK;
	}


	STDMETHODIMP CEnvironsGrabber::ToggleGrabbing ( bool activate )
	{
		CVerb ( "ToggleGrabbing" );

		enabled = activate;

		return S_OK;
	}


	STDMETHODIMP CEnvironsGrabber::NonDelegatingQueryInterface ( REFIID riid, void **ppv )
	{
		CVerb ( "NonDelegatingQueryInterface" );

		CheckPointer ( ppv, E_POINTER );

		if ( riid == IID_IEnvironsGrabber ) {
			return GetInterface ( (IEnvironsGrabber *) this, ppv );
		}
		else if ( riid == IID_ISpecifyPropertyPages ) {
			return GetInterface ( (ISpecifyPropertyPages *) this, ppv );
			/*
			} else if ( riid == IID_IAMCameraControl ) {
			return GetInterface ( (IAMCameraControl *) this, ppv );
			*/
		}
		else {
			return CTransInPlaceFilter::NonDelegatingQueryInterface ( riid, ppv );
		}
	}


	CUnknown * WINAPI CEnvironsGrabber::CreateInstance ( LPUNKNOWN punk, HRESULT *phr )
	{
		CVerb ( "CreateInstance" );

		CEnvironsGrabber *pNewObject = new CEnvironsGrabber ( L"CEnvironsGrabber", punk, phr );
		if ( !pNewObject )
			*phr = E_OUTOFMEMORY;

		return pNewObject;
	}


	HRESULT CEnvironsGrabber::CheckInputType ( const CMediaType *mtIn )
	{
		CVerb ( "CheckInputType" );

		if ( CanPerformTransform ( mtIn ) )
			return S_OK;
		else
			return VFW_E_TYPE_NOT_ACCEPTED;
	}


	BOOL CEnvironsGrabber::CanPerformTransform ( const CMediaType *pMediaType ) const
	{
		if ( IsEqualGUID ( *pMediaType->Type (), MEDIATYPE_Video ) )
		{
			VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format ();
			//if ( pvi->bmiHeader.biBitCount == 16 )
			//	return FALSE;

			// We accept anything that the CamWin has accepted
			return TRUE;
		}
		return FALSE;
	}


	STDMETHODIMP CEnvironsGrabber::GetPages ( CAUUID *pPages )
	{
		pPages->cElems = 1;
		pPages->pElems = (GUID *) CoTaskMemAlloc ( sizeof ( GUID ) );
		if ( pPages->pElems == NULL )
		{
			return E_OUTOFMEMORY;
		}
		*(pPages->pElems) = CLSID_CEnvironsGrabberPropertyPage;
		return NOERROR;
	}

} /* namespace environs */

#endif

#endif


