OS ?= $(shell uname)

build ?= debug

ifeq "$(OS)" "Darwin"
MUPDF_JAVA := libmupdf_java64.jnilib
JAVA_CFLAGS := \
	-I /Library/Java/JavaVirtualMachines/jdk1.8.0_102.jdk/Contents/Home/include \
	-I /Library/Java/JavaVirtualMachines/jdk1.8.0_102.jdk/Contents/Home/include/darwin
else

BITS := $(shell getconf LONG_BIT)

MUPDF_JAVA := libmupdf_java$(BITS).so
JAVA_CFLAGS := \
	-I /usr/lib/jvm/java-7-openjdk-i386/include \
	-I /usr/lib/jvm/java-7-openjdk-i386/include/linux \
	-I /usr/lib/jvm/java-7-openjdk-amd64/include \
	-I /usr/lib/jvm/java-7-openjdk-amd64/include/linux \
	$(shell pkg-config --cflags freetype2) \
	$(shell pkg-config --cflags libcrypto)
JAVA_LIBS := \
	$(shell pkg-config --libs freetype2) \
	$(shell pkg-config --libs libcrypto) \
	-lz
endif

FLAGS ?= debug
ifeq "$(build)" "debug"
FLAGS = -g -fPIC -DDebug -DDEBUG -Wall -Wextra -Wno-unused-parameter -I../../include $(JAVA_CFLAGS)
else
build = release
FLAGS = -fPIC -Wall -Wextra -Wno-unused-parameter -I../../include $(JAVA_CFLAGS)
endif

default: libmupdf.jar

MUPDF_CORE := ../../build/java/libmupdf.a ../../build/java/libmupdfthird.a

HAVE_LIBCRYPTO ?= no
MUPDF_OPENSSL_MAKE_FLAGS := 

ifeq "$(HAVE_LIBCRYPTO)" "yes"
subdir := $(subst Windows_NT,windows,$(subst Darwin,macos,$(OS:MIINGW%=MINGW)))
OPENSSL_DIR := ../../thirdparty/openssl/$(subdir)/
OPENSSL_LIBS := -L./thirdparty/openssl/$(subdir)/ -lcrypto -lssl
LIBCRYPTO_CFLAGS:= -DHAVE_LIBCRYPTO
MUPDF_CORE += $(OPENSSL_DIR)libcrypto.a $(OPENSSL_DIR)libssl.a 
MUPDF_OPENSSL_MAKE_FLAGS = HAVE_LIBCRYPTO=yes LIBCRYPTO_CFLAGS=$(LIBCRYPTO_CFLAGS) LIBCRYPTO_LIBS=$(OPENSSL_LIBS)
endif

ifeq "$(OS)" "Linux"
$(MUPDF_CORE) :
	$(MAKE) -C ../.. OUT=build/java XCFLAGS=-fPIC $(MUPDF_OPENSSL_MAKE_FLAGS) FREETYPE_DIR=/foo ZLIB_DIR=/foo build=$(build) libs
else
$(MUPDF_CORE) :
	$(MAKE) -C ../.. OUT=build/java XCFLAGS=-fPIC $(MUPDF_OPENSSL_MAKE_FLAGS) build=$(build) libs
endif

JAVA_SRC := ./src/
JAVA_SRC_DIRS := $(JAVA_SRC)com/artifex/mupdf/fitz/*.java $(JAVA_SRC)com/z/math/*.java $(JAVA_SRC)com/z/*.java $(JAVA_SRC)*.java $(JAVA_SRC)example/*.java
LIBRARY_JAVA_SOURCES := $(sort $(wildcard $(JAVA_SRC_DIRS)))

LIBRARY_JAVA_OBJECTS := $(LIBRARY_JAVA_SOURCES:%.java=%.class)

LIBRARY_JAVA_CLASSES := $(subst $(JAVA_SRC),,$(LIBRARY_JAVA_SOURCES:%.java=%))
LIBRARY_JAVA_CLASSES := $(subst com/artifex/mupdf/fitz/,com.artifex.mupdf.fitz.,$(LIBRARY_JAVA_CLASSES))
LIBRARY_JAVA_CLASSES := $(subst com/z/math/,com.z.math.,$(LIBRARY_JAVA_CLASSES))
LIBRARY_JAVA_CLASSES := $(subst com/z/,com.z.,$(LIBRARY_JAVA_CLASSES))
LIBRARY_JAVA_CLASSES := $(subst example/,example.,$(LIBRARY_JAVA_CLASSES))

$(LIBRARY_JAVA_OBJECTS) : $(LIBRARY_JAVA_SOURCES)
	javac -source 1.7 -target 1.7 $^

EXAMPLE_JAVA_SOURCES := $(sort $(wildcard $(JAVA_SRC)example/*.java))
EXAMPLE_JAVA_OBJECTS := $(EXAMPLE_JAVA_SOURCES:%.java=%.class)

$(EXAMPLE_JAVA_OBJECTS) : $(EXAMPLE_JAVA_SOURCES)
	javac -source 1.7 -target 1.7  -classpath .:src $^

example: $(EXAMPLE_JAVA_SOURCES)
	javac -source 1.7 -target 1.7 -classpath .:src $^

src_lib_dir := $(JAVA_SRC)lib/

$(src_lib_dir) :
	mkdir $@

# jar_objects := $(LIBRARY_JAVA_OBJECTS:$(JAVA_SRC)%=%)
jar_objects := $(LIBRARY_JAVA_OBJECTS) $(src_lib_dir)$(MUPDF_JAVA)
jar_objects := $(subst $(JAVA_SRC),-C $(JAVA_SRC) ,$(jar_objects)) 

libmupdf.jar : $(LIBRARY_JAVA_OBJECTS) $(src_lib_dir) $(MUPDF_JAVA)
	cp $(MUPDF_JAVA) $(src_lib_dir)
	jar cvf $@ $(jar_objects)

mupdf_native.h : $(LIBRARY_JAVA_OBJECTS)
	rm -f $@
	javah -classpath .:src -o $@ $(LIBRARY_JAVA_CLASSES)

mupdf_native.o : mupdf_native.c mupdf_native.h
	$(CC) $(FLAGS) -o $@ -c $< 

$(MUPDF_JAVA) : mupdf_native.o $(MUPDF_CORE) 
	$(CC) -shared -o $(MUPDF_JAVA) $^ $(JAVA_LIBS)

viewer: $(MUPDF_JAVA) $(EXAMPLE_JAVA_OBJECTS)
	LD_LIBRARY_PATH=. java -classpath .:src example.Viewer

clean:
	rm -f $(JAVA_SRC)com/z/*.class
	rm -f $(JAVA_SRC)com/z/math/*.class
	rm -f $(JAVA_SRC)com/artifex/mupdf/fitz/*.class
	rm -f $(JAVA_SRC)lib/*
	rm -f $(JAVA_SRC)example/*.class
	rm -f mupdf_native.o
	rm -f $(MUPDF_JAVA)

nuke: clean
	$(MAKE) -C ../.. OUT=build/java clean

showinformation:
	@echo compile informations_______________
	@echo build mupdf command 
	@echo "$(MAKE) -C ../.. OUT=build/java XCFLAGS=-fPIC $(MUPDF_OPENSSL_MAKE_FLAGS) build=$(build) libs"
	@echo build mupdf_native.c command
	@echo "$(CC) -o ... -c ... -fPIC -g -Wall -Wextra -Wno-unused-parameter -I ../../include $(JAVA_CFLAGS)"
	@echo "---------------------"

.NOTPARALLEL : # disable -j option (it breaks since javac compiles all class files in one command)
