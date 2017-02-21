#!/bin/bash

# Call this script from a "Run Script" target in the Xcode project to
# cross compile MuPDF and third party libraries using the regular Makefile.
# Also see "iOS" section in Makerules.

if [ ! -e ../../generated/gen_cmap_korea.h ]
then
	echo 'ERROR: You are missing the generated files.'
	echo 'ERROR: Please run "make generate" from the mupdf directory.'
	exit 1
fi

export OS=ios

export build=$(echo $CONFIGURATION | tr A-Z a-z)


echo "note: export CONFIGURATION to set 'build' setting of makefile..."
echo "      export SVR_SIGN to make 'esp parsing' and 'ntko server sign helper' to
build in mupdf core"

FLAGS="-Wno-unused-function -Wno-empty-body -Wno-implicit-function-declaration"

for A in $ARCHS
do
    FLAGS="$FLAGS -arch $A -D"`(echo $A)| tr "[a-z]" "[A-Z]"`
done

#  add bitcode for Xcode 7 and up
XCODE_VER=`xcodebuild -version | head -1`
ARRAY=(${XCODE_VER// / })
XCODE_VER_NUM=${ARRAY[1]}
ARRAY=(${XCODE_VER_NUM//./ })
MAJOR=${ARRAY[0]}
if [ "$MAJOR" -ge "7" ]
then
	FLAGS="$FLAGS -fembed-bitcode"
fi

OUT=build/$build-$OS-$(echo $ARCHS | tr ' ' '-')

MUPDF_TARGET="third libs"  


BUILD_VAR=
if [ "$SVR_SIGN" = "yes" ]
then
    echo "compile esp and svrhelper into mupdf lib"
    export CORE_ESP=yes
    export CORE_SVR_HELPER=yes
    MUPDF_TARGET="$MUPDF_TARGET curl"
fi

echo Compiling libraries for $ARCHS.
echo HAVE_LIBCRYPTO=$HAVE_LIBCRYPTO
make -j4 -C ../..  OUT=$OUT XCFLAGS="$FLAGS" XLDFLAGS="$FLAGS" $MUPDF_TARGET || exit 1

echo Copying library to $BUILT_PRODUCTS_DIR/.
mkdir -p "$BUILT_PRODUCTS_DIR"
cp -f ../../$OUT/lib*.a $BUILT_PRODUCTS_DIR
ranlib $BUILT_PRODUCTS_DIR/lib*.a

if [ "$HAVE_LIBCRYPTO" = "yes" ] 
then
echo copy openssl libs
cp -f  ../../thirdparty/openssl/ios/lib*.a $BUILT_PRODUCTS_DIR
fi

echo Done.
