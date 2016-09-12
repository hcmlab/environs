
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
// Windows header files
#include <windows.h>

// C RunTime header files
#if !defined(WINDOWS_PHONE) && !defined(_WIN32)
#	include <stdlib.h>
#	include <malloc.h>
#endif
#include <memory.h>
#include <tchar.h>

// Additional headers
