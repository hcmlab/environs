/**
 * Environs Win32 Cryptography Next Generation API Implementation
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
//#   define DEBUGCIPHERS
#endif

#include "Environs.Crypt.h"

#ifdef USE_WIN32_CNG_CRYPT

#include "Environs.Utils.h"
#include "Interop.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Strsafe.h>
#include <Bcrypt.h>

#pragma comment ( lib, "Bcrypt.lib" )
#define USE_CACHED_HKEY

#define CLASS_NAME	"Environs.Crypt.Win32 . ."


namespace environs
{
	// AES 256-bit blob
	struct AES_256_BLOB {
		BLOBHEADER hdr;
		DWORD keySize;
		BYTE raw [32];
	};

	static const LPWSTR ENVIRONS_KEY_STORE_NAME = L"Environs";

	
	const LPWSTR bGetUserEnvironsName ( unsigned int counter )
	{
		static WCHAR	userName [ 256 ];
		DWORD			size = 256;

		if ( !GetUserName ( userName, &size ) ) {
			CErr ( "GetUserEnvironsName: GetUserName Failed." );
			return 0;
		}

		static WCHAR	userKeyStoreName [ 1024 ];
		DWORD			KSsize = 1024;

		HRESULT hr = StringCbPrintfW ( userKeyStoreName, KSsize, L"%s.%u.%s", ENVIRONS_KEY_STORE_NAME, counter, userName );
		if ( hr != S_OK ){
			CErr ( "GetUserEnvironsName: StringCbPrintfW Failed." );
			return 0;
		}

		return userKeyStoreName;
	}  

        
	bool bEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
	{
		if ( !cert || !msg || !msgLen ) {
			CErrID ( "EncryptMessage: Called with at least one null argument." );
			return false;
		}

		return false;
	}

    
	bool bDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !key || !msg || !msgLen || !decrypted || !decryptedSize ) {
			CErr ( "DecryptMessage: Called with at least one null argument." );
			return false;
        }
		CVerbArg ( "DecryptMessage: Decrypting msg of size [ %i ]", msgLen );

		bool success = false;

		return success;
    }


	bool bDecryptMessageWithKeyHandles ( HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !hKey || !msg || !msgLen || !decrypted || !decryptedSize ) {
			CErr ( "DecryptMessageWithKeyHandles: Called with at least one null argument." );
			return false;
		}
		CVerbArg ( "DecryptMessageWithKeyHandles: Decrypting msg of size [ %i ]", msgLen );

		bool success = false;

		return success;
	}

    
	void bAESDisposeKeyContext ( AESContext * ctx )
	{
		CVerb ( "AESDisposeKeyContext" );

		if ( !ctx || !ctx->encCtx )
			return;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        LockDispose ( &ctx->encLock );
        LockDispose ( &ctx->decLock );
#endif

		memset ( ctx, 0, sizeof ( AESContext ) );
	}

    
	bool bAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
	{
		CVerb ( "AESDeriveKeyContexts" );

		if ( !key || keyLen < AES_SHA256_KEY_LENGTH || !ctx ) {
			CErrArg ( "AESDeriveKeyContexts: Called with at least one NULL argument or keyLen [ %u ] < [ %u ].", keyLen, AES_SHA256_KEY_LENGTH ); return false;
		}

        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        if ( !LockInit ( &ctx->encLock ) )
            return false;
        
        if ( !LockInit ( &ctx->decLock ) )
            return false;
#endif
		bool ret = false;

		
		return ret;
    }
    
    
    /*
     * Encrypt *len bytes of data
     * All data going in & out is considered binary (unsigned char[])
     */
	bool bAESEncrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** cipher )
	{
		CVerbVerb ( "AESEncrypt" );

		if ( !ctx || !ctx->encCtx || !buffer || !bufferLen || !cipher ) {
			CErr ( "AESEncrypt: Called with at least one NULL argument." ); return false;
		}

		return false;
    }

    
	bool bAESDecrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** decrypted )
	{
		CVerbVerb ( "AESDecrypt" );

		if ( !ctx || !ctx->decCtx || !buffer || !bufferLen || *bufferLen < 36 || !decrypted ) {
			CErr ( "AESDecrypt: Called with at least one NULL argument or encrypted packet is < 36." ); return false;
		}

		bool			success				= false;
		return success;
    }
        

	/**
	*	SHAHashCreate: Creates a SHA512 hash for a given message, i.e. used to hash user passwords.
	*
	*/
	bool bSHAHashCreate ( const char * msg, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashCreate" );

		if ( !msg || !hash || !xchLen ) {
			CErr ( "SHAHashCreate: Invalid params." );
            return false;
		}
        
		bool		success		= false;

		
		return success;
	}


	bool InitEnvironsCryptCNG ()
	{
		CVerb ( "InitEnvironsCryptCNG" );

		EncryptMessage      = bEncryptMessage;
		DecryptMessage      = bDecryptMessage;
		ReleaseCert         = dReleaseCert;
		SHAHashCreate       = bSHAHashCreate;
		AESEncrypt          = bAESEncrypt;
		AESDecrypt          = bAESDecrypt;
		GenerateCertificate = dGenerateCertificate;
		AESDeriveKeyContext = bAESDeriveKeyContext;
		AESUpdateKeyContext = dAESUpdateKeyContext;
		AESDisposeKeyContext = bAESDisposeKeyContext;
		PreparePrivateKey   = dPreparePrivateKey;
		//DisposePublicKey    = bDisposePublicKey;
		//DisposePrivateKey   = bDisposePrivateKey;
		UpdateKeyAndCert    = dUpdateKeyAndCert;

		return true;
	}

	pInitEnvironsCrypt		InitEnvironsCryptPlatform			= InitEnvironsCryptCNG;
    
}


#endif