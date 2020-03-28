package mupdf

import (
	"testing"
)

func Test_newCtx(t *testing.T) {
	ctx := newCtx()
	ctx.release()
}
