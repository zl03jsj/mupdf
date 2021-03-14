package mupdf

//#cgo CFLAGS: -I../../../../include
//#cgo LDFLAGS: -L../../../../build/release -lmupdf -lmupdfthird
//#include "mupdf/pdf.h"
//#include "mupdf/fitz.h"
/*
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"unsafe"
)

type context struct {
	fz_version *C.char
	ctx        *C.fz_context
}

func (self *context) release() {
	if self.fz_version != nil {
		C.free(unsafe.Pointer(self.fz_version))
		self.fz_version = nil
	}

	if self.ctx != nil {
		C.fz_drop_context(self.ctx)
		self.ctx = nil
	}
}

// newCtx ... export function should have comment or un-exporteable
func newCtx() *context {
	ctx := &context{}
	ctx.fz_version = C.CString(C.FZ_VERSION)
	ctx.ctx = C.fz_new_context_imp(nil, nil, C.ulong(0), ctx.fz_version)

	return ctx
}
