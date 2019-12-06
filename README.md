about:
extensions of mupdf.              by 88911562@qq.com

extension functions:
1)add pdf signature with image appearance.
2)add pdf signature with path appearance.
3)insert image to pdf.
4)add hand draw annotation to pdf.
5)annotation can added with password, and check password on deleting annotation.
the all above functions could be used on windows, ios, android platform, and for java, c++ language.

samples,look at:
mupdf_path/source/z/pdf/samples/pdf_samples.c

x11 is needed by mupdf, to compile mupdf lib, install XQuartz first:
https://dl.bintray.com/xquartz/downloads/XQuartz-2.7.11.dmg

digtal signature is depend on openssl, you can compile it by yourself, or
get pre-built(for macos) binary and include header files at:
https://github.com/zl03jsj/openssl_mac.git

run following command to update submodules:
git submodule update --init --recursive

after all above, you may need to take a look at ./compile.sh, make sure
all 'include' path is at right place

then, compile samples:
$cd mupdf_path/source/z/pdf/
$./compile.sh

if every thing is ok,  you can get executable file: ./pdftest

to test adding annotation, un-comment this code befor run 'compile.sh'
    // testAddAnnotation(ctx, doc, p, ofile);
$./pdftest -p [pageno] -i [./pdffile/test.pdf] -o [./pdffile/out.pdf]

to test add pdf signature, un-comment following code before run 'compile.sh'
    // doTestPdfSign(ctx, doc, pageno, r, ofile);
$./pdftest -p [pageno] -i [./pdffile/test.pdf] -o [./pdffile/out.pdf] -x [x position] -y[y position] -w [width] -h [height]

and the same of insert image into pdf file:
    // doTestAddImage(ctx, doc);

note:
pdf signature samples may needs certificate, there is a test certificate at:
mupdf_path/source/z/pdf/samples/user/zl.pfx
and it's password is '111111'

you must reset the path of pdf file, signature image, certificate
, in pdf_samples.c
the source code is like:
#define RES_PATH        "ResourecePath"
#define RES_Image_file  RES_PATH"/esp_saved.png"
#define RES_Pdf_file    RES_PATH"/pdffile/test.pdf"
#define RES_Cert_file   RES_PATH"/user/zl.pfx"


compile mupdf core library on windows,mac,linux platform:
first step:
compile your openssl lib, get 2 library: libcrypto.a, libssl.a

second:
copy to  mupdf_dir_path/thirdparty/openssl/platform/ dir
platform is: ios, macos, win, linux

finnally:
$cd mupdf_dir_path
$make -f Makefile HAVE_LIBCRYPTO=yes HAVE_OPENSSL_SSL=yes build=debug
you can use "HAVE_OPENSSL=yes" to replace  "HAVE_LIBCRYPTO=yes HAVE_OPENSSL_SSL=yes"
if everything is ok, you can get libmupdf.a at mupdf_dir_path/build/release(debug)/

now you can use libmupdf.a to add image/(hand draw) signature, add image, add
hand draw annotations....

do not forget:
when compile application, you must link openssl library.
example:
gcc ./test_pdf_signature.c -LPathofOpensslLib -lcrypto -lssl -lmupdf

compile mupdf for android:
must install ndk, and compile neeeded architecture openssl lib.
then, excute the following command:
$cd mupdf_dir_path/platform/android/viewer/
$make -C ../../.. generate
$ndk-build
you can get dynamic lib for android at: mupdf_dir_path/platform/android/viewer/lib

for ios:
you can open the project at(mupdf_dir_path/platform/ios) with xcode directly.
select the Mupdf_openssl_sign target to compile.
the ios application's directory like this:
app_path/Documents
    +/imagefiles
        image_files
    +/pfxfiles
        certificate_files
    .....
    pdffiles.pdf
    .....

the test directory is created at:
mupdf_path/platform/ios/Documents/
you can use this folder directly.

if you want show your appreciation, you can contribute money to me:^_^!
![image](https://github.com/zl03jsj/hand-draw-algorithm/blob/master/hand_writing/%E5%BE%AE%E4%BF%A1%E5%9B%BE%E7%89%87_20171216183802.jpg)

mupdf official website:
www.mupdf.com
