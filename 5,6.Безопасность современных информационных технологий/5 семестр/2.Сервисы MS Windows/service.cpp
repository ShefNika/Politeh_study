#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>
#include <zip.h> 
#include <atlstr.h>
#include <algorithm>
#include <shlwapi.h> 
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "zip.lib") 
#pragma comment(lib, "shlwapi.lib")

#define SERVICE_NAME _T("Archive_Service")
#define LOG_PATH     _T("C:\\ArchiveService\\service.log")

SERVICE_STATUS  g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE  g_StatusHandle = NULL;
HANDLE  g_ServiceStopEvent = NULL;
TCHAR  g_ConfigPath[MAX_PATH] = { 0 };

void    WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void    WINAPI ServiceCtrlHandler(DWORD CtrlCode);
void    AddToLog(const TCHAR* msg);
bool    ReadConfig(TCHAR* sourceDir, TCHAR* archivePath, std::vector<std::wstring>& masks);
bool    MatchMask(const std::wstring& filename, const std::wstring& mask);
void    BackupDirectory(const TCHAR* source, const TCHAR* archive, const std::vector<std::wstring>& masks);
int     InstallService(const TCHAR* exePath);
int     RemoveService();
int     StartServiceProc();
int     StopServiceProc();


void AddToLog(const TCHAR* msg) {
    FILE* f;
    if (_tfopen_s(&f, LOG_PATH, _T("a+, ccs=UTF-8")) == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        _ftprintf(f, _T("[%02d:%02d:%02d] %s\n"), st.wHour, st.wMinute, st.wSecond, msg);
        fclose(f);
    }
}


bool MatchMask(const std::wstring& filename, const std::wstring& mask) {
    size_t i = 0, j = 0, star = std::wstring::npos, match = 0;
    while (i < filename.size()) {
        if (j < mask.size() && (mask[j] == L'?' || mask[j] == filename[i])) {
            ++i; ++j;
        }
        else if (j < mask.size() && mask[j] == L'*') {
            star = j++;
            match = i;
        }
        else if (star != std::wstring::npos) {
            j = star + 1;
            match = ++i;
        }
        else return false;
    }
    while (j < mask.size() && mask[j] == L'*') ++j;
    return j == mask.size();
}


bool ReadConfig(TCHAR* sourceDir, TCHAR* archivePath, std::vector<std::wstring>& masks) {
    if (GetPrivateProfileString(_T("Backup"), _T("SourceDir"), _T(""), sourceDir, MAX_PATH, g_ConfigPath) == 0) {
        AddToLog(_T("ERROR: SourceDir not found in config"));
        return false;
    }
    if (GetPrivateProfileString(_T("Backup"), _T("ArchiveFile"), _T(""), archivePath, MAX_PATH, g_ConfigPath) == 0) {
        AddToLog(_T("ERROR: ArchiveFile not found in config"));
        return false;
    }

    for (int i = 0; i < 10; i++) {
        TCHAR key[32], mask[256];
        _stprintf(key, _T("FileMask%d"), i);
        if (GetPrivateProfileString(_T("Backup"), key, _T(""), mask, 256, g_ConfigPath) > 0) {
            masks.push_back(mask);
        }
        else break;
    }
    if (masks.empty()) {
        AddToLog(_T("ERROR: No FileMask found"));
        return false;
    }
    return true;
}


void BackupDirectory(const TCHAR* source, const TCHAR* archive, const std::vector<std::wstring>& masks) {
    int zip_err = 0;
    USES_CONVERSION; 

    zip_t* zip = zip_open(CT2A(archive), 0, &zip_err);
    if (!zip) {
        zip = zip_open(CT2A(archive), ZIP_CREATE, &zip_err);
    }
    namespace fs = std::filesystem;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(source)) {
            if (!entry.is_regular_file()) continue;

            std::wstring fullPath = entry.path().wstring();
            std::wstring filename = entry.path().filename().wstring();

            bool matches = false;
            for (const auto& mask : masks) {
                if (MatchMask(filename, mask)) {
                    matches = true;
                    break;
                }
            }
            if (!matches) continue;

            std::wstring relPath = fullPath;
            relPath.erase(0, wcslen(source) + 1);
            std::replace(relPath.begin(), relPath.end(), L'\\', L'/'); // относительный путь

            auto file_time = fs::last_write_time(entry);
            auto file_sys_time = std::chrono::clock_cast<std::chrono::system_clock>(file_time);
            auto file_time_t = std::chrono::system_clock::to_time_t(file_sys_time);

            // существует ли в архиве
            int index = zip_name_locate(zip, CT2A(relPath.c_str()), 0);

            if (index == -1) {
                // добавить новый файл
                zip_source_t* src = zip_source_file(zip, CT2A(fullPath.c_str()), 0, 0); //CT2A - преобразует const TCHAR* (Unicode) в const char* (ANSI) 
                if (src) {
                    zip_int64_t new_index = zip_file_add(zip, CT2A(relPath.c_str()), src, ZIP_FL_ENC_UTF_8);
                    if (new_index >= 0) {
                        zip_file_set_mtime(zip, new_index, file_time_t, 0);
                        TCHAR buf[512];
                        _stprintf_s(buf, _T("ADDED: %s"), relPath.c_str());
                        AddToLog(buf);
                    }
                    else {
                        zip_source_free(src);
                        AddToLog(_T("ERROR: Failed to add file to archive"));
                    }
                }
            }
            else {
                // время модификации из архива
                zip_stat_t zs;
                if (zip_stat_index(zip, index, 0, &zs) == 0) {
                    if (file_time_t > zs.mtime) {
                        zip_source_t* src = zip_source_file(zip, CT2A(fullPath.c_str()), 0, 0);
                        if (src && zip_file_replace(zip, index, src, 0) >= 0) {
                            zip_file_set_mtime(zip, index, file_time_t, 0);
                            //TCHAR buf[512];
                            //_stprintf_s(buf, _T("UPDATED: %s"), relPath.c_str());
                            //AddToLog(buf);
                        }
                        else {
                            if (src) zip_source_free(src);
                            AddToLog(_T("ERROR: Failed to update file in archive"));
                        }
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        AddToLog(CA2T(e.what()));
    }
    if (zip_close(zip)) {
        AddToLog(_T("ERROR: Failed to close archive"));
    }
    else {
        AddToLog(_T("Backup completed successfully."));
    }
}


void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    // Регистрирация обработчик команд службы
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (!g_StatusHandle) {
        AddToLog(_T("ERROR: RegisterServiceCtrlHandler failed"));
        return;
    }

    // Инициализация статуса службы
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 1;
    g_ServiceStatus.dwWaitHint = 3000;

    if (!SetServiceStatus(g_StatusHandle, &g_ServiceStatus)) {
        AddToLog(_T("ERROR: SetServiceStatus failed"));
        return;
    }

    // Получить путь к config.ini
    if (!GetModuleFileName(NULL, g_ConfigPath, MAX_PATH)) {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    PathRemoveFileSpec(g_ConfigPath);
    PathAppend(g_ConfigPath, _T("config.ini"));

    TCHAR sourceDir[MAX_PATH] = { 0 };
    TCHAR archiveFile[MAX_PATH] = { 0 };
    std::vector<std::wstring> masks;

    if (!ReadConfig(sourceDir, archiveFile, masks)) {
        AddToLog(_T("ERROR: Failed to read configuration"));
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        g_ServiceStatus.dwServiceSpecificExitCode = 1;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    DWORD dirAttr = GetFileAttributes(sourceDir);
    if (dirAttr == INVALID_FILE_ATTRIBUTES || !(dirAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        TCHAR buf[512];
        _stprintf_s(buf, _T("ERROR: Source directory does not exist: %s"), sourceDir);
        AddToLog(buf);
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }
    BackupDirectory(sourceDir, archiveFile, masks);

    // Настройка мониторинга директории
    HANDLE hChange = FindFirstChangeNotification(
        sourceDir,
        TRUE,  //  мониторим поддиректории
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE
    );

    if (hChange == INVALID_HANDLE_VALUE) {
        TCHAR buf[256];
        _stprintf_s(buf, _T("ERROR: Cannot monitor directory: %d"), GetLastError());
        AddToLog(buf);
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;

    if (!SetServiceStatus(g_StatusHandle, &g_ServiceStatus)) {
        AddToLog(_T("ERROR: SetServiceStatus failed in running state"));
    }
   

    while (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
        DWORD waitResult = WaitForSingleObject(hChange, 5000);

        if (waitResult == WAIT_OBJECT_0) {
            // Обнаружены изменения 
            AddToLog(_T("Directory changes detected, starting backup..."));
            BackupDirectory(sourceDir, archiveFile, masks);
            if (!FindNextChangeNotification(hChange)) { // сбрасывает уведомление для следующего ожидания
                AddToLog(_T("ERROR: FindNextChangeNotification failed"));
                break;
            }
        }
        else if (waitResult != WAIT_TIMEOUT) {
            AddToLog(_T("ERROR: WaitForSingleObject failed"));
            break;
        }
    }

    FindCloseChangeNotification(hChange); // завершение иониторинга
    AddToLog(_T("Service stopping..."));

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwControlsAccepted = 0;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    AddToLog(_T("Service stopped"));
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        AddToLog(_T("Stop command received"));
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwCheckPoint = 1;
        g_ServiceStatus.dwWaitHint = 3000;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        break;

    default:
        break;
    }
}


int InstallService(const TCHAR* exePath) {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        AddToLog(_T("ERROR: OpenSCManager failed"));
        return -1;
    }

    TCHAR fullPath[MAX_PATH];
    if (GetModuleFileName(NULL, fullPath, MAX_PATH) == 0) {
        AddToLog(_T("ERROR: GetModuleFileName failed"));
        CloseServiceHandle(hSCM);
        return -1;
    }
    TCHAR cmd[MAX_PATH + 3];
    _stprintf_s(cmd, _T("\"%s\""), fullPath);

    SC_HANDLE hService = CreateService(
        hSCM, SERVICE_NAME, SERVICE_NAME, // Имя и отображаемое имя
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,  // Уровень доступа, тип службы
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, // Запись вручную, реакция на ошибки (логирует, но продолжает)
        cmd, NULL, NULL, NULL, NULL, NULL); // путь к исполняемому файлу

    if (!hService) {
        DWORD err = GetLastError();
        TCHAR buf[256];
        _stprintf_s(buf, _T("ERROR: CreateService failed: %d"), err);
        AddToLog(buf);
        CloseServiceHandle(hSCM);
        return -1;
    }
    TCHAR buf[512];
    _stprintf_s(buf, _T("SUCCESS: Service installed with path: %s"), cmd);
    AddToLog(buf);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

int RemoveService() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) return -1;

    SC_HANDLE hService = OpenService(hSCM, SERVICE_NAME, SERVICE_STOP | DELETE);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return -1;
    }

    DeleteService(hService);
    AddToLog(_T("SUCCESS: Service removed"));
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

int StartServiceProc() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) {
        AddToLog(_T("ERROR: OpenSCManager failed in StartServiceProc"));
        return -1;
    }

    SC_HANDLE hService = OpenService(hSCM, SERVICE_NAME, SERVICE_START);
    if (!hService) {
        DWORD err = GetLastError();
        TCHAR buf[256];
        _stprintf_s(buf, _T("ERROR: OpenService failed: %d"), err);
        AddToLog(buf);
        CloseServiceHandle(hSCM);
        return -1;
    }

    if (!StartService(hService, 0, NULL)) {
        DWORD err = GetLastError();
        TCHAR buf[256];
        _stprintf_s(buf, _T("ERROR: StartService failed: %d"), err);
        AddToLog(buf);
    }
    else {
        AddToLog(_T("SUCCESS: Service started"));
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

int StopServiceProc() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    SC_HANDLE hService = OpenService(hSCM, SERVICE_NAME, SERVICE_STOP);
    SERVICE_STATUS ss;
    if (ControlService(hService, SERVICE_CONTROL_STOP, &ss)) {
        AddToLog(_T("SUCCESS: Service stopped"));
    }
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}


int _tmain(int argc, _TCHAR* argv[]) {
    if (argc == 1) {
        SERVICE_TABLE_ENTRY ste[] = {
            { (LPTSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
            { NULL, NULL }
        };
        StartServiceCtrlDispatcher(ste);
    }
    else if (_tcscmp(argv[1], _T("install")) == 0) InstallService(argv[0]);
    else if (_tcscmp(argv[1], _T("remove")) == 0) RemoveService();
    else if (_tcscmp(argv[1], _T("start")) == 0) StartServiceProc();
    else if (_tcscmp(argv[1], _T("stop")) == 0) StopServiceProc();

    return 0;
}