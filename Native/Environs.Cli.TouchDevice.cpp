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
#include "stdafx.h"

#if (defined(CLI_CPP) && (defined(CLI_STT)  || defined(CLI_PS)))

#include "Environs.Cli.STT.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.TouchDevice.h"


namespace environs
{

	/// <summary>
	/// Constructor for a new input pack.
	/// </summary>
	/// <param name="pack">The environs input pack.</param>
	EnvironsTouchDevice::EnvironsTouchDevice ( int nativeID, InputPack ^ pack ) : TouchDevice ( pack->id )
	{		
		this->nativeID = nativeID;
		this->pack = pack;
	}


	void EnvironsTouchDevice::Init ( ENV_CLIENTWINDOW ^ clientWindow )
	{
		if ( clientWindow != nullptr )
		{
			touchSource = PresentationSource::FromVisual ( clientWindow );
			if ( touchSource != nullptr )
				return;
		}
		touchSource = Mouse::PrimaryDevice->ActiveSource;
	}


		/// <summary>
		/// Call this method to signal a new touch down event.
		/// </summary>
		/// <param name="nativeID">The device id.</param>
		/// <param name="pack">The input package.</param>
	void EnvironsTouchDevice::ContactDown ( int nativeID, InputPack ^ pack )
	{
		try
		{
			if ( deviceDictionary->ContainsKey ( pack->id ) )
			{
				return;
			}
			EnvironsTouchDevice ^ device = gcnew EnvironsTouchDevice ( nativeID, pack );
			deviceDictionary->Add ( pack->id, device );

			//PresentationSource.CurrentSources.FromVisual(SurfaceEnvirons.instance.winForm.);
			//device.SetActiveSource(System.Windows.Input.Stylus.CurrentStylusDevice.ActiveSource);
			//device.SetActiveSource(PresentationSource.FromVisual(SurfaceEnvirons.instance.winForm.Handle));
			//device.SetActiveSource(PresentationSource.FromVisual(SurfaceEnvirons.instance.surfaceWindow));
			//device.SetActiveSource(Mouse.PrimaryDevice.ActiveSource);
			device->SetActiveSource ( touchSource );
			device->Activate ();
			device->ReportDown ();
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^e )
		{
			Debug::WriteLine ( e->Message );
			Debug::WriteLine ( e->StackTrace );
		}
	}


	/// <summary>
	/// Call this method to signal that a contact has moved to a new location.
	/// </summary>
	/// <param name="nativeID">The device id.</param>
	/// <param name="pack">The input package.</param>
	void EnvironsTouchDevice::ContactChanged ( int nativeID, InputPack ^ pack )
	{
		try
		{
			if ( !deviceDictionary->ContainsKey ( pack->id ) )
			{
				ContactDown ( nativeID, pack );
			}

			EnvironsTouchDevice ^ device = deviceDictionary [ pack->id ];
			if ( device != nullptr &&
				device->IsActive )
			{
				if ( device->pack->x != pack->x || device->pack->y != pack->y || device->pack->angle != pack->angle || device->pack->size != pack->size )
				{
					device->pack = pack;
					device->ReportMove ();
				}
			}
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^)
		{
		}
	}


	/// <summary>
	/// Call this method to signal that a contact has been left from the surface at the given position.
	/// </summary>
	/// <param name="nativeID">The device id.</param>
	/// <param name="pack">The input package.</param>
	void EnvironsTouchDevice::ContactUp ( int nativeID, InputPack ^ pack )
	{
		try
		{
			if ( !deviceDictionary->ContainsKey ( pack->id ) )
			{
				ContactDown ( nativeID, pack );
			}
			EnvironsTouchDevice ^ device = deviceDictionary [ pack->id ];

			if ( device != nullptr &&
				device->IsActive )
			{
				device->ReportUp ();
				device->Deactivate ();

				deviceDictionary->Remove ( pack->id );
			}
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^ )
		{
		}
	}


	bool EnvironsTouchDevice::HasContactChanged ( int nativeID, InputPack ^ pack )
	{
		try
		{
			if ( deviceDictionary->ContainsKey ( pack->id ) )
			{
				EnvironsTouchDevice ^device = deviceDictionary [ pack->id ];

				int a1 = ( int ) device->pack->angle;
				int a2 = ( int ) pack->angle;
				if ( pack->type == INPUT_TYPE_MARKER )
				{
					if ( Math::Abs ( device->pack->x - pack->x ) > 30 || Math::Abs ( device->pack->y - pack->y ) > 30 || a1 != a2 )
					{
						return true;
					}
					return false;
				}

				if ( device->pack->x != pack->x || device->pack->y != pack->y || device->pack->size != pack->size || a1 != a2 )
				{
					return true;
				}
			}
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^)
		{
		}
		return false;
	}

		/// <summary>
		/// RegisterEvents is not used yet.
		/// </summary>
		/// <param name="root"></param>
	void EnvironsTouchDevice::RegisterEvents ( FrameworkElement ^ root )
	{
	}

	int EnvironsTouchDevice::GetNativeID ( int touchID )
	{
		if ( deviceDictionary->ContainsKey ( touchID ) )
		{
			EnvironsTouchDevice ^ device = deviceDictionary [ touchID ];
			if ( device != nullptr )
				return device->nativeID;
		}
		return 0;
	}


		/// <summary>
		/// GetIntermediateTouchPoints determines the current state of all active contact events.
		/// </summary>
		/// <param name="relativeTo">An InputElement to which the contacts' locations should be computed relative to. If set to <c>null</c>, then the application window is used as reference.</param>
		/// <returns>A TouchPointCollection containing all the active contact events.</returns>
	TouchPointCollection ^ EnvironsTouchDevice::GetIntermediateTouchPoints ( IInputElement ^ relativeTo )
	{
		TouchPointCollection ^ collection = gcnew TouchPointCollection ();
		UIElement ^ element = ( UIElement^ ) relativeTo;

		if ( element == nullptr )
			return collection;
		try
		{
			/*foreach (IntermediateContact c in Contact.GetIntermediateContacts())
			{
			Point point = c.GetPosition(null);
			if (relativeTo != null)
			{
			point = this.ActiveSource.RootVisual.TransformToDescendant((Visual)relativeTo).Transform(point);
			}
			collection.Add(new TouchPoint(this, point, c.BoundingRect, TouchAction.Move));
			}
			*/
			Point point ( pack->x, pack->y );
			if ( relativeTo != nullptr )
			{
				point = ActiveSource->RootVisual->TransformToDescendant ( ( System::Windows::Media::Visual ^ ) relativeTo )->Transform ( point );
			}
			collection->Add ( gcnew TouchPoint ( this, point, Rect ( pack->x - 10, pack->y - 10, 20, 20 ), TouchAction::Move ) );
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^ )
		{
		}
		return collection;
	}

	/// <summary>
	/// Determines the position and boundings of this contact event instance.
	/// </summary>
	/// <param name="relativeTo">An InputElement to which the contacts' locations should be computed relative to. If set to <c>null</c>, then the application window is used as reference.</param>
	/// <returns>A TouchPoint with position and boundings.</returns>
	TouchPoint ^ EnvironsTouchDevice::GetTouchPoint ( IInputElement ^ relativeTo )
	{
		try
		{
			Point point ( pack->x, pack->y );
			if ( relativeTo != nullptr )
			{
				point = ActiveSource->RootVisual->TransformToDescendant ( ( System::Windows::Media::Visual ^ ) relativeTo )->Transform ( point );
			}

			Rect rect ( pack->x - 10, pack->y - 10, 20, 20 ); // this.Contact.BoundingRect;

			return gcnew TouchPoint ( this, point, rect, TouchAction::Move );
		}
		//Ignore InvalidOperationException due to race condition on Surface hardware
		catch ( InvalidOperationException ^)
		{
		}
		return nullptr;
	}

	/// <summary>
	/// Determine whether this contact is a finger or not.
	/// </summary>
	/// <returns>Returns true for a finger contact and false otherwise.</returns>
	bool EnvironsTouchDevice::GetIsFingerRecognized ()
	{
		return ( pack->type == INPUT_TYPE_FINGER );
	}

	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	bool EnvironsTouchDevice::GetIsTagRecognized ()
	{
		return ( pack->type == INPUT_TYPE_MARKER );
	}

	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	bool EnvironsTouchDevice::GetIsPenRecognized ()
	{
		return ( pack->type == INPUT_TYPE_PEN );
	}

	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	Microsoft::Surface::Presentation::Input::TagData ^ EnvironsTouchDevice::GetTagData ()
	{
		Microsoft::Surface::Presentation::Input::TagData ^ data = gcnew Microsoft::Surface::Presentation::Input::TagData ( 0, 0, ( long ) pack->value, ( long ) pack->value );

		return data;
	}

	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	double EnvironsTouchDevice::GetOrientation ()
	{
		return ( double ) pack->angle;
	}


	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	System::Windows::Point ^ EnvironsTouchDevice::GetPosition ()
	{
		System::Windows::Point ^ p = gcnew System::Windows::Point ();
		p->X = pack->x;
		p->Y = pack->y;

		return p;
	}


	/// <summary>
	/// Determine whether this contact is a marker or not.
	/// </summary>
	/// <returns>Returns true for a marker contact and false otherwise.</returns>
	double EnvironsTouchDevice::GetPhysicalArea ()
	{
		return pack->axisx * pack->axisy;
	}


}


#endif