package com.artifex.mupdf.z.algorithm;

import java.util.ArrayList;
import java.lang.Math;
import com.artifex.mupdf.fitz.Point;

public class Z_Bezier{
    private Point m_lastpoint;
    private float m_lastwidth;
    private long m_lastms;
    private ArrayList<Z_Point> m_points;
    private int m_lastsize;
    private float m_maxwidth;
    // private boolean m_finished;
    // private float m_startWidth;
    public Z_Bezier(float maxwidth) {
        m_lastpoint = null;
        m_lastms = 0;
        m_points = new ArrayList<>();
        m_lastsize = 0;
        m_maxwidth = maxwidth < 2 ? 2 : maxwidth;
        // m_finished = false;
        // m_startWidth = (float)0.5;
    }
    /**
     * addPoint 添加一个点用于绘制bezier曲线!!!
     * @param p 新增加的点
     * @return  返回新产生的点的起始index,如果返回值小于0,则这个点被忽略
     */
    public int add(Point p){
        if( m_points.isEmpty() ) {
            return addFirst(p);
        }
        return addMiddle(p);
    }

    public int addLast(Point p){
        m_lastsize = m_points.size();
        if( 0==m_lastsize ) return -1;
        Z_Point lp = new Z_Point(p, 0.1f);
        addDifferentationToArray(m_points, lp);
        // m_finished = true;
        return m_lastsize;
    }

    private int addFirst(Point p){
        if(m_lastpoint!=null)
            return m_lastsize;
        m_lastpoint = p;
        m_lastwidth = 0.5f;
        m_lastms = new java.util.Date().getTime();
        m_points.add( new Z_Point(p, m_lastwidth) );
        return m_lastsize;
    }

    private int addMiddle(Point p){
        m_lastsize = m_points.size();
        if( 0==m_lastsize ){
            return addFirst(p);
        }

        float step = m_lastsize > 4 ? 0.01f : 0.4f;
        long ms = new java.util.Date().getTime();
		if( ms-m_lastms < 35 ||
		    distance(p, m_lastpoint) < 4 ){
			return -1;
		}
        float w = width(m_lastpoint, p, ms - m_lastms, m_lastwidth, step);
        if( w > m_maxwidth ) { w = m_maxwidth; }

        Z_Point e = new Z_Point (
            (m_lastpoint.x + p.x) / 2,
            (m_lastpoint.y + p.y) / 2,
            (w + m_lastwidth) / 2 );
		
        if( 1==m_lastsize ){
            addDifferentationToArray(m_points, e);
        }
        else {
            Z_Point b = m_points.get(m_lastsize-1).copy();
            Point   c = new Point(m_lastpoint);
            squareBezier(m_points, b, c, e);
        }
        m_lastpoint.x = p.x; m_lastpoint.y = p.y;
        m_lastwidth = w;
        m_lastms = ms;
        return m_lastsize;
    }

    static float width(Point b,Point e, long timediff, float lastWidth, float step){
        final float max_speed = 2.0f;
        float d = distance(b, e);
        float s = d / timediff;
        s = s > max_speed ? max_speed : s;
        float w = (max_speed - s) / max_speed;
        float max_diff = d * step;
        w = Math.max(w, 0.05f);

        if( Math.abs(w-lastWidth) > max_diff) {
            if( w>lastWidth ) w = lastWidth + max_diff;
            else
                w = lastWidth - max_diff;
        }
        return w;
    }
    static float distance(Point b, Point e){
        float dx = e.x - b.x;
        float dy = e.y - b.y;
        return (float)Math.hypot(dx, dy);
    }
    static void addDifferentationToArray(ArrayList<Z_Point> arr, Z_Point p){
        if(null==arr) return;
        if( arr.isEmpty() ) {
            arr.add(p); return;
        }

        final float max_diff = (float)0.05;
        Z_Point last = arr.get( arr.size()-1 ).copy();

        int n = (int)(Math.abs(p.w-last.w) / max_diff) + 1;
        float step_x = stepDiff(p.p.x, last.p.x, n);
        float step_y = stepDiff(p.p.y, last.p.y, n);
        float step_w = stepDiff(p.w,   last.w,   n);

        for( int i=1; i<(n-1); i++) {
            last.p.x += step_x;
            last.p.y += step_y;
            last.w   += step_w;
            arr.add(last.copy());
        }
    }
    static void squareBezier(ArrayList<Z_Point> arr,
        Z_Point b, Point c, Z_Point e) {
        if( null==arr ) return;
        final float f = (float) 0.05;
        for( float t=0; t<1.0; t+=f) {
            float x = stepBezier(b.p.x, c.x, e.p.x, t);
            float y = stepBezier(b.p.y, c.y, e.p.y, t);
            float w  = b.w + (t * ( e.w-b.w ) );
            addDifferentationToArray(arr, new Z_Point(x,y,w));
        }
    }
    static float x2(float x){
        return x*x;
    }
    private static float stepBezier(float b, float c, float e, float t){
        return (x2(1-t) * b) + (2 * t * (1-t) * c) + (x2(t) * e);
    }
    private static float stepDiff(float b, float e, int n){
        return (b - e) / n;
    }
}

