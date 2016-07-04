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

#if (defined(_WIN32) && !defined(USE_OPENSSL))
#   include <Wincrypt.h>

//#	define USE_WIN32_CNG_CRYPT
#endif

#define ENVIRONS_MAX_KEYBUFFER_SIZE		(ENVIRONS_MAX_KEYSIZE + 1024)
#define AES_SHA256_KEY_LENGTH           32
#define AES_256_BLOCK_SIZE				16

#define	ENVIRONS_CRYPT_PAD_FLAG			0xFF000000
#define	ENVIRONS_DEFAULT_CRYPT_PAD		ENVIRONS_CRYPT_PAD_OAEP

#define ENVIRONS_CERT_SIGNED_FLAG       0x80000000

#define ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS


#if defined(ENVIRONS_IOS) || defined(_WIN32)

#   define IGNORE_OPENSSL_AS_DEFAULT
#   define IGNORE_OPENSSL_AS_PLATFORM

#else
#   if defined(ANDROID) || defined(ENVIRONS_OSX)
#       define IGNORE_OPENSSL_AS_PLATFORM
#   endif

#   define USE_OPENSSL
#   define USE_OPENSSL_AES

#endif

#ifdef ENVIRONS_OSX1
#   ifdef USE_OPENSSL
#       undef USE_OPENSSL
#   endif
#   ifdef USE_OPENSSL_AES
#       undef USE_OPENSSL_AES
#   endif
#endif

#ifndef OPENSSL1
//#	ifndef _WIN32
#		define ENABLE_CRYPT_AES_LOCKED_ACCESS
//#	endif
#endif

#ifdef ENVIRONS_IOS
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#       undef ENABLE_CRYPT_AES_LOCKED_ACCESS
#   endif
#endif

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#   define  CRYPT_AES_LOCK_EXP(exp)       exp
#else
#   define  CRYPT_AES_LOCK_EXP(exp)
#endif

#if ( defined(ENVIRONS_IOS) )
//#   define	ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
#endif

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
//#   define	ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_CLIENT_ACCESS
#endif


//#define ENABLE_AES_GCM

namespace environs
{
#define ENVIRONS_AES_CBC    0
#define ENVIRONS_AES_GCM    1

	typedef struct AESContext {
		char		*	keyCtx;
		char		*	encCtx;
        char		*	decCtx;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		bool			lockAllocated;
		pthread_mutex_t	encLock;
		pthread_mutex_t	decLock;
#endif        
		unsigned int	size;
        unsigned int    version;
		int				deviceID;
        int             nounce;
	}
	AESContext;

	extern const char		*	hashSalt;
	extern pthread_mutex_t		privKeyMutex;

	typedef bool ( CallConv * pInitEnvironsCrypt )( void );
	typedef void ( CallConv * pReleaseEnvironsCrypt )( void );

    extern bool InitEnvironsCrypt ();
	extern pInitEnvironsCrypt InitEnvironsCryptPlatform;
	extern pInitEnvironsCrypt InitEnvironsCryptLocksPlatform;

    extern void ReleaseEnvironsCrypt ();
	extern pReleaseEnvironsCrypt ReleaseEnvironsCryptPlatform;
	extern pReleaseEnvironsCrypt ReleaseEnvironsCryptLocksPlatform;

	/// <summary>
	/// The certificate file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	///		The upper 3rd byte (0x00FF0000) contains the certificate format: d = binary (DER), p = x509 cert file (PEM)
	/// - remaining bytes contains the certificate
	/// </summary>
	extern bool LoadPublicCertificate ( const char * pathFile, char ** cert );
    
    typedef void (CallConv * pDisposePublicKey)(void * key);
    
    extern pDisposePublicKey DisposePublicKey;
	extern void dDisposePublicKey ( void * key );

	/// <summary>
	/// The private key file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	/// - remaining bytes contains the private key in binary format (DER)
	/// </summary>
	extern bool LoadPrivateKey ( const char * pathFile, char ** keyDER, unsigned int * keyDERSize );
    
    typedef void (CallConv * pDisposePrivateKey)(void ** key);
    
    extern pDisposePrivateKey DisposePrivateKey;
	extern void dDisposePrivateKey ( void ** key );

    typedef bool (CallConv * pUpdateKeyAndCert)(char * priv, char * cert);
    extern pUpdateKeyAndCert UpdateKeyAndCert;

    extern bool dUpdateKeyAndCert ( char * priv, char * cert );

    extern bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert );
    
    typedef bool (CallConv * pGenerateCertificate)(char ** priv, char ** pub);
    
    extern pGenerateCertificate GenerateCertificate;
    extern bool dGenerateCertificate ( char ** priv, char ** pub );
    
    typedef bool (CallConv * pPreparePrivateKey)(char ** privKey);
    
    extern pPreparePrivateKey PreparePrivateKey;
    extern bool dPreparePrivateKey ( char ** privKey );
    
    typedef bool (CallConv * pEncryptMessage)(int deviceID, char * cert, char * msg, unsigned int * msgLen);
    
    extern pEncryptMessage EncryptMessage;
	extern bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen );
    
    typedef bool (CallConv * pDecryptMessage)(char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize);
    
    extern pDecryptMessage DecryptMessage;
	extern bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize );
    
#ifdef _WIN32    
	typedef bool (CallConv * pDecryptMessageWithKeyHandles )( HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize);

	extern pDecryptMessageWithKeyHandles DecryptMessageWithKeyHandles;
	extern bool dDecryptMessageWithKeyHandles ( HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize );
#endif

    typedef void (CallConv * pReleaseCert)(int deviceID);
    
    extern pReleaseCert ReleaseCert;
	extern void dReleaseCert ( int deviceID );

    
    typedef bool (CallConv * pSHAHashCreate)(const char * msg, char ** hash, unsigned int * hashLen);
    
    extern pSHAHashCreate SHAHashCreate;
	extern bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen );
    
	extern bool SHAHashPassword ( const char * msg, char ** hash, unsigned int * hashLen );
    
	extern bool BuildEnvironsResponse ( unsigned int response, const char * userName, const char * userPass, char ** hash, unsigned int * hashLen );
    
    
    typedef bool (CallConv * pAESDeriveKeyContext)(char * key, unsigned int keyLen, AESContext * ctx);
    
    extern pAESDeriveKeyContext AESDeriveKeyContext;
	extern bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx );
    
    typedef void (CallConv * pAESUpdateKeyContext)(AESContext * ctx, int deviceID);
    
    extern pAESUpdateKeyContext AESUpdateKeyContext;
	extern void dAESUpdateKeyContext ( AESContext * ctx, int deviceID );
    
    typedef void (CallConv * pAESDisposeKeyContext)(AESContext * ctx);
    
    extern pAESDisposeKeyContext AESDisposeKeyContext;
	extern void dAESDisposeKeyContext ( AESContext * ctx );
    

#define BUILD_IV_128(pUS)     { unsigned short * dpUS = (unsigned short *)(pUS);\
                              for ( unsigned int i=0; i < 8; i++ ) {\
                                *dpUS = (unsigned short) rand ( ); dpUS++;\
                              } }

    typedef bool (CallConv * pAESEncrypt)(AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher );

    extern pAESEncrypt AESEncrypt;
	extern bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher );


    typedef bool (CallConv * pAESDecrypt)(AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted);

    extern pAESDecrypt AESDecrypt;
	extern bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted );
	
	extern const char * ConvertToHexString ( const char * src, unsigned int length );
    extern const char * ConvertToHexSpaceString ( const char * src, unsigned int length );
    extern const char * ConvertToHexSpaceBuffer ( const char * src, unsigned int length, char * buffer, bool limit = true );

	extern const char * ConvertToBytes ( const char * src, unsigned int length );
	extern char * ConvertToNewBytes ( const char * src, unsigned int length );
	extern char * ConvertToByteBuffer ( const char * src, unsigned int length, char * buffer );
}


#endif /// ->INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H
