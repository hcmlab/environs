/**
 * Utils and common stuff
 * --------------------------------------------------------------------
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
#ifndef INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H
#define INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H


namespace environs
{
	extern void refactorBuffer ( char * &curStart, char * bufferStart, unsigned int remainingMsg, char * &curEnd );
	extern char * LoadBinary ( const char * fileName, int * size );

	extern char * LoadPrivateBinary ( const char * fileName, int * size );
	extern bool SavePrivateBinary ( const char * fileName, const char * buffer, int size );

	extern unsigned int getRandomValue ( void * value );
	
	INCLINEFUNC INTEROPTIMEVAL GetEnvironsTickCount ();
    extern unsigned long long   GetUnixEpoch ();
    extern bool CreateDataDirectory ( char * dir );
	extern void CreateCopyString ( const char * src, char ** dest );
}



#endif	/// INCLUDE_HCM_ENVIRONS_UTILS_AND_TOOLS_H