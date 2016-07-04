/**
 * Human input declarations and data structures
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
#ifndef INCLUDE_HCM_ENVIRONS_HUMAN_INPUT_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_HUMAN_INPUT_DECLARATIONS_H_

#ifndef CLI_CPP

#include "Environs.Types.h"
#include "Environs.Build.Opts.h"

#ifndef USE_INPUT_PACK_STDINT_CPP


 /** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
	namespace lib
	{
#endif

#ifndef PI
#define PI 3.14159265
#endif

#define FRAME_ALIVE				0x00
#define	FRAME_INIT				0x01
#define FRAME_ADD				0x02
#define	FRAME_FLUSH				0x10


		/**
		* InputFrame layout used for transport
		*/
		typedef struct _InputFrame
		{
			// Static data initialized at construction time
			char 			preamble [ 3 ];
			char			version;

			// Static data initialized at construction time
			int				deviceID;

			// Increase with each new frame
			int				frameNumber;

			// An application specific identifier
			int				appOrPortalID;

			// Dynamic data
			short			size;
			char			frameState;
			unsigned char	count;
		}
		InputFrame;


		/**
		* InputPack layout V4 used for UDP transport
		*/
		typedef struct _InputPackRaw
		{
			// An id that identifies this input entity
			// In case of the stream: this id is used by the particular device and identifies the item within the device
			// In case of environs native layer: this id identifies the item for its whole lifecycle
			int				id;

			// A state declared by INPUT_STATE_* or INPUTSOURCE_COMMAND_*
			short			state;

			// A type declared by INPUT_TYPE_*
			char			type;

			bool			valid;

			int				x;
			int				y;  // V2 ends here
			int				value;

			float           angle;
			float			size;

			float			axisx;
			float			axisy;	// 9 ints        
		}
		InputPackRaw;


		/**
		* InputPack layout V4 used in platform layer
		*/
		typedef struct _InputPack
		{
			int             uniqueID;
			InputPackRaw    raw;
		}
		InputPack;


		/**
		* InputPack layout V4 used for gesture recognizer
		*/
		typedef struct _InputPackRec
		{
			int             uniqueID;
			InputPackRaw    raw;
			char            cache [ 24 ]; // additional memory reserved for gesture recognizers
		}
		InputPackRec;

#define INPUTPACK_V3_SIZE				sizeof(environs::lib::InputPackRaw)
#define INPUTPACK_V2_SIZE				(INPUTPACK_V3_SIZE - (5 * sizeof(float)))
#define INPUTPACK_V3_INT_SIZE			(INPUTPACK_V3_SIZE / 4)
#define INPUTPACK_V2_INT_SIZE			(INPUTPACK_V2_SIZE / 4)

#define INPUTPACK_X_TO_END_SIZE         ( INPUTPACK_V3_SIZE - (2 * sizeof(int) ) )
#define INPUTPACK_STATE_TO_END_SIZE     ( INPUTPACK_V3_SIZE - sizeof(int) )
#define INPUTPACK_START_IN_INPUTFRAME	sizeof(InputFrame)


		/**
		* Human input memory layout and object used within Environs
		*/
		typedef struct Input
		{
			InputPack	pack;

			bool		idHandlerManaged;
			int			flags;

			int			x_raw;	// Will be used as PID for Surface1 injection
			int			y_raw;	// Will be used as HWND for Surface1 injection

			int			x_old;
			int			y_old;

#ifdef __cplusplus
			Input () { }

			Input ( InputPackRaw * in, int ints )
			{
				pack.uniqueID = 0;

				int * src = ( int * ) in;
				int * dst = ( int * ) &pack.raw;

				while ( ints-- )
					*dst++ = *src++;

				pack.raw.valid = true;
				pack.raw.state = INPUT_STATE_ADD;

				idHandlerManaged = false;
			}

			bool Update ( InputPackRaw * in, int ints )
			{
				int * src = ( int * ) in;
				int * dst = ( int * ) &pack.raw;

				bool changed = false;
				pack.raw.valid = in->valid;

				while ( ints-- ) {
					if ( *dst != *src ) {
						changed = true;
						*dst = *src;
					}
					dst++; src++;
				}
				return changed;
			}

			void Copy ( InputPackRaw * in, int ints )
			{
				int * src = ( int * ) in;
				int * dst = ( int * ) &pack.raw;

				while ( ints-- )
					*dst++ = *src++;
			}
#endif
		}
		Input;


		/** Place declarations to global namespace for plain C */
#ifdef __cplusplus
	}

	typedef lib::Input		Input;
	typedef lib::InputPack	InputPack;
}
#endif

#else
#include "Interop.h"
#include <stdint.h>
#include <cstring>

/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
	namespace lib
	{
#endif

#ifndef PI
#define PI 3.14159265
#endif

#define FRAME_ALIVE				0x00
#define	FRAME_INIT				0x01
#define FRAME_ADD				0x02
#define	FRAME_FLUSH				0x10


		/**
		* InputFrame layout used for transport
		*/
		NET_PACK_PUSH1
		typedef struct _InputFrame
		{
			// Static data initialized at construction time
			int8_t 			preamble [ 3 ];
			int8_t			version;

			// Static data initialized at construction time
			int32_t			deviceID;

			// Increase with each new frame
			int32_t			frameNumber;

			// An application specific identifier
			int32_t			appOrPortalID;

			// Dynamic data
			int16_t			size;
			int8_t			frameState;
			uint8_t			count;
		}
		NET_PACK_ALIGN InputFrame;
		NET_PACK_POP

		/**
		* InputPack layout V2 used for UDP transport
		*/
		NET_PACK_PUSH1
		typedef struct _InputPackRawV2
		{
			// An id that identifies this input entity
			// In case of the stream: this id is used by the particular device and identifies the item within the device
			// In case of environs native layer: this id identifies the item for its whole lifecycle
			int32_t			id;

			// A state declared by INPUT_STATE_* or INPUTSOURCE_COMMAND_*
			int16_t			state;

			// A type declared by INPUT_TYPE_*
			int8_t			type;

			int8_t			valid;

			int32_t			x;
			int32_t			y;  // V2 ends here    
		}
		NET_PACK_ALIGN InputPackRawV2;
		NET_PACK_POP

		/**
		 * InputPack layout V4 used for UDP transport
		 */
		NET_PACK_PUSH1
		typedef struct _InputPackRaw
		{
			// An id that identifies this input entity
			// In case of the stream: this id is used by the particular device and identifies the item within the device
			// In case of environs native layer: this id identifies the item for its whole lifecycle
			int32_t			id;

			// A state declared by INPUT_STATE_* or INPUTSOURCE_COMMAND_*
			int16_t			state;

			// A type declared by INPUT_TYPE_*
			int8_t			type;

			int8_t			valid;

			int32_t			x;
			int32_t			y;  // V2 ends here    

			// V3
			int32_t			value;

			float32_t		angle;
			float32_t		size;

			float32_t		axisx;
			float32_t		axisy;	// 9 ints        
		}
		NET_PACK_ALIGN InputPackRaw;
		NET_PACK_POP

		/**
		 * InputPack layout V4 used in platform layer
		 */
		typedef struct _InputPack
		{
			int             uniqueID;
			
			union
			{
				InputPackRaw	raw;
				InputPackRawV2	rawV2;
			};
		}
		InputPack;


		/**
		 * InputPack layout V4 used for gesture recognizer
		 */
		typedef struct _InputPackRec
		{
			int             uniqueID;
			InputPackRaw    raw;
			char            cache [ 24 ]; // additional memory reserved for gesture recognizers
		}
		InputPackRec;

#define INPUTPACK_V3_SIZE				sizeof(environs::lib::InputPackRaw)
#define INPUTPACK_V2_SIZE				sizeof(environs::lib::InputPackRawV2)

#define INPUTPACK_X_TO_END_SIZE         ( INPUTPACK_V3_SIZE - (2 * sizeof(int32_t) ) )
#define INPUTPACK_STATE_TO_END_SIZE     ( INPUTPACK_V3_SIZE - sizeof(int32_t) )
#define INPUTPACK_START_IN_INPUTFRAME	sizeof(InputFrame)


		/**
		 * Human input memory layout and object used within Environs
		 */
		typedef struct Input
		{
			InputPack	pack;

			bool		idHandlerManaged;
			int			flags;

			int			x_raw;	// Will be used as PID for Surface1 injection
			int			y_raw;	// Will be used as HWND for Surface1 injection

			int			x_old;
			int			y_old;

#ifdef __cplusplus
			Input ( ) : pack ( InputPack() ), idHandlerManaged ( false ), flags ( 0 ), x_raw ( 0 ), y_raw ( 0 ), x_old ( 0 ), y_old ( 0 )
			{ }

			Input ( InputPackRaw * in, unsigned int size ) : flags ( 0 ), x_raw ( 0 ), y_raw ( 0 ), x_old ( 0 ), y_old ( 0 )
			{
				pack.uniqueID = 0;

				if ( size >= INPUTPACK_V3_SIZE )
				{
					pack.raw = *in;
				}
				else if ( size >= INPUTPACK_V2_SIZE )
				{
					pack.raw = InputPackRaw ( );
					pack.rawV2 = *( reinterpret_cast < InputPackRawV2 * > ( in ) );
				}
				else
				{
					pack.raw = InputPackRaw ( );
				}

				pack.raw.valid = true;
				pack.raw.state = INPUT_STATE_ADD;

				idHandlerManaged = false;
			}


			bool Update ( InputPackRaw * in, unsigned int size )
			{
				bool changed;

				if ( size >= INPUTPACK_V3_SIZE )
				{
					pack.raw.valid = in->valid;

					changed = ( std::memcmp ( &pack.raw, in, INPUTPACK_V3_SIZE ) != 0 );

					pack.raw = *in;
				}
				else if ( size >= INPUTPACK_V2_SIZE )
				{
					pack.raw.valid = in->valid;

					changed = ( std::memcmp ( &pack.raw, in, INPUTPACK_V2_SIZE ) != 0 );

					pack.raw = InputPackRaw ( );
					pack.rawV2 = *( reinterpret_cast < InputPackRawV2 * > ( in ) );
				}
				else
				{
					changed = false;

					pack.raw = InputPackRaw ( );
				}

				return changed;
			}

			void Copy ( InputPackRaw * in, unsigned int size )
			{
				if ( size >= INPUTPACK_V3_SIZE )
				{
					pack.raw = *in;
				}
				else if ( size >= INPUTPACK_V2_SIZE )
				{
					pack.raw = InputPackRaw ( );
					pack.rawV2 = *( reinterpret_cast < InputPackRawV2 * > ( in ) );
				}
				else
				{
					pack.raw = InputPackRaw ( );
				}
			}
#endif
		}
		Input;


		/** Place declarations to global namespace for plain C */
#ifdef __cplusplus
	}

	typedef lib::Input		Input;
	typedef lib::InputPack	InputPack;
}
#endif

#endif

#endif

#endif /* INCLUDE_HCM_ENVIRONS_HUMAN_INPUT_DECLARATIONS_H_ */
