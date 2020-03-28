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
type Page struct {
	page * C.pdf_page
}