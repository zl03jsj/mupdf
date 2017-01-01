/*
 * =====================================================================================
 *
 *       Filename:  z_util.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016/12/30 10时56分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "mupdf/fitz.h"
#include "../../thirdparty/zlib/zlib.h"

fz_buffer *z_new_buffer_deflate_data(fz_context *ctx, char *p, size_t n) {
	fz_buffer *buf;
	uLongf csize;
	int t;
	uLong longN = (uLong)n;
	unsigned char *data;
	size_t cap;

	if (n != (size_t)longN)
		fz_throw(ctx, FZ_ERROR_GENERIC, "Buffer to large to deflate");

	cap = compressBound(longN);
	data = fz_malloc(ctx, cap);
	buf = fz_new_buffer_from_data(ctx, data, cap);
	csize = (uLongf)cap;
	t = compress(data, &csize, (unsigned char*)p, longN);
	if (t != Z_OK)
	{
		fz_drop_buffer(ctx, buf);
		fz_throw(ctx, FZ_ERROR_GENERIC, "cannot deflate buffer");
	}
	fz_resize_buffer(ctx, buf, csize);
	return buf;
}

fz_buffer *z_deflate_buffer(fz_context *ctx, fz_buffer *bf) {
    unsigned char *d;
    size_t size = fz_buffer_extract(ctx, bf, &d);
    return z_new_buffer_deflate_data(ctx, (char*)d, size);
}

fz_buffer *z_new_buffer_filedata(fz_context *ctx, char *filename, int ofs, int n) {
    if(0==fz_file_exists(ctx, filename))
        return NULL;

	fz_stream *fs = fz_open_file(ctx, filename);
    unsigned char *d = fz_malloc(ctx, n);

	fz_seek(ctx, fs, ofs, 0);
	n = fz_read(ctx, fs, d, n);
	fz_drop_stream(ctx, fs);

    d = fz_resize_array(ctx, d, n, 1);

	return fz_new_buffer_from_data(ctx, d, n);
} 
