/**
* FileInstance CPP Object
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

#include "Interop/Stat.h"
#include "Environs.Utils.h"
#include "Environs.Native.h"
#include "Environs.Cli.Forwards.h"
#include "Environs.Lib.h"

#ifdef CLI_CPP
#	include "Environs.h"
#	include "Core/File.Instance.Cli.h"
#	include "Device/Device.Instance.Cli.h"
#else
#	include "File.Instance.h"
#	include "Device.Instance.h"
#	include "Portal.Instance.h"
#endif

using namespace std;

// The TAG for prepending to log messages
#define CLASS_NAME	"File.Instance. . . . . ."


/* Namespace: environs -> */
namespace environs
{    
	namespace lib
	{
		ENVIRONS_OUTPUT_ALLOC_WP ( FileInstance );
        

		FileInstance::FileInstance ()
		{
			CVerbVerb ( "Construct" );
            
            disposed_       = 0;

			fileID_         = -1;
			sent_           = false;
			created_        = 0;
			size_           = 0;
			type_           = 0;

			sendProgress_   = 0;
			receiveProgress_    = 0;

            device_         = nill;            
            
#if ( defined(ENVIRONS_OSX) || defined(ENVIRONS_IOS) )
            platformKeep            = nill;
#endif

			ENVIRONS_OUTPUT_ALLOC_INIT_WSP ();
		}


		FileInstance::~FileInstance ()
		{
			CVerbVerb ( "Destruct" );

            device_ = nill;            
		}


		bool FileInstance::disposed ()
		{
			CVerbVerb ( "disposed" );

			return ( disposed_ == 1 );
		}


		void FileInstance::DisposeInstance ()
		{
			CVerbVerbArg1 ( "DisposeInstance", "", "i", objID_ );

			//bool resetSP = ( ___sync_val_compare_and_swap ( c_Addr_of ( disposed_ ), 0, 1 ) == 0 );

			PlatformDispose ();

            /*if ( resetSP ) {
                Cli_Only ( myself = nill );
			}*/
		}

		/**
		 * Release ownership on this interface and mark it disposable.
		 * Release must be called once for each Interface that the Environs framework returns to client code.
		 * Environs will dispose the underlying object if no more ownership is hold by anyone.
		 *
		 */
		void FileInstance::Release ()
		{
			ENVIRONS_OUTPUT_RELEASE_SP ( FileInstance );
		}
        

		FileInstanceESP FileInstance::Create ( c_const DeviceInstanceESP c_ref device, int fileID, String_ptr fullPath, size_t length )
		{
			CVerbVerb ( "Create" );

			if ( device == nill )
				return nill;

			FileInstanceESP inst = sp_make ( EPSPACE FileInstance );
			if ( inst == nill )
				return nill;

			C_Only ( inst->myself = inst );

			inst->device_ = device;
			inst->fileID_ = fileID;

			STAT_STRUCT ( st );
			try
			{
#ifdef CLI_CPP
				IntPtr platformPath = StringToPlatformIntPtr ( fullPath );

				const char * pPath = ( const char * ) platformPath.ToPointer ();

				if ( stat ( pPath, &st ) == 0 )
#else
				if ( stat ( fullPath, &st ) == 0 )
#endif
				{
					inst->size_ = ( long ) st.st_size;
					inst->created_ = ( long ) unixEpoch ( st );
				}

				DisposePlatPointer ( platformPath );

				int descSize = 0;

#ifdef CLI_CPP
				fullPath = fullPath->Substring ( 0, (int)length - 4 ) + ".txt";
#else
				memcpy ( fullPath + length - 4, ".txt", 4 );
#endif
				char * descContents = environs::API::LoadBinaryN ( fullPath, &descSize );
				if ( descContents != nill )
				{
					inst->descriptor_ = CCharToString ( descContents );

					free_plt ( descContents );
				}

				return inst;
			}
			catch ( ... )
			{
			}
            
            inst->DisposeInstance ();
            
            C_Only ( inst->device_ = nill );
            
			return nill;
		}

        
		/**
		 * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
		 * A value of 0 indicates an invalid fileID.
		 * */
		int FileInstance::fileID ()
		{
			CVerbVerb ( "fileID" );

			return fileID_;
		}


		/**
		 * Used internally.
		 * */
		int FileInstance::type ()
		{
			CVerbVerb ( "type" );

			return type_;
		}


		/**
		 * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
		 * */
		CString_ptr FileInstance::descriptor ()
		{
			CVerbVerb ( "descriptor" );

			return STRING_get ( descriptor_ );
		}


		/**
		 * sent is true if this FileInstance is data that was sent or received (false).
		 * */
		bool FileInstance::sent ()
		{
			CVerbVerb ( "sent" );

			return sent_;
		}


		/**
		 * created is a posix timestamp that determines the time and date that this FileInstance
		 * has been received or sent.
		 * */
		unsigned long long FileInstance::created ()
		{
			CVerbVerb ( "created" );

			return created_;
		}


		/**
		 * The size in bytes of a buffer to send or data received.
		 * */
		long FileInstance::size ()
		{
			CVerbVerb ( "size" );

			return size_;
		}


		/**
		 * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
		 * */
		CString_ptr FileInstance::path ()
		{
			CVerbVerb ( "path" );

			return STRING_get ( path_ );
		}


		/**
		* Load data into a binary byte array and returns a pointer to that data.
		* */
		UCharArray_ptr FileInstance::data ()
		{
			CVerbVerb ( "data" );

			int size = 0;
			return device_->GetFile ( fileID_, size );
		}


		/**
		 * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
		 * file or buffer that has already been sent.
		 * If this value changes, then the corresponding device's DeviceObserver is notified
		 * with this FileInstance object as the sender
		 * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
		 * */
		int FileInstance::sendProgress ()
		{
			CVerbVerb ( "sendProgress" );

			return sendProgress_;
		}


		/**
		 * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
		 * file or buffer that has already been received.
		 * If this value changes, then the corresponding device's DeviceObserver is notified
		 * with this FileInstance object as the sender
		 * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
		 * */
		int FileInstance::receiveProgress ()
		{
			CVerbVerb ( "receiveProgress" );

			return receiveProgress_;
		}


		/**
		 * Get the DeviceInstance that this FileInstance is attached to.
		 *
		 * */
#ifndef CLI_CPP
		environs::DeviceInstancePtr FileInstance::deviceRetained ()
		{
			CVerbVerb ( "deviceRetained" );

			if ( device_ == nill )
				return nill;

			if ( device_->Retain () )
                return ( environs::DeviceInstance * ) device_.get ();
            return nill;
		}
#else
		environs::DeviceInstancePtr FileInstance::device ()
		{
			CVerbVerb ( "device" );
			return device_;
		}
#endif


		CString_ptr FileInstance::toString ()
		{
			CVerbVerb ( "toString" );

			if ( STRING_empty ( toString_ ) )
			{
#ifdef CLI_CPP
				StringBuilder ^ sbl = gcnew StringBuilder ( "FileInstance fileID [ " );
				
				sbl->Append ( fileID_ )->Append ( "]  desc [" )->Append ( STRING_empty ( descriptor_ ) ? "" : descriptor_ )
					->Append ( "]  size [" )->Append ( size_ )->Append ( " bytes] created at [" )->Append ( created_ )
					->Append ( "]" );

				toString_ = sbl->ToString ();
#else
				char buffer [ 256 ];

				if ( snprintf ( buffer, 256, "FileInstance fileID [ %i ]  desc [ %s ]  size [ %ld bytes ] created at [ %llu ]", fileID_, STRING_empty ( descriptor_ ) ? "" : STRING_get_cstr ( descriptor_ ), size_, created_ ) < 0 )
					*buffer = 0;

				toString_ = CCharToString ( buffer );
#endif
			}

			return STRING_get ( toString_ );
		}


		CString_ptr FileInstance::GetPath ()
		{
			CVerbVerb ( "GetPath" );

			if ( STRING_empty ( path_ ) )
			{
				if ( device_ != nill && device_->GetStoragePath () != nill )
				{
#ifdef CLI_CPP
					StringBuilder ^ sbl = gcnew StringBuilder ( "FileInstance fileID [ " );

					sbl->Append ( device_->GetStoragePath () )->Append ( fileID_ )->Append ( ".bin" );

					path_ = sbl->ToString ();
#else
					char buffer [ 1024 ];

					if ( snprintf ( buffer, 1024, "%s%i.bin", StringToCChar ( device_->GetStoragePath () ), fileID_ ) < 0 )
						*buffer = 0;

					path_ = CCharToString ( buffer );
#endif
				}
			}
			return STRING_get ( path_ );
		}

	}

} /* namespace environs */




