/**
* Windows D3D screen capture
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

#include "Environs.Obj.h"
#include "Capture.Win.D3D.h"
#include "DynLib/dyn.Direct3D.11.h"
#include "Interfaces/IPortal.Renderer.h"

// The TAG for prepending in log messages
#define CLASS_NAME	"CaptureWinD3D"


#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)

namespace environs 
{
	extern void DisposeThread ( LONGSYNC * threadState, pthread_t &threadID, pthread_t_id handleID, const char * threadName, pthread_cond_t &threadEvent );

	PortalBufferType_t	CaptureWinD3D_outputTypeSupport [] = { PortalBufferType::Texture3D, PortalBufferType::ARGB };

	LONGSYNC		CaptureWinD3D::referenceCount	= 0;
	unsigned int	CaptureWinD3D::framesAvail		= 0;
	HANDLE			CaptureWinD3D::capturedEvent	= 0;
	pthread_t		CaptureWinD3D::captureThreadID	= 0;
	HANDLE			CaptureWinD3D::captureThreadEvent = 0;
	void *			CaptureWinD3D::d3dDevice		= 0;
	LONGSYNC 		CaptureWinD3D::clientAccessed	= 1;

	CRITICAL_SECTION CaptureWinD3D::capturedFrameCS;


	ID3D11Device			* d3dMainDevice			= 0;
	ID3D11DeviceContext		* d3dContext			= 0;
	IDXGIOutputDuplication	* d3dOutputDuplication	= 0;
	ID3D11Texture2D			* d3dAcquiredDesktopImage = 0;
	IDXGISurface			* d3dRenderSurface		= 0;

	DXGI_OUTDUPL_FRAME_INFO		FrameInfo;
	//IDXGISwapChain			*	capWinD3DswapChain	= 0;


	/* Driver types supported */
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	/* Feature levels supported */
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
	
	D3D_FEATURE_LEVEL FeatureLevel;

	/// static lock start 
	class winD3DInitLockClass
	{
	public:
		winD3DInitLockClass ( )
		{
			if ( !didiInitStaticInitDelLock )
			{
				didiInitStaticInitDelLock = true;
				InitializeCriticalSection ( &staticInitDelLock );
			}
		}

		~winD3DInitLockClass ( )
		{
			DeleteCriticalSection ( &staticInitDelLock );
		}

		static CRITICAL_SECTION		staticInitDelLock;

	private:
		static bool					didiInitStaticInitDelLock;
	};

	CRITICAL_SECTION		winD3DInitLockClass::staticInitDelLock;
	bool					winD3DInitLockClass::didiInitStaticInitDelLock = false;

	static winD3DInitLockClass staticLock;
	/// static lock end


CaptureWinD3D::CaptureWinD3D ( )
{
	CLogID ( "Construct" );

	name			= "Windows D3D DXGI Grabber";
	captureType		= CaptureType::Screen;
	bufferType		= CaptureBufferType::Texture3D;

	didAddRef		= false;

	d3dTexture		= 0;

	outputTypes			= CaptureWinD3D_outputTypeSupport;
	outputTypesLength	= sizeof(CaptureWinD3D_outputTypeSupport) / sizeof(CaptureWinD3D_outputTypeSupport [0]);
}


int CaptureWinD3D::Init ()
{
	CVerbID ( "Init" );

	int ret = 1;

	didAddRef = true;

	EnterCriticalSection ( &winD3DInitLockClass::staticInitDelLock );

	if ( __sync_add_and_fetch ( &referenceCount, 1 ) == 1 ) {
		if ( CInit ( this, deviceID ) ) {
			enabled = true;
			ret  = 1;
		}
		else {
			CErrID ( "Init: Failed to initialize Interface resources for threading" );
			ret = -1;
		}
	}

	LeaveCriticalSection ( &winD3DInitLockClass::staticInitDelLock );

	if ( ret > 0 )
		initialized = true;

	return ret;
}


bool CaptureWinD3D::CInit ( void * arg, int deviceID )
{
	CLogID ( "CInit" );

	if ( !captureThreadEvent ) {
		captureThreadEvent = CreateEvent ( 0, TRUE, FALSE, 0 );
		if ( !captureThreadEvent ) {
			CErrID ( "CInit: Failed to create thread event!" );
			return false;
		}
	}

	if ( !capturedEvent ) {
		capturedEvent = CreateEvent ( 0, TRUE, FALSE, 0 );
		if ( !capturedEvent ) {
			CErrID ( "CInit: Failed to create thread captured event!" );
			return false;
		}
	}

	// Create capture thread
	if ( !pthread_valid ( captureThreadID ) )
	{
		ResetEvent ( captureThreadEvent );

		pthread_create ( &captureThreadID, NULL, &CaptureWinD3D::Thread_Capture, arg );
		if ( !pthread_valid ( captureThreadID ) )
		{
			CErrID ( "CInit: Failed to create thread for capturing!" );
			return false;
		}

		if ( WaitForSingleObject ( captureThreadEvent, 1000 ) != WAIT_OBJECT_0 ) {
			CErrID ( "CInit: Failed to wait for thread becoming ready!" );
			return false;
		}
	}

	return Duplicate ( );
}

//ID3D11Texture2D	* capWinD3DBackBuffer	= 0;
//ID3D11RenderTargetView *capWinD3DRTV = 0;

bool CaptureWinD3D::InitD3D ( Instance * env )
{
	CVerb ( "InitD3D" );

	if ( d3dMainDevice && d3dContext ) {
		d3dDevice = d3dMainDevice;
		return true;
	}

	DXGI_SWAP_CHAIN_DESC swapDesc;
	Zero ( swapDesc );

	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = env->appWindowWidth;
	swapDesc.BufferDesc.Height = env->appWindowHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = GetDesktopWindow (); //env->appWindowHandle;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Windowed = TRUE;

	if ( !LockAcquire ( &native.kernelLock, "InitD3D" ) )
		return false;

	HRESULT status;
	bool	ret = false;

	for ( unsigned int i = 0; i < NumDriverTypes; i++ )
	{
		status = D3D11CreateDevice ( NULL, DriverTypes [i], NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &d3dMainDevice, &FeatureLevel, &d3dContext );

		/*status = D3D11CreateDeviceAndSwapChain ( NULL, DriverTypes [i], NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &swapDesc, &capWinD3DswapChain, &d3dMainDevice, &FeatureLevel, &d3dContext );*/

		if ( SUCCEEDED ( status ) && d3dContext  ) {
			ret = true;

			// Get a pointer to the back buffer
			/*status = capWinD3DswapChain->GetBuffer ( 0, __uuidof(ID3D11Texture2D),
				(LPVOID*)&capWinD3DBackBuffer );
			if ( FAILED ( status ) ) {
				CErr ( "InitD3D: Failed to get backbuffer." );
			}*/

			//// Create a render-target view
			//status = d3dMainDevice->CreateRenderTargetView ( capWinD3DBackBuffer, NULL,
			//	&capWinD3DRTV );
			//if ( FAILED ( status ) ) {
			//	CErr ( "InitD3D: Failed to create render target view." );
			//}

			//// Bind the view
			//d3dContext->OMSetRenderTargets ( 1, &capWinD3DRTV, NULL );
			//

			//// Setup the viewport
			//D3D11_VIEWPORT vp;
			//vp.Width = environs::appWindowWidth;
			//vp.Height = environs::appWindowHeight;
			//vp.MinDepth = 0.0f;
			//vp.MaxDepth = 1.0f;
			//vp.TopLeftX = 0;
			//vp.TopLeftY = 0;
			//d3dContext->RSSetViewports ( 1, &vp );

			//status = d3dMainDevice->CreateDeferredContext ( 0, &capWinD3DDeferedContext );
			//if ( FAILED ( status ) ) {
			//	CErr ( "InitD3D: Failed to create deferred context." );
			//}
			break;
		}

		CErrArg ( "InitD3D: D3D11CreateDevice failed. Status [%#X] for DriverType %d", status, DriverTypes [i] );
	}

	if ( ret ) {
		d3dDevice = d3dMainDevice; // "Export D3D device handle
		InitializeCriticalSection ( &capturedFrameCS );
	}
	else {
		d3dDevice = d3dMainDevice = 0;
		CErr ( "InitD3D: Failed to create a D3D device." );
	}

	LockReleaseV ( &native.kernelLock, "InitD3D" );

	return ret;
}


bool CaptureWinD3D::DisposeD3D ( )
{
	CVerb ( "DisposeD3D" );

	LockAcquireV ( &native.kernelLock, "DisposeD3D" );

	OBJ_RELEASE ( d3dContext );
	OBJ_RELEASE ( d3dMainDevice );
	d3dDevice = 0;

	DeleteCriticalSection ( &capturedFrameCS );

	LockReleaseV ( &native.kernelLock, "DisposeD3D" );

	return true;
}


CaptureWinD3D::~CaptureWinD3D ()
{
	CLogID ( "Destruct" );

	Release ();

	if ( didAddRef )
	{
		EnterCriticalSection ( &winD3DInitLockClass::staticInitDelLock );

		if ( __sync_sub_and_fetch ( &referenceCount, 1 ) == 0 ) {
			CDispose ( );
		}

		LeaveCriticalSection ( &winD3DInitLockClass::staticInitDelLock );
	}
}


void CaptureWinD3D::CDispose ()
{
	CLog ( "CDispose" );

	// At first terminate the thread
	DisposeThread ( 0, captureThreadID, 0, "CaptureWinD3D class", capturedEvent );	

	OBJ_RELEASE ( d3dAcquiredDesktopImage );
	OBJ_RELEASE ( d3dOutputDuplication );
}


void CaptureWinD3D::Release ( )
{
	CVerbID ( "Release" );

	initialized = false;

	buffersInitialized	= false;

	framesAvail = 0;

}


bool CaptureWinD3D::Duplicate ()
{
	CVerb ( "Duplicate" );

	HRESULT status;
	DXGI_OUTPUT_DESC desc;
	IDXGIDevice	* device	= 0;
	IDXGIAdapter* adapter	= 0;
	IDXGIOutput	* output;
	IDXGIOutput1* output1;

	status = d3dMainDevice->QueryInterface ( IID_IDXGIDevice, (void**)&device );
	if ( FAILED ( status ) || !device  ) {
		CErr ( "Duplicate: Failed to get Interface for DXGI" );
		return false;
	}
	
	status = device->GetParent ( IID_IDXGIAdapter, (void**)&adapter );
	OBJ_RELEASE ( device  );

	if ( FAILED ( status ) || !adapter ) {
		CErr ( "Duplicate: Failed to get DXGI adapter" );
		return false;
	}

	unsigned int i = 0;
	Zero ( desc );

	output = 0;
	while ( adapter->EnumOutputs ( i, &output ) != DXGI_ERROR_NOT_FOUND && output )
	{
		DXGI_OUTPUT_DESC* pDesc = &desc;

		status = output->GetDesc ( pDesc );
		if ( FAILED ( status ) ) {
			CErrArg ( "Duplicate: Failed to get adapter description [%d]", i );
			return false;
		}

		CVerbArg ( "Duplicate: Adapter %d: [%s] [%d]", i, pDesc->DeviceName, pDesc->AttachedToDesktop );

		output->Release ( );
		i++;
	}

	output = 0;
	status = adapter->EnumOutputs ( 0, &output );
	OBJ_RELEASE ( adapter );
	if ( FAILED ( status ) ) {
		CErr ( "Duplicate: Failed to get output for adapter 0" );
		return false;
	}

	output1	= 0;
	status = output->QueryInterface ( IID_IDXGIOutput1, (void**) &output1 );
	OBJ_RELEASE ( output );
	if ( FAILED ( status ) ) {
		CErr ( "Duplicate: Failed to get IDXGIOutput1 for adapter 0" );
		return false;
	}

	status = output1->DuplicateOutput ( (IUnknown*) d3dMainDevice, &d3dOutputDuplication );
	OBJ_RELEASE ( output1 );
	if ( FAILED ( status ) ) {
		if ( status == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE )
		{
			CErr ( "Duplicate: DXGI currently not available. Maybe too many apps using dxgi." );
			return false;
		}
		
		CErrArg ( "Duplicate: Failed to get output duplication. Status [%#X]", status );
		return false;
	}

	if ( !d3dOutputDuplication ) {
		CErr ( "Duplicate: Failed to get output duplication interface!" );
		return false;
	}
	return true;
}


bool CaptureWinD3D::AcquireNextFrame ( UINT timeout )
{
	//CLog ( "AcquireNextFrame" );

	if ( !d3dOutputDuplication ) {
		CErr ( "AcquireNextFrame: Output duplication interface is invalid!" );
		return false;
	}

	HRESULT				status = 0;
	IDXGIResource *		duplicatedResource = 0;

	if ( framesAvail > 0 ) {
		ReleaseFrame ();
	}

	status = d3dOutputDuplication->AcquireNextFrame ( timeout, &FrameInfo, &duplicatedResource );
	if ( status == DXGI_ERROR_WAIT_TIMEOUT ) {
		return false;
	}

	if ( FAILED ( status ) ) {
		CErrArg ( "AcquireNextFrame: Failed to acquire next frame. Status [%#X]", status );

		if ( status == DXGI_ERROR_ACCESS_LOST )
		{
			CErr ( "AcquireNextFrame: ACCESS LOST -> try connecting again..." );

			OBJ_RELEASE ( d3dAcquiredDesktopImage );
			OBJ_RELEASE ( d3dOutputDuplication );

			Duplicate ( );
			return false;
		}
		else
		{
			status = d3dOutputDuplication->ReleaseFrame ( );
			if ( FAILED ( status ) ) {
				CErrArg ( "AcquireNextFrame: Failed to release frame. Status [%#X]", status );
			}
			return false;
		}
	}


	EnterCriticalSection ( &capturedFrameCS );

	OBJ_RELEASE ( d3dAcquiredDesktopImage );

	status = duplicatedResource->QueryInterface ( IID_ID3D11Texture2D, (void**)&d3dAcquiredDesktopImage );

	LeaveCriticalSection ( &capturedFrameCS );

	OBJ_RELEASE ( duplicatedResource );
	if ( FAILED ( status ) ) {
		return false;
	}

	framesAvail = FrameInfo.AccumulatedFrames;

	if ( FrameInfo.AccumulatedFrames == 0 )
	{
		status = d3dOutputDuplication->ReleaseFrame ( );
		if ( FAILED ( status ) ) {
			CErrArg ( "AcquireNextFrame: Failed to release frame. Status [%#X]", status );
		}
	}

	return true;
}




void CaptureWinD3D::ReleaseFrame ()
{
	//CLog ( "ReleaseFrame" );

	HRESULT status = d3dOutputDuplication->ReleaseFrame ( );

	if ( FAILED ( status ) ) {
		CErrArg ( "ReleaseFrame: Failed to release frame. Status [%#X]", status );
		return;
	}

	framesAvail = 0;
}



int CaptureWinD3D::AllocateResources ( RenderDimensions * dims )
{
	CVerbID ( "AllocateResources" );

	dataSize = dims->square * dims->square * 4;

	HRESULT				status;

	D3D11_TEXTURE2D_DESC desc;
	Zero ( desc );

	desc.Width = dims->square;
	desc.Height = dims->square;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // seems to work on intel, check for ati
	//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if ( bufferType == CaptureBufferType::Texture3D )
		desc.Usage = D3D11_USAGE_DEFAULT; // Working
	else {
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	}
	//desc.Usage = D3D11_USAGE_DYNAMIC;

	//desc.CPUAccessFlags = 0;	// Working
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//desc.BindFlags = 0; 	// Working
	//desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	status = d3dMainDevice->CreateTexture2D ( &desc, 0, (ID3D11Texture2D **) &d3dTexture);

	if ( FAILED ( status ) || !d3dTexture ) {
		CErrArgID ( "AllocateResources: Failed to create D3D Texture2D. Status [%#X]", status );
		return -1;
	}

	if ( bufferType == CaptureBufferType::Texture3D ) {
		data = (unsigned char *)d3dTexture;
		return 1;
	}

	DXGI_MAPPED_RECT mappedRect;
	status = ((ID3D11Texture2D *)d3dTexture)->QueryInterface ( IID_IDXGISurface, (void**)&d3dRenderSurface );
	if ( FAILED ( status ) ) {
		CErrArgID ( "AllocateResources: Failed to query interface for surface rendering. Status [%#X]", status );
		return -1;
	}

	status = d3dRenderSurface->Map ( &mappedRect, DXGI_MAP_READ );
	if ( FAILED ( status ) ) {
		CErrArgID ( "AllocateResources: Failed to map render surface for staging. Status [%#X]", status );
		return -1;
	}

	dataStride = mappedRect.Pitch;
	data = mappedRect.pBits;
	return 1;
}


int CaptureWinD3D::ReleaseResources ( )
{
	//CLog ( "ReleaseResources" );

	if ( d3dRenderSurface ) {
		d3dRenderSurface->Unmap ( );
		OBJ_RELEASE ( d3dRenderSurface );
	}

	ID3D11Texture2D	* captureTexture2D = (ID3D11Texture2D *)d3dTexture;
	if ( captureTexture2D ) {
		captureTexture2D->Release ( );
		d3dTexture = 0;
	}

	data				= 0;
	dataSize			= 0;
	buffersInitialized	= false;
    
    return 1;
}


int CaptureWinD3D::Perform ( RenderDimensions * dims, RenderContext * context )
{
	D3D11_BOX Box;

	if ( WaitForSingleObject ( capturedEvent, 500 ) != WAIT_OBJECT_0 )
		return 0;

	//CLog ( "Capture" );

	/*D3D11_TEXTURE2D_DESC desc;
	Zero ( desc );
	d3dAcquiredDesktopImage->GetDesc ( &desc );*/

	unsigned int _squareWidth = dims->square;
	unsigned int _squareHeight = dims->square;

	int destLeft = 0;
	int leftCap = 0;
	if ( dims->left_cap < 0 ) {
		destLeft -= dims->left_cap;
		_squareWidth -= destLeft;
		leftCap = 0;
	}
	else if ( dims->left_cap + dims->square > static_cast< int >( env->appWindowWidth ) ) {
		unsigned int diff = ( dims->left_cap + dims->square ) - env->appWindowWidth;
		_squareWidth -= diff;
	}

	int destTop = 0;
	int topCap = 0;
	if ( dims->top_cap < 0 ) {
		destTop -= dims->top_cap;
		_squareHeight -= dims->top_cap;
		topCap = 0;
	}
	else if ( dims->top_cap + dims->square >static_cast< int >( env->appWindowHeight ) ) {
		unsigned int diff = ( dims->top_cap + dims->square ) - env->appWindowHeight;
		_squareHeight -= diff;
	}

	Box.top = topCap;
	Box.left = leftCap;
	Box.right = _squareWidth;
	Box.bottom = _squareHeight;
	Box.front = 0;
	Box.back = 1;

	EnterCriticalSection ( &capturedFrameCS );

	if ( !d3dAcquiredDesktopImage ) {
		LeaveCriticalSection ( &capturedFrameCS );
		CWarnID ( "Perform: Missing desktop image from acquire thread." );
		return 0;
	}

	d3dContext->CopySubresourceRegion ( (ID3D11Resource*)d3dTexture, 0, destLeft, destTop, 0, (ID3D11Resource*)d3dAcquiredDesktopImage, 0, &Box );
	
	LeaveCriticalSection ( &capturedFrameCS );

	InterlockedIncrement ( &clientAccessed );

	return 1;
}


bool CaptureWinD3D::GetInvalidRegion ( RECT * rc )
{
	UINT i;
	HRESULT status;
	UINT dirty;
	UINT BufSize;
	RECT* pRect;
	BYTE* DirtyRects;
	UINT DataBufferSize = 0;
	BYTE* DataBuffer = NULL;

	if (FrameInfo.AccumulatedFrames == 0) {
		return false;
	}

	if (FrameInfo.TotalMetadataBufferSize) {
		if (FrameInfo.TotalMetadataBufferSize > DataBufferSize)
		{
			if (DataBuffer) {
				free(DataBuffer);
				DataBuffer = NULL;
			}

			DataBuffer = (BYTE*)malloc(FrameInfo.TotalMetadataBufferSize);
			if (!DataBuffer) {
				DataBufferSize = 0;
				CErr ( "Failed to allocate memory for metadata\n" );
				return false;
			}
			DataBufferSize = FrameInfo.TotalMetadataBufferSize;
		}

		BufSize = FrameInfo.TotalMetadataBufferSize;

		status = d3dOutputDuplication->GetFrameMoveRects ( BufSize, (DXGI_OUTDUPL_MOVE_RECT*) DataBuffer, &BufSize );

		if (FAILED(status)) {
			CErr ( "Failed to get frame move rects\n" );
			return false;
		}

		DirtyRects = DataBuffer + BufSize;
		BufSize = FrameInfo.TotalMetadataBufferSize - BufSize;

		status = d3dOutputDuplication->GetFrameDirtyRects ( BufSize, (RECT*) DirtyRects, &BufSize );

		if (FAILED(status)) {
			CErr ( "Failed to get frame dirty rects\n" );
			return false;
		}
		dirty = BufSize / sizeof(RECT);

		pRect = (RECT*)DirtyRects;

		for (i = 0; i<dirty; ++i) {
			UnionRect(rc, rc, pRect);
			++pRect;
		}
	}

	return 0;
}


void * CaptureWinD3D::Thread_Capture ( void * arg )
{
	CLog ( "Thread_Capture: created." );

	long clientAccessedCache = clientAccessed + 1;

	SetEvent ( captureThreadEvent );

	while ( referenceCount )
	{
		DWORD res = WaitForSingleObject ( captureClassTimerSignal, 1000 );
		if ( res == WAIT_OBJECT_0 )
		{
			//CLog ( "Thread_Capture: signaled." );
			if ( !referenceCount )
				break;

			ResetEvent ( capturedEvent );

			long incValue = InterlockedIncrement ( &clientAccessed );

			clientAccessedCache++;
			if ( incValue == clientAccessedCache ) {
				SetEvent ( capturedEvent );
				continue;
			}

			clientAccessedCache = incValue;

			AcquireNextFrame ( 1000 );

			SetEvent ( capturedEvent );
		}
		else if ( res != WAIT_TIMEOUT ) {
			break;
		}
	}

	//pthread_reset ( captureThreadID );  // Will be done by the "closer"
	CLog ( "Thread_Capture: bye bye..." );
	return 0;
}


/*
bool CaptureWinD3D::ConnectOutput ( IEnvironsBase * dest )
{
	CVerbID ( "ConnectOutput" );

	if ( dest->interfaceType != InterfaceType::Render ) {
		return false;
	}

	CVerbArgID ( "ConnectOutput: Try connecting with input of [%s]", dest->name );

	IPortalRenderer * renderer = (IPortalRenderer *) dest;

	unsigned int outputLengt = sizeof(CaptureWinD3D_outputTypeSupport) / sizeof(PortalBufferType::PortalBufferType);

	for ( unsigned int i = 0; i < renderer->inputTypesLength; i++ )
	{
		for ( unsigned int j = 0; j < outputLengt; j++ )
		{
			if ( renderer->inputTypes [i] == CaptureWinD3D_outputTypeSupport [j] ) {
				outputType = CaptureWinD3D_outputTypeSupport [j];
				renderer->inputType = outputType;
				return true;
			}
		}
	}

	CErrArgID ( "ConnectOutput: Failed connecting with input of [%s]", dest->name );
	return false;
}
*/


} /* namespace environs */


#else
namespace environs
{
	void *			CaptureWinD3D::d3dDevice		= 0;
}
#endif

#endif