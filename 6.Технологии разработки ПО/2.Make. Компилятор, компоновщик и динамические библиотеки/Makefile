ZLIB_DIR = zlib
LIBPNG_DIR = libpng
FREETYPE_DIR = freetype
APP_DIR = app/png_app
ELFLOAD_DIR = elfload

ZLIB_STATIC = $(CURDIR)/$(ZLIB_DIR)/libz.a
LIBPNG_STATIC = $(CURDIR)/$(LIBPNG_DIR)/.libs/libpng16.a
FREETYPE_STATIC = $(CURDIR)/$(FREETYPE_DIR)/objs/.libs/libfreetype.a

ZLIB_DYNAMIC = $(CURDIR)/$(ZLIB_DIR)/libz.so
LIBPNG_DYNAMIC = $(CURDIR)/$(LIBPNG_DIR)/.libs/libpng16.so
FREETYPE_DYNAMIC = $(CURDIR)/$(FREETYPE_DIR)/objs/.libs/libfreetype.so

ELFLOADER_BIN = $(CURDIR)/$(ELFLOAD_DIR)/elfloader

.PHONY: all static dynamic blob clean

all: clean static dynamic blob

static: $(ZLIB_STATIC) $(LIBPNG_STATIC) $(FREETYPE_STATIC)
	$(MAKE) -C $(APP_DIR) static

dynamic: $(ZLIB_DYNAMIC) $(LIBPNG_DYNAMIC) $(FREETYPE_DYNAMIC)
	$(MAKE) -C $(APP_DIR) dynamic

blob: $(ZLIB_DYNAMIC) $(LIBPNG_DYNAMIC) $(FREETYPE_DYNAMIC) $(ELFLOADER_BIN)
	$(MAKE) -C $(APP_DIR) blob

$(ZLIB_STATIC):
	cd $(ZLIB_DIR) && ./configure --static && $(MAKE)

$(LIBPNG_STATIC): $(ZLIB_STATIC)
	cd $(LIBPNG_DIR) && ./configure --disable-shared --enable-static \
		CPPFLAGS="-I$(CURDIR)/$(ZLIB_DIR)" \
		LDFLAGS="-L$(CURDIR)/$(ZLIB_DIR)" && $(MAKE)

$(FREETYPE_STATIC):
	cd $(FREETYPE_DIR) && ./configure --disable-shared --enable-static && $(MAKE)

$(ZLIB_DYNAMIC):
	cd $(ZLIB_DIR) && ./configure && $(MAKE)

$(LIBPNG_DYNAMIC): $(ZLIB_DYNAMIC)
	cd $(LIBPNG_DIR) && ./configure --enable-shared --disable-static \
		CPPFLAGS="-I$(CURDIR)/$(ZLIB_DIR)" \
		LDFLAGS="-L$(CURDIR)/$(ZLIB_DIR)" && $(MAKE)

$(FREETYPE_DYNAMIC):
	cd $(FREETYPE_DIR) && ./configure --enable-shared --disable-static && $(MAKE)

$(ELFLOADER_BIN):
	$(MAKE) -C $(ELFLOAD_DIR) elfloader

clean:
	-$(MAKE) -C $(APP_DIR) clean
	-cd $(ZLIB_DIR) && $(MAKE) clean
	-cd $(LIBPNG_DIR) && $(MAKE) clean
	-cd $(FREETYPE_DIR) && $(MAKE) clean
	-cd $(ELFLOAD_DIR) && $(MAKE) clean