#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void print_help(const char* program_name)
{
    printf("Help:\n");
    printf("  %s write <filename> <text>\n", program_name);
    printf("  %s read <filename>\n", program_name);
}

int write_to_file(const char* filename, const char* text)
{
    FILE* file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error: cannot open file for writing: %s\n", filename);
        return 1;
    }
    if (fprintf(file, "%s", text) < 0)
    {
        printf("Error: cannot write to file: %s\n", filename);
        fclose(file);
        return 1;
    }
    fclose(file);
    printf("Text was written to file: %s\n", filename);
    return 0;
}

int read_from_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    char buffer[BUFFER_SIZE];
    if (file == NULL)
    {
        printf("Error: cannot open file for reading: %s\n", filename);
        return 1;
    }
    printf("File contents:\n");
    while (fgets(buffer, sizeof(buffer), file) != NULL)
        printf("%s", buffer);
    fclose(file);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        print_help(argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "write") == 0)
    {
        if (argc < 4)
        {
            print_help(argv[0]);
            return 1;
        }
        return write_to_file(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "read") == 0)
        return read_from_file(argv[2]);
    else
    {
        print_help(argv[0]);
        return 1;
    }
}