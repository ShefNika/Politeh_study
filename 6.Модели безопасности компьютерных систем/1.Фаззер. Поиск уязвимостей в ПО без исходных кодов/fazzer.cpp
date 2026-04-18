#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h> 
#include <windows.h>
#include <tlhelp32.h>


#define MAX_FILE_SIZE 1024 * 1024  
#define MAX_BUFFER_SIZE 4096         
#define LOG_FILE "fuzzing_log.txt"   
#define CRASH_LOG "crashes.log" 
#define MAX_SEEN 10000
int baseline_coverage = 0;
int seen_coverages[MAX_SEEN];
int seen_count = 0;



typedef struct {
    bool crash_detected;
    int error_code;
    unsigned long crash_address;
    char registers[256];
    char mutation_info[512];
    char timestamp[64];
} CrashInfo;

typedef struct {
    char target_exe[256];
    char target_config[256];
    char original_config[256];
    uint8_t* file_buffer;
    size_t file_size;
    FILE* log_file;
} FuzzerConfig;


FuzzerConfig config = {
    "vuln11.exe",
    "config_11",
    "config_11.original",
    NULL,
    0,
    NULL
};


void print_menu(void);
void log_message(const char* format, ...);
void log_crash(CrashInfo* crash_info);
int read_file(const char* filename, uint8_t** buffer, size_t* size);
int write_file(const char* filename, uint8_t* buffer, size_t size);
void restore_original_config(void);
void change_one_byte(void);
void change_sequence(void);
void add_byte(void);
void add_sequence(void);
void automatic_mode(void);
void find_separators(void);
void increase_buffer_size(void);
void increase_line_length(void);
int run_target_program(CrashInfo* crash_info);
void get_current_time(char* buffer, size_t size);



void print_menu(void) {
    printf("MENU:\n");
    printf("----------------------------------------\n");
    printf(" 1. Change one byte\n");
    printf(" 2. Change sequence of bytes\n");
    printf(" 3. Add one byte\n");
    printf(" 4. Add sequence of bytes\n");
    printf(" 5. Automatic mode (sequential byte replacement)\n");
    printf(" 6. Find separators (\",:=;\")\n");
    printf(" 7. Increase buffer size (append to end)\n");
    printf(" 8. Increase line length\n");
    printf(" 9. Run target program\n");
    printf("10. Restore original configuration\n");
    printf("11. DBI MODE with DynamoRIO\n");
    printf(" 0. Exit\n");
    printf("----------------------------------------\n");
}


void get_current_time(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}


void log_message(const char* format, ...) {
    if (!config.log_file) return;

    char timestamp[64];
    get_current_time(timestamp, sizeof(timestamp));

    fprintf(config.log_file, "[%s] ", timestamp);

    va_list args;
    va_start(args, format);
    vfprintf(config.log_file, format, args);
    va_end(args);

    fprintf(config.log_file, "\n");
    fflush(config.log_file);
}


void log_crash(CrashInfo* crash_info) {
    if (!crash_info->crash_detected) return;

    FILE* crash_log = fopen(CRASH_LOG, "a");
    if (!crash_log) return;

    fprintf(crash_log, "========================================\n");
    fprintf(crash_log, "Timestamp: %s\n", crash_info->timestamp);
    fprintf(crash_log, "Error code: 0x%08X\n", crash_info->error_code);
    fprintf(crash_log, "Registers/Info: %s\n", crash_info->registers);
    fprintf(crash_log, "Mutation: %s\n", crash_info->mutation_info);
    fprintf(crash_log, "========================================\n\n");

    fclose(crash_log);
    log_message("CRASH DETECTED! Error code: 0x%08X, Mutation: %s",
        crash_info->error_code, crash_info->mutation_info);
}


int read_file(const char* filename, uint8_t** buffer, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (*size > MAX_FILE_SIZE) {
        printf("Error: File too large\n", MAX_FILE_SIZE);
        fclose(file);
        return 0;
    }
    *buffer = (uint8_t*)malloc(*size);
    size_t read_bytes = fread(*buffer, 1, *size, file);
    fclose(file);
    return read_bytes == *size;
}


int write_file(const char* filename, uint8_t* buffer, size_t size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        log_message("ERROR: Cannot write file %s", filename);
        return 0;
    }
    size_t written = fwrite(buffer, 1, size, file);
    fclose(file);
    return written == size;
}


void restore_original_config(void) {
    uint8_t* orig_buffer;
    size_t orig_size;

    if (read_file(config.original_config, &orig_buffer, &orig_size)) {
        free(config.file_buffer);
        config.file_buffer = orig_buffer;
        config.file_size = orig_size;
        write_file(config.target_config, config.file_buffer, config.file_size);
        log_message("Configuration restored to original state");
    }
    else {
        log_message("ERROR: Failed to restore original configuration");
    }
}


void change_one_byte(void) {
    size_t offset;
    int byte_value;
    printf("Enter offset from file start (0-%zu): ", config.file_size - 1);
    scanf("%zu", &offset);
    if (offset >= config.file_size) {
        printf("Error: Offset beyond file size!\n");
        return;
    }
    if (offset == 0) {
        printf("WARNING: Changing variant doesn't allow the program to start!\n");
        printf("Continue? (1-yes, 0-no): ");
        int confirm;
        scanf("%d", &confirm);
        if (!confirm) {
            return;
        }
    }
    printf("Enter new byte value (0-255): ");
    scanf("%d", &byte_value);
    if (byte_value < 0 || byte_value > 255) {
        printf("Error: Value must be 0-255\n");
        return;
    }
    uint8_t original = config.file_buffer[offset];
    config.file_buffer[offset] = (uint8_t)byte_value;
    write_file(config.target_config, config.file_buffer, config.file_size);
    printf("Byte at offset %zu changed: 0x%02X -> 0x%02X\n",
        offset, original, config.file_buffer[offset]);
    log_message("Manual mutation: changed byte[%zu] from 0x%02X to 0x%02X",
        offset, original, config.file_buffer[offset]);
}


void change_sequence(void) {
    size_t offset;
    int len;
    uint8_t new_bytes[MAX_BUFFER_SIZE];

    printf("Enter offset from file start (0-%zu): ", config.file_size - 1);
    scanf("%zu", &offset);
    if (offset >= config.file_size) {
        printf("Error: Offset beyond file size!\n");
        return;
    }
    printf("Enter sequence length: ");
    scanf("%d", &len);
    if (len <= 0 || len > MAX_BUFFER_SIZE || offset + len > config.file_size) {
        printf("Error: Invalid length\n");
        return;
    }
    printf("Enter %d bytes in hex with spaces (ex: 32 AF 47):\n", len);
    for (int i = 0; i < len; i++) {
        int val;
        scanf("%x", &val);
        new_bytes[i] = (uint8_t)val;
    }
    char orig_hex[256] = { 0 };
    char new_hex[256] = { 0 };
    for (int i = 0; i < len && i < 32; i++) {
        char temp[8];
        snprintf(temp, sizeof(temp), "%02X ", config.file_buffer[offset + i]);
        strcat(orig_hex, temp);
        snprintf(temp, sizeof(temp), "%02X ", new_bytes[i]);
        strcat(new_hex, temp);
    }

    memcpy(&config.file_buffer[offset], new_bytes, len);
    write_file(config.target_config, config.file_buffer, config.file_size);
    printf("Sequence changed\n");
    log_message("Manual mutation: changed %d bytes at offset %zu [%s] -> [%s]",
        len, offset, orig_hex, new_hex);
}


void add_byte(void) {
    int choice;
    size_t offset;
    int byte_value;
    uint8_t* new_buffer;

    printf("Where to add byte?\n");
    printf("1 - Append to end\n");
    printf("2 - Insert at offset\n");
    printf("Choice: ");
    scanf("%d", &choice);
    printf("Enter byte value (0-255): ");
    scanf("%d", &byte_value);

    if (choice == 1) {
        new_buffer = (uint8_t*)malloc(config.file_size + 1);
        memcpy(new_buffer, config.file_buffer, config.file_size);
        new_buffer[config.file_size] = (uint8_t)byte_value;
        free(config.file_buffer);
        config.file_buffer = new_buffer;
        config.file_size++;
        printf("Byte 0x%02X appended to file\n", byte_value);
        log_message("Manual mutation: appended byte 0x%02X to end", byte_value);
    }
    else if (choice == 2) {
        printf("Enter insertion offset (0-%zu): ", config.file_size);
        scanf("%zu", &offset);
        if (offset > config.file_size) {
            printf("Error: Invalid offset\n");
            return;
        }
        new_buffer = (uint8_t*)malloc(config.file_size + 1);
        memcpy(new_buffer, config.file_buffer, offset);
        new_buffer[offset] = (uint8_t)byte_value;
        memcpy(&new_buffer[offset + 1], &config.file_buffer[offset],
            config.file_size - offset);
        free(config.file_buffer);
        config.file_buffer = new_buffer;
        config.file_size++;
        printf("Byte 0x%02X inserted at offset %zu\n", byte_value, offset);
        log_message("Manual mutation: inserted byte 0x%02X at offset %zu",
            byte_value, offset);
    }
    else {
        printf("Invalid choice\n");
        return;
    }
    write_file(config.target_config, config.file_buffer, config.file_size);
}


void add_sequence(void) {
    int choice;
    size_t offset;
    int len;
    uint8_t new_bytes[MAX_BUFFER_SIZE];
    uint8_t* new_buffer;
    printf("Where to add sequence?\n");
    printf("1 - Append to end\n");
    printf("2 - Insert at offset\n");
    printf("Choice: ");
    scanf("%d", &choice);
    printf("Enter sequence length: ");
    scanf("%d", &len);
    if (len <= 0 || len > MAX_BUFFER_SIZE) {
        printf("Error: Invalid length\n");
        return;
    }
    printf("Enter %d bytes in hex format:\n", len);
    for (int i = 0; i < len; i++) {
        int val;
        scanf("%x", &val);
        new_bytes[i] = (uint8_t)val;
    }
    char hex_bytes[256] = { 0 };
    for (int i = 0; i < len && i < 32; i++) {
        char temp[8];
        snprintf(temp, sizeof(temp), "%02X ", new_bytes[i]);
        strcat(hex_bytes, temp);
    }

    if (choice == 1) {
        new_buffer = (uint8_t*)malloc(config.file_size + len);
        memcpy(new_buffer, config.file_buffer, config.file_size);
        memcpy(&new_buffer[config.file_size], new_bytes, len);

        free(config.file_buffer);
        config.file_buffer = new_buffer;
        config.file_size += len;

        printf("Sequence appended to file\n");
        log_message("Manual mutation: appended %d bytes [%s] to end", len, hex_bytes);
    }
    else if (choice == 2) {
        printf("Enter insertion offset (0-%zu): ", config.file_size);
        scanf("%zu", &offset);

        if (offset > config.file_size) {
            printf("Error: Invalid offset\n");
            return;
        }

        new_buffer = (uint8_t*)malloc(config.file_size + len);
        memcpy(new_buffer, config.file_buffer, offset);
        memcpy(&new_buffer[offset], new_bytes, len);
        memcpy(&new_buffer[offset + len], &config.file_buffer[offset],
            config.file_size - offset);

        free(config.file_buffer);
        config.file_buffer = new_buffer;
        config.file_size += len;

        printf("Sequence inserted at offset %zu\n", offset);
        log_message("Manual mutation: inserted %d bytes [%s] at offset %zu",
            len, hex_bytes, offset);
    }
    else {
        printf("Invalid choice\n");
        return;
    }

    write_file(config.target_config, config.file_buffer, config.file_size);
}


void find_separators(void) {
    const char* separators = ",:=;";
    int found = 0;
    for (size_t i = 0; i < config.file_size; i++) {
        char current = (char)config.file_buffer[i];
        if (current == ',' || current == ':' || current == '=' || current == ';') {
            printf("Position %5zu: '%c' (0x%02X)\n", i, current, current);
            found++;
        }
    }

    if (found == 0) 
        printf("No separators found.\n");
    else 
        printf("Total found: %d separators\n", found);
    log_message("Found %d separators in file", found);
}


void increase_buffer_size(void) {
    int increase_by;
    uint8_t* new_buffer;
    printf("Current file size: %zu bytes\n", config.file_size);
    printf("How many bytes to append?");
    scanf("%d", &increase_by);
    if (increase_by <= 0 || increase_by > MAX_FILE_SIZE - config.file_size) {
        printf("Error: Invalid value\n");
        return;
    }
    new_buffer = (uint8_t*)malloc(config.file_size + increase_by);
    memcpy(new_buffer, config.file_buffer, config.file_size);
    for (int i = 0; i < increase_by; i++) {
        switch (i % 4) {
        case 0: new_buffer[config.file_size + i] = 0x41; break;  
        case 1: new_buffer[config.file_size + i] = 0xFF; break;
        case 2: new_buffer[config.file_size + i] = 0x00; break;
        case 3: new_buffer[config.file_size + i] = 0x7F; break;
        }
    }

    free(config.file_buffer);
    config.file_buffer = new_buffer;
    config.file_size += increase_by;

    write_file(config.target_config, config.file_buffer, config.file_size);

    printf("File increased by %d bytes. New size: %zu bytes\n",
        increase_by, config.file_size);
    log_message("Increased file size by %d bytes (buffer overflow test)", increase_by);
}


void increase_line_length(void) {
    uint8_t* new_buffer;
    size_t new_size = config.file_size;
    int strings_found = 0;
    printf("Searching for strings\n");

    for (size_t i = 0; i < config.file_size; i++) {
        if (config.file_buffer[i] >= 32 && config.file_buffer[i] <= 126) {
            size_t start = i;
            size_t len = 0;
            while (i < config.file_size &&
                config.file_buffer[i] >= 32 &&
                config.file_buffer[i] <= 126) {
                len++;
                i++;
            }
            if (len > 3) {
                strings_found++;
                new_size += len;
                printf("Found string at offset %zu, length %zu\n", start, len);
            }
        }
    }
    if (strings_found == 0) {
        printf("No strings found.\n");
        return;
    }
    printf("Found %d strings\n", strings_found);
    new_buffer = (uint8_t*)malloc(new_size);
    size_t src_pos = 0;
    size_t dst_pos = 0;
    while (src_pos < config.file_size) {
        if (config.file_buffer[src_pos] >= 32 && config.file_buffer[src_pos] <= 126) {
            size_t start = src_pos;
            size_t len = 0;
            while (src_pos < config.file_size &&
                config.file_buffer[src_pos] >= 32 &&
                config.file_buffer[src_pos] <= 126) {
                len++;
                src_pos++;
            }
            if (len > 3) {
                memcpy(&new_buffer[dst_pos], &config.file_buffer[start], len);
                dst_pos += len;
                for (size_t i = 0; i < len; i++) 
                    new_buffer[dst_pos++] = 'A';
                log_message("Lengthened string at offset %zu: %zu -> %zu chars",
                    start, len, len * 2);
            }
            else {
                memcpy(&new_buffer[dst_pos], &config.file_buffer[start], len);
                dst_pos += len;
            }
        }
        else 
            new_buffer[dst_pos++] = config.file_buffer[src_pos++];
    }

    free(config.file_buffer);
    config.file_buffer = new_buffer;
    config.file_size = new_size;

    write_file(config.target_config, config.file_buffer, config.file_size);

    printf("Done! New file size: %zu bytes\n", config.file_size);
    log_message("Lengthened %d strings, new file size: %zu bytes",
        strings_found, config.file_size);
}


void automatic_mode(void) {
    uint8_t boundary_values[] = {
        0x00, 0xFF, 0x7F, 0x80, 0x01, 0xFE
    };
    uint16_t boundary_16bit[] = {
        0x0000, 0xFFFF, 0x7FFF, 0x8000, 0x7FFF + 1, 0x7FFF - 1
    };

    int num_values = sizeof(boundary_values) / sizeof(boundary_values[0]);
    int num_16bit = sizeof(boundary_16bit) / sizeof(boundary_16bit[0]);

    printf("Starting automatic mode...\n");
    printf("Total iterations: %zu positions x %d values\n",
        config.file_size, num_values + num_16bit);
    printf("Press any key to stop\n");

    log_message("Starting automatic fuzzing mode");
    log_message("Testing %zu positions with %d boundary values",
        config.file_size, num_values + num_16bit);

    uint8_t* original = (uint8_t*)malloc(config.file_size);
    memcpy(original, config.file_buffer, config.file_size);

    int crash_count = 0;
    bool stop = false;

    for (size_t pos = 1; pos < config.file_size && !stop; pos++) {
        if (_kbhit()) {
            _getch();
            stop = true;
            log_message("Automatic fuzzing interrupted at position %zu", pos);
            break;
        }
        for (int v = 0; v < num_values; v++) {
            memcpy(config.file_buffer, original, config.file_size);
            config.file_buffer[pos] = boundary_values[v];
            write_file(config.target_config, config.file_buffer, config.file_size);

            CrashInfo crash = { 0 };
            get_current_time(crash.timestamp, sizeof(crash.timestamp));
            snprintf(crash.mutation_info, sizeof(crash.mutation_info),
                "auto: changed byte[%zu] to 0x%02X", pos, boundary_values[v]);
            if (run_target_program(&crash)) {
                crash_count++;
                log_crash(&crash);
            }
        }
        if (pos < config.file_size - 1) {
            for (int v = 0; v < num_16bit; v++) {
                memcpy(config.file_buffer, original, config.file_size);
                config.file_buffer[pos] = (boundary_16bit[v] >> 8) & 0xFF;
                config.file_buffer[pos + 1] = boundary_16bit[v] & 0xFF;
                write_file(config.target_config, config.file_buffer, config.file_size);

                CrashInfo crash = { 0 };
                get_current_time(crash.timestamp, sizeof(crash.timestamp));
                snprintf(crash.mutation_info, sizeof(crash.mutation_info),
                    "auto: changed bytes[%zu-%zu] to 0x%04X",
                    pos, pos + 1, boundary_16bit[v]);
                if (run_target_program(&crash)) {
                    crash_count++;
                    log_crash(&crash);
                }
            }
        }
    }

    memcpy(config.file_buffer, original, config.file_size);
    write_file(config.target_config, config.file_buffer, config.file_size);
    free(original);

    log_message("Automatic fuzzing completed. Total crashes: %d", crash_count);
    printf("\n\nAutomatic mode completed!\n");
    printf("Total crashes detected: %d\n", crash_count);
}


int run_target_program(CrashInfo* crash_info) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char cmd_line[512];
    snprintf(cmd_line, sizeof(cmd_line), "%s %s",
        config.target_exe, config.target_config);
    if (!CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE,
        DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi)) {
        log_message("ERROR: Failed to start process: %lu", GetLastError());
        return 0;
    }
    DEBUG_EVENT debug_event;
    bool crash_detected = false;
    DWORD continue_status = DBG_CONTINUE;
    DWORD exit_code = 0;

    DWORD start_time = GetTickCount();
    const DWORD MAX_EXECUTION_TIME = 5000;

    while (true) {
        if (WaitForDebugEvent(&debug_event, 100)) {
            switch (debug_event.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT: {
                EXCEPTION_RECORD* exception = &debug_event.u.Exception.ExceptionRecord;
                exit_code = exception->ExceptionCode;
                if (exception->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
                    exception->ExceptionCode == EXCEPTION_STACK_OVERFLOW ||
                    exception->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO ||
                    exception->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
                    crash_detected = true;
                    crash_info->crash_detected = true;
                    crash_info->error_code = exception->ExceptionCode;
                    crash_info->crash_address = (unsigned long)exception->ExceptionAddress;
                    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
                        debug_event.dwThreadId);
                    if (hThread) {
                        CONTEXT context;
                        context.ContextFlags = CONTEXT_FULL;
                        if (GetThreadContext(hThread, &context)) {
                            char registers[2048] = { 0 };
                            snprintf(registers, sizeof(registers),
                                "EAX=0x%08X EBX=0x%08X ECX=0x%08X EDX=0x%08X\n"
                                "ESI=0x%08X EDI=0x%08X EBP=0x%08X ESP=0x%08X\n"
                                "EIP=0x%08X EFLAGS=0x%08X\n",
                                context.Eax, context.Ebx, context.Ecx, context.Edx,
                                context.Esi, context.Edi, context.Ebp, context.Esp,
                                context.Eip, context.EFlags);
                            if (context.Esp) {
                                DWORD stack[8];
                                SIZE_T bytes_read;
                                if (ReadProcessMemory(pi.hProcess, (LPCVOID)context.Esp,
                                    stack, sizeof(stack), &bytes_read)) {
                                    strcat(registers, "\nStack:\n");
                                    for (int i = 0; i < bytes_read / 4; i++) {
                                        char line[64];
                                        snprintf(line, sizeof(line),
                                            "  [ESP+%d] = 0x%08X", i * 4, stack[i]);
                                        strcat(registers, line);
                                        strcat(registers, "\n");
                                    }
                                }
                            }
                            strncpy(crash_info->registers, registers,
                                sizeof(crash_info->registers) - 1);
                        }
                        CloseHandle(hThread);
                    }
                    continue_status = DBG_EXCEPTION_NOT_HANDLED;
                }
                else {
                    continue_status = DBG_EXCEPTION_NOT_HANDLED;
                }
                break;
            }
            case EXIT_PROCESS_DEBUG_EVENT: {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                if (crash_detected) 
                    return 1;
                return 0;
            }
            }
            ContinueDebugEvent(debug_event.dwProcessId,
                debug_event.dwThreadId,
                continue_status);
            continue_status = DBG_CONTINUE;
        }
    }
    return 0;
}


void delete_drcov_logs(void) {
    WIN32_FIND_DATAA find_data;
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "drcov.%s.*.log", config.target_exe);
    HANDLE find = FindFirstFileA(pattern, &find_data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
                DeleteFileA(find_data.cFileName);
        } while (FindNextFileA(find, &find_data));
        FindClose(find);
    }
}


int run_with_dynamorio(CrashInfo* crash_info, int* coverage) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmd_line[1024];

    *coverage = 0;

    delete_drcov_logs();

    snprintf(cmd_line, sizeof(cmd_line),"cmd /c \"C:\\DynamoRIO\\bin32\\drrun.exe\" -t drcov -- %s", config.target_exe);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return 0;

    DWORD wait = WaitForSingleObject(pi.hProcess, 7000);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (wait == WAIT_TIMEOUT) {
        crash_info->crash_detected = true;
        crash_info->error_code = 0xDEAD;
        return 1;
    }
    WIN32_FIND_DATAA find_data;
    HANDLE find = FindFirstFileA("drcov.*.log", &find_data);
    if (find != INVALID_HANDLE_VALUE) {
        FILE* f = fopen(find_data.cFileName, "r");
        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                if (strstr(line, "BB Table:")) {
                    sscanf(line, "BB Table: %d bbs", coverage);
                    break;
                }
            }
            fclose(f);
        }
        DeleteFileA(find_data.cFileName);
        FindClose(find);
    }
    if (exit_code != 0)
    {
        crash_info->crash_detected = true;
        crash_info->error_code = exit_code;
        return 1;
    }
    return 0;
}



void get_baseline_coverage(void) {

    CrashInfo dummy = { 0 };
    get_current_time(dummy.timestamp, sizeof(dummy.timestamp));
    int result = run_with_dynamorio(&dummy, &baseline_coverage);
    if (result) {;
        printf("Check if config_11 is correct and program runs normally.\n");
        baseline_coverage = 0;
    }
    printf("Baseline coverage: %d basic blocks\n", baseline_coverage);
    log_message("Baseline coverage: %d blocks", baseline_coverage);
}


void automatic_mode_dbi(void) {

    get_baseline_coverage();
    seen_coverages[seen_count++] = baseline_coverage;
    uint8_t boundary_values[] = { 0x00,0xFF,0x7F,0x80,0x01,0xFE };
    int num_values = sizeof(boundary_values) / sizeof(boundary_values[0]);

    printf("Starting automatic DBI mode...\n");
    printf("Total iterations: %zu positions x %d values\n",
        config.file_size, num_values);
    printf("Press any key to stop\n");

    log_message("Starting automatic DBI fuzzing mode");
    log_message("Testing %zu positions with %d boundary values",
        config.file_size, num_values);

    uint8_t* original = (uint8_t*)malloc(config.file_size);
    memcpy(original, config.file_buffer, config.file_size);

    int crash_count = 0;
    int new_paths = 0;
    bool stop = false;

    for (size_t pos = 1; pos < config.file_size && !stop; pos++) {

        for (int v = 0; v < num_values; v++) {
            if (_kbhit()) {
                _getch();
                stop = true;
                log_message("Automatic fuzzing interrupted at position %zu", pos);
                break;
            }
            memcpy(config.file_buffer, original, config.file_size);
            config.file_buffer[pos] = boundary_values[v];
            write_file(config.target_config, config.file_buffer, config.file_size);

            CrashInfo crash = { 0 };
            int coverage = 0;

            get_current_time(crash.timestamp, sizeof(crash.timestamp));
            int result = run_with_dynamorio(&crash, &coverage);
            if (result) {
                crash_count++;
                char name[64];
                snprintf(name, sizeof(name),
                    "crash_%zu_%02X.bin", pos, boundary_values[v]);
                write_file(name, config.file_buffer, config.file_size);
                snprintf(crash.mutation_info, sizeof(crash.mutation_info),
                    "auto: changed byte[%zu] to 0x%02X", pos, boundary_values[v]);
                log_crash(&crash);
                log_message("CRASH at %zu value 0x%02X\n", pos, boundary_values[v]);
                continue;
            }
            bool is_new = true;
            for (int i = 0; i < seen_count; i++)
                if (seen_coverages[i] == coverage)
                    is_new = false;
            if (is_new && seen_count < MAX_SEEN) {
                seen_coverages[seen_count++] = coverage;
                new_paths++;
                char name[64];
                snprintf(name, sizeof(name),
                    "path_%zu_%02X.bin", pos, boundary_values[v]);
                write_file(name, config.file_buffer, config.file_size);
                printf("NEW PATH: coverage %d\n", coverage);
            }
        }
    }
    memcpy(config.file_buffer, original, config.file_size);
    write_file(config.target_config, config.file_buffer, config.file_size);
    free(original);

    log_message("Automatic fuzzing with DBI completed. Total crashes: %d", crash_count);
    printf("\n\nAutomatic DBI mode completed!\n");
    printf("Total crashes detected: %d\n", crash_count);
    printf("New paths: %d\n", new_paths);
}


int main(void) {
    int choice;
    config.log_file = fopen(LOG_FILE, "a");
    log_message("Session started");
    if (!read_file(config.target_config, &config.file_buffer, &config.file_size)) {
        printf("Error: Cannot read file %s\n", config.target_config);
        if (config.log_file) fclose(config.log_file);
        return 1;
    }
    write_file(config.original_config, config.file_buffer, config.file_size);
    log_message("Loaded file: %s (size: %zu bytes)", config.target_config, config.file_size);
    printf("All test results will be logged to: %s and %s\n", LOG_FILE, CRASH_LOG);

    while (1) {
        print_menu();
        printf("\nYour choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input!\n");
            continue;
        }
        printf("\n");
        switch (choice) {
        case 1:
            change_one_byte();
            break;
        case 2:
            change_sequence();
            break;
        case 3:
            add_byte();
            break;
        case 4:
            add_sequence();
            break;
        case 5:
            automatic_mode();
            break;
        case 6:
            find_separators();
            break;
        case 7:
            increase_buffer_size();
            break;
        case 8:
            increase_line_length();
            break;
        case 9:
        {
            printf("Running target program...\n");
            CrashInfo crash = { 0 };
            if (run_target_program(&crash)) {
                printf("Crash detected! Check %s for details.\n", CRASH_LOG);
            }
            else {
                printf("Program finished normally.\n");
            }
        }
        break;
        case 10:
            restore_original_config();
            printf("Configuration restored to initial state.\n");
            break;
        case 11:
            automatic_mode_dbi();
            break;
        case 0:
            printf("\nRestore original configuration? (1-yes, 0-no): ");
            int restore;
            scanf("%d", &restore);
            if (restore) {
                restore_original_config();
                printf("Configuration restored.\n");
            }
            log_message("Fuzzer session ended\n");
            free(config.file_buffer);
            if (config.log_file) fclose(config.log_file);
            return 0;
        default:
            printf("Invalid choice!\n");
        }
        printf("\nPress Enter to continue...");
        while (getchar() != '\n');
        getchar();
    }
    return 0;
}

