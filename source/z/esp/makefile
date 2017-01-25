out := ./
build ?= debug
include_path := -I../../../include
mupdf_lib_path := ../../../build/$(build)
flags := $(include_path)
flags += -Wno-c++11-extensions -fshort-wchar

ifeq "$(build)" "debug"
flags += -g
endif

mupdf_libs := $(mupdf_lib_path)/libmupdf.a $(mupdf_lib_path)/libmupdfthird.a

srcs := $(wildcard ./*.cpp)
srcs += $(wildcard ./*.c)

objects := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(srcs)))

$(mupdf_libs):
	$(MAKE) -C ../../../ build=$(build) 

%.o : %.cpp
	$(CXX) $(flags) -o $@ -c $<

%.o : %.c
	$(CC) $(flags) -o $@ -c $<

info:
	@echo clang $(flags) -o $(objects) -c $(srcs)

esplib: $(objects)
	ar cr libesp.a $^

clean:
	rm -f ./*.o
	rm -f ./*.a
	rm -f ./esptest

esptest: $(objects) $(mupdf_libs)
	$(CXX) -L$(mupdf_lib_path) -lmupdf -lmupdfthird -o $@ $(objects)

