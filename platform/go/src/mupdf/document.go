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
	doc *C.pdf_document

	page_count int64

	is_page_cached bool

	mutx sync.Mutex
}

func (self *Document) Release() {
	if self.ctx != nil {
		self.ctx.drop()
		self.ctx = nil
	}
}

func (self *Document) PageCount() int64 {
	if self.is_page_cached {
		return self.page_count
	}
	return self.page_count
}

func (self *Document) Page(pageno int) (*Page, error) {
	var page = &Page{ }
	var err error
	if page.page, err = C.pdf_load_page(self.ctx.ctx ,self.doc, C.int(pageno)); err!=nil {
		return nil, err
	}
	return page, nil
}

func (self *Document) OpenDocument(file string) (*Document, error) {
	var err error
	cfile := C.CString(file)
	defer C.free(unsafe.Pointer(cfile))
	if self.doc = C.pdf_open_document(self.ctx.ctx, cfile); err!=nil {
		return nil, err
	}
	return self, nil
}

func OpenDocument(file string) (*Document, error) {
	return (&Document{ctx: newCtx()}).OpenDocument(file)
}
