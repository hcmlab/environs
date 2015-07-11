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
#include "stdio.h"

#ifndef INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H

#define	MAX_TOUCH_VISUALS	10


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
			triggerTouchCount ( 0 ), deviceBase ( 0 )
		{};

		virtual ~IInputRecognizer ( ) {};
        
        
        void            *   deviceBase;

		/** Init
		*	@param	device_width
		*	@param	device_height
		*	@return	bool
		*/
		bool Init ( void * deviceBase, DeviceDisplay * display )
		{
            this->deviceBase = deviceBase;
            
            if ( display ) {
                this->display.width = display->width;
                this->display.height = display->height;
                this->display.width_mm = display->width_mm;
                this->display.height_mm = display->height_mm;
            }
			return Init ();
		};

		virtual bool Init () = 0;


		/** RecognizeGestures
		*	@param	void
		*	@return	bool
		*/
        int                 triggerTouchCount;
		virtual int			Trigger ( InputPackRec **	inputs, int inputCount ) = 0;

        virtual int			Perform ( InputPackRec **	inputs, int inputCount ) = 0;
        
        virtual void		Flush () {};

		virtual bool		SetIncomingPortalID ( int portalID ) { return false; };

	protected:
        DeviceDisplay       display;
	};

} /* namespace environs */


#endif /// -> INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H


