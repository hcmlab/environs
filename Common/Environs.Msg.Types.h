/**
 * Message type definitions
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
#ifndef _INCLUDE_HCM_ENVIRONS_MSG_TYPES_H
#define _INCLUDE_HCM_ENVIRONS_MSG_TYPES_H

#include "Interop.h"

/* Namespace: environs -> */

#ifdef __cplusplus
namespace environs
{
	namespace lib
    {
#endif
        
		NET_PACK_PUSH1

		typedef struct SensorFrame
		{
			// Static data initialized at construction time
			char 		preamble [ 3 ];
			char		version;

			// A value of type environs::SensorType
			int			type;

			// Increase with each frame
			int         seqNumber;

			// Dynamic data
			union
			{
				struct
                {
                    // Location: f1 = accuracy (latitude/longitude)
                    // Light: f1 = light in Lux
                    float		f1;
                    // Location: f2 = accuracy (longitude)
                    float		f2;
                    // Location: f3 = speed
					float		f3;
				}
				floats;

				struct
				{
					float		azimut;
					float		pitch;
					float		roll;
				}
				gravity;

				struct
				{
					float		x;
					float		y;
					float		z;
				}
				accel;
			}
			data;
        }
		NET_PACK_ALIGN SensorFrame;
        
        
        typedef struct SensorFrameExt
        {
            // Static data initialized at construction time
            char 		preamble [ 3 ];
            char		version;
            
            // A value of type environs::SensorType
            int			type;
            
            // Increase with each frame
            int         seqNumber;
            
            // Dynamic data
            union
            {
                struct
                {
                    // Location: f1 = accuracy (latitude/longitude)
                    // Light: f1 = light in Lux
                    float		f1;
                    // Location: f2 = accuracy (longitude)
                    float		f2;
                    // Location: f3 = speed
                    float		f3;
                }
                floats;
                
                struct
                {
                    float		azimut;
                    float		pitch;
                    float		roll;
                }
                gravity;
                
                struct
                {
                    float		x;
                    float		y;
                    float		z;
                }
                accel;
            }
            data;
            
            // Dynamic data
            struct
            {
                // Location: d1 = latitude
                double		d1;
                // Location: d2 = longitude
                double		d2;
                // Location: d3 = altitude
                double		d3;
            }
            doubles;
        }
		NET_PACK_ALIGN SensorFrameExt;
        
		NET_PACK_POP


		/*
		* Format: HEADER (12) DATA (Length)

		* STRUCT: Preamble   Length     Version  Type         SubType                  Msg pack
		* HEADER: 0 1 2 3    4 5 6 7    8        9		       10 11                    12 13 14 15
		* SEM:    M S G ;    int[4]     byte[1]  char[1]      char[2]                  int[4]
		* Type: 0 - Portal stream commands
		* SubType: 0 - Stop portal stream
		* SubType: 1 - Start portal stream
		*/
		typedef struct _ComMessageHeader
		{
			// Static data initialized at construction time
			char 			preamble [ 4 ];	// 4 bytes
			unsigned int	length;			// 4 bytes
			char			version;		// 1 byte
			char			type;			// 1 byte

			union
			{
				struct
				{
					char		subtype;
					char		packetTypeSpec;
				} _subtype;
				unsigned short	payloadType;
			} MessageType;

			union
			{
				int		sequenceNumber;
				int		fileID;
				int		portalID;
				int		payload;
				char	cstring;
			} MessagePack;

			unsigned int payload;
		}
		ComMessageHeader;

		//#define MSG_HEADER_LEN	16
#define MSG_HEADER_SIZE	(sizeof(environs::lib::ComMessageHeader) - sizeof(unsigned int))

		typedef struct _MessageHeaderPartitions
		{
			// Static data initialized at construction time
			char 			preamble [ 4 ];	// 4 bytes
			unsigned int	length;			// 4 bytes
			char			version;		// 1 byte
			char			type;			// 1 byte

			union
			{
				struct
				{
					char		subtype;
					char		packetTypeSpec;
				} _subtype;
				unsigned short	payloadType;
			} MessageType;

			int				fileID;

			unsigned int	parts;
			unsigned int	part;

			unsigned int	descriptorLength;
			unsigned int    payload;
		}
		MessageHeaderPartitions;

#define MSG_PARTITIONS_HEADER_SIZE	(sizeof(environs::lib::MessageHeaderPartitions) - sizeof(unsigned int))


		typedef struct _UdpMessageHeader
		{
			unsigned short	deviceID;
			unsigned short	version;
			unsigned short	payloadSize;
			unsigned short	payloadType;
			unsigned int	sequence;
			int				fileSize;
			unsigned short	fragments;
			unsigned short	fragment;
			int				portalID;
			unsigned int	payload;
		} UdpMessageHeader;

#define PKT_HEADER_SIZE	(sizeof(environs::lib::UdpMessageHeader) - sizeof(unsigned int))
        
#ifdef __cplusplus
	}
#endif

#ifndef CLI_CPP
#   ifdef __cplusplus
	class DeviceInstance;

	typedef struct SensorFrame
	{
		lib::SensorFrameExt	frame;

		DeviceInstance  *   device;
        
        void            *   devicePlatform;
	}
	SensorFrame;

    //typedef lib::SensorFrame SensorFrame;
#   endif
#endif


    
#ifdef __cplusplus
} /* namespace Environs */
#endif

#endif // _INCLUDE_HCM_ENVIRONS_MSG_TYPES_H








