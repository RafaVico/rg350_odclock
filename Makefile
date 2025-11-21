APPNAME      := odclock
ifeq ($(PLATFORM), rg350)
  CC         := /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
  STRIP      := /opt/gcw0-toolchain/usr/bin/mipsel-linux-strip
  LIBS       := -L/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib
  INCS	     := -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include -I inc
  DEFS       := -DPLATFORM_RG350
else ifeq ($(PLATFORM), miyoo)
  CC         := /opt/mmiyoo/usr/bin/arm-linux-gnueabihf-gcc
  AR         := /opt/mmiyoo/usr/bin/arm-linux-gnueabihf-ar
  STRIP      := /opt/mmiyoo/usr/bin/arm-linux-gnueabihf-strip
  LIBS       := -L/opt/mmiyoo/usr/arm-buildroot-linux-gnueabihf/sysroot/usr/lib -L/opt/mmiyoo/usr/lib
  INCS       := -I/opt/mmiyoo/usr/arm-buildroot-linux-gnueabihf/sysroot/usr/include -Iinc
  DEFS       := -DPLATFORM_MIYOO
endif

CC           ?= g++
STRIP        ?= strip
TARGET       ?= release/$(PLATFORM)/$(APPNAME)
SYSROOT      := $(shell $(CC) --print-sysroot)
CFLAGS       := $(LIBS) -lSDL_mixer -lSDL_ttf -lSDL_image -lfreetype -lz -lSDL -lm
SRCDIR       := src
OBJDIR       := .obj
TARDIR	     := release/$(PLATFORM)
SRC          := $(wildcard $(SRCDIR)/*.cpp)
OBJ          := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
SOVERSION    := $(TARGET).0

ifdef DEBUG
  CFLAGS += -ggdb -Wall -Werror
else
  ifeq ($(PLATFORM), RG350)
    CFLAGS += -O3
  else ifeq ($(PLATFORM), MIYOO)
    CFLAGS += -O3 -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve -Wall
    LDFLAGS = -lpthread -s -lpng16
  endif
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(TARDIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
	$(STRIP) $@
ifeq ($(PLATFORM), linux)
	cp $(TARGET) .
endif

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) -c $< -o $@ $(INCS) $(DEFS)

$(OBJDIR):
	mkdir -p $@

clean:
	rm -Rf $(TARGET) $(OBJDIR)

