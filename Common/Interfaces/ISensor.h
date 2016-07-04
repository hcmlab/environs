/**
 * Sensor Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_ISENSOR_H
#define INCLUDE_HCM_ENVIRONS_ISENSOR_H

#include "IEnvirons.Base.h"
#include "Interface.Exports.h"
#include "Core/Callbacks.h"


namespace environs 
{
	/**
	*	Interface for Environs sensors
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( ISensor )

	public:
		/** Default constructor */
		ISensor ( ) :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Sensor ),

			/** Default initialization */
            hEnvirons ( 0 ), sensorType ( SensorType::All ), sensorID ( 0 ), initialized ( false ), started ( false ),
			state ( 0 )
		{};

		virtual ~ISensor ( ) {};

		/** IsValid verifies whether the current platform is valid, that is the platform that this tracker is designed for. */
		virtual bool		IsRuntimeSupported ( unsigned int platform ) = 0;

		/** Interface initializer. Do not override this method. Init () is called at the end of the Interface initializer */
		int					Init ( SensorType_t type ) {
								sensorType = type;  return Init ();
							}

		virtual int			Init ( ) = 0;
		virtual void		Release ( ) = 0;

		/**
		* Start is called by the framework in order to initialize the tracker and start the resources.
		*
		* @return	status	2 = postponed (initialization and start will be handled at a later time); 1 = success (initialized and started); 0 = failed, do not exclude this tracker now; 
		*/
		virtual int			Start ( ) { return 0; };
		virtual int			Stop ( ) { return 0; };

		virtual bool		Execute ( int command ) { return false; };

		virtual void *		Read ( int command ) { return 0; };

		virtual bool		Write ( int command ) { return false; };
						
        int                 hEnvirons;
		SensorType_t		sensorType;
		int					sensorID;
		bool                initialized;
        bool                started;
		int					state;

	protected:

		
	EndInterface


} /* namespace environs */

#endif // INCLUDE_HCM_ENVIRONS_ISENSOR_H
