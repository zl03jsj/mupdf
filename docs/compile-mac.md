# build on macos

执行编译命令:`make apps` 报错: `ar: no archive members specified`

# build MUpdf  with `opengl` render
在mac上编译mupdf的阅读器, 有两个targe,
分别对应了分opengl渲染和x11渲染.

在文件`MakeThird`中,定义了如下内容:
```
ifeq ($(OS),MACOS)
  USE_SYSTEM_GLUT := yes
endif
```
但是在`MakeFile`中, 有如下内容:
```
THIRD_GLUT_LIB = $(OUT)/libmupdf-glut.a
..
ifeq ($(HAVE_GLUT),yes)
  MUVIEW_GLUT_SRC += $(sort $(wildcard platform/gl/*.c))
  MUVIEW_GLUT_OBJ := $(MUVIEW_GLUT_SRC:%.c=$(OUT)/%.o)
  MUVIEW_GLUT_EXE := $(OUT)/mupdf-gl
  $(MUVIEW_GLUT_EXE) : $(MUVIEW_GLUT_OBJ) $(MUPDF_LIB) $(THIRD_LIB) $(THIRD_GLUT_LIB) $(PKCS7_LIB)
	$(LINK_CMD) $(THIRD_LIBS) $(LIBCRYPTO_LIBS) $(WIN32_LDFLAGS) $(THIRD_GLUT_LIBS)
  VIEW_APPS += $(MUVIEW_GLUT_EXE)
endif
```
**mpudf-gl版本的阅读器依赖了`$(THIRD_GLUT_LIB)`,但在mac系统上只需`$(THIRD_GLUT_LIBS)`**

**并且target:`$(THIRD_GLUT__LIB)`依赖的的src和object在macos中为空,所以添加如下内容:**

```
ifeq ($(USE_SYSTEM_GLUT), yes)
  THIRD_GLUT_LIB =
endif
```

# build pdf with `x11` render
可以在`MakeRule 上看到, MACOS上并没有自动检测`x11`版本的viewer.
```
ifeq ($(OS),MACOS)
  HAVE_GLUT := yes
  SYS_GLUT_CFLAGS := -Wno-deprecated-declarations
  SYS_GLUT_LIBS := -framework GLUT -framework OpenGL
  CC = xcrun cc
  AR = xcrun ar
  LD = xcrun ld
  RANLIB = xcrun ranlib

else ifeq ($(OS),Linux)
..
  HAVE_X11 := $(shell pkg-config --exists x11 xext && echo yes)
  ifeq ($(HAVE_X11),yes)
	X11_CFLAGS := $(shell pkg-config --cflags x11 xext)
	X11_LIBS := $(shell pkg-config --libs x11 xext)
  endif
..
```
所以,如果需要编译`x11`版本的阅读器,把这一部分迁移到了外部,不管什么系统都执行自动检测:
```
  HAVE_X11 := $(shell pkg-config --exists x11 xext && echo yes)
  ifeq ($(HAVE_X11),yes)
	X11_CFLAGS := $(shell pkg-config --cflags x11 xext)
	X11_LIBS := $(shell pkg-config --libs x11 xext)
  endif
```

**此外,在macos上编译x11版本的viewer需要安装以下包:**

>XQuartz:https://github.com/XQuartz/XQuartz/releases/download/XQuartz-2.8.0/XQuartz-2.8.0.dmg
>xproto:https://www.x.org/archive/individual/proto/xproto-7.0.31.tar.gz
>kbproto:https://www.x.org/archive/individual/proto/kbproto-1.0.7.tar.gz
>xextproto:https://www.x.org/archive/individual/proto/xextproto-7.0.2.tar.bz2

**除了XQuartz是直接下载安装外, 剩余3个依赖项安装方法为(以xext)为例**

```
wget https://www.x.org/archive/individual/proto/xextproto-7.0.2.tar.bz2
tar -xvf ./xextproto-7.0.2.tar.bz2 && cd exetproto
./configure --prefix=/usr/local/exetproto/7.0.2
make install
echo export PKG_CONFIG_PATH=/usr/local/exetproto/7.0.2/lib/pkgconfig:$PKG_CONFIG_PATH  >> ~/.zshrc
source ~/.zshrc
pkg-config --cflags xext
`-I/opt/X11/include -I/usr/local/xproto/7_0_31/include -I/usr/local/kbproto/1.0.7/include -I/usr/local/exetproto/7.0.2/include`
```

**完成以上操作后, 执行`make HAVE_X11=yes apps`可以编译出`mupdf-gl(opengl)`版本和`mupdf-x11(x11)`版本的可执行程序**