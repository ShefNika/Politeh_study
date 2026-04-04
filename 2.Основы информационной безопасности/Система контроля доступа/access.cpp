#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h> 
#include <Windows.h> 
#include <clocale>

int remove_file_attribute(const char* filename, DWORD attribute) {
    DWORD currentAttributes = GetFileAttributesA(filename);
    if (currentAttributes == INVALID_FILE_ATTRIBUTES) {
        printf("Ошибка получения\n");
        return 0;
    }
    // Проверяем, установлен ли атрибут, который мы хотим удалить
    if (currentAttributes & attribute) {
        currentAttributes &= ~attribute; // Удаляем атрибут

        // Применяем измененные атрибуты
        if (SetFileAttributesA(filename, currentAttributes)) {
            printf("Атрибут файла удален\n");
            return 1;
        }
        else {
            printf("Ошибка удаления\n");
            return 0;
        }
    }
    else {
        printf("Атрибут не установлен, удаление не требуется\n");
        return 1;
    }
}


void set_file_attributes(const char* filename, DWORD attributes) {
    DWORD currentAttributes = GetFileAttributesA(filename);
    if (currentAttributes == INVALID_FILE_ATTRIBUTES) {
        printf("Ошибка получения\n");
        return;
    }

    // Добавляем новые атрибуты
    int result = SetFileAttributesA(filename, currentAttributes | attributes);
    if (result) 
        printf("Атрибуты файла изменены\n");
    else 
        printf("Ошибка установки\n");
    printf("\n");
}


void print_file_attributes(const char* filename) {
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &file_info)) {
        printf("Размер файла: %lld байт\n", (long long)file_info.nFileSizeLow);
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) 
            printf("Скрытый файл\n");
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) 
            printf("Архивный файл\n");
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) 
            printf("Файл только для чтения\n");
        else 
            printf("Файл для записи и чтения\n");
    }
    else {
        printf("Ошибка получения\n");
    }
}


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
    const char* filename = "C:/test_folder/text.txt";
    wchar_t File[] = TEXT("C:/test_folder/text.txt");
    int n;
    while (1) {
        printf("0) Текущие атрибуты\n1) Создание файла\n2) Прочитать из файла\n3) Записать в файл\n4) Только для чтения\n5) Архивный файл\n6) Для чтения и записи\n7) Скрыть файл\n8) Удалить атрибуты\n9) Выход\n");
        scanf_s(" %d", &n);
        if (n == 0) {
            const char* final = "C:/test_folder/text.txt";
            print_file_attributes(final);
            printf("\n");
        }
        if (n == 1) {
            WIN32_FIND_DATAA findData; ///Структура WIN32_FIND_DATA описывает файл, найденный функцией FindFirstFile 
            HANDLE hFile = FindFirstFileA("C:/test_folder/text.txt", &findData);/// Функция FindFirstFile ищет каталог файла или подкаталог, название которого соответствует указанному имени файла 
            
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
                printf("Архивный файл\n");
            else {
                FILE* file1 = fopen("C:/test_folder/text.txt", "w");
                printf("Файл создан\n");
                fclose(file1);
            }
        }
        if (n == 2) {
            WIN32_FIND_DATAA findData;
            HANDLE hFile = FindFirstFileA("C:/test_folder/text.txt", &findData);
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY || findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
                FILE* file3 = fopen("C:/test_folder/text.txt", "r");
                char a[100] = { 0 };
                char c;
                while ((c = fgetc(file3)) != EOF)
                    printf("%c", c);
                printf("\n");
                fclose(file3);

            }
        }
        if (n == 3) {
            WIN32_FIND_DATAA findData;
            HANDLE hFile = FindFirstFileA("C:/test_folder/text.txt", &findData);
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                printf("Файл только для чтения\n");
            else {

                FILE* file2 = fopen("C:/test_folder/text.txt", "w");
                char m[100];
                scanf("%99s", &m);
                fputs(m, file2);
                fclose(file2);
            }
        }
        if (n == 4) set_file_attributes(filename, FILE_ATTRIBUTE_READONLY);
        if (n == 5) set_file_attributes(filename, FILE_ATTRIBUTE_ARCHIVE);
        if (n == 6) SetFileAttributes(File, FILE_ATTRIBUTE_NORMAL);
        if (n == 7) set_file_attributes(filename, FILE_ATTRIBUTE_HIDDEN);
        if (n == 8) {
            while (true) {
                int ch;
                printf("Удаление атрибута:\n1) Только для чтения\n2) Архивный файл\n3) Скрытый файл\n");
                scanf("%d", &ch);
                if (ch == 1) {
                    if (!remove_file_attribute(filename, FILE_ATTRIBUTE_READONLY))
                        printf("Атрибут 'Только для чтения' не найден\n");
                }
                if (ch == 2) {
                    if (!remove_file_attribute(filename, FILE_ATTRIBUTE_ARCHIVE))
                        printf("Атрибут 'Архивный файл' не найден\n");
                }
                if (ch == 3) {
                    if (!remove_file_attribute(filename, FILE_ATTRIBUTE_HIDDEN))
                        printf("Атрибут 'Скрытый файл' не найден\n");
                }
                break;
            }
        }
        if (n == 9)
            break;
    }
    return 0;
}
