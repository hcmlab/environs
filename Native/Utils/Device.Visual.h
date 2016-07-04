/**
 * Environs CPP Device Visual
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_DEVICE_VISUAL_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_DEVICE_VISUAL_H

#if (defined(CLI_CPP))


namespace environs 
{
	/**
	*	Device Visual
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	public ref class DeviceVisual
	{
	public:
		DeviceVisual ( PortalInstance ^ portal );

		Microsoft::Surface::Presentation::Controls::ScatterViewItem ^scatterItem;
		System::Windows::Controls::Canvas ^drawingContent;

		System::Windows::Point ^ currentPosition;
		float currentOrientation;

		int currentWidth;
		int currentHeight;

		bool visible;

		PortalInstance ^ portal;

		static Dictionary<int, DeviceVisual^> ^deviceVisuals = gcnew Dictionary<int, DeviceVisual^> ();

		property bool isActiveItemVisual
		{
			bool get () { return activeItemVisual; }
		}

		void Renderer ( System::Windows::Media::Imaging::WriteableBitmap ^ bitmap );

		static bool IsLocationOccludedByDevice ( int x, int y );

		void UpdateSize ( PortalInfo ^ info );

		void UpdateSize ( int width, int height );

		void Size ( int width, int height );

		void Orientation ( float angle );

		void UpdatePosition ( PortalInfoBase ^ info );

		void Position ( int center_x, int center_y );

		void Position ( System::Windows::Point ^ pt );

		void Show ();

		void Hide ();

		void UpdateContactStatus ();

		bool UnRegister ();

		void Remove ();

		static DeviceVisual ^ GetOrCreate ( PortalInstance ^ portal, System::Windows::Controls::ItemCollection ^ itemCollection );

		void RebuildVisual ();

		bool Init ( System::Windows::Controls::ItemCollection ^ itemCollection );

		static DeviceVisual ^ GetDevice ( int key );

		void OnPreviewDown ( System::Windows::Point ^ tapPoint );

		void OnPreviewMouseDown ( Object ^ sender, System::Windows::Input::MouseButtonEventArgs ^ e );

#ifdef CLI_PS
		void OnPreviewTouchDown ( Object ^ sender, System::Windows::Input::TouchEventArgs ^ e );
#endif
		void Release ();


	internal:
		void Init ();

		System::Windows::Controls::Border ^ border;
		System::Windows::Shapes::Line ^ stem;
		bool activeItemVisual;
		System::Windows::Controls::ItemCollection ^ parentCollection;

		System::Diagnostics::Stopwatch ^ tapStopwatch;
		System::Windows::Point ^ lastTapPoint;

		int prevX; 
		int prevY;
		float angle;

		void onLayoutUpdated ( Object ^ sender, System::EventArgs ^ e );
		void item_SizeChanged ( Object ^ sender, System::Windows::SizeChangedEventArgs ^ e );
	};
}


#endif

#endif