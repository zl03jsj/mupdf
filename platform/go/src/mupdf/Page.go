package mupdf

//#cgo CFLAGS: -I../../../../include
//#cgo LDFLAGS: -L../../../../build/release -lmupdf -lmupdfthird
//#include "mupdf/pdf.h"
//#include "mupdf/fitz.h"
//#include "mupdf/fitz/math.h"
/*
#include <stdio.h>
#include <stdlib.h>

struct Size {
	float width;
	float height;
};

static fz_pixmap *createPixMap(fz_context *ctx, struct Size size)
{
	fz_pixmap *pix = NULL;
	fz_try(ctx) {
		pix = fz_new_pixmap(ctx, fz_device_rgb(ctx), size.width, size.height, 1);
	}
	fz_catch(ctx) {
		printf("Failed to create pixmap\n");
	}
	return pix;
}
*/
import "C"

type Page struct {
	ctx  *context
	page *C.fz_page
}

type PageBox struct {
	X0, X1, Y0, Y1 float64
}

type PageSize struct {
	With   float64
	Height float64
}

func (self *Page) Drop() {
	if self.page != nil {

		C.fz_drop_page(self.ctx.ctx, self.page)
		self.page = nil
	}
}

func (self *Page) Size() (*PageSize, error) {
	var rect C.fz_rect
	C.fz_bound_page(self.ctx.ctx, self.page, &rect)
	return &PageSize{With: float64(rect.x1 - rect.x0), Height: float64(rect.y1 - rect.y0)}, nil
}

func (self *Page) PageBox() (*PageBox, error) {
	var rect C.fz_rect
	C.fz_bound_page(self.ctx.ctx, self.page, &rect)
	return &PageBox{X0: float64(rect.x0),
		X1: float64(rect.x1),
		Y0: float64(rect.y0),
		Y1: float64(rect.y1)}, nil
}

func (self *Page) Render
