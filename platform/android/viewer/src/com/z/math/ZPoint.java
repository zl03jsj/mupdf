package com.z.math;

import com.artifex.mupdf.fitz.Point;

public class ZPoint {
    public Point p = new Point(0.0f, 0.0f);
    public float w = 1.0f;

    public ZPoint(float x, float y, float width) {
        this.p.x = x;
        this.p.y = y;
        this.w = width;
    }

    public ZPoint(Point point, float width) {
        this.p.x = point.x;
        this.p.y = point.y;
        this.w = width;
    }

    public float x() {
        return p.x;
    }

    public float y() {
        return p.y;
    }

    @Override
    public String toString() {
        return "ZPoint{" + "p=" + p + ", w=" + w + '}';
    }

    synchronized ZPoint copy() {
        return new ZPoint(p, w);
    }
}
