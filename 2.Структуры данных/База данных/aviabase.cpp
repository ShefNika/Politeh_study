#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>


enum plane_class {
    passenger = 1,
    cargo = 2,
    military = 3,
    special = 4
};

union special_mark {
    char symbol;
    short int digit;

};

struct date {
    int year;
    int day;
    int month;
};

struct plane {
    char manufacturer[256]; //название производителя
    struct date assembly; //дата сборки
    double fly_hours; //время в полёте в часах
    unsigned int model_number;
    enum plane_class plane_class;
    int p_class;
    union special_mark special_mark;
    struct plane* previous; //указатель на предыдущий самолёт в базе данных
    struct plane* next; // указатель на следующий самолёт в базе данных
};

void rec_enter(struct plane* new_plane)
{
    printf("Введите производителя самолёта: \n");
    scanf("%s", new_plane->manufacturer);
    printf("Введите номер модели: \n");
    scanf("%d", &new_plane->model_number);
    printf("Введите дату сборки: \n");
    scanf("%d %d %d", &new_plane->assembly.day, &new_plane->assembly.month, &new_plane->assembly.year);
    printf("Введите лётные часы: \n");
    scanf("%lf", &new_plane->fly_hours);
    printf("Введите класс самолёта: \n");
    scanf("%d", &new_plane->p_class);
    printf("Введите особую отметку: \n");
    while (getchar() != '\n');
    scanf("%c", &new_plane->special_mark.symbol);
    while (getchar() != '\n');
}
struct plane* new_rec() 
{
    struct plane* new_plane = (struct plane*)malloc(sizeof(struct plane));
    new_plane->next = NULL;
    rec_enter(new_plane);
    return new_plane;
}

void add(struct plane* head) {
    struct plane* new_plane = new_rec();
    new_plane->next = NULL;
    struct plane* uk = head;
    for (; uk->next != NULL; uk = uk->next);
    new_plane->previous = uk;
    uk->next = new_plane;
}

void print(struct plane* new_plane) 
{
    printf("Производитель: %s\n", new_plane->manufacturer);
    printf("Модель: %d\n", new_plane->model_number);
    printf("Дата сборки: %d %d %d\n", new_plane->assembly.day, new_plane->assembly.month, new_plane->assembly.year);
    printf("Лётные часы: %.2lf\n", new_plane->fly_hours);
    printf("Класс самолёта: %d\n", new_plane->p_class);
    printf("Особая отметка: %c\n", new_plane->special_mark.symbol);
    printf("\n");
}

void rremove(struct plane* uk) 
{
    if (uk == NULL) return; 
    uk->previous->next = uk->next; 
    if (uk->next != NULL) uk->next->previous = uk->previous; 
    free(uk);
}

void insert(struct plane* uk) 
{
    struct plane* new_plane = new_rec();
    new_plane->previous = uk; 
    new_plane->next = uk->next; 
    if (uk->next != NULL) uk->next->previous = new_plane;
    uk->next = new_plane; 
}

struct plane* find(struct plane* head, int model_number) 
{
    struct plane* uk = head;
    for (; uk != NULL; uk = uk->next)
        if (uk->model_number == model_number)
            break;
    return uk;
}

int main() {
    setlocale(LC_ALL, "Russian");
    struct plane* head = (struct plane*)malloc(sizeof(struct plane));
    head->next = NULL;
    int m, Out = 0;

    while (!Out)
    {
        printf("\nВыберите действие:\n");
        printf("1. Считать из файла\n");
        printf("2. Добавить самолёт\n");
        printf("3. Редактировать данные самолёта\n");
        printf("4. Удалить самолёт\n");
        printf("5. Поиск по базе данных\n");
        printf("6. Вывод базы данных\n");
        printf("7. Сохранить в файл\n");
        printf("8. Выход\n");
        printf("Ваша комманда: ");
        scanf("%d", &m);
        switch (m)
        {
        case 1: // чтение базы данных из файла
        {
            FILE* FIn;
            FIn = fopen("plane_base.txt", "r");
            if (FIn == NULL) {
                printf("Невозможно открыть файл!\n");
                break;
            }
          
            for (struct plane* uk = head->next; uk != NULL;)
            {
                struct plane* tmp = uk;
                uk = uk->next;
                free(tmp);
            }
            head->next = NULL;
            while (!feof(FIn))
            {
                char str[10];
                struct plane* new_plane = (struct plane*)malloc(sizeof(struct plane));

                if (fscanf(FIn, "%s", new_plane->manufacturer) == EOF)
                {
                    free(new_plane);
                    break;
                }


                fscanf(FIn, "%d", &new_plane->model_number);
                fscanf(FIn, "%d", &new_plane->assembly.day);
                fscanf(FIn, "%d", &new_plane->assembly.month);
                fscanf(FIn, "%d", &new_plane->assembly.year); 
                fscanf(FIn, "%lf", &new_plane->fly_hours);
                fscanf(FIn, "%d", &new_plane->p_class);
                fscanf(FIn, "%s", str);
                new_plane->special_mark.symbol = str[0];

                new_plane->next = NULL;
                struct plane* uk = head;
                for (; uk->next != NULL; uk = uk->next);
                new_plane->previous = uk;
                uk->next = new_plane;
            }
            fclose(FIn);
        }
        break;
        case 2:
        // добавить самолёт
            add(head);
            break;
  
        case 3: //отредактировать самолёт
        {
            printf("Введите модель самолёта, которую хотите редактировать: ");
            int model_number;
            scanf("%d", &model_number);
            struct plane* uk = find(head, model_number);
            if (uk == NULL) {
                printf("Модель не найдена в базе данных\n");
                break;
            }
            rec_enter(uk);
            printf("Модель отредактирована\n\n");
            break;
        }
        case 4: // удалить самолёт
        {
            printf("Введите модель самолёта, которую хотите удалить: ");
            int model_number;
            scanf("%d", &model_number);
            struct plane* uk = find(head, model_number);
            if (uk == NULL) {
                printf("Модель не найдена в базе данных\n");
                break;
            }
            rremove(uk);
            printf("Модель удалена из базы данных\n\n");
            break;
        }
        case 5: // поиск по базе данных
        {
            printf("Введите модель самолёта: ");
            int model_number;
            scanf("%d", &model_number);
            struct plane* uk = find(head, model_number);
            if (uk == NULL) {
                printf("Модель не найдена в базе данных\n");
                break;
            }
            print(uk);
            break;
        }
        case 6: // вывод базы данных
        {
            struct plane* uk = head->next;
            for (; uk != NULL; uk = uk->next)
            {
                print(uk);
                printf("*****************\n");
            }
            break;
        }
        case 7: // запись базы данных в файл
        {
            FILE* FOut;
            FOut = fopen("plane_base.txt", "w");
            for (struct plane* uk = head->next; uk != NULL; uk = uk->next)
            {
                fprintf(FOut, "%s\n", uk->manufacturer);
                fprintf(FOut, "%d\n", uk->model_number);
                fprintf(FOut, "%d\n", uk->assembly.day);
                fprintf(FOut, "%d\n", uk->assembly.month);
                fprintf(FOut, "%d\n", uk->assembly.year);
                fprintf(FOut, "%.2lf\n", uk->fly_hours);
                fprintf(FOut, "%d\n", uk->p_class);
                fprintf(FOut, "%c\n", uk->special_mark.symbol);
            }
            fclose(FOut);
        }
        break;
        case 8: // выход
            Out = 1;
            break;
        default:
            printf("Неверный ввод комманды!\n");
            break;
        }
    }
    return 0;
}
