echo compile mupdf lib!!
make -C ../../../ build=debug  HAVE_OPENSSL=yes
echo mupdf lib done!!!

echo compile test!!!
clang -I../../../include/ -I~/opnessl/include -L../../../build/debug -L/Users/zl03jsj/Documents/mupdf/thirdparty/openssl/macos -lcrypto -lssl -lmupdf -lmupdfthird ./Test/Z_pdftest.c -g -o pdftest
echo done!!!
