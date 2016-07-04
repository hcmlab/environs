/**
 * FileInstance for iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H

#include "Environs.iOSX.Imp.h"

/**
 *	FileInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface FileInstance : NSObject
{
}

/**
 * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
 * A value of 0 indicates an invalid fileID.
 * */
@property (readonly, nonatomic) int fileID;

/**
 * Used internally.
 * */
@property (readonly, nonatomic) int type;

/**
 * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
 * */
@property (readonly) NSString * descriptor;

/**
 * sent is true if this FileInstance is data that was sent or received (false).
 * */
@property (readonly, nonatomic) bool sent;

/**
 * created is a posix timestamp that determines the time and date that this FileInstance
 * has been received or sent.
 * */
@property (readonly, nonatomic) unsigned long long created;

/**
 * The size in bytes of a buffer to send or data received.
 * */
@property (readonly, nonatomic) long size;

/**
 * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
 * */
@property (readonly) NSString * path;

/**
 * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
 * file or buffer that has already been sent.
 * If this value changes, then the corresponding device's DeviceObserver is notified
 * with this FileInstance object as the sender
 * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
 * */
@property (readonly, nonatomic) int sendProgress;

/**
 * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
 * file or buffer that has already been received.
 * If this value changes, then the corresponding device's DeviceObserver is notified
 * with this FileInstance object as the sender
 * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
 * */
@property (readonly, nonatomic) int receiveProgress;

/**
 * A reference to the DeviceInstance that is responsible for this FileInstance.
 * */
@property (readonly, nonatomic) id device;

@property (readonly) NSString * toString;

@property (readonly) NSString * GetPath;

@end

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H

