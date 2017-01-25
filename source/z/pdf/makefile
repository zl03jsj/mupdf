# make pdftest
os ?= $(shell uname)
os := $(os:MINGW%=MINGW)
os := $(os:Windows_NT=MINGW)
os := $(os:Darwin=macos)

HAVE_LIBCRYPTO ?= yes
build ?= debug

incpath := -I../../../include -I./ 
libs := -L../../../build/$(build) -lmupdf -lmupdfthird

ifeq "$(HAVE_LIBCRYPTO)" "yes"
libs += -L../../../thirdparty/openssl/$(os) -lcrypto -lssl
endif

sourcedirs := ./Test/
source := $(wildcard $(sourcedirs)*.c)
objs := $(source:%.c=%.o)

flags :=

ifeq "$(build)" "debug"
flags += -g
endif

default: mupdf $(objs) showinfo
	@echo ........................
	clang $(flags) -o pdftest $(objs) $(libs) 
	@echo ........................

$(objs): %.o:%.c
	clang $(flags) -c $(incpath) $< -o $@
	
# $(sourcedirs)%.o : $(sourcedirs)%.c
	# clang -c $(flags) $< -o $@


mupdf:
	$(MAKE) -C ../../../ XCFLAGS=-fPIC build=$(build) HAVE_LIBCRYPTO=$(HAVE_LIBCRYPTO) libs

clean:
	rm -f ./Test/*.o
	rm -r -f ../../../build/$(build)

showinfo:
	@echo flags=$(flags)

