#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <setjmp.h>

#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "elfload.h"
#include "../app/png_app/blob_func.h"

extern int _setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);
extern void __longjmp_chk(jmp_buf env, int val);

FILE *f;
void *buf;

typedef int (*entrypoint_t)(blob_ctx *ctx);

static bool fpread(el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
    (void) ctx;

    if (fseek(f, offset, SEEK_SET))
        return false;

    if (fread(dest, nb, 1, f) != 1)
        return false;

    return true;
}

static void *alloccb(el_ctx *ctx, Elf_Addr phys, Elf_Addr virt, Elf_Addr size)
{
    (void) ctx;
    (void) phys;
    (void) size;
    return (void *) virt;
}

static void check(el_status stat, const char *expln)
{
    if (stat) {
        fprintf(stderr, "%s: error %d\n", expln, stat);
        exit(1);
    }
}

static void fill_blob_functions(void **functions)
{
    functions[BLOB_CLOCK] = clock;
    functions[BLOB_FOPEN] = fopen;
    functions[BLOB_FCLOSE] = fclose;
    functions[BLOB_FREAD] = fread;
    functions[BLOB_FWRITE] = fwrite;
    functions[BLOB_MALLOC] = malloc;
    functions[BLOB_FREE] = free;
    functions[BLOB_REALLOC] = realloc;
    functions[BLOB_MEMSET] = memset;
    functions[BLOB_MEMCPY] = memcpy;
    functions[BLOB_PRINTF] = printf;
    functions[BLOB_FPRINTF] = fprintf;
    functions[BLOB__SETJMP] = _setjmp;
    functions[BLOB_LONGJMP] = longjmp;
    functions[BLOB___LONGJMP_CHK] = __longjmp_chk;

    functions[BLOB_FT_Init_FreeType] = FT_Init_FreeType;
    functions[BLOB_FT_New_Face] = FT_New_Face;
    functions[BLOB_FT_Set_Pixel_Sizes] = FT_Set_Pixel_Sizes;
    functions[BLOB_FT_Load_Char] = FT_Load_Char;
    functions[BLOB_FT_Done_Face] = FT_Done_Face;
    functions[BLOB_FT_Done_FreeType] = FT_Done_FreeType;

    functions[BLOB_png_create_write_struct] = png_create_write_struct;
    functions[BLOB_png_create_info_struct] = png_create_info_struct;
    functions[BLOB_png_destroy_write_struct] = png_destroy_write_struct;
    functions[BLOB_png_set_longjmp_fn] = png_set_longjmp_fn;
    functions[BLOB_png_init_io] = png_init_io;
    functions[BLOB_png_set_IHDR] = png_set_IHDR;
    functions[BLOB_png_write_info] = png_write_info;
    functions[BLOB_png_write_image] = png_write_image;
    functions[BLOB_png_write_end] = png_write_end;
}

int main(int argc, char **argv)
{
    el_ctx ctx;
    uintptr_t epaddr;
    entrypoint_t ep;
    void *functions[64] = {0};
    int result;

    if (argc < 5) {
        fprintf(stderr, "usage: %s [blob-file] [font.ttf] [output.png] [text]\n", argv[0]);
        return 1;
    }

    f = fopen(argv[1], "rb"); // open blob file
    if (!f) {
        perror("opening file");
        return 1;
    }

    ctx.pread = fpread;

    check(el_init(&ctx), "initialising"); // init the context

    if (posix_memalign(&buf, ctx.align, ctx.memsz)) { // allocate memory for blob and give rights
        perror("memalign");
        return 1;
    }

    if (mprotect(buf, ctx.memsz, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        perror("mprotect");
        return 1;
    }

    ctx.base_load_vaddr = ctx.base_load_paddr = (uintptr_t) buf;

    check(el_load(&ctx, alloccb), "loading"); // load segments
    check(el_relocate(&ctx), "relocating"); // relocate segments

    epaddr = ctx.ehdr.e_entry + (uintptr_t) buf;
    ep = (entrypoint_t) epaddr; // get address of entrypoint

    fill_blob_functions(functions);
    printf("Binary entrypoint is %" PRIxPTR "; invoking %p\n",(uintptr_t) ctx.ehdr.e_entry, ep);
    
    blob_ctx ctx_blob;
    ctx_blob.font_path = argv[2];
    ctx_blob.output_png_path = argv[3];
    ctx_blob.text = argv[4];
    ctx_blob.functions = functions;
    result = ep(&ctx_blob);

    fclose(f);
    free(buf);

    return result;
}