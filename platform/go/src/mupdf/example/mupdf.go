package main

//#cgo CFLAGS: -I../../../../../include
//#cgo LDFLAGS: -L../../../../../build/release -lmupdf -lmupdfthird
//#include "mupdf/pdf.h"
//#include "mupdf/fitz.h"
/*
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"fmt"
	"unsafe"
)

var pdffile = "/Users/zl/workspace/c++/mupdf/source/z/pdf/samples/pdffile/test.pdf"

func main() {
	var x *C.char
	x = C.CString("hello")
	C.free(unsafe.Pointer(x))

	fz_version := C.CString(C.FZ_VERSION)
	defer C.free(unsafe.Pointer(fz_version))

	file := C.CString(pdffile)
	defer C.free(unsafe.Pointer(file))

	ctx := C.fz_new_context_imp(nil, nil, C.ulong(0), fz_version)
	var err error
	C.fz_register_document_handlers(ctx)
	var document *C.fz_document
	if document, err = C.fz_open_document(ctx, file); err != nil {
		panic(err)
	}

	var page *C.fz_page
	if page, err = C.fz_load_page(ctx, document, 0); err != nil {
		panic(err)
	}

	var bounds C.fz_rect
	C.fz_bound_page(ctx, page, &bounds)
	fmt.Println(bounds)

	// var document *C.pdf_document
	// var err error
	// document, err = C.pdf_open_document(ctx, file)
	// if err!=nil {
	// 	fmt.Printf("%s\n", err.Error())
	// 	return
	// }
	//
	// defer C.fz_drop_context(ctx)
	// defer C.pdf_drop_document(ctx, document)
	//
	// fmt.Printf("call fz_drop_context ok\n")
}
