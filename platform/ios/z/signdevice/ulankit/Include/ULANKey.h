//
//  ULAN.h
//  ULANToolKit
//
//  Created by LiChuang on 9/23/13.
//  Copyright (c) 2013 cfist. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ULANKeyDelegate.h"

#define CERT_TYPE_SM2_CFIST         @"SM2"
#define CERT_TYPE_RSA1024_CFIST     @"RSA1024"
#define CERT_TYPE_RSA2048_CFIST     @"RSA2048"

#define HASH_ALGORITHM_SHA1         @"SHA1"
#define HASH_ALGORITHM_SHA256       @"SHA256"
#define HASH_ALGORITHM_MD5          @"MD5"
#define HASH_ALGORITHM_SM3          @"SM3"

#define SIGN_TYPE_PKCS1             @"PKCS1"
#define SIGN_TYPE_PKCS7_DETACHED    @"PKCS7_DETACHED"
#define SIGN_TYPE_PKCS7_ATTACHED    @"PKCS7_ATTACHED"

@interface ULANKeyError : NSObject

//if errorType is CFIST_ERROR_INVALID_PIN,the key will be locked after try another pinCanRetries times;
@property (nonatomic, assign) int pinCanRetries;
//if  errorType is CFIST_ERROR_KEY_UNEXPECTED_RETURN,underlyErrorData is underly errorcode
@property (nonatomic, assign) int currentStatus;

@property (nonatomic, assign) int errorCode;
@property (nonatomic, strong) NSString* errorMessage;
@property (nonatomic, strong) NSData* underlyErrorData;

-(NSString* )toString;

@end

@interface ULANKeyCertificate : NSObject

@property (copy, nonatomic) NSData      *certEncode;
@property (copy, nonatomic) NSString    *serialNumber;
@property (copy, nonatomic) NSString    *issuerDN;
@property (copy, nonatomic) NSDate      *notBefore;
@property (copy, nonatomic) NSDate      *notAfter;
@property (copy, nonatomic) NSString    *subjectDN;

@end

@interface ULANKey : NSObject

/**
*  sharedBluetoothKey
*
*  @description get the instance of CFISTBluetoothKey,which implements singleton mode
*
*  @param  delegator the delegator that implements callback
*  @return instance of CFISTBluetoothKey
*/
+ (ULANKey *)sharedULANKit:(id<ULANKeyDelegate>)delegator;

+ (NSString *)getVersion;

/**
 *  isConnected
 *
 *  @return is connected with the CFIST bluetooth key
 */
- (BOOL)isConnected;

/**
 *  connectKey
 *
 *  scan and try to connect the CFIST bluetooth key
 */
- (void)connectKey:(NSString *)keyID;

/**
 *  disconnection initiative, such when the user cancel the operation
 */
- (void)disConnect;

/**
 *   signData
 *
 *  @param dataToSign data to sign
 *  @param pin        pinCode
 *  @param certType   sm2,rsa1024,rsa2048,nil is auto select
 *  @param hashAlgorithm   SM3,SHA1,SHA256
 *  @param signType   PKCS1, PKCS7_DETACHED, PKCS7_ATTACHED
 */
- (void)signData:(NSData *)dataToSign
             pin:(NSString *)pin
        signType:(NSString *)signType
   hashAlgorithm:(NSString *)hashAlgorithm
        certType:(NSString *)certType;

/**
 *   fetchCert
 *
 *  @param certType  sm2,rsa1024,rsa2048, nil is auto select cert
 
 */
- (void)fetchCert:(NSString *)certType;

@end