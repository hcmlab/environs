/**
 * MessageInstance CPP Object
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Native.h"
#include "Environs.Cli.Forwards.h"

#ifndef CLI_CPP
#	include "Message.Instance.h"
#	include "Device.Instance.h"
#	include "Portal.Instance.h"
	using namespace std;
#else
#	include "Core/Message.Instance.Cli.h"
#endif
#include "Tracer.h"

using namespace environs;

// The TAG for prepending to log messages
#define CLASS_NAME	"Message.Instance . . . ."


/* Namespace: environs -> */
namespace environs
{
	namespace lib
	{

		ENVIRONS_OUTPUT_ALLOC_WP ( MessageInstance );


		MessageInstance::MessageInstance ()
		{
			CVerbVerb ( "Construct" );

            TraceMessageInstanceAdd ( this );

            disposed_   = 0;
			sent_       = false;
			created_    = 0;
			length_     = 0;
			connection_ = 'c';

			text_       = nill;
            
            textShortEnd = CPP_CLI ( 0, nill );

            device_     = nill;

			ENVIRONS_OUTPUT_ALLOC_INIT_WSP ();
		}


		MessageInstance::~MessageInstance ()
        {
            CVerbVerb ( "Destruct" );

            TraceMessageInstanceRemove ( this );

            device_ = nill;
            
            free_m ( text_ );

            CVerbVerb ( "Destruct: done" );
		}


		bool MessageInstance::disposed ()
		{
			CVerbVerb ( "disposed" );

			return ( disposed_ == 1 );
		}


		void MessageInstance::DisposeInstance ()
		{
			CVerbVerbArg1 ( "DisposeInstance", "", "i", objID_ );
			
			//bool resetSP = ( ___sync_val_compare_and_swap ( c_Addr_of ( disposed_ ), 0, 1 ) == 0 );

			PlatformDispose ();
            
            /*if ( resetSP ) {
                CVerbVerbArg1 ( "DisposeInstance: Reseting SP", "", "i", objID_ );
                
                Cli_Only ( myself = nill );
			}*/
		}


		/**
		 * Release ownership on this interface and mark it disposable.
		 * Release must be called once for each Interface that the Environs framework returns to client code.
		 * Environs will dispose the underlying object if no more ownership is hold by anyone.
		 *
		 */
		void MessageInstance::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE_SP ( MessageInstance );
		}


		MessageInstanceESP MessageInstance::Create ( char * line, int length, c_const DeviceInstanceESP c_ref device )
		{
			CVerbVerb ( "Create" );

			if ( line == nill || length < 13 || *line == 0 || device == nill ) {
				CVerbVerbArg1 ( "Create: line invalid", "length", "i", length );
				return nill;
			}

			char * cur = line;

			int pos = 0;
			for ( ; pos < length; ++pos, ++cur )
				if ( *cur == ' ' )
					break;

			if ( pos >= length ) {
				CVerbVerbArg2 ( "Create: line invalid, pos of ' ' ", "", "i", pos, ">= length", "i", length );
				return nill;
			}

			*cur = 0;
			char * unixTime = line + 3;

			MessageInstanceESP msg = sp_make ( EPSPACE MessageInstance );
			if ( msg == nill )
				return nill;

			C_Only ( msg->myself = msg );

			msg->sent_       = ( line [ 0 ] == 'o' );
			msg->connection_ = ( char ) line [ 1 ];
			msg->device_     = device;

			msg->created_    = ( long ) atol ( unixTime );

			//*cur = ' ';

			cur++;

			long size = ( long ) ( length - ( cur - line ) );
			if ( size > 0 ) {
#ifdef CLI_CPP
				char t = cur [ size ];
				cur [ size ] = 0;

				msg->text_ = gcnew String ( cur );

				cur [ size ] = t;
				return msg;
#else
				msg->text_ = ( char * ) malloc ( size + 2 );
				if ( msg->text_ != nill )
				{
					memcpy ( msg->text_, cur, size );
					msg->text_ [ size ] = 0;

					msg->length_ = ( int ) size;
					return msg;
				}
#endif
			}
            
            msg->DisposeInstance ();
            
            C_Only ( msg->device_ = nill );
            
			return nill;

		}


		bool MessageInstance::HasPrefix ( char * line, int length )
		{
			CVerbVerb ( "HasPrefix" );

			if ( line == nill || length < 13 )
				return false;

			if ( line [ 0 ] != 'i' && line [ 0 ] != 'o' )
				return false;

			if ( line [ 2 ] != ':' )
				return false;

			return true;
		}


		bool MessageInstance::sent ()
		{
			CVerbVerb ( "sent" );
			return sent_;
		}


#ifndef CLI_CPP
		environs::DeviceInstancePtr MessageInstance::deviceRetained ()
		{
			CVerbVerb ( "deviceRetained" );

			if ( device_ == nill )
				return nill;

			if ( device_->Retain () )
                return ( environs::DeviceInstancePtr ) sp_get ( device_ );
            return nill;
		}
#endif

		char MessageInstance::connection ()
		{
			CVerbVerb ( "connection" );

			return connection_;
		}
        

		CString_ptr MessageInstance::text ()
		{
			CVerbVerb ( "text" );

            if ( text_ == nill || CString_ptr_empty ( text_ ) ) return "";

			if ( textShortEnd )
#ifdef CLI_CPP
				text_ = textShortEnd;
#else
				text_ [ 260 ] = textShortEnd;
#endif
			return text_;
		}

        
		int MessageInstance::length ()
		{
			CVerbVerb ( "length" );

			return length_;
		}
        

		unsigned long long MessageInstance::created ()
		{
			CVerbVerb ( "created" );

			return created_;
		}


		CString_ptr MessageInstance::toString ()
		{
			CVerbVerb ( "toString" );

			if ( STRING_empty ( toString_ ) )
			{
#ifdef CLI_CPP
				StringBuilder ^ sbl = gcnew StringBuilder ( "MessageInstance " );

				sbl->Append ( sent_ ? "sent" : "received" )->Append ( " on [" )->Append ( created_ )->Append ( "]  text [" )->Append ( shortText () )->Append ( "]" );

				toString_ = sbl->ToString ();
#else
				char buffer [ 340 ];

				if ( snprintf ( buffer, 256, "MessageInstance %s on [ %llu ]  text [ %s ]", ( sent_ ? "sent" : "received" ), created_, StringToCChar ( shortText () ) ) < 0 )
					*buffer = 0;

				toString_ = CCharToString ( buffer );
#endif
			}
			return STRING_get ( toString_ );
		}


		CString_ptr MessageInstance::shortText ()
		{
			CVerbVerb ( "shortText" );

			if ( text_ == nill || CString_ptr_empty ( text_ ) ) return "";

			if ( length_ < 260 ) return text_;

#ifdef CLI_CPP
			if ( textShortEnd == nill )
				textShortEnd = text_->Substring ( 0, 260 );
			text_ = textShortEnd;
#else
            if ( text_ [ 260 ] )
            {
                textShortEnd = text_ [ 260 ];

                text_ [ 260 ] = 0;
            }
#endif
			return text_;
		}
	}

} /* namespace environs */



