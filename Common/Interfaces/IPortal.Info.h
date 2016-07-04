/**
 * Portal Info Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINFO_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_PORTALINFO_INTERFACE_H

#include "Interop/Smart.Pointer.h"
#include "Portal.Info.Base.h"
#include <string>


/* Namespace: environs -> */
namespace environs
{
	/**
	*	A PortalInfo object serves as container for portal information.
    *   Environs makes use of such objects to get/set portal details.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	current size is (4 + 4 + 4 + 4 + 4) = 20 bytes
	* ****************************************************************************************
	*/
	class IPortalInfo
	{
        public:
		PortalInfoBase  base;

        IPortalInfo () : base ( PortalInfoBase () ) {};
        virtual ~IPortalInfo ( ) {};

        virtual std::string toString ( ) = 0;
        
        virtual void NotifyObservers ( environs::Notify::Portale_t notification ) = 0;
        
        virtual void SetSize ( int width, int height ) = 0;
        
        virtual void SetOrientation ( float angle ) = 0;
        
        virtual void SetLocation ( int centerX, int centerY ) = 0;
        virtual void SetLocation ( int centerX, int centerY, float angle ) = 0;
        
        virtual void Set ( int centerX, int centerY, float angle, int width, int height ) = 0;
        
        
        virtual bool Update ( environs::Notify::Portale_t notification, PortalInfoBase * info ) = 0;
	};

} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTALINFO_INTERFACE_H
