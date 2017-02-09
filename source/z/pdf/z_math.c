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

/***************this algorathim not used!!!***********************
static float z_bezier_split_factor = 0.05;
void z_points_time_to_width(z_points_array *points) {
    if(!points || points->count<=1)  return;
	points->data[0].l = 0x20;
	float step = 0x40;
    for(int i=1; i<points->count-1; i++) {
        z_point *b = points->data + (i-1);
        z_point *e = points->data + i;
        if( i>4 ) {
            step = 0x20;
        }
		e->l = z_get_point_width(*b, *e, step);
    }
    points->data[0].l = 0x20;
}

// use point as control point
// b(t) = (1-t)^2*p0 + 2t(1-t)*p1 + t^2*p2
z_point_array* z_points_to_smoothpoints_0(z_point_array *points) {
    if( !points || points->count<1 ) {
        return NULL;
    }
    float bx, by, cx, cy, ex, ey;
    int count = points->count;
    z_point *ps = points->data;
    z_point_array *out_points = z_points_new(count* (1.0/z_bezier_split_factor) );
    if( !out_points ){ return NULL; }
	z_points_add(out_points, ps[0]);
    for( int i=1; i<count-1; i++ ) {
#if 0
		bx = (ps[i-1].x+ ps[i].x) /2;
		by = (ps[i-1].y+ ps[i].y) /2;
		bw = (ps[i-1].l+ ps[i].l) /2;
        ex = (ps[i].x + ps[i+1].x) / 2;   // start point
        ey = (ps[i].y + ps[i+1].y) / 2;
        ew = (ps[i].l + ps[i+1].l) / 2;
        cx = ps[i].x;                     // end point
        cy = ps[i].y;
		float x0 = bx, y0 = by;
        z_points_add_differentation(out_points, x0, y0, bw);
		for(float t=z_bezier_split_factor; t<=1.0; t+=z_bezier_split_factor) {
			float x1 = z_square(1-t)*bx + 2*t*(1-t)*cx + z_square(t)*ex;
			float y1 = z_square(1-t)*by + 2*t*(1-t)*cy + z_square(t)*ey;
			float w1 = bw + (t * (we - wb));
            z_points_add_differentation(out_points, x1, y1, w1);
		}
#else
		z_point b = {(ps[i-1].x+ ps[i].x) /2,(ps[i-1].y+ ps[i].y) /2,(ps[i-1].l+ ps[i].l) /2};
		z_point e = {(ps[i].x + ps[i+1].x)/2,(ps[i].x + ps[i+1].x)/2,(ps[i].l + ps[i+1].l)/2};
		z_point c = ps[i];
		z_quare_bezier(out_points, b, e, c);
#endif
    }
	z_points_add_differentation(out_points,
		points[count-1].x, points[count-1].y, points[count-1].l);
    return out_points; 
}
					 

// b(t) = p0*(1-t)^3 + 3p1*t*(1-t)^2 + 3*p2*t^2(1-t) + p3*t^3
static float p0(float v, float t){ return v * z_cubic(1-t); }
static float p1(float v, float t){ return 3 * v * t * z_square( 1-t ); }
static float p2(float v, float t){ return 3 * v * z_square(t) * (1-t); }
static float p3(float v, float t){ return v * z_cubic(t); }
static z_point z_bezier_split_point(z_point b,z_point e,z_point c1,z_point c2,
     float t){
	z_point point = {
		p0(b.x,t) + p1(c1.x,t) + p2(c2.x,t) + p3(e.x,t),
		p0(b.y,t) + p1(c1.y,t) + p2(c2.y,t) + p3(e.y,t),
		b.w + (e.w-b.w)*t
	};
	return point;
} 
void z_bezier_points(z_points* out_points, z_point b,
        z_point e, z_point b_c, z_point e_c) {
    float f = z_bezier_split_factor;
    int count = 1.0 / f;
    z_point_array *out_points = z_points_new(count);

    int dif_w = b.l - e.l;
    for(float t=f; t<=1.0; t+=f) {
        z_point curpoint = z_point_get_split(b, e, b_c, e_c, t);
        curpoint.l = dif_w * f;
        z_points_add(out_points, curpoint); 
    }
} 
z_point_array* z_points_to_smoothpoints_1(z_point_array *points) {
	z_point b, e, n, b_c, e_c, c;
    z_point_array *out_points = NULL;
    int count = points->count;
    if( count<=2 ) 
        return NULL;

    float w = 1.0f;
    z_point *ps = points->data;

    out_points = z_points_new(count * (1.0 / z_bezier_split_factor) );
    
    if( !out_points ){ return NULL; } 

    z_points_add_xyl(out_points, ps[0].x, ps[0].y, w);
    c = ps[0];
    for(int i=0; i<(count-1); i++) {
		b = ps[i+0]; 
        e = ps[i+1];
        while( i<(count-1) && z_point_pos_equals(&b, &e) ) {
            i ++; 
            e = ps[i+1];
        }

        if( i >= (count-2) ) n = e;
        else{
            n = ps[i+2];
            while( (i<(count-3)) && z_point_pos_equals(&e, &n) ) {
                i ++;
                n = ps[i+2];
            }
        } 
		b_c = c;
		e_c = z_bezier_control_point(b, e, n, &c, f);
        z_bezier_points(out_points, b, e, b_c, e_c);
	} 
    return out_points;
}

z_point z_bezier_control_point(z_point b,z_point e,z_point n,z_point *c,float f) {
    if(f>0.5) f= 0.5;
    if(f<0.2) f= 0.2;
	// 计算AB,BC的中点坐标,连接后记为线段M
	float xm1 = (e.x + b.x)/2;
	float ym1 = (e.y + b.y)/2;
	float xm2 = (n.x + e.x)/2;
	float ym2 = (n.y + e.y)/2;
	// 计算线段AB,BC的长度,分别记为l1,l2
	float l1 = z_distance(b, e);
	float l2 = z_distance(e, n);
	float k = l1/(l2+l1);
	// 把线段M,以l1:l2的比例分割,分割点记为T
	float xt = xm1 + (xm2-xm1) * k;
	float yt = ym1 + (ym2-ym1) * k;
	// 将线段M平行移动到顶点B,将线段上的分割点T对其到B,
	// 线段的两个端点就是控制点!!!!,加上平滑因子f,计算控制点!!!
	float xc1 = e.x - (xt - xm1) * f;
	float yc1 = e.y - (yt - ym1) * f;
	float xc2 = e.x + (xm2 - xt) * f;
	float yc2 = e.y + (ym2 - yt) * f;
	z_point point = {xc1, yc1, 0};
	if( c ) {
		c->x = xc2;
        c->y = yc2;
	}
	return point;
}
//////////////////////////////////////*/
