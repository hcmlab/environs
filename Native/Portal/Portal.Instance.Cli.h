/**
 * PortalInstance CLI
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_CLI_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_CLI_H

#ifdef CLI_CPP

#include "Portal.Instance.h"


/**
 *	PortalInstance CLI
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
	public ref class PortalInstance : public environs::lib::PortalInstance
	{
	public:
		PortalInstance ();
		~PortalInstance ();

		property Object ^ appContext0 { Object ^ get () { return appContext0_; } void set ( Object ^ value ) { appContext0_ = value; } };
		property Object ^ appContext1 { Object ^ get () { return appContext1_; } void set ( Object ^ value ) { appContext1_ = value; } };
		property Object ^ appContext2 { Object ^ get () { return appContext2_; } void set ( Object ^ value ) { appContext2_ = value; } };
		property Object ^ appContext3 { Object ^ get () { return appContext3_; } void set ( Object ^ value ) { appContext3_ = value; } };

		/** An ID that identifies this portal across all available portals. */
		property int portalID { int get () { return portalID_; } };

		/** true = Object is disposed and not updated anymore. */
		property bool disposed { bool get () { return ( disposed_ == 1 ); } };

		/**
		* Get the DeviceInstance that this PortalInstance is attached to.
		* */
		property EPSPACE DeviceInstance ^ device { EPSPACE DeviceInstance ^ get () { return device_; } };


		property environs::PortalInfo  ^ info { environs::PortalInfo  ^ get () { return %info_; } };

		property environs::PortalStatus status { environs::PortalStatus get () { return status_; } };

		property bool disposeOngoing { bool get () { return disposeOngoing_; } };


		/** true = outgoing (Generator), false = incoming (Receiver). */
		property bool outgoing { bool get () { return outgoing_; } };


		/** true = outgoing (Generator), false = incoming (Receiver). */
		property bool isOutgoing { bool get () { return outgoing_; } };


		/** true = outgoing (Generator), false = incoming (Receiver). */
		property bool isIncoming { bool get () { return !outgoing_; } };

		property environs::PortalType portalType
		{
			environs::PortalType get () { return ( environs::PortalType) portalType_; }
			void set ( environs::PortalType value ) { portalType_ = ( environs::PortalType_t) value; }
		};

		bool SetRenderCallback ( Environs::PortalRenderer ^ renderer );

		bool SetRenderCallback ( Environs::PortalRenderer ^ renderer, int width, int height );

		bool SetPortalOverlayPNG ( int layerID, int left, int top, String ^ pathToPNG, int alpha, bool positionDevice );

		void PortalSinkEvent ( int type, IntPtr surface, IntPtr avContext );

		bool SetRenderSurface ( System::Windows::Controls::Control ^ surface, int width, int height );


	internal:
		/** Application defined contexts for arbitrary use. */
		Object                ^ appContext0_;
		Object                ^ appContext1_;
		Object                ^ appContext2_;
		Object                ^ appContext3_;

		virtual EPSPACE PortalInstance ^ GetPlatformObj () override;

		Environs::PortalRenderer ^ portalRenderer;

		System::Windows::Media::Imaging::WriteableBitmap ^ surfaceBitmap;

		PortalSinkSource ^ portalSinkHandler;

		System::Windows::Controls::Control ^ surface;

		virtual void UpdateCallbacks ( int notification ) override;
	};
}

#endif // CLI_CPP

#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_CLI_H


