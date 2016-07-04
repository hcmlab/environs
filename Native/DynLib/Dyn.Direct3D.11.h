/**
* Dynamically accessing DirectX 3D API
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
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_DIRECTX_3D_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_DIRECTX_3D_H

#ifdef _WIN32

#include <d3d11.h>

#ifdef WINDOWS_8
//#define CINTERFACE
#include <dxgi1_2.h>
#endif

namespace environs
{
	typedef HRESULT ( WINAPI *pD3D11CreateDevice )(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT32, D3D_FEATURE_LEVEL *, UINT, UINT32, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);

extern void ReleaseDirect3D ( );
extern bool InitLibDirect3D ( );

extern bool								direct3D_LibInitialized;

extern pD3D11CreateDevice				dD3D11CreateDevice;


} // -> namespace environs


#endif // INCLUDE_HCM_ENVIRONS_DYNAMIC_DIRECTX_3D_H

#endif