/**
 * Portal Info Object
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINFO_OBJECT_H
#define INCLUDE_HCM_ENVIRONS_PORTALINFO_OBJECT_H

#include "Interop.h"
#include "Interop/Smart.Pointer.h"
#include "Portal.Info.Base.h"

#ifndef CLI_CPP
#	include "Interfaces/IPortal.Info.h"
#endif


/* Namespace: environs -> */
namespace environs
{
#ifndef CLI_CPP
    namespace lib {
        class PortalInstance;
    }
#endif
    
	/**
	*	A PortalInfo object serves as container for portal information.
    *   Environs makes use of such objects to get/set portal details.
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	current size is (4 + 4 + 4 + 4 + 4) = 20 bytes
	* ****************************************************************************************
	*/
	PUBLIC_CLASS PortalInfo DERIVE_c_only ( environs::IPortalInfo ) DERIVE_DISPOSABLE
	{
        public:
		PortalInfoBase  base;
        
        ENVIRONS_LIB_API PortalInfo ( );
        ENVIRONS_LIB_API ~PortalInfo ( ) {};
        
        int hEnvirons;

#ifdef CLI_CPP
		property int width { int get () { return base.width; }};
		property int height { int get () { return base.height; }};
#endif

		CLIBSPACE PortalInstance OBJ_ptr portal;

		ENVIRONS_LIB_API STRING_T toString ();

#ifdef CLI_CPP
		ENVIRONS_LIB_API CLI_VIRTUAL STRING_T ToString ( ) CLI_OVERRIDE;
#endif
        ENVIRONS_LIB_API void NotifyObservers ( environs::Notify::Portale_t notification );
        
        ENVIRONS_LIB_API void SetSize ( int width, int height );
        
        ENVIRONS_LIB_API void SetOrientation ( float angle );
        
        ENVIRONS_LIB_API void SetLocation ( int centerX, int centerY );
        ENVIRONS_LIB_API void SetLocation ( int centerX, int centerY, float angle );
        
        ENVIRONS_LIB_API void Set ( int centerX, int centerY, float angle, int width, int height );
        
        
        ENVIRONS_LIB_API bool Update ( environs::Notify::Portale_t notification, PortalInfoBase OBJ_ptr info ) ;
	};

} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTALINFO_OBJECT_H
