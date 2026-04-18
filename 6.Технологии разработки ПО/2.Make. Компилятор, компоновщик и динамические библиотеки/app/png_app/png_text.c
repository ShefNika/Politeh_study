#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h> // API for libpng
#include <ft2build.h>
#include FT_FREETYPE_H // standart for freetype
#include "png_text.h"

#ifdef BUILD_BLOB
#include "blob_func.h"
#endif

#define IMAGE_WIDTH   1000
#define IMAGE_HEIGHT  250
#define FONT_SIZE     64
#define START_X       40
#define BASELINE_Y    150

#ifdef BUILD_BLOB

clock_t (*blob_clock_ptr)(void) = NULL;
FILE *(*blob_fopen_ptr)(const char *, const char *) = NULL;
int (*blob_fclose_ptr)(FILE *) = NULL;
size_t (*blob_fread_ptr)(void *, size_t, size_t, FILE *) = NULL;
size_t (*blob_fwrite_ptr)(const void *, size_t, size_t, FILE *) = NULL;
void *(*blob_malloc_ptr)(size_t) = NULL;
void (*blob_free_ptr)(void *) = NULL;
void *(*blob_realloc_ptr)(void *, size_t) = NULL;
void *(*blob_memset_ptr)(void *, int, size_t) = NULL;
void *(*blob_memcpy_ptr)(void *, const void *, size_t) = NULL;
int (*blob_printf_ptr)(const char *, ...) = NULL;
int (*blob_fprintf_ptr)(FILE *, const char *, ...) = NULL;
int (*blob__setjmp_ptr)(jmp_buf env) = NULL;
void (*blob_longjmp_ptr)(jmp_buf env, int val) = NULL;
void (*blob___longjmp_chk_ptr)(jmp_buf env, int val) = NULL;

int _setjmp(jmp_buf env)
{
    return blob__setjmp_ptr(env);
}

void longjmp(jmp_buf env, int val)
{
    blob_longjmp_ptr(env, val);
    for (;;)
        ;
}

void __longjmp_chk(jmp_buf env, int val)
{
    blob___longjmp_chk_ptr(env, val);
    for (;;)
        ;
}

FT_Error (*blob_FT_Init_FreeType_ptr)(FT_Library *) = NULL;
FT_Error (*blob_FT_New_Face_ptr)(FT_Library, const char *, FT_Long, FT_Face *) = NULL;
FT_Error (*blob_FT_Set_Pixel_Sizes_ptr)(FT_Face, FT_UInt, FT_UInt) = NULL;
FT_Error (*blob_FT_Load_Char_ptr)(FT_Face, FT_ULong, FT_Int32) = NULL;
FT_Error (*blob_FT_Done_Face_ptr)(FT_Face) = NULL;
FT_Error (*blob_FT_Done_FreeType_ptr)(FT_Library) = NULL;

png_structp (*blob_png_create_write_struct_ptr)(png_const_charp, png_voidp, png_error_ptr, png_error_ptr) = NULL;
png_infop (*blob_png_create_info_struct_ptr)(png_structp) = NULL;
void (*blob_png_destroy_write_struct_ptr)(png_structpp, png_infopp) = NULL;
jmp_buf *(*blob_png_set_longjmp_fn_ptr)(png_structrp, png_longjmp_ptr, size_t) = NULL;
void (*blob_png_init_io_ptr)(png_structp, FILE *) = NULL;
void (*blob_png_set_IHDR_ptr)(png_structp, png_infop, png_uint_32, png_uint_32, int, int, int, int, int) = NULL;
void (*blob_png_write_info_ptr)(png_structp, png_infop) = NULL;
void (*blob_png_write_image_ptr)(png_structp, png_bytepp) = NULL;
void (*blob_png_write_end_ptr)(png_structp, png_infop) = NULL;

void set_blob_functions(void **functions)
{
    blob_clock_ptr = (clock_t (*)(void))functions[BLOB_CLOCK];
    blob_fopen_ptr = (FILE *(*)(const char *, const char *))functions[BLOB_FOPEN];
    blob_fclose_ptr = (int (*)(FILE *))functions[BLOB_FCLOSE];
    blob_fread_ptr = (size_t (*)(void *, size_t, size_t, FILE *))functions[BLOB_FREAD];
    blob_fwrite_ptr = (size_t (*)(const void *, size_t, size_t, FILE *))functions[BLOB_FWRITE];
    blob_malloc_ptr = (void *(*)(size_t))functions[BLOB_MALLOC];
    blob_free_ptr = (void (*)(void *))functions[BLOB_FREE];
    blob_realloc_ptr = (void *(*)(void *, size_t))functions[BLOB_REALLOC];
    blob_memset_ptr = (void *(*)(void *, int, size_t))functions[BLOB_MEMSET];
    blob_memcpy_ptr = (void *(*)(void *, const void *, size_t))functions[BLOB_MEMCPY];
    blob_printf_ptr = (int (*)(const char *, ...))functions[BLOB_PRINTF];
    blob_fprintf_ptr = (int (*)(FILE *, const char *, ...))functions[BLOB_FPRINTF];
    blob__setjmp_ptr = (int (*)(jmp_buf))functions[BLOB__SETJMP];
    blob_longjmp_ptr = (void (*)(jmp_buf, int))functions[BLOB_LONGJMP];
    blob___longjmp_chk_ptr = (void (*)(jmp_buf, int))functions[BLOB___LONGJMP_CHK];
    
    blob_FT_Init_FreeType_ptr = (FT_Error (*)(FT_Library *))functions[BLOB_FT_Init_FreeType];
    blob_FT_New_Face_ptr = (FT_Error (*)(FT_Library, const char *, FT_Long, FT_Face *))functions[BLOB_FT_New_Face];
    blob_FT_Set_Pixel_Sizes_ptr = (FT_Error (*)(FT_Face, FT_UInt, FT_UInt))functions[BLOB_FT_Set_Pixel_Sizes];
    blob_FT_Load_Char_ptr = (FT_Error (*)(FT_Face, FT_ULong, FT_Int32))functions[BLOB_FT_Load_Char];
    blob_FT_Done_Face_ptr = (FT_Error (*)(FT_Face))functions[BLOB_FT_Done_Face];
    blob_FT_Done_FreeType_ptr = (FT_Error (*)(FT_Library))functions[BLOB_FT_Done_FreeType];

    blob_png_create_write_struct_ptr = (png_structp (*)(png_const_charp, png_voidp, png_error_ptr, png_error_ptr)) functions[BLOB_png_create_write_struct];
    blob_png_create_info_struct_ptr = (png_infop (*)(png_structp)) functions[BLOB_png_create_info_struct];
    blob_png_destroy_write_struct_ptr = (void (*)(png_structpp, png_infopp)) functions[BLOB_png_destroy_write_struct];
    blob_png_set_longjmp_fn_ptr = (jmp_buf *(*)(png_structrp, png_longjmp_ptr, size_t)) functions[BLOB_png_set_longjmp_fn];
    blob_png_init_io_ptr = (void (*)(png_structp, FILE *))functions[BLOB_png_init_io];
    blob_png_set_IHDR_ptr = (void (*)(png_structp, png_infop, png_uint_32, png_uint_32, int, int, int, int, int))functions[BLOB_png_set_IHDR];
    blob_png_write_info_ptr = (void (*)(png_structp, png_infop))functions[BLOB_png_write_info];
    blob_png_write_image_ptr =(void (*)(png_structp, png_bytepp))functions[BLOB_png_write_image];
    blob_png_write_end_ptr =(void (*)(png_structp, png_infop))functions[BLOB_png_write_end];
}

#endif

#ifdef BUILD_DYNAMIC
#include <dlfcn.h>

static void *zlib_handle = NULL; 
static void *libpng_handle = NULL;
static void *freetype_handle = NULL;

static FT_Error (*dyn_FT_Init_FreeType)(FT_Library *) = NULL;
static FT_Error (*dyn_FT_New_Face)(FT_Library, const char *, FT_Long, FT_Face *) = NULL;
static FT_Error (*dyn_FT_Set_Pixel_Sizes)(FT_Face, FT_UInt, FT_UInt) = NULL;
static FT_Error (*dyn_FT_Load_Char)(FT_Face, FT_ULong, FT_Int32) = NULL;
static FT_Error (*dyn_FT_Done_Face)(FT_Face) = NULL;
static FT_Error (*dyn_FT_Done_FreeType)(FT_Library) = NULL;

static png_structp (*dyn_png_create_write_struct)(png_const_charp, png_voidp, png_error_ptr, png_error_ptr) = NULL;
static png_infop (*dyn_png_create_info_struct)(png_structp) = NULL;
static void (*dyn_png_destroy_write_struct)(png_structpp, png_infopp) = NULL;
static jmp_buf *(*dyn_png_set_longjmp_fn)(png_structrp, png_longjmp_ptr, size_t) = NULL;
static void (*dyn_png_init_io)(png_structp, FILE *) = NULL;
static void (*dyn_png_set_IHDR)(png_structp, png_infop, png_uint_32, png_uint_32, int, int, int, int, int) = NULL;
static void (*dyn_png_write_info)(png_structp, png_infop) = NULL;
static void (*dyn_png_write_image)(png_structp, png_bytepp) = NULL;
static void (*dyn_png_write_end)(png_structp, png_infop) = NULL;

#define FT_Init_FreeType dyn_FT_Init_FreeType
#define FT_New_Face dyn_FT_New_Face
#define FT_Set_Pixel_Sizes dyn_FT_Set_Pixel_Sizes
#define FT_Load_Char dyn_FT_Load_Char
#define FT_Done_Face dyn_FT_Done_Face
#define FT_Done_FreeType dyn_FT_Done_FreeType

#define png_create_write_struct dyn_png_create_write_struct
#define png_create_info_struct dyn_png_create_info_struct
#define png_destroy_write_struct dyn_png_destroy_write_struct
#define png_set_longjmp_fn dyn_png_set_longjmp_fn
#define png_init_io dyn_png_init_io
#define png_set_IHDR dyn_png_set_IHDR
#define png_write_info dyn_png_write_info
#define png_write_image dyn_png_write_image
#define png_write_end dyn_png_write_end

static int load_symbol(void **func_ptr, void *handle, const char *func_name)
{
    *func_ptr = dlsym(handle, func_name);
    if (*func_ptr == NULL) {
        printf("Error: cannot load function %s\n", func_name);
        return 1;
    }
    return 0;
}

int load_dynamic_libraries(void)
{
    zlib_handle = dlopen("zlib/libz.so", RTLD_LAZY);
    if (zlib_handle == NULL) {
        printf("Error: cannot open zlib/libz.so\n");
        return 1;
    }

    libpng_handle = dlopen("libpng/.libs/libpng16.so", RTLD_LAZY);
    if (libpng_handle == NULL) {
        printf("Error: cannot open libpng/.libs/libpng16.so\n");
        dlclose(zlib_handle);
        zlib_handle = NULL;
        return 1;
    }

    freetype_handle = dlopen("freetype/objs/.libs/libfreetype.so", RTLD_LAZY);
    if (freetype_handle == NULL) {
        printf("Error: cannot open freetype/objs/.libs/libfreetype.so\n");
        dlclose(libpng_handle);
        libpng_handle = NULL;
        dlclose(zlib_handle);
        zlib_handle = NULL;
        return 1;
    }

    if (load_symbol((void **)&dyn_FT_Init_FreeType, freetype_handle, "FT_Init_FreeType")) return 1;
    if (load_symbol((void **)&dyn_FT_New_Face, freetype_handle, "FT_New_Face")) return 1;
    if (load_symbol((void **)&dyn_FT_Set_Pixel_Sizes, freetype_handle, "FT_Set_Pixel_Sizes")) return 1;
    if (load_symbol((void **)&dyn_FT_Load_Char, freetype_handle, "FT_Load_Char")) return 1;
    if (load_symbol((void **)&dyn_FT_Done_Face, freetype_handle, "FT_Done_Face")) return 1;
    if (load_symbol((void **)&dyn_FT_Done_FreeType, freetype_handle, "FT_Done_FreeType")) return 1;

    if (load_symbol((void **)&dyn_png_create_write_struct, libpng_handle, "png_create_write_struct")) return 1;
    if (load_symbol((void **)&dyn_png_create_info_struct, libpng_handle, "png_create_info_struct")) return 1;
    if (load_symbol((void **)&dyn_png_destroy_write_struct, libpng_handle, "png_destroy_write_struct")) return 1;
    if (load_symbol((void **)&dyn_png_set_longjmp_fn, libpng_handle, "png_set_longjmp_fn")) return 1;
    if (load_symbol((void **)&dyn_png_init_io, libpng_handle, "png_init_io")) return 1;
    if (load_symbol((void **)&dyn_png_set_IHDR, libpng_handle, "png_set_IHDR")) return 1;
    if (load_symbol((void **)&dyn_png_write_info, libpng_handle, "png_write_info")) return 1;
    if (load_symbol((void **)&dyn_png_write_image, libpng_handle, "png_write_image")) return 1;
    if (load_symbol((void **)&dyn_png_write_end, libpng_handle, "png_write_end")) return 1;

    return 0;
}

void unload_dynamic_libraries(void)
{
    if (freetype_handle != NULL) {
        dlclose(freetype_handle);
        freetype_handle = NULL;
    }
    if (libpng_handle != NULL) {
        dlclose(libpng_handle);
        libpng_handle = NULL;
    }
    if (zlib_handle != NULL) {
        dlclose(zlib_handle);
        zlib_handle = NULL;
    }
}

#endif


static void fill_background_white(unsigned char *image, int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            image[index + 0] = 255;
            image[index + 1] = 255;
            image[index + 2] = 255;
        }
    }
}

static void draw_bitmap_to_image(unsigned char *image, int width, int height, FT_Bitmap *bitmap, int x_offset, int y_offset)
{
    for (int y = 0; y < (int)bitmap->rows; y++) {
        for (int x = 0; x < (int)bitmap->width; x++) {
            int image_x = x_offset + x;
            int image_y = y_offset + y;
            if (image_x < 0 || image_x >= width || image_y < 0 || image_y >= height) 
                continue;
            unsigned char value = bitmap->buffer[y * bitmap->pitch + x];
            int image_index = (image_y * width + image_x) * 3;
            image[image_index + 0] = (unsigned char)((255 * (255 - value)) / 255);
            image[image_index + 1] = (unsigned char)((255 * (255 - value)) / 255);
            image[image_index + 2] = (unsigned char)((255 * (255 - value)) / 255);
        }
    }
}

static int render_text(unsigned char *image, int width, int height, const char *font_path, const char *text)
{
    FT_Library library; // context for FreeType
    FT_Face face; // object for the certain font
    if (FT_Init_FreeType(&library)) {
        printf("Error: FT_Init_FreeType failed\n");
        return 1;
    }
    if (FT_New_Face(library, font_path, 0, &face)) { // open font file, read it anf cratr object FT_Face
        printf("Error: cannot load font file: %s\n", font_path);
        FT_Done_FreeType(library);
        return 1;
    }
    if (FT_Set_Pixel_Sizes(face, 0, FONT_SIZE)) { // set size of font in pixels
        printf("Error: FT_Set_Pixel_Sizes failed\n");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 1;
    }
    int pen_x = START_X;
    int pen_y = BASELINE_Y;

    for (int i = 0; text[i] != '\0'; i++) {
        if (FT_Load_Char(face, (unsigned char)text[i], FT_LOAD_RENDER)) { // find glif fot symbol, load it, render to bitmap 
            printf("Warning: cannot render symbol '%c'\n", text[i]); // face->glyph - pointer to object FT_GlyphSlot
            continue; // face->gylph->bitmap has: rows, width, pitch, buffer
        }
        draw_bitmap_to_image(image, width, height, &face->glyph->bitmap, pen_x + face->glyph->bitmap_left, pen_y - face->glyph->bitmap_top);
        pen_x += face->glyph->advance.x >> 6; // if frxed-point 26.6 so /64
        if (pen_x >= width - 20) 
            break;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return 0;
}

static int save_png(const char *filename, unsigned char *image,int width, int height)
{
    FILE *fp;
    png_structp png_ptr; // main obkect for libpng
    png_infop info_ptr; // object for info (parametrs of image, metadata, header info)
    png_bytep *row_pointers; // array of pointers on strings
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Error: cannot open output file: %s\n", filename);
        return 1;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        printf("Error: png_create_write_struct failed\n");
        fclose(fp);
        return 1;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        printf("Error: png_create_info_struct failed\n");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return 1;
    }
#if defined(BUILD_DYNAMIC)
    if (setjmp(*png_set_longjmp_fn(png_ptr, longjmp, sizeof(jmp_buf)))) {
#elif defined(BUILD_BLOB)
    if (_setjmp(*png_set_longjmp_fn(png_ptr, longjmp, sizeof(jmp_buf)))) {
#else
    if (setjmp(png_jmpbuf(png_ptr))) {
#endif
        printf("Error: libpng write error\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }
    png_init_io(png_ptr, fp); // connect libpng object with open file
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // set image parameters
    png_write_info(png_ptr, info_ptr); // write header info
    row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    if (row_pointers == NULL) {
        printf("Error: memory allocation failed\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }
    for (int y = 0; y < height; y++) 
        row_pointers[y] = image + y * width * 3;
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);
    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

int create_png_with_text(const char *font_path, const char *output_png_path,const char *text)
{
#ifdef BUILD_BLOB
    if (blob_fopen_ptr == NULL || blob_FT_Init_FreeType_ptr == NULL || blob_png_create_write_struct_ptr == NULL) {
        printf("Error: blob function table is not initialized\n");
        return 1;
    }
#endif
    unsigned char *image;
    int image_size;
    int result;

    if (font_path == NULL || output_png_path == NULL || text == NULL) {
        printf("Error: invalid arguments\n");
        return 1;
    }

    image_size = IMAGE_WIDTH * IMAGE_HEIGHT * 3;

    image = (unsigned char *)malloc(image_size);
    if (image == NULL) {
        printf("Error: cannot allocate memory for image\n");
        return 1;
    }
    fill_background_white(image, IMAGE_WIDTH, IMAGE_HEIGHT);
    result = render_text(image, IMAGE_WIDTH, IMAGE_HEIGHT, font_path, text);
    if (result != 0) {
        free(image);
        return 1;
    }
    result = save_png(output_png_path, image, IMAGE_WIDTH, IMAGE_HEIGHT);
    if (result != 0) {
        free(image);
        return 1;
    }
    free(image);
    return 0;
}