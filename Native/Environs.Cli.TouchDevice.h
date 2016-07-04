/**
 * Environs CLI touchdevice extension
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CLI_TOUCHDEVICE_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CLI_TOUCHDEVICE_H

#if (defined(CLI_CPP) && (defined(CLI_STT)  || defined(CLI_PS)))

#include "Environs.Cli.h"
#include "Utils/Touch.Extensions.h"

namespace environs 
{
	/// <remarks>
	/// This class is a helper class to injects touch events into the microsoft surface 2 surface application windows.
	/// Each instance represents a touch contact, which were household in a static class dictionary.
	/// </remarks>
	ref class EnvironsTouchDevice : TouchDevice
	{
	public:
		EnvironsTouchDevice ( int nativeID, InputPack ^pack );

		/// <summary>
		/// The nativeID that this touch belongs to
		/// </summary>
		int nativeID;

		/// <summary>
		/// GetIntermediateTouchPoints determines the current state of all active contact events.
		/// </summary>
		/// <param name="relativeTo">An InputElement to which the contacts' locations should be computed relative to. If set to <c>null</c>, then the application window is used as reference.</param>
		/// <returns>A TouchPointCollection containing all the active contact events.</returns>
		virtual TouchPointCollection ^ GetIntermediateTouchPoints ( IInputElement ^ relativeTo ) override;


		/// <summary>
		/// Determines the position and boundings of this contact event instance.
		/// </summary>
		/// <param name="relativeTo">An InputElement to which the contacts' locations should be computed relative to. If set to <c>null</c>, then the application window is used as reference.</param>
		/// <returns>A TouchPoint with position and boundings.</returns>
		virtual TouchPoint ^ GetTouchPoint ( IInputElement ^ relativeTo ) override;


		/// <summary>
		/// Determine whether this contact is a finger or not.
		/// </summary>
		/// <returns>Returns true for a finger contact and false otherwise.</returns>
		bool GetIsFingerRecognized ();

		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		bool GetIsTagRecognized ();

		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		bool GetIsPenRecognized ();

		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		Microsoft::Surface::Presentation::Input::TagData ^ GetTagData ();

		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		double GetOrientation ();


		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		System::Windows::Point ^ GetPosition ();


		/// <summary>
		/// Determine whether this contact is a marker or not.
		/// </summary>
		/// <returns>Returns true for a marker contact and false otherwise.</returns>
		double GetPhysicalArea ();

	internal:
			/// <summary>
			/// Dictionary that holds the state of the contact events that are currently in active state
			/// </summary>
		static Dictionary<int, EnvironsTouchDevice ^> ^deviceDictionary = gcnew Dictionary<int, EnvironsTouchDevice ^> ();

		/// <summary>
		/// Dictionary that holds the state of the contact events that are currently in active state
		/// </summary>
		static PresentationSource ^ touchSource = nullptr;

		/// <summary>
		/// The input pack.
		/// </summary>
		InputPack ^ pack;

		static void Init ( ENV_CLIENTWINDOW ^ clientWindow );

		/// <summary>
		/// Call this method to signal a new touch down event.
		/// </summary>
		/// <param name="nativeID">The device id.</param>
		/// <param name="pack">The input package.</param>
		static void ContactDown ( int nativeID, InputPack ^ pack );

		/// <summary>
		/// Call this method to signal that a contact has moved to a new location.
		/// </summary>
		/// <param name="nativeID">The device id.</param>
		/// <param name="pack">The input package.</param>
		static void ContactChanged ( int nativeID, InputPack ^ pack );


		/// <summary>
		/// Call this method to signal that a contact has been left from the surface at the given position.
		/// </summary>
		/// <param name="nativeID">The device id.</param>
		/// <param name="pack">The input package.</param>
		static void ContactUp ( int nativeID, InputPack ^ pack );

		static bool HasContactChanged ( int nativeID, InputPack ^ pack );

		static void RegisterEvents ( FrameworkElement ^root );

		static int GetNativeID ( int touchID );
	};
}


#endif

#endif