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
#include "stdafx.h"

#if (defined(CLI_CPP))

#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Environs.Lib.h"
#include "Environs.h"
#include "Portal/Portal.Instance.Cli.h"
#include "Device/Device.Instance.Cli.h"
#include "Device.Visual.h"
#include "Environs.Native.h"

using namespace System::Windows;
using namespace System::Windows::Shapes;
using namespace System::Windows::Controls;
using namespace System::Diagnostics;
using namespace System::Windows::Media;
using namespace System::Windows::Input;
using namespace System::Windows::Media::Effects;
using namespace Microsoft::Surface::Presentation::Controls;

#ifdef CLI_PS
using namespace Microsoft::Surface::Presentation::Controls::TouchVisualizations;
#else
using namespace Microsoft::Surface::Presentation::Controls::ContactVisualizations;
#endif

#define	CLASS_NAME 	"Device.Visual. . . . . ."


namespace environs
{
	void DeviceVisual::Init ()
	{
		visible = false;
		activeItemVisual = false;
		currentOrientation = 0;

		currentPosition = gcnew System::Windows::Point ( 42, 42 );

		scatterItem = nullptr;
		drawingContent = nullptr;
		portal = nullptr;
		border = nullptr;
		stem = nullptr;
		parentCollection = nullptr;

		prevX = 0;
		prevY = 0;
		angle = 0;

		currentWidth = 512;
		currentHeight = 512;

		tapStopwatch = gcnew Stopwatch ();
	}


	DeviceVisual::DeviceVisual ( PortalInstance ^ portal )
	{
		CVerb ( "Construct" );

		Init ();

		this->portal = portal;

		scatterItem = gcnew ScatterViewItem ();

		scatterItem->Visibility = Visibility::Hidden;
		visible = false;

		scatterItem->Background = Brushes::Transparent;
		//scatterItem.Foreground = Brushes.Transparent;
		scatterItem->IsTopmostOnActivation = false;
		//scatterItem.Background = Brushes.WhiteSmoke;
		scatterItem->Opacity = 1.0f;

		//scatterItem.PreviewTouchDown += onTouchIgnore;
		//scatterItem.PreviewTouchMove += onTouchIgnore;
		//scatterItem.PreviewTouchUp += onTouchIgnore;            

		DropShadowEffect ^ effect = gcnew DropShadowEffect ();
		effect->ShadowDepth = 1.0f;
		effect->Color = Colors::Black;
		effect->Opacity = 1;
		effect->BlurRadius = 12;
		scatterItem->Effect = effect;
		scatterItem->Width = currentWidth;
		scatterItem->Height = currentHeight;

		if ( activeItemVisual )
		{
#if CLI_PS
			scatterItem->PreviewTouchDown += gcnew EventHandler<TouchEventArgs ^> ( this, &DeviceVisual::OnPreviewTouchDown );
#endif
			scatterItem->PreviewMouseDown += gcnew MouseButtonEventHandler ( this, &DeviceVisual::OnPreviewMouseDown );

			scatterItem->IsEnabled = true;
			//TouchVisualizer.SetShowsVisualizations(scatterItem, true);
		}
		else
		{
			scatterItem->IsEnabled = false;

		}
#if CLI_PS
		TouchVisualizer::SetShowsVisualizations ( scatterItem, false );
#else
		ContactVisualizer::SetShowsVisualizations ( scatterItem, false );
#endif
	}


	void DeviceVisual::onLayoutUpdated ( Object ^ sender, EventArgs ^ e )
	{		
		if ( !portal->outgoing )
		{
			scatterItem->LayoutUpdated -= gcnew EventHandler ( this, &DeviceVisual::onLayoutUpdated );
			return;
		}

		Point p = scatterItem->ActualCenter;
		int x = ( int ) p.X;
		int y = ( int ) p.Y;
		float a = ( float ) ( scatterItem->ActualOrientation + 90 );
		if ( x != prevX || y != prevY || a != angle )
		{
			int width = ( ( int ) scatterItem->ActualWidth ) - 80;
			int height = ( ( int ) scatterItem->ActualHeight ) - 80;

			portal->info->Set ( x, y, a, width, height );

			prevX = x;
			prevY = y;
			angle = a;
			CVerb ( "onLayoutUpdated: x=" + x + " y=" + y + " a=" + a );
		}
	}


	void DeviceVisual::Release ()
	{
		CVerb (  "Dispose" );

		portal->ReleaseRenderSurface ();
	}


#if CLI_PS
	void DeviceVisual::OnPreviewTouchDown ( Object ^ sender, TouchEventArgs ^ e )
	{
		Point ^ tapPoint = e->GetTouchPoint ( scatterItem )->Position;
		OnPreviewDown ( tapPoint );
	}
#endif


	void DeviceVisual::OnPreviewMouseDown ( Object ^ sender, MouseButtonEventArgs ^ e )
	{
		Point tapPoint = e->MouseDevice->GetPosition ( scatterItem );
		OnPreviewDown ( tapPoint );
	}


	void DeviceVisual::OnPreviewDown ( Point ^ tapPoint )
	{
		TimeSpan elapsed = tapStopwatch->Elapsed;

#if CLI_PS
		tapStopwatch->Restart ();
#else
		tapStopwatch->Reset ();
#endif

		if ( ( elapsed == TimeSpan::Zero || elapsed > TimeSpan::FromSeconds ( 0.7 ) ) )
		{
			lastTapPoint = tapPoint;
			//Utils.Log(0, "OnPreviewDown: timespan failed");
			return;
		}

		double dist = Math::Sqrt ( Math::Pow ( tapPoint->X - lastTapPoint->X, 2 ) + Math::Pow ( tapPoint->Y - lastTapPoint->Y, 2 ) );
		lastTapPoint = tapPoint;
		if ( dist > 40 )
		{
			//Utils.Log(0, "OnPreviewDown: dist > 40 [" + dist + "]");
			return;
		}

		//Utils.Log(0, "OnPreviewDown: double tap");
		if ( portal->outgoing )
			portal->Establish ( true );
		//Environs.RequestPortalStream(deviceID, Environs.CALL_ASYNC, Environs.PortalType.Any);
	}


	/**
	* Query the DeviceVisual for deviceID.
	*
	* @param		key         Device identifier or portalID.
	* @return		DeviceVisual object or null
	*/
	DeviceVisual ^ DeviceVisual::GetDevice ( int key )
	{
		DeviceVisual ^ visual = nullptr;
		deviceVisuals->TryGetValue ( key, visual );
		return visual;
	}


	/**
	* Create a new DeviceVisual and add the visual to the internal collection.
	* Note: This method must be called within the UI context.
	*
	* @param		itemCollection
	* @return		success
	*/
	bool DeviceVisual::Init ( ItemCollection ^ itemCollection )
	{
		CVerb ( "Init: portal " + portal->portalID );
		if ( itemCollection == nullptr )
			return false;

		DeviceVisual ^ visual = nullptr;
		if ( deviceVisuals->TryGetValue ( portal->portalID, visual ) )
		{
			// Remove old visual at first
			visual->UnRegister ();
		}

		RebuildVisual ();

		deviceVisuals->Add ( portal->portalID, this );

		parentCollection = itemCollection;
		parentCollection->Add ( scatterItem );

		return true;
	}


	/**
	* Rebuild the elements of the DeviceVisual.
	* Note: This method must be called within the UI context.
	*
	*/
	void DeviceVisual::RebuildVisual ()
	{
		CVerb ( "RebuildVisual: portal " + portal->portalID );

		drawingContent = gcnew Canvas ();
		drawingContent->HorizontalAlignment = System::Windows::HorizontalAlignment::Stretch;
		drawingContent->VerticalAlignment = VerticalAlignment::Stretch;
		//drawingContent->Background = Brushes.Blue;
		drawingContent->PreviewMouseDown += gcnew MouseButtonEventHandler ( this, &DeviceVisual::OnPreviewMouseDown );


		border = gcnew Border ();
		border->Tag = this;
		border->BorderThickness = Thickness ( 0.5f, 0.5f, 0.5f, 0.5f );
		border->BorderBrush = Brushes::LightGreen;
		border->CornerRadius = CornerRadius ( 8, 8, 8, 8 );

		drawingContent->Children->Add ( border );

		stem = gcnew Line ();
		stem->Tag = this;
		stem->StrokeThickness = 0.5f;
		stem->Stroke = Brushes::LightGreen;
		drawingContent->Children->Add ( stem );

		scatterItem->Content = drawingContent;

		if ( !activeItemVisual )
		{
			ResourceDictionary ^ styles = gcnew ResourceDictionary ();
			styles->Source = gcnew Uri ( "pack://application:,,,/AppStyles.xaml", UriKind::RelativeOrAbsolute );
			Style ^ scs = ( Style ^) styles [ "RectangleShape" ];
			if ( scs != nullptr )
				scatterItem->Style = scs;

			//sb = styles["RectangleShapeColorChanger"] as Storyboard;
		}
	}


	ref class DeviceVisualWrapper
	{
	public:
		DeviceVisual ^ visual;
	};

	ref class DeviceVisualGetOrCreate
	{
		DeviceVisualWrapper ^ wrapper;
		PortalInstance ^ portal;
		ItemCollection ^ itemCollection;
		
	public:
		DeviceVisualGetOrCreate ( DeviceVisualWrapper ^ w, PortalInstance ^ p, ItemCollection ^ i ) : wrapper ( w ), portal (p ), itemCollection ( i ) { }
		void Run () 
		{
			DeviceVisual ^ visual = gcnew DeviceVisual ( portal );
			if ( visual == nullptr )
				return;
			portal->appContext1 = visual;

			if ( !visual->Init ( itemCollection ) )
			{
				CErr ( "GetOrCreate: Failed to identify portal id [0x" + portal->portalID.ToString ( "X" ) + "]!!" );
				visual = nullptr;
				return;
			}
			wrapper->visual = visual;
		}
	};


	/**
	* Query the DeviceVisual for the given portal. If it does not exist, then create a new one
	* and add the visual to the given itemCollection.
	*
	* @param		portal             The responsible PortalInstance object.
	* @param		itemCollection      The scatterItem collection in which this visual shall be added to.
	* @return		DeviceVisual object or null
	*/
	DeviceVisual ^ DeviceVisual::GetOrCreate ( PortalInstance ^ portal, ItemCollection ^ itemCollection )
	{
		CVerb ( "GetOrCreate: portal " + portal->portalID );
		if ( portal->appContext1 != nullptr )
			return ( DeviceVisual ^ ) portal->appContext1;

		DeviceVisual ^ visual = nullptr;

		Monitor::Enter ( deviceVisuals );

		visual = GetDevice ( portal->portalID );
		if ( visual == nullptr )
		{
			DeviceVisualWrapper wrapper;

			DeviceVisualGetOrCreate ^ act = gcnew DeviceVisualGetOrCreate ( %wrapper, portal, itemCollection );

			Action ^ action = gcnew Action ( act, &DeviceVisualGetOrCreate::Run );

			environs::Environs::dispatchSync ( action );

			visual = wrapper.visual;
		}
		Monitor::Exit ( deviceVisuals );

		return visual;
	}

	ref class DeviceVisualRemove
	{
		DeviceVisual ^ visual;

	public:
		DeviceVisualRemove ( DeviceVisual ^ v ) : visual ( v ) { }
		void Run () { visual->parentCollection->Remove ( visual->scatterItem ); }
	};

	/**
	* Removes the DeviceVisual with id deviceID from the internal collection as well as from the given itemCollection.
	*
	*/
	void DeviceVisual::Remove ()
	{
		CVerb ( "Remove: Disposing visual for tag " + portal->device->deviceID + " !!" );

		UnRegister ();

		DeviceVisualRemove ^ act = gcnew DeviceVisualRemove ( this );

		Action ^ action = gcnew Action ( act, &DeviceVisualRemove::Run );

		environs::Environs::dispatch ( action );
	}


	/**
	* Removes the DeviceVisual with id deviceID from the internal collection.
	*
	*/
	bool DeviceVisual::UnRegister ()
	{
		CVerb ( "UnRegister: Try disposing visual for tag " + portal->device->deviceID + " !!" );

		deviceVisuals->Remove ( portal->portalID );
		return true;
	}


	/**
	* Updates the contact status of the given DeviceVisual.
	* Query the contact status from Environs and update accordingly.
	*
	*/
	void DeviceVisual::UpdateContactStatus ()
	{
		if ( portal->device->directStatus == 1 )
		{
			CVerb ( "UpdateContactStatus: hiding visual" );
			Hide ();
		}
		else
		{
			CVerb ( "UpdateContactStatus: showing visual" );
			Show ();
		}
	}


	ref class DeviceVisualHide
	{
		DeviceVisual ^ visual;

	public:
		DeviceVisualHide ( DeviceVisual ^ v ) : visual ( v ) { }
		void Run () { visual->scatterItem->Visibility = Visibility::Hidden; }
	};

	/**
	* Hide the DeviceVisual.
	*
	*/
	void DeviceVisual::Hide ()
	{
		if ( !visible )
			return;

		DeviceVisualHide ^ act = gcnew DeviceVisualHide ( this );

		Action ^ action = gcnew Action ( act, &DeviceVisualHide::Run );

		environs::Environs::dispatch ( action );
	}


	ref class DeviceVisualShow
	{
		DeviceVisual ^ visual;

	public:
		DeviceVisualShow ( DeviceVisual ^ v ) : visual ( v ) { }
		void Run () {
			visual->scatterItem->Visibility = Visibility::Visible;
			visual->visible = true;

			if ( visual->portal->outgoing )
			{
				if ( visual->activeItemVisual )
				{
					visual->scatterItem->LayoutUpdated += gcnew EventHandler ( visual, &DeviceVisual::onLayoutUpdated );
				}
				else
				{
					visual->scatterItem->IsEnabled = false;
					//if (sb != null)
					//    sb.Begin(drawingContent);
				}
				visual->scatterItem->Background = Brushes::Transparent;
			}
			else
			{
				visual->scatterItem->IsEnabled = true;
				visual->scatterItem->Style = nullptr;
				visual->scatterItem->Effect = nullptr;
				visual->scatterItem->CanScale = true;
				visual->scatterItem->CanMove = true;

				visual->drawingContent->ClearValue ( Canvas::WidthProperty );
				visual->drawingContent->ClearValue ( Canvas::HeightProperty );
				visual->drawingContent->Children->Remove ( visual->border );
				visual->drawingContent->Children->Remove ( visual->stem );
				visual->scatterItem->ClearValue ( ScatterViewItem::WidthProperty );
				visual->scatterItem->ClearValue ( ScatterViewItem::HeightProperty );

				visual->drawingContent->HorizontalAlignment = System::Windows::HorizontalAlignment::Stretch;
				visual->drawingContent->VerticalAlignment = VerticalAlignment::Stretch;

				visual->scatterItem->SizeChanged += gcnew SizeChangedEventHandler ( visual, &DeviceVisual::item_SizeChanged );
			}
		}
	};


	/**
	* Make the DeviceVisual visible.
	*
	*/
	void DeviceVisual::Show ()
	{
		if ( visible )
			return;

		DeviceVisualShow ^ act = gcnew DeviceVisualShow ( this );

		Action ^ action = gcnew Action ( act, &DeviceVisualShow::Run );

		environs::Environs::dispatch ( action );
	}


	void DeviceVisual::item_SizeChanged ( Object ^ sender, SizeChangedEventArgs ^ e )
	{
		drawingContent->Width = e->NewSize.Width;
		drawingContent->Height = e->NewSize.Height;
	}


	/**
	* Update the center position.
	* Note: This method must be called within the UI context.
	*
	* @param		pt        A Point object
	*/
	void DeviceVisual::Position ( Point ^ pt )
	{
		currentPosition = pt;
		scatterItem->Center = *pt;
	}


	/**
	* Update the center position.
	* Note: This method must be called within the UI context.
	*
	* @param		pt        A Point object
	*/
	void DeviceVisual::Position ( int center_x, int center_y )
	{
		Point ^ pt = gcnew Point ( center_x, center_y );
		if ( scatterItem->Center.X != pt->X || scatterItem->Center.Y != pt->Y )
		{
			scatterItem->Center = *pt;
			currentPosition = pt;
		}
	}


	ref class DeviceVisualUpdatePosition
	{
		DeviceVisual ^ visual;
		Point ^ pt;
		double orientation;

	public:
		DeviceVisualUpdatePosition ( DeviceVisual ^ v, Point ^ p, double o ) : visual ( v ), pt ( p ), orientation ( o ) { }
		void Run () {
			visual->Position ( pt );
			visual->scatterItem->Orientation = ( double ) ( orientation );
		}
	};


	/**
	* Update the center position and orientation.
	*
	* @param		deviceInstance        A PortalInfo object.
	*/
	void DeviceVisual::UpdatePosition ( PortalInfoBase ^ info )
	{
		if ( info->centerX == currentPosition->X && info->centerY == currentPosition->Y && info->orientation == currentOrientation )
			return;
		CVerb ( "UpdatePosition: portal " + portal->portalID + " x:" + info->centerX + " y:" + info->centerY + " angle:" + info->orientation );

		Point ^ pt = gcnew Point ( info->centerX, info->centerY );
		currentPosition = pt;

		DeviceVisualUpdatePosition ^ act = gcnew DeviceVisualUpdatePosition ( this, pt, ( double ) ( info->orientation - 90 ) );

		Action ^ action = gcnew Action ( act, &DeviceVisualUpdatePosition::Run );

		environs::Environs::dispatch ( action );
	}


	/**
	* Update the orientation.
	* Note: This method must be called within the UI context.
	*
	* @param		pt        A Point object
	*/
	void DeviceVisual::Orientation ( float angle )
	{
		double newAngle = ( double ) angle - 90;

		if ( scatterItem->Orientation != newAngle )
		{
			scatterItem->Orientation = newAngle;
			currentOrientation = ( float ) newAngle;
		}
	}


	/**
	* Update the size.
	* Note: This method must be called within the UI context.
	*
	* @param		width        The new width.
	* @param		height       The new height.
	*/
	void DeviceVisual::Size ( int width, int height )
	{
		//if (isPortalReceiver)
		//{
		//    return;
		//}

		const int extensionWidth = 30;
		const int extensionHeight = ( int ) ( ( double ) extensionWidth * 2 );
		currentWidth = width;
		currentHeight = height;

		scatterItem->Width = width + extensionWidth;
		scatterItem->Height = height + extensionHeight;

		drawingContent->Height = scatterItem->Height;
		drawingContent->Width = scatterItem->Width;

		border->Height = scatterItem->Height;
		border->Width = scatterItem->Width;
		stem->X1 = 0;
		stem->X2 = 0;
		stem->Y1 = 0;
		stem->Y2 = scatterItem->Height / 2;
		Canvas::SetBottom ( stem, height / 2 + ( extensionHeight / 2 ) );
		Canvas::SetLeft ( stem, width / 2 + ( extensionWidth / 2 ) );
	}


	ref class DeviceVisualUpdateSize
	{
		DeviceVisual ^ visual;
		int width;
		int height;

	public:
		DeviceVisualUpdateSize ( DeviceVisual ^ v, int w, int h ) : visual ( v ), width ( w ), height ( h ) { }
		void Run () { visual->Size ( width, height ); }
	};


	/**
	* Update the size of the visual.
	*
	* @param		width        The new width.
	* @param		height       The new height.
	*/
	void DeviceVisual::UpdateSize ( int width, int height )
	{
		CVerb ( "UpdateSize: portal " + OBJ_p_cli ( portal, portalID ) );

		if ( currentWidth != width || currentHeight != height )
		{
			DeviceVisualUpdateSize ^ act = gcnew DeviceVisualUpdateSize ( this, width, height );

			Action ^ action = gcnew Action ( act, &DeviceVisualUpdateSize::Run );

			environs::Environs::dispatch ( action );
		}
	}

	/**
	* Update the size of the visual.
	* Note: This method must be called within the UI context.
	*
	* @param		info        A PortalInfo object with the desired size
	*/
	void DeviceVisual::UpdateSize ( PortalInfo ^ info )
	{
		CVerb ( "UpdateSize: portal " + OBJ_p_cli ( portal, portalID ) );

		if ( currentWidth != info->width || currentHeight != info->height )
		{
			DeviceVisualUpdateSize ^ act = gcnew DeviceVisualUpdateSize ( this, info->width, info->height );

			Action ^ action = gcnew Action ( act, &DeviceVisualUpdateSize::Run );

			environs::Environs::dispatch ( action );
		}
	}


	bool DeviceVisual::IsLocationOccludedByDevice ( int x, int y )
	{
		for each ( DeviceVisual ^ visual in deviceVisuals->Values )
		{
			int widthHalf = visual->portal->device->display.width / 2; // TODO calculate pixels of screen in relation to size
			int heightHalf = visual->portal->device->display.width / 2; // TODO calculate pixels of screen in relation to size

			if ( x < ( visual->currentPosition->X - widthHalf ) )
				return false;
			if ( x > ( visual->currentPosition->X + widthHalf ) )
				return false;
			if ( y < ( visual->currentPosition->Y - heightHalf ) )
				return false;
			if ( y > ( visual->currentPosition->Y + heightHalf ) )
				return false;
			return true;
		}

		return false;
	}



	void DeviceVisual::Renderer ( System::Windows::Media::Imaging::WriteableBitmap ^ bitmap )
	{
		drawingContent->Background = gcnew ImageBrush ( bitmap );
	}
}


#endif