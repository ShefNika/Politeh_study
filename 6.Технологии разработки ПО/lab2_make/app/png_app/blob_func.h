#ifndef BLOB_FUNC_H
#define BLOB_FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>

#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

enum {
    BLOB_CLOCK = 0,
    BLOB_FOPEN,
    BLOB_FCLOSE,
    BLOB_FREAD,
    BLOB_FWRITE,
    BLOB_MALLOC,
    BLOB_FREE,
    BLOB_REALLOC,
    BLOB_MEMSET,
    BLOB_MEMCPY,
    BLOB_PRINTF,
    BLOB_FPRINTF,

    BLOB__SETJMP,
    BLOB_LONGJMP,
    BLOB___LONGJMP_CHK,
    
    BLOB_FT_Init_FreeType,
    BLOB_FT_New_Face,
    BLOB_FT_Set_Pixel_Sizes,
    BLOB_FT_Load_Char,
    BLOB_FT_Done_Face,
    BLOB_FT_Done_FreeType,

    BLOB_png_create_write_struct,
    BLOB_png_create_info_struct,
    BLOB_png_destroy_write_struct,
    BLOB_png_set_longjmp_fn,
    BLOB_png_init_io,
    BLOB_png_set_IHDR,
    BLOB_png_write_info,
    BLOB_png_write_image,
    BLOB_png_write_end
};

typedef struct {
    const char *font_path;
    const char *output_png_path;
    const char *text;
    void **functions;
} blob_ctx;

typedef int (*blob_entry_t)(blob_ctx *ctx);

#ifdef BUILD_BLOB

extern clock_t (*blob_clock_ptr)(void);
extern FILE *(*blob_fopen_ptr)(const char *, const char *);
extern int (*blob_fclose_ptr)(FILE *);
extern size_t (*blob_fread_ptr)(void *, size_t, size_t, FILE *);
extern size_t (*blob_fwrite_ptr)(const void *, size_t, size_t, FILE *);
extern void *(*blob_malloc_ptr)(size_t);
extern void (*blob_free_ptr)(void *);
extern void *(*blob_realloc_ptr)(void *, size_t);
extern void *(*blob_memset_ptr)(void *, int, size_t);
extern void *(*blob_memcpy_ptr)(void *, const void *, size_t);
extern int (*blob_printf_ptr)(const char *, ...);
extern int (*blob_fprintf_ptr)(FILE *, const char *, ...);
extern int (*blob__setjmp_ptr)(jmp_buf env);
extern void (*blob_longjmp_ptr)(jmp_buf env, int val);
extern void (*blob___longjmp_chk_ptr)(jmp_buf env, int val);

int _setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
void __longjmp_chk(jmp_buf env, int val);

extern FT_Error (*blob_FT_Init_FreeType_ptr)(FT_Library *);
extern FT_Error (*blob_FT_New_Face_ptr)(FT_Library, const char *, FT_Long, FT_Face *);
extern FT_Error (*blob_FT_Set_Pixel_Sizes_ptr)(FT_Face, FT_UInt, FT_UInt);
extern FT_Error (*blob_FT_Load_Char_ptr)(FT_Face, FT_ULong, FT_Int32);
extern FT_Error (*blob_FT_Done_Face_ptr)(FT_Face);
extern FT_Error (*blob_FT_Done_FreeType_ptr)(FT_Library);

extern png_structp (*blob_png_create_write_struct_ptr)(png_const_charp, png_voidp, png_error_ptr, png_error_ptr);
extern png_infop (*blob_png_create_info_struct_ptr)(png_structp);
extern void (*blob_png_destroy_write_struct_ptr)(png_structpp, png_infopp);
extern jmp_buf *(*blob_png_set_longjmp_fn_ptr)(png_structrp, png_longjmp_ptr, size_t);
extern void (*blob_png_init_io_ptr)(png_structp, FILE *);
extern void (*blob_png_set_IHDR_ptr)(png_structp, png_infop, png_uint_32, png_uint_32, int, int, int, int, int);
extern void (*blob_png_write_info_ptr)(png_structp, png_infop);
extern void (*blob_png_write_image_ptr)(png_structp, png_bytepp);
extern void (*blob_png_write_end_ptr)(png_structp, png_infop);

void set_blob_functions(void **functions);

#define clock   blob_clock_ptr
#define fopen   blob_fopen_ptr
#define fclose  blob_fclose_ptr
#define fread   blob_fread_ptr
#define fwrite  blob_fwrite_ptr
#define malloc  blob_malloc_ptr
#define free    blob_free_ptr
#define realloc blob_realloc_ptr
#define memset  blob_memset_ptr
#define memcpy  blob_memcpy_ptr
#define printf  blob_printf_ptr
#define fprintf blob_fprintf_ptr

#define FT_Init_FreeType   blob_FT_Init_FreeType_ptr
#define FT_New_Face        blob_FT_New_Face_ptr
#define FT_Set_Pixel_Sizes blob_FT_Set_Pixel_Sizes_ptr
#define FT_Load_Char       blob_FT_Load_Char_ptr
#define FT_Done_Face       blob_FT_Done_Face_ptr
#define FT_Done_FreeType   blob_FT_Done_FreeType_ptr

#define png_create_write_struct  blob_png_create_write_struct_ptr
#define png_create_info_struct   blob_png_create_info_struct_ptr
#define png_destroy_write_struct blob_png_destroy_write_struct_ptr
#define png_set_longjmp_fn       blob_png_set_longjmp_fn_ptr
#define png_init_io              blob_png_init_io_ptr
#define png_set_IHDR             blob_png_set_IHDR_ptr
#define png_write_info           blob_png_write_info_ptr
#define png_write_image          blob_png_write_image_ptr
#define png_write_end            blob_png_write_end_ptr

#endif

#endif