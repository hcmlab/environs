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
#pragma once

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_LATENCY_INDICATOR_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_CPP_LATENCY_INDICATOR_H

#if (defined(CLI_PS) || defined(CLI_STT))


namespace environs 
{
	/**
	*	Latency Indicator
	*	---------------------------------------------------------
	*	Copyright (C) 2015 Chi-Tai Dang
	*   All rights reserved.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/

	public ref class LatencyIndicator DERIVE_DISPOSABLE
	{
	public:
		LatencyIndicator ();
		~LatencyIndicator ();

		void Release ();

		void Init ( System::Windows::Controls::TextBlock ^ textBlock );

		void Start ();

		void Stop ();

	internal:
		/// <summary>
		/// This bool flag is set by the Disposal process to reflect disposal of this object. It is used internally.
		/// </summary>
		bool disposed;

		System::Threading::Timer ^ latencyTimer;

		literal int countInterval = 33;
		literal int maxCount = 120;
		int counter;

		System::Windows::Controls::TextBlock ^ tbCounter;

		void LatencyCallback ( Object ^ stateInfo );
		void LatencyCallbackAction ();
	};
}


#endif

#endif