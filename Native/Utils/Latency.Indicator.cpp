/**
* Environs CPP Latency Indicator
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

#include "Interop.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Cli.h"
#include "Latency.Indicator.h"
#include "Environs.Native.h"

using namespace System::Threading;
using namespace System::Windows::Controls;

#define	CLASS_NAME 	"Latency.Indicator. . . ."


namespace environs
{
	LatencyIndicator::LatencyIndicator ()
	{
		counter = 0;
		disposed = false;
		latencyTimer = nullptr;
		tbCounter = nullptr;
	}


	LatencyIndicator::~LatencyIndicator ()
	{
		if ( latencyTimer ) {
			delete latencyTimer;
			latencyTimer = nullptr;
		}

		tbCounter = nullptr;
	}


	void LatencyIndicator::Release ()
	{
		CVerb ( "LatencyIndicator.Dispose" );

		if ( !disposed )
		{
			// Managed resources
			if ( latencyTimer != nullptr )
			{
				latencyTimer->Dispose ( nullptr );
				latencyTimer = nullptr;
			}

			// Unmanaged resources
			disposed = true;
		}
	}


	void LatencyIndicator::Init ( TextBlock ^ textBlock )
	{
		if ( textBlock == nullptr )
		{
			CVerb ( "Invalid TextBlock initialization!" );
			return;
		}
		tbCounter = textBlock;

		if ( latencyTimer != nullptr )
			latencyTimer->Dispose ( nullptr );

		latencyTimer = gcnew System::Threading::Timer ( gcnew TimerCallback ( this, &LatencyIndicator::LatencyCallback ), nullptr, Timeout::Infinite, countInterval );
	}


	void LatencyIndicator::Start ()
	{
		if ( latencyTimer != nullptr )
		{
			counter = 0;
			latencyTimer->Change ( 0, 33 );
		}
	}


	void LatencyIndicator::Stop ()
	{
		if ( latencyTimer != nullptr )
			latencyTimer->Change ( Timeout::Infinite, 33 );
	}


	void LatencyIndicator::LatencyCallbackAction ()
	{
		tbCounter->Text = counter + "";
	}


	void LatencyIndicator::LatencyCallback ( Object ^ stateInfo )
	{
		counter++;

		if ( tbCounter != nullptr )
		{
			Environs::dispatch ( gcnew Action ( this, &LatencyIndicator::LatencyCallbackAction ) );
		}

		if ( counter >= maxCount )
			counter = 0;
	}
}


#endif