#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

 
int repc = 0;

void decrypted_sort(char* code, int all, char* freq) {
    char words[10000][30];
    int i = 0, j = 0, k = 0;
    int decr[10000] = { 0 };
    int length[10000] = { 0 };
    while (code[i] != '\0') {
        if (((code[i] > 'Я' || code[i] < 'А') && code[i] != 'Ё') && ((code[i] > 'я' || code[i] < 'а') && code[i] != 'ё')) {
                words[j][k] = '\0';
                length[j] = k;
                j++;
                k = 0;
            
        }
        else {
            words[j][k] = code[i];
            k++;
        }
        i++;
    }
    words[j][k] = '\0';
    for (int i = 0; i <= j; i++) {
        int p=0;
        while (words[i][p] != '\0') {
            for (int l = 0; l < 33; l++)
                if (words[i][p] == freq[l])
                    decr[i]++;
            p++;
        }
    }
    for (i = 0; i < j - 1; i++) {
        for (int h = 0; h < j - i - 1; h++) {
            if (decr[h] < decr[h + 1]) {
                int temp = decr[h];
                decr[h] = decr[h + 1];
                decr[h + 1] = temp;
                int t = length[h];
                length[h] = length[h + 1];
                length[h + 1] = t;
                char tempWord[1000];
                strcpy(tempWord, words[h]);
                strcpy(words[h], words[h + 1]);
                strcpy(words[h + 1], tempWord);
            }
        }
    }
    printf("Слова, отсортированные по убыванию количества расшифрованных букв в них:\n");
    for (i = 0; i < j; i++) 
        if ((words[i][0] != '\0')) {
            printf("%s\n", words[i]);
        }
    int v;
    printf("Отсортировать также по длине слов? Введите 1, если \"да\" или 0, если \"нет\"\n");
    scanf_s("%d", &v);
    if (v == 1) {
        for (i = 0; i < j - 1; i++) {
            for (int h = 0; h < j - i - 1; h++) {
                if (length[h] < length[h + 1]) {
                    int temp = decr[h];
                    decr[h] = decr[h + 1];
                    decr[h + 1] = temp;
                    int t = length[h];
                    length[h] = length[h + 1];
                    length[h + 1] = t;
                    char tempWord[1000];
                    strcpy(tempWord, words[h]);
                    strcpy(words[h], words[h + 1]);
                    strcpy(words[h + 1], tempWord);
                }
            }
        }
        for (i = 0; i < j; i++)
            if ((words[i][0] != '\0')) {
                printf("%s\n", words[i]);
            }
    }
}

void sort_words(char* code, int all) {
    char words[10000][30];
    int i = 0, j = 0, k = 0;
    int length[10000] = { 0 };
    while (code[i] != '\0') {
        if (((code[i] > 'Я' || code[i] < 'А') && code[i] != 'Ё') && ((code[i] > 'я' || code[i] < 'а') && code[i] != 'ё')) {
            words[j][k] = '\0';
            length[j] = k;
            j++;
            k = 0;

        }
        else {
            words[j][k] = code[i];
            k++;
        }
        i++;
    }
    words[j][k] = '\0';
    for (i = 0; i < j - 1; i++) {
        for (int h = 0; h < j - i - 1; h++) {
            if (length[h] < length[h + 1]) {
                int t = length[h];
                length[h] = length[h + 1];
                length[h + 1] = t;
                char tempWord[1000];
                strcpy(tempWord, words[h]);
                strcpy(words[h], words[h + 1]);
                strcpy(words[h + 1], tempWord);
            }
        }
    }
    printf("Слова, отсортированные по убыванию количества букв в них:\n");
    for (i = 0; i < j; i++)
        if ((words[i][0] != '\0')) {
            printf("%s\n", words[i]);
        }
} 

void frequency_analysis(char* alph, char* freq) {
    FILE* in = fopen("text1.txt", "r");
    float frequency[33] = { 0 };
    char c;
    int k = 0;
    while (!feof(in)) {
        c = fgetc(in);
        for (int i = 0; i < 33; i++) {
            if (c == alph[i]) {
                frequency[i]++;
                k++;
           }
            
        }
    }
    for (int i = 0; i < 33; i++)
        frequency[i] = frequency[i] / k;
    for (int j = 0; j < 33; j++) {
        for (int i = 0; i < 33 - j; i++) {
            if (frequency[i + 1] > frequency[i]) {
                float t = frequency[i];
                frequency[i] = frequency[i + 1];
                frequency[i + 1] = t;
                c = alph[i];
                alph[i] = alph[i + 1];
                alph[i + 1] = c;
            }
        }
    }
    for (int i = 0; i < 33; i++) {
        printf("%c ", alph[i]);
        printf("%f ", frequency[i]);
        printf("%c\n", freq[i]);
    }
    fclose(in);
}

void autoreplacement(char* code, int all, char* alph, char* freq, char* that, char* what, char* answ) {
    FILE* in = fopen("text1.txt", "r");
    float frequency[33] = { 0 };
    char c;
    int k = 0;
    int b;
    printf("Сколько самых часто встречающихся букв заменить?\n");
    scanf_s("%d", &b);
    while (!feof(in)) {
        c = fgetc(in);
        for (int i = 0; i < 33; i++) {
            if (c == alph[i]) {
                frequency[i]++;
                k++;
            }

        }
    }
    for (int i = 0; i < 33; i++)
        frequency[i] = frequency[i] / k;
    for (int j = 0; j < 33; j++) {
        for (int i = 0; i < 33 - j; i++) {
            if (frequency[i + 1] > frequency[i]) {
                float t = frequency[i];
                frequency[i] = frequency[i + 1];
                frequency[i + 1] = t;
                c = alph[i];
                alph[i] = alph[i + 1];
                alph[i + 1] = c;
            }
        }
    }
    int kolvo = 0;
    for (int i = 0; i < 33; i++) 
        if (answ[i] == alph[i])
            kolvo++;
    float percent = float(kolvo) / 33;
    for (int i = 0; i < all; i++) {
        for (int j = 0; j < b; j++) {
            if (code[i] == alph[j]) {
                code[i] = freq[j];
                break;
            }

        }
    }
    for (int i = 0; i < b; i++) {
        that[repc] = alph[i];
        what[repc] = freq[i];
        repc++;
    }
    if (b==33)
        printf("Правильно расшифровано %.1f%% символов", percent * 100);
    fclose(in);
    
}

void replacement(char* code, int all, char* that, char* what) {
    char thatc, whatc;
    printf("Какую букву заменить? Если вы уже меняли эту букву, то введите строчную букву, в противном случае введите заглавную\n");
    getchar();
    thatc = getchar();
    printf("На какую букву заменить? Введите строчную букву\n");
    getchar();
    whatc = getchar();
    for (int i = 0; i < all; i++)
        if (code[i] == thatc)
            code[i] = whatc;
    that[repc] = thatc;
    what[repc] = whatc;
    repc++;
}

void history_of_replacements(char* that, char* what) {
    for (int i = 0; i < repc; i++) 
        printf("%c -> %c\n", that[i], what[i]);
}

void rollback(char* code, int all, char* that, char* what) {
    int b;
    printf("На какое количество ходов сделать откат?\n");
    scanf_s("%d", &b);
    for (int j = 1; j < b + 1; j++) {
        for (int i = 0; i < all; i++) {
            if (code[i] == what[repc - j])
                code[i] = that[repc - j];
        }
    }
    for (int j = 1; j < b + 1; j++) {
        that[repc] = what[repc - 2*j+1];
        what[repc] = that[repc - 2*j+1];
        repc++;
    }
}

void to_file(char* code, int all) {
    FILE* out = fopen("decrypted.txt", "w");
    for (int i = 0; i < all; i++)
        fprintf(out, "%c", code[i]);
    fclose(out);
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
    FILE* in = fopen("text1.txt", "r");
    char alph[] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    char freq[] = "оеаинтсрвлкмдпуяыьгзбчйхжшюцщэфъё";
    char that[1000] = { 0 };
    char what[1000] = { 0 };
    int i = 0;
    char answ[] = "ЖФЙЕЫЛАДХУНЭЗВПЁСТЦШЪЯГРКБМЮЧИОЬЩ";
    char* code = (char*)malloc(sizeof(char));
    while (!feof(in)) {
       code = (char*)realloc(code, (i + 1) * sizeof(char));
       code[i] = fgetc(in);
       i++;
    }
   code[i - 1] = '\0';
   int all = i - 1;

    while (true) {
        int a;
        printf("Выберите операцию:\n 1.Анализ частоты\n 2.Показать криптограмму\n 3.Группировка по количеству букв\n 4.Группировка по количеству нерасшифрованных букв\n 5.Замена букв\n 6.Автозамена\n 7.Откат ходов\n 8.История замены букв\n 9.Записать криптограмму в файл\n 10.Выход\n");
        scanf_s("%d", &a);
        if (a == 1)
            frequency_analysis(alph, freq);
        if (a== 2)
            printf("%s", code);
        if (a == 3)
            sort_words(code, all);
        if (a == 4)
            decrypted_sort(code, all, freq);
        if (a == 5)
            replacement(code, all, that, what);
        if (a == 6) 
            autoreplacement(code, all, alph, freq, that, what, answ);
        if (a == 7)
            rollback(code, all, that, what);
        if (a == 8)
            history_of_replacements(that, what);
        if (a == 9)  
            to_file(code, all);
        if (a == 10)
            break;
        printf("\n");
    } 
    fclose(in);
    return 0;
    
}
