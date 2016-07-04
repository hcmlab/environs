/**
 *	Interface for a gesture recognizer working on touch contacts 
    on mobile devices.
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
#include "Interface.Exports.h"
#include "IEnvirons.Ident.h"
#include "Human.Input.Decl.h"
#include "Device.Display.Decl.h"

#ifndef WINDOWS_PHONE
#	include "stdio.h"
#endif


#ifndef INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H

#define	MAX_TOUCH_VISUALS	10


#ifdef __cplusplus

namespace environs 
{
	/**
	* IInputRecognizer
	*
	*/
	class IInputRecognizer : public IEnvironsIdent
	{
	public:
		/** Constructor */
		IInputRecognizer () :
			/** Base class initialization */
			IEnvironsIdent ( InterfaceType::InputRecognizer ),
			/** Default initialization */
			deviceBase ( 0 ), triggerTouchCount ( 0 ), display ( DeviceDisplay() )
		{};

		virtual ~IInputRecognizer ( ) {};
        
        
        void            *   deviceBase;

		/** Init
		*	@param	device_width
		*	@param	device_height
		*	@return	bool
		*/
		bool Init ( void * deviceBasea, DeviceDisplay * displaya )
		{
            deviceBase = deviceBasea;
            
            if ( displaya ) {
                display.width = displaya->width;
                display.height = displaya->height;
                display.width_mm = displaya->width_mm;
                display.height_mm = displaya->height_mm;
            }
			return Init ();
		};

		virtual bool Init () = 0;


		/** RecognizeGestures
		*	@param	void
		*	@return	bool
		*/
        int                 triggerTouchCount;
		virtual int			Trigger ( environs::lib::InputPackRec **	inputs, int inputCount ) = 0;

        virtual int			Perform ( environs::lib::InputPackRec **	inputs, int inputCount ) = 0;
        
        virtual void		Flush () {};

		virtual bool		SetIncomingPortalID ( int portalID ) { return false; };

	protected:
        DeviceDisplay       display;
	};

} /* namespace environs */

#endif

#endif /// -> INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H


