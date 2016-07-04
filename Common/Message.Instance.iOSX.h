/**
 * MessageInstance for iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_H

#include "Environs.iOSX.Imp.h"

#   if (!defined(DISABLE_ENVIRONS_OBJC_API))

/**
 *	MessageInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface MessageInstance : NSObject
{
}


/**
 * sent is true if this MessageInstance is data that was sent or received (false).
 * */
@property (readonly, nonatomic) bool sent;

/**
 * created is a posix timestamp that determines the time and date that this MessageInstance
 * has been received or sent.
 * */
@property (readonly, nonatomic) unsigned long long created;

/**
 * The length of the text message in bytes (characters).
 * */
@property (readonly, nonatomic) int length;

/**
 * The text message.
 * */
@property (readonly) NSString * text;

/**
 * Determins the type of connection (channel type) used to exchange this message.
 * c = in connected state
 * d = in not connected state through a direct device to device channel.
 * m = in not connected state by means of a Mediator service.
 * */
@property (readonly, nonatomic) char connection;

/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
@property (readonly) id device;

@property (readonly) NSString * toString;
@property (readonly) NSString * shortText;

@end

#   endif

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_H

