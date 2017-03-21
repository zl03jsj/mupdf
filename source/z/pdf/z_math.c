/*
 *
 * =====================================================================================
 *       Filename:  z_math.c
 *    Description:
 *        Version:  1.0
 *        Created:  06/23/2016 14:53:43
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  zl(88911562@qq.com), 
 *   Organization:  
 * =====================================================================================
 */
#include "mupdf/fitz.h"
#include "mupdf/z/z_math.h"
#include <math.h>
/***************************** mac stdlib location:
Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/stdio.h
*/
static float z_square(float f){ return (float)f*f; };
static float z_cubic_(float f){ return (float)powf(f, 3); };

typedef struct z_bezier_factors_s {
    float bezier_step;      // must be divisible by 1.0f
	float max_width_diff;   // max width diff between two near lines
    float max_move_speed;   // 
    float max_linewith;     
} z_bezier_factors ;

int z_point_equals(fz_point *p1, fz_point *p2) {
    return (p1->x==p2->x&&p1->y==p2->y) ? 1 : 0;
}

z_fpoint_array *z_keep_fpoint_array(fz_context *ctx, z_fpoint_array *a) {
    if(a) a->ref++;
    return a;
}

void z_drop_fpoint_array(fz_context *ctx, z_fpoint_array *a) {
    if(!a) return;

    if( !(--(a->ref)) ) {
        fz_free(ctx, a);
    }
}

z_fpoint_arraylist *z_keep_fpoint_arraylist(fz_context *ctx, z_fpoint_arraylist *l) {
    if(!l) return NULL;
    l->ref++;
    return l;
}

void z_drop_fpoint_arraylist(fz_context *ctx, z_fpoint_arraylist *l) {
    if(!l) return;

    if( !(--(l->ref)) ) {
        z_fpoint_arraylist_node *c = l->first;
        z_fpoint_arraylist_node *n;
        while(c) {
            z_drop_fpoint_array(ctx, c->a);
            n = c->n;
            fz_free(ctx, c);
            c = n;
        }
    } 
}

static const float defualt_max_width = 5.0f;
static const float default_min_width = 1.0f;

z_fpoint_array *z_new_fpoint_array(fz_context *ctx, int initsize, float maxwidth, float minwidth) {
    if(initsize<=0) return NULL;
    z_fpoint_array *a = fz_malloc(ctx, sizeof(z_fpoint_array));
    a->point = fz_malloc_array(ctx, initsize, sizeof(z_fpoint));
    a->ref = 1;
    a->len = 0;

    if(maxwidth<0 || minwidth<0 || maxwidth<minwidth ){
        fz_warn(ctx, "invalid maxwidth or minwidth, use default(max:%.2f, min:%.2f)\n", defualt_max_width, default_min_width);
        maxwidth = defualt_max_width;
        minwidth = default_min_width;
    }

    a->maxwidth = maxwidth;
    a->minwidth = minwidth;

    a->cap = initsize;
    return a;
}

z_fpoint_array *z_resize_fpoints_array(fz_context *ctx, z_fpoint_array* a, int size){
    if(!a || size<=0) return NULL;

    a->point = (z_fpoint*)fz_resize_array(ctx, a->point, size, sizeof(z_fpoint));
    a->cap = size;
    a->len = fz_min(a->cap, a->len);
    return a;
}

z_fpoint_arraylist *z_new_fpoint_arraylist(fz_context *ctx) {
    z_fpoint_arraylist *l = fz_malloc_struct(ctx, z_fpoint_arraylist);
    l->ref = 1;
    l->first = l->end = NULL;
    return l;
}

void z_fpoint_arraylist_append(fz_context *ctx, z_fpoint_arraylist *l, z_fpoint_array *a) {
    z_fpoint_arraylist_node *node = fz_malloc_struct(ctx, z_fpoint_arraylist_node);
    node->a = z_keep_fpoint_array(ctx, a);
    node->n = NULL;

    if(!l->first) {
        l->first = node;
    }
    else {
        l->end->n = node;
    }

    l->end = node;
}

z_fpoint_array *z_fpoint_arraylist_append_new(fz_context *ctx, z_fpoint_arraylist *l, float max, float min) {
    z_fpoint_array *a = z_new_fpoint_array(ctx, 24, max, min);
    z_fpoint_arraylist_append(ctx, l, a);
    return a; 
}

void z_fpoint_arraylist_removelast(fz_context *ctx, z_fpoint_arraylist *l) {
    
    z_fpoint_arraylist_node *c = l->first;

    z_drop_fpoint_array(ctx, l->end->a);
    fz_free(ctx, l->end);

    while(c->n != l->end) { c = c->n; }

    c->n = NULL;
    l->end = c; 
}

z_fpoint_array *z_auto_increase_fpoints_array(fz_context *ctx, z_fpoint_array *a) {
    int cap = a->cap + (a->cap+3)/4;
    return z_resize_fpoints_array(ctx, a, cap);
}

float z_movespeed(z_ipoint s, z_ipoint e) {
	float d = z_distance(s.p, e.p);
    return (0==d) ? 0 : d/(e.t-s.t);
}

float z_distance(z_point b, z_point e){
    return (float)sqrtf( z_square(e.x-b.x) + z_square(e.y-b.y) );
}

void  z_fpoint_add_xyw(fz_context *ctx, z_fpoint_array *a, float x, float y, float w)  {
	if( !a || (a->point[a->len-1].p.x==x && a->point[a->len-1].p.y==y) ) return;
	
    if(a->len==a->cap) 
        z_auto_increase_fpoints_array(ctx, a);

	z_fpoint *p = a->point + (a->len++);
	p->p.x = x; p->p.y = y; p->w = w;
}

void  z_fpoint_add(fz_context *ctx, z_fpoint_array *a, z_fpoint p) {
    z_fpoint_add_xyw(ctx, a, p.p.x, p.p.y, p.w);
}

void  z_fpoint_differential_add(fz_context *ctx, z_fpoint_array *a, z_fpoint p) {
    if(!a) return; 

    if( a->len==0 ) {
        z_fpoint_add(ctx, a, p);
        return;
    }

	float max_diff = 0.1f;
    z_fpoint *last = a->point + (a->len-1);
    fz_point sp = last->p;
	float sw = last->w;
	
    int n = ( fz_abs(p.w - last->w) / max_diff) + 1;
    float x_step = (p.p.x - sp.x) / n;
    float y_step = (p.p.y - sp.y) / n;
    float w_step = (p.w - sw)	  / n;
	
    int i;
    for(i=0; i<(n-1); i++ ){
		sp.x += x_step;
		sp.y += y_step;
		sw += w_step;
        z_fpoint_add_xyw(ctx, a, sp.x, sp.y, sw);
    }
	z_fpoint_add(ctx, a, p);
}

void  z_square_bezier(fz_context *ctx, z_fpoint_array *a, z_fpoint b, fz_point c, z_fpoint e){
	if(!a) return;
	const float f = 0.1f;
	for(float t=f; t<=1.0; t+=f ) {
		float x1 = z_square(1-t)*b.p.x + 2*t*(1-t)*c.x + z_square(t)*e.p.x;
		float y1 = z_square(1-t)*b.p.y + 2*t*(1-t)*c.y + z_square(t)*e.p.y;
		float w = b.w + (t* (e.w-b.w));
		z_fpoint pw = { {x1, y1}, w};
		z_fpoint_differential_add(ctx, a, pw);
	}
}

float z_linewidth(z_ipoint b, z_ipoint e, float bwidth, float step) {
	const float max_speed = 2.0f;
	float d = z_distance(b.p, e.p);
	float s = d / (e.t - b.t); s = s > max_speed ? max_speed : s;
	float w = (max_speed-s) / max_speed;
	float max_dif = d * step;
	if( w<0.05 ) w = 0.05;
	if( fabs( w-bwidth ) > max_dif ) {
		if( w > bwidth )
			w = bwidth + max_dif;
		else
			w = bwidth - max_dif;
	}
	// printf("d:%.4f, time_diff:%lld, speed:%.4f, width:%.4f\n", d, e.t-b.t, s, w);
	return w;
}


float z_insertPoint(fz_context *ctx, z_fpoint_array *arr, fz_point lastpoint, int64_t lastms,
	float lastwidth, fz_point point, int64_t ms) {
	if(!arr) return 0;
    int len = arr->len;
	fz_point zp = {point.x, point.y};
	if( 0==len ){
		z_fpoint p = {zp, 0.4};
        z_fpoint_add(ctx, arr, p);
		return p.w;
	}

	float step = arr->len > 4 ? 0.05: 0.2;
	z_ipoint bt = { {lastpoint.x,lastpoint.y}, lastms};
	z_ipoint et = { zp, ms};
	float w = (z_linewidth(bt, et, lastwidth, step) + lastwidth) / 2;
	z_fpoint_array *points = z_new_fpoint_array(ctx, 51, arr->maxwidth, arr->minwidth);
    z_fpoint tmppoint = arr->point[len-1];
	z_fpoint_add(ctx, points, tmppoint);

	if( 1==len ) {
		z_fpoint p = { {(bt.p.x + et.p.x + 1) / 2, (bt.p.y + et.p.y +1) / 2}, w};
        z_fpoint_differential_add(ctx, points, p);
		w = p.w;
	}
	else {
		z_fpoint bw = tmppoint;
		fz_point c =  {lastpoint.x,lastpoint.y};
		z_fpoint ew = {{(lastpoint.x + point.x)/2, (lastpoint.y + point.y)/2}, w};
		z_square_bezier(ctx, points, bw, c, ew);
	}
	
	// escape the first point
    int i;
	for(i=1; i<points->len; i++) {
        z_fpoint_add(ctx, arr, points->point[i]);
	}

	z_drop_fpoint_array(ctx, points);
	return w;
}

void z_insertLastPoint(fz_context *ctx, z_fpoint_array *arr, fz_point e) {
	if(!arr) return;
	long len= arr->len;
	if(len==0 ) return;
	z_fpoint_array *points = z_new_fpoint_array(ctx, 51, arr->maxwidth, arr->minwidth);
	z_fpoint zb = arr->point[len-1];
	z_fpoint_add(ctx, points, zb);
	
	z_fpoint ze = { {e.x, e.y}, 0.1};
    z_fpoint_differential_add(ctx, points, ze);
    int i;
	for(i=1; i<points->len; i++) {
        z_fpoint_add(ctx, arr, points->point[i]);
	}
	z_drop_fpoint_array(ctx, points);
}

z_list *z_list_new(fz_context *ctx, z_list_node_alloc_fun allocfun, z_list_node_drop_fun dropfun)
{
    z_list *l = NULL;
    fz_try(ctx) {
        l = fz_malloc_struct(ctx, z_list);
        l->alloc = allocfun;
        l->drop = dropfun;
        l->first = l->last = NULL;
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l;
}

void *z_list_append_new(fz_context *ctx, z_list *zlist) 
{
    z_list_node *node = NULL;
    void *data = NULL;

    if(!zlist->alloc || !zlist->drop) 
        return NULL;

    fz_try(ctx) {
        node = fz_malloc_struct(ctx, z_list_node);
        node->data = zlist->alloc(ctx); 
        node->n = NULL;
        node->p = NULL;
    }
    fz_always(ctx){}
    fz_catch(ctx) {
        if(node){
            if(node->data) zlist->drop(ctx, node->data);
            fz_free(ctx, node);
            node = NULL; 
        } 
    }

    if(node) {
        if(!zlist->first) {
            zlist->first = zlist->last = node;
        }
        else {
            node->n = NULL;
            node->p = zlist->last;
            zlist->last->n = node; 
            zlist->last = node;
        } 
        data = node->data;
    }

    return data;
}
void *z_list_remove_last(fz_context *ctx, z_list *zlist) 
{
    void *data = NULL;
    z_list_node *tmp = zlist->last;
    if(zlist->last) {
        tmp = zlist->last;
        if(zlist->last==zlist->first){
            zlist->last = zlist->first = NULL;
        }
        else {
            zlist->last = tmp->p;
            zlist->last->n = NULL;
        }
    }

    if(tmp) {
        data = tmp->data; 
        fz_free(ctx, tmp);
    }

    return data; 
}

void z_list_clear(fz_context *ctx, z_list *zlist) 
{
    fz_try(ctx) {
        while(zlist->first)
            zlist->drop(ctx, z_list_remove_last(ctx, zlist));
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
}

void z_list_free(fz_context *ctx, z_list *zlist) 
{
    fz_try(ctx) {
        z_list_clear(ctx, zlist);
        fz_free(ctx, zlist);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

}

/* digest must be 33 char size  */
void
z_text_md5(fz_context *ctx, const char* str, char *digest)
{
    int len = strlen(str);
    unsigned char d[16];
	fz_md5 state;
	fz_md5_init(&state);
	fz_md5_update(&state, (const unsigned char*)str, len);
	fz_md5_final(&state, d);

    int i;
    for(i=0; i<(int)sizeof(d); i++) {
        sprintf(digest, "%02x", d[i]);
        digest+=2;
    }
    *digest = '\0';
}
