/**
 * Environs.crypt.ios.mm
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

#import "Environs.iOSX.h"
#include "Environs.Crypt.h"


#import <SystemConfiguration/SystemConfiguration.h>
#import <CommonCrypto/CommonHMAC.h>
#import <CommonCrypto/CommonCryptor.h>

#import <Foundation/Foundation.h>
#import <Security/Security.h>


#include "Environs.Native.h"
using namespace environs;

using namespace environs::API;

#define	CLASS_NAME 	"Environs.Crypt.iOSX. . ."


namespace environs
{

#if defined(ENVIRONS_IOS) || !defined(USE_OPENSSL)
    pInitEnvironsCrypt		InitEnvironsCryptPlatform			= 0;
    pInitEnvironsCrypt		InitEnvironsCryptLocksPlatform		= 0;
    pReleaseEnvironsCrypt	ReleaseEnvironsCryptPlatform		= 0;
    pReleaseEnvironsCrypt	ReleaseEnvironsCryptLocksPlatform	= 0;

#endif
    
    bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen )
    {
        CVerb ( "SHAHashCreate" );
        
        char * blob = (char *) malloc ( CC_SHA512_DIGEST_LENGTH );
        if ( blob ) {
            if ( !CC_SHA512 ( msg, *hashLen, (unsigned char *)blob ) ) {
                free ( blob );
                blob = 0;
            }
            else {
                *hash = blob;
                *hashLen = CC_SHA512_DIGEST_LENGTH;
                return true;
            }
        }
        return false;
    }
    
    
    size_t encodeLength(unsigned char * buf, size_t length)
    {
        if ( !buf )
            return 0;
        
        if (length < 128)
        {
            buf[0] = length;
            return 1;
        }
        
        size_t i = (length / 256) + 1;
        buf[0] = i + 0x80;
        for (size_t j = 0 ; j < i; ++j)
        {
            buf[i - j] = length & 0xFF;  
            length = length >> 8;
        }
        
        return i + 1;
    }
    
    static unsigned char oidSequence[] = { 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00 };

    NSString * GetKeyIdent ( bool isPubKey )
    {
        return [[NSString alloc ] initWithFormat:@"hcm.environs.App.%@.%@",[[NSBundle mainBundle] bundleIdentifier], isPubKey ? @"pubKey" : @"privKey"];
    }
    
    
    char * GetKeyBitsFromKey ( bool pub, SecKeyRef key )
    {
        const char * keyS = pub ? "public key" : "private key";
        
        if ( !key ) {
            CErrArg ( "GetKeyBitsFromKey: Called with NULL argument for %s.", keyS );
            return 0;
        }

        char * keyData = 0;

        do
        {
            NSString * tagString = GetKeyIdent ( pub );
            if ( !tagString ) {
                CErrArg ( "GetKeyBitsFromKey: Failed to get key ident for %s.", keyS ); break;
            }
            
            NSData * tag = [tagString dataUsingEncoding:NSUTF8StringEncoding];
            if ( !tag ) {
                CErrArg ( "GetKeyBitsFromKey: Failed to build key tag for [%s]", [tagString UTF8String] ); break;
            }
            
            NSMutableDictionary * query = [[NSMutableDictionary alloc] init];
            if ( !query ) {
                CErr ( "GetKeyBitsFromKey: Failed to allocate query dict." ); break;
            }
            
            [query setObject:(__bridge id)kSecClassKey          forKey:(__bridge id)kSecClass];
            [query setObject:tag                                forKey:(__bridge id)kSecAttrApplicationTag];
            [query setObject:(__bridge id)kSecAttrKeyTypeRSA    forKey:(__bridge id)kSecAttrKeyType];
            [query setObject:[NSNumber numberWithBool:YES]      forKey:(__bridge id)kSecReturnData];
            
            NSData      *   keyBits     = 0;
            CFDataRef       resultRef   = 0;
            OSStatus status = SecItemCopyMatching ( (__bridge CFDictionaryRef)query, (CFTypeRef* )&resultRef );
            
            keyBits = (__bridge_transfer NSData*)resultRef;
            
            if ( status != errSecSuccess ) {
                CErrArg ( "GetKeyBitsFromKey: SecItemCopyMatching failed for %s.", keyS ); break;
            }
            if ( [keyBits length] > 12000 ) {
                CErrArg ( "GetKeyBitsFromKey: Size of key bits [%u] exceeds limit 12000 bytes.", (unsigned int)[keyBits length] ); break;
            }
            
            unsigned char builder[15];
            NSMutableData * encKey = [[NSMutableData alloc] init];
            int bitstringEncLength;
            
            if  ([keyBits length ] + 1  < 128 )
                bitstringEncLength = 1 ;
            else
                bitstringEncLength = (int)( (([keyBits length ] +1 ) / 256 ) + 2 );
            
            builder[0] = 0x30;

            size_t i = sizeof(oidSequence) + 2 + bitstringEncLength + [keyBits length];
            
            size_t j = encodeLength(&builder[1], i);
            [encKey appendBytes:builder length:j +1];
            
            [encKey appendBytes:oidSequence length:sizeof(oidSequence)];
            
            builder[0] = 0x03;
            j = encodeLength(&builder[1], [keyBits length] + 1);
            
            builder[j+1] = 0x00;
            [encKey appendBytes:builder length:j + 2];
            
            [encKey appendData:keyBits];
            
            unsigned int keySize = (unsigned int) [encKey length];
            if ( keySize > 12000 ) {
                CErrArg ( "GetKeyBitsFromKey: Size of encoded DER key [%u] exceeds limit 12000 bytes.", keySize ); break;
            }
            
            keyData = (char *)malloc ( keySize + 4 );
            if ( !keyData ) {
                CErrArg ( "GetKeyBitsFromKey: Memory alloc [%u bytes] failed.", keySize + 4 ); break;
            }
            
            memcpy ( keyData + 4, [encKey bytes], keySize);
            *((unsigned int *) keyData ) = keySize;
        }
        while ( 0 );
        
        return keyData;
    }
    
    
    void CleanUpKey ( NSString * tagStr )
    {
        if ( !tagStr ) {
            CErr ( "CleanUpKey: Called with null argument." );
            return;
        }
        
        CVerb ( "CleanUpKey" );
        
        NSData * tag = [tagStr dataUsingEncoding:NSUTF8StringEncoding];
        if ( !tag )
            return;
        
        NSMutableDictionary * dict = [[NSMutableDictionary alloc] init];
        if ( !dict )
            return;
        
        [dict setObject:(__bridge id) kSecClassKey          forKey:(__bridge id) kSecClass];
        [dict setObject:(__bridge id) kSecAttrKeyTypeRSA    forKey:(__bridge id) kSecAttrKeyType];
        [dict setObject:tag                                 forKey:(__bridge id) kSecAttrApplicationTag];
        
        OSStatus status = SecItemDelete ( (__bridge CFDictionaryRef) dict );
        if ( status == noErr ) {
            CVerbArg ( "CleanUpKey: Key successfully destroyed. [%s]", [tagStr UTF8String] );
        }
    }
    
    
    SecKeyRef GetInstanceKey ( bool pubKey )
    {
        CVerb ( "GetInstanceKey" );
        
        SecKeyRef keyRef = 0;
        
        do
        {
            NSString * tagStr = GetKeyIdent ( pubKey );
            if ( !tagStr ) {
                CErr ( "GetInstanceKey: Failed to build key ident." ); break;
            }
            
            NSData * tag = [tagStr dataUsingEncoding:NSUTF8StringEncoding];
            if ( !tag ) {
                CErr ( "GetInstanceKey: Failed to build key ident data." ); break;
            }
            
            NSMutableDictionary * query = [[NSMutableDictionary alloc] init];
            if ( !tag ) {
                CErr ( "GetInstanceKey: Failed to alloc query." ); break;
            }
            
            [query setObject:(__bridge id)kSecClassKey          forKey:(__bridge id)kSecClass];
            [query setObject:tag                                forKey:(__bridge id)kSecAttrApplicationTag];
            [query setObject:(__bridge id)kSecAttrKeyTypeRSA    forKey:(__bridge id)kSecAttrKeyType];
            [query setObject:[NSNumber numberWithBool:YES]      forKey:(__bridge id)kSecReturnRef];
            
            OSStatus status = SecItemCopyMatching ( (__bridge CFDictionaryRef)query, (CFTypeRef *)&keyRef );
            if ( status != noErr || !keyRef ) {
                CErr ( "GetInstanceKey: SecItemCopyMatching failed." );
            }
        }
        while ( 0 );
        
        return keyRef;
    }
    
    
    SecKeyRef AttachPublicKey ( char * key, unsigned int keyLen )
    {
        if ( !key || !keyLen ) {
            CErr ( "AttachPublicKey: Called with null argument." );
            return 0;
        }
        
        CVerb ( "AttachPublicKey" );
        
        OSStatus status;
        
        SecKeyRef keyRef = 0;
        
        do
        {
            NSString * tagStr = GetKeyIdent ( false );
            if ( !tagStr )
                break;
            
            CleanUpKey ( tagStr );
            
            NSData * tag = [tagStr dataUsingEncoding:NSUTF8StringEncoding];
            if ( !tag )
                break;
            
            NSMutableDictionary * query = [[NSMutableDictionary alloc] init];
            if ( !query )
                break;
            
            [query setObject:(__bridge id) kSecClassKey         forKey:(__bridge id)kSecClass];
            [query setObject:(__bridge id) kSecAttrKeyTypeRSA   forKey:(__bridge id)kSecAttrKeyType];
            [query setObject:tag                                forKey:(__bridge id)kSecAttrApplicationTag];
            
            NSData * keyData = [NSData dataWithBytes:key length:keyLen];
            if ( !keyData )
                break;

            CFTypeRef persistKey = 0;
            
            [query setObject:keyData                                forKey:(__bridge id)kSecValueData];
            [query setObject:(__bridge id) kSecAttrKeyClassPublic  forKey:(__bridge id)kSecAttrKeyClass];
            [query setObject:[NSNumber numberWithBool:YES]          forKey:(__bridge id)kSecReturnPersistentRef];
            
            status = SecItemAdd ( (__bridge CFDictionaryRef)query, &persistKey );
            
            if ( persistKey )
                CFRelease ( persistKey );
            
            if ( status != noErr && status != errSecDuplicateItem )
                break;
            
            [query removeObjectForKey:(__bridge id)kSecValueData];
            [query removeObjectForKey:(__bridge id)kSecReturnPersistentRef];
            
            [query setObject:[NSNumber numberWithBool:YES]      forKey:(__bridge id)kSecReturnRef];
            [query setObject:(__bridge id) kSecAttrKeyTypeRSA   forKey:(__bridge id)kSecAttrKeyType];
            
            status = SecItemCopyMatching ( (__bridge CFDictionaryRef)query, (CFTypeRef *)&keyRef );
            if ( status != noErr || !keyRef ) {
                CErr ( "AttachPublicKey: Failed." );
            }
        }
        while ( 0 );
        
        return keyRef;
    }
    
    
	bool dGenerateCertificate ( char ** priv, char ** pub )
	{
        if ( !priv || !pub ) {
            CErr ( "GenerateCertificate: Called with null argument." );
            return false;
        }
        
        CVerb ( "GenerateCertificate" );
        
		bool        ret         = false;
		char    *   privKey     = 0;
		char    *   pubKey      = 0;
        OSStatus    status      = noErr;
        SecKeyRef   publicKey   = 0;
        SecKeyRef   privateKey  = 0;

		do
		{
            NSString * pubTagStr    = GetKeyIdent ( true );
            NSString * privTagStr   = GetKeyIdent ( false );
            if ( !pubTagStr || !privTagStr )
                return false;
            
            CleanUpKey ( pubTagStr );
            CleanUpKey ( privTagStr );
            
            NSData * pubTag     = [pubTagStr dataUsingEncoding:NSUTF8StringEncoding];
            NSData * privTag    = [privTagStr dataUsingEncoding:NSUTF8StringEncoding];
            if ( !pubTag || !privTag ) {
				CErr ( "GenerateCertificate: Failed to build key tags." ); break;
            }
            
            NSMutableDictionary * privDict  = [[NSMutableDictionary alloc] init];
            NSMutableDictionary * pubDict   = [[NSMutableDictionary alloc] init];
            NSMutableDictionary * dict      = [[NSMutableDictionary alloc] init];
            if ( !privDict || !pubDict || !dict ) {
				CErr ( "GenerateCertificate: Failed to build query dicts." ); break;
            }
            
            [privDict setObject:[NSNumber numberWithBool:YES]   forKey:(__bridge id)kSecAttrIsPermanent];
            [privDict setObject:privTag                         forKey:(__bridge id)kSecAttrApplicationTag];
            
            [pubDict setObject:[NSNumber numberWithBool:YES]    forKey:(__bridge id)kSecAttrIsPermanent];
            [pubDict setObject:pubTag                           forKey:(__bridge id)kSecAttrApplicationTag];
            
            [dict setObject:privDict                            forKey:(__bridge id)kSecPrivateKeyAttrs];
            [dict setObject:pubDict                             forKey:(__bridge id)kSecPublicKeyAttrs];
            [dict setObject:(__bridge id)kSecAttrKeyTypeRSA     forKey:(__bridge id)kSecAttrKeyType];
            
            [dict setObject:[NSNumber numberWithUnsignedInteger:ENVIRONS_DEVICES_KEYSIZE] forKey:(__bridge id)kSecAttrKeySizeInBits];
            
            status = SecKeyGeneratePair ( (__bridge CFDictionaryRef)dict, &publicKey, &privateKey );
            if ( status != noErr || !publicKey || !privateKey ) {
				CErr ( "GenerateCertificate: SecKeyGeneratePair failed." ); break;
            }
            
            pubKey  = GetKeyBitsFromKey ( true, publicKey );
            
            const char * privSID = "iOSKeyChain";

            unsigned int privLen = (unsigned int) strlen ( privSID );
            privKey = (char *) malloc ( privLen );
            
            if ( !privKey || !pubKey ) {
				CErr ( "GenerateCertificate: GetKeyBitsFromKey failed." ); break;
            }
            
            unsigned int pubLen = *((unsigned int *) pubKey);
            *((unsigned int *) pubKey) = (pubLen | ('d' << 16));
            
            memcpy ( privKey + 4, privSID, privLen );
            *((unsigned int *) privKey) = privLen;
            
            /// Test the certificate
            /*char msg [512];
            strlcpy ( msg, "test" );
            unsigned int msgLen = 4;
            char * dec = 0;

            if ( EncryptMessage ( 0, pubKey, msg, &msgLen ) ) {
                if ( DecryptMessage ( privKey + 4, privLen, msg, msgLen, &dec, &msgLen)) {
                    msgLen++;
                }
            }
            */

			*priv = privKey;
			privKey = 0;
			*pub = pubKey;
			pubKey = 0;
			ret = true;
		}
		while ( 0 );
        
		if ( privKey ) free ( privKey );
		if ( pubKey ) free ( pubKey );
        
		return ret;
	}
    
    
    bool UpdatePrivateKey ( char * key, unsigned int keyLen )
    {
        bool ret = false;
        OSStatus status;
        
        do
        {
            NSString * tagStr = GetKeyIdent ( false );
            if ( !tagStr )
                break;
            
            CleanUpKey ( tagStr );
            
            NSData * tag = [tagStr dataUsingEncoding:NSUTF8StringEncoding];
            if ( !tag )
                break;
            
            NSMutableDictionary * query = [[NSMutableDictionary alloc] init];
            if ( !query )
                break;
            
            [query setObject:(__bridge id) kSecClassKey         forKey:(__bridge id)kSecClass];
            [query setObject:(__bridge id) kSecAttrKeyTypeRSA   forKey:(__bridge id)kSecAttrKeyType];
            [query setObject:tag                                forKey:(__bridge id)kSecAttrApplicationTag];
            
            NSData * keyData = [NSData dataWithBytes:key length:keyLen];
            if ( !keyData )
                break;
            
            CFTypeRef persistKey = 0;
            
            [query setObject:keyData                                forKey:(__bridge id)kSecValueData];
//            [query setObject:(__bridge id) kSecAttrKeyClassPrivate  forKey:(__bridge id)kSecAttrKeyClass];
            [query setObject:[NSNumber numberWithBool:YES]          forKey:(__bridge id)kSecReturnPersistentRef];
            
            status = SecItemAdd ( (__bridge CFDictionaryRef)query, &persistKey );
            
            if ( persistKey )
                CFRelease ( persistKey );
            
            if ( status != noErr && status != errSecDuplicateItem )
                break;
            /*
            [query removeObjectForKey:(__bridge id)kSecValueData];
            [query removeObjectForKey:(__bridge id)kSecReturnPersistentRef];
            
            [query setObject:[NSNumber numberWithBool:YES]      forKey:(__bridge id)kSecReturnRef];
            [query setObject:(__bridge id) kSecAttrKeyTypeRSA   forKey:(__bridge id)kSecAttrKeyType];
            
            status = SecItemCopyMatching ( (__bridge CFDictionaryRef)query, (CFTypeRef *)&keyRef );
            if ( status != noErr || !keyRef ) {
                CErr ( "UpdatePrivateKey: Failed." ); break;
            }
            */
            ret = true;
        }
        while ( 0 );
        
        return ret;
    }
    

    bool dUpdateKeyAndCert ( char * priv, char * cert )
    {
        if ( !priv || ! cert ) {
            CErr ( "UpdateKeyAndCert: Called with null argument." );
            return false;
        }

        CVerb ( "UpdateKeyAndCert" );

        if ( !UpdatePrivateKey ( priv + 4, *((unsigned int *) priv)))
            return false;

        return true;
    }


    bool dPreparePrivateKey ( char ** privKey )
    {
        return true;
    }


    void dAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
    }

    
    SecKeyRef GetPublicKeyRef ( NSData * publicKey )
    {
		if ( !publicKey ) {
            CErr ( "GetPublicKeyRef: Called with null argument." );
			return 0;
        }
        
        CVerb ( "GetPublicKeyRef" );
        
        OSStatus status = noErr;
        SecKeyRef keyRef = NULL;
        CFTypeRef persistPeer = NULL;
        CFTypeRef CKeyRef = 0;
        
        char peerBuffer [128];
        sprintf ( peerBuffer, "Environs.%u.pubkey", rand() );
        CVerbArg ( "GetPublicKeyRef: peerName [%s]", peerBuffer );
        
        NSData * peerTag = [[NSData alloc] initWithBytes:(const void *)peerBuffer length:strlen(peerBuffer)];
        
        NSMutableDictionary * attr = [[NSMutableDictionary alloc] init];
        
        [attr setObject:(__bridge id)kSecClassKey       forKey:(__bridge id)kSecClass];
        [attr setObject:(__bridge id)kSecAttrKeyTypeRSA forKey:(__bridge id)kSecAttrKeyType];
        [attr setObject:peerTag                         forKey:(__bridge id)kSecAttrApplicationTag];
        [attr setObject:publicKey                       forKey:(__bridge id)kSecValueData];
        
        //status = SecItemDelete ( (__bridge CFDictionaryRef) attr );
        
        [attr setObject:(__bridge id)kSecAttrKeyClassPublic forKey:(__bridge id)kSecAttrKeyClass];
        //[attr setObject:[NSNumber numberWithBool:YES]   forKey:(__bridge id)kSecReturnPersistentRef];
        [attr setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecReturnRef];
        status = SecItemCopyMatching((__bridge CFDictionaryRef) attr, &CKeyRef);
        
        status = SecItemAdd ( (__bridge CFDictionaryRef) attr, (CFTypeRef *)&persistPeer );
        
        if ( status != noErr /*&& status != errSecDuplicateItem */ )
            CErrArg ( "Problem adding the peer public key to the keychain, OSStatus == %d.", (int)status );
        
        if (persistPeer) {
            NSMutableDictionary * queryKey = [[NSMutableDictionary alloc] init];
            
            [queryKey setObject:(__bridge id)persistPeer        forKey:(__bridge id)kSecValuePersistentRef];
            [queryKey setObject:[NSNumber numberWithBool:YES]   forKey:(__bridge id)kSecReturnRef];
            
            status = SecItemCopyMatching((__bridge CFDictionaryRef)queryKey, (CFTypeRef *)&keyRef);
            NSData *data = (__bridge NSData *)keyRef;
            if ( data ) {
                CLog ("...");
            }
        } else {
            [attr removeObjectForKey:(__bridge id)kSecValueData];
            [attr setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecReturnRef];

            status = SecItemCopyMatching((__bridge CFDictionaryRef) attr, (CFTypeRef *)&keyRef);
        }
        
        if (persistPeer) CFRelease(persistPeer);
        return keyRef;
    }
    
    
    bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
    {
		if ( !cert || !msg || !msgLen ) {
            CErr ( "EncryptMessage: Called with at least one null argument." );
			return false;
        }
        
        CVerbArg ( "EncryptMessage: Encrypting msg of size [%u]", *msgLen );
        
        bool                success = false;

        @autoreleasepool
        {
            SecCertificateRef   pubCert  = nil;

            unsigned int certProp       = *((unsigned int *) cert);
            unsigned int certSize       = certProp & 0xFFFF;

            char    *   ciphers         = 0;
            NSData  *   certificateData = 0;
            SecKeyRef   publicKey       = 0;
            SecPolicyRef policy         = 0;
            SecTrustRef trust           = 0;

            do
            {
                do
                {
                    certificateData = [[NSData alloc] initWithBytes:(cert + 4) length:certSize];
                    if ( !certificateData ) {
                        CErr ( "EncryptMessage: Failed to allocated certificate data." ); break;
                    }

                    policy = SecPolicyCreateBasicX509 ();
                    if ( !policy ) {
                        CErr ( "EncryptMessage: Failed to create basic X509 policy" ); break;
                    }

                    pubCert = SecCertificateCreateWithData ( NULL, (__bridge CFDataRef)certificateData );
                    if ( !pubCert ) {
                        CErr ( "EncryptMessage: Failed to create certificate with cert data." ); break;
                    }

                    if ( SecTrustCreateWithCertificates ( pubCert, policy, &trust ) ) {
                        CErr ( "EncryptMessage: SecTrustCreateWithCertificates failed." ); break;
                    }

                    publicKey = SecTrustCopyPublicKey ( trust );
                    if ( !publicKey ) {
                        CErr ( "EncryptMessage: Failed to trust public key." ); break;
                    }
                }
                while ( 0 );

                /*if ( !publicKey ) {
                 CWarn ( "EncryptMessage: Failed to create certificate with cert data. We're trying to import the key." );
                 publicKey = GetInstanceKey ( true );
                 }

                 if ( !publicKey ) {
                 publicKey = GetPublicKeyRef ( [NSData dataWithBytes:(cert + 4) length:certSize]);
                 }
                 */

                if ( !publicKey ) {
                    CErr ( "EncryptMessage: Failed to parse public key." ); break;
                }


                size_t ciphersSize = SecKeyGetBlockSize ( publicKey ) + *msgLen;

                ciphers = (char *) malloc ( ciphersSize );
                if ( !ciphers ) {
                    CErr ( "EncryptMessage: Memory alloc failed." ); break;
                }

                OSStatus status = EncryptMessageX ( publicKey, certProp, msg, *msgLen, ciphers, &ciphersSize );
                if ( status ) {
                    CErr ( "EncryptMessage: SecKeyEncrypt failed." ); break;
                }

                memcpy ( msg, ciphers, ciphersSize );

                *msgLen = (unsigned int) ciphersSize;

                success = true;
                
            } while ( 0 );
            
            if ( ciphers )
                free ( ciphers );
            if ( trust )
                CFRelease( trust );
            if ( policy )
                CFRelease( policy );
            if ( pubCert )
                CFRelease( pubCert );
            if ( publicKey )
                CFRelease( publicKey );
        }
        
        return success;
    }
    
    
    bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
    {
		if ( !key || !msg || !msgLen || !decrypted || !decryptedSize ) {
            CErr ( "DecryptMessage: Called with at least one null argument." );
			return false;
        }
        CVerbArg ( "DecryptMessage: Decrypting msg of size [%i]", msgLen );
        
		bool        ret         = false;
        char   *    decrypt     = 0;
        int         decSize     = -1;
        size_t      decSizet    = msgLen + 1;
        
        key += 4;
        
        SecKeyRef   pKey        = nil;
        
		if ( pthread_mutex_lock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to acquire lock." );
			return false;
		}

        do
        {
            pKey = GetInstanceKey ( false );
            if ( !pKey )
                break;
            
            decrypt = (char *) malloc ( decSizet );
            if ( !decrypt ) {
                CErrArg ( "DecryptMessage: Memory alloc [%zu] failed.", decSizet );
                break;
            }
            
#ifdef ENVIRONS_IOS
            OSStatus status = SecKeyDecrypt ( pKey, kSecPaddingOAEP, (uint8_t *) msg, msgLen, (uint8_t *) decrypt, &decSizet);
            if ( status == noErr ) {
                ret = true; break;
            }
            CWarnArg ( "DecryptMessage: kSecPaddingOAEP Decrypted message status [%i].", (int)status );

            status = SecKeyDecrypt ( pKey, kSecPaddingPKCS1, (uint8_t *) msg, msgLen, (uint8_t *) decrypt, &decSizet);
            if ( !status ) {
                ret = true; break;
            }
            CWarnArg ( "DecryptMessage: kSecPaddingPKCS1 Decrypted message status [%i].", (int)status );
            
            status = SecKeyDecrypt ( pKey, kSecPaddingPKCS1SHA1, (uint8_t *) msg, msgLen, (uint8_t *) decrypt, &decSizet);
            if ( !status ) {
                ret = true; break;
            }
            CWarnArg ( "DecryptMessage: kSecPaddingPKCS1SHA1 Decrypted message status [%i].", (int)status );

            status = SecKeyDecrypt ( pKey, kSecPaddingPKCS1SHA256, (uint8_t *) msg, msgLen, (uint8_t *) decrypt, &decSizet);
            if ( !status ) {
                ret = true; break;
            }
            CWarnArg ( "DecryptMessage: kSecPaddingPKCS1SHA256 Decrypted message status [%i].", (int)status );
            
            status = SecKeyDecrypt ( pKey, kSecPaddingNone, (uint8_t *) msg, msgLen, (uint8_t *) decrypt, &decSizet);
            if ( !status ) {
                ret = true; break;
            }
            CWarnArg ( "DecryptMessage: kSecPaddingNone Decrypted message status [%i].", (int)status );
#endif
        }
        while (0);
        
        if ( ret ) {
            decSize = (int) decSizet;
            
            decrypt [ decSizet ] = 0;
            CVerbArg ( "DecryptMessage: [%s]", decrypt );
            
            *decrypted = decrypt;
            decrypt = 0;
            *decryptedSize = (unsigned int)decSize;
        }
        else {
            CErrArg ( "DecryptMessage: Failed. MsgLen [%u]", msgLen );
        }
        
		if ( decrypt )
			free ( decrypt );
        
        if ( pKey )
            CFRelease( pKey );
        
		if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to release lock." );
			ret = false;
		}

        return ret;
    }
    
    
    void dReleaseCert ( int deviceID )
    {
        
    }
    
    
    /// Not used!
	// AES encryption
	bool AESEncryptMessage ( unsigned char * keyData, unsigned char * salt, char * msg, unsigned int * msgLen, unsigned int msgBufferSize )
	{
		if ( !keyData || !msg || !msgLen )
            return false;
        
        NSUInteger dataLength = *msgLen;
        
        size_t bufferSize = dataLength + kCCBlockSizeAES128;
        
        void * buffer = malloc(bufferSize);
        if ( !buffer )
            return false;
        
        size_t numBytesEncrypted = 0;
        CCCryptorStatus cryptStatus = CCCrypt ( kCCEncrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding,
                                              keyData, kCCKeySizeAES256,
                                              NULL /* IV*/,
                                              msg, dataLength, /* in */
                                              buffer, bufferSize, /* out */
                                              &numBytesEncrypted );
        if (cryptStatus == kCCSuccess) {
            free ( buffer );
            return true;
        }
        
        free ( buffer );
        return false;
    }
    
    
	bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
    {
        CVerb ( "AESDeriveKeyContext" );
        
		if ( !key || keyLen < (AES_SHA256_KEY_LENGTH * 2) || !ctx ) {
			CErrArg ( "AESDeriveKeyContext: Called with at least one NULL argument or keyLen [%u] < [%u].", keyLen, AES_SHA256_KEY_LENGTH * 2 ); return false;
		}
        
		char		*	blob		= 0;
        
		bool ret = false;
        
        do
        {
            blob = (char *) malloc ( AES_SHA256_KEY_LENGTH );
            if ( !blob ) {
                CErr ( "AESDeriveKeyContext: Failed to allocate memory for hashed blob." ); break;
            }
            
            if ( !CC_SHA256 ( key, AES_SHA256_KEY_LENGTH, (unsigned char *)blob ) ) {
                CErr ( "AESDeriveKeyContext: CC_SHA256 failed." ); break;
            }

            CVerbArg ( "AESDeriveKeyContext: AES key [%s]", ConvertToHexSpaceString ( blob, AES_SHA256_KEY_LENGTH ) );
            
            ctx->encCtx = blob;
            ctx->decCtx = blob;
            blob = 0;
            ctx->size = AES_SHA256_KEY_LENGTH;
            ret = true;
        }
        while ( 0 );
        
		if ( blob ) free ( blob );
        
		return ret;
    }
    
	
	void dAESDisposeKeyContext ( AESContext * ctx )
	{
        CVerb ( "AESDisposeKeyContext" );

        if ( !ctx )
            return;
        
        if ( ctx->encCtx )
            free ( ctx->encCtx );
        
        memset ( ctx, 0, sizeof(AESContext) );
	}
    
    
    bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher )
    {
        CVerb ( "AESEncrypt" );

        if ( !ctx || !buffer || !bufferLen || !cipher )
            return false;
        
        NSUInteger      dataLength  = *bufferLen;
        unsigned int    deviceID    = ctx->deviceID;
        
        size_t cipherSize = dataLength + kCCBlockSizeAES128;
        
        char * ciphers = (char *) malloc ( cipherSize + 21 );
        if ( !ciphers ) {
            CErrArgID ( "AESEncrypt: Memory allocation [%zu bytes] failed.", cipherSize );
            return false;
        }
        
        BUILD_IV_128 ( ciphers + 4 );

        CVerbVerbArgID ( "AESEncrypt: IV [%s]", ConvertToHexSpaceString ( ciphers + 4, 16 ) );

        size_t numBytesEncrypted = 0;
        CCCryptorStatus cryptStatus = CCCrypt ( kCCEncrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding,
                                               ctx->encCtx,
                                               kCCKeySizeAES256,
                                               ciphers + 4, /* IV */
                                               buffer,
                                               dataLength, /* in */
                                               ciphers + 20,
                                               cipherSize, /* out */
                                               &numBytesEncrypted );
        if ( cryptStatus == kCCSuccess )
        {
            numBytesEncrypted += 20;
            ciphers [ numBytesEncrypted ] = 0;
            
            /// Update enc header
            *((unsigned int *) ciphers) = (unsigned int)(0x40000000 | numBytesEncrypted);
            
            *cipher = ciphers;
            *bufferLen = (unsigned int) numBytesEncrypted;
            
            return true;
        }
        else {
            CErrID ( "AESEncrypt: Failed." );
        }
        
        if ( ciphers )
            free ( ciphers );
        return false;
    }

    
    bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted )
    {
        CVerb ( "AESDecrypt" );
        
        if ( !ctx || !buffer || !bufferLen || *bufferLen < 36 || !decrypted )
            return false;
        
        NSUInteger      dataLength  = *bufferLen;
        unsigned int    deviceID    = ctx->deviceID;
        
        size_t plainSize = dataLength + kCCBlockSizeAES128;
        char * plainText = (char *) malloc ( plainSize + 1 );

        CVerbVerbArgID ( "AESDecrypt: IV [%s]", ConvertToHexSpaceString ( buffer + 4, 16 ) );

        size_t numBytesDecrypted = 0;
        CCCryptorStatus cryptStatus = CCCrypt ( kCCDecrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding,
                                               ctx->decCtx,
                                               kCCKeySizeAES256,
                                               buffer + 4, /* IV */
                                               buffer + 20,
                                               dataLength - 20, /* input */
                                               plainText,
                                               plainSize, /* output */
                                               &numBytesDecrypted );
        if ( cryptStatus == kCCSuccess ) {
            
            plainText [ numBytesDecrypted ] = 0;
            *decrypted = plainText;
            *bufferLen = (unsigned int) numBytesDecrypted;
            
            return true;
        }
        else {
            CErrID ( "AESDecrypt: Failed." );
        }
        
        if ( plainText )
            free ( plainText );
        return false;
    }
}




