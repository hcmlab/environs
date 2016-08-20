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

#ifdef USE_OPENSSL

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "Environs.Utils.h"
#include "DynLib/Dyn.Lib.Crypto.h"

#define CLASS_NAME	"Environs.Crypt.Lib . . ."


namespace environs
{
    bool                    openssl_alg_added       = false;

    
    /*
     * Additional functions for dynamic locks
     */
    struct CRYPTO_dynlock_value {
        pthread_mutex_t lock;
    };
    
    struct CRYPTO_dynlock_value * CryptDynLockCreate ( const char * file, int line )
    {
        CVerbVerbArg ( "CryptDynLockCreate: line [%i] file [%s]", line, file );
        
        struct CRYPTO_dynlock_value * value;
        
        value = (struct CRYPTO_dynlock_value *) malloc ( sizeof(struct CRYPTO_dynlock_value) );
        if ( !value ) {
            return 0;
        }
        
        if ( !LockInitA ( value->lock ) ) {
            free ( value );
            return 0;
        }
        
        return value;
    }
    
    void CryptDynLockDispose ( struct CRYPTO_dynlock_value * l, const char * file, int line )
    {
        CVerbVerbArg ( "CryptDynLockDispose: line [%i] file [%s]", line, file );
        
        if ( !l )
            return;
        
        LockDisposeA ( l->lock );
        
        free ( l );
    }
    
    
    unsigned long CryptoGetThreadID ( void )
    {
        unsigned long threadID;
        
        threadID = (unsigned long) GetCurrentThreadId (); // pthread_self();
        
        return threadID;
    }
    
    
    void CryptDynLockLock ( int mode, struct CRYPTO_dynlock_value * lock, const char *file, int line )
    {
#ifdef DEBUGVERBVerbCrypt
        CVerbVerbArg ( "CryptDynLockLock: mode [%i] lock [%b] line [%i] file [%s]", mode, lock, line, file );
#endif
        if ( mode & CRYPTO_LOCK ) {
            pthread_mutex_lock ( &lock->lock );
        } else {
            pthread_mutex_unlock ( &lock->lock );
        }
    }
    
    // Static locks used by crypt layer
    pthread_mutex_t *    cryptLocks = 0;
    
    /*
     * Required for muti-threading
     */
    void CryptLock ( int mode, int n, const char * file, int line )
    {
#ifdef DEBUGVERBVerbCrypt
        CVerbVerbArg ( "CryptLock: mode [%i] n [%i] line [%i] file [%s]", mode, n, line, file );
#endif
        if ( mode & CRYPTO_LOCK ) {
            pthread_mutex_lock ( &cryptLocks[n] );
        } else {
            pthread_mutex_unlock ( &cryptLocks[n] );
        }
    }
    
    
    bool CryptLocksCreate ()
    {
        CVerb ( "CryptLocksCreate" );
        
        cryptLocks = (pthread_mutex_t *) malloc ( dCRYPTO_num_locks () * sizeof(pthread_mutex_t) );
        if ( !cryptLocks ) {
            return false;
        }
        
        for ( int i = 0; i < dCRYPTO_num_locks (); i++ )
        {
            if ( !LockInitA ( cryptLocks [ i ] ) )
                return false;
        }
        
        dCRYPTO_set_locking_callback            ( CryptLock );
        dCRYPTO_set_id_callback                 ( CryptoGetThreadID );
        
        dCRYPTO_set_dynlock_create_callback     ( CryptDynLockCreate );
        dCRYPTO_set_dynlock_destroy_callback    ( CryptDynLockDispose );
        dCRYPTO_set_dynlock_lock_callback       ( CryptDynLockLock );
        
        CVerbVerb ( "CryptLocksCreate: ok" );
        return true;
    }
    
    
    void CryptLocksDispose ()
    {
        CVerb ( "CryptLocksDispose" );
        
        if ( !cryptLocks )
            return;
        
        for ( int i = 0; i < dCRYPTO_num_locks (); i++ )
        {
            LockDisposeA ( cryptLocks [ i ] );
        }
        
        free ( cryptLocks );
        cryptLocks = 0;
        
        dCRYPTO_set_locking_callback            ( 0 );
        dCRYPTO_set_id_callback                 ( 0 );
        
        dCRYPTO_set_dynlock_create_callback     ( 0 );
        dCRYPTO_set_dynlock_destroy_callback    ( 0 );
        dCRYPTO_set_dynlock_lock_callback       ( 0 );
        
        CVerbVerb ( "CryptLocksDispose: ok" );
    }


    bool cryptUpdateKeyAndCert ( char * priv, char * cert )
    {
        CVerb ( "UpdateKeyAndCert" );
        return true;
    }


    void cryptAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
    }

    
    bool cryptPreparePrivateKey ( char ** privKey )
    {
        if ( !privKey || !*privKey ) {
            CErr ( "PreparePrivateKey: Called with NULL argument" ); return false;
        }
        
        CVerb ( "PreparePrivateKey" );
        
        bool    success = false;
        RSA *   rsaKey  = 0;
        char *  key     = *privKey;

        *privKey = 0;
        
        const unsigned char * inKey = (const unsigned char *) (key + 4);
        unsigned int len = *((unsigned int *)key);
        if ( !len ) {
            CErr ( "PreparePrivateKey: Invalid length" ); goto Finish;
        }
        
        if ( !dd2i_RSAPrivateKey ( &rsaKey, &inKey, len) || !rsaKey ) {
            CErr ( "PreparePrivateKey: Converting key failed." ); goto Finish;
        }
        
        *privKey    = (char *) rsaKey;
        rsaKey      = 0;
        success     = true;
        
    Finish:
        free_n ( key );

        if ( rsaKey ) dRSA_free ( rsaKey );
        
        return success;
    }

    
#  define dEVP_PKEY_assign_RSA(pkey,rsa) dEVP_PKEY_assign((pkey),EVP_PKEY_RSA,\
(char *)(rsa))
    
#ifdef MEDIATORDAEMON
    
	bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert )
	{
		bool ret = false;
        
        RSA         *   rsa         = (RSA *) key;
        RSA         *   rsaPub      = 0;
        
        EVP_PKEY    *   pKey        = 0;
        EVP_PKEY    *   pPubKey     = 0;
        X509        *   x509        = 0;
        unsigned int    certDataSize = 0;
        char        *   certData    = 0;
        const unsigned char * certIn = (const unsigned char *) (pub + 4);
        
		do
		{
            pKey = dEVP_PKEY_new ();
            if ( !pKey ) {
                break;
            }
            
            if ( !dEVP_PKEY_set1_RSA ( pKey, rsa ) ) {
                break;
            }
            
            unsigned int certSize = *((unsigned int *)pub) & 0xFFFF;
			x509 = dd2i_X509 ( 0, &certIn, certSize );
            if ( !x509 ) {
                CVerb ( "SignCertificate: Read X509 cert file failed." );
                dERR_print_errors_fp ( stderr );
                
                if ( !dd2i_RSA_PUBKEY ( &rsaPub, &certIn, certSize ) ) {
                    CErr ( "SignCertificate: Read cert file and RSA public key failed." );
                    dERR_print_errors_fp ( stderr );
                    break;
                }
                CVerb ( "SignCertificate: Read RSA public key ok." );
                pPubKey = dEVP_PKEY_new ();
                if ( !pPubKey ) {
                    break;
                }
                
                if ( !dEVP_PKEY_assign_RSA ( pPubKey, rsaPub ) ) {
                    break;
                }
                
                x509 = dX509_new ();
                if ( !x509 ) {
                    break;
                }
                
                dX509_set_pubkey ( x509, pPubKey );
            }
            
            if ( !x509 ) {
                CErr ( "SignCertificate: Read cert data failed." );
                dERR_print_errors_fp ( stderr );
                break;
            }
            
            dASN1_INTEGER_set ( dX509_get_serialNumber ( x509 ), rand () );
            
            dX509_gmtime_adj ( X509_get_notBefore(x509), 0 );
            dX509_gmtime_adj ( X509_get_notAfter(x509), 94608000L );
            
            
            X509_NAME * subject = dX509_get_subject_name ( x509 );
            
            dX509_NAME_add_entry_by_txt ( subject, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "O",  MBSTRING_ASC, (unsigned char *)"Environs", -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "CN", MBSTRING_ASC, (unsigned char *)"hcm-lab.de", -1, -1, 0 );
            
            dX509_set_issuer_name ( x509, subject );
            
            if ( !dX509_sign ( x509, pKey, dEVP_sha1 () ) ) { /// We should switch to sha2
//            if ( !X509_sign ( x509, pKey, EVP_sha () ) ) {
                break;
            }
            
            certDataSize = di2d_X509 ( x509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "SignCertificate: i2d_X509 returned size [%i].", certDataSize ); break;
            }
            
            certData = (char *) malloc ( certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "SignCertificate: Memory allocation failed. size [%i].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = (unsigned char *) (certData + 4);
            if ( !di2d_X509 ( x509, &certDataStore ) ) {
                CErr ( "SignCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize | ENVIRONS_CERT_SIGNED_FLAG);
            *cert = (char *) certData;
            certData = 0;
            
            ret = true;
		}
		while ( 0 );
        
        if ( x509 )     dX509_free ( x509 );
        if ( pKey )     dEVP_PKEY_free ( pKey );
        free_n ( certData );
        
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }

		return ret;
	}
#endif

    
	bool cryptGenerateCertificate ( char ** priv, char ** pub )
	{
		bool            success     = false;
        
		char		*	privKey		= 0;
        EVP_PKEY    *   pKey        = 0;
        RSA         *   rsa         = 0;
        X509        *   x509        = 0;
        unsigned int    certDataSize = 0;
        char        *   certData    = 0;
        BIGNUM      *   bne         = 0;
        
        unsigned char * tmpPrivKey  = 0;
        
		do
		{
            pKey = dEVP_PKEY_new ();
            if ( !pKey ) {
                break;
            }
            
            if ( dRSA_generate_key ) {
                rsa = dRSA_generate_key ( ENVIRONS_DEVICES_KEYSIZE, RSA_F4, NULL, NULL );
            }
            else {
                bne = dBN_new ();
                int rc = success = dBN_set_word ( bne, RSA_F4 );
                if ( rc != 1 ) {
                    break;
                }
                
                rsa = dRSA_new ();
                
                rc = dRSA_generate_key_ex ( rsa, ENVIRONS_DEVICES_KEYSIZE, bne, NULL );
                if ( rc != 1 ) {
                    break;
                }
            }
            if ( !dEVP_PKEY_assign_RSA ( pKey, rsa ) ) {
                break;
            }
            
            x509 = dX509_new ();
            if ( !x509 ) {
                break;
            }
            
            dASN1_INTEGER_set( dX509_get_serialNumber ( x509 ), rand () );
            
            dX509_gmtime_adj ( X509_get_notBefore(x509), 0 );
            dX509_gmtime_adj ( X509_get_notAfter(x509), 94608000L );
            
            dX509_set_pubkey ( x509, pKey );
            
            X509_NAME * subject = dX509_get_subject_name ( x509 );
            
            dX509_NAME_add_entry_by_txt ( subject, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "O",  MBSTRING_ASC, (unsigned char *)"Environs", -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "CN", MBSTRING_ASC, (unsigned char *)"hcm-lab.de", -1, -1, 0 );
            
            dX509_set_issuer_name ( x509, subject );
            
            if ( !dX509_sign ( x509, pKey, dEVP_sha1 () ) ) {  /// We should switch to sha2
                break;
            }
            
            certDataSize = di2d_X509 ( x509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "GenerateCertificate: i2d_X509 returned size [%i].", certDataSize ); break;
            }
            
            certData = (char *) calloc ( 1, certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "GenerateCertificate: Memory allocation failed. size [%i].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = (unsigned char *) (certData + 4);
            if ( !di2d_X509 ( x509, &certDataStore ) ) {
                CErr ( "GenerateCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize | ENVIRONS_CERT_SIGNED_FLAG);
            *pub = (char *) certData;
            certData = 0;
            
            unsigned int privKeyLen = di2d_RSAPrivateKey(rsa, 0);
            if ( privKeyLen <= 0 ) {
                break;
            }
            certData = (char *) calloc ( 1, privKeyLen + 4 );
            if ( !certData ) {
                CErrArg ( "GenerateCertificate: Memory allocation failed. size [%i].", privKeyLen ); break;
            }
            
            privKeyLen = di2d_RSAPrivateKey (rsa, (unsigned char **)&tmpPrivKey);
            
            if ( privKeyLen <= 0 || !tmpPrivKey ) {
                CErr ( "GenerateCertificate: Failed to get private key." );
                break;
            }
            memcpy ( certData + 4, tmpPrivKey, privKeyLen );
            
            *((unsigned int *) certData) = privKeyLen;
            *priv = (char *) certData;
            certData = 0;
            
            
            success = true;
		}
		while ( 0 );
        
        if ( bne )      dBN_free ( bne );
        if ( x509 )     dX509_free ( x509 );
        if ( pKey )     dEVP_PKEY_free ( pKey );
//        if ( rsa )      dRSA_free ( rsa );
		free_n ( privKey );
        free_n ( certData );
		return success;
	}

    
	bool cryptEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
	{
		if ( !cert || !msg || !msgLen ) {
			CErrID ( "EncryptMessage: Called with at least one null argument." );
			return false;
		}

		unsigned int certProp   = *( ( unsigned int * ) cert );
		unsigned int certSize   = certProp & 0xFFFF;

		/// Determine format of certificate
		unsigned int format     = ( certProp >> 16 ) & 0xFF;
        
		CVerbArgID ( "EncryptMessage: Encrypting buffer sized [ %i ]", *msgLen );

//		CVerbArgID ( "EncryptMessage: Cert size [%u] [%s]", certSize, ConvertToHexSpaceString ( cert + 4, certSize ) );
//
//#ifdef DEBUGVERB
//		char c = cert [certSize]; cert [certSize] = 0;
//		CVerbArg ( "EncryptMessage: Cert raw [%s]", cert + 4 );
//		cert [certSize] = c;
//#endif

        if ( !openssl_alg_added ) {
			if ( dOpenSSL_add_all_algorithms )
				dOpenSSL_add_all_algorithms ();
			else
#ifdef OPENSSL_LOAD_CONF
				if ( dOPENSSL_add_all_algorithms_conf )
					dOPENSSL_add_all_algorithms_conf();
#else
				if ( dOPENSSL_add_all_algorithms_noconf )
					dOPENSSL_add_all_algorithms_noconf();
#endif
            dERR_load_crypto_strings();
            
            openssl_alg_added = true;
        }

        bool            success = false;
        EVP_PKEY    *   pkey	= 0;
        X509		*	cert509 = 0;
        RSA			*	rsaKey	= 0;
		char		*	ciphers = 0;
		unsigned int	cipherSize = 0;
        const unsigned char * certD = (unsigned char *) (cert + 4);
        
        do
        {            
			cert509 = dd2i_X509 ( 0, &certD, certSize );
            if ( !cert509 ) {
				CErrArgID ( "EncryptMessage: Read X509 cert file (format %c) failed.", format );
                dERR_print_errors_fp ( stderr );
                break;
            }
            
            pkey = dX509_get_pubkey ( cert509 );
            if ( !pkey ) {
				CErrID ( "EncryptMessage: Failed to load public key from certificate." ); break;
            }
            
            rsaKey = dEVP_PKEY_get1_RSA ( pkey );
            if ( !rsaKey ) {
				CErrID ( "EncryptMessage: Failed to load public key from certificate." ); break;
            }

            cipherSize = (unsigned int) dRSA_size(rsaKey);
            //unsigned char * pubK = 0;
            //unsigned int pubKSize =
            //i2d_RSAPublicKey(rsaKey, &pubK);
			/// Print the public key blob
			//CLogArg ( "EncryptMessage: Public key [%s]", ConvertToHexSpaceString ( (const char *)pubK, pubKSize ) );
            
            ciphers = (char *) malloc ( cipherSize );
            if ( !ciphers ) {
				CErrID ( "EncryptMessage: Memory alloc failed." ); break;
            }
            
			int pad = 0;
			//if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
			//	pad = RSA_PKCS1_OAEP_PADDING;
			if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1 )
                pad = RSA_PKCS1_PADDING;
            else
                pad = RSA_PKCS1_OAEP_PADDING; //RSA_NO_PADDING;

            int ciphersSize = dRSA_public_encrypt ( *msgLen, (unsigned char *)msg, (unsigned char *)ciphers, rsaKey, pad );
            if ( ciphersSize <= 0 ) {
				CErrID ( "EncryptMessage: Encrypt failed." ); break;
            }
            memcpy ( msg, ciphers, cipherSize );
            *msgLen = cipherSize;
            success = true;
        }
        while ( 0 );
        
		free_n ( ciphers );

        if ( pkey )		dEVP_PKEY_free ( pkey );
        if ( cert509 )	dX509_free ( cert509 );
        if ( rsaKey )	dRSA_free ( rsaKey );

        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

		return success;
	}

    
	bool cryptDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !key || !msg || !msgLen || !decrypted || !decryptedSize ) {
			CErr ( "DecryptMessage: Called with at least one null argument." );
			return false;
        }
		CVerbArg ( "DecryptMessage: Decrypting msg of size [ %i ]", msgLen );

		bool success = false;

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
		if ( pthread_mutex_lock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to acquire lock." );
			return false;
		}
#endif
		RSA     * rsa       = ( RSA * ) key;
		char    * decrypt   = 0;
		unsigned int rsaSize = ( unsigned int ) dRSA_size ( rsa );
        
        do
        {
			if ( msgLen != rsaSize ) {
				CErrArg ( "DecryptMessage: msgLen [ %u ] must be [ %u ].", msgLen, rsaSize ); break;
			}

			decrypt = ( char * ) malloc ( rsaSize + 1 );
			if ( !decrypt ) {
				CErr ( "DecryptMessage: Failed to allocate memory." ); break;
			}

			CVerb ( "DecryptMessage: Decrypting..." );

			int decSize = -1;

			do
			{
				decSize = dRSA_private_decrypt ( rsaSize, ( unsigned char* ) msg, ( unsigned char* ) decrypt, rsa, RSA_PKCS1_OAEP_PADDING );
				if ( decSize <= 0 ) {
					dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_OAEP_PADDING Decrypted message size is [ %i ].", decSize );
				}
				else break;

				decSize = dRSA_private_decrypt ( rsaSize, ( unsigned char* ) msg, ( unsigned char* ) decrypt, rsa, RSA_PKCS1_PADDING );
				if ( decSize <= 0 ) {
					dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_PADDING Decrypted message size is [ %i ].", decSize );
				}
				else break;

				decSize = dRSA_private_decrypt ( rsaSize, ( unsigned char* ) msg, ( unsigned char* ) decrypt, rsa, RSA_PKCS1_PSS_PADDING );
				if ( decSize <= 0 ) {
					dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_PSS_PADDING Decrypted message size is [ %i ].", decSize );
				}
				/*
				else break;
				decSize = dRSA_private_decrypt ( rsaSize, (unsigned char*)msg, (unsigned char*)decrypt, rsa, RSA_NO_PADDING );
				if ( decSize <= 0 ) {
				dERR_print_errors_fp ( stderr );
				CWarnArg ( "DecryptMessage: RSA_NO_PADDING Decrypted message size is [%i].", decSize );
				}
				*/
			}
			while ( 0 );

			if ( decSize <= 0 ) {
				dERR_print_errors_fp ( stderr );
				CErrArg ( "DecryptMessage: All padding types failed. Decrypted message size is [ %i ].", decSize ); break;
			}

			if ( decSize > ( int ) rsaSize ) {
				dERR_print_errors_fp ( stderr );
				CErrArg ( "DecryptMessage: Decrypted message size [ %u ] is larger than RSA buffer.", decSize ); break;
			}

			decrypt [ decSize ] = 0;
			//CVerbVerbArg ( "DecryptMessage: [%s]", decrypt );

			*decrypted      = ( char * ) decrypt;
			decrypt         = 0;
			*decryptedSize  = ( unsigned int ) decSize;
			success         = true;
        }
        while ( 0 );

		free_n ( decrypt );

        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
		if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to release lock." );
			success = false;
		}
#endif
		return success;
    }

	    
    void cryptReleaseCert ( int deviceID )
    {
    }
	
    
	void cryptAESDisposeKeyContext ( AESContext * ctx )
	{
		CVerb ( "AESDisposeKeyContext" );

		if ( !ctx )
			return;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( ctx->lockAllocated ) {
			LockDispose ( &ctx->encLock );
			LockDispose ( &ctx->decLock );

			ctx->lockAllocated = false;
		}
#endif
		if ( !ctx->encCtx )
			return;

        if ( ctx->encCtx ) {
            EVP_CIPHER_CTX * ect = ( EVP_CIPHER_CTX * ) ctx->encCtx;
            dEVP_CIPHER_CTX_cleanup ( ect );
            free ( ctx->encCtx );
        }
        if ( ctx->decCtx ) {
            EVP_CIPHER_CTX * ect = ( EVP_CIPHER_CTX * ) ctx->decCtx;
            dEVP_CIPHER_CTX_cleanup ( ect );
            free ( ctx->decCtx );
        }
        memset ( ctx, 0, sizeof ( AESContext ) );
	}

    
	bool cryptAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
	{
		CVerb ( "AESDeriveKeyContexts" );

		if ( !key || keyLen < AES_SHA256_KEY_LENGTH || !ctx ) {
			CErrArg ( "AESDeriveKeyContexts: Called with at least one NULL argument or keyLen [ %u ] < [ %u ].", keyLen, AES_SHA256_KEY_LENGTH ); return false;
		}

		char		*	blob		= 0;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        if ( !LockInit ( &ctx->encLock ) )
            return false;
        
        if ( !LockInit ( &ctx->decLock ) )
            return false;

		ctx->lockAllocated = true;
#endif
		bool success = false;

		do
		{
			SHA256_CTX sha;
			Zero ( sha );

			blob = (char *) malloc ( AES_SHA256_KEY_LENGTH );
			if ( !blob ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for hashed blob or IV." ); break;
			}

			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) malloc ( sizeof(EVP_CIPHER_CTX) );
			EVP_CIPHER_CTX * d = (EVP_CIPHER_CTX *) malloc ( sizeof(EVP_CIPHER_CTX) );
			if ( !e || !d ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for enc/dec contexts." );
                if ( e ) free ( e );
                if ( d ) free ( d );
                break;
			}

			ctx->encCtx = (char *)e;
			ctx->decCtx = (char *)d;

			unsigned char hash [ SHA256_DIGEST_LENGTH ];
			Zero ( hash );

			dSHA256_Init ( &sha );
			dSHA256_Update ( &sha, key, AES_SHA256_KEY_LENGTH );

			dSHA256_Final ( hash, &sha );
			memcpy ( blob, hash, AES_SHA256_KEY_LENGTH );
			
            dEVP_CIPHER_CTX_init ( e );

#ifdef ENABLE_AES_GCM
            success = (dEVP_EncryptInit_ex ( e, dEVP_aes_256_gcm (), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !success ) {
                CWarn ( "AESDeriveKeyContexts: Failed to initialize encryption." );
            }

            dEVP_CIPHER_CTX_init ( d );
            success = (dEVP_DecryptInit_ex ( d, dEVP_aes_256_gcm (), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !success ) {
                CWarn ( "AESDeriveKeyContexts: Failed to decrypt." );
            }
#else
            success = (dEVP_EncryptInit_ex ( e, dEVP_aes_256_cbc (), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !success ) {
                CWarn ( "AESDeriveKeyContexts: Failed to initialize encryption." );
            }

            dEVP_CIPHER_CTX_init ( d );
            success = (dEVP_DecryptInit_ex ( d, dEVP_aes_256_cbc (), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !success ) {
                CWarn ( "AESDeriveKeyContexts: Failed to decrypt." );
            }
#endif
			CVerbVerbArg ( "AESDeriveKeyContexts: AES key [ %s ]", ConvertToHexSpaceString ( blob, AES_SHA256_KEY_LENGTH ) );

			ctx->size = AES_SHA256_KEY_LENGTH;
			success = true;
		}
		while ( 0 );

        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

		free_n ( blob );
		
		return success;
    }
    
    
    /*
     * Encrypt *len bytes of data
     * All data going in & out is considered binary (unsigned char[])
     */
	bool cryptAESEncrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** cipher )
	{
		CVerbVerb ( "AESEncrypt" );

		if ( !ctx || !ctx->encCtx || !buffer || !bufferLen || !cipher ) {
			CErr ( "AESEncrypt: Called with at least one NULL argument." ); return false;
		}

		bool			success		= false;
		char		*	ciphers		= 0;
		char		*	cipherStart = 0;
		unsigned int	ciphersSize = 0;
        unsigned int    deviceID    = ctx->deviceID;

#ifdef ENABLE_AES_GCM
        int             nounce;
        char		*	aad         = 0;
#endif
		//CVerbVerbArgID ( "AESEncrypt: [%s]", buffer + 4 );
        //CVerbVerbArgID ( "AESEncrypt: [%s]", ConvertToHexSpaceString ( buffer, *bufferLen ) );

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        bool            locked      = false;
#endif
        /*************************************
         * OpenSSL Crypt
         *
         */
        
		do
		{
			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) ctx->encCtx;

			/* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
			ciphersSize = *bufferLen + AES_SHA256_KEY_LENGTH;
            
            CVerbVerbArg ( "AESEncrypt: ciphersSize [ %i ]", ciphersSize );
            
			int finalSize = 0;

#ifdef ENABLE_AES_GCM
#   ifdef NDEBUG
            ciphers = (char *) malloc ( ciphersSize + 41 );
#   else
            ciphers = (char *) calloc ( 1, ciphersSize + 41 );
#   endif
#else
#   ifdef NDEBUG
			ciphers = (char *) malloc ( ciphersSize + 21 );
#   else
            ciphers = (char *) calloc ( 1, ciphersSize + 21 );
#   endif
#endif
			if ( !ciphers ) {
				CErrArgID ( "AESEncrypt: Memory allocation [ %i bytes ] failed.", ciphersSize ); break;
			}
        
            BUILD_IV_128 ( ciphers + 4 );

#ifdef ENABLE_AES_GCM
            cipherStart = ciphers + 41;

            nounce      = ++ctx->nounce;

            aad         = cipherStart + 37;
            *(reinterpret_cast<int *>( aad )) = nounce;
#else
			cipherStart = ciphers + 20;
#endif
            CVerbVerb ( "AESEncrypt: dEVP_EncryptInit_ex" );

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
            if ( pthread_mutex_lock ( &ctx->encLock ) ) {
                CErrID ( "AESEncrypt: Failed to acquire mutex." );
                break;
            }
            locked = true;
#endif
            if ( dEVP_EncryptInit_ex ( e, NULL, NULL, NULL, (unsigned char *) (ciphers + 4) ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptInit_ex IV failed." ); break;
            }

			/*if ( !dEVP_EncryptInit_ex ( e, NULL, NULL, NULL, NULL ) ) {
				CErrID ( "AESEncrypt: EVP_EncryptInit_ex failed." ); break;
			}
            */

#ifdef ENABLE_AES_GCM
            if ( dEVP_EncryptUpdate ( e, NULL, (int *)&cipherStart, (unsigned char *)aad, 4 ) != 1 ) {
                CErrID ( "AESEncrypt: EVP_EncryptUpdate aad failed." ); break;
            }
#endif
            CVerbVerbArg ( "AESEncrypt: bufferLen [%i]", *bufferLen );
            
			if ( dEVP_EncryptUpdate ( e, (unsigned char *)cipherStart, (int *)&ciphersSize, (unsigned char *)buffer, *bufferLen ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptUpdate failed." ); break;
			}
        
            /* update ciphertext with the final remaining bytes */
            CVerbVerbArg ( "AESEncrypt: ciphersSize [%i]", ciphersSize );
            
			if ( dEVP_EncryptFinal_ex ( e, (unsigned char *) (cipherStart + ciphersSize), &finalSize ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptFinal_ex failed." ); break;
            }

#ifdef ENABLE_AES_GCM
            if ( dEVP_CIPHER_CTX_ctrl ( e, EVP_CTRL_GCM_GET_TAG, 16, ciphers + 20 ) != 1 ) {
                CErrID ( "AESEncrypt: Query gcm tag failed." ); break;
            }
#endif
            CVerbVerbArg ( "AESEncrypt: finalSize [ %i ]", finalSize );
       
			ciphersSize += finalSize;
			success = true;
		}
		while ( 0 );
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( locked && pthread_mutex_unlock ( &ctx->encLock ) ) {
			CErrID ( "AESEncrypt: Failed to release mutex." );
			success = false;
		}
#endif
        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

		if ( success ) {
			cipherStart [ ciphersSize ] = 0;
			ciphersSize += 20;

			/// Update enc header
			*( ( unsigned int * ) ciphers ) = ( 0x40000000 | ciphersSize );

			//CVerbVerbArgID ( "AESEncrypt: [%s]", ConvertToHexSpaceString ( ciphers, ciphersSize ) );

			*cipher = ciphers;
			//CVerbVerbArgID ( "AESEncrypt: IV [%s]", ConvertToHexSpaceString ( ciphers + 4, 16 ) );

			ciphers = 0;
			*bufferLen = ciphersSize;
		}

		free_n ( ciphers );
		return success;
    }

    
	bool cryptAESDecrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** decrypted )
	{
		CVerbVerb ( "AESDecrypt" );

		if ( !ctx || !ctx->decCtx || !buffer || !bufferLen || *bufferLen < 36 || !decrypted ) {
			CErr ( "AESDecrypt: Called with at least one NULL argument or encrypted packet is < 36." ); return false;
		}

		bool			success         = false;
		char		*	decrypt			= 0;
        char        *   IV              = buffer + 4;
		unsigned int	decryptedBufSize = *bufferLen + AES_SHA256_KEY_LENGTH + 5;
        int             deviceID        = ctx->deviceID;

#ifdef ENABLE_AES_GCM
        char        *   aesTag          = IV + 16;
        char        *   aad             = aesTag + 16;
#endif

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        bool            locked          = false;
#endif
		//CVerbVerbArgID ( "AESDecrypt: IV [%s]", ConvertToHexSpaceString ( IV, 16 ) );
		
		//CVerbVerbArgID ( "AESDecrypt: [%s]", ConvertToHexSpaceString ( buffer, *bufferLen ) );

		unsigned int	decryptedSize	= 0;

		do
		{
			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) ctx->decCtx;			

			decryptedSize = *bufferLen;
			int finalSize = 0;
#ifdef NDEBUG
			decrypt = (char *) malloc ( decryptedBufSize + 2 );
#else
            decrypt = (char *) calloc ( 1, decryptedBufSize + 2 );
#endif
			if ( !decrypt ) {
				CErrArg ( "AESDecrypt: Memory allocation [ %i bytes ] failed.", decryptedBufSize ); break;
            }

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#   ifdef MEASURE_LOCK_ACQUIRE
            if ( !LockAcquireA ( ctx->decLock, "AESDecrypt" ) )
#   else
                if ( pthread_mutex_lock ( &ctx->decLock ) )
#   endif
                {
                    CErr ( "AESDecrypt: Failed to acquire mutex." );
                    break;
                }
            locked = true;
#endif
            if ( dEVP_DecryptInit_ex ( e, NULL, NULL, NULL, (unsigned char *) IV ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptInit_ex for IV failed." ); break;
            }

#ifdef ENABLE_AES_GCM
            if ( dEVP_DecryptUpdate ( e, NULL, (int *)&decryptedSize, (unsigned char *)aad, 4 ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptUpdate aad failed." ); break;
            }

            unsigned int len = *bufferLen - 40;
            if ( dEVP_DecryptUpdate ( e, (unsigned char *)decrypt, (int *)&decryptedSize, (unsigned char *)(buffer + 40), len ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptUpdate failed." ); break;
            }

            if ( dEVP_CIPHER_CTX_ctrl ( e, EVP_CTRL_GCM_SET_TAG, 16, aesTag ) != 1 ) {
                CErrID ( "AESDecrypt: Apply of gcm tag failed." ); break;
            }
#else
            unsigned int len = *bufferLen - 20;
            if ( dEVP_DecryptUpdate ( e, (unsigned char *)decrypt, (int *)&decryptedSize, (unsigned char *)(buffer + 20), len ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptUpdate failed." ); break;
            }
#endif
			if ( dEVP_DecryptFinal_ex ( e, (unsigned char *) (decrypt + decryptedSize), &finalSize ) != 1 ) {
				CErrID ( "AESDecrypt: EVP_DecryptFinal_ex failed." ); break;
			}

			decryptedSize   += finalSize;
			success         = true;
		}
        while ( 0 );

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        if ( locked && pthread_mutex_unlock ( &ctx->decLock ) ) {
            CErrID ( "AESDecrypt: Failed to release mutex." );
            success = false;
        }
#endif
        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

        if ( success ) {
            if ( decryptedSize >= decryptedBufSize ) {
                CErrArgID ( "AESDecrypt: Decrypted message size [ %u ] > buffer size [ %u ]", decryptedSize, decryptedBufSize );
                success = false;
            }
            else {
                //CVerbVerbArgID ( "AESDecrypt: [%s]", ConvertToHexSpaceString ( decrypt, decryptedSize ) );

                *decrypted	= decrypt;
                decrypt [ decryptedSize ] = 0;
                decrypt		= 0;
                *bufferLen	= decryptedSize;
            }
        }
        
        free_n ( decrypt );
		return success;
    }

    

	/**
	*	SHAHashCreate: Creates a SHA512 hash for a given message, i.e. used to hash user passwords.
	*
	*/
	bool cryptSHAHashCreate ( const char * msg, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashCreate" );

		if ( !msg || !hash || !xchLen ) {
			CErr ( "SHAHashCreate: Invalid params." );
            return false;
		}
        
		bool		success = false;
		char	*	blob	= 0;
		
        SHA512_CTX ctx;
		do
		{
			if ( !dSHA512_Init ( &ctx ) ) {
				CErr ( "SHAHashCreate: SHA512_Init failed." ); break;
			}

			if ( !dSHA512_Update ( &ctx, msg, *xchLen ) ) {
				CErr ( "SHAHashCreate: SHA512_Update failed." ); break;
			}

			blob = (char *) malloc ( SHA512_DIGEST_LENGTH + 1 );
			if ( !blob ) {
				CErr ( "SHAHashCreate: Memory allocation failed." ); break;
			}

			if ( !dSHA512_Final ( (unsigned char *)blob, &ctx ) ) {
				CErr ( "SHAHashCreate: SHA512_Final failed." ); break;
			}            

			blob [ SHA512_DIGEST_LENGTH ] = 0;

			*hash   = blob;
			blob    = 0;
			*xchLen = SHA512_DIGEST_LENGTH;
			success = true;
		}
		while ( 0 );

		free_n ( blob );

        if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
        else
            if ( dERR_remove_state ) dERR_remove_state ( 0 );

		return success;
	}

    
	void cryptDisposePrivateKey ( void ** key )
	{
        if ( key && *key ) {
            dRSA_free ( (RSA *)*key );
            *key = 0;
        }
    }
    
    
	void cryptDisposePublicKey ( void * key )
	{
        if ( key ) dRSA_free ( (RSA *)key );
    }


#ifdef MEDIATORDAEMON

	bool LoadPublicCertificate ( const char * pathFile, char ** cert )
	{
		if ( !pathFile ) {
			CErr ( "LoadPublicCertificate: Called with NULL argument for path and file name" );
			return false;
		}

		if ( *cert ) free ( *cert );
		*cert = 0;

		bool ret = false;
        
        X509            *   cert509		= 0;
        unsigned char   *   certData	= 0;
        int                 certDataSize = 0;
        
        FILE * fp = fopen ( pathFile, "rb" );
        
        do
        {
            CLogArg ( "LoadPublicCertificate: [ %s ]", pathFile );
            
            if ( !fp ) {
                CErr ( "LoadPublicCertificate: 404." ); break;
            }
            
            if ( !dPEM_read_X509 ( fp, &cert509, NULL, NULL) ) {
                CErr ( "LoadPublicCertificate: Read X509 cert file failed." );
                dERR_print_errors_fp ( stderr );
                break;
            }

            certDataSize = di2d_X509 ( cert509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "LoadPublicCertificate: i2d_X509 returned size [ %i ].", certDataSize ); break;
            }
            
            certData = (unsigned char *) malloc ( certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "LoadPublicCertificate: Memory allocation failed. size [ %i ].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = certData + 4;
            if ( !di2d_X509 ( cert509, &certDataStore ) ) {
                CErr ( "LoadPublicCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize );
            *cert = (char *) certData;
            certData = 0;
            ret = true;
        }
        while ( 0 );
        
        if ( certData )
            free ( certData );
        if ( cert509 )
            dX509_free ( cert509 );
        if ( fp )
            fclose ( fp );
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }

		return ret;
	}

	
	bool LoadPrivateKey ( const char * pathFile, char ** key, unsigned int * keySize )
	{
		if ( !pathFile ) {
			CErr ( "LoadPrivateKey: Called with NULL argument for path and file name" ); return false;
		}

		bool ret = false;

		CLogArg ( "LoadPrivateKey: [%s]", pathFile );
		
        RSA * rsaKey = 0;

		FILE * fp = fopen ( pathFile, "rb" );
		if ( !fp ) {
			CErr ( "LoadPrivateKey: 404." );
			goto Finish;
		}

		if ( !dPEM_read_RSAPrivateKey ( fp, &rsaKey, NULL, NULL) )
		{
			CErr ( "LoadPrivateKey: Read RSA file failed." );
            dERR_print_errors_fp ( stderr );
			goto Finish;
        }

        *key = (char *) rsaKey;
        rsaKey = 0;
		ret = true;

	Finish:
		if ( rsaKey )
			dRSA_free ( rsaKey );
		if ( fp )
			fclose ( fp );
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }

		return ret;
	}
#endif

    bool InitEnvironsCryptOpenSSL ()
    {
        CVerb ( "InitEnvironsCryptOpenSSL" );

        if ( InitLibOpenSSL ( 0 ) ) {
            CInfo ( "InitEnvironsCrypt: Successfuly loaded libcrypto!" );

            EncryptMessage      = cryptEncryptMessage;
            DecryptMessage      = cryptDecryptMessage;
            ReleaseCert         = cryptReleaseCert;
            SHAHashCreate       = cryptSHAHashCreate;
            AESEncrypt          = cryptAESEncrypt;
            AESDecrypt          = cryptAESDecrypt;
            GenerateCertificate = cryptGenerateCertificate;
            AESDeriveKeyContext = cryptAESDeriveKeyContext;
            AESUpdateKeyContext = cryptAESUpdateKeyContext;
            AESDisposeKeyContext = cryptAESDisposeKeyContext;
            PreparePrivateKey   = cryptPreparePrivateKey;
            DisposePublicKey    = cryptDisposePublicKey;
            DisposePrivateKey   = cryptDisposePrivateKey;
            UpdateKeyAndCert    = cryptUpdateKeyAndCert;

            if ( !openssl_alg_added )
            {
                if ( dOpenSSL_add_all_algorithms )
                    dOpenSSL_add_all_algorithms ();
                else
#   ifdef OPENSSL_LOAD_CONF
                    if ( dOPENSSL_add_all_algorithms_conf )
                        dOPENSSL_add_all_algorithms_conf ();
#   else
                if ( dOPENSSL_add_all_algorithms_noconf )
                    dOPENSSL_add_all_algorithms_noconf ();
#   endif
                if ( dERR_load_crypto_strings )
                    dERR_load_crypto_strings ();

                openssl_alg_added = true;
                return true;
            }
        }

#   ifdef ANDROID
        CWarn ( "InitEnvironsCrypt: Failed to load libcrypto! Using platform layer encryption!" );
#   else
        CErr ( "InitEnvironsCrypt: Failed to initialize platform crypto! Using fallback platform layer encryption!" );
#   endif
        return false;
    }

    
    bool InitEnvironsCryptLocksOpenSSL ()
    {
        CVerb ( "InitEnvironsCryptLocksOpenSSL" );
        
        if ( !openssl_LibInitialized || !CryptLocksCreate () )
            return false;
        
        return true;
    }
    
    
    void ReleaseEnvironsCryptOpenSSL ()
    {
        CVerb ( "ReleaseEnvironsCryptOpenSSL" );
        
        ReleaseLibOpenSSL ();
    }
    
    
    void ReleaseEnvironsCryptLocksOpenSSL ()
    {
        CVerb ( "ReleaseEnvironsCryptLocksOpenSSL" );
        
        CryptLocksDispose ();
    }
    
    
    void InitEnvironsCryptToOpenSSL ()
    {
        CVerb ( "InitEnvironsCryptToOpenSSL" );

        InitEnvironsCryptPlatform			= InitEnvironsCryptOpenSSL;
        InitEnvironsCryptLocksPlatform		= InitEnvironsCryptLocksOpenSSL;
        ReleaseEnvironsCryptPlatform		= ReleaseEnvironsCryptOpenSSL;
        ReleaseEnvironsCryptLocksPlatform	= ReleaseEnvironsCryptLocksOpenSSL;
    }


#ifndef IGNORE_OPENSSL_AS_DEFAULT
        pInitEnvironsCrypt		InitEnvironsCryptPlatform			= InitEnvironsCryptOpenSSL;
        pInitEnvironsCrypt		InitEnvironsCryptLocksPlatform		= InitEnvironsCryptLocksOpenSSL;
        pReleaseEnvironsCrypt	ReleaseEnvironsCryptPlatform		= ReleaseEnvironsCryptOpenSSL;
        pReleaseEnvironsCrypt	ReleaseEnvironsCryptLocksPlatform	= ReleaseEnvironsCryptLocksOpenSSL;
#endif

}


#endif
