//
//  SignDevice.m
//  MxPDF
//
//  Created by 曾亮 on 9/2/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

// #import <Foundation/Foundation.h>
#import "mupdf/pdf.h"

typedef struct Z_bluetooth_signer_s{
	
}Z_bluttooth_signer;

//struct Z_sign_device_context_s
//{
//    int ref;
//    void *signer;
//    Z_sign_fn *dosign;
//    fz_buffer *signdata;
//    Z_SignDev_drop_fn *signdevdrop;
//    Z_SignDev_keep_fn *signdevkeep;
//};


Z_sign_device *Z_bluetooth_SignDev_new(fz_context *ctx)
{
//    Z_sign_device *signDev = NULL;
//    Z_openssl_signer *signer = NULL;
//    fz_try(ctx)
//    {
//        signer = Z_openssl_signer_new(ctx, file, pw);
//
//        signDev = fz_malloc_struct(ctx, Z_sign_device);
//        memset(signDev, 0, sizeof(Z_sign_device));
//
//        signDev->ref = 1;
//        signDev->signer = signer;
//        signDev->dosign = Z_sign_inner;
//        signDev->signdevdrop = Z_signdev_drop_inner;
//        signDev->signdevkeep = Z_signdev_keep_inner;
//    }
//    fz_catch(ctx) {
//		Z_openssl_drop_signer(ctx, signer);
//        fz_rethrow(ctx);
//    }
//    return signDev;
	return NULL;
}

