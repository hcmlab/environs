/**
 * Environs Crypto Common Utils and Helpers
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>



#define CLASS_NAME	"Environs.Crypt . . . . ."

#ifdef _WIN32
#   pragma warning( push )
#   pragma warning( disable: 4996 )
#   pragma warning( push )
#   pragma warning( disable: 4995 )
#endif


namespace environs
{
	/// Some salt to the hash soup...
	const char *            hashSalt                = "hcmEnvirons";

#ifdef ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS
	/// A mutex to synchronize access to the private key
	pthread_mutex_t         privKeyMutex;
#endif
    
    bool                    allocated               = false;
    
    pEncryptMessage         EncryptMessage  = 0;
    pDecryptMessage         DecryptMessage  = 0;
    pReleaseCert            ReleaseCert     = 0;
    pSHAHashCreate          SHAHashCreate   = 0;
    pAESEncrypt             AESEncrypt      = 0;
    pAESDecrypt             AESDecrypt      = 0;
    
    pGenerateCertificate    GenerateCertificate     = 0;
    
    pAESDeriveKeyContext    AESDeriveKeyContext     = 0;
    pAESUpdateKeyContext    AESUpdateKeyContext     = 0;
    pAESDisposeKeyContext   AESDisposeKeyContext    = 0;
    pPreparePrivateKey      PreparePrivateKey       = 0;
    pDisposePublicKey       DisposePublicKey        = 0;
    pDisposePrivateKey      DisposePrivateKey       = 0;
    pUpdateKeyAndCert       UpdateKeyAndCert        = 0;

#ifdef _WIN32  
	pDecryptMessageWithKeyHandles         DecryptMessageWithKeyHandles = 0;
#endif
	
	const char * ConvertToHexString ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		static char buffer [4096];

		for ( unsigned int i = 0; i < length; i++ )
		{
			sprintf ( buffer + (i * 2), "%02X", (unsigned char)src [i] );
			//	CLogArg ( "%02x", (unsigned char)blob [i] );
		}

		return buffer;
	}
    
	const char * ConvertToHexSpaceString ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;
        
		static char buffer [4096];
        
		for ( unsigned int i = 0; i < length; i++ )
		{
			sprintf ( buffer + (i * 3), "%02X ", (unsigned char)src [i] );
			//	CLogArg ( "%02x", (unsigned char)blob [i] );
		}
        
		return buffer;
    }
    
    const char * ConvertToHexSpaceBuffer ( const char * src, unsigned int length, char * buffer, bool limit )
    {
        if ( !src || !length )
            return 0;
        
		if ( limit && length > 300 )
			length = 300;
        for ( unsigned int i = 0; i < length; i++ )
        {
            sprintf ( buffer + (i * 3), "%02X ", (unsigned char)src [i] );
            //	CLogArg ( "%02x", (unsigned char)blob [i] );
        }
        
        buffer [ length * 3 ] = 0;
        
        return buffer;
    }


	char * ConvertToByteBuffer ( const char * src, unsigned int length, char * buffer )
	{
		length >>= 1;
		for ( unsigned int i = 0; i < length; i++ )
		{
			if ( sscanf_s ( src + (i * 2), "%2hhx", (unsigned char *)(buffer + i) ) != 1 )
			//if ( sscanf_s ( src + (i * 2), "%02X", (unsigned char *)(buffer + i) ) != 1 )
				return 0;
		}

		buffer [ length ] = 0;

		return buffer;
	}


	const char * ConvertToBytes ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		static char buffer [1024];

		return ConvertToByteBuffer ( src, length, buffer );
	}


	char * ConvertToNewBytes ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		char * buffer = (char *)calloc ( 1, length >> 1 );
		if ( !buffer )
			return 0;

		char * bufferNew = ConvertToByteBuffer ( src, length, buffer );
		if ( !bufferNew ) {
			free ( buffer );
		}
		return bufferNew;
	}

	
	/**
	*	SHAHashPassword: Creates a SHA512 hashed and salted digest for a given password.
	*
	*/
	bool SHAHashPassword ( const char * pass, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashPassword" );

		if ( !pass || !xchLen ) {
			CErr ( "SHAHashPassword: Invalid params." );
			return false;
		}
		
		unsigned int len = (unsigned int) strlen ( pass );

		char saltedSoup [MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1];
		memcpy ( saltedSoup, pass, len );

		unsigned int saltLen = (unsigned int) strlen ( hashSalt );
		if ( saltLen + len > MAX_NAMEPROPERTY + MAX_NAMEPROPERTY )
			saltLen = (MAX_NAMEPROPERTY + MAX_NAMEPROPERTY) - len;

		memcpy ( saltedSoup + len, hashSalt, saltLen );

		len += saltLen;
		saltedSoup [len] = 0;

		*xchLen = len;
		return SHAHashCreate ( saltedSoup, hash, xchLen );
	}


	/**
	*	BuildEnvironsResponse: Creates a response hash and prepent the username
	*		Format: Length(4) userName(x) Length(4) hash
	*
	*/
	bool BuildEnvironsResponse ( unsigned int response, const char * userName, const char * userPass, char ** hash, unsigned int * hashLen )
	{
		CVerb ( "BuildEnvironsResponse" );

		char buffer [ENVIRONS_USER_PASSWORD_LENGTH + (MAX_NAMEPROPERTY * 8) + 12];

		unsigned int * pUI = (unsigned int *) buffer;

		*pUI = response;
		unsigned int responseLen = 4;

		if ( userName && *userName && userPass && *userPass ) {
			CVerb ( "BuildEnvironsResponse: using user/pass to secure the response" );
			unsigned int length = (unsigned int) strlen ( userName );
			memcpy ( buffer + responseLen, userName, length );
			responseLen += length;

			length = (unsigned int) strlen ( userPass );
			if ( length > ENVIRONS_USER_PASSWORD_LENGTH )
				return false;

			memcpy ( buffer + responseLen, userPass, length );
			responseLen += length;
		}
		else {
			/// fill in random data 
			CVerb ( "BuildEnvironsResponse: using random data to secure the response" );
			pUI++;
			unsigned int ints = ENVIRONS_USER_PASSWORD_LENGTH / 4;
			for ( unsigned int i=0; i < ints; i++ ) {
				*pUI = rand (); pUI++;
			}
			responseLen += ints;
		}
		*hashLen = responseLen;

		return SHAHashCreate ( (const char *) buffer, hash, hashLen );

#ifdef DEBUGCIPHERS_
		char v = 'a';
		char * ciphers = (char *) calloc ( 1, 65 );
		if ( !ciphers )
			return false;

		for ( int i=0; i<64; i++) {
			ciphers[i] = v;
			v++;
			if ( v == 'z' )
				v = 'a';
		}
		*hash = ciphers;
		*hashLen  = 64;
		return true;
#else
#endif
	}

    
    
    void dDisposePrivateKey ( void ** key )
    {
        if ( key && *key ) {
            unsigned int keySize = *((unsigned int *)(*key));
            memset ( *key, 0, keySize );
            free ( *key );
            *key = 0;
        }
    }

    
    void dDisposePublicKey ( void * key )
    {
        if ( key )  free ( key );
    }
	    
    
    void ReleaseEnvironsCrypt ()
    {
        if ( allocated )
        {
            allocated = false;
            
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
            LockDispose ( &privKeyMutex );
#endif
			if ( ReleaseEnvironsCryptLocksPlatform )
				ReleaseEnvironsCryptLocksPlatform ();
        }
        
		if ( ReleaseEnvironsCryptPlatform )
			ReleaseEnvironsCryptPlatform ();
    }
    
    
    bool InitEnvironsCrypt ()
    {
        bool initToPlatform = true;

		if ( InitEnvironsCryptPlatform ) {
			if ( InitEnvironsCryptPlatform () )
				initToPlatform = false;
            else
                InitEnvironsCryptLocksPlatform = 0;
		}

        if ( initToPlatform )
        {
            EncryptMessage      = dEncryptMessage;
            DecryptMessage      = dDecryptMessage;
            ReleaseCert         = dReleaseCert;
            SHAHashCreate       = dSHAHashCreate;
            AESEncrypt          = dAESEncrypt;
            AESDecrypt          = dAESDecrypt;
            GenerateCertificate = dGenerateCertificate;
            AESDeriveKeyContext = dAESDeriveKeyContext;
            AESUpdateKeyContext = dAESUpdateKeyContext;
            AESDisposeKeyContext = dAESDisposeKeyContext;
            PreparePrivateKey   = dPreparePrivateKey;
            DisposePublicKey    = dDisposePublicKey;
            DisposePrivateKey   = dDisposePrivateKey;
            UpdateKeyAndCert    = dUpdateKeyAndCert;
            
#ifdef _WIN32
            DecryptMessageWithKeyHandles = dDecryptMessageWithKeyHandles;
#endif
        }

        if ( !allocated )
		{
#ifdef ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS
			if ( !LockInit ( &privKeyMutex ) )
				return false;
#endif
			if ( InitEnvironsCryptLocksPlatform )
				if ( !InitEnvironsCryptLocksPlatform () )
					return false;

			allocated = true;
		}

        return true;
    }


#ifndef IGNORE_OPENSSL_AS_PLATFORM
    bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen )
    {
        CVerb ( "SHAHashCreate" );
        return false;
    }


    bool dGenerateCertificate ( char ** priv, char ** pub )
    {
        CVerb ( "GenerateCertificate" );
        return false;
    }


    bool dPreparePrivateKey ( char ** privKey )
    {
        CVerb ( "PreparePrivateKey" );
        return false;
    }

    bool dUpdateKeyAndCert ( char * priv, char * cert )
    {
        CVerb ( "UpdateKeyAndCert" );
        return false;
    }


    void dAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
        CVerb ( "AESUpdateKeyContext" );
    }


    bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
    {
        CVerb ( "EncryptMessage" );
        return false;
    }


    bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
    {
        CVerb ( "DecryptMessage" );
        return false;
    }


    void dReleaseCert ( int deviceID )
    {
        CVerb ( "ReleaseCert" );
    }

    bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher )
    {
        CVerb ( "AESEncrypt" );
        return false;
    }

    bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted )
    {
        CVerb ( "AESDecrypt" );
        return false;
    }


    bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
    {
        CVerb ( "AESDeriveKeyContext" );
        return false;
    }

    void dAESDisposeKeyContext ( AESContext * ctx )
    {
        CVerb ( "AESDisposeKeyContext" );
    }
#endif

}


#ifdef _WIN32
#   pragma warning( pop )
#   pragma warning( pop )
#endif

