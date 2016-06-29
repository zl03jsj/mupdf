/*
 * =====================================================================================
 *       Filename:  z_algorithm.h
 *    Description:  
 *        Version:  1.0
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
#ifndef _Z_ALGORITHM_H_
#define _Z_ALGORITHM_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "mupdf/fitz.h"
typedef struct {
    float x;
    float y;
} z_point;

typedef struct{
	z_point p;
	float w;
} z_point_width;

typedef struct{
	z_point p;
	int64_t t;
} z_point_time;

typedef struct {
	z_point_width *data;
	int count;
    int cap;
    int ref;
} z_points, z_points_width_array;

int z_points_addref (z_points *points);
int z_points_release(z_points *points);
z_points* z_points_new(int initsize);
int z_points_increasesize(z_points *points, int count);
float z_get_width(z_point_time b, z_point_time e, float bw, float step);
int z_points_add_xyw(z_points *points, float x, float y, float w);
int z_points_add(z_points *points, z_point_width p);
int z_points_add_differentation(z_points *points, z_point_width p);
void z_quare_bezier(z_points *out, z_point_width b, z_point c, z_point_width e);

#ifdef __cplusplus
}
#endif

#endif
