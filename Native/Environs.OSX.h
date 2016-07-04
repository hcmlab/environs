/**
 * Environs OSX platform specific
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
#ifndef INCLUDE_HCM_ENVIRONS_IOS_COMMON_H
#define INCLUDE_HCM_ENVIRONS_IOS_COMMON_H

#include "Environs.Platforms.h"

#if defined(ENVIRONS_OSX)

#import <Security/Security.h>

#ifdef __cplusplus

namespace environs
{
    
    void            DetectSDKs ( );
    void            DetectPlatform ( );
    
    bool            DetermineAndInitWorkDir ();
    bool            AllocNativePlatform ();
    
    void            InitIOSX ();
        
    void            UpdateDeviceParams ();
    
    void            SetRenderSurfaceIOSX ( void * surface );
    
    OSStatus        EncryptMessageX ( SecKeyRef publicKey, unsigned int certProp, char * msg, size_t msgLen, char * ciphers, size_t *ciphersLen );

    bool            RenderDecoderToSurface ( void * surface, void * decoderOrByteBuffer );
    bool            RenderImageToSurface ( void * surface, void * byteBuffer );
    
    bool            SetUseEncoderIOSX ( const char * moduleName );
    bool            SetUseDecoderIOSX ( const char * moduleName );
    
	namespace API
    {
        const char * GetSSID ( bool desc = false );

	} /* namespace API */

} /* namespace environs */

#endif

#endif


#endif  /// end-INCLUDE_HCM_ENVIRONS_IOS_COMMON_H




