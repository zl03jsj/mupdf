package com.artifex.mupdf.z_.algorithm;
import com.artifex.mupdf.fitz.Point;

public class Z_Point {
    public Point p;
    public float w;
    public Z_Point(float x, float y, float _w){
        p.x = x; p.y = y; w = _w;
    }
    public Z_Point(Point _p, float _w) {
        p.x = _p.x; p.y = _p.y; w = _w;
    }
    Z_Point copy(){
        return new Z_Point(p, w);
    }
}
