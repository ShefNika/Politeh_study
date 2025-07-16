#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>



void encryption_key_xor(char key[]) {
    FILE* in = fopen("source_text_500.txt", "rb");
    FILE* out = fopen("encrypted.txt", "wb");
    if (in == NULL) {
        printf("in == NULL\n");
        return;
    }
    char* pKey = key;
    while (!feof(in)) {
        char tmp = fgetc(in);
        if (!*pKey) pKey = key;
        if (tmp != ' ' && tmp != '\n' && tmp != ',' && tmp != '.' && tmp != '!' && tmp != '?' && tmp != ':' && tmp != ';' && tmp != '\'' && tmp != '\"' && tmp != '-') {
            fputc(*pKey ^ tmp, out);
            pKey++;
        }
    }
    fclose(in);
    fclose(out);
    FILE* key_out = fopen("key.txt", "wb");
    for (int i = 0; i < strlen(key); i++)
        fputc(key[i], key_out);
    fclose(key_out);
}

int main()
{
    char key[1000];
    printf("Enter the encryption key:\n");
    fgets(key, 1000, stdin);
    key[strlen(key) - 1] = '\0';
    encryption_key_xor(key);
}
