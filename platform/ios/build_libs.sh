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


echo Compiling libraries for $ARCHS.
echo HAVE_OPENSSL=$HAVE_OPENSSL
echo make -j4 -C ../.. OUT=$OUT XCFLAGS="$FLAGS" XLDFLAGS="$FLAGS" third libs
make -j4 -C ../.. OUT=$OUT XCFLAGS="$FLAGS" XLDFLAGS="$FLAGS" third libs || exit 1

echo Copying library to $BUILT_PRODUCTS_DIR/.
mkdir -p "$BUILT_PRODUCTS_DIR"
cp -f ../../$OUT/lib*.a $BUILT_PRODUCTS_DIR
ranlib $BUILT_PRODUCTS_DIR/lib*.a

if [ "$HAVE_OPENSSL" = "yes" ] 
then
echo copy openssl libs
cp -f  ../../thirdparty/openssl/ios/lib*.a $BUILT_PRODUCTS_DIR
fi

if [ "$MTokenLib" = "yes" ]
then
cp -f ./z/signdevice/mtoken/Lib/lib*.a $BUILT_PRODUCTS_DIR
fi

echo Done.
