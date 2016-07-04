/**
 * PortalInstance CLI Object
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Environs.Cli.Forwards.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"

#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Environs.h"

#include "Device/Device.Instance.Cli.h"
#include "Portal.Instance.Cli.h"

#ifndef CLI_NOUI
	using namespace System::Drawing;
#endif

#define CLASS_NAME	"Portal.Instance.Cli. . ."


namespace environs
{
	PortalInstance::PortalInstance ()
	{
		CVerb ( "Construct" );

		appContext0 = nullptr;
		appContext1 = nullptr;
		appContext2 = nullptr;
		appContext3 = nullptr;

		portalRenderer		= nullptr;

		surfaceBitmap		= nullptr;

		portalSinkHandler	= nullptr;
		portalRenderer		= nullptr;

		surface				= nullptr;
		info_.portal		= this;
	}


	PortalInstance::~PortalInstance ()
	{
		CVerbArg1 ( "Destruct", "", "i", objID_ );
	}


	EPSPACE PortalInstance ^ PortalInstance::GetPlatformObj ()
	{
		return this;
	}


	bool PortalInstance::SetRenderCallback ( Environs::PortalRenderer ^ renderer )
	{
		return SetRenderSurface ( nullptr, 0, 0 );
	}


	bool PortalInstance::SetRenderCallback ( Environs::PortalRenderer ^ renderer, int width, int height )
	{
		if ( renderer != nullptr )
			portalRenderer += renderer;

		renderActivated = SetRenderSurface ( nullptr, width, height );
		return renderActivated;
	}


	ref class PortalInstancePortalSinkEvent
	{
		PortalInstance ^ portal;
		IntPtr ^ avContext;

	public:
		PortalInstancePortalSinkEvent ( PortalInstance ^ p, IntPtr ^ a ) : portal ( p ), avContext ( a ) { }
		void Run () {
			if ( !Utils::CreateUpdateBitmap ( portal->surfaceBitmap, avContext ) )
				return;

			if ( portal->surfaceBitmap == nullptr )
				return;

			if ( portal->portalRenderer != nullptr )
				portal->portalRenderer ( portal->surfaceBitmap );

			if ( portal->surface != nullptr )
				portal->surface->Background = gcnew System::Windows::Media::ImageBrush ( portal->surfaceBitmap );
		}
	};


	void PortalInstance::PortalSinkEvent ( int type, IntPtr surface, IntPtr avContext )
	{
		//Debug.WriteLine("[INFO] PortalSinkEvent:");

		if ( type != RENDER_CALLBACK_TYPE_AVCONTEXT )
			return;

		PortalInstancePortalSinkEvent ^ act = gcnew PortalInstancePortalSinkEvent ( this, avContext );

		Action ^ action = gcnew Action ( act, &PortalInstancePortalSinkEvent::Run );

		environs::Environs::dispatchSync ( action );
	}


	bool PortalInstance::SetRenderSurface ( System::Windows::Controls::Control ^ surface, int width, int height )
	{
		portalSinkHandler += gcnew PortalSinkSource ( this, &PortalInstance::PortalSinkEvent );

		if ( surface != nullptr )
			this->surface = surface;

		if ( width != 0 && height != 0 && info->width == 0 && info->height == 0 )
		{
			info_.base.width = width; info_.base.height = height;
		}

		return environs::API::SetRenderCallbackN ( hEnvirons_, CALL_WAIT, portalID, Marshal::GetFunctionPointerForDelegate ( portalSinkHandler ).ToPointer (), RENDER_CALLBACK_TYPE_AVCONTEXT );
	}


	void PortalInstance::UpdateCallbacks ( int notification )
	{
		if ( surface != nullptr || portalRenderer == nullptr )
			return;

		portalSinkHandler += gcnew PortalSinkSource ( this, &PortalInstance::PortalSinkEvent );

		environs::API::SetRenderCallbackN ( hEnvirons_, CALL_WAIT, portalID, Marshal::GetFunctionPointerForDelegate ( portalSinkHandler ).ToPointer (), RENDER_CALLBACK_TYPE_AVCONTEXT );
	}


	bool PortalInstance::SetPortalOverlayPNG ( int layerID, int left, int top, String ^ pathToPNG, int alpha, bool positionDevice )
	{
#ifndef CLI_NOUI
		Bitmap ^ image = nullptr;

		try
		{
			if ( File::Exists ( pathToPNG ) )
				image = gcnew Bitmap ( pathToPNG );
			else
			{
				String ^ path = Utils::GetExecPath () + "/" + pathToPNG;
				if ( File::Exists ( path ) )
					image = gcnew Bitmap ( path );
				else
					return false;
			}
		}
		catch ( Exception ^ )
		{
			return false;
		}

		if ( image == nullptr )
			return false;

		System::Drawing::Imaging::BitmapData ^ data = image->LockBits (
			System::Drawing::Rectangle ( 0, 0, image->Width, image->Height ),
			System::Drawing::Imaging::ImageLockMode::ReadOnly, System::Drawing::Imaging::PixelFormat::Format32bppPArgb );

		if ( data == nullptr )
			return false;

		bool ret = environs::API::SetPortalOverlayARGBN ( hEnvirons_, device->info_->nativeID, portalID_, layerID,
			left, top, data->Width, data->Height, data->Stride, data->Scan0, alpha, positionDevice );

		image->UnlockBits ( data );
		//image->Dispose ();
		image = nullptr;
		return ret;
#else
		return false;
#endif
	}
}

#endif // CLI_CPP






