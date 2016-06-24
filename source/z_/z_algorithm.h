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
#include "../../include/mupdf/fitz.h"
typedef struct {
    float x;
    float y;
    int64_t l; // w = l*maxwidth/256, time unit is ms;
} z_point, z_point_time, z_point_with;

typedef struct {
    z_point *data;
    int count;
    int cap;
    int ref;
} z_point_array;

void z_points_addref(z_point_array *points);
void z_points_release(z_point_array *points);

z_point_array* z_points_new(int initsize);
int z_points_resize(z_point_array *points, int newsize);
void z_points_add(z_point_array *points, float x, float y, float w);
void z_points_add(z_point_array *points, z_point point);
void z_points_time_to_width(z_points_time *points);

z_point_array* z_points_to_smoothpoints_0(z_point_array *points);
z_point_array* z_points_to_smoothpoints_1(z_point_array *points);

void z_bezier_points(z_points* outp_points, z_point b, z_point e, z_point b_c, z_point e_c);
// f smooth factor [0.2-0.5];
z_point z_bezier_control_point(z_point b,z_point e,z_point n,z_point *c,float f);
