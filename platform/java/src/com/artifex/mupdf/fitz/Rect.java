package com.artifex.mupdf.fitz;

public class Rect
{
	public float x0;
	public float y0;
	public float x1;
	public float y1;

	public Rect()
	{
		x0 = y0 = x1 = y1 = 0;
	}

	public Rect(float x0, float y0, float x1, float y1)
	{
		this.x0 = x0;
		this.y0 = y0;
		this.x1 = x1;
		this.y1 = y1;
	}

	public Rect(Rect r)
	{
		this(r.x0, r.y0, r.x1, r.y1);
	}

	public float width() {
		return x1 - x0;
	}

	public float height() {
		return y1 - y0;
	}

	public Rect(RectI r)
	{
		this(r.x0, r.y0, r.x1, r.y1);
	}

	public String toString()
	{
		return "[" + x0 + " " + y0 + " " + x1 + " " + y1 + "]";
	}

	public void move(float dx, float dy)
	{
		float x = x0 + dx;
		float y = y0 + dy;
		setLocation(x, y);
	}

	public Rect transform(Matrix tm)
	{
		float ax0 = x0 * tm.a;
		float ax1 = x1 * tm.a;

		if (ax0 > ax1)
		{
			float t = ax0;
			ax0 = ax1;
			ax1 = t;
		}

		float cy0 = y0 * tm.c;
		float cy1 = y1 * tm.c;

		if (cy0 > cy1)
		{
			float t = cy0;
			cy0 = cy1;
			cy1 = t;
		}
		ax0 += cy0 + tm.e;
		ax1 += cy1 + tm.e;

		float bx0 = x0 * tm.b;
		float bx1 = x1 * tm.b;

		if (bx0 > bx1)
		{
			float t = bx0;
			bx0 = bx1;
			bx1 = t;
		}

		float dy0 = y0 * tm.d;
		float dy1 = y1 * tm.d;

		if (dy0 > dy1)
		{
			float t = dy0;
			dy0 = dy1;
			dy1 = t;
		}
		bx0 += dy0 + tm.f;
		bx1 += dy1 + tm.f;

		x0 = ax0;
		x1 = ax1;
		y0 = bx0;
		y1 = bx1;

		return this;
	}

	public boolean contains(float x, float y)
	{
		if (isEmpty())
			return false;

		return (x >= x0 && x < x1 && y >= y0 && y < y1);
	}

	public boolean contains(Rect r)
	{
		if (isEmpty() || r.isEmpty())
			return false;

		return (r.x0 >= x0 && r.x1 <= x1 && r.y0 >= y0 && r.y1 <= y1);
	}

	public boolean isEmpty()
	{
		return (x0 == x1 || y0 == y1);
	}

	public void includeTo(int x, int y) {
		if(x0>x) x0 = x;
		else if(x1<x) x1 = x;
		if(y0>y) y0 = y;
		else if(y1<y) y1 = y;
	}

	public void inflate(float dx, float dy) {
		if(width() < dx*2) x0 = x1 = (x0+x1)/2;
		else {
			x0 += dx; x1 -= dx;
		}
		if(height()< dy*2) y0 = y1 = (y0+y1)/2;
		else {
			y0 += dy; y1 -= dy;
		}

	}

	public RectI round()
	{
		return new RectI( (int)(x0+0.5), (int)(y0+0.5),
				(int)(x1+0.5), (int)(y1+0.5) );
	}

	public void set(float x, float y, float w, float h)
	{
		setLocation(x, y);
		setSize(w, h);
	}

	public void setLocation(float x, float y)
	{
		float w = width();
		float h = height();
		x0 = x; y0 = y;
		setSize(w, h);
	}

	public void setSize(float w, float h)
	{
		x1 = x0 + w; y1 = y0 + h;
	}

	public void union(Rect r)
	{
		if (isEmpty())
		{
			x0 = r.x0;
			y0 = r.y0;
			x1 = r.x1;
			y1 = r.y1;
		}
		else
		{
			if (r.x0 < x0)
				x0 = r.x0;
			if (r.y0 < y0)
				y0 = r.y0;
			if (r.x1 > x1)
				x1 = r.x1;
			if (r.y1 > y1)
				y1 = r.y1;
		}
	}
}
