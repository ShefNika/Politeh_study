#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


int toUpper(int a) {
    char up[] = "ETAOINSHRDLCUMWFGYPBVKXJQZ\0";
    char lw[] = "etaoinshrflcumwfgypbvkxjqz\0";
    int i = 0;
    while (lw[i++] != a);
    return up[--i];
}

void get_key(char* key, char text[], long long text_size, int key_length) {

    // По убыванию самые часто встречающиеся символы в англ алфавите
    char common[] = "ETAOIN SHRDLU\0";
    char dopusk[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm\0";
    char Alphabet[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm\0";

    for (int i = 0; i < key_length; i++) {

        // Создаем массив из групп символов, закодированных одинаковым байтом
        int* block = (int*)calloc(text_size / key_length + 1, sizeof(int));
        int block_size = 0;
        for (int j = i; j < text_size; j += key_length)
            block[block_size++] = text[j];


        int current_high_score_common = 0;
        char current_key_char = '\0';
        int current_high_score_dopusk = 0;

        for (int j = 0; j < 127; j++) {

            //Проверяем, чтобы j был из алфавита
            char* pAlph = Alphabet;
            bool proverka_j = true;
            while (*pAlph) {
                if (*pAlph++ == j) {
                    proverka_j = false;
                    break;
                }
            }
            if (proverka_j) continue;


            // Создаем массив для всех XOR'd
            int* x = (int*)calloc(block_size, sizeof(int));
            for (int k = 0; k < block_size; k++) {
                int tmp = j ^ block[k];
                x[k] = tmp;
            }

            // Увеличиваем кол-во очков везде, где есть совпадения
            int score_common = 0;
            for (int k = 0; k < 12; k++) {
                for (int l = 0; l < block_size; l++) {
                    bool pr = false;
                    char* alph = Alphabet;
                    while (*alph) {
                        if (*alph == x[l])
                            pr = true;
                        alph++;
                    }
                    if (pr && common[k] == toUpper(x[l]))
                        score_common++;
                }
            }

            // Увеличиваем кол-во очков, если это нормальный символ
            char* pDopusk = dopusk;
            int score_dopusk = 0;
            while (*pDopusk) {
                for (int k = 0; k < block_size; k++) {
                    if (*pDopusk == x[k])
                        score_dopusk++;
                }
                pDopusk++;
            }
            free(x);

            // Если очков больше, записываем в high_score
            if ((score_dopusk > current_high_score_dopusk) || (score_common > current_high_score_common && score_dopusk == current_high_score_dopusk)) {
                current_high_score_dopusk = score_dopusk;
                current_high_score_common = score_common;
                current_key_char = j;
            }


        }
        free(block);
        *key = current_key_char;
        key++;
    }
    *key = '\0';
}

// Функция для нахождения НОД (наибольшего общего делителя) двух чисел
unsigned int gcd(unsigned int a, unsigned int b) {
    while (b != 0) {
        unsigned int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Функция для нахождения НОД множества чисел
unsigned int gcd_multiple(const unsigned int* arr, unsigned int n) {
    if (n == 0) return 0;
    unsigned int result = arr[0];
    for (unsigned int i = 1; i < n; i++) {
        result = gcd(result, arr[i]);
    }
    return result;
}


unsigned int determine_key_length(const char* encrypted_text, unsigned long long size) {
    unsigned int* valid_key_lengths = (unsigned int*)malloc(size * sizeof(unsigned int));
    unsigned int valid_key_count = 0;

    for (unsigned int i = 1; i < size / 3; i++) {
        unsigned int match_count = 0;

        // Циклический сдвиг шифротекста на i символов и подсчет совпадений
        for (unsigned long long j = 0; j < size; j++) {
            if (encrypted_text[j] == encrypted_text[(j + i) % size]) {
                match_count++;
            }
        }

        // Вычисление процента совпадений
        double match_percentage = (double)match_count / size;
        //printf("%d %f\n", i, match_percentage);
            //if (size <= 150) {
          //    if (match_percentage > 0.07 && i != 1) {
          //      printf("%d %f\n", i, match_percentage);
          //      valid_key_lengths[valid_key_count++] = i;
          //    }
          //  }
          //  else {
            if (match_percentage > 0.06 && i != 1) {
                //      printf("%d %f\n", i, match_percentage);
                valid_key_lengths[valid_key_count++] = i;
            }
        //  }

    }

    unsigned int key_length = gcd_multiple(valid_key_lengths, valid_key_count);
    free(valid_key_lengths);

    return key_length;
}

void decrypt(char text[], long long text_size, char key[]) {
    FILE* out = fopen("decrypted.txt", "wb");
    char* pKey = key;
    for (int i = 0; i < text_size; i++) {
        fputc(text[i] ^ *pKey, out);
        pKey++;
        if (*pKey == '\0') pKey = key;
    }
    fclose(out);
}

void percent_of_key_lenght(char* key, int key_lenght) {
    FILE* true_key = fopen("key.txt", "rb");
    if (true_key == NULL) {
        printf("No File for true_key\n");

        return;
    }
    int count_lenght = 0, count_key = 0;
    char tmp;
    while ((tmp = fgetc(true_key)) != EOF) {
        count_lenght++;
        if (*key++ == tmp) count_key++;
    }
    printf("%d key lenght\n%d key matched count\n", count_lenght, count_key);
    fclose(true_key);
}

int main() {
    // Чтение шифротекста из файла
    FILE* in = fopen("encrypted.txt", "rb");
    if (in == NULL) {
        printf("Error: Could not open file\n");
        return 1;
    }
    // Определение размера файла
    fseek(in, 0, SEEK_END);
    unsigned long long size = ftell(in);
    fseek(in, 0, SEEK_SET);

    char* encrypted_text = (char*)malloc(size * sizeof(char));
    if (encrypted_text == NULL) {
        printf("Error: Could not allocate memory.\n");
        fclose(in);
        return 1;
    }
    fread(encrypted_text, sizeof(char), size, in);
    fclose(in);

    //Считаем длинну ключа
    unsigned int key_length = determine_key_length(encrypted_text, size);
    printf("Determined key length: %u\n", key_length);


    // Узнаем ключ
    char* key = (char*)malloc(key_length * sizeof(char));
    get_key(key, encrypted_text, size, key_length);
    for (int i = 0; i < key_length; i++) printf("%c", key[i]);
    printf("\n");

    // Расшифровываем с указанным ключом key
    decrypt(encrypted_text, size, key);

    //Считаем процент совпадений с настоящим ключём
    percent_of_key_lenght(key, key_length);

    free(encrypted_text);
    return 0;
}
