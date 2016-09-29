#include "mupdf/pdf.h"

// #define HAVE_OPENSSL 
#ifdef HAVE_OPENSSL
#pragma message("HAVE_OPENSSL is defined!!!")

#ifdef Z_pdf_sign_ 
#pragma message("Z_pdf_sign_ is defined!!!")
#else
#pragma message("Z_pdf_sign_ is not defined!!!")
#endif

#include "openssl/err.h"
#include "openssl/bio.h"
#include "openssl/asn1.h"
#include "openssl/x509.h"
#include "openssl/err.h"
#include "openssl/objects.h"
#include "openssl/pem.h"
#include "openssl/pkcs7.h"
#include "openssl/pkcs12.h"

extern const char* ntkoextobjname;
enum
{
	SEG_START = 0,
	SEG_SIZE = 1
};

typedef struct bsegs_struct
{
	int (*seg)[2];
	int nsegs;
	int current_seg;
	int seg_pos;
} BIO_SEGS_CTX;

int OpensslInited = 0;

static int bsegs_read(BIO *b, char *buf, int size)
{
	BIO_SEGS_CTX *ctx = (BIO_SEGS_CTX *)b->ptr;
	int read = 0;
	while (size > 0 && ctx->current_seg < ctx->nsegs)
	{
		int nb = ctx->seg[ctx->current_seg][SEG_SIZE] - ctx->seg_pos;

		if (nb > size)
			nb = size;

		if (nb > 0)
		{
			if (ctx->seg_pos == 0)
				(void)BIO_seek(b->next_bio, ctx->seg[ctx->current_seg][SEG_START]);

			(void)BIO_read(b->next_bio, buf, nb);
			ctx->seg_pos += nb;
			read += nb;
			buf += nb;
			size -= nb;
		}
		else
		{
			ctx->current_seg++;

			if (ctx->current_seg < ctx->nsegs)
				ctx->seg_pos = 0;
		}
	}

	return read;
}

static int bsegs_gets(BIO *b, char *buf, int size)
{
	BIO_SEGS_CTX *ctx = (BIO_SEGS_CTX *)b->ptr;
	int read = 0;
	while (size > 0 && ctx->current_seg < ctx->nsegs)
	{
		int nb = ctx->seg[ctx->current_seg][SEG_SIZE] - ctx->seg_pos;

		if (nb > size)
			nb = size;

		if (nb > 0)
		{
			if (ctx->seg_pos == 0)
				(void)BIO_seek(b->next_bio, ctx->seg[ctx->current_seg][SEG_START]);

			(void)BIO_read(b->next_bio, buf, nb);
			ctx->seg_pos += nb;
			read += nb;
			buf += nb;
			size -= nb;
		}
		else
		{
			ctx->current_seg++;

			if (ctx->current_seg < ctx->nsegs)
				ctx->seg_pos = 0;
		}
	}

	return read;
}

static long bsegs_ctrl(BIO *b, int cmd, long arg1, void *arg2)
{
	BIO_SEGS_CTX *ctx = (BIO_SEGS_CTX *)b->ptr;
    long ofs;

    switch (cmd) {
        case BIO_CTRL_RESET:
        case BIO_C_FILE_SEEK: {
            int i;
            i = 0;
            ofs = cmd==BIO_CTRL_RESET ? 0:arg1;
            while( i<ctx->nsegs ) {
                if( ofs < ctx->seg[i][1] ) {
                    break;
                }
                ofs -= ctx->seg[i][1];
                i ++;
            }
            ctx->current_seg = i;
            ctx->seg_pos = fz_min(ofs, ctx->seg[i][1]-1);
            arg1 = ctx->seg[i][0] + ctx->seg_pos;
            }
            break;
    }
	return BIO_ctrl(b->next_bio, cmd, arg1, arg2);
}

static int bsegs_new(BIO *b)
{
	BIO_SEGS_CTX *ctx;

	ctx = (BIO_SEGS_CTX *)malloc(sizeof(BIO_SEGS_CTX));
	if (ctx == NULL)
		return 0;

	ctx->current_seg = 0;
	ctx->seg_pos = 0;
	ctx->seg = NULL;
	ctx->nsegs = 0;

	b->init = 1;
	b->ptr = (char *)ctx;
	b->flags = 0;
	b->num = 0;

	return 1;
}

static int bsegs_free(BIO *b)
{
	if (b == NULL)
		return 0;
    BIO_SEGS_CTX *ctx = b->ptr;
    if(ctx) {
        free(ctx->seg);
        free(ctx);
    }
	b->ptr = NULL;
	b->init = 0;
	b->flags = 0;

	return 1;
}

static long bsegs_callback_ctrl(BIO *b, int cmd, bio_info_cb *fp)
{
    return BIO_callback_ctrl(b->next_bio, cmd, fp);
}

static BIO_METHOD methods_bsegs =
{
	0, "segment reader",
	NULL,
	bsegs_read,
	NULL,
	bsegs_gets,
	bsegs_ctrl,
	bsegs_new,
	bsegs_free,
	bsegs_callback_ctrl,
};

static BIO_METHOD *BIO_f_segments(void)
{
	return &methods_bsegs;
}

static void BIO_set_segments(BIO *b, int (*seg)[2], int nsegs)
{
	BIO_SEGS_CTX *ctx = (BIO_SEGS_CTX *)b->ptr;
	ctx->seg = seg;
	ctx->nsegs = nsegs;
}

typedef struct verify_context_s
{
	X509_STORE_CTX x509_ctx;
	char certdesc[256];
	int err;
} verify_context;

static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	verify_context *vctx;
	X509 *err_cert;
	int err, depth;

	vctx = (verify_context *)ctx;

	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	X509_NAME_oneline(X509_get_subject_name(err_cert), vctx->certdesc, sizeof(vctx->certdesc));

	if (!ok && depth >= 6)
	{
		X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
	}

	switch (ctx->error)
	{
	case X509_V_ERR_INVALID_PURPOSE:
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
		err = X509_V_OK;
		X509_STORE_CTX_set_error(ctx, X509_V_OK);
		ok = 1;
		break;

	case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
		/*
			In this case, don't reset err to X509_V_OK, so that it can be reported,
			although we do return 1, so that the digest will still be checked
		*/
		ok = 1;
		break;

	default:
		break;
	}

	if (ok && vctx->err == X509_V_OK)
		vctx->err = err;
	return ok;
}

// static int pk7_verify(X509_STORE *cert_store, PKCS7 *p7, BIO *detached, char *ebuf, int ebufsize)
// {
// 	PKCS7_SIGNER_INFO *si;
// 	verify_context vctx;
// 	BIO *p7bio=NULL;
// 	char readbuf[1024*4];
// 	int res = 1;
// 	int i;
// 	STACK_OF(PKCS7_SIGNER_INFO) *sk;
// 
// 	vctx.err = X509_V_OK;
// 	ebuf[0] = 0;
// 
// 	OpenSSL_add_all_algorithms();
// 
// 	EVP_add_digest(EVP_md5());
// 	EVP_add_digest(EVP_sha1());
// 
// 	ERR_load_crypto_strings();
// 
// 	ERR_clear_error();
// 
// 	X509_VERIFY_PARAM_set_flags(cert_store->param, X509_V_FLAG_CB_ISSUER_CHECK);
// 	X509_STORE_set_verify_cb_func(cert_store, verify_callback);
// 
// 	p7bio = PKCS7_dataInit(p7, detached);
// 
// 	/* We now have to 'read' from p7bio to calculate digests etc. */
// 	while (BIO_read(p7bio, readbuf, sizeof(readbuf)) > 0)
// 		;
// 
// 	/* We can now verify signatures */
// 	sk = PKCS7_get_signer_info(p7);
// 	if (sk == NULL)
// 	{
// 		/* there are no signatures on this data */
// 		res = 0;
// 		fz_strlcpy(ebuf, "No signatures", ebufsize);
// 		goto exit;
// 	}
// 
// 	for (i=0; i<sk_PKCS7_SIGNER_INFO_num(sk); i++)
// 	{
// 		int rc;
// 		si = sk_PKCS7_SIGNER_INFO_value(sk, i);
// 		rc = PKCS7_dataVerify(cert_store, &vctx.x509_ctx, p7bio,p7, si);
// 		if (rc <= 0 || vctx.err != X509_V_OK)
// 		{
// 			char tbuf[120];
// 
// 			if (rc <= 0)
// 			{
// 				fz_strlcpy(ebuf, ERR_error_string(ERR_get_error(), tbuf), ebufsize);
// 			}
// 			else
// 			{
// 				/* Error while checking the certificate chain */
// 				snprintf(ebuf, ebufsize, "%s(%d): %s", X509_verify_cert_error_string(vctx.err), vctx.err, vctx.certdesc);
// 			}
// 
// 			res = 0;
// 			goto exit;
// 		}
// 	}
// 
// exit:
// 	X509_STORE_CTX_cleanup(&vctx.x509_ctx);
// 	ERR_free_strings();
// 
// 	return res;
// }

// static unsigned char adobe_ca[] =
// {
// #include "../../generated/gen_adobe_ca.h"
// };
// 
// static int verify_sig(char *sig, int sig_len, char *file, int (*byte_range)[2], int byte_range_len, char *ebuf, int ebufsize)
// {
// 	PKCS7 *pk7sig = NULL;
// 	PKCS7 *pk7cert = NULL;
// 	X509_STORE *st = NULL;
// 	BIO *bsig = NULL;
// 	BIO *bcert = NULL;
// 	BIO *bdata = NULL;
// 	BIO *bsegs = NULL;
// 	STACK_OF(X509) *certs = NULL;
// 	int t;
// 	int res = 0;
// 
// 	bsig = BIO_new_mem_buf(sig, sig_len);
// 	pk7sig = d2i_PKCS7_bio(bsig, NULL);
// 	if (pk7sig == NULL)
// 		goto exit;
// 
// 	bdata = BIO_new(BIO_s_file());
// 	if (bdata == NULL)
// 		goto exit;
// 	BIO_read_filename(bdata, file);
// 
// 	bsegs = BIO_new(BIO_f_segments());
// 	if (bsegs == NULL)
// 		goto exit;
// 
// 	bsegs->next_bio = bdata;
// 	BIO_set_segments(bsegs, byte_range, byte_range_len);
// 
// 	/* Find the certificates in the pk7 file */
// 	bcert = BIO_new_mem_buf(adobe_ca, sizeof(adobe_ca));
// 	pk7cert = d2i_PKCS7_bio(bcert, NULL);
// 	if (pk7cert == NULL)
// 		goto exit;
// 
// 	t = OBJ_obj2nid(pk7cert->type);
// 	switch (t)
// 	{
// 	case NID_pkcs7_signed:
// 		certs = pk7cert->d.sign->cert;
// 		break;
// 
// 	case NID_pkcs7_signedAndEnveloped:
// 		certs = pk7cert->d.sign->cert;
// 		break;
// 
// 	default:
// 		break;
// 	}
// 
// 	st = X509_STORE_new();
// 	if (st == NULL)
// 		goto exit;
// 
// 	/* Add the certificates to the store */
// 	if (certs != NULL)
// 	{
// 		int i, n = sk_X509_num(certs);
// 
// 		for (i = 0; i < n; i++)
// 		{
// 			X509 *c = sk_X509_value(certs, i);
// 			X509_STORE_add_cert(st, c);
// 		}
// 	}
// 
// 	res = pk7_verify(st, pk7sig, bsegs, ebuf, ebufsize);
// 
// exit:
// 	BIO_free(bsig);
// 	BIO_free(bdata);
// 	BIO_free(bsegs);
// 	BIO_free(bcert);
// 	PKCS7_free(pk7sig);
// 	PKCS7_free(pk7cert);
// 	X509_STORE_free(st);
// 
// 	return res;
// }

typedef struct pdf_designated_name_openssl_s
{
	pdf_designated_name base;
	char buf[8192];
} pdf_designated_name_openssl;

typedef struct Z_openssl_signer_s
{
	int ref;
	X509 *cert;
	EVP_PKEY *key;
    STACK_OF(X509) *ca;
} Z_openssl_signer;

Z_openssl_signer* Z_openssl_signer_new(fz_context *ctx, const char *pfile, const char *pass);
Z_openssl_signer* Z_keep_signer(fz_context *ctx,  Z_openssl_signer *signer);
static void Z_InitOpenSSL(fz_context *ctx);
static void Z_openssl_drop_signer(fz_context *ctx, Z_openssl_signer *signer);
static Z_openssl_signer *Z_openssl_keep_signer(fz_context *ctx, Z_openssl_signer *signer);
typedef int(Z_SignDev_drop_fn)(Z_sign_device *signDev, fz_context *ctx);
typedef Z_sign_device*(Z_SignDev_keep_fn)(Z_sign_device *signdev, fz_context *ctx); 
typedef fz_buffer* (Z_sign_fn)(Z_sign_device *signDev, fz_context *ctx, pdf_document *, const char *filename, pdf_obj *byterange);
static fz_buffer* Z_sign_inner(Z_sign_device *signDev, fz_context *ctx, pdf_document *doc, const char *filename, pdf_obj *byrterange);
static int Z_signdev_drop_inner(Z_sign_device *signDev, fz_context *ctx);
static Z_sign_device *Z_signdev_keep_inner(Z_sign_device *signDev, fz_context *ctx);
static BIO* Z_file_segment_BIO(fz_context *ctx, const char *filename, pdf_obj *byterange);
static fz_buffer* Z_openssl_sign_bio(fz_context *ctx, Z_openssl_signer *signer, BIO *bio);
static pdf_obj *Z_pdf_addSign_annot_ap_image (Z_PdfSignContext *signctx);
static pdf_obj *Z_pdf_addSign_annot_ap_script(Z_PdfSignContext *signctx);
static pdf_obj *Z_pdf_addSign_annot_appearence(fz_context *ctx, pdf_document *doc, pdf_page *page);
static pdf_obj *Z_pdf_addSign_annot_oc(fz_context * ctx, pdf_document *doc);
// static void *Z_didSignedCallback(fz_context *ctx, Z_sign_device *signdev, char *signeddata, int len);

struct Z_sign_device_context_s
{
    int ref;
    void *signer;
    Z_sign_fn *dosign; 
    fz_buffer *signdata;
    Z_SignDev_drop_fn *signdevdrop;
    Z_SignDev_keep_fn *signdevkeep;
};

Z_sign_device *Z_openssl_SignDev_new(fz_context *ctx, const char *file, const char *pw)
{
    Z_sign_device *signDev = NULL;
    Z_openssl_signer *signer = NULL;
    fz_try(ctx)
    {
        Z_InitOpenSSL(ctx);
        signer = Z_openssl_signer_new(ctx, file, pw);

        signDev = fz_malloc_struct(ctx, Z_sign_device);
        memset(signDev, 0, sizeof(Z_sign_device));

        signDev->ref = 1;
        signDev->signer = signer;
        signDev->dosign = Z_sign_inner;
        signDev->signdevdrop = Z_signdev_drop_inner;
        signDev->signdevkeep = Z_signdev_keep_inner;
    }
    fz_catch(ctx) {
		Z_openssl_drop_signer(ctx, signer);
        fz_rethrow(ctx);
    }
    return signDev;
}

static fz_buffer* Z_sign_inner(Z_sign_device *signdev, fz_context *ctx, pdf_document * doc,
        const char *filename, pdf_obj *byterange)
{
    BIO *bio = NULL;
    fz_try(ctx) {
        bio = Z_file_segment_BIO(ctx, filename, byterange);
        signdev->signdata = Z_openssl_sign_bio(ctx, (Z_openssl_signer*)signdev->signer, bio);
    }
    fz_always(ctx) {
        if(bio) BIO_free_all(bio); 
    }
    fz_catch(ctx) {
        fz_rethrow(ctx); 
    }
    return signdev->signdata;
}

static int Z_signdev_drop_inner(Z_sign_device *this, fz_context *ctx)
{
    int ref = this->ref--;
    if( this->ref<=0 ) {
        Z_openssl_drop_signer(ctx, (Z_openssl_signer*)this->signer);
        fz_drop_buffer(ctx, this->signdata);
        fz_free(ctx, this);
    }
    return ref;
}

static Z_sign_device *Z_signdev_keep_inner(Z_sign_device *signdev, fz_context *ctx)
{
    signdev->ref++;
    return signdev;
}

static void add_from_bags(X509 **pX509, EVP_PKEY **pPkey, STACK_OF(PKCS12_SAFEBAG) *bags, const char *pw);

static void add_from_bag(X509 **pX509, EVP_PKEY **pPkey, PKCS12_SAFEBAG *bag, const char *pw)
{
	EVP_PKEY *pkey = NULL;
	X509 *x509 = NULL;
	PKCS8_PRIV_KEY_INFO *p8 = NULL;
	switch (M_PKCS12_bag_type(bag))
	{
	case NID_keyBag:
		p8 = bag->value.keybag;
		pkey = EVP_PKCS82PKEY(p8);
		break;

	case NID_pkcs8ShroudedKeyBag:
		p8 = PKCS12_decrypt_skey(bag, pw, (int)strlen(pw));
		if (p8)
		{
			pkey = EVP_PKCS82PKEY(p8);
			PKCS8_PRIV_KEY_INFO_free(p8);
		}
		break;

	case NID_certBag:
		if (M_PKCS12_cert_bag_type(bag) == NID_x509Certificate)
			x509 = PKCS12_certbag2x509(bag);
		break;

	case NID_safeContentsBag:
		add_from_bags(pX509, pPkey, bag->value.safes, pw);
		break;
	}

	if (pkey)
	{
		if (!*pPkey)
			*pPkey = pkey;
		else
			EVP_PKEY_free(pkey);
	}

	if (x509)
	{
		if (!*pX509)
			*pX509 = x509;
		else
			X509_free(x509);
	}
}

static void add_from_bags(X509 **pX509, EVP_PKEY **pPkey, STACK_OF(PKCS12_SAFEBAG) *bags, const char *pw)
{
	int i;

	for (i = 0; i < sk_PKCS12_SAFEBAG_num(bags); i++)
		add_from_bag(pX509, pPkey, sk_PKCS12_SAFEBAG_value(bags, i), pw);
}

Z_openssl_signer* Z_openssl_signer_new(fz_context *ctx, const char *pfile, const char *pass)
{
	BIO *pfxbio = NULL;
	PKCS12 *p12 = NULL;
	Z_openssl_signer *signer = NULL;

	fz_var(pfxbio);
	fz_var(p12);
	fz_var(signer);
	fz_try(ctx)
	{
		pfxbio = BIO_new_file(pfile, "r");
		if (pfxbio == NULL)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Can't open pfx file: %s", pfile);

		p12 = d2i_PKCS12_bio(pfxbio, NULL);
		if (p12 == NULL)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Invalid pfx file: %s", pfile);

		signer = fz_malloc_struct(ctx, Z_openssl_signer);
		signer->ref = 1;
        if( !PKCS12_parse(p12, pass, &signer->key, &signer->cert, &signer->ca) ){
            fz_throw(ctx, FZ_ERROR_GENERIC, ERR_error_string(ERR_get_error(), NULL));
        }
		if (signer->key == NULL)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to obtain public key");

		if (signer->cert == NULL)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to obtain certificate");
    }
	fz_always(ctx)
	{
		BIO_free(pfxbio);
		PKCS12_free(p12);
	}
	fz_catch(ctx)
	{
		Z_openssl_drop_signer(ctx, signer);
		fz_rethrow(ctx);
	}
    return signer;
}

static Z_openssl_signer *Z_openssl_keep_signer(fz_context *ctx,  Z_openssl_signer *signer)
{
	if (signer)
		signer->ref++;
	return signer;
}

static void Z_openssl_drop_signer(fz_context *ctx, Z_openssl_signer *signer)
{
    if(!signer) return;
    if ( --(signer->ref)==0 )
    {
        X509_free(signer->cert);
        EVP_PKEY_free(signer->key);
        sk_X509_free(signer->ca);
        fz_free(ctx, signer);
    }
}

BIO *Z_file_segment_BIO(fz_context *ctx, const char *filename,
        pdf_obj *byterange)
{
	BIO *bdata = NULL;
	BIO *bsegs = NULL;
	FILE *f = NULL;

	int (*brange)[2] = NULL;
	int brange_len = pdf_array_len(ctx, byterange)/2;

	fz_var(bdata);
	fz_var(bsegs);
	fz_var(f);

	fz_try(ctx)
	{
		int i;
		brange = fz_calloc(ctx, brange_len, sizeof(*brange));
		for (i = 0; i < brange_len; i++)
		{
			brange[i][0] = pdf_to_int(ctx, pdf_array_get(ctx, byterange, 2*i));
			brange[i][1] = pdf_to_int(ctx, pdf_array_get(ctx, byterange, 2*i+1));
		}
        bdata = BIO_new_file(filename, "rb");
		bsegs = BIO_new(BIO_f_segments());
		if (bsegs == NULL)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to create segment filter");

		bsegs->next_bio = bdata;
		BIO_set_segments(bsegs, brange, brange_len);
    }
    fz_always(ctx) {
        if(f) fclose(f);
    }
    fz_catch(ctx) {
        if(brange) fz_free(ctx, brange);
        fz_rethrow(ctx);

    }
    return bsegs;
}

static fz_buffer *Z_openssl_sign_bio(fz_context *ctx, Z_openssl_signer *signer,
        BIO *bio) 
{ 
    if( !signer || !signer->cert || !signer->key ){
        return NULL;
    }
    PKCS7 *p7 = NULL;
    fz_buffer *signdata = NULL;
    fz_try(ctx) {
        unsigned char *data = NULL;
        int size = 0;
        p7 = PKCS7_sign(signer->cert, signer->key, signer->ca, 
                bio, PKCS7_DETACHED|PKCS7_BINARY);
        if( !p7 ){
            fz_throw(ctx, FZ_ERROR_GENERIC, ERR_error_string(ERR_get_error(), NULL));
        }
        size = i2d_PKCS7(p7, NULL);
        data = fz_malloc(ctx, size);
        unsigned char* tmpdata = data;
        memset(data, 0, size);
        i2d_PKCS7(p7, &tmpdata);
        signdata = fz_new_buffer_from_data(ctx, data, size);
    }
    fz_always(ctx) {
        if(p7) PKCS7_free(p7);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return signdata;
}
fz_buffer *Z_sign(Z_sign_device *signdev, fz_context *ctx, pdf_document *doc, 
        const char *filename, pdf_obj *byterange) 
{
    fz_buffer *buff= NULL;
    fz_try(ctx) {
        buff = signdev->dosign(signdev, ctx, doc, filename, byterange);
    }
    fz_always(ctx) {
    }
    fz_catch(ctx) {
        // printf("sign error:%s\n", ctx->error->message);
        fz_rethrow(ctx);
    } 
    return buff;
}

void Z_pdf_signComplete(Z_sign_device *signCtx, fz_context *ctx, pdf_document *doc, 
        const char *filename, pdf_obj *byterange, int ofs, int size)
{
    fz_try(ctx) {
        fz_buffer *sign = Z_sign(signCtx, ctx, doc, filename, byterange);
        Z_writeSign(ctx, filename, sign, ofs, size);
    }
    fz_catch(ctx) {
        printf("sign error:%s\n", ctx->error->message);
    }
}

int Z_signdev_drop(Z_sign_device *dev, fz_context *ctx)
{
    if(dev && dev->signdevdrop) {
        return dev->signdevdrop(dev, ctx);
    } 
    return 0; 
}

Z_sign_device *Z_signdev_keep(Z_sign_device *dev, fz_context *ctx)
{
    if(dev && dev->signdevkeep) {
        return dev->signdevkeep(dev, ctx);
    }
    return NULL; 
} 
// void pdf_sign_signature(fz_context *ctx, pdf_document *doc, pdf_widget *widget, const char *sigfile, const char *password)
// {
// 	pdf_signer *signer = pdf_read_pfx(ctx, sigfile, password);
// 	pdf_designated_name *dn = NULL;
// 	fz_buffer *fzbuf = NULL;
// 
// 	fz_try(ctx)
// 	{
// 		char *dn_str;
// 		pdf_obj *wobj = ((pdf_annot *)widget)->obj;
// 		fz_rect rect = fz_empty_rect;
// 
// 		pdf_signature_set_value(ctx, doc, wobj, signer);
// 
// 		pdf_to_rect(ctx, pdf_dict_get(ctx, wobj, PDF_NAME_Rect), &rect);
// 		/* Create an appearance stream only if the signature is intended to be visible */
// 		if (!fz_is_empty_rect(&rect))
// 		{
// 			dn = pdf_signer_designated_name(ctx, signer);
// 			fzbuf = fz_new_buffer(ctx, 256);
// 			if (!dn->cn)
// 				fz_throw(ctx, FZ_ERROR_GENERIC, "Certificate has no common name");
// 
// 			fz_buffer_printf(ctx, fzbuf, "cn=%s", dn->cn);
// 
// 			if (dn->o)
// 				fz_buffer_printf(ctx, fzbuf, ", o=%s", dn->o);
// 
// 			if (dn->ou)
// 				fz_buffer_printf(ctx, fzbuf, ", ou=%s", dn->ou);
// 
// 			if (dn->email)
// 				fz_buffer_printf(ctx, fzbuf, ", email=%s", dn->email);
// 
// 			if (dn->c)
// 				fz_buffer_printf(ctx, fzbuf, ", c=%s", dn->c);
// 
// 			(void)fz_buffer_storage(ctx, fzbuf, (unsigned char **) &dn_str);
// 			pdf_set_signature_appearance(ctx, doc, (pdf_annot *)widget, dn->cn, dn_str, NULL);
// 		}
// 	}
// 	fz_always(ctx)
// 	{
// 		pdf_drop_signer(ctx, signer);
// 		pdf_drop_designated_name(ctx, dn);
// 		fz_drop_buffer(ctx, fzbuf);
// 	}
// 	fz_catch(ctx)
// 	{
// 		fz_rethrow(ctx);
// 	}
// }


struct Z_pdf_SignContext_s
{
    fz_context *ctx;
    pdf_document *doc;
    int pageno;
    int stmtype; // 1:image, 2:compressedscriptbuffer
    struct IMAGE_CTX {
        fz_image *image;
        pdf_obj *obj;
    };

    union {
        struct IMAGE_CTX imagectx;
        fz_buffer *scriptbuf;
    };
    pdf_obj *dsblankobj;
    pdf_obj *extgstate;
    pdf_obj *annot;
    pdf_obj *annotap;
    pdf_obj *annotfrm;
    pdf_obj *annotoc;
    pdf_obj *signobj;
    fz_rect range;
};

int Z_pdf_signatures_supported(fz_context *ctx) {return 1;}

Z_PdfSignContext *Z_PdfSignCtxInit(fz_context *ctx, pdf_document *doc, 
        int pageno, int stmtype, void *stm, fz_rect r)
{
    Z_PdfSignContext* signCtx = fz_malloc(ctx, sizeof(*signCtx));
    memset(signCtx, 0, sizeof(*signCtx));

    signCtx->ctx = ctx;
    signCtx->doc = doc;
    signCtx->pageno = pageno;
    signCtx->stmtype = stmtype;

    if( 1==stmtype ) {
        signCtx->imagectx.image = stm;
        signCtx->imagectx.obj = NULL; 
        fz_keep_image(ctx, (fz_image*)stm);
    } 

    if( 2==stmtype ) {
        signCtx->scriptbuf = stm;
        fz_keep_buffer(ctx, signCtx->scriptbuf);
    }
    // signCtx->page = pdf_load_page(ctx, doc, pageno);
    signCtx->extgstate = NULL;
    signCtx->annot = NULL;
    signCtx->annotap = NULL;
    signCtx->annotfrm = NULL;
    signCtx->annotoc = NULL;
    signCtx->signobj = NULL;

    signCtx->range = r;
    return signCtx;
}

int Z_PdfSignCtxClear(Z_PdfSignContext *signCtx) {
    if( 1==signCtx->stmtype ) {
        fz_drop_image(signCtx->ctx, signCtx->imagectx.image);
        signCtx->imagectx.image = NULL;
        pdf_drop_obj(signCtx->ctx, signCtx->imagectx.obj);
        signCtx->imagectx.obj = NULL;
    } 
    if( 2==signCtx->stmtype ) {
        fz_drop_buffer(signCtx->ctx, signCtx->scriptbuf);
        signCtx->scriptbuf = NULL;
    }

    pdf_drop_obj(signCtx->ctx, signCtx->dsblankobj);
    pdf_drop_obj(signCtx->ctx, signCtx->extgstate);
    pdf_drop_obj(signCtx->ctx, signCtx->annot);
    pdf_drop_obj(signCtx->ctx, signCtx->annotap);
    pdf_drop_obj(signCtx->ctx, signCtx->annotfrm);
    pdf_drop_obj(signCtx->ctx, signCtx->annotoc);
    pdf_drop_obj(signCtx->ctx, signCtx->signobj);

    return 1;
}

pdf_obj *Z_pdf_getPageRef(fz_context *ctx, pdf_document *doc, int number) {
    pdf_obj *pageref;

	if (doc->file_reading_linearly)
	{
		pageref = pdf_progressive_advance(ctx, doc, number);
		if (pageref == NULL)
			fz_throw(ctx, FZ_ERROR_TRYLATER, "page %d not available yet", number);
	}
	else
		pageref = pdf_lookup_page_obj(ctx, doc, number);

    return pageref;
}

pdf_obj *Z_pdf_add_dsblankfrm(fz_context *ctx, pdf_document *doc) 
{
    fz_rect r = {0,0,100,100};
    pdf_obj *obj = pdf_new_xobject(ctx, doc, &r, &fz_identity);
    pdf_obj *subobj = pdf_dict_getp(ctx, obj, "Resources/ProcSet");
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageB"));
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageC"));
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageI"));
    pdf_dict_puts_drop(ctx, obj, "FormType", pdf_new_int(ctx, doc, 1));
    obj = pdf_add_object_drop(ctx, doc, obj);

    fz_buffer *buf = fz_new_buffer(ctx, 9);
    fz_buffer_printf(ctx, buf, "%% DSBlank");
    pdf_update_stream(ctx, doc, obj, buf, 0);
    fz_drop_buffer(ctx, buf); 
    return obj;
}

pdf_obj *Z_pdf_add_ntko_extgstate(fz_context *ctx, pdf_document *doc) 
{
    const char *extgstate_string = "<</Type/ExtGState/BM/Darken/OP true " \
            "/AIS false /ca 1.0>>";
    pdf_obj *obj = pdf_new_obj_from_str(ctx, doc, extgstate_string);
    return pdf_add_object_drop(ctx, doc, obj);
}

// pageref = pdf_lookup_page_obj(ctx, doc, number);
// pagenum = pdf_lookup_page_number
// pdf_obj_parent_num(ctx, page->me);
static pdf_obj *Z_pdf_addSign_annot(fz_context *ctx, pdf_document *doc,
    int pageno, fz_rect *range) 
{
    const char * objstr = "<</Type/Annot/Subtype/Widget/FT/Sig/F 0>>";
    pdf_obj *annot = NULL;

    annot = pdf_new_obj_from_str(ctx, doc, objstr);
    // add "/P"(parent) object(actrually the page object)
    pdf_obj *pageref = pdf_lookup_page_obj(ctx, doc, pageno);
    pdf_dict_put(ctx, annot, PDF_NAME_P, pageref);

    // add "/T(string)" object, the unique unid of signature
    char *tagstr = new_unique_string(ctx, NULL, NULL);
    pdf_obj *tag = pdf_new_string(ctx, doc, tagstr, strlen(tagstr));
    pdf_dict_puts_drop(ctx, annot, "T", tag);
    fz_free(ctx, tagstr);

    // Z_pdf_obj_display(ctx, annot);
    // add "/Rect[]" pdf object
    pdf_obj *rectobj = pdf_new_rect(ctx, doc, range);
    pdf_dict_puts_drop(ctx,  annot, "Rect", rectobj);

    annot = pdf_add_object_drop(ctx, doc, annot); 
    return annot;
}

static pdf_obj *Z_pdf_addSign_annot_ap_script(Z_PdfSignContext *signctx)
{
    fz_context *ctx = signctx->ctx;
    pdf_document *doc = signctx->doc;
    fz_buffer *bf = signctx->scriptbuf;

    char *objstr = "<</Type/XObject/Subtype/Form/FormType 1/Filter/FlateDecode>>";
    pdf_obj *frm = NULL;

    frm = pdf_new_obj_from_str(ctx, doc, objstr);
    frm = pdf_add_object_drop(ctx, doc, frm);
    pdf_update_stream(ctx, doc, frm, bf, 1); 
    return frm;
}

static pdf_obj *Z_pdf_addSign_annot_ap_image(Z_PdfSignContext *signctx) 
{
    fz_context *ctx = signctx->ctx;
    pdf_document *doc = signctx->doc; 
    fz_image *image = signctx->imagectx.image;
    fz_rect *rect = &signctx->range;

    const char *imagename = "ntkoimage";
    const char *script = "q /%s gs 1 0 0 1 0 0 cm\n"    \
            "100.00 0 0 100.00 0 0 cm /%s Do Q\n"       \
            "q 0.9 0 0 0.9 5 5 cm /n3 Do Q";

    pdf_obj *xobj = NULL;
    pdf_obj *resobj = NULL;
    pdf_obj *subobj = NULL;

    xobj = pdf_new_dict(ctx, doc, 10); // mainobj
    pdf_dict_puts_drop(ctx, xobj, "Type", PDF_NAME_XObject);
    pdf_dict_puts_drop(ctx, xobj, "Subtype", PDF_NAME_Form);
    /* "/BBox[]" */
    fz_rect r = {0, 0, 100, 100};
    subobj = pdf_new_rect(ctx, doc, &r);
    pdf_dict_put_drop(ctx, xobj, PDF_NAME_BBox, subobj);

    /* "/Matrix[] */
    subobj = pdf_new_matrix(ctx, doc, &fz_identity);
    pdf_dict_put_drop(ctx, xobj, PDF_NAME_Matrix, subobj);

    // /Resources
    resobj = pdf_new_dict(ctx, doc, 10);

    /* /Resources<</ProcSet [/PDF/ImageB/ImageC/ImageI] */
    subobj = pdf_new_array(ctx, doc, 5);
    pdf_array_push_drop(ctx, subobj, PDF_NAME_PDF);
    pdf_array_push_drop(ctx, subobj, PDF_NAME_Text);
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageB"));
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageC"));
    pdf_array_push_drop(ctx, subobj, pdf_new_name(ctx, doc, "ImageI"));
    pdf_dict_put_drop(ctx, resobj, PDF_NAME_ProcSet, subobj);

    /* /Resources<</XObject<</n0 /n1 /n2 /n3 /signimagename ..... */
    subobj = pdf_new_dict(ctx, doc, 5);
    // create a bsblank frm
    if(!signctx->dsblankobj) {
        signctx->dsblankobj = Z_pdf_add_dsblankfrm(ctx, doc);
    }

    pdf_dict_puts(ctx, subobj, "n0", signctx->dsblankobj);
    pdf_dict_puts(ctx, subobj, "n1", signctx->dsblankobj);
    pdf_dict_puts(ctx, subobj, "n2", signctx->dsblankobj);
    pdf_dict_puts(ctx, subobj, "n3", signctx->dsblankobj);
    pdf_dict_puts(ctx, subobj, "n4", signctx->dsblankobj);

    pdf_dict_puts_drop(ctx, xobj, "FormType", pdf_new_int(ctx, doc, 1));

    if( !signctx->imagectx.obj) {
        signctx->imagectx.obj = pdf_add_image(ctx, doc, image, 0);
    }

    char *uniqeName = new_unique_string(ctx, (char*)imagename, NULL);
    pdf_dict_puts(ctx, subobj, uniqeName, signctx->imagectx.obj);

    pdf_dict_put_drop(ctx, resobj, PDF_NAME_XObject, subobj);
    pdf_dict_put_drop(ctx, xobj, PDF_NAME_Resources, resobj);

    xobj = pdf_add_object_drop(ctx, doc, xobj);

    fz_buffer *buff = fz_new_buffer(ctx, 128);
    fz_buffer_printf(ctx, buff, script, ntkoextobjname, uniqeName);
    pdf_update_stream(ctx, doc, xobj, buff, 1);
    fz_drop_buffer(ctx, buff); buff=NULL;

    return xobj;
}

static pdf_obj *Z_pdf_addSign_annot_appearence(fz_context *ctx, pdf_document *doc,
        pdf_page *page) 
{
    const char *objstring = "<</Type/XObject/Subtype/Form"       \
        "/Resources<</ProcSet[/PDF/Text/ImageB/ImageC/ImageI]>>" \
        "/BBox[0 0 100 100]/FormType 1/Matrix [1 0 0 1 0 0]>>";
    pdf_obj *apobj = pdf_new_obj_from_str(ctx, doc, objstring);

    fz_buffer *buf = fz_new_buffer(ctx, 64);
    fz_buffer_printf(ctx, buf, "q 1 0 0 1 0 0 cm /FRM Do Q");

    apobj = pdf_add_object(ctx, doc, apobj); 
    pdf_update_stream(ctx, doc, apobj, buf, 0); 
    fz_drop_buffer(ctx, buf);

    return apobj;
}

static pdf_obj *Z_pdf_addSign_annot_oc(fz_context * ctx, pdf_document *doc)
{
    const char *objstr = "<</Type/OCG/Name(Copyright notice)/Usage<</Print<</PrintState/OFF>>>>>>"; 
    pdf_obj *obj = pdf_new_obj_from_str(ctx, doc, objstr);
    return pdf_add_object_drop(ctx, doc, obj); 
}

int Z_pdf_add_sign(Z_PdfSignContext *signctx, Z_sign_device *signdev)
{
    if(!signctx || !signdev)
        return 0; 

    char path[64] = {0};
    snprintf(path, 64, "Resources/ExtGState/%s", ntkoextobjname);

    fz_context *ctx = signctx->ctx;
    pdf_document *doc = signctx->doc;
    pdf_page *page = NULL;//  pdf_load_page(signctx->ctx, signctx->doc, signctx->pageno);
    
    signctx->dsblankobj = Z_pdf_add_dsblankfrm(ctx, doc);
    signctx->extgstate = Z_pdf_add_ntko_extgstate(ctx, doc);

    if(1==signctx->stmtype)
        signctx->annotfrm = Z_pdf_addSign_annot_ap_image(signctx);
    if(2==signctx->stmtype)
        signctx->annotfrm = Z_pdf_addSign_annot_ap_script(signctx);

    signctx->annot = Z_pdf_addSign_annot(ctx, doc, signctx->pageno, &signctx->range);
    signctx->annotap = Z_pdf_addSign_annot_appearence(ctx, doc, page);
    signctx->annotoc = Z_pdf_addSign_annot_oc(ctx, doc);
    pdf_dict_putp(ctx, signctx->annotap, "Resources/XObject/FRM", signctx->annotfrm);
    pdf_dict_putp(ctx, signctx->annotfrm, path, signctx->extgstate);
    pdf_dict_putp(ctx, signctx->annot, "AP/N", signctx->annotap);
    pdf_dict_putp(ctx, signctx->annot, "DR/XObject/FRM", signctx->annotfrm);
    pdf_dict_put (ctx, signctx->annot, PDF_NAME_OC, signctx->annotoc);

    pdf_obj *pageobj = pdf_lookup_page_obj(ctx, doc, 0);
    if(pageobj){
        pdf_obj *annots = pdf_dict_get(ctx, pageobj, PDF_NAME_Annots);
        if(!annots) {
            annots = pdf_new_array(ctx, doc, 1);
            pdf_dict_put_drop(ctx, pageobj, PDF_NAME_Annots, annots);
        }
        pdf_array_push(ctx, annots, signctx->annot);
    }

    // add /catalog/acrofrom<</signflags 3/fields[]
	pdf_obj *catalog = pdf_dict_get(ctx, pdf_trailer(ctx, doc), PDF_NAME_Root);
    pdf_obj *acroform = pdf_dict_get(ctx, catalog, PDF_NAME_AcroForm);
    if(!acroform) {
        acroform = pdf_new_dict(ctx, doc, 2);
        pdf_dict_put_drop(ctx, catalog, PDF_NAME_AcroForm, acroform);
    }

    pdf_obj *fields = pdf_dict_get(ctx, acroform, PDF_NAME_Fields);
    if(!fields) {
        fields = pdf_new_array(ctx, doc, 1);
        pdf_dict_put_drop(ctx, acroform, PDF_NAME_Fields, fields);
    }
    pdf_array_push(ctx, fields, signctx->annot);

    pdf_obj *signflag = pdf_dict_get(ctx, acroform, PDF_NAME_SigFlags);
    if(!signflag) {  
        pdf_dict_put_drop(ctx, acroform, PDF_NAME_SigFlags, pdf_new_int(ctx, doc, 3));
    }

    pdf_drop_page(ctx, page); 

    fz_try(ctx){
        signctx->signobj = pdf_signature_set_value(ctx, doc, signctx->annot, signdev);
    }
    fz_catch(ctx){
        printf("openssl error:%s\n",ctx->error->message);
    }
    return 1;
} 

void Z_PdfSignCtxDisplay(Z_PdfSignContext *signctx)
{
    fz_context *ctx = signctx->ctx;
    pdf_document *doc = signctx->doc;
    printf("===pdf sign context objs display====\n");
    fz_output *o = fz_new_output_with_file_ptr(ctx, stdout, 0);
    pdf_print_obj(ctx, o, signctx->extgstate, 1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->extgstate), 1); printf("\n");
    pdf_print_obj(ctx, o, signctx->annot,   1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->annot), 1); printf("\n");
    pdf_print_obj(ctx, o, signctx->annotap, 1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->annotap), 1); printf("\n");
    pdf_print_obj(ctx, o, signctx->annotfrm,1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->annotfrm),1); printf("\n");
    pdf_print_obj(ctx, o, signctx->annotoc, 1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->annotoc), 1); printf("\n");
    if(signctx->stmtype==1){
        pdf_print_obj(ctx, o, signctx->imagectx.obj, 1); printf("\n");
        pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->imagectx.obj), 1); printf("\n");
    }
    pdf_print_obj(ctx, o, signctx->signobj, 1); printf("\n");
    pdf_print_obj(ctx, o, pdf_resolve_indirect(ctx, signctx->signobj), 1); printf("\n");
    printf("====pdf sign context objs display====\n");
    fz_drop_output(ctx, o);
}

void Z_InitOpenSSL(fz_context *ctx) {
    if(1==OpensslInited) {
        return; 
    }
    ERR_load_crypto_strings();
    // add all digest and chiphers
    OpenSSL_add_all_algorithms();
    ERR_clear_error();
    OpensslInited = 1;
}  

char *base64(const void *input, int length)  
{  
    BIO *bmem, *b64;  
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());  
    bmem = BIO_new(BIO_s_mem());  
    b64 = BIO_push(b64, bmem);  
    BIO_write(b64, input, length);  
    BIO_flush(b64);  
    BIO_get_mem_ptr(b64, &bptr);  
    char *buff = (char*)malloc(bptr->length);  
    memcpy(buff, bptr->data, bptr->length-1);  
    buff[bptr->length-1] = 0;  

    BIO_free_all(b64);  

    return buff;  
}  
  
int decodeBase64(char *inb64, void *obf, int obflen)
{  
    BIO *b64, *bmem;  

    b64 = BIO_new(BIO_f_base64());  
    bmem = BIO_new_mem_buf(inb64, strlen((const char*)inb64));  
    bmem = BIO_push(b64, bmem);   
    int err=0;  
    int i=0;  
    do{  
        err = BIO_read(bmem, (void *)( (char *)obf+i++), 1);  
    } while( err==1 && i<obflen );  
    BIO_free_all(bmem);  

    return --i;
}  

#else /* HAVE_OPENSSL */
#pragma message("HAVE_OPENSSL is not defined!!!")
int Z_pdf_signatures_supported(fz_context *ctx){return 0;}
Z_sign_device *Z_openssl_SignCtx_new(fz_context *ctx, const char *file,
        const char *pw){return NULL;}
int Z_signdev_drop(Z_sign_device *this, fz_context *ctx){return 0;}
void Z_pdf_signComplete(Z_sign_device *signdev, fz_context *ctx,
        pdf_document *doc, const char *filename, pdf_obj *byterange,
        int ofs, int size){return;}
fz_buffer *Z_sign(Z_sign_device *signdev, fz_context *ctx, pdf_document *doc,
        const char *filename, pdf_obj *byterange){return NULL;}
void Z_InitOpenSSL(fz_context *ctx) {
    fz_throw(ctx, FZ_ERROR_GENERIC, "no openssl supporeted!!");
};
#endif /* HAVE_OPENSSL */










void Z_writeSign(fz_context *ctx, const char*file, fz_buffer *buff, 
        int ofs, int size)
{
    if(!buff) 
        return;

    int len = buff->len;
    unsigned char *data = buff->data;
    int i;

    if (len*2 + 2 > size)
        fz_throw(ctx, FZ_ERROR_GENERIC, "Insufficient space for data");

    FILE *f = fz_fopen(file, "rb+");
    if (f == NULL)
        fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to write data");
    fz_fseek(f, ofs+1, SEEK_SET);
    for (i = 0; i < len; i++)
        fprintf(f, "%02x", data[i]);
    fclose(f);
}
