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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H
#define INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H

#include "Environs.Native.h"
#include "Interop/Threads.h"

#define ENVIRONS_MAX_KEYBUFFER_SIZE		(ENVIRONS_MAX_KEYSIZE + 1024)
#define AES_SHA256_KEY_LENGTH           32
#define AES_256_BLOCK_SIZE				16

#define	ENVIRONS_CRYPT_PAD_FLAG			0xFF000000
#define	ENVIRONS_DEFAULT_CRYPT_PAD		ENVIRONS_CRYPT_PAD_OAEP

#define ENVIRONS_CERT_SIGNED_FLAG       0x80000000


#ifndef ANDROID

    #if ( defined(MEDIATORDAEMON) || defined(ENVIRONS_OSX) ) /// we use openssl for linux (and macos as apple's crypt simply doesn't work for rsa?)

        #if ( !defined(ENVIRONS_IOS) && !defined(_WIN32) )
        /// Use openssl for macos and mediatordaemon
            #define USE_OPENSSL
            #define USE_OPENSSL_AES
        #endif


    #endif

#endif


namespace environs
{
	typedef struct AESContext {
		char		*	keyCtx;
		char		*	encCtx;
		char		*	decCtx;
		pthread_mutex_t	encMutex;
		pthread_mutex_t	decMutex;
		unsigned int	size;
        unsigned int    version;
		unsigned int	deviceID;
	}
	AESContext;

	extern const char		*	hashSalt;
	extern pthread_mutex_t		privKeyMutex;

    extern bool InitEnvironsCrypt ();
    extern void ReleaseEnvironsCrypt ();

	/// <summary>
	/// The certificate file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	///		The upper 3rd byte (0x00FF0000) contains the certificate format: d = binary (DER), p = x509 cert file (PEM)
	/// - remaining bytes contains the certificate
	/// </summary>
	extern bool LoadPublicCertificate ( const char * pathFile, char ** cert );
	extern void DisposePublicKey ( void * key );

	/// <summary>
	/// The private key file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	/// - remaining bytes contains the private key in binary format (DER)
	/// </summary>
	extern bool LoadPrivateKey ( const char * pathFile, char ** keyDER, unsigned int * keyDERSize );
	extern void DisposePrivateKey ( void ** key );

    extern bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert );
    extern bool GenerateCertificate ( char ** priv, char ** pub );
    
    extern bool PreparePrivateKey ( char ** privKey );
    extern bool UpdateKeyAndCert ( char * priv, char * cert );

	extern bool EncryptMessage ( unsigned int deviceID, char * cert, char * msg, unsigned int * msgLen );
	extern bool DecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize );
	extern void ReleaseCert ( unsigned int deviceID );

    
	extern bool SHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen );
	extern bool SHAHashPassword ( const char * msg, char ** hash, unsigned int * hashLen );
	extern bool BuildEnvironsResponse ( unsigned int response, const char * userName, const char * userPass, char ** hash, unsigned int * hashLen );

	extern bool AESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx );
	extern void AESUpdateKeyContext ( AESContext * ctx, unsigned int deviceID );
	extern void AESDisposeKeyContext ( AESContext * ctx );
    

#define BUILD_IV_128(pUS)     { unsigned short * dpUS = (unsigned short *)(pUS);\
                              for ( unsigned int i=0; i < 8; i++ ) {\
                                *dpUS = (unsigned short) rand ( ); dpUS++;\
                              } }

	extern bool AESEncrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher );
	extern bool AESDecrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted );
	
	extern const char * ConvertToHexString ( const char * src, unsigned int length );
	extern const char * ConvertToHexSpaceString ( const char * src, unsigned int length );
    
	extern const char * ConvertToBytes ( const char * src, unsigned int length );
	extern char * ConvertToNewBytes ( const char * src, unsigned int length );
	extern char * ConvertToByteBuffer ( const char * src, unsigned int length, char * buffer );
}


#endif /// ->INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H
