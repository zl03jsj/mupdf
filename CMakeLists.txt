project(mupdf)

cmake_minimum_required(VERSION 3.14)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

#find_library(libs crypto ssl mupdfthird curl bglfw  ./thirdparty/openssl/macos ./build NO_DEFAULT_PATH)
link_directories(./thirdparty/openssl/macos ./build/release)
link_libraries(crypto ssl mupdfthird curl glfw mutools)

#-Wunused-variable -Wunused-parameter -Wunused-function -Wunused
#-Wincompatible-pointer-types-discards-qualifiers

add_definitions(-v -ffunction-sections -fdata-sections -pipe -O2
		-fomit-frame-pointer -O3 -g -W -Wall -Wno-deprecated -Woverloaded-virtual
		-Wwrite-strings -D__WUR= -D_REENTRANT -D_FILE_OFFSET_BITS=64
		-DTIXML_USE_STL -DOPJ_HAVE_STDINT_H -DOPJ_HAVE_INTTYPES_H=1 -DUSE_JPIP=1
        -Dverbose=-1 -DHAVE_UNISTD_H -DHAVE_STDARG_H -DFT2_BUILD_LIBRARY
		-DDARWIN_NO_CARBON -DHAVE_LIBCRYPTO=yes -DNDEBUG -DHAVE_PTHREADS
		-DOPJ_HAVE_INTTYPES_H=1 -DUSE_JPIP=1
		-DFT_CONFIG_MODULES_H="slimftmodules.h" -DFT_CONFIG_OPTIONS_H="slimftoptions.h")

include_directories(./include)
include_directories(./generated)
include_directories(./scripts/freetype)
include_directories(./thirdparty/freetype/include)
include_directories(./thirdparty/harfbuzz/src)
include_directories(./thirdparty/jbig2dec)
include_directories(./scripts/jpeg)
include_directories(./thirdparty/jpeg)
include_directories(./thirdparty/mujs)
include_directories(./thirdparty/openjpeg/src/lib/openjp2)
include_directories(./thirdparty/zlib)
include_directories(./source/fitz)
include_directories(./include)
include_directories(./thirdparty/jpeg)
include_directories(/usr/X11R6/include)

aux_source_directory(./source/cbz cbz)
aux_source_directory(./source/fitz fitz)
aux_source_directory(./source/gprf gprf)
aux_source_directory(./source/html html)
aux_source_directory(./source/pdf pdf)
aux_source_directory(./source/svg svg)
aux_source_directory(./source/svg tools)
aux_source_directory(./source/xps xps)
aux_source_directory(./source/z/pdf zpdf)
aux_source_directory(./generated generated)

add_executable(pdf-sample ./source/z/pdf/samples/pdf_samples.c
        ${cbz} ${fitz} ${gprf} ${html} ${pdf} ${svg} ${xps} ${zpdf} ${generated})

