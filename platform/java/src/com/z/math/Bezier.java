package com.z.math;

import com.artifex.mupdf.fitz.Point;

import java.util.ArrayList;

public class Bezier {
    private Point mLastPoint;
    private float mLastWidth;
    private long mLastMS;
    private ArrayList<ZPoint> zPoints = new ArrayList<>();
    private int mLastSize;

    public Bezier() {
    }

    /**
     * addPoint 添加一个点用于绘制bezier曲线!!!
     *
     * @param p 新增加的点
     * @return 返回新产生的点的起始index, 如果返回值小于0, 则这个点被忽略
     */
    public int add(Point p) {
        if (zPoints.isEmpty()) {
            return addFirst(p);
        }
        return addMiddle(p);
    }

public int add(float x, float y){
        Point point = new Point(x, y);
        return add(point);
    }

    public int addLast(float x, float y) {
        Point point = new Point(x, y);
        return addLast(point);
    }

    public int addLast(Point p) {
        mLastSize = zPoints.size();
        if (0 == mLastSize) return -1;
        ZPoint lp = new ZPoint(p, 0.1f);
        diffToArray(zPoints, lp);
        return mLastSize;
    }

    public int addFirst(Point p) {
        if (mLastPoint != null)
            return mLastSize;
        mLastPoint = p;
        mLastWidth = 0.5f;
        mLastMS = new java.util.Date().getTime();
        //System.out.println("hand_draw_Test",String.format("point(%7.2f,%7.2f, 0)", p.x, p.y));
        zPoints.add(new ZPoint(p, mLastWidth));
        return mLastSize;
    }

    public int addMiddle(Point p) {
        mLastSize = zPoints.size();
        if (0 == mLastSize) {
            return addFirst(p);
        }

        float step = mLastSize > 4 ? 0.1f : 0.05f;
        long ms = new java.util.Date().getTime();

        float d = distance(p, mLastPoint);
        if (ms - mLastMS < 40 || d < 8.0f) {
            //System.out.println("ignore",String.format("point(%7.2f,%7.2f,%4d) no---", p.x, p.y, ms-mLastMS));
            return -1;
        }
        //System.out.println("added",String.format("point(%7.2f,%7.2f,%4d) yes++++", p.x, p.y, ms-mLastMS));

        float w = width(mLastPoint, p, ms - mLastMS, mLastWidth, step);
        ZPoint e = new ZPoint(
                (mLastPoint.x + p.x) / 2,
                (mLastPoint.y + p.y) / 2,
                (w + mLastWidth) / 2);

        if (1 == mLastSize) {
            diffToArray(zPoints, e);
        } else {
            ZPoint b = zPoints.get(mLastSize - 1).copy();
            Point c = new Point(mLastPoint);
            squareBezier(zPoints, b, c, e);
        }
        mLastPoint.x = p.x;
        mLastPoint.y = p.y;
        mLastWidth = w;
        mLastMS = ms;

        return mLastSize;
    }

    public ZPoint getLastPoint() {
        return zPoints.get(zPoints.size() - 1);
    }

    public ArrayList<ZPoint> getZPoints() {
        return zPoints;
    }

    private float width(Point b, Point e, long timediff, float lastWidth, float step) {
        final float max_speed = 2.0f;
        float d = distance(b, e);
        float s = d / timediff;
        s = s > max_speed ? max_speed : s;
        float w = (max_speed - s) / max_speed;
        float max_diff = d * step;
        w = Math.max(w, 0.04f);

        if (Math.abs(w - lastWidth) > max_diff) {
            if (w > lastWidth) w = lastWidth + max_diff;
            else
                w = lastWidth - max_diff;
        }
        return w;
    }

    private float distance(Point b, Point e) {
        float dx = e.x - b.x;
        float dy = e.y - b.y;
        return (float) Math.hypot(dx, dy);
    }

    private void diffToArray(ArrayList<ZPoint> arr, ZPoint p) {
        if (null == arr) return;
        if (arr.isEmpty()) {
            arr.add(p);
            return;
        }

        final float max_diff = 0.05f;
        ZPoint last = arr.get(arr.size() - 1).copy();

        int n = (int) (Math.abs(p.w - last.w) / max_diff) + 1;
        float step_x = stepDiff(p.p.x, last.p.x, n);
        float step_y = stepDiff(p.p.y, last.p.y, n);
        float step_w = stepDiff(p.w, last.w, n);

        for (int i = 0; i < n; i++) {
            last.p.x += step_x;
            last.p.y += step_y;
            last.w += step_w;
            arr.add(last.copy());
        }
    }

    private void squareBezier(ArrayList<ZPoint> arr, ZPoint b, Point c, ZPoint e) {
        if (null == arr) return;
        final float f = 0.1f;
        for (float t = 0; t < 1.0; t += f) {
            float x = stepBezier(b.p.x, c.x, e.p.x, t);
            float y = stepBezier(b.p.y, c.y, e.p.y, t);
            float w = b.w + (t * (e.w - b.w));
            diffToArray(arr, new ZPoint(x, y, w));
        }
    }

    private float x2(float x) {
        return x * x;
    }

    private float stepBezier(float b, float c, float e, float t) {
        return (x2(1 - t) * b) + (2 * t * (1 - t) * c) + (x2(t) * e);
    }

    private float stepDiff(float b, float e, int n) {
        return (b - e) / n;
    }
}
