CC = gcc
CFLAGS = -Wall -O2

OBJDIR = ../../build/obj/png_app
BINDIR = ../../build/bin

ZLIB_DIR = ../../zlib
LIBPNG_DIR = ../../libpng
FREETYPE_DIR = ../../freetype

INCLUDES = -I../../elfload -I$(LIBPNG_DIR) -I$(ZLIB_DIR) -I$(FREETYPE_DIR)/include

STATIC_TARGET = $(BINDIR)/app-static
DYNAMIC_TARGET = $(BINDIR)/app-dynamic
BLOB_TARGET = $(BINDIR)/app-blob

STATIC_OBJS = $(OBJDIR)/static_main.o $(OBJDIR)/static_png_text.o
DYNAMIC_OBJS = $(OBJDIR)/dynamic_main.o $(OBJDIR)/dynamic_png_text.o
BLOB_OBJS = $(OBJDIR)/blob_main.o $(OBJDIR)/blob_png_text.o

STATIC_LIBS = $(LIBPNG_DIR)/.libs/libpng16.a \
              $(FREETYPE_DIR)/objs/.libs/libfreetype.a \
              $(ZLIB_DIR)/libz.a \
              -lm -ldl

DYNAMIC_LIBS = -ldl -lm

BLOB_LDFLAGS = -nostdlib -nodefaultlibs -nostartfiles -pie \
               -Wl,-e,main \
               -Wl,--no-dynamic-linker \
               -Wl,--build-id=none

BLOB_CFLAGS = -fPIE -DBUILD_BLOB \
              -fno-stack-protector \
              -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
              -fcf-protection=none \
              -fno-asynchronous-unwind-tables \
              -fno-unwind-tables
			  
.PHONY: all static dynamic blob directories clean

all: static dynamic blob

static: directories $(STATIC_TARGET)

dynamic: directories $(DYNAMIC_TARGET)

blob: directories $(BLOB_TARGET)

directories:
	mkdir -p $(OBJDIR) $(BINDIR)

$(STATIC_TARGET): $(STATIC_OBJS)
	$(CC) -static $(STATIC_OBJS) $(STATIC_LIBS) -o $@

$(DYNAMIC_TARGET): $(DYNAMIC_OBJS)
	$(CC) $(DYNAMIC_OBJS) $(DYNAMIC_LIBS) -o $@

$(BLOB_TARGET): $(BLOB_OBJS)
	$(CC) $(BLOB_LDFLAGS) $(BLOB_OBJS) -o $@

$(OBJDIR)/static_main.o: main.c png_text.h
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_STATIC -c main.c -o $@

$(OBJDIR)/static_png_text.o: png_text.c png_text.h
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_STATIC -c png_text.c -o $@

$(OBJDIR)/dynamic_main.o: main.c png_text.h
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_DYNAMIC -c main.c -o $@

$(OBJDIR)/dynamic_png_text.o: png_text.c png_text.h
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_DYNAMIC -c png_text.c -o $@

$(OBJDIR)/blob_main.o: main.c png_text.h blob_func.h
	$(CC) $(CFLAGS) $(BLOB_CFLAGS) $(INCLUDES) -c main.c -o $@

$(OBJDIR)/blob_png_text.o: png_text.c png_text.h blob_func.h
	$(CC) $(CFLAGS) $(BLOB_CFLAGS) $(INCLUDES) -c png_text.c -o $@

clean:
	rm -f $(OBJDIR)/*.o $(STATIC_TARGET) $(DYNAMIC_TARGET) $(BLOB_TARGET)