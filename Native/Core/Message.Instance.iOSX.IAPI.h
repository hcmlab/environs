/**
 * MessageInstance for iOSX internal
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_INTERNAL_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_INTERNAL_H

#include "Environs.iOSX.Imp.h"
#import "Message.Instance.iOSX.h"

/**
 *	MessageInstance iOSX internal
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface MessageInstance (internal)
{
}

- (bool) SetInst : ( const sp ( environs::lib::MessageInstance ) & ) inst;

@end

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_IOSX_INTERNAL_H

