# make pdftest
os ?= $(shell uname)
os := $(os:MINGW%=MINGW)
os := $(os:Windows_NT=MINGW)
os := $(os:Darwin=macos)


ifeq "$(OS)" "macos"
CC = xcrun cc
AR = xcrun ar
LD = xcrun ld
RANLIB_CMD = xcrun ranlib $@
else ifeq "$(OS)" "ios" 
CC = xcrun cc
AR = xcrun ar
LD = xcrun ld
RANLIB_CMD = xcrun ranlib $@
endif


HAVE_LIBCRYPTO ?= yes
build ?= debug

incpath := -I../../../include -I./ 
libs := -L../../../build/$(build) -lmupdf -lmupdfthird -lm

ifeq "$(HAVE_LIBCRYPTO)" "yes"
libs += -L../../../thirdparty/openssl/$(os) -lcrypto -lssl
endif

sourcedirs := ./samples/
source := $(wildcard $(sourcedirs)*.c)
objs := $(source:%.c=%.o)

flags :=

ifeq "$(build)" "debug"
flags += -g
endif

default: mupdf $(objs) showinfo
	@echo ........................
	$(CC) $(flags) -o pdftest $(objs) $(libs) 
	@echo ........................

$(objs): %.o:%.c
	$(CC) $(flags) -c $(incpath) $< -o $@
	
# $(sourcedirs)%.o : $(sourcedirs)%.c
	# $(CC) -c $(flags) $< -o $@


mupdf:
	$(MAKE) -C ../../../ XCFLAGS=-fPIC build=$(build) HAVE_LIBCRYPTO=$(HAVE_LIBCRYPTO) libs

clean:
	rm -f ./Test/*.o
	rm -r -f ../../../build/$(build)

showinfo:
	@echo flags=$(flags)

