project(mupdf)

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(./include)
include_directories(./thirdparty/jpeg)

aux_source_directory(./source/cbz cbz)
aux_source_directory(./source/fitz fitz)
aux_source_directory(./source/gprf gprf)
aux_source_directory(./source/html html)
aux_source_directory(./source/pdf pdf)
aux_source_directory(./source/svg svg)
aux_source_directory(./source/svg tools)
aux_source_directory(./source/xps xps)
aux_source_directory(./source/z/pdf zpdf)

MESSAGE(" the source directory : ${cbz}")
MESSAGE(" the source directory : ${fitz}")
MESSAGE(" the source directory : ${gprf}")
MESSAGE(" the source directory : ${html}")
MESSAGE(" the source directory : ${pdf}")
MESSAGE(" the source directory : ${svg}")
MESSAGE(" the source directory : ${xps}")
MESSAGE(" the source directory : ${zpdf}")

add_executable(pdf-sample ./source/z/pdf/samples/pdf_samples.c
        ${cbz} ${fitz} ${gprf} ${html} ${pdf} ${svg} ${xps} ${zpdf})

