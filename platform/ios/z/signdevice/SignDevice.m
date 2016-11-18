//
//  SignDevice.m
//  MxPDF
//
//  Created by 曾亮 on 9/2/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import "mupdf/pdf.h"
#import "SignkeyFindView.h"

#if defined(SIGN_SUPPORTED)
#include "./mtoken/Include/K5SOFApp.h"
#include "./mtoken/Include/K5AlgoDefs.h"
#endif

@interface SignkeyView ()
@end

@implementation SignkeyView {
	NSData *_signdata;
	NSData *_digest;
	key_sign_error _errcode;
}

@synthesize signdata = _signdata;
@synthesize digest = _digest;
@synthesize errcode = _errcode;

-(id)init {
	NSLog(@"[SignKeyView init] was call");
	if(self = [super init])	{
		_errcode = sign_code_unkown;
		_digest = nil;
		_signdata = nil;
	}
	return self;
}

-(void)dealloc {
	if(_signdata) [_signdata release];
	if(_digest) [_digest release];
	[super dealloc];
}

-(NSData*) dosign:(NSData *)digest{
	NSLog(@"not implement!");
	return nil;
}

@end

typedef struct z_key_device_s {
	z_device supper;
	UIView *view;
} z_key_device;


static void z_key_dosign(fz_context *ctx, z_device *device, pdf_document *doc,
						 pdf_annot *annot, z_pdf_sign_appearance *app)
{
	fz_try(ctx)
	{
		fz_rect rect = annot->rect;
		pdf_obj *wobj = annot->obj;
		pdf_signature_set_value(ctx, doc, wobj, device);
		/* Create an appearance stream only if the signature is intended to be visible */
		if ( !fz_is_empty_rect(&rect) && app)
		{
			app->set_app(ctx, doc, annot, app);
		}
	}
	fz_always(ctx) {
	}
	fz_catch(ctx) {
		fz_rethrow(ctx);
	}
}

static NSData* z_pdf_signdata_from_byterange(fz_context *ctx, pdf_document *doc, char *filename, pdf_obj *byterange)
{
	fz_buffer *digest_buffer = NULL;
	NSData *signdata = NULL;
	fz_try(ctx){
		digest_buffer = z_openssl_pdf_sha1(ctx, doc, byterange, filename);
		signdata = [[NSData alloc]initWithBytes:digest_buffer->data length:digest_buffer->len];
	}
	fz_always(ctx) {
		if(digest_buffer) fz_drop_buffer(ctx, digest_buffer);
	}
	fz_catch(ctx){
		fz_rethrow(ctx);
	}
	return signdata;
}

fz_buffer *z_key_pdf_get_digest(fz_context *ctx, pdf_document *doc,	z_device *device, char *filename, pdf_obj *byte_range)
{
	z_key_device *dev = (z_key_device*)device;
	UIView *view = dev->view;
	
	NSData *signdata = nil;
	NSData *digest = nil;
	fz_buffer *buffer = NULL;
	unsigned char *buffdata = NULL;
	SignkeyFindView *findView = NULL;
	SignkeyView *signkeyView = NULL;
	
	fz_try(ctx) {
		findView = [[[NSBundle mainBundle]loadNibNamed:@"KeysignFindView" owner:nil options:nil]lastObject];
		if(!findView)
			fz_throw(ctx, FZ_ERROR_GENERIC, "load sign device discovery view faild!");

		CGRect rect = CGRectInset(view.bounds, 20, 40);

		// show UIView to sign!!!
		findView.frame = rect;
		[view addSubview:findView];
		
		signkeyView = [findView signkeyView];
		if(!signkeyView) {
			fz_throw(ctx, FZ_ERROR_GENERIC, "signkeyView property of SinkeyFindView is nil, may be error code:%d",
					 (int)findView.errcode);
		}
		
		digest = z_pdf_signdata_from_byterange(ctx, doc, filename, byte_range);
		if(!digest)
			fz_throw(ctx, FZ_ERROR_GENERIC, "could not get sha1 gigest");

		signkeyView.frame = rect;
		signkeyView.digest = digest;
		[view addSubview:signkeyView];
		
		signdata = signkeyView.signdata;
		if(nil==signdata)
			fz_throw(ctx, FZ_ERROR_GENERIC, "sign error:code %d.", (int)signkeyView.errcode);

		buffdata = fz_malloc(ctx, (int)[signdata length]);
		[signdata getBytes:buffdata length:[signdata length]];
		buffer = fz_new_buffer_from_data(ctx, buffdata, (int)[signdata length]);
	}
	fz_always(ctx){
		if(signdata) [signdata release];
	}
	fz_catch(ctx){
		if(buffdata) fz_free(ctx, buffdata);
	}
	return buffer;
}

void z_key_device_release(fz_context *ctx, z_device *device)
{
	z_key_device *dev = (z_key_device*)device;
	[dev->view release];
	fz_free(ctx, dev);
}

z_device *z_key_new_device(fz_context *ctx, UIView *view) {
	z_key_device *device = NULL;
	z_device *supper = NULL;
	
	fz_try(ctx){
		device = fz_malloc_struct(ctx, z_key_device);
		memset(device, 0, sizeof(z_key_device));
		
		supper = &device->supper;
		supper->refcount = 1;
		supper->do_sign = z_key_dosign;
		supper->get_digest = z_key_pdf_get_digest;
		supper->release = z_key_device_release;
		device->view = [view retain];
	}
	fz_catch(ctx){
		z_key_device_release(ctx, (z_device*)device);
	}
	return (z_device*)device;
}



