/**
 *	Platform interop socket inclusion
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
#include "Interop.h"
 
#if defined(_WIN32)

//#include <ws2ipdef.h>
#include <ws2tcpip.h>

#define LogSocketError()					CWarnArg("SocketError: %d",WSAGetLastError())
#define VerbLogSocketError()				CVerbArg("SocketError: %d",WSAGetLastError())
#define SOCK_IN_PROGRESS					(WSAGetLastError() == WSAEWOULDBLOCK)

#else

// -> POSIX includes
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#ifndef ANDROID
// -> Linux/iOS includes
#include <ifaddrs.h>

//#ifndef __APPLE__
//size_t strlcpy ( char *d, char const *s, size_t n )
//{
//	return snprintf ( d, n, "%s", s );
//}
//#endif
#endif

#define LogSocketError()					CWarnArg("SocketError: %s",strerror(errno))
#define SOCK_IN_PROGRESS					(errno == EINPROGRESS || errno == EALREADY)
#define SOCK_CONNECTED						(errno == EISCONN)
#define SOCK_CON_REFUSED					(errno == ECONNREFUSED)

#define closesocket(s) 						close(s)
#define WSACleanup()
#define WSAData								void *
#define WSAStartup(a,b)						((*b = 0) == (WSAData)1)
#endif


#endif // INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
