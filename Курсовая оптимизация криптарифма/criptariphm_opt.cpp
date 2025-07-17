#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <chrono>
char addend1_[10];
char addend2_[10];
char result_word[10];
int used_digits[10] = { 0 };

typedef struct {
    char letter;
    int value;
} Variable;

typedef struct {
    Variable* result;
    Variable* addend1;
    Variable* addend2;
} Column;

Variable variables[26];
int num_variables = 0;
Column columns[10];
int num_columns = 0;

Variable* get_or_create_variable(char letter) {
    for (int i = 0; i < num_variables; i++) {
        if (variables[i].letter == letter)
            return &variables[i];
    }
    Variable new_var;
    new_var.letter = letter;
    new_var.value = -1;
    variables[num_variables++] = new_var;
    return &variables[num_variables - 1];
}

void in_columns(char* addend1, char* addend2, char* result_word) {
    int max_len = strlen(result_word);
    num_columns = max_len;
    for (int i = 0; i < max_len; i++) {
        columns[i].result = NULL;
        columns[i].addend1 = NULL;
        columns[i].addend2 = NULL;
    }
    int len1 = strlen(addend1);
    for (int i = 0; i < len1; i++) {
        int col_index = max_len - (len1 - i);
        Variable* var = get_or_create_variable(addend1[i]);
        columns[col_index].addend1 = var;
    }
    int len2 = strlen(addend2);
    for (int i = 0; i < len2; i++) {
        int col_index = max_len - (len2 - i);
        Variable* var = get_or_create_variable(addend2[i]);
        columns[col_index].addend2 = var;
    }
    for (int i = 0; i < max_len; i++) {
        Variable* var = get_or_create_variable(result_word[i]);
        columns[i].result = var;
    }
}



void print_solution(const char* addend1, const char* addend2, const char* result_word) {
    int num1 = 0, num2 = 0, result = 0;

    for (int i = 0; addend1[i] != '\0'; i++) {
        for (int j = 0; j < num_variables; j++) {
            if (variables[j].letter == addend1[i]) {
                num1 = num1 * 10 + variables[j].value;
                break;
            }
        }
    }

    for (int i = 0; addend2[i] != '\0'; i++) {
        for (int j = 0; j < num_variables; j++) {
            if (variables[j].letter == addend2[i]) {
                num2 = num2 * 10 + variables[j].value;
                break;
            }
        }
    }

    for (int i = 0; result_word[i] != '\0'; i++) {
        for (int j = 0; j < num_variables; j++) {
            if (variables[j].letter == result_word[i]) {
                result = result * 10 + variables[j].value;
                break;
            }
        }
    }
    printf("%d - %d = %d\n", result, num1, num2);
}


bool solve(int col_index, int carry_in) {
    if (col_index < 0)
        return carry_in == 0;

    Column* col = &columns[col_index];

    Variable* addend1 = col->addend1;
    Variable* addend2 = col->addend2;
    Variable* result = col->result;

    int addend1_value = (addend1 && addend1->value != -1) ? addend1->value : -1;
    int addend2_value = (addend2 && addend2->value != -1) ? addend2->value : -1;
    int result_value = (result && result->value != -1) ? result->value : -1;

    //если одного/обоих слагаемых в столбце нет
    if (addend2 == NULL && addend1 == NULL) {
        if (result_value == 1 && carry_in == 1) return true;
        else if (result_value == -1 && carry_in == 1 && !used_digits[1]) {
            result->value = 1;
            return true;
        }
        else return false;
    }
    if (addend2 != NULL && addend1 == NULL) {
        if (addend2_value == -1) {
            if (result_value == -1) {
                if (carry_in == 1) {
                    for (int i = 0; i <= 8; i++) {
                        if (!used_digits[i] && !used_digits[i + 1]) {
                            addend2->value = i;
                            result->value = i + 1;
                            return true;
                        }
                    }
                    return false;
                }
                else {
                    for (int i = 0; i <= 9; i++) {
                        if (!used_digits[i]) {
                            addend2->value = i;
                            result->value = i;
                            return true;
                        }
                    }
                    return false;
                }
            }
            else if (!used_digits[result_value - carry_in]) {
                addend2->value = result_value - carry_in;
                return true;
            }
            else return false;
        }
        else if (result_value == -1) {
            if (!used_digits[addend2_value + carry_in]) {
                result->value = addend2_value + carry_in;
                return true;
            }
            else return false;
        }
        else {
            if (result_value - carry_in == addend2_value) return true;
            else return false;
        }
    }
    if (addend1 != NULL && addend2 == NULL) {
        if (addend1_value == -1) {
            if (result_value == -1) {
                if (carry_in == 1) {
                    for (int i = 0; i <= 8; i++) {
                        if (!used_digits[i] && !used_digits[i + 1]) {
                            addend1->value = i;
                            result->value = i + 1;
                            return true;
                        }
                    }
                    return false;
                }
                else {
                    for (int i = 0; i <= 9; i++) {
                        if (!used_digits[i]) {
                            addend1->value = i;
                            result->value = i;
                            return true;
                        }
                    }
                    return false;
                }
            }
            else if (!used_digits[result_value - carry_in]) {
                addend1->value = result_value - carry_in;
                return true;
            }
            else return false;
        }
        else if (result_value == -1) {
            if (!used_digits[addend1_value + carry_in]) {
                result->value = addend1_value + carry_in;
                return true;
            }
            else return false;
        }
        else {
            if (result_value - carry_in == addend1_value) return true;
            else return false;
        }
    }

    //слагаемым присвоена цифра, результату - нет
    if (addend1_value != -1 && addend2_value != -1 && result_value == -1) {
        int column_sum = addend1_value + addend2_value + carry_in;
        int carry_out = column_sum / 10;
        int result_digit = column_sum % 10;

        if (used_digits[result_digit] == 0) {
            result->value = result_digit;
            used_digits[result_digit] = 1;
        }
        else return false;
        if (solve(col_index - 1, carry_out)) {
            return true;
        }
            result->value = -1;
            used_digits[result_digit] = 0;
        return false;
    }
    //всем буквам столбца присвоена цифра
    else if (addend1_value != -1 && addend2_value != -1 && result_value != -1) {
        int column_sum = addend1_value + addend2_value + carry_in;
        int carry_out = column_sum / 10;
        int result_digit = column_sum % 10;
       if (result_value != result_digit) {
           return false;
        }
        if (solve(col_index - 1, carry_out)) {
            return true;
        }
        return false;
    }

    //буквы слагаемых одинаковые
    if (addend1->letter == addend2->letter) {
        //никакой букве не пресвоена цифра
        if (addend1_value == -1 && addend2_value == -1 && result_value == -1) {
            {
                if (used_digits[0]) goto next_digit_1;
                if (addend1->letter == addend1_[0] || addend1->letter == addend2_[0] || addend1->letter == result_word[0]) goto next_digit_1;
                addend1->value = 0;
                addend2->value = 0;
                used_digits[0] = 1;
                int column_sum = carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;
                    addend2->value = -1;
                    used_digits[0] = 0;

                    goto next_digit_1;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
               
                    result->value = -1;
                    used_digits[result_digit] = 0;
                
                addend2->value = -1;
                addend1->value = -1;
                used_digits[0] = 0;
            }
        next_digit_1:
            {
                if (used_digits[1]) goto next_digit_2;
                addend1->value = 1;
                addend2->value = 1;
                used_digits[1] = 1;
                int column_sum = 2 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;
                    used_digits[1] = 0;

                    goto next_digit_2;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
               
                    result->value = -1;
                    used_digits[result_digit] = 0;
                
                addend1->value = -1;
                used_digits[1] = 0;
            }
        next_digit_2:


            {
                if (used_digits[2]) goto next_digit_3;
                addend1->value = 2;
                addend2->value = 2;
                used_digits[2] = 1;
                int column_sum = 4 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;

                    used_digits[2] = 0;

                    goto next_digit_3;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
               

                addend1->value = -1;
                used_digits[2] = 0;
            }
        next_digit_3:
            {
                if (used_digits[3]) goto next_digit_4;
                addend1->value = 3;

                used_digits[3] = 1;
                int column_sum = 6 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else  {
                    addend1->value = -1;
                   
                    used_digits[3] = 0;

                    goto next_digit_4;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
               
                    result->value = -1;
                    used_digits[result_digit] = 0;
                

                addend1->value = -1;
                used_digits[3] = 0;
            }
        next_digit_4:


            {
                if (used_digits[4]) goto next_digit_5;
                addend1->value = 4;

                used_digits[4] = 1;
                int column_sum = 8 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;
                  
                    used_digits[4] = 0;

                    goto next_digit_5;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
                

                addend1->value = -1;
                used_digits[4] = 0;
            }
        next_digit_5:
            {
                if (used_digits[5]) goto next_digit_6;
                addend1->value = 5;

                used_digits[5] = 1;
                int column_sum = 10 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;
                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else  {
                    addend1->value = -1;
                   
                    used_digits[5] = 0;
                    goto next_digit_6;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
                

                addend1->value = -1;
                used_digits[5] = 0;
            }
        next_digit_6:


            {
                if (used_digits[6]) goto next_digit_7;

                addend1->value = 6;

                used_digits[6] = 1;
                int column_sum = 12 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else  {
                    addend1->value = -1;
                    
                    used_digits[6] = 0;

                    goto next_digit_7;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
                

                addend1->value = -1;
                used_digits[6] = 0;
            }
        next_digit_7:


            {
                if (used_digits[7]) goto next_digit_8;
                addend1->value = 7;

                used_digits[7] = 1;
                int column_sum = 14 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;
                    
                    used_digits[7] = 0;

                    goto next_digit_8;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
                

                addend1->value = -1;
                used_digits[7] = 0;
            }
        next_digit_8:


            {
                if (used_digits[8]) goto next_digit_9;

                addend1->value = 8;

                used_digits[8] = 1;
                int column_sum = 16 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else  {
                    addend1->value = -1;
                   
                    used_digits[8] = 0;

                    goto next_digit_9;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                
                    result->value = -1;
                    used_digits[result_digit] = 0;
               

                addend1->value = -1;
                used_digits[8] = 0;
            }
        next_digit_9:

            {
                if (used_digits[9]) goto end_loop;
                addend1->value = 9;
                used_digits[9] = 1;
                int column_sum = 18 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (used_digits[result_digit] == 0) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else {
                    addend1->value = -1;
                    
                    used_digits[9] = 0;

                    goto end_loop;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
              
                    result->value = -1;
                    used_digits[result_digit] = 0;
                
                addend1->value = -1;
                used_digits[9] = 0;
            }
        end_loop:
            return false;
        }
        //результату присвоена цифра, слагаемым - нет
        if (addend1_value == -1 && addend2_value == -1 && result_value != -1) {
            {
                if (used_digits[0]) goto next_digit_11;
                if (addend1->letter == addend1_[0] || addend1->letter == addend2_[0] || addend1->letter == result_word[0]) goto next_digit_11;
                addend1->value = 0;
                addend2->value = 0;
                used_digits[0] = 1;
                int column_sum = carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

               if (result_value != result_digit)
               {
                    addend1->value = -1;
                    addend2->value = -1;
                    used_digits[0] = 0;

                    goto next_digit_1;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                addend2->value = -1;
                addend1->value = -1;
                used_digits[0] = 0;
            }
        next_digit_11:
            {
                if (used_digits[1]) goto next_digit_12;
                addend1->value = 1;
                addend2->value = 1;
                used_digits[1] = 1;
                int column_sum = 2 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[1] = 0;

                    goto next_digit_12;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

               
                addend1->value = -1;
                used_digits[1] = 0;
            }
        next_digit_12:


            {
                if (used_digits[2]) goto next_digit_13;
                addend1->value = 2;
                addend2->value = 2;
                used_digits[2] = 1;
                int column_sum = 4 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[2] = 0;

                    goto next_digit_13;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }


                addend1->value = -1;
                used_digits[2] = 0;
            }
        next_digit_13:
            {
                if (used_digits[3]) goto next_digit_14;
                addend1->value = 3;

                used_digits[3] = 1;
                int column_sum = 6 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[3] = 0;

                    goto next_digit_14;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

               

                addend1->value = -1;
                used_digits[3] = 0;
            }
        next_digit_14:


            {
                if (used_digits[4]) goto next_digit_15;
                addend1->value = 4;

                used_digits[4] = 1;
                int column_sum = 8 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[4] = 0;

                    goto next_digit_15;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                

                addend1->value = -1;
                used_digits[4] = 0;
            }
        next_digit_15:
            {
                if (used_digits[5]) goto next_digit_16;
                addend1->value = 5;

                used_digits[5] = 1;
                int column_sum = 10 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;
                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[5] = 0;
                    goto next_digit_16;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

              

                addend1->value = -1;
                used_digits[5] = 0;
            }
        next_digit_16:


            {
                if (used_digits[6]) goto next_digit_17;

                addend1->value = 6;

                used_digits[6] = 1;
                int column_sum = 12 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[6] = 0;

                    goto next_digit_17;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                

                addend1->value = -1;
                used_digits[6] = 0;
            }
        next_digit_17:


            {
                if (used_digits[7]) goto next_digit_18;
                addend1->value = 7;

                used_digits[7] = 1;
                int column_sum = 14 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[7] = 0;

                    goto next_digit_18;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

               
                addend1->value = -1;
                used_digits[7] = 0;
            }
        next_digit_18:


            {
                if (used_digits[8]) goto next_digit_19;

                addend1->value = 8;

                used_digits[8] = 1;
                int column_sum = 16 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[8] = 0;

                    goto next_digit_19;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

              

                addend1->value = -1;
                used_digits[8] = 0;
            }
        next_digit_19:

            {
                if (used_digits[9]) goto end_1loop;
                addend1->value = 9;
                used_digits[9] = 1;
                int column_sum = 18 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value != result_digit) {
                    addend1->value = -1;

                    used_digits[9] = 0;

                    goto end_1loop;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }
                addend1->value = -1;
                used_digits[9] = 0;
            }
        end_1loop:
            return false;
        }
    }
    //буквы слагаемых разные
    else {
        //слагаемым не присвоены цифры
        if (addend1_value == -1 && addend2_value == -1) {
            for (int digit1 = 0; digit1 <= 9; digit1++) {
                if (used_digits[digit1]) continue;
                if (digit1 == 0 && (addend1->letter == addend1_[0] || addend1->letter == addend2_[0] || addend1->letter == result_word[0])) continue;


                addend1->value = digit1;
                used_digits[digit1] = 1;

                {
                    if (used_digits[0]) goto next_digit_2_1;
                    if (addend2->letter == addend1_[0] || addend2->letter == addend2_[0] || addend2->letter == result_word[0]) goto next_digit_2_1;

                    addend2->value = 0;
                    used_digits[0] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 0;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[0] = 0;
                        goto next_digit_2_1;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[0] = 0;
                }
            next_digit_2_1:


                {
                    if (used_digits[1]) goto next_digit_2_2;

                    addend2->value = 1;
                    used_digits[1] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 1;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 1 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[1] = 0;
                        goto next_digit_2_2;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[1] = 0;
                }
            next_digit_2_2:


                {
                    if (used_digits[2]) goto next_digit_2_3;

                    addend2->value = 2;
                    used_digits[2] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 2;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 2 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[2] = 0;
                        goto next_digit_2_3;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[2] = 0;
                }
            next_digit_2_3:


                {
                    if (used_digits[3]) goto next_digit_2_4;

                    addend2->value = 3;
                    used_digits[3] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 3;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 3 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[3] = 0;
                        goto next_digit_2_4;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[3] = 0;
                }
            next_digit_2_4:


                {
                    if (used_digits[4]) goto next_digit_2_5;

                    addend2->value = 4;
                    used_digits[4] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 4;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 4 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[4] = 0;
                        goto next_digit_2_5;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[4] = 0;
                }
            next_digit_2_5:


                {
                    if (used_digits[5]) goto next_digit_2_6;

                    addend2->value = 5;
                    used_digits[5] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 5;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 5 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[5] = 0;
                        goto next_digit_2_6;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[5] = 0;
                }
            next_digit_2_6:

                {
                    if (used_digits[6]) goto next_digit_2_7;

                    addend2->value = 6;
                    used_digits[6] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 6;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 6 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[6] = 0;
                        goto next_digit_2_7;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[6] = 0;
                }
            next_digit_2_7:


                {
                    if (used_digits[7]) goto next_digit_2_8;

                    addend2->value = 7;
                    used_digits[7] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 7;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 7 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[7] = 0;
                        goto next_digit_2_8;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[7] = 0;
                }
            next_digit_2_8:


                {
                    if (used_digits[8]) goto next_digit_2_9;

                    addend2->value = 8;
                    used_digits[8] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 8;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 8 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[8] = 0;
                        goto next_digit_2_9;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[8] = 0;
                }
            next_digit_2_9:


                {
                    if (used_digits[9]) goto end_loop_2;

                    addend2->value = 9;
                    used_digits[9] = 1;

                    if (addend1->letter == result->letter) {
                        result_value = digit1;
                        result->value = result_value;
                    }
                    if (addend2->letter == result->letter) {
                        result_value = 9;
                        result->value = result_value;
                    }

                    int column_sum = digit1 + 9 + carry_in;
                    int carry_out = column_sum / 10;
                    int result_digit = column_sum % 10;

                    if (result_value == -1 && !used_digits[result_digit]) {
                        result->value = result_digit;
                        used_digits[result_digit] = 1;
                    }
                    else if (result_value != result_digit) {
                        addend2->value = -1;
                        used_digits[9] = 0;
                        goto end_loop_2;
                    }

                    if (solve(col_index - 1, carry_out)) {
                        return true;
                    }

                    if (result_value == -1) {
                        result->value = -1;
                        used_digits[result_digit] = 0;
                    }

                    addend2->value = -1;
                    used_digits[9] = 0;
                }
            end_loop_2:
                addend1->value = -1;
                used_digits[digit1] = 0;
            }
            return false;
        }
        //первому слагаемому присвоена цифра, второму - нет
        else if (addend1_value != -1 && addend2_value == -1) {
            {
                if (used_digits[0]) goto next_digit_3_1;
                if (addend2->letter == addend1_[0] || addend2->letter == addend2_[0] || addend2->letter == result_word[0]) goto next_digit_3_1;

                addend2->value = 0;
                used_digits[0] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 0;
                    result->value = result_value;
                }

                int column_sum = addend1_value + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[0] = 0;
                    goto next_digit_3_1;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[0] = 0;
            }
        next_digit_3_1:


            {
                if (used_digits[1]) goto next_digit_3_2;

                addend2->value = 1;
                used_digits[1] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 1;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 1 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[1] = 0;
                    goto next_digit_3_2;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[1] = 0;
            }
        next_digit_3_2:


            {
                if (used_digits[2]) goto next_digit_3_3;

                addend2->value = 2;
                used_digits[2] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 2;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 2 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[2] = 0;
                    goto next_digit_3_3;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[2] = 0;
            }
        next_digit_3_3:


            {
                if (used_digits[3]) goto next_digit_3_4;

                addend2->value = 3;
                used_digits[3] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 3;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 3 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[3] = 0;
                    goto next_digit_3_4;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[3] = 0;
            }
        next_digit_3_4:


            {
                if (used_digits[4]) goto next_digit_3_5;

                addend2->value = 4;
                used_digits[4] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 4;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 4 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[4] = 0;
                    goto next_digit_3_5;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[4] = 0;
            }
        next_digit_3_5:


            {
                if (used_digits[5]) goto next_digit_3_6;

                addend2->value = 5;
                used_digits[5] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 5;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 5 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[5] = 0;
                    goto next_digit_3_6;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[5] = 0;
            }
        next_digit_3_6:


            {
                if (used_digits[6]) goto next_digit_3_7;

                addend2->value = 6;
                used_digits[6] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 6;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 6 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[6] = 0;
                    goto next_digit_3_7;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[6] = 0;
            }
        next_digit_3_7:


            {
                if (used_digits[7]) goto next_digit_3_8;

                addend2->value = 7;
                used_digits[7] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 7;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 7 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[7] = 0;
                    goto next_digit_3_8;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[7] = 0;
            }
        next_digit_3_8:


            {
                if (used_digits[8]) goto next_digit_3_9;

                addend2->value = 8;
                used_digits[8] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 8;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 8 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[8] = 0;
                    goto next_digit_3_9;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[8] = 0;
            }
        next_digit_3_9:


            {
                if (used_digits[9]) goto end_loop_3;

                addend2->value = 9;
                used_digits[9] = 1;

                if (addend2->letter == result->letter) {
                    result_value = 9;
                    result->value = result_value;
                }

                int column_sum = addend1_value + 9 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend2->value = -1;
                    used_digits[9] = 0;
                    goto end_loop_3;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend2->value = -1;
                used_digits[9] = 0;
            }
        end_loop_3:
            return false;
        }
        //второму слагаемому присвоена цифра, первому - нет
        else if (addend1_value == -1 && addend2_value != -1) {
            {
                if (used_digits[0]) goto next_digit_4_1;
                if (addend1->letter == addend1_[0] || addend1->letter == addend2_[0] || addend1->letter == result_word[0]) goto next_digit_4_1;

                addend1->value = 0;
                used_digits[0] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 0;
                    result->value = result_value;
                }

                int column_sum = addend2_value + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[0] = 0;
                    goto next_digit_4_1;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[0] = 0;
            }
        next_digit_4_1:


            {
                if (used_digits[1]) goto next_digit_4_2;

                addend1->value = 1;
                used_digits[1] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 1;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 1 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[1] = 0;
                    goto next_digit_4_2;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[1] = 0;
            }
        next_digit_4_2:


            {
                if (used_digits[2]) goto next_digit_4_3;

                addend1->value = 2;
                used_digits[2] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 2;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 2 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[2] = 0;
                    goto next_digit_4_3;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[2] = 0;
            }
        next_digit_4_3:


            {
                if (used_digits[3]) goto next_digit_4_4;

                addend1->value = 3;
                used_digits[3] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 3;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 3 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[3] = 0;
                    goto next_digit_4_4;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[3] = 0;
            }
        next_digit_4_4:


            {
                if (used_digits[4]) goto next_digit_4_5;

                addend1->value = 4;
                used_digits[4] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 4;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 4 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[4] = 0;
                    goto next_digit_4_5;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[4] = 0;
            }
        next_digit_4_5:


            {
                if (used_digits[5]) goto next_digit_4_6;

                addend1->value = 5;
                used_digits[5] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 5;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 5 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[5] = 0;
                    goto next_digit_4_6;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[5] = 0;
            }
        next_digit_4_6:


            {
                if (used_digits[6]) goto next_digit_4_7;

                addend1->value = 6;
                used_digits[6] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 6;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 6 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[6] = 0;
                    goto next_digit_4_7;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[6] = 0;
            }
        next_digit_4_7:


            {
                if (used_digits[7]) goto next_digit_4_8;

                addend1->value = 7;
                used_digits[7] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 7;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 7 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[7] = 0;
                    goto next_digit_4_8;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[7] = 0;
            }
        next_digit_4_8:


            {
                if (used_digits[8]) goto next_digit_4_9;

                addend1->value = 8;
                used_digits[8] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 8;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 8 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[8] = 0;
                    goto next_digit_4_9;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[8] = 0;
            }
        next_digit_4_9:


            {
                if (used_digits[9]) goto end_loop_4;

                addend1->value = 9;
                used_digits[9] = 1;

                if (addend1->letter == result->letter) {
                    result_value = 9;
                    result->value = result_value;
                }

                int column_sum = addend2_value + 9 + carry_in;
                int carry_out = column_sum / 10;
                int result_digit = column_sum % 10;

                if (result_value == -1 && !used_digits[result_digit]) {
                    result->value = result_digit;
                    used_digits[result_digit] = 1;
                }
                else if (result_value != result_digit) {
                    addend1->value = -1;
                    used_digits[9] = 0;
                    goto end_loop_4;
                }

                if (solve(col_index - 1, carry_out)) {
                    return true;
                }

                if (result_value == -1) {
                    result->value = -1;
                    used_digits[result_digit] = 0;
                }

                addend1->value = -1;
                used_digits[9] = 0;
            }
        end_loop_4:
            return false;
        }
    }
    return false;
}


int main() {
    scanf("%s %s %s", result_word, addend1_, addend2_);
    auto start = std::chrono::high_resolution_clock::now();

    in_columns(addend1_, addend2_, result_word);

    if (solve(num_columns - 1, 0)) {
        printf("Solution found!\n");
        print_solution(addend1_, addend2_, result_word);
	}
    else
        printf("No solution\n");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    printf("Total time: %.7f seconds\n", elapsed.count());
    return 0;
}
