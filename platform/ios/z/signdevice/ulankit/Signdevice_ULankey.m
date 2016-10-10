//
//  Signdevice_ULankey.m
//  MxPDF
//
//  Created by 曾亮 on 9/5/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//
#import "Signdevice_ULankey.h"
#import "Include/ULANKey.h"
#import "Include/ULANKeyDelegate.h"

@interface Signdevice_ULankey ()<ULANKeyDelegate> {
@private
	dispatch_semaphore_t _sema; //  = dispatch_semaphore_create(0);
}
@property (strong, nonatomic) ULANKey * ulankey;
@property (retain, nonatomic) NSData * data;
@end


@implementation Signdevice_ULankey
static Signdevice_ULankey * instance = nil;
- (id)init {
	printf("please call:[Signdevice_ULanbkey getInstance].");
	return nil;
}

- (id) initPrivate {
	self = [super init];
	self.ulankey = [ULANKey sharedULANKit:self];
	self.data = nil;
	self->_sema = dispatch_semaphore_create(0);
	return self;
}

+ (id) getInstance {
	@synchronized(self) {
		if (instance == nil) {
			instance = [[self alloc] initPrivate];
		}
		else {
			[instance retain];
		}
	}
	return instance;
}

- (NSData*) sign:(NSData*)dataTosign {
	//    一开始执行到这里信号量为0，线程被阻塞，直到上述操作完成使信号量+1,线程解除阻塞
	
	/**
	 *  signData
	 *  @param dataToSign data to sign
	 *  @param pin        pinCode
	 *  @param certType   sm2,rsa1024,rsa2048,nil is auto select
	 *  @param hashAlgorithm   SM3,SHA1,SHA256
	 *  @param signType   PKCS1, PKCS7_DETACHED, PKCS7_ATTACHED
	 */
	[self.ulankey signData:dataTosign pin:@"123455"
				  signType:SIGN_TYPE_PKCS7_DETACHED
			 hashAlgorithm:HASH_ALGORITHM_SHA1
				  certType:CERT_TYPE_RSA1024_CFIST];
	dispatch_semaphore_wait(self->_sema, DISPATCH_TIME_FOREVER);
	return self.data;
}

- (void) dealloc {
	@synchronized (self) {
		if(self.ulankey!=nil) {
			[self.ulankey release];
			self.ulankey = nil;
		}
		instance = nil;
	}
	[super dealloc];
	printf("ULankeyDevice dealloc!\n");
}

#pragma mark - Ulan delegate
/**
 *  called when CFIST BlueTooth Key connected,
 *
 *  @param err : nil is ok
 *  @param keyID : which ulan key is connected, when error keyID is nil
 */
- (void)didConnected:(ULANKeyError *)err keyID:(NSString *)keyID {
	if (err != nil) {
		printf("Connect filed!");
		return;
	}
	printf("key connected! Keyid=%s\n", [keyID UTF8String]);
}

/**
 *  called when Key disConnected
 *
 *  @param err :nil is ok
 */
- (void)didDisConnected:(ULANKeyError *)err {
	if (err != nil) {
		printf("Connect filed!");
		return;
	}
	printf("key disconnected!\n");
}

/**
 *  called when Key when signing is end
 *
 *  @param err : nil is ok
 *  @param signature :the signature by the key
 */
- (void)didSigned:(ULANKeyError *)err result:(NSString *)signature {
	// 完成签名, 触发信号量!
	if (err != nil) {
		printf("Connect filed!");
		return;
	}
	printf("signed!\n");
	self.data = [signature dataUsingEncoding:NSUTF8StringEncoding];
	dispatch_semaphore_signal(self->_sema);
}

/**
 *  called when Key when fethch cert is end
 *
 *  @param err : nil is ok
 *  @param cert :the detail info or cert
 */
- (void)didCertFetched:(ULANKeyError *)err cert:(ULANKeyCertificate *)cert {
	if (err != nil) {
		printf("Connect filed!");
		return;
	}
	printf("didCertFetched!\n");
}

@end