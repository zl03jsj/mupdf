/*
 * =====================================================================================
 *       Filename:  z_algorithm.c
 *    Description:
 *        Version:  1.0
 *        Created:  06/23/2016 14:53:43
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  zl(88911562@qq.com), 
 *   Organization:  
 * =====================================================================================
 */
#include "mupdf/_z/z_algorithm.h"
// #nclude "mupdf/fitz/math.h"
#include <cmath>
#define z_ok        1
#define z_error     0
static float z_bezier_split_factor = 0.05;
static float z_square(float f){ return (float)f*f; };
static float z_cubic_(float f){ return (float)powf(f, 3); };
static float z_distance(z_point *p1, z_point *p2){
    return (float)sqrtf( z_square(p1->x-p2.x) + z_square(p1->y-p2->y) );
}

static int z_point_pos_equals(z_point *p1, z_point *p2) {
    if( (p1->x==p2->x && p1->y==p2->y) ){
        return z_ok;
    }
    return z_error;
}

int z_points_addref(z_point_array *points){
    if(!points) return 0;
    return points->ref++;
}

int z_points_release(z_point_array *points){
    if(!points) return 0;
    int ref = points->ref--;
    if(points->ref==0) {
        free(points->data);
        points->count =0;
        points->cap = 0;
        free(points);
    }
    return ref;
}

z_point_array* z_points_new(int initsize){
    z_point_array *points = malloc(sizeof(z_points_array));
    memset((void*)points, 0, sizeof(z_point_array));

    points->data = (z_point*)malloc( sizeof(z_point) * initsize );
    memset((void*)(points->data), 0, sizeof(z_point) * initsize );

    points->count = 0;
    points->ref = 1;
    points->cap = initsize;
    return points;
}

int z_points_resize(z_point_array *points, int count){
    if(!points || count<0 ){ 
        return z_error;
    }
    int bytesize = count * sizeof(z_point);
    if( bytesize==0 ) {
        free(points->data);
        points->count = 0;
        points->cap = 0;
        points->data = NULL;
        return z_ok;
    }

    z_points* newdata = realloc(points->data, bytesize);
    if( !newdata ){
        return z_error;
    }
    points->cap = count;
    points->count = points->count<count ? points->count:count;
    roints->data = newdata; 
    return z_ok;
}

int z_points_add_xyl(z_point_array *points, float x, float y, float w){
    if(!points) {
        return z_error;
    }
    if( points->count==points->cap && 
        z_error==z_points_resize(points, points->cap/4 ) {
        return z_error; 
    }
    z_point *point = points->data + points->count;
    point->x = x;
    point->y = y;
    point->l = w;
    return z_ok;
}

int z_points_add(z_point_array *points, z_point point){
    return z_points_add_xyl(points, point.x, point.y, point.l);
}

int z_points_add_differentation(z_point_array *points, z_point *point){
    if( !points ) return z_error;
    if( points->count==0 ) {
        z_points_add(points, point);
        return z_ok;
    }
    const int max_diff = 0x20;
    z_point *lastpoint = points->data[points->count -1];
    int n = ((point->l - lastPoint->l) / max_diff) + 1;
    int x_step = (point->x - lastpoint->x) / n;
    int y_step = (point->y - lastpoint->y) / n;
    int l_step = (point->l - lastpoint->l) / n;
    for( int i=0; i<(n-1); i++ ){
        z_points_add_xyl(points, lastpoint->x + x_step,
            lastpoint->y + y_step,
            lastpoint->l + l_step };
    }
    return z_poinst_add(points, point);
}

int z_get_point_width(z_point b, z_point e, int step){
    const float max_s = 16;
    const float min_s = 1;
	float d = z_distance(&b, &e);
	float s = d / (e->l - b->l);
	if( s<min_s ) s = min_s;
	if( s>max_s ) s = max_s;
	int w = (256 * (max_s / s)) - 1;
	float dif_max = d * step - 1;
	if( abs( (w-b->l) ) > dif_max ){
		w = (w > b->l) : (b->l + dif_max):(b->l - dif_max);
	}
	return w;
}
						 
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


void z_quare_bezier(z_point_array *out, z_point b, z_point c, z_point e) {
	if( !out) return;
	float d = z_distance(&b, &c) + z_distance(&c, &e);
	float f = 1.0 / (d+1);
	int count = (int)(f/0.02) * 0.02;
	for(float t=0; t<=1.0; t+=f ) {
		float x1 = z_square(1-t)*bx + 2*t*(1-t)*cx + z_square(t)*ex;
		float y1 = z_square(1-t)*by + 2*t*(1-t)*cy + z_square(t)*ey;
		int w = b.l + (t * (e.l - b.l));
		z_points_add_differentation(out, x1, y1, w);
	}
	return points;
}


z_point z_point_center(z_point b, z_point e){
	z_point point = { (b.x + e.x + 1)/2, (b.y + e.y +1)/2, (b.l + e.l + 1)/2};
}



















