/**
 * Environs Win32 CryptoAPI Implementation
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

#ifdef _WIN32

#include "Environs.Crypt.h"
#include "Environs.Utils.h"
#include "Interop.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Strsafe.h>

#pragma comment ( lib, "Crypt32.lib" )
#define USE_CACHED_HKEY

#define CLASS_NAME	"Environs.Crypt.Win32 . ."


namespace environs
{
#ifndef USE_WIN32_CNG_CRYPT
	pInitEnvironsCrypt		InitEnvironsCryptPlatform			= 0;
#endif

	pInitEnvironsCrypt		InitEnvironsCryptLocksPlatform		= 0;
	pReleaseEnvironsCrypt	ReleaseEnvironsCryptPlatform		= 0;
	pReleaseEnvironsCrypt	ReleaseEnvironsCryptLocksPlatform	= 0;

    bool dPreparePrivateKey ( char ** privKey )
    {
        return true;
    }

	// AES 256-bit blob
	struct AES_256_BLOB {
		BLOBHEADER hdr;
		DWORD keySize;
		BYTE raw [32];
	};

	static const LPWSTR ENVIRONS_KEY_STORE_NAME = L"Environs";

	
	const LPWSTR GetUserEnvironsName ( unsigned int counter )
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


	bool AcquireContext ( HCRYPTPROV * hCSP, unsigned int counter, LPWSTR &keyStore )
	{
		keyStore = ENVIRONS_KEY_STORE_NAME;
		if ( counter > 0 )
			keyStore = GetUserEnvironsName ( counter );

		if ( !keyStore )
			return false;

		if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET ) )
		{ //CRYPT_VERIFYCONTEXT  | CRYPT_NEWKEYSET CRYPT_MACHINE_KEYSET | 
#ifndef NDEBUG
			CErrArg ( "AcquireContext: CryptAcquireContext failed [ %d ].", );
#endif
			CVerb ( "AcquireContext: CryptAcquireContext failed. We're gonna remove the container and try again..." );
			CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_DELETEKEYSET );

			if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET ) )
			{ //CRYPT_VERIFYCONTEXT  | CRYPT_NEWKEYSET
#ifndef NDEBUG
				CErrArg ( "AcquireContext: CryptAcquireContext failed [ %d ].", GetLastError () );
#endif
				if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
				{
#ifndef NDEBUG
					CErrArg ( "AcquireContext: CryptAcquireContext failed [ %d ].", GetLastError () );
#endif
					CVerb ( "AcquireContext: CryptAcquireContext failed. We're gonna remove the container and try again..." );
					CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_DELETEKEYSET );

					if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
					{
						CErrArg ( "AcquireContext: CryptAcquireContext failed [ %d ].", GetLastError () );
						return false;
					}
				}
			}
		}

		return true;
	}


	bool dUpdateKeyAndCert ( char * priv, char * cert )
	{
		return true;
	}
    
#ifdef MEDIATORDAEMON
    
	bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert )
	{
		bool ret = false;
        
		bool						imported	= false;
		HCRYPTPROV					hCSP		= 0;
		HCRYPTKEY					hKey		= 0;
		CERT_PUBLIC_KEY_INFO	*	certPubInfo = 0;
		CERT_INFO					certInfo;
		DWORD						certInfoLen;
		LPWSTR						keyStore	= 0;
		char					*   certData    = 0;
		const char 				*	issuerStr		= "CN=Environs, T=Secure";
		const char 				*	subjStr			= "CN=Environs, T=Secure, Client";
		CRYPT_ALGORITHM_IDENTIFIER	alg;
		
		DWORD						nameLen		= 0;
		BYTE					*	nameSubject	= 0;
		BYTE					*	nameIssuer	= 0;
		BYTE					*	certBin		= (BYTE *) (pub + 4);
        unsigned int				certSize	= *((unsigned int *)pub) & 0xFFFF;

		do
		{
			DWORD certBufferSize = 0;
			if ( !CryptStringToBinaryA ( ( char * ) certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize ) {
				CErr ( "SignCertificate: CryptStringToBinaryA (retrieve size) failed." );
			}
			else {
				BYTE * certBuffer = ( BYTE * ) malloc ( certBufferSize );
				if ( !certBuffer ) {
					CErrArg ( "SignCertificate: Memory alloc for certBuffer failed [%u].", certSize ); break;
				}

				if ( !CryptStringToBinaryA ( ( char * ) certBin, certSize, CRYPT_STRING_ANY, certBuffer, &certBufferSize, NULL, NULL ) || !certBufferSize ) {
					CErr ( "SignCertificate: CryptStringToBinaryA (convert) failed." ); break;
				}
				certBin = certBuffer;
			}

			certInfoLen = sizeof ( CERT_PUBLIC_KEY_INFO );
			if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, ( BYTE * ) certBin, certSize, CRYPT_ENCODE_ALLOC_FLAG, NULL, &certPubInfo, &certInfoLen ) ) {
				imported = true;
			}

			if ( !imported ) {
				certInfoLen = sizeof ( CERT_INFO );
				if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, ( BYTE * ) certBin, certSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &certInfo, &certInfoLen ) ) {
					imported = true;
				}
			}

			if ( !imported )
				break;

			if ( !AcquireContext ( &hCSP, 0, keyStore ) ) {
				unsigned int trys = 3;
				do
				{
					if ( AcquireContext ( &hCSP, trys, keyStore ) )
						break;
					trys--;
				}
				while ( trys > 0 );
			}

			if ( !hCSP ) {
				CErr ( "SignCertificate: AcquireContext failed." ); break;
			}

			/*if ( !CryptImportKey ( hCSP, (BYTE *) key, keySize, NULL, 0, &hKey ) )
			{
			CErr ( "SignCertificate: CryptImportKey failed." ); break;
			}*/

			if ( !CryptGenKey ( hCSP, AT_SIGNATURE, CRYPT_EXPORTABLE, &hKey ) ) {
				CErr ( "SignCertificate: CryptGenKey failed." ); break;
			}

			Zero ( certInfo );

			certInfo.dwVersion = 2;

			BYTE serial [ ] = "\x01\x02\x04\x08\x0F\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
			certInfo.SerialNumber.cbData = 16;
			certInfo.SerialNumber.pbData = serial;

			certInfo.SignatureAlgorithm.pszObjId = szOID_RSA_SHA1RSA;
			certInfo.SignatureAlgorithm.Parameters.cbData = 0;

			CERT_RDN_ATTR nameAttr =
			{
				szOID_COMMON_NAME,
				CERT_RDN_PRINTABLE_STRING,
				( DWORD ) ( sizeof ( issuerStr ) + 1 ),
				( BYTE * ) issuerStr
			};

			CERT_RDN rgRDN [ ] = { 1, &nameAttr };

			CERT_NAME_INFO CertName = {
				1, rgRDN
			};


			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, NULL, &nameLen ) || nameLen <= 0 ) {
				CErr ( "SignCertificate: AcquireContext failed." ); break;
			}

			nameIssuer = ( BYTE* ) malloc ( nameLen );
			if ( !nameIssuer ) {
				CErrArg ( "SignCertificate: Memory alloc for nameIssuer failed [%u].", certSize ); break;
			}

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, nameIssuer, &nameLen ) ) {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameIssuer failed." ); break;
			}

			certInfo.Issuer.cbData = nameLen;
			certInfo.Issuer.pbData = nameIssuer;

			SYSTEMTIME SysTime;
			GetSystemTime ( &SysTime );
			SystemTimeToFileTime ( &SysTime, &certInfo.NotBefore );

			SysTime.wYear += 3;
			SystemTimeToFileTime ( &SysTime, &certInfo.NotAfter );

			nameAttr.pszObjId		= szOID_COMMON_NAME;
			nameAttr.dwValueType	= CERT_RDN_PRINTABLE_STRING;
			nameAttr.Value.cbData	= ( DWORD ) ( sizeof ( subjStr ) + 1 );
			nameAttr.Value.pbData	= ( PBYTE ) subjStr;

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, NULL, &nameLen ) ) {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameSubject len failed." ); break;
			}

			nameSubject = ( BYTE* ) malloc ( nameLen );
			if ( !nameSubject ) {
				CErrArg ( "SignCertificate: Memory alloc for nameSubject failed [%u].", certSize ); break;
			}

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, nameSubject, &nameLen ) ) {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameSubject failed." ); break;
			}

			certInfo.Subject.cbData = nameLen;
			certInfo.Subject.pbData = nameSubject;
			certInfo.SubjectPublicKeyInfo = *certPubInfo;

			certInfo.cExtension = 0;
			certInfo.rgExtension = NULL;

			certInfo.IssuerUniqueId.cbData = 0 ;
			certInfo.SubjectUniqueId.cbData = 0;

			BYTE paraData [ 16 ];
			paraData [ 0 ] = 0x05; paraData [ 1 ] = 0x00;

			alg.pszObjId = szOID_RSA_SHA1RSA;
			alg.Parameters.cbData = 2;
			alg.Parameters.pbData = paraData;

			if ( !CryptSignAndEncodeCertificate ( hCSP, AT_SIGNATURE, X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, ( void* ) &certInfo, &alg, NULL, NULL, ( DWORD * ) &certSize ) ) {
				CErr ( "SignCertificate: CryptSignAndEncodeCertificate length failed." ); break;
			}

			certData = ( char * ) malloc ( certSize + 4 );
			if ( !certData ) {
				CErrArg ( "SignCertificate: Memory alloc for certData failed [%u].", certSize ); break;
			}

			if ( !CryptSignAndEncodeCertificate ( hCSP, AT_SIGNATURE, X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, ( void* ) &certInfo, &alg, NULL, ( PBYTE ) ( certData + 4 ), ( DWORD * ) &certSize ) ) {
				CErr ( "SignCertificate: CryptSignAndEncodeCertificate failed." ); break;
			}

			*( ( unsigned int * ) certData ) = ( 'd' << 16 | certSize | ENVIRONS_CERT_SIGNED_FLAG );
			*cert = ( char * ) certData;
			certData = 0;

			ret = true;
		} 
		while ( 0 );

		if ( !ret ) {
			CErrArg ( "SignCertificate: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( certBin != (BYTE *)(cert + 4)) free ( certBin );
		if ( nameIssuer )	free ( nameIssuer );
		if ( nameSubject )	free ( nameSubject );
		if ( certData )		free ( certData );
		if ( certPubInfo )	LocalFree ( certPubInfo );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );

		return ret;
	}
#endif
	   

	bool dGenerateCertificate ( char ** priv, char ** pub )
	{
		bool ret = false;

		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		char		*	privKey		= 0;
		char		*	pubKey		= 0;
		BYTE		*	certSubj	= 0;
		DWORD			certSubjLen;
		DWORD			length;
		PCCERT_CONTEXT	certContext = 0;
		LPCTSTR			certStr		= L"CN=Environs, T=Secure";

		CERT_NAME_BLOB				certName;
		CRYPT_KEY_PROV_INFO			keyProv;
		CRYPT_ALGORITHM_IDENTIFIER	alg;
		LPWSTR			keyStore	= 0;

		do
		{
			if ( !AcquireContext ( &hCSP, 0, keyStore ) ) {
				unsigned int trys = 3;
				do 
				{
					if ( AcquireContext ( &hCSP, trys, keyStore ) )
						break;
					trys--;
				} 
				while ( trys > 0 );
			}

			if ( !hCSP ) {
				CErr ( "GenerateCertificate: AcquireContext failed." ); break;
			}

			if ( !CryptGenKey ( hCSP, CALG_RSA_KEYX, (ENVIRONS_DEVICES_KEYSIZE << 16) | CRYPT_EXPORTABLE, &hKey ) ) {//0x08000000 // AT_KEYEXCHANGE
				CErr ( "GenerateCertificate: CryptGenKey failed." ); break;
			}

			certSubjLen = 0;
			if ( !CertStrToName ( X509_ASN_ENCODING, certStr, CERT_X500_NAME_STR, NULL, 0, &certSubjLen, NULL ) ) {
				CErr ( "GenerateCertificate: CertStrToName length failed." ); break;
			}

			certSubj = ( BYTE * ) malloc ( certSubjLen );
			if ( !certSubj ) {
				CErr ( "GenerateCertificate: Memory alloc for pubKeyInfo failed." ); break;
			}

			if ( !CertStrToName ( X509_ASN_ENCODING, certStr, CERT_X500_NAME_STR, NULL, certSubj, &certSubjLen, NULL ) ) {
				CErr ( "GenerateCertificate: CertStrToName failed." ); break;
			}

			Zero ( certName );
			certName.cbData = certSubjLen;
			certName.pbData = certSubj;

			Zero ( keyProv );
			keyProv.pwszContainerName	= keyStore;
			keyProv.pwszProvName		= 0;
			keyProv.dwProvType			= PROV_RSA_FULL;
			keyProv.dwFlags				= ( ENVIRONS_DEVICES_KEYSIZE << 16 ) | CERT_SET_KEY_CONTEXT_PROP_ID; // CRYPT_MACHINE_KEYSET;
			keyProv.cProvParam			= 0;
			keyProv.rgProvParam			= NULL;
			keyProv.dwKeySpec			= AT_KEYEXCHANGE;

			Zero ( alg );
			alg.pszObjId = szOID_RSA_SHA1RSA;  /// We should switch to sha2

			SYSTEMTIME validTime;
			GetSystemTime ( &validTime );
			validTime.wYear += 3;

			certContext = CertCreateSelfSignCertificate ( hCSP, &certName, ( ENVIRONS_DEVICES_KEYSIZE << 16 ), &keyProv, &alg, 0, &validTime, 0 );
			if ( !certContext ) {
				CErr ( "GenerateCertificate: CertCreateSelfSignCertificate length failed." ); break;
			}

			length = certContext->cbCertEncoded;
			pubKey = ( char * ) malloc ( length + 4 );
			if ( !pubKey ) {
				CErr ( "GenerateCertificate: Memory alloc for pubKey failed." ); break;
			}

			memcpy ( pubKey + 4, certContext->pbCertEncoded, length );

			*( ( unsigned int * ) pubKey ) = ( ( 'd' << 16 ) | length | ENVIRONS_CERT_SIGNED_FLAG );

			length = 0;
			if ( !CryptExportKey ( hKey, 0, PRIVATEKEYBLOB, 0, NULL, &length ) ) {
				CErr ( "GenerateCertificate: CryptExportKey PRIVATEKEYBLOB length failed." ); break;
			}

			privKey = ( char * ) malloc ( length + 4 );
			if ( !privKey ) {
				CErr ( "GenerateCertificate: Memory alloc for privKey failed." ); break;
			}
			if ( !CryptExportKey ( hKey, NULL, PRIVATEKEYBLOB, 0, ( PBYTE ) ( privKey + 4 ), &length ) ) {
				CErr ( "GenerateCertificate: CryptExportKey PRIVATEKEYBLOB failed." ); break;
			}
			*( ( unsigned int * ) privKey ) = length;

			*priv = privKey;
			privKey = 0;

			*pub = pubKey;
			pubKey = 0;
			ret = true;
		} 
		while ( 0 );
		
		if ( !ret ) {
			DWORD err = GetLastError ( );
			CErrArg ( "GenerateCertificate: Last error [ 0x%08x ] / [ %u ]", err, err );
		}

		if ( certSubj )	free ( certSubj );
		if ( privKey )	free ( privKey );
		if ( pubKey )	free ( pubKey );
		if ( certContext )	CertFreeCertificateContext ( certContext );
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );

		return ret;
	}

        
	bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
	{
		if ( !cert || !msg || !msgLen ) {
			CErrID ( "EncryptMessage: Called with at least one null argument." );
			return false;
		}

		unsigned int certProp = *( ( unsigned int * ) cert );
		unsigned int certSize = certProp & 0xFFFF;

		/// Determine format of certificate
		unsigned int format = ( certProp >> 16 ) & 0xFF;
        
		CVerbArgID ( "EncryptMessage: Encrypting buffer sized [ %i ]", *msgLen );

//		CVerbArgID ( "EncryptMessage: Cert size [%u] [%s]", certSize, ConvertToHexSpaceString ( cert + 4, certSize ) );
//
//#ifdef DEBUGVERB
//		char c = cert [certSize]; cert [certSize] = 0;
//		CVerbArg ( "EncryptMessage: Cert raw [%s]", cert + 4 );
//		cert [certSize] = c;
//#endif
		bool ret = false;

		bool						imported	= false;
		HCRYPTPROV					hCSP		= 0;
		HCRYPTKEY					hKey		= 0;
		CERT_PUBLIC_KEY_INFO	*	certPubInfo = 0;
		CERT_INFO				*	certInfo	= 0;
		DWORD						certInfoLen;
		//char					*	ciphers		= 0;
		char					*	reverseBuffer = 0;
		BYTE					*	certBin		= ( BYTE * ) ( cert + 4 );
		BYTE					*	certBuffer	= 0;

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) {
				CErrID ( "EncryptMessage: CryptAcquireContext failed." ); break;
			}

			if ( format == 'p' ) {
				DWORD certBufferSize = 0;
				if ( !CryptStringToBinaryA ( ( char * ) certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize )
				{
					CErrID ( "EncryptMessage: CryptStringToBinaryA (retrieve size) failed." ); break;
				}

				certBuffer = ( BYTE * ) malloc ( certBufferSize );
				if ( !certBuffer ) {
					CErrArgID ( "EncryptMessage: Memory alloc failed [ %u ].", certBufferSize ); break;
				}

				if ( !CryptStringToBinaryA ( ( char * ) certBin, certSize, CRYPT_STRING_ANY, certBuffer, &certBufferSize, NULL, NULL ) || !certBufferSize )
				{
					CErrID ( "EncryptMessage: CryptStringToBinaryA (convert) failed." ); break;
				}
				certBin = certBuffer;
			}

			certInfoLen = sizeof ( CERT_PUBLIC_KEY_INFO );
			if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, ( BYTE * ) certBin, certSize, CRYPT_ENCODE_ALLOC_FLAG, NULL, &certPubInfo, &certInfoLen ) )
			{
				if ( !CryptImportPublicKeyInfo ( hCSP, X509_ASN_ENCODING, certPubInfo, &hKey ) ) {
					CErrID ( "EncryptMessage: CryptImportPublicKeyInfo (1) failed." );
				}
				else imported = true;
			}

			if ( !imported ) {
				certInfoLen = sizeof ( CERT_INFO );
				if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, ( BYTE * ) certBin, certSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &certInfo, &certInfoLen ) )
				{
					if ( !CryptImportPublicKeyInfo ( hCSP, X509_ASN_ENCODING, &certInfo->SubjectPublicKeyInfo, &hKey ) ) {
						CErrID ( "EncryptMessage: CryptImportPublicKeyInfo (2) failed." );
					}
					else imported = true;
				}
			}

			if ( !imported ) {
				if ( !CryptImportKey ( hCSP, ( BYTE * ) certBin, certSize, NULL, 0, &hKey ) ) {
					CErrID ( "EncryptMessage: CryptImportKey (3) failed." ); break;
				}
				else imported = true;
			}

			int pad = 0;
			//if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
			pad = CRYPT_OAEP;
			//else
			//	pad = 0;

			DWORD ciphersLen = ( DWORD ) *msgLen;

			DWORD reqSize = ciphersLen;
			if ( !CryptEncrypt ( hKey, 0, TRUE, pad, 0, &reqSize, reqSize ) || !reqSize )
			{
				CErrID ( "EncryptMessage: CryptEncrypt retrieve cipher text size failed ." ); break;
			}

			reverseBuffer = ( char * ) malloc ( reqSize );
			if ( !reverseBuffer ) {
				CErrID ( "EncryptMessage: Allocation of reverse buffer failed ." ); break;
			}
			memcpy ( reverseBuffer, msg, ciphersLen );

			if ( !CryptEncrypt ( hKey, 0, TRUE, pad, ( BYTE * ) reverseBuffer, &ciphersLen, reqSize ) )
			{
				CErrID ( "EncryptMessage: CryptEncrypt failed ." ); break;
			}

			for ( unsigned int i = 0; i < ciphersLen; ++i )
				msg [ i ] = reverseBuffer [ ciphersLen - 1 - i ];

			*msgLen = ciphersLen;
			ret = true;

			CVerbArgID ( "EncryptMessage: Encrypted message size [ %i ]", ciphersLen );
		} 
		while ( 0 );

		if ( !ret ) {
			CErrArgID ( "EncryptMessage: Last error [ 0x%08x ]", GetLastError ( ) );
		}

		free_n ( certBuffer );
		free_n ( reverseBuffer );
		if ( certInfo )		LocalFree ( certInfo );
		if ( certPubInfo )	LocalFree ( certPubInfo );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );

		return ret;
	}

    
	bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
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
		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		char		*	plainText	= 0;
		DWORD			plainSize	= 0;
        
#ifndef MEDIATORDAEMON
        key += 4;
#endif
		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) 
			{
				CErr ( "DecryptMessage: CryptAcquireContext failed." ); break;
			}
			
			if ( !CryptImportKey ( hCSP, (BYTE *) key, keySize, NULL, 0, &hKey ) )
			{
				CErr ( "DecryptMessage: CryptImportKey failed." ); break;
			}

			plainText = (char *) malloc ( msgLen + 2 );
			if ( !plainText ) {
				CErrArg ( "DecryptMessage: Memory allocation [%i bytes] failed.", msgLen ); break;
			}

			for ( unsigned int i = 0; i < msgLen; i++ )
				plainText [i] = msg [msgLen - 1 - i];
            
			plainSize = msgLen;
			if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_OAEP, (BYTE *) plainText, &plainSize ) && plainSize )
				success = true;
			else
			{
				CVerb ( "DecryptMessage: CryptDecrypt failed with OAEP padding." );

				plainSize = msgLen;
				if ( CryptDecrypt ( hKey, NULL, TRUE, 0, (BYTE *) plainText, &plainSize ) && plainSize )
					success = true;
				else
				{
					CVerb ( "DecryptMessage: CryptDecrypt failed without specific padding flags." );

					plainSize = msgLen;
					if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_DECRYPT_RSA_NO_PADDING_CHECK, (BYTE *) plainText, &plainSize ) && plainSize )
						success = true;
					else {
						CErr ( "DecryptMessage: CryptDecrypt failed with no padding check." ); break;
					}
				}
			}
			
			if ( success ) {
				*decryptedSize = plainSize;
				plainText [plainSize] = 0;
				CVerbVerbArg ( "DecryptMessage: size [ %u ] ", plainSize );

				*decrypted = plainText;
				plainText = 0;
			}
		}
		while ( 0 );

		if ( !success ) {
			CErrArg ( "DecryptMessage: Last error [ 0x%08x ]", GetLastError ( ) );
		}

		if ( plainText )	free ( plainText );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
		if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to release lock." );
			success = false;
		}
#endif
		return success;
    }


	bool dDecryptMessageWithKeyHandles ( HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !hKey || !msg || !msgLen || !decrypted || !decryptedSize ) {
			CErr ( "DecryptMessageWithKeyHandles: Called with at least one null argument." );
			return false;
		}
		CVerbArg ( "DecryptMessageWithKeyHandles: Decrypting msg of size [ %i ]", msgLen );

		bool success = false;

		char		*	plainText = 0;
		DWORD			plainSize = 0;

		do
		{
			plainText = ( char * ) malloc ( msgLen + 2 );
			if ( !plainText ) {
				CErrArg ( "DecryptMessageWithKeyHandles: Memory allocation [ %i bytes ] failed.", msgLen ); break;
			}

			for ( unsigned int i = 0; i < msgLen; i++ )
				plainText [ i ] = msg [ msgLen - 1 - i ];

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_CLIENT_ACCESS
			if ( pthread_mutex_lock ( &privKeyMutex ) ) {
				CErr ( "DecryptMessageWithKeyHandles: Failed to acquire lock." );
				break;
			}
#endif
			plainSize = msgLen;
			if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_OAEP, ( BYTE * ) plainText, &plainSize ) && plainSize )
				success = true;
			else
			{
				CVerbArg ( "DecryptMessageWithKeyHandles: CryptDecrypt failed with OAEP padding [ 0x%08x ].", GetLastError () );

				plainSize = msgLen;
				if ( CryptDecrypt ( hKey, NULL, TRUE, 0, ( BYTE * ) plainText, &plainSize ) && plainSize )
					success = true;
				else
				{
					CVerbArg ( "DecryptMessageWithKeyHandles: CryptDecrypt failed without specific padding flags [ 0x%08x ].", GetLastError () );

					plainSize = msgLen;
					if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_DECRYPT_RSA_NO_PADDING_CHECK, ( BYTE * ) plainText, &plainSize ) && plainSize )
						success = true;
					else {
						CErrArg ( "DecryptMessageWithKeyHandles: CryptDecrypt failed with no padding check [ 0x%08x ].", GetLastError () );
					}
				}
			}

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_CLIENT_ACCESS
			if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
				CErr ( "DecryptMessageWithKeyHandles: Failed to release lock." );
				success = false;
			}
#endif
			if ( !success )
				break;

			*decryptedSize = plainSize;
			plainText [ plainSize ] = 0;
			CVerbVerbArg ( "DecryptMessageWithKeyHandles: size [%u] ", plainSize );

			*decrypted = plainText;
			plainText = 0;
		}
		while ( 0 );

		if ( !success ) {
			CErrArg ( "DecryptMessageWithKeyHandles: Last error [ 0x%08x ]", GetLastError () );
		}

		free_n ( plainText );

		return success;
	}

    
    void dReleaseCert ( int deviceID )
    {
    }

    
    void dAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
    }

    
	void dAESDisposeKeyContext ( AESContext * ctx )
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

#ifdef USE_CACHED_HKEY
		if ( ctx->decCtx ) CryptDestroyKey ( ( HCRYPTKEY ) ctx->decCtx );
		if ( ctx->encCtx ) CryptDestroyKey ( ( HCRYPTKEY ) ctx->encCtx );
		if ( ctx->keyCtx ) CryptReleaseContext ( ( HCRYPTPROV ) ctx->keyCtx, 0 );
#else
		if ( ctx->encCtx ) free ( ctx->encCtx );
#endif
		memset ( ctx, 0, sizeof ( AESContext ) );
	}

    
	bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
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
		bool ret = false;

		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKeyEnc		= 0;
		HCRYPTKEY		hKeyDec		= 0;
		HCRYPTHASH		hHash       = 0;
		DWORD			blobLen		= 0;

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptAcquireContext failed." ); break;
			}

			if ( !CryptCreateHash ( hCSP, CALG_SHA_256, 0, 0, &hHash ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptCreateHash failed." ); break;
			}

			if ( !CryptHashData ( hHash, ( const BYTE * ) key, AES_SHA256_KEY_LENGTH, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptHashData failed." ); break;
			}

			/* Encryption key */
			//if ( !CryptDeriveKey ( hCSP, CALG_AES_256, hHash, 0x00800000 /*128 bit*/, &hKeyEnc ) )
			if ( !CryptDeriveKey ( hCSP, CALG_AES_256, hHash, 0x01000000 /*256 bit*/, &hKeyEnc ) )//CRYPT_EXPORTABLE | 
			{
				CErr ( "AESDeriveKeyContexts: CryptDeriveKey failed." ); break;
			}

			/*DWORD cryptMode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKeyEnc, KP_MODE, (BYTE*) &cryptMode, 0 ) )
			{
			CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}*/

#ifdef ENABLE_AES_GCM
            DWORD mode = CRYPT_MODE_GCM;
            if ( !CryptSetKeyParam ( hKeyEnc, KP_MODE, ( BYTE* ) &mode, 0 ) )
            {
                CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
            }
#else
			DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKeyEnc, KP_MODE, ( BYTE* ) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKeyEnc, KP_PADDING, ( BYTE* ) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}
#endif
			/* Encryption key */

			/* Decryption key */
			if ( !CryptDeriveKey ( hCSP, CALG_AES_256, hHash, 0x01000000 /*256 bit*/, &hKeyDec ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptDeriveKey failed." ); break;
			}

			/*DWORD cryptMode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKeyDec, KP_MODE, (BYTE*) &cryptMode, 0 ) )
			{
			CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}*/

#ifdef ENABLE_AES_GCM
            mode = CRYPT_MODE_GCM;
            if ( !CryptSetKeyParam ( hKeyDec, KP_MODE, ( BYTE* ) &mode, 0 ) )
            {
                CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
            }
#else
            mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKeyDec, KP_MODE, ( BYTE* ) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKeyDec, KP_PADDING, ( BYTE* ) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}
#endif
			/* Decryption key */

			DWORD blobLenLen = sizeof ( DWORD );
			if ( !CryptGetHashParam ( hHash, HP_HASHSIZE, ( BYTE * ) &blobLen, &blobLenLen, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptGetHashParam HP_HASHSIZE failed." ); break;
			}

			blob = ( char * ) malloc ( blobLen );
			if ( !blob ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for hashed blob." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, ( BYTE * ) blob, &blobLen, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptGetHashParam HP_HASHVAL failed." ); break;
			}

			CVerbVerbArg ( "AESDeriveKeyContexts: AES key [ %s ]", ConvertToHexSpaceString ( blob, AES_SHA256_KEY_LENGTH ) );

#ifdef USE_CACHED_HKEY
			ctx->keyCtx = (char *) hCSP;
			hCSP = 0;
			ctx->encCtx = (char *) hKeyEnc;
			ctx->decCtx = (char *) hKeyDec;
			ctx->size = blobLen;
			hKeyEnc = 0; hKeyDec = 0;
#else
			ctx->encCtx = blob;
			ctx->decCtx = blob;
			ctx->size = blobLen;
			blob = 0;		
#endif
			ret = true;
		} 
		while ( 0 );

		if ( hHash )	CryptDestroyHash ( hHash );
		if ( hKeyEnc )	CryptDestroyKey ( hKeyEnc );
		if ( hKeyDec )	CryptDestroyKey ( hKeyDec );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );

		if ( blob )		free ( blob );
		
		return ret;
    }
    
    
    /*
     * Encrypt *len bytes of data
     * All data going in & out is considered binary (unsigned char[])
     */
	bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** cipher )
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

        /*************************************
         * Windows Crypt
         *
         */
#   ifdef USE_CACHED_HKEY
		HCRYPTKEY		hKey		= ( HCRYPTKEY ) ctx->encCtx;
#   else
		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		AES_256_BLOB blob = {
			{ PLAINTEXTKEYBLOB, CUR_BLOB_VERSION, 0, CALG_AES_256 },
			32,
			{ },
		};
		memcpy ( &blob.raw, ctx->encCtx, ctx->size );
#   endif

#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#		ifdef MEASURE_LOCK_ACQUIRE
		if ( !LockAcquireA ( ctx->encLock, "AESEncrypt" ) ) {
#		else
		if ( pthread_mutex_lock ( &ctx->encLock ) ) {
#		endif
			CErrID ( "AESEncrypt: Failed to acquire mutex." );
			return false;
		}
#   endif
        
		do
		{
#   ifndef USE_CACHED_HKEY
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErrID ( "AESEncrypt: CryptAcquireContext failed." ); break;
			}

			if ( !CryptImportKey ( hCSP, ( BYTE * ) &blob, sizeof ( blob ), NULL, 0, &hKey ) )
			{
				CErrID ( "AESEncrypt: CryptImportKey failed." ); break;
			}
#   endif
			/*DWORD param = 0;
			DWORD paramSize = sizeof(param);
			if ( !CryptGetKeyParam ( hKey, KP_BLOCKLEN, (BYTE*) &param, &paramSize, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptGetKeyParam KP_BLOCKLEN failed." ); break;
			}
			CVerbVerbArgID ( "AESEncrypt: Blocklen [%u]", param );*/

			/*DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptSetKeyParam failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKey, KP_PADDING, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptSetKeyParam failed." ); break;
			}*/

			/*DWORD reqSize = (DWORD) *bufferLen;
			if ( !CryptEncrypt ( hKey, NULL, TRUE, 0, 0, &reqSize, reqSize ) || !reqSize )
			{
				CErrID ( "AESEncrypt: CryptEncrypt retrieve cipher text size failed ." ); break;
			}*/
			DWORD reqSize = ( DWORD ) *bufferLen;
			reqSize += AES_256_BLOCK_SIZE;

			ciphers = ( char * ) malloc ( reqSize + 21 );
			if ( !ciphers ) {
				CErrArgID ( "AESEncrypt: Memory allocation [ %i bytes ] failed.", reqSize ); break;
			}

			BUILD_IV_128 ( ciphers + 4 );

			if ( !CryptSetKeyParam ( hKey, KP_IV, ( BYTE* ) ( ciphers + 4 ), 0 ) ) {
				CErrID ( "AESEncrypt: CryptSetKeyParam KP_IV failed." ); break;
			}

			DWORD ciphersLen = ( DWORD ) *bufferLen;
			cipherStart = ciphers + 20;
			memcpy ( cipherStart, buffer, ciphersLen );

			if ( !CryptEncrypt ( hKey, NULL, TRUE, 0, ( BYTE * ) cipherStart, &ciphersLen, reqSize ) )
			{
				CErrID ( "AESEncrypt: CryptEncrypt failed." ); break;
			}

			ciphersSize = ciphersLen;
			success = true;
		}
		while ( 0 );

#   ifndef USE_CACHED_HKEY
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
#   endif
        
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_unlock ( &ctx->encLock ) ) {
			CErrID ( "AESEncrypt: Failed to release mutex." );
			success = false;
		}
#   endif

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

		if ( ciphers ) free ( ciphers );
		return success;
    }

    
	bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** decrypted )
	{
		CVerbVerb ( "AESDecrypt" );

		if ( !ctx || !ctx->decCtx || !buffer || !bufferLen || *bufferLen < 36 || !decrypted ) {
			CErr ( "AESDecrypt: Called with at least one NULL argument or encrypted packet is < 36." ); return false;
		}

		bool			ret				= false;
		char		*	decrypt			= 0;
        char        *   IV              = buffer + 4;
		unsigned int	decryptedBufSize = *bufferLen + AES_SHA256_KEY_LENGTH + 5;
        int             deviceID        = ctx->deviceID;

#ifdef ENABLE_AES_GCM
        char        *   aesTag          = IV + 16;
        char        *   aad             = aesTag + 16;
#endif

		//CVerbVerbArgID ( "AESDecrypt: IV [%s]", ConvertToHexSpaceString ( IV, 16 ) );
		
		//CVerbVerbArgID ( "AESDecrypt: [%s]", ConvertToHexSpaceString ( buffer, *bufferLen ) );
				
		DWORD			decryptedSize	= 0;
#ifdef USE_CACHED_HKEY
		HCRYPTKEY		hKey			= ( HCRYPTKEY ) ctx->decCtx;
#   else
		HCRYPTKEY		hKey			= 0;
		HCRYPTPROV		hCSP			= 0;
		
		AES_256_BLOB blob = {
			{ PLAINTEXTKEYBLOB, CUR_BLOB_VERSION, 0, CALG_AES_256 },
			32,
			{ },
		};
		memcpy ( &blob.raw, ctx->decCtx, ctx->size );
#endif

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#		ifdef MEASURE_LOCK_ACQUIRE
		if ( !LockAcquireA ( ctx->decLock, "AESDecrypt" ) ) {
#		else
		if ( pthread_mutex_lock ( &ctx->decLock ) ) {
#		endif
			CErr ( "AESDecrypt: Failed to acquire mutex." );
			return false;
	}
#endif
		do
		{
#ifndef USE_CACHED_HKEY
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErrID ( "AESDecrypt: CryptAcquireContext failed." ); break;
			}

			if ( !CryptImportKey ( hCSP, ( BYTE * ) &blob, sizeof ( blob ), NULL, 0, &hKey ) )
			{
				CErrID ( "AESDecrypt: CryptImportKey failed." ); break;
			}

			/*DWORD cryptMode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &cryptMode, 0 ) )
			{
				CErrID ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
				}*/
#endif
			/*DWORD param = 0;
			DWORD paramSize = sizeof(param);
			if ( !CryptGetKeyParam ( hKey, KP_BLOCKLEN, (BYTE*) &param, &paramSize, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptGetKeyParam KP_BLOCKLEN failed." ); break;
			}
			CVerbVerbArgID ( "AESDecrypt: Blocklen [%u]", param );*/

			/*DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_MODE failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKey, KP_PADDING, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_PADDING failed." ); break;
			}*/

			decrypt	= ( char * ) malloc ( decryptedBufSize + 2 );
			if ( !decrypt ) {
				CErrArgID ( "AESDecrypt: Memory allocation [ %i bytes ] failed.", decryptedBufSize ); break;
			}

			if ( !CryptSetKeyParam ( hKey, KP_IV, ( BYTE * ) IV, 0 ) ) {
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_IV failed." ); break;
			}

			decryptedSize = *bufferLen - 20;
			memcpy ( decrypt, buffer + 20, decryptedSize );

			if ( !CryptDecrypt ( hKey, NULL, TRUE, 0, ( BYTE * ) decrypt, &decryptedSize ) ) {
				CErrID ( "AESDecrypt: CryptDecrypt failed." ); break;
			}

			ret = true;
		} 
		while ( 0 );

		if ( !ret ) {
			DWORD err = GetLastError ( );
			CErrArgID ( "AESDecrypt: Last error [ 0x%08x ] [ %u ]", err, err );
		}

#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_unlock ( &ctx->decLock ) ) {
			CErrID ( "AESDecrypt: Failed to release mutex." );
			ret = false;
		}
#endif

#ifndef USE_CACHED_HKEY
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
#endif
		if ( ret ) {
			if ( decryptedSize >= decryptedBufSize ) {
				CErrArgID ( "AESDecrypt: Decrypted message size [ %u ] > buffer size [ %u ]", decryptedSize, decryptedBufSize );
				ret = false;
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
		return ret;
    }
        

	/**
	*	SHAHashCreate: Creates a SHA512 hash for a given message, i.e. used to hash user passwords.
	*
	*/
	bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashCreate" );

		if ( !msg || !hash || !xchLen ) {
			CErr ( "SHAHashCreate: Invalid params." );
            return false;
		}
        
		bool		ret		= false;
		char	*	blob	= 0;
		
		HCRYPTPROV			hCSP		= 0;
		HCRYPTHASH			hHash       = 0;
		DWORD				blobLen		= 0;
		HCRYPTKEY			hSessKey	= 0;

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErr ( "SHAHashCreate: CryptAcquireContext failed." ); break;
			}

			//ALG_ID  algorithmID = CALG_SHA1; CALG_SHA_256
			if ( !CryptCreateHash ( hCSP, CALG_SHA_512, 0, 0, &hHash ) )
			{
				CErr ( "SHAHashCreate: CryptCreateHash failed." ); break;
			}

			if ( !CryptHashData ( hHash, (const BYTE *) msg, *xchLen, 0 ) )
			{
				CErr ( "SHAHashCreate: CryptHashData failed." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, 0, &blobLen, 0 ) || !blobLen )
			{
				CErr ( "SHAHashCreate: Get hash value length failed." ); break;
			}

			blob = (char *) malloc ( blobLen + 1 );
			if ( !blob ) {
				CErr ( "SHAHashCreate: Failed to allocate memory for hashed blob." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, (BYTE*) blob, &blobLen, 0 ) )
			{
				CErr ( "SHAHashCreate: Retrieve hash value failed" ); break;
			}
			blob [ blobLen ] = 0;

			*hash = blob;
			blob = 0;
			*xchLen = blobLen;
			ret = true;

			if ( !ret ) {
				CErrArg ( "SHAHashCreate: Last error [0x%08x]", GetLastError ( ) );
			}
		}
		while ( 0 );

		if ( hSessKey ) CryptDestroyKey ( hSessKey );
		if ( blob )		free ( blob );
		if ( hHash )	CryptDestroyHash ( hHash );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
		
		return ret;
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

		int certSize = 0;
		char * certBin = LoadBinary ( pathFile, &certSize );
		if ( !certBin ) {
			CErrArg ( "LoadPublicCertificate: Failed to load [ %s ]", pathFile );
			return false;
		}
        
		if ( !certSize ) {
			CErrArg ( "LoadPublicCertificate: Invalid (Zero) sized file loaded [ %s ]", pathFile );
			free ( certBin );
			return false;
		}

		BYTE	*	certBuffer = 0;

		do
		{
			DWORD certBufferSize = 0;

			if ( !CryptStringToBinaryA ( certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize )
			{
				CErr ( "LoadPublicCertificated: CryptStringToBinaryA (retrieve size) failed." ); break;
			}

			certBuffer = (BYTE *)malloc ( certBufferSize + 4 );
			if ( !certBuffer ) {
				CErrArg ( "LoadPublicCertificated: Memory alloc failed [%u].", certBufferSize + 4 ); break;
			}

			if ( !CryptStringToBinaryA ( certBin, certSize, CRYPT_STRING_ANY, certBuffer + 4, &certBufferSize, NULL, NULL ) || !certBufferSize )
			{
				CErr ( "LoadPublicCertificated: CryptStringToBinaryA (convert) failed." ); break;
			}
			
			*((unsigned int *) certBuffer) = (('d' << 16) | certBufferSize );
			*cert = (char *) certBuffer;
			certBuffer = 0; 
			ret = true;
		}
		while ( 0 );
		
		if ( !ret ) {
			CErrArg ( "LoadPublicCertificated: Last error [ 0x%08x ]", GetLastError ( ) );
		}

		if ( certBuffer )     free ( certBuffer );
		if ( certBin )   free ( certBin );

		return ret;
	}

	
	bool LoadPrivateKey ( const char * pathFile, char ** key, unsigned int * keySize )
	{
		if ( !pathFile ) {
			CErr ( "LoadPrivateKey: Called with NULL argument for path and file name" ); return false;
		}

		bool ret = false;

		CLogArg ( "LoadPrivateKey: [%s]", pathFile );
		
		int size = 0;
		char * keyBin = LoadBinary ( pathFile, &size );
		if ( !keyBin ) {
			CErrArg ( "LoadPrivateKey: Failed to load [%s]", pathFile );
			return false;
		}

		if ( !size ) {
			CErrArg ( "LoadPrivateKey: Invalid (Zero) sized file [%s]", pathFile );
			free ( keyBin );
			return false;
		}

		BYTE		*	keyBlob		= 0;
		DWORD			keyBlobSize = 0;
		BYTE		*	keyData		= 0;
		DWORD			keyDataSize = 0;
		char		*	keyRet		= 0;

		// PEM - Base64 to DER
		if ( !CryptStringToBinaryA ( keyBin, size, CRYPT_STRING_BASE64HEADER, 0, &keyDataSize, NULL, NULL) || !keyDataSize )
		{
			CErr ( "LoadPrivateKey: CryptStringToBinaryA (retrieve size)" );
			goto Finish;
		}

		keyData = (BYTE *) malloc ( keyDataSize );
		if ( !keyData || !CryptStringToBinaryA ( keyBin, size, CRYPT_STRING_ANY, keyData, &keyDataSize, NULL, NULL) || !keyDataSize )
		{
			CErr ( "LoadPrivateKey: CryptStringToBinaryA (convert)" );
			goto Finish;
		}

		if ( !CryptDecodeObjectEx ( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, keyData, keyDataSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &keyBlob, &keyBlobSize ) )
		{
			CErr ( "LoadPrivateKey: CryptDecodeObjectEx (retrieve size)" );
			goto Finish;
		}

		keyRet = (char *)malloc ( keyBlobSize );
		if ( !keyRet ) {
			CErr ( "LoadPrivateKey: Failed to allocate memory for key." );
			goto Finish;
		}

		memcpy ( keyRet, keyBlob, keyBlobSize );
		*key = keyRet;
		keyRet = 0;
		*keySize = keyBlobSize;


		ret = true;

	Finish:

		if ( !ret ) {
			CErrArg ( "LoadPrivateKey: Last error [ 0x%08x ]", GetLastError ( ) );
		}

		if ( keyData )	free ( keyData );
		if ( keyBlob )	LocalFree ( keyBlob );
		if ( keyRet )	free ( keyRet );
		if ( keyBin )	free ( keyBin );

		return ret;
	}
#endif
    
}


#endif