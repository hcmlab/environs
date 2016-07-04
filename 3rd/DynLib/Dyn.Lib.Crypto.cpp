/**
 * Dynamically accessing openssl
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

#ifndef MEDIATORDAEMON
#   include "Environs.Obj.h"
#endif

#include "Interop.h"
#include "Dyn.Lib.Crypto.h"
using namespace environs;

#include <errno.h>

// Disable this flag to use library as statically linked library again
//#ifdef ENVIRONS_IOS
#define USE_DYNAMIC_LIB
//#endif

#define CLASS_NAME	"Dyn.Lib.OpenSSL. . . . ."

#define	MODLIBNAME	"libcrypto"


namespace environs
{
	bool							     openssl_LibInitialized		= false;
	HMODULE							     hLibOpenSSL                 = 0;

	pOpenSSL_version_num				 dOpenSSL_version_num = 0;

    pOPENSSL_add_all_algorithms_noconf   dOPENSSL_add_all_algorithms_noconf = 0;
    pOPENSSL_add_all_algorithms_conf     dOPENSSL_add_all_algorithms_conf = 0;
	pOpenSSL_add_all_algorithms			 dOpenSSL_add_all_algorithms = 0;

    pERR_load_ERR_strings                dERR_load_ERR_strings = 0;
    pERR_load_crypto_strings             dERR_load_crypto_strings = 0;
    pERR_remove_state                    dERR_remove_state = 0;
    pERR_remove_thread_state             dERR_remove_thread_state = 0;

    pERR_free_strings                    dERR_free_strings = 0;
    pENGINE_cleanup                      dENGINE_cleanup = 0;
    
    pCRYPTO_num_locks                    dCRYPTO_num_locks = 0;
    pCRYPTO_set_locking_callback         dCRYPTO_set_locking_callback = 0;
    pCRYPTO_set_id_callback              dCRYPTO_set_id_callback = 0;
    pCRYPTO_set_dynlock_create_callback  dCRYPTO_set_dynlock_create_callback = 0;
    pCRYPTO_set_dynlock_lock_callback    dCRYPTO_set_dynlock_lock_callback = 0;
    pCRYPTO_set_dynlock_destroy_callback dCRYPTO_set_dynlock_destroy_callback = 0;
    pCRYPTO_cleanup_all_ex_data          dCRYPTO_cleanup_all_ex_data = 0;
    
    pBN_free                             dBN_free = 0;
    pBN_new                              dBN_new = 0;
    pBN_set_word                         dBN_set_word = 0;
    pRSA_new                             dRSA_new = 0;
    
    pEVP_sha1                            dEVP_sha1 = 0;
    pEVP_PKEY_new                        dEVP_PKEY_new = 0;
    pEVP_PKEY_free                       dEVP_PKEY_free = 0;
    pEVP_cleanup                         dEVP_cleanup = 0;
    pX509_free                           dX509_free = 0;
    pRSA_generate_key                    dRSA_generate_key = 0;
    pRSA_generate_key_ex                 dRSA_generate_key_ex = 0;
    pEVP_PKEY_assign                     dEVP_PKEY_assign = 0;
    pRSA_free                            dRSA_free = 0;

    pASN1_INTEGER_set                    dASN1_INTEGER_set = 0;
    pX509_gmtime_adj                     dX509_gmtime_adj = 0;

    pX509_set_pubkey                     dX509_set_pubkey = 0;
    pX509_get_subject_name               dX509_get_subject_name = 0;
    pX509_NAME_add_entry_by_txt          dX509_NAME_add_entry_by_txt = 0;
    pX509_set_issuer_name                dX509_set_issuer_name = 0;
    pX509_get_serialNumber               dX509_get_serialNumber = 0;

    pX509_sign                           dX509_sign = 0;
    pd2i_RSAPrivateKey                   dd2i_RSAPrivateKey = 0;
    pi2d_RSAPrivateKey                   di2d_RSAPrivateKey = 0;

    pX509_new                            dX509_new = 0;
    pi2d_X509                            di2d_X509 = 0;
    pd2i_X509                            dd2i_X509 = 0;
    pX509_get_pubkey                     dX509_get_pubkey = 0;
    pEVP_PKEY_get1_RSA                   dEVP_PKEY_get1_RSA = 0;
    pRSA_size                            dRSA_size = 0;
    pRSA_public_encrypt                  dRSA_public_encrypt = 0;
    pRSA_private_decrypt                 dRSA_private_decrypt = 0;
    pERR_print_errors_fp                 dERR_print_errors_fp = 0;

    pEVP_PKEY_set1_RSA                   dEVP_PKEY_set1_RSA = 0;
    pd2i_RSA_PUBKEY                      dd2i_RSA_PUBKEY = 0;
    pPEM_read_X509                       dPEM_read_X509 = 0;
    pPEM_read_RSAPrivateKey              dPEM_read_RSAPrivateKey = 0;

    pEVP_CIPHER_CTX_cleanup              dEVP_CIPHER_CTX_cleanup = 0;
    pEVP_CIPHER_CTX_init                 dEVP_CIPHER_CTX_init = 0;

    pEVP_EncryptInit_ex                  dEVP_EncryptInit_ex = 0;
    pEVP_DecryptInit_ex                  dEVP_DecryptInit_ex = 0;
    pEVP_DecryptUpdate                   dEVP_DecryptUpdate = 0;
    pEVP_DecryptFinal_ex                 dEVP_DecryptFinal_ex = 0;
    pEVP_EncryptUpdate                   dEVP_EncryptUpdate = 0;
    pEVP_EncryptFinal_ex                 dEVP_EncryptFinal_ex = 0;
    pEVP_CIPHER_CTX_ctrl                 dEVP_CIPHER_CTX_ctrl = 0;

    pEVP_aes_256_cbc                     dEVP_aes_256_cbc = 0;
    pEVP_aes_256_gcm                     dEVP_aes_256_gcm = 0;

    pSHA256_Init                         dSHA256_Init = 0;
    pSHA256_Update                       dSHA256_Update = 0;
    pSHA256_Final                        dSHA256_Final = 0;

    pSHA512_Init                         dSHA512_Init = 0;
    pSHA512_Update                       dSHA512_Update = 0;
    pSHA512_Final                        dSHA512_Final = 0;


	bool VerifyLibOpenSSLAccess ()
	{
		if ( !dOpenSSL_add_all_algorithms && !dOPENSSL_add_all_algorithms_noconf && !dOPENSSL_add_all_algorithms_conf ) {
			CWarn ( "VerifyLibOpenSSLAccess: Openssl init function could not be loaded!" );
			return false;
		}

		if ( !dERR_load_ERR_strings || !dERR_load_crypto_strings
             || !dEVP_PKEY_new || !dEVP_PKEY_free || !dX509_free || !dRSA_generate_key_ex || !dEVP_PKEY_assign || !dRSA_free || !dX509_get_serialNumber
            || !dASN1_INTEGER_set || !dX509_gmtime_adj || !dX509_set_pubkey || !dX509_get_subject_name || !dX509_NAME_add_entry_by_txt || !dX509_set_issuer_name
            || !dX509_sign || !dd2i_RSAPrivateKey || !di2d_RSAPrivateKey || !dd2i_X509 || !di2d_X509 || !dX509_get_pubkey || !dEVP_PKEY_get1_RSA
            || !dRSA_size || !dRSA_public_encrypt || !dRSA_private_decrypt || !dERR_print_errors_fp || !dEVP_CIPHER_CTX_cleanup || !dEVP_CIPHER_CTX_init
            || !dEVP_EncryptInit_ex || !dEVP_DecryptInit_ex || !dEVP_DecryptUpdate || !dEVP_DecryptFinal_ex || !dEVP_EncryptUpdate || !dEVP_EncryptFinal_ex
            || !dSHA256_Init || !dSHA256_Update || !dSHA256_Final || !dSHA512_Init || !dSHA512_Update || !dSHA512_Final || !dX509_new || !dEVP_sha1
            || !dEVP_aes_256_cbc || !dRSA_new || !dBN_new || !dBN_set_word || !dBN_free || !dCRYPTO_set_dynlock_destroy_callback
            || !dCRYPTO_set_locking_callback || !dCRYPTO_num_locks || !dCRYPTO_set_id_callback || !dCRYPTO_set_dynlock_create_callback || !dCRYPTO_set_dynlock_lock_callback
#ifdef MEDIATORDAEMON
            || !dEVP_PKEY_set1_RSA || !dd2i_RSA_PUBKEY || !dPEM_read_X509 || !dPEM_read_RSAPrivateKey
#endif

#ifdef ENABLE_AES_GCM
            || !dEVP_aes_256_gcm || !dEVP_CIPHER_CTX_ctrl
#endif
			) {
			CWarn ( "VerifyLibOpenSSLAccess: One of the openssl functions could not be loaded!" );

#ifndef NDEBUG
			CLogArg ( "VerifyLibOpenSSLAccess: dOpenSSL_add_all_algorithms [%i], dOPENSSL_add_all_algorithms_noconf [%i], dOPENSSL_add_all_algorithms_conf [%i], dERR_load_ERR_strings [%i], dERR_load_crypto_strings [%i]",
				dOpenSSL_add_all_algorithms ? 1 : 0, dOPENSSL_add_all_algorithms_noconf ? 1 : 0, dOPENSSL_add_all_algorithms_conf ? 1 : 0, dERR_load_ERR_strings ? 1 : 0, dERR_load_crypto_strings ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dEVP_PKEY_new [%i], dEVP_PKEY_free [%i], dX509_free [%i], dRSA_generate_key_ex [%i]",
				dEVP_PKEY_new ? 1 : 0, dEVP_PKEY_free ? 1 : 0, dX509_free ? 1 : 0, dRSA_generate_key_ex ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dEVP_PKEY_assign [%i], dRSA_free [%i], dX509_get_serialNumber [%i], dASN1_INTEGER_set [%i]",
				dEVP_PKEY_assign ? 1 : 0, dRSA_free ? 1 : 0, dX509_get_serialNumber ? 1 : 0, dASN1_INTEGER_set ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dX509_gmtime_adj [%i], dX509_set_pubkey [%i], dX509_get_subject_name [%i], dX509_NAME_add_entry_by_txt [%i]",
				dX509_gmtime_adj ? 1 : 0, dX509_set_pubkey ? 1 : 0, dX509_get_subject_name ? 1 : 0, dX509_NAME_add_entry_by_txt ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dX509_set_issuer_name [%i], dX509_sign [%i], dd2i_RSAPrivateKey [%i], di2d_RSAPrivateKey [%i]",
				dX509_set_issuer_name ? 1 : 0, dX509_sign ? 1 : 0, dd2i_RSAPrivateKey ? 1 : 0, di2d_RSAPrivateKey ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dd2i_X509 [%i], di2d_X509 [%i], dX509_get_pubkey [%i], dEVP_PKEY_get1_RSA [%i]",
				dd2i_X509 ? 1 : 0, di2d_X509 ? 1 : 0, dX509_get_pubkey ? 1 : 0, dEVP_PKEY_get1_RSA ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dRSA_size [%i], dRSA_public_encrypt [%i], dRSA_private_decrypt [%i], dERR_print_errors_fp [%i]",
				dRSA_size ? 1 : 0, dRSA_public_encrypt ? 1 : 0, dRSA_private_decrypt ? 1 : 0, dERR_print_errors_fp ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dEVP_CIPHER_CTX_cleanup [%i], dEVP_CIPHER_CTX_init [%i], dEVP_EncryptInit_ex [%i], dEVP_DecryptInit_ex [%i]",
				dEVP_CIPHER_CTX_cleanup ? 1 : 0, dEVP_CIPHER_CTX_init ? 1 : 0, dEVP_EncryptInit_ex ? 1 : 0, dEVP_DecryptInit_ex ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dEVP_DecryptUpdate [%i], dEVP_DecryptFinal_ex [%i], dEVP_EncryptUpdate [%i], dEVP_EncryptFinal_ex [%i]",
				dEVP_DecryptUpdate ? 1 : 0, dEVP_DecryptFinal_ex ? 1 : 0, dEVP_EncryptUpdate ? 1 : 0, dEVP_EncryptFinal_ex ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dSHA256_Init [%i], dSHA256_Update [%i], dSHA256_Final [%i], dSHA512_Init [%i]",
				dSHA256_Init ? 1 : 0, dSHA256_Update ? 1 : 0, dSHA256_Final ? 1 : 0, dSHA512_Init ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dSHA512_Update [%i], dSHA512_Final [%i], dX509_new [%i], dEVP_sha1 [%i]",
				dSHA512_Update ? 1 : 0, dSHA512_Final ? 1 : 0, dX509_new ? 1 : 0, dEVP_sha1 ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dEVP_aes_256_cbc [%i], dBN_new [%i], dBN_set_word [%i], dRSA_new [%i], dBN_free [%i]",
				dEVP_aes_256_cbc ? 1 : 0, dBN_new ? 1 : 0, dBN_set_word ? 1 : 0, dRSA_new ? 1 : 0, dBN_free ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dCRYPTO_num_locks [%i], dCRYPTO_set_locking_callback [%i], dCRYPTO_set_id_callback [%i], dCRYPTO_set_dynlock_create_callback [%i]",
				dCRYPTO_num_locks ? 1 : 0, dCRYPTO_set_locking_callback ? 1 : 0, dCRYPTO_set_id_callback ? 1 : 0, dCRYPTO_set_dynlock_create_callback ? 1 : 0 );

			CLogArg ( "VerifyLibOpenSSLAccess: dCRYPTO_set_dynlock_lock_callback [%i], dCRYPTO_set_dynlock_destroy_callback [%i]",
				dCRYPTO_set_dynlock_lock_callback ? 1 : 0, dCRYPTO_set_dynlock_destroy_callback ? 1 : 0 );

#ifdef MEDIATORDAEMON

            CLogArg ( "VerifyLibOpenSSLAccess: dEVP_PKEY_set1_RSA [%i], dd2i_RSA_PUBKEY [%i], dPEM_read_X509 [%i], dPEM_read_RSAPrivateKey [%i]",
                     dEVP_PKEY_set1_RSA ? 1 : 0, dd2i_RSA_PUBKEY ? 1 : 0, dPEM_read_X509 ? 1 : 0, dPEM_read_RSAPrivateKey ? 1 : 0 );
#endif
#endif
			return false;
		}

		return true;
	}


	void ReleaseLibOpenSSL ()
	{
		CVerb ( "ReleaseLibOpenSSL" );

		openssl_LibInitialized		= false;

        if ( dENGINE_cleanup )
            dENGINE_cleanup ();
        
        if ( dERR_free_strings )
            dERR_free_strings();
        
        if ( dEVP_cleanup )
            dEVP_cleanup ();
        
        if ( dCRYPTO_cleanup_all_ex_data )
            dCRYPTO_cleanup_all_ex_data ();
    
		dEVP_PKEY_new               = 0;

		if ( hLibOpenSSL ) {
			dlclose ( hLibOpenSSL );
			hLibOpenSSL = 0;
		}
	}


#ifdef USE_DYNAMIC_LIB


	bool InitLibOpenSSL ( Instance * env )
	{
		CVerb ( "InitLibOpenSSL" );

		if ( openssl_LibInitialized ) {
			CVerb ( "InitLibOpenSSL: already initialized." );
			return true;
		}

		HMODULE				hLib	= 0;
		bool				ret = false;

		hLib = LocateLoadModule ( MODLIBNAME, 0, env );
		if ( !hLib ) {
#ifdef _WIN32
			CWarnArg ( "InitLibOpenSSL: Loading of " MODLIBNAME " FAILED with error [0x%.8x]", GetLastError () );
#else
			CWarnArg ( "InitLibOpenSSL: Loading of " MODLIBNAME " FAILED with error [0x%.8x]", errno );
#endif
			goto Finish;
		}

		dOpenSSL_version_num				= ( pOpenSSL_version_num ) dlsym ( hLib, "OpenSSL_version_num" );

        if ( !dOpenSSL_version_num ) {
            dOpenSSL_version_num			= ( pOpenSSL_version_num ) dlsym ( hLib, "SSLeay" );

            if ( dOpenSSL_version_num ) {
                unsigned long version = dOpenSSL_version_num ();
                CLogArg ( "InitLibOpenSSL: Version [ 0.%X ]", version );
            }
        }

		dOPENSSL_add_all_algorithms_noconf  = ( pOPENSSL_add_all_algorithms_noconf ) dlsym ( hLib, "OPENSSL_add_all_algorithms_noconf" );
		dOPENSSL_add_all_algorithms_conf    = ( pOPENSSL_add_all_algorithms_conf ) dlsym ( hLib, "OPENSSL_add_all_algorithms_conf" );
		dOpenSSL_add_all_algorithms			= ( pOpenSSL_add_all_algorithms ) dlsym ( hLib, "OpenSSL_add_all_algorithms" );

		dERR_load_ERR_strings               = ( pERR_load_ERR_strings ) dlsym ( hLib, "ERR_load_ERR_strings" );
		dERR_load_crypto_strings            = ( pERR_load_crypto_strings ) dlsym ( hLib, "ERR_load_crypto_strings" );
		dERR_remove_state                   = ( pERR_remove_state ) dlsym ( hLib, "ERR_remove_state" );
		dERR_remove_thread_state            = ( pERR_remove_thread_state ) dlsym ( hLib, "ERR_remove_thread_state" );

		dERR_free_strings                   = ( pERR_free_strings ) dlsym ( hLib, "ERR_free_strings" );
        dENGINE_cleanup                     = ( pENGINE_cleanup ) dlsym ( hLib, "ENGINE_cleanup" );
        
		dCRYPTO_num_locks                   = ( pCRYPTO_num_locks ) dlsym ( hLib, "CRYPTO_num_locks" );
		dCRYPTO_set_locking_callback        = ( pCRYPTO_set_locking_callback ) dlsym ( hLib, "CRYPTO_set_locking_callback" );
		dCRYPTO_set_id_callback             = ( pCRYPTO_set_id_callback ) dlsym ( hLib, "CRYPTO_set_id_callback" );
		dCRYPTO_set_dynlock_create_callback = ( pCRYPTO_set_dynlock_create_callback ) dlsym ( hLib, "CRYPTO_set_dynlock_create_callback" );
		dCRYPTO_set_dynlock_lock_callback   = ( pCRYPTO_set_dynlock_lock_callback ) dlsym ( hLib, "CRYPTO_set_dynlock_lock_callback" );
		dCRYPTO_set_dynlock_destroy_callback = ( pCRYPTO_set_dynlock_destroy_callback ) dlsym ( hLib, "CRYPTO_set_dynlock_destroy_callback" );
        dCRYPTO_cleanup_all_ex_data         = ( pCRYPTO_cleanup_all_ex_data ) dlsym ( hLib, "CRYPTO_cleanup_all_ex_data" );
        
		dBN_free                            = ( pBN_free ) dlsym ( hLib, "BN_free" );
		dBN_new                             = ( pBN_new ) dlsym ( hLib, "BN_new" );
		dBN_set_word                        = ( pBN_set_word ) dlsym ( hLib, "BN_set_word" );
		dRSA_new                            = ( pRSA_new ) dlsym ( hLib, "RSA_new" );

		dEVP_sha1                           = ( pEVP_sha1 ) dlsym ( hLib, "EVP_sha1" );
		dEVP_PKEY_new                       = ( pEVP_PKEY_new ) dlsym ( hLib, "EVP_PKEY_new" );
		dEVP_PKEY_free                      = ( pEVP_PKEY_free ) dlsym ( hLib, "EVP_PKEY_free" );

		dX509_new                           = ( pX509_new ) dlsym ( hLib, "X509_new" );

		dX509_free                          = ( pX509_free ) dlsym ( hLib, "X509_free" );
		dRSA_generate_key                   = ( pRSA_generate_key ) dlsym ( hLib, "RSA_generate_key" );
		dRSA_generate_key_ex                = ( pRSA_generate_key_ex ) dlsym ( hLib, "RSA_generate_key_ex" );
		dEVP_PKEY_assign                    = ( pEVP_PKEY_assign ) dlsym ( hLib, "EVP_PKEY_assign" );
		dRSA_free                           = ( pRSA_free ) dlsym ( hLib, "RSA_free" );

		dASN1_INTEGER_set                   = ( pASN1_INTEGER_set ) dlsym ( hLib, "ASN1_INTEGER_set" );
		dX509_gmtime_adj                    = ( pX509_gmtime_adj ) dlsym ( hLib, "X509_gmtime_adj" );

		dX509_set_pubkey                    = ( pX509_set_pubkey ) dlsym ( hLib, "X509_set_pubkey" );
		dX509_get_subject_name              = ( pX509_get_subject_name ) dlsym ( hLib, "X509_get_subject_name" );
		dX509_NAME_add_entry_by_txt         = ( pX509_NAME_add_entry_by_txt ) dlsym ( hLib, "X509_NAME_add_entry_by_txt" );
		dX509_set_issuer_name               = ( pX509_set_issuer_name ) dlsym ( hLib, "X509_set_issuer_name" );
		dX509_get_serialNumber              = ( pX509_get_serialNumber ) dlsym ( hLib, "X509_get_serialNumber" );

		dX509_sign                          = ( pX509_sign ) dlsym ( hLib, "X509_sign" );
		dd2i_RSAPrivateKey                  = ( pd2i_RSAPrivateKey ) dlsym ( hLib, "d2i_RSAPrivateKey" );
		di2d_RSAPrivateKey                  = ( pi2d_RSAPrivateKey ) dlsym ( hLib, "i2d_RSAPrivateKey" );

		di2d_X509                           = ( pi2d_X509 ) dlsym ( hLib, "i2d_X509" );
		dd2i_X509                           = ( pd2i_X509 ) dlsym ( hLib, "d2i_X509" );
		dX509_get_pubkey                    = ( pX509_get_pubkey ) dlsym ( hLib, "X509_get_pubkey" );
		dEVP_PKEY_get1_RSA                  = ( pEVP_PKEY_get1_RSA ) dlsym ( hLib, "EVP_PKEY_get1_RSA" );
		dRSA_size                           = ( pRSA_size ) dlsym ( hLib, "RSA_size" );

		dRSA_public_encrypt                 = ( pRSA_public_encrypt ) dlsym ( hLib, "RSA_public_encrypt" );
		dRSA_private_decrypt                = ( pRSA_private_decrypt ) dlsym ( hLib, "RSA_private_decrypt" );
		dERR_print_errors_fp                = ( pERR_print_errors_fp ) dlsym ( hLib, "ERR_print_errors_fp" );

		dEVP_CIPHER_CTX_cleanup             = ( pEVP_CIPHER_CTX_cleanup ) dlsym ( hLib, "EVP_CIPHER_CTX_cleanup" );
		dEVP_CIPHER_CTX_init                = ( pEVP_CIPHER_CTX_init ) dlsym ( hLib, "EVP_CIPHER_CTX_init" );
        dEVP_cleanup                        = ( pEVP_cleanup ) dlsym ( hLib, "EVP_cleanup" );

        
		dEVP_PKEY_set1_RSA                  = ( pEVP_PKEY_set1_RSA ) dlsym ( hLib, "EVP_PKEY_set1_RSA" );
		dd2i_RSA_PUBKEY                     = ( pd2i_RSA_PUBKEY ) dlsym ( hLib, "d2i_RSA_PUBKEY" );
		dPEM_read_X509                      = ( pPEM_read_X509 ) dlsym ( hLib, "PEM_read_X509" );
		dPEM_read_RSAPrivateKey             = ( pPEM_read_RSAPrivateKey ) dlsym ( hLib, "PEM_read_RSAPrivateKey" );

		dEVP_EncryptInit_ex                 = ( pEVP_EncryptInit_ex ) dlsym ( hLib, "EVP_EncryptInit_ex" );
		dEVP_DecryptInit_ex                 = ( pEVP_DecryptInit_ex ) dlsym ( hLib, "EVP_DecryptInit_ex" );
		dEVP_DecryptUpdate                  = ( pEVP_DecryptUpdate ) dlsym ( hLib, "EVP_DecryptUpdate" );

		dEVP_DecryptFinal_ex                = ( pEVP_DecryptFinal_ex ) dlsym ( hLib, "EVP_DecryptFinal_ex" );
		dEVP_EncryptUpdate                  = ( pEVP_EncryptUpdate ) dlsym ( hLib, "EVP_EncryptUpdate" );
        dEVP_EncryptFinal_ex                = ( pEVP_EncryptFinal_ex ) dlsym ( hLib, "EVP_EncryptFinal_ex" );
        dEVP_CIPHER_CTX_ctrl                = ( pEVP_CIPHER_CTX_ctrl ) dlsym ( hLib, "EVP_CIPHER_CTX_ctrl" );

        dEVP_aes_256_cbc                    = ( pEVP_aes_256_cbc ) dlsym ( hLib, "EVP_aes_256_cbc" );

        dEVP_aes_256_gcm                    = ( pEVP_aes_256_gcm ) dlsym ( hLib, "EVP_aes_256_gcm" );

		dSHA256_Init                        = ( pSHA256_Init ) dlsym ( hLib, "SHA256_Init" );
		dSHA256_Update                      = ( pSHA256_Update ) dlsym ( hLib, "SHA256_Update" );
		dSHA256_Final                       = ( pSHA256_Final ) dlsym ( hLib, "SHA256_Final" );

		dSHA512_Init                        = ( pSHA512_Init ) dlsym ( hLib, "SHA512_Init" );
		dSHA512_Update                      = ( pSHA512_Update ) dlsym ( hLib, "SHA512_Update" );
		dSHA512_Final                       = ( pSHA512_Final ) dlsym ( hLib, "SHA512_Final" );


        if ( !VerifyLibOpenSSLAccess () ) {
			goto Finish;
		}

		ret = true;

	Finish:
		if ( ret ) {
			hLibOpenSSL             = hLib;
			openssl_LibInitialized  = true;
			CVerb ( "InitLibOpenSSL: successfully initialized access to " MODLIBNAME );
		}
        else {
            CErr ( "InitLibOpenSSL: Failed to initialize " MODLIBNAME );
			ReleaseLibOpenSSL ();
		}

		return ret;
	}


#else

#ifdef _WIN32
#pragma comment ( lib, "libcrypto.lib" )
#endif

	bool InitLibOpenSSL ( Instance * env )
	{
		CVerb ( "InitLibOpenSSL" );

		if ( openssl_LibInitialized ) {
			CVerb ( "InitLibOpenSSL: already initialized." );
			return true;
		}

		dOpenSSL_version_num				= ( pOpenSSL_version_num ) OpenSSL_version_num;

		dOPENSSL_add_all_algorithms_noconf  = ( pOPENSSL_add_all_algorithms_noconf ) OPENSSL_add_all_algorithms_noconf;
		dOPENSSL_add_all_algorithms_conf    = ( pOPENSSL_add_all_algorithms_conf ) OPENSSL_add_all_algorithms_conf;
		dOpenSSL_add_all_algorithms			= ( pOpenSSL_add_all_algorithms ) OpenSSL_add_all_algorithms;

		dERR_load_ERR_strings               = ( pERR_load_ERR_strings ) ERR_load_ERR_strings;
		dERR_load_crypto_strings            = ( pERR_load_crypto_strings ) ERR_load_crypto_strings;
		dERR_remove_state                   = ( pERR_remove_state ) ERR_remove_state;
		dERR_remove_thread_state            = ( pERR_remove_thread_state ) ERR_remove_thread_state;

		dERR_free_strings                   = ( pERR_free_strings ) ERR_free_strings;
        dENGINE_cleanup                     = ( pENGINE_cleanup ) ENGINE_cleanup;

		dCRYPTO_num_locks                   = ( pCRYPTO_num_locks ) CRYPTO_num_locks;
		dCRYPTO_set_locking_callback        = ( pCRYPTO_set_locking_callback ) CRYPTO_set_locking_callback;
		dCRYPTO_set_id_callback             = ( pCRYPTO_set_id_callback ) CRYPTO_set_id_callback;
		dCRYPTO_set_dynlock_create_callback = ( pCRYPTO_set_dynlock_create_callback ) CRYPTO_set_dynlock_create_callback;
		dCRYPTO_set_dynlock_lock_callback   = ( pCRYPTO_set_dynlock_lock_callback ) CRYPTO_set_dynlock_lock_callback;
		dCRYPTO_set_dynlock_destroy_callback    = ( pCRYPTO_set_dynlock_destroy_callback ) CRYPTO_set_dynlock_destroy_callback;

		dBN_free                            = ( pBN_free ) BN_free;
		dBN_new                             = ( pBN_new ) BN_new;
		dBN_set_word                        = ( pBN_set_word ) BN_set_word;
		dRSA_new                            = ( pRSA_new ) RSA_new;

		dEVP_sha1                           = ( pEVP_sha1 ) EVP_sha1;
		dEVP_PKEY_new                       = ( pEVP_PKEY_new ) EVP_PKEY_new;
		dEVP_PKEY_free                      = ( pEVP_PKEY_free ) EVP_PKEY_free;
        dEVP_cleanup                        = ( pEVP_cleanup ) EVP_cleanup;
		dX509_new                           = ( pX509_new ) X509_new;
		dX509_free                          = ( pX509_free ) X509_free;
		dRSA_generate_key                   = ( pRSA_generate_key ) RSA_generate_key;
		dRSA_generate_key_ex                = ( pRSA_generate_key_ex ) RSA_generate_key_ex;
		dEVP_PKEY_assign                    = ( pEVP_PKEY_assign ) EVP_PKEY_assign;
		dRSA_free                           = ( pRSA_free ) RSA_free;

		dASN1_INTEGER_set                   = ( pASN1_INTEGER_set ) ASN1_INTEGER_set;
		dX509_gmtime_adj                    = ( pX509_gmtime_adj ) X509_gmtime_adj;

		dX509_set_pubkey                    = ( pX509_set_pubkey ) X509_set_pubkey;
		dX509_get_subject_name              = ( pX509_get_subject_name ) X509_get_subject_name;
		dX509_NAME_add_entry_by_txt         = ( pX509_NAME_add_entry_by_txt ) X509_NAME_add_entry_by_txt;
		dX509_set_issuer_name               = ( pX509_set_issuer_name ) X509_set_issuer_name;
		dX509_get_serialNumber              = ( pX509_get_serialNumber ) X509_get_serialNumber;


		dX509_sign                          = ( pX509_sign ) X509_sign;
		dd2i_RSAPrivateKey                  = ( pd2i_RSAPrivateKey ) d2i_RSAPrivateKey;
		di2d_RSAPrivateKey                  = ( pi2d_RSAPrivateKey ) i2d_RSAPrivateKey;

		di2d_X509                           = ( pi2d_X509 ) i2d_X509;
		dd2i_X509                           = ( pd2i_X509 ) d2i_X509;
		dX509_get_pubkey                    = ( pX509_get_pubkey ) X509_get_pubkey;
		dEVP_PKEY_get1_RSA                  = ( pEVP_PKEY_get1_RSA ) EVP_PKEY_get1_RSA;
		dRSA_size                           = ( pRSA_size ) RSA_size;

		dRSA_public_encrypt                 = ( pRSA_public_encrypt ) RSA_public_encrypt;
		dRSA_private_decrypt                = ( pRSA_private_decrypt ) RSA_private_decrypt;
		dERR_print_errors_fp                = ( pERR_print_errors_fp ) ERR_print_errors_fp;

		dEVP_CIPHER_CTX_cleanup             = ( pEVP_CIPHER_CTX_cleanup ) EVP_CIPHER_CTX_cleanup;
		dEVP_CIPHER_CTX_init                = ( pEVP_CIPHER_CTX_init ) EVP_CIPHER_CTX_init;

		dEVP_EncryptInit_ex                 = ( pEVP_EncryptInit_ex ) EVP_EncryptInit_ex;
		dEVP_DecryptInit_ex                 = ( pEVP_DecryptInit_ex ) EVP_DecryptInit_ex;
		dEVP_DecryptUpdate                  = ( pEVP_DecryptUpdate ) EVP_DecryptUpdate;

		dEVP_PKEY_set1_RSA                  = ( pEVP_PKEY_set1_RSA ) EVP_PKEY_set1_RSA;
		dd2i_RSA_PUBKEY                     = ( pd2i_RSA_PUBKEY ) d2i_RSA_PUBKEY;
		dPEM_read_X509                      = ( pPEM_read_X509 ) PEM_read_X509;
		dPEM_read_RSAPrivateKey             = ( pPEM_read_RSAPrivateKey ) PEM_read_RSAPrivateKey;

		dEVP_DecryptFinal_ex                = ( pEVP_DecryptFinal_ex ) EVP_DecryptFinal_ex;
		dEVP_EncryptUpdate                  = ( pEVP_EncryptUpdate ) EVP_EncryptUpdate;
		dEVP_EncryptFinal_ex                = ( pEVP_EncryptFinal_ex ) EVP_EncryptFinal_ex;
		dEVP_aes_256_cbc                    = ( pEVP_aes_256_cbc ) EVP_aes_256_cbc;

		dSHA256_Init                        = ( pSHA256_Init ) SHA256_Init;
		dSHA256_Update                      = ( pSHA256_Update ) SHA256_Update;
		dSHA256_Final                       = ( pSHA256_Final ) SHA256_Final;

		dSHA512_Init                        = ( pSHA512_Init ) SHA512_Init;
		dSHA512_Update                      = ( pSHA512_Update ) SHA512_Update;
		dSHA512_Final                       = ( pSHA512_Final ) SHA512_Final;

        if ( !VerifyLibOpenSSLAccess ( ) ) {
            CErr ( "InitLibOpenSSL: Failed to initialize " MODLIBNAME );
			goto Failed;
		}

		openssl_LibInitialized = true;

		CVerb ( "InitLibOpenSSL: successfully initialized access to " MODLIBNAME );
		return true;

	Failed:
		ReleaseLibOpenSSL ();
		return false;
	}
#endif



} // -> namespace environs



