#include <stdio.h>
#include <time.h>
#include "png_text.h"

#ifdef BUILD_BLOB
#include "blob_func.h"
#endif

#ifdef BUILD_BLOB
int main(blob_ctx *ctx)
#else
int main(int argc, char *argv[])
#endif
{
    const char *font_path;
    const char *output_png_path;
    const char *text;
    int result;
    clock_t load_start_time;
    clock_t work_start_time;
    clock_t finish_time;
    double load_time;
    double total_time;
    

#ifdef BUILD_BLOB
    if (ctx == NULL || ctx->functions == NULL) 
        return 1;
    set_blob_functions(ctx->functions);
#endif
    load_start_time = clock();

#ifdef BUILD_BLOB
    if (ctx == NULL ||
        ctx->font_path == NULL ||
        ctx->output_png_path == NULL ||
        ctx->text == NULL) {
        printf("Error: invalid blob params\n");
        return 1;
    }

    font_path = ctx->font_path;
    output_png_path = ctx->output_png_path;
    text = ctx->text;
#else
    if (argc < 4) {
        printf("Usage: %s <font.ttf> <output.png> <text>\n", argv[0]);
        return 1;
    }

    font_path = argv[1];
    output_png_path = argv[2];
    text = argv[3];
#endif
    

#ifdef BUILD_DYNAMIC
    if (load_dynamic_libraries() != 0) {
        printf("Error: failed to load dynamic libraries\n");
        return 1;
    }
#endif
    work_start_time = clock();
    result = create_png_with_text(font_path, output_png_path, text);
    finish_time = clock();
    load_time = (double)(work_start_time - load_start_time) / CLOCKS_PER_SEC;
    total_time = (double)(finish_time - load_start_time) / CLOCKS_PER_SEC;

    if (result != 0) {
        printf("Error: failed to create PNG file\n");
        printf("System load time: %.6f seconds\n", load_time);
        printf("Total execution time: %.6f seconds\n", total_time);
#ifdef BUILD_DYNAMIC
        unload_dynamic_libraries();
#endif
        return result;
    }
    printf("PNG file created successfully: %s\n", output_png_path);
    printf("System load time: %.6f seconds\n", load_time);
    printf("Total execution time: %.6f seconds\n", total_time);

#ifdef BUILD_DYNAMIC
    unload_dynamic_libraries();
#endif

    return 0;
}