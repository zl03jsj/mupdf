#!/bin/bash - 
set -o nounset                              # Treat unset variables as an error

echo compile mupdf lib!!
make -C ../../../ build=debug  HAVE_OPENSSL=yes
echo mupdf lib done!!
echo compile test!!!

clang -I../../../include/ -I~/code/openssl_mac/include -L../../../build/debug -L../../../thirdparty/openssl/macos -lcrypto -lssl -lmupdf -lmupdfthird ./samples/pdf_samples.c -g -o "./pdftest"
echo done!!!
