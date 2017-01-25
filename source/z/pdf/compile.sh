cd ../../
echo compile mupdf lib!!
make debug HAVE_LIBCRYPTO=yes
echo mupdf lib done!!!

echo compile test!!!
cd  ./source/z
clang -I../../include/ -I~/opnessl/include -L../../build/debug -L/Users/zl03jsj/Documents/mupdf/thirdparty/openssl/macos -lcrypto -lssl -lmupdf -lmupdfthird ./Test/Z_pdftest.c -g -o pdftest
echo done!!!
