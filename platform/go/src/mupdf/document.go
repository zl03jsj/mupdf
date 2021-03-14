package mupdf

//#cgo CFLAGS: -I../../../../include
//#cgo LDFLAGS: -L../../../../build/release -lmupdf -lmupdfthird
//#include "mupdf/pdf.h"
//#include "mupdf/fitz.h"
//#include "mupdf/pdf/object.h"
/*
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"sync"
	"unsafe"
)

// Document ...
type Document struct {
	ctx *context
	doc *C.fz_document

	page_count int64

	is_page_cached bool

	mutx sync.Mutex
}

func (self *Document) Drop() {
	if self.doc != nil {
		C.fz_drop_document(self.ctx.ctx, self.doc)
	}
	self.ctx.release()
	self.ctx = nil
}

func (self *Document) PageCount() int64 {
	if self.is_page_cached {
		return self.page_count
	}
	pdf_doc := C.pdf_specifics(self.ctx.ctx, self.doc)
	self.page_count = int64(C.pdf_count_pages(self.ctx.ctx, pdf_doc))
	self.is_page_cached = true
	return self.page_count
}

func (self *Document) Page(pageno int) (*Page, error) {
	var page = &Page{}
	var err error
	if page.page, err = C.fz_load_page(self.ctx.ctx, self.doc, C.int(pageno)); err != nil {
		return nil, err
	}
	return page, nil
}

func (self *Document) OpenDocument(file string) (*Document, error) {
	var err error
	cfile := C.CString(file)
	defer C.free(unsafe.Pointer(cfile))
	if self.doc, err = C.fz_open_document(self.ctx.ctx, cfile); err != nil {
		return nil, err
	}
	return self, nil
}

func (self *Document) DrawPage(page *Page) {
	page.Size()

}

func OpenDocument(file string) (*Document, error) {
	return (&Document{ctx: newCtx()}).OpenDocument(file)
}
