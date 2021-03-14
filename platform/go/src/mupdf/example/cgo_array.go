package main

/*
#include <stdio.h>

typedef struct {
  int foo;
} CStruct;

void test(CStruct* p) {
  printf("%d\n", p->foo);
}

const char *myStringArray[] = {
  "NAME_OF_FIRST_THING",
  "NAME_OF_SECOND_THING",
  "NAME_OF_THIRD_THING"};
*/
import "C"
import (
	"fmt"
	"unsafe"
)
func cgo_array() {
	var x C.CStruct
	x.foo = 42
	C.test(&x)

	const arraySize = 3
	cStrings := (*[arraySize]*C.char)(unsafe.Pointer(&C.myStringArray))[:arraySize:arraySize]

	for _, cString := range cStrings {
		fmt.Println(C.GoString(cString))
	}
}
