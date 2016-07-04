/**
 * iOS Camera capture
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
#import <Foundation/Foundation.h>

#ifdef ENVIRONS_IOS
#import <UIKit/UIKit.h>
#endif

#import <AVFoundation/AVFoundation.h>

#include "Interfaces/IPortal.Capture.h"
#include "Cam.Base.h"

@class iOSCam;

namespace environs
{
	/**
     *	iOS camera capture module
     *
     *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
     *	@version	1.0
     *	@remarks
     * ****************************************************************************************
     */
	class IOSCam : implements CamBase
	{
	public:
		IOSCam ( );
        ~IOSCam ( );
        
        int                         PreInit ( );
		int							Init ( );
        
		void						Release ( );
        
		int							Start ( );
		int							Stop ( );
        
	private:
        iOSCam                  *   camera;
        
	protected:
	};
}


@interface iOSCam : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
@public
    bool                    disposed;
    bool                    capturing;
    
    AVCaptureSession    *   session;
    AVCaptureDevice     *   camera;
    environs::IOSCam    *   capture;
    
    int                     deviceID;
    int                     camNumber;
}

- (bool) PreInitCamera:(int) cameraType;
- (bool) InitCamera;

- (AVCaptureDevice *) GetCameraDevice:(int) cameraType;

- (void) SetCapture:(environs::IOSCam *) newCapture;

- (bool) Start;
- (void) Stop;

@end

