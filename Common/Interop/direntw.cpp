/**
* Dirent wrapper that includes either dirent.h (POSIX)
* or provides wrapping around win32 / cpp/cli
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
#include "stdafx.h"

#ifdef _WIN32

#include <direntw.h>
#include <io.h>
#include <string.h>

#if !defined(WINDOWS_PHONE) && !defined(_WIN32)
#	include <stdlib.h>
#endif

#ifdef CLI_CPP
using namespace System::IO;
#endif


PUBLIC_STRUCT DIR
{
	String_ptr			pattern;

#ifdef CLI_CPP
	array<String^> ^	files;
	int					hFind;
#else
	intptr_t			hFind;
	struct _finddata_t	dFind;
#endif

	STRUCT dirent		item;
};


DIR OBJ_ptr opendir ( CString_ptr name )
{
	if ( name == nill || CString_ptr_empty ( name ) )
		return nill;

	bool success = false;
	DIR OBJ_ptr dir = nill;

	size_t length = CString_length ( name );

#ifdef CLI_CPP
	dir = gcnew ( DIR );
#else
	dir = (DIR *) calloc ( 1, sizeof ( DIR ) );
#endif

	if ( dir != nill )
	{
		// Build search pattern
#ifdef CLI_CPP
		dir->hFind = -1;
		dir->pattern = name;

		dir->files = Directory::GetFiles ( name );

		if ( dir->files != nill && dir->files->Length > 0 )
		{
			dir->item.d_type = DT_REG;
			success = true;
		}
#else
		dir->pattern = (char *) malloc ( length + 4 );
		if ( dir->pattern )
		{
			memcpy ( dir->pattern, name, length );

			char last = dir->pattern [ length - 1 ];

			if ( last != '/' && last != '\\' )
			{
				dir->pattern [ length ] = '/'; length++;
			}
			dir->pattern [ length ] = '*';
			dir->pattern [ length + 1 ] = 0;

			dir->hFind = _findfirst ( dir->pattern, &dir->dFind );
			if ( dir->hFind != -1 )
			{
				memset ( &dir->item, 0, sizeof ( dir->item ) );
				success = true;
			}
		}
#endif
	}

	if ( !success && dir != nill )
	{
#ifdef CLI_CPP
#else
		if ( dir->pattern )
			free ( dir->pattern );
		free ( dir );
#endif
		dir = nill;
	}
	return dir;
}


int closedir ( DIR OBJ_ptr dir )
{
	int success = -1;

	if ( dir != nill )
	{
#ifdef CLI_CPP
		dir->pattern = nill;
		dir->files = nill;
#else
		if ( dir->hFind != -1 )
			success = _findclose ( dir->hFind );

		if ( dir->pattern )
			free ( dir->pattern );
		free ( dir );
#endif
	}

	return success;
}


STRUCT dirent OBJ_ptr readdir ( DIR OBJ_ptr dir )
{
	if ( dir == nill C_Only ( || dir->hFind == -1 ) )
		return nill;

	STRUCT dirent OBJ_ptr nextDir = nill;

#ifdef CLI_CPP
	int next = dir->hFind + 1;

	if ( next < dir->files->Length )
	{
		dir->item.d_name = dir->files [ next ];
		dir->item.d_namlen = dir->item.d_name->Length;
		dir->hFind = next;

		nextDir = dir->item;
	}
	else {
		dir->hFind = -1;
	}
#else

	if ( !dir->item.d_name || _findnext ( dir->hFind, &dir->dFind ) != -1 )
	{
		nextDir         = &dir->item;
		nextDir->d_name = dir->dFind.name;

		if ( dir->dFind.attrib & _A_SUBDIR )
			nextDir->d_type = DT_DIR;
		else
			nextDir->d_type = DT_REG;
	}
#endif

	return nextDir;
}

#endif

