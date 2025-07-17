#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int decoder() {
	FILE* answer, * final;
	answer = fopen("answer.bmp", "rb");
	final = fopen("final.txt", "wb");
	fseek(answer, 49, SEEK_SET);
	int deg = fgetc(answer);
    int size_file = 0, imgbit = 0, f = 0;
    for (int i = 3; i >= 0; i--) {
        fread(&imgbit, 1, 1, answer);
        f = (unsigned int)imgbit;
        f = f << (8 * i);
        size_file = size_file | f;
    }
    char img_mask = ~(255 << deg);
    for (unsigned int j = 0; j < size_file; j++) {
        char ch = 0;
        for (int i = 8 / deg - 1; i >= 0; i--) {
            if (!feof(answer)) {
                fread(&imgbit, 1, 1, answer);
                imgbit = imgbit & img_mask;
                imgbit = imgbit << deg * i;
                ch = ch | imgbit;
            }
        }
        fwrite(&ch, 1, 1, final);
    }
    fclose(final);
    fclose(answer);
	return 0;
}

int main() {
	setlocale(LC_ALL, "Rus");
	decoder();
	return 0;
}

