#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>
#include <winreg.h>
#pragma comment(lib, "Advapi32.lib")
 

char addend1_[10];
char addend2_[10];
char result_word[10];
int used_digits[10] = { 0 };

#define MAX_LENGTH 50
#define Key "qWeRtY5"
#define right_crc 1545082755
#define CODE_KEY 0xAA
#define CODE_LEN 16

char s1[] = { 0x01, 0x36, 0x16, 0x21, 0x03, 0x36, 0x47, 0x15, 0x79, 0x11, 0x2A, 0x00, 0x80 };
char s2[] = { 0x03, 0x67 };
char s3[] = { 0x02, 0x32, 0x17, 0x3B, 0x15, 0x35, 0x1B, 0x05, 0x2F, 0x11, 0x12 };
char s4[] = { 0x10, 0x89 };
char cor[] = { 0x22, 0x23, 0x17, 0x3D, 0x1A, 0x3E, 0x65, 0x10, 0x24, 0x16, 0x25, 0x1B, 0x2B, 0x51, 0x40, 0x65, 0x56, 0x66, 0x87 };
char s5[] = { 0x21, 0x36, 0x16, 0x21, 0x03, 0x36, 0x47, 0x15, 0x77, 0x0C, 0x21, 0x54, 0x3A, 0x5A, 0x03, 0x25, 0x00, 0x31, 0x00, 0x78, 0x07 };
char s6[] = { 0x22, 0x38, 0x09, 0x27, 0x00, 0x30, 0x5A, 0x1F, 0x77, 0x03, 0x3D, 0x01, 0x37, 0x51, 0x50, 0x69 };
char s7[] = { 0x3F, 0x38, 0x45, 0x21, 0x1B, 0x35, 0x40, 0x05, 0x3E, 0x0A, 0x3C, 0x11 };
char s8[] = { 0x34, 0x25, 0x17, 0x3D, 0x06, 0x63, 0x15, 0x21, 0x36, 0x16, 0x21, 0x03, 0x36, 0x47, 0x15, 0x77, 0x0C, 0x21, 0x54, 0x30, 0x5B, 0x12, 0x38, 0x17, 0x20, 0x11, 0x3A, 0x41, 0x50, 0x12 };
char s65[] = { 0x39, 0x36, 0x16, 0x81, 0x63, 0x36, 0x47, 0x85, 0x79, 0x14, 0x2A, 0x00, 0x80, 0x78, 0x54, 0x46 };
char s66[] = { 0x10, 0x89, 0x60, 0x89,0x19, 0x99,0x40, 0x29,0x10, 0x89, };
char dummy[10] = { 0x18, 0x96, 0xB6, 0x89, 0x16, 0xE5, 0x4F, 0x00, 0x11, 0x82 };
char s9[] = {0x1F, 0x23, 0x01, 0x3E, 0x18, 0x77, 0x51, 0x1D, 0x3B, 0x71};
char s10[] = {0x3F, 0x23, 0x34, 0x27, 0x11, 0x2B, 0x4C, 0x38, 0x39, 0x03, 0x3D, 0x06, 0x34, 0x54, 0x05, 0x3E, 0x0A, 0x3C, 0x24, 0x2B, 0x5A, 0x12, 0x32, 0x16, 0x21, 0x77};
char encrypted_smc[CODE_LEN] = {0x29, 0xD7, 0x1E, 0x98, 0xA5, 0x27, 0x66, 0xAA, 0xAA, 0xAA, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A};
char s11[] = {0x27, 0x3E, 0x17, 0x26, 0x01, 0x38, 0x59, 0x51, 0x3A, 0x04, 0x31, 0x1C, 0x30, 0x5B, 0x14, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x73, 0x34};
char s12[] = {0x27, 0x3E, 0x17, 0x26, 0x01, 0x38, 0x59, 0x51, 0x3A, 0x04, 0x31, 0x1C, 0x30, 0x5B, 0x14, 0x77, 0x16, 0x37, 0x17, 0x36, 0x5B, 0x15, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x73, 0x33};
char s13[] = {0x22, 0x0E, 0x36, 0x06, 0x31, 0x14, 0x69, 0x32, 0x22, 0x17, 0x20, 0x11, 0x37, 0x41, 0x32, 0x38, 0x0B, 0x26, 0x06, 0x36, 0x59, 0x22, 0x32, 0x11, 0x0E, 0x37, 0x36, 0x5B, 0x05, 0x25, 0x0A, 0x3E, 0x28, 0x0A, 0x4C, 0x02, 0x23, 0x00, 0x3F, 0x3D, 0x37, 0x53, 0x1E, 0x25, 0x08, 0x33, 0x00, 0x30, 0x5A, 0x1F, 0x12};
char s14[] = {0x22, 0x2E, 0x16, 0x26, 0x11, 0x34, 0x65, 0x03, 0x38, 0x01, 0x27, 0x17, 0x2D, 0x7B, 0x10, 0x3A, 0x00, 0x30};
char s15[] = {0x27, 0x3E, 0x17, 0x26, 0x01, 0x38, 0x59, 0x33, 0x38, 0x1D, 0x13};
char s16[] = {0x35, 0x32, 0x07, 0x27, 0x13, 0x3E, 0x50, 0x03, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x72, 0x16, 0x20, 0x15, 0x25, 0x25, 0x04, 0x22, 0x32, 0x35, 0x54, 0x16, 0x77, 0x08, 0x37, 0x00, 0x31, 0x5A, 0x15, 0x78};
char s17[] = {0x35, 0x32, 0x07, 0x27, 0x13, 0x3E, 0x50, 0x03, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x72, 0x16, 0x20, 0x15, 0x38, 0x24, 0x21, 0x37, 0x16, 0x2C, 0x52, 0x16, 0x32, 0x17, 0x02, 0x06, 0x3C, 0x46, 0x14, 0x39, 0x11, 0x72, 0x19, 0x3C, 0x41, 0x19, 0x38, 0x01, 0x98};
char s18[] = {0x35, 0x32, 0x07, 0x27, 0x13, 0x3E, 0x50, 0x03, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x72, 0x16, 0x20, 0x15, 0x32, 0x3F, 0x00, 0x31, 0x1F, 0x0B, 0x50, 0x1C, 0x38, 0x11, 0x37, 0x30, 0x3C, 0x57, 0x04, 0x30, 0x02, 0x37, 0x06, 0x09, 0x47, 0x14, 0x24, 0x00, 0x3C, 0x00, 0x79, 0x58, 0x14, 0x23, 0x0D, 0x3D, 0x10, 0x67};
char s19[] = {0x32, 0x3F, 0x00, 0x31, 0x1F, 0x06, 0x45, 0x10, 0x24, 0x16, 0x25, 0x1B, 0x2B, 0x51, 0x51, 0x20, 0x04, 0x21, 0x54, 0x34, 0x5A, 0x15, 0x3E, 0x03, 0x3B, 0x11, 0x3D, 0x14, 0x78};
char s20[] = {0x35, 0x32, 0x07, 0x27, 0x13, 0x3E, 0x50, 0x03, 0x77, 0x01, 0x37, 0x00, 0x3C, 0x56, 0x05, 0x32, 0x01, 0x72, 0x16, 0x20, 0x15, 0x21, 0x25, 0x0A, 0x31, 0x11, 0x2A, 0x46, 0x35, 0x32, 0x07, 0x27, 0x13, 0x16, 0x57, 0x1B, 0x32, 0x06, 0x26, 0x3C, 0x38, 0x5B, 0x15, 0x3B, 0x00, 0x72, 0x19, 0x3C, 0x41, 0x19, 0x38, 0x01, 0x00};


typedef NTSTATUS (WINAPI *NtQueryInformationProcess_t)(
    HANDLE, ULONG, PVOID, ULONG, PULONG
);

typedef struct {
    char letter;
    int value;
} Variable;

typedef struct {
    Variable* result;
    Variable* addend1;
    Variable* addend2;
} Column;

void xor_(char* data, size_t len);
int check_password(char* password);
void check_password_end();
void display_result(int flag);
DWORD CalcFuncCrc(PUCHAR funcBegin, PUCHAR funcEnd);
Variable* get_or_create_variable(char letter);
int check(char* password);
void in_columns(char* addend1, char* addend2, char* result_word, char* password);
void print_solution(const char* addend1, const char* addend2, const char* result_word);
bool solve(int col_index, int carry_in);
int read_password(char* password);
char* create_serial();
int print_serial(char* serial);
int check_crc();
int pdoh_debugger(); // ProcessDebugObjectHandle debugger
int crdp_debugger(); // CheckRemoteDebuggerPresent debugger
int tf_debugger();  // TrapFlag debugger
int reg_key_ex(HKEY hKeyRoot, const char* lpSubKey); // RegOpenKeyExA anti-VM
int reg_key_compare(HKEY hKeyRoot, const char* lpSubKey, const char* regVal, const char* compare); // RegQueryValueExA anti-VM

Variable variables[26];
int num_variables = 0;
Column columns[10];
int num_columns = 0;
volatile int dummy_obf = 0;

int reg_key_ex(HKEY hKeyRoot, const char* lpSubKey) {
    HKEY hKey = NULL;
    LONG ret = RegOpenKeyExA(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1; 
    }
    if (hKey != NULL) RegCloseKey(hKey);
    return 0;  
}

int reg_key_compare(HKEY hKeyRoot, const char* lpSubKey, const char* regVal, const char* compare) {
    HKEY hKey = NULL;
    LONG ret;
    char value[1024] = {0};
    DWORD size = sizeof(value);
    ret = RegOpenKeyExA(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS) {
        ret = RegQueryValueExA(hKey, regVal, NULL, NULL, (LPBYTE)value, &size);
        if (ret == ERROR_SUCCESS) {
            if (strcmp(value, compare) == 0) {
                RegCloseKey(hKey);
                return 1;  
            }
        }
        RegCloseKey(hKey);
    }
    return 0;  
}


int pdoh_debugger() {
    xor_(s10, 26);
    HMODULE hNtdll = LoadLibraryA("ntdll.dll");
    NtQueryInformationProcess_t NtQueryInformationProcess =
        (NtQueryInformationProcess_t)GetProcAddress(hNtdll, s10);
    HANDLE hDebugObject = NULL;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(), 0x1e, &hDebugObject, sizeof(HANDLE), NULL);
    FreeLibrary(hNtdll);
    return (status == 0 && hDebugObject != NULL);
}

int crdp_debugger() {
    BOOL isDebugger = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugger);
    return isDebugger;
}

int tf_debugger() {
        bool isDebugged = TRUE;
    __try {
        __asm {
            pushfd // Сохранение регистра EFLAGS в стеке
            or dword ptr[esp], 0x100 // Установка флага TF
            popfd // Загрузка регистра EFLAGS из стека
            nop
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        isDebugged = FALSE;
    }
    return isDebugged;
} 

void xor_(char* data, size_t len) {
    for (size_t i = 0; i < len; i++)
        data[i] ^= Key[i % strlen(Key)];
    data[len - 1] = '\0';
}

int check_password(char* password) {
    char temp_cor[20];
    size_t len_cor = 19;
    int x, y;
    x=1;
    x++;
    y=x;
    memcpy(temp_cor, cor, len_cor);
    xor_(temp_cor, len_cor);
    __asm {
        call $+5              // Call следующей инструкции
        add dword ptr [esp], 8 // Пропускаем 8 байт
        ret                   // Возврат по новому адресу
        // Мусорные байты (8 байт)
        _emit 0xE8
        _emit 0x12
        _emit 0x34
        _emit 0x56
        _emit 0x78
        _emit 0x90
        _emit 0x90
        _emit 0x90
        // Сюда произойдет возврат после ret
        nop
    }
    int result = strcmp(password, temp_cor);
    return (result == 0) ? (rand() % 50 + 1) : (rand() % 50 + 51);
}


void check_password_end() {
}

void display_result(int flag) {
    xor_(s5, 21);
    xor_(s8, 30);

    void* branch_true = NULL;
    void* branch_false = NULL;
    __asm {
        mov [branch_true], offset branch_true_label
        mov [branch_false], offset branch_false_label
    }

    volatile int trigger = 0; 
    __try {
        int result = 1 / trigger; //  EXCEPTION_INT_DIVIDE_BY_ZERO
    }
    __except (
        (flag + 30 < 80) ? (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                         : (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    ) {
        void* target = (flag + 30 < 80) ? branch_true : branch_false;
        __asm {
            mov eax, [target]
            jmp eax // Косвенный переход
        }
    }
branch_true_label:
    printf("%s\n", s5);
    goto end_branches;

branch_false_label:
    printf("%s\n", s8);

end_branches:
    return;
}


DWORD CalcFuncCrc(PUCHAR funcBegin, PUCHAR funcEnd) {
    DWORD crc = 0xFFFFFFFF;  // все биты установлены в 1
    
    for (; funcBegin < funcEnd; ++funcBegin) {
        crc ^= *funcBegin;   // XOR с текущим байтом
        // Обработка 8 битов текущего байта
        for (int i = 0; i < 8; i++) {
            if (crc & 1) 
                crc = (crc >> 1) ^ 0xEDB88320;  // Полином CRC32 
            else 
                crc = crc >> 1;
        }
    }
    return ~crc;  // Инвертирование конечного результата
}

int check_crc() {
    DWORD crc = CalcFuncCrc((PUCHAR)&check_password, (PUCHAR)&check_password_end);
    return crc == right_crc;
}


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

int check(char* password) {
    xor_(dummy, 10);
      __asm {
        xor eax, eax        // ZF = 1
        test eax, eax       // ZF = 1
        jz label_2  // Выполнится (ZF = 1)
        jnz label_2 // Не выполнится
        _emit 0xE8  // call (opcode) - на этот байт указывают переходы 
    label_2:
        nop
    }
    int result = strcmp(password, dummy);
    return (result == 0) ? (rand() % 50 + 51) : (rand() % 50 + 52);
}

void in_columns(char* addend1, char* addend2, char* result_word, char* password) {
    int max_len = strlen(result_word);
    num_columns = max_len;
    if (check_password(password) > 50)
        return;
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

       if (tf_debugger()) {
            xor_(s16,37);
            printf("%s\n", s16);
            return;
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

    if (IsDebuggerPresent()) { // IsDebuggerPresent debugger
        xor_(s17,46);
        printf("%s\n", s17);
        return false;
    }

    int addend1_value = (addend1 && addend1->value != -1) ? addend1->value : -1;
    int addend2_value = (addend2 && addend2->value != -1) ? addend2->value : -1;
    int result_value = (result && result->value != -1) ? result->value : -1;
    bool is1 = (addend1_value != -1) ? true : false;
    bool is2 = (addend2_value != -1) ? true : false;
    char hi[7] = { 0x12, 0x89, 0x7D, 0x78, 0x5F, 0x03, 0x88 };
    xor_(hi, 7);
    if (check_password(hi) < 50)
    {
        if (addend2 == NULL && addend1 == NULL) {
            if (result_value == 1 && carry_in == 1) return true;
            else if (result_value == -1 && carry_in == 1 && !used_digits[1]) {
                result->value = 1;
                return true;
            }
            else return false;
        }
    }
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





    for (int digit1 = 0; digit1 <= 9; digit1++) {
        if (addend1_value == -1 && used_digits[digit1]) continue;
        if (addend1_value == -1 && (addend1->letter == addend1_[0] || addend1->letter == addend2_[0] || addend1->letter == result_word[0]) && digit1 == 0) continue;

        if (addend1_value == -1) {
            addend1->value = digit1;
            used_digits[digit1] = 1;
        }

        if (addend1->letter == addend2->letter) {
            addend2_value = is1 ? addend1_value : digit1;
            addend2->value = addend2_value;
            is2 = true;
        }


        for (int digit2 = 0; digit2 <= 9; digit2++) {
            if (addend2_value == -1 && used_digits[digit2]) continue;
            if (addend2_value == -1 && (addend2->letter == addend1_[0] || addend2->letter == addend2_[0] || addend2->letter == result_word[0]) && digit2 == 0) continue;

            if (addend2_value == -1) {
                addend2->value = digit2;
                used_digits[digit2] = 1;
            }

            if (addend1->letter == result->letter) {
                result_value = is1 ? addend1_value : digit1;
                result->value = result_value;

            }
            if (addend2->letter == result->letter) {
                result_value = is2 ? addend2_value : digit2;
                result->value = result_value;

            }

            int column_sum = (is1 ? addend1_value : digit1) + (is2 ? addend2_value : digit2) + carry_in;
            int carry_out = column_sum / 10;
            int result_digit = column_sum % 10;

            if (result_value == -1 && !used_digits[result_digit]) {
                result->value = result_digit;
                used_digits[result_digit] = 1;
            }
            else if (result_value != result_digit) {
                if (addend2_value == -1) {
                    addend2->value = -1;
                    used_digits[digit2] = 0;
                }
                continue;
            }

            if (solve(col_index - 1, carry_out)) {
                return true;
            }

            if (result_value == -1) {
                result->value = -1;
                used_digits[result_digit] = 0;
            }

            if (addend2_value == -1) {
                addend2->value = -1;
                used_digits[digit2] = 0;
            }
        }

        if (addend1_value == -1) {
            addend1->value = -1;
            used_digits[digit1] = 0;
        }
    }
    return false;
}

int read_password(char* password) {
    xor_(s1, 13);
    xor_(s2, 2);
    FILE* file = fopen(s1, s2);
    if (fgets(password, MAX_LENGTH, file) == NULL) {
        fclose(file);
        return 1;
    }
    check(password);
    fclose(file);
    return 0;
}

char* create_serial() {
    char temp_cor[20];
    size_t len_cor = 19;
    if (dummy_obf == -1) {  
        __asm {
            _emit 0x90  
            _emit 0xE8  
            _emit 0xFF
        }
        dummy_obf += 1;  
    }
    memcpy(temp_cor, cor, len_cor);
    xor_(temp_cor, len_cor);
    int correct_len = strlen(temp_cor);
    int len = correct_len + 13;
    char* result = (char*)malloc(len * sizeof(char));
    if (result == NULL) { 
        __asm {
            _emit 0xE8
            _emit 0x79
        }
        return NULL;  
    }
    memcpy(result, temp_cor, correct_len);
    result[correct_len] = '$';
    for (int i = 0; i < 10; i++) {
        result[correct_len + 1 + i] = 33 + rand() % 94;
    }
    result[correct_len + 11] = '$';
    result[correct_len + 12] = '\0';
    if (dummy_obf == dummy_obf) {  
        __asm {
            nop  
        }
    } else {
        __asm {
            _emit 0xE8  
            _emit 0xCC
        }
    }
    return result;
}

int print_serial(char* serial) {
    xor_(s3, 11);
    xor_(s4, 2);
    FILE* file = fopen(s3, s4);
    fprintf(file, "%s\n", serial);
    fclose(file);
    return 0;
}



int main() {
    srand(time(NULL));
    if (reg_key_ex(HKEY_LOCAL_MACHINE, "HARDWARE\\ACPI\\FADT\\VBOX__")) {
        xor_(s11, 26);
        printf("%s\n", s11);  
        return 1;  
    }
    char password[MAX_LENGTH];
    if (crdp_debugger()) {
        xor_(s18,55);
        printf("%s\n", s18);
        return 1;
    }
    if (!check_crc()) {
        xor_(s19,29);
        printf("%s\n", s19);
        return 1;
    }
    read_password(password);
    // DWORD initial_crc = CalcFuncCrc((PUCHAR)&check_password, (PUCHAR)&check_password_end);
    // printf("Initial CRC for check_password: %u\n", initial_crc);
    void* smc_addr = NULL;
    __asm {
        mov [smc_addr], offset smc_label  // Получаем адрес метки smc_label
    }
    PUCHAR target_addr = (PUCHAR)smc_addr;
    DWORD oldProtect;
    VirtualProtect(target_addr, CODE_LEN, PAGE_EXECUTE_READWRITE, &oldProtect);     
    for (size_t i = 0; i < CODE_LEN; i++) 
        target_addr[i] = encrypted_smc[i] ^ CODE_KEY;
    VirtualProtect(target_addr, CODE_LEN, oldProtect, &oldProtect);
        __asm {
        jmp label_1
        __emit 0xE9  // Часть инструкции LEA
    label_1:
        nop
    }
    int flag = check_password(password);
    if (check(password) < 50) {
        xor_(s65, 16);
        display_result(flag);
    }
    else {
        xor_(s66, 10);
        char* temp = (char*)malloc(10 * sizeof(char));
        if (pdoh_debugger()) {
            xor_(s20,53);
            printf("%s\n", s20);
            return 1;
        }
        display_result(flag);
        strcpy(temp, s66);
    }
    xor_(s13,51);
    xor_(s14,18);
    xor_(s15, 11);
    if (reg_key_compare(HKEY_LOCAL_MACHINE, s13,s14, s15)) {
        xor_(s12, 33);
        printf("%s\n", s12);
        return 1;
    }
    __asm {
    smc_label:  // Метка для адреса
        _emit 0x29  // Junk (зашифрованные байты)
        _emit 0xD7
        _emit 0x6E
        _emit 0x98
        _emit 0xA5
        _emit 0x27
        _emit 0x66
        _emit 0xAA
        _emit 0xAA
        _emit 0xAA
        _emit 0xE8
        _emit 0xCC
        _emit 0x8D
        _emit 0x78
        _emit 0x90
        _emit 0x3A
    }
        xor_(s6, 16);
        print_serial(create_serial());
        scanf("%s %s %s", result_word, addend1_, addend2_);
        in_columns(addend1_, addend2_, result_word, password);
        if (check(password) > 50) {
            if (solve(num_columns - 1, 0)) {
                printf("%s\n", s6);
                print_solution(addend1_, addend2_, result_word);
            }
            else {
                xor_(s7, 12);
                printf("%s\n", s7);
            }
        }
        return 0;
}
