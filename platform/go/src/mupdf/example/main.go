package main

/*
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
typedef struct{
	int x, y;
} Point;

Point* NewPoint(int i) {
	if (i<0) {
		return;
	}
	Point *p = malloc(sizeof(Point));
	p->x = 10;
	p->y = 10;
	return p;
};
*/
import "C"
import "fmt"

func main() {
	p := C.NewPoint(-1)
	fmt.Println(p)
}
