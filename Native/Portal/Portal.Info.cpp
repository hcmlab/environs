/**
 * A PortalInfo object serves as container for portal information.
 * Environs makes use of such objects to get/set portal details.
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

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifndef CLI_CPP
#	include "Interop.h"
#	include "Interop/Smart.Pointer.h"
#	include "Environs.Platforms.h"

#	include "Environs.Obj.h"
#endif

#	ifdef CLI_CPP
#		include "Environs.Cli.Forwards.h"
#		include "Environs.Native.h"
#		include "Environs.Utils.h"

#		include "Environs.Cli.h"
#		include "Environs.Lib.h"
#		include "Portal.Instance.Cli.h"
#	else
#		include "Environs.Observer.h"
#		include "Portal.Instance.h"

		using namespace std;
#	endif


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Info. . . . . . ."


namespace environs 
{
	PortalInfo::PortalInfo ( )
	{
		CVerb ( "Construct" );
        
		C_Only ( Zero ( base ); )
       
        hEnvirons	= 0;
		portal		= nill;
    }
    
    
    void PortalInfo::NotifyObservers ( environs::Notify::Portale_t notification )
    {
        CVerb ( "NotifyObservers" );

		CLIBSPACE PortalInstance OBJ_ptr portalInstance = portal;

		if ( portalInstance != nill )
			portalInstance->NotifyObservers ( notification );
    }
    
    
    void PortalInfo::SetSize ( int width, int height )
    {
        base.flags = 0;
        base.width = width;
        base.height = height;
        base.flags |= PORTAL_INFO_FLAG_SIZE;

		CLIBSPACE PortalInstance::SetPortalInfo ( hEnvirons, Addr_of ( base ) );
    }
    

    void PortalInfo::SetOrientation ( float angle )
    {
        base.flags = 0;
        base.orientation = angle;
        base.flags |= PORTAL_INFO_FLAG_ANGLE;

		CLIBSPACE PortalInstance::SetPortalInfo ( hEnvirons, Addr_of ( base ) );
    }
    

    void PortalInfo::SetLocation ( int centerX, int centerY )
    {
        base.flags = 0;
        base.centerX = centerX;
        base.centerY = centerY;
		base.flags |= PORTAL_INFO_FLAG_LOCATION;

		CLIBSPACE PortalInstance::SetPortalInfo ( hEnvirons, Addr_of ( base ) );
    }
    

    void PortalInfo::SetLocation ( int centerX, int centerY, float angle )
    {
        base.flags = 0;
        base.centerX = centerX;
        base.centerY = centerY;
        base.orientation = angle;
		base.flags = PORTAL_INFO_FLAG_ANGLE | PORTAL_INFO_FLAG_LOCATION;

		CLIBSPACE PortalInstance::SetPortalInfo ( hEnvirons, Addr_of ( base ) );
    }
    

    void PortalInfo::Set ( int centerX, int centerY, float angle, int width, int height )
    {
        base.flags = 0;
        base.centerX = centerX;
        base.centerY = centerY;
        base.orientation = angle;
        base.width = width;
        base.height = height;
		base.flags = PORTAL_INFO_FLAG_LOCATION | PORTAL_INFO_FLAG_ANGLE | PORTAL_INFO_FLAG_SIZE;

		CLIBSPACE PortalInstance::SetPortalInfo ( hEnvirons, Addr_of ( base ) );
    }
    
    
	STRING_T PortalInfo::toString ()
    {
#ifdef CLI_CPP
		StringBuilder ^ sbl = gcnew StringBuilder ( "Portal: center coordinates [ " );

		sbl->Append ( base.centerX )->Append ( " / " )->Append ( base.centerY )
			->Append ( " ], size [ " )->Append ( base.width )->Append ( " / " )
			->Append ( base.height )->Append ( " ], orientation [ " )->Append ( base.orientation )->Append ( " ]" );

		return sbl->ToString ();
#else            
		char buffer [ 256 ];

		if ( snprintf ( buffer, 256, "Portal: center coordinates [ %i / %i ], size [ %i / %i ], orientation [ %f ]",
			base.centerX, base.centerY, base.width, base.height, base.orientation ) < 0 )
			*buffer = 0;

		return string ( buffer );
#endif
    }


#ifdef CLI_CPP
	STRING_T PortalInfo::ToString ()
	{
		return toString ();
	}
#endif
    
    
    bool PortalInfo::Update ( environs::Notify::Portale_t notification, PortalInfoBase OBJ_ptr src )
    {
        bool changedl = false;
        bool changeds = false;

        if (base.portalID != src->portalID && src->portalID > 0)
            base.portalID = src->portalID;
        
        if (notification == Notify::Portale::Zero || notification == Notify::Portale::LocationChanged )
        {
            if (base.centerX != src->centerX)
            {
                changedl = true;
                base.centerX = src->centerX;
            }
            
            if (base.centerY != src->centerY)
            {
                changedl = true;
                base.centerY = src->centerY;
            }
            
            if (base.orientation != src->orientation)
            {
                changedl = true;
                base.orientation= src->orientation;
            }
            
            if (changedl)
                NotifyObservers ( Notify::Portale::LocationChanged );
        }
        
        if (notification == Notify::Portale::Zero || notification == Notify::Portale::SizeChanged )
        {
            if (base.width != src->width)
            {
                changeds = true;
                base.width= src->width;
            }
            
            if (base.height != src->height)
            {
                changeds = true;
                base.height= src->height;
            }
            
            if (changeds)
                NotifyObservers ( Notify::Portale::SizeChanged );
        }
        return (changedl | changeds);
    }
    
} /// -> namespace environs



