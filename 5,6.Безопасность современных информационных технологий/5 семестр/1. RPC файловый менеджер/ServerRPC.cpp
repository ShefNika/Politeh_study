#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "advapi32.lib")

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "InterfaceRPC.h"

using namespace std;

HANDLE g_UserToken = NULL;

void* __RPC_USER MIDL_user_allocate(size_t size) { return malloc(size); }
void __RPC_USER MIDL_user_free(void* p) { free(p); }

int OpenSession(handle_t hBinding, unsigned char* login, unsigned char* password, int* servErrorCode)
{
    *servErrorCode = 0;
    cout << "[SERVER] OpenSession: user=" << login << endl;
    if (g_UserToken != NULL)
    {
        cout << "[SERVER] Session already open!" << endl;
        *servErrorCode = ERROR_ALREADY_ASSIGNED;
        return -1;
    }
    HANDLE hToken = NULL;
    if (!LogonUserA((LPCSTR)login, NULL, (LPCSTR)password, // LPSTR - unsigned char* строки с нулем
        LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken))
    {
        *servErrorCode = GetLastError();
        cout << "[SERVER] LogonUser failed: " << *servErrorCode << endl;
        return -1;
    }
    g_UserToken = hToken;
    cout << "[SERVER] Authentication success for " << login << endl;
    return 0;
}

int CloseSession(handle_t hBinding, int* servErrorCode)
{
    *servErrorCode = 0;
    if (g_UserToken)
    {
        cout << "[SERVER] Closing session" << endl;
        CloseHandle(g_UserToken); // Освобождение дескриптора
        g_UserToken = NULL;
    }
    else
        cout << "[SERVER] No open session" << endl;
    return 0;
}

int CopyToServer(handle_t hBinding, unsigned char* servUNCPath, long fileSize, unsigned char fileData[], int* servErrorCode)
{
    *servErrorCode = 0;
    if (!g_UserToken)
    {
        cout << "[SERVER] No active session" << endl;
        *servErrorCode = ERROR_INVALID_HANDLE;
        return -1;
    }
    if (!ImpersonateLoggedOnUser(g_UserToken))
    {
        *servErrorCode = GetLastError();
        cout << "[SERVER] ImpersonateLoggedOnUser failed: " << *servErrorCode << endl;
        return -1;
    }
    ofstream file((char*)servUNCPath, ios::binary);
    if (!file.is_open())
    {
        *servErrorCode = errno;
        cout << "[SERVER] Failed to open file for write: " << servUNCPath << endl;
        RevertToSelf();
        return -1;
    }
    file.write((char*)fileData, fileSize);
    file.close();
    RevertToSelf();
    cout << "[SERVER] File uploaded: " << servUNCPath << endl;
    return 0;
}

long FileSize(handle_t hBinding, unsigned char* Path)
{
    if (!g_UserToken)
    {
        cout << "[SERVER] No active session" << endl;
        return 0;
    }
    if (!ImpersonateLoggedOnUser(g_UserToken))
    {
        cout << "[SERVER] ImpersonateLoggedOnUser failed" << endl;
        return 0;
    }
    ifstream file((char*)Path, ios::binary | ios::ate);
    if (!file.is_open())
    {
        cout << "[SERVER] Cannot open file for size: " << Path << endl;
        RevertToSelf();
        return 0;
    }
    long size = (long)file.tellg();
    file.close();
    RevertToSelf();
    cout << "[SERVER] FileSize returned: " << Path << " (" << size << " bytes)" << endl;
    return size;
}

int DownloadFromServer(handle_t hBinding, unsigned char* servUNCPath, long fileSize, unsigned char* Arr, int* servErrorCode)
{
    *servErrorCode = 0;
    if (!g_UserToken)
    {
        cout << "[SERVER] No active session" << endl;
        *servErrorCode = ERROR_INVALID_HANDLE;
        return -1;
    }
    if (!ImpersonateLoggedOnUser(g_UserToken))
    {
        *servErrorCode = GetLastError();
        cout << "[SERVER] ImpersonateLoggedOnUser failed: " << *servErrorCode << endl;
        return -1;
    }
    ifstream file((char*)servUNCPath, ios::binary);
    if (!file.is_open())
    {
        *servErrorCode = GetLastError();
        cout << "[SERVER] Cannot open file for read: " << servUNCPath << endl;
        RevertToSelf();
        return -1;
    }

    file.read((char*)Arr, fileSize);
    if (file.fail() || file.gcount() != fileSize) {
        *servErrorCode = GetLastError();
        cout << "[SERVER] Read incomplete: expected " << fileSize << ", read " << file.gcount() << endl;
        file.close();
        RevertToSelf();
        return -1;
    }
    file.close();
    RevertToSelf();
    cout << "[SERVER] File downloaded: " << servUNCPath << " (" << fileSize << " bytes)" << endl;
    return 0;
}

int DeleteOnServer(handle_t hBinding, unsigned char* servUNCPath, int* servErrorCode)
{
    *servErrorCode = 0;
    if (!g_UserToken)
    {
        cout << "[SERVER] No active session" << endl;
        *servErrorCode = ERROR_INVALID_HANDLE;
        return -1;
    }
    if (!ImpersonateLoggedOnUser(g_UserToken))
    {
        *servErrorCode = GetLastError();
        cout << "[SERVER] ImpersonateLoggedOnUser failed: " << *servErrorCode << endl;
        return -1;
    }
    if (remove((char*)servUNCPath) != 0)
    {
        *servErrorCode = errno;
        cout << "[SERVER] Delete failed for " << servUNCPath << endl;
        RevertToSelf();
        return -1;
    }
    RevertToSelf();
    cout << "[SERVER] File deleted: " << servUNCPath << endl;
    return 0;
}

int main()
{
    RPC_STATUS status;
    status = RpcServerUseProtseqEpA((RPC_CSTR)"ncacn_ip_tcp", // настройка протокола и эндпоинта
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        (RPC_CSTR)"1500",
        NULL);
    if (status != RPC_S_OK) {
        cout << "RpcServerUseProtseqEpA failed: " << status << endl;
        return 1;
    }
    status = RpcServerRegisterIf2(InterfaceRPC_v1_0_s_ifspec, // подключаем интерфейс
        NULL, NULL,
        RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        -1,
        NULL);
    if (status != RPC_S_OK) {
        cout << "RpcServerRegisterIf2 failed: " << status << endl;
        return 1;
    }
    cout << "[SERVER] Listening on port 1500..." << endl;
    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 0);
    cout << "[SERVER] RpcServerListen returned: " << status << endl;
    return 0;
}
