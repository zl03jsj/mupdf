/*
 * =====================================================================================
 *       Filename:  z_math.h
 *    Description:  
 *        Version:  2.0
 *        Created:  06/23/2016 14:53:43
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  zl(88911562@qq.com), 
 *   Organization:  
 * =====================================================================================
 */
//_________________________________
// stl on mac location is:
// /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
// Mac OS X 10.9+ no longer uses GCC/libstdc++ but uses libc++ and Clang.
//---------------------------------
//td c++ (STL, streams, ...) : Modified libstdc++ headers for use with ctags 
//use ctags for c++(stl,streams....)
//www.vim.org/scripts/script.php?script_id=2358
//ctags -R --c++-kinds=+p --fields=+ias --extra=+q --language-force=c++ cpp_src 
//sudo ctags -R --c++-kinds=+p --fields=+ias --extra=+q --language-force=c++ ./
//================================= */
#ifndef z_math_h_
#define z_math_h_

#include "mupdf/fitz.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef fz_point z_point;
typedef struct z_fpoint_s z_fpoint;
typedef struct z_ipoint_s z_ipoint;
typedef struct z_fpoint_array_s z_fpoint_array;
typedef struct z_fpoint_arraylist_node_s  z_fpoint_arraylist_node;
typedef struct z_fpoint_arraylist_s z_fpoint_arraylist;

struct z_fpoint_s{
    fz_point p;
    float w;
};

struct z_ipoint_s {
    fz_point p;
    int64_t t;
};

struct z_fpoint_array_s {
	z_fpoint *point;
    float maxwidth;
    float minwidth;
    int ref;
	int len;
    int cap;
};

struct z_fpoint_arraylist_node_s {
    z_fpoint_array *a;
    z_fpoint_arraylist_node *n;
};

struct z_fpoint_arraylist_s {
    int ref;
    z_fpoint_arraylist_node *first;
    z_fpoint_arraylist_node *end;
	z_fpoint_arraylist_node *cur;
};

z_fpoint_array *z_keep_fpoint_array(fz_context *ctx, z_fpoint_array *a);
void z_drop_fpoint_array(fz_context *ctx, z_fpoint_array *a);

z_fpoint_arraylist* z_keep_fpoint_arraylist(fz_context *ctx, z_fpoint_arraylist *l);
void z_drop_fpoint_arraylist(fz_context *ctx, z_fpoint_arraylist *l);

z_fpoint_array *z_new_fpoint_array(fz_context *ctx, int initsize, float maxwidth, float minwidth);
z_fpoint_array *z_resize_fpoints_array(fz_context *ctx, z_fpoint_array* a, int size);

z_fpoint_arraylist *z_new_fpoint_arraylist(fz_context *ctx);
void z_fpoint_arraylist_append(fz_context *ctx, z_fpoint_arraylist *l, z_fpoint_array *a);
// must be drop after used
z_fpoint_array *z_fpoint_arraylist_append_new(fz_context *ctx, z_fpoint_arraylist *l, float maxwidth, float minwidth);
void z_fpoint_arraylist_removelast(fz_context *ctx, z_fpoint_arraylist *l);

float z_movespeed(z_ipoint s, z_ipoint e);
float z_distance(fz_point s, fz_point e);
void  z_fpoint_add_xyw(fz_context *ctx, z_fpoint_array *a, float x, float y, float w);
void  z_fpoint_add(fz_context *ctx, z_fpoint_array *a, z_fpoint p);
void  z_fpoint_differential_add(fz_context *ctx, z_fpoint_array *a, z_fpoint p);
void  z_square_bezier(fz_context *ctx, z_fpoint_array *a, z_fpoint b, fz_point c, z_fpoint e);
float z_linewidth(z_ipoint b, z_ipoint e, float w, float step);

float z_insertPoint(fz_context *ctx, z_fpoint_array *arr, fz_point lastpoint, int64_t lastms, float lastwidth, fz_point point, int64_t ms);
void  z_insertLastPoint(fz_context *ctx, z_fpoint_array *arr, fz_point e);


typedef struct z_list_node_s z_list_node;
struct z_list_node_s {
    void *data; 
    z_list_node *n;
    z_list_node *p;
}; 
typedef void*(*z_list_node_alloc_fun)(fz_context *ctx);
typedef void(*z_list_node_drop_fun) (fz_context *ctx, void *data);


struct z_list_s {
    z_list_node_alloc_fun alloc;
    z_list_node_drop_fun  drop;
    z_list_node *first;
    z_list_node *last;
};
typedef struct z_list_s z_list;

z_list *z_list_new(fz_context *ctx, z_list_node_alloc_fun allocfun, z_list_node_drop_fun dropfun);
void *z_list_append_new(fz_context *ctx, z_list *zlist);
void *z_list_remove_last(fz_context *ctx, z_list *zlist);
void z_list_clear(fz_context *ctx, z_list *zlist);
void z_list_free(fz_context *ctx, z_list *zlist);

/* digest must be 33 char size  */
void z_text_md5(fz_context *ctx, const char* str, char *digest);

#ifdef __cplusplus
}
#endif

#endif
