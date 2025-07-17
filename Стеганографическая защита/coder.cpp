#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>



unsigned int fileSize(FILE* file) {
	fseek(file, 0, SEEK_END);
	int s = ftell(file);
	fseek(file, 0, SEEK_SET);
	return(s);
}

int coder() {
	int deg;
	printf("Введите степень упаковки вашего сообщения: ");
	scanf("%d", &deg);
	FILE* in = fopen("sky.bmp", "rb"); 
	FILE* out = fopen("answer.bmp", "wb+"); 
	FILE* in_text = fopen("code.txt", "r");
	int text_size = fileSize(in_text);
	int pick_size = fileSize(in);
	int maxsize = (pick_size * (unsigned int)deg / 8 - 54);
	if (text_size > maxsize)  printf("Размеры вашего текста не позволяют полностью поместить текст в стегоконтейнер!\n");
	else {
		char ch = 0;
		for (int i = 0; i < 49; i++) {
			fread(&ch, 1, 1, in);
			fwrite(&ch, 1, 1, out);
		}
		fread(&ch, 1, 1, in);
		fwrite(&deg, 1, 1, out);
		int size;
		for (int i = 3; i >= 0; i--) {
			size = text_size;
			size = size >> (8 * i);
			fread(&ch, 1, 1, in);
			fwrite(&size, 1, 1, out);
		}
		char bit, newbit, imgbit = 255;
		unsigned char img_mask = 255 << deg;
		unsigned char txt_mask = ~(255 << deg);
		while (!feof(in_text)) {
			fread(&bit, 1, 1, in_text);
			if (!feof(in_text)) {
				for (int i = 8 / deg - 1; i >= 0; i--) {
					newbit = bit;
					newbit = newbit >> deg * i;
					newbit = newbit & txt_mask;
					fread(&imgbit, 1, 1, in);
					imgbit = imgbit & img_mask;
					imgbit = imgbit | newbit;
					if (!feof(in_text)) 
						fwrite(&imgbit, 1, 1, out);
				}
			}
		}
		while (!feof(in)) {
			fread(&imgbit, 1, 1, in);
			if (!feof(in))
				fwrite(&imgbit, 1, 1, out);
		}
	}

	fclose(in_text);
	fclose(in);
	fclose(out);
	return 0;
}
	
int main() {
	setlocale(LC_ALL, "Rus");
	coder();
	return 0;
}
