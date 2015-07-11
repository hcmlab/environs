/**
 * Raw image tracker interface
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
#ifndef INCLUDE_HCM_ENVIRONS_ITRACKER_RAWIMAGE_H
#define INCLUDE_HCM_ENVIRONS_ITRACKER_RAWIMAGE_H

#include "IEnvirons.Base.h"
#include "Interface.Exports.h"
#include "Core/Callbacks.h"


#define	ENVIRONS_TRACKER_MAX	10

namespace environs 
{
	/**
	*	Interface for a raw image tracker
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( ITracker )

	public:
		/** Default constructor */
		ITracker ( ) :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Tracker ),

			/** Default initialization */
			dataSize ( 0 ), width ( 0 ), height ( 0 ), channels ( 0 ), stride ( 0 ) 
		{};

		virtual ~ITracker ( ) {};

		/** IsValid verifies whether the current platform is valid, that is the platform that this tracker is designed for. */
		virtual bool							IsRuntimeSupported ( unsigned int platform, unsigned int sdks ) = 0;

		/** Interface initializer. Do not override this method. Init () is called at the end of the Interface initializer */
		int										Init ( unsigned int Width, unsigned int Height, unsigned int Channels, unsigned int Stride ) {
													width = Width;
													height = Height;
													channels = Channels;
													stride = Stride;

													return Init ();
												}

		virtual int								Init ( ) = 0;
		virtual void							Release ( ) = 0;

		/**
		* Start is called by the framework in order to initialize the tracker and start the resources.
		*
		* @return	status	2 = postponed (initialization and start will be handled at a later time); 1 = success (initialized and started); 0 = failed, do not exclude this tracker now; 
		*/
		virtual int								Start ( ) { return 0; };
		virtual int								Stop ( ) { return 0; };

		virtual void							ReleaseResources () = 0;
		virtual int								AllocateResources ( ) = 0;

		virtual bool							Execute ( int command ) { return false; };

		/**
		* Tracking.
		*
		* @param	RenderDimensions
		*
		* @return	status	1 = success; 0 = failed this time, error status is recoverable, so try again with next round; -1 = failed, skip this plugin for further procesing
		*/
		virtual int								Perform ( void * rawImage, unsigned int size ) = 0;
				
		unsigned int 							dataSize;
		unsigned int 							width;
		unsigned int 							height;
		unsigned int 							channels;
		unsigned int 							stride;

	protected:

		
	EndInterface


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_ITRACKER_RAWIMAGE_H
