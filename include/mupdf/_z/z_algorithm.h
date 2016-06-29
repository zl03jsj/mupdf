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

void z_points_addref (z_points *points);
void z_points_release(z_points *points);
z_points* z_points_new(int initsize);
int z_points_increasesize(z_points *points, int count);
float z_get_width(z_point_time b, z_point_time e, float bw, float step);

/*
int z_points_resize(z_point_array *points, int newsize);
void z_points_add_xyl(z_point_array *points, float x, float y, float w);
void z_points_add(z_point_array *points, z_point point);
void z_points_time_to_width(z_point_time *points);
void z_get_point_width(z_point b, z_point e, int step);

z_point_array* z_points_to_smoothpoints_0(z_point_array *points);
z_point_array* z_points_to_smoothpoints_1(z_point_array *points);

void z_bezier_points(z_point_array* outp_points, z_point b, z_point e, z_point b_c, z_point e_c);
// f smooth factor [0.2-0.5];
z_point z_bezier_control_point(z_point b,z_point e,z_point n,z_point *c,float f);
z_point z_point_middle(z_point b, z_point e);
z_point_array *z_quare_bezier(z_point b, z_point c, z_point e);

 */
