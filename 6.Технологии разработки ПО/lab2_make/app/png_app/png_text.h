#ifndef PNG_TEXT_H
#define PNG_TEXT_H

int create_png_with_text(const char *font_path, const char *output_png_path, const char *text);

#ifdef BUILD_DYNAMIC
int load_dynamic_libraries(void);
void unload_dynamic_libraries(void);
#endif

#ifdef BUILD_BLOB
void set_blob_functions(void **functions);
#endif

#endif