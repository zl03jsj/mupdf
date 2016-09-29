//
//  CFISTBTKeyDelegate.h
//  BLEKeyDemo
//
//  Created by LiChuang on 8/26/13.
//  Copyright (c) 2013 cfist. All rights reserved.
//

#import <Foundation/Foundation.h>

//error type
#define CFIST_OK                                0
#define CFIST_ERROR_INVALID_PARAMETER           0x20338001
#define CFIST_ERROR_INVALID_PIN                 0x20338002
#define CFIST_ERROR_PIN_LOCKED                  0x20338003

//TIMEOUT ERROR
#define CFIST_ERROR_KEY_OKBUTTON_SIGN_TIMEOUT   0x20339001
#define CFIST_ERROR_KEY_OKBUTTON_PIN_TIMEOUT    0x20339002
#define CFIST_ERROR_KEY_RESPONSE_TIMEOUT        0x20339003
#define CFIST_ERROR_SCAN_TIMEOUT                0x20339004

//BLE ERROR
#define CFIST_ERROR_BLE_CONNECTION              0x2033A001
#define CFIST_ERROR_BLE_COMMUNICATION           0x2033A002

//APDU ERROR
#define CFIST_ERROR_INTERNAL_RUNTIME            0x2033B001
#define CFIST_ERROR_KEY_UNEXPECTED_RETURN       0x2033B002
#define CFIST_ERROR_KEY_BUSY                    0x2033B003
#define CFIST_ERROR_ULAN_ALREADY_CONNECTED      0x2033B004
#define CFIST_ERROR_SIGN_FAILED                 0x2033B005
#define CFIST_ERROR_INVALID_HASH                0x2033B006
#define CFIST_ERROR_USER_CANCEL                 0x2033B007

//DEVICE ERROR
#define CFIST_ERROR_ULAN_NO_VALID_CERT          0x2033C001
#define CFIST_ERROR_ULAN_NO_SPECIFIED_CERT      0x2033C002
#define CFIST_ERROR_NO_SEAL_IMAGE               0x2033C003

#define CFIST_ERROR_UNKNOW                      0x2033FFFF

@class ULANKeyError;
@class ULANKeyCertificate;

@protocol ULANKeyDelegate

/**
 *  called when CFIST BlueTooth Key connected,
 *
 *  @param err : nil is ok
 *  @param keyID : which ulan key is connected, when error keyID is nil
 */
- (void)didConnected:(ULANKeyError *)err keyID:(NSString *)keyID;

/**
 *  called when Key disConnected
 *
 *  @param err :nil is ok
 */
- (void)didDisConnected:(ULANKeyError *)err;

/**
 *  called when Key when signing is end
 *
 *  @param err : nil is ok
 *  @param signature :the signature by the key
 */
- (void)didSigned:(ULANKeyError *)err result:(NSString *)signature;

/**
 *  called when Key when fethch cert is end
 *
 *  @param err : nil is ok
 *  @param cert :the detail info or cert
 */
- (void)didCertFetched:(ULANKeyError *)err cert:(ULANKeyCertificate *)cert;

@end
