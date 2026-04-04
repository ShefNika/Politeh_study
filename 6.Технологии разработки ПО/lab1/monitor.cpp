#include "server.h"   
#include <tlhelp32.h>   
#include <cstdio>
#include <string>
#include <vector>

#include "injector.h"
#include "protocol.h"




static DWORD FindPidByName(const wchar_t* exeName)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe))
    {
        do
        {
            if (_wcsicmp(pe.szExeFile, exeName) == 0)
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}


static std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, const_cast<char*>(out.data()), n, nullptr, nullptr);
    return out;
}

int wmain(int argc, wchar_t* argv[])
{
    DWORD pid = 0;
    std::wstring name;
    std::string func = "ALL";
    bool hasFunc = false;
    bool hasHide = false;
    std::wstring hidePath;
    for (int i = 1; i < argc; i++)
    {
        if (_wcsicmp(argv[i], L"--pid") == 0 && i + 1 < argc)
            pid = (DWORD)_wtoi(argv[++i]);
        else if (_wcsicmp(argv[i], L"--name") == 0 && i + 1 < argc)
            name = argv[++i];
        else if (_wcsicmp(argv[i], L"--func") == 0 && i + 1 < argc)
        {
            func = WideToUtf8(argv[++i]);
            hasFunc = true;
        }
        else if (_wcsicmp(argv[i], L"--hide") == 0 && i + 1 < argc)
        {
            hidePath = argv[++i];
            hasHide = true;
        }
        else
        {
            std::wprintf(L"Unknown arg: %s\n", argv[i]);
            return 1;
        }
    }
    if (pid == 0)
    {
        if (name.empty())
            return 1;
        pid = FindPidByName(name.c_str());
        if (pid == 0)
        {
            std::wprintf(L"Process not found: %s\n", name.c_str());
            return 1;
        }
    }
    std::printf("Target PID: %lu\n", pid);
    uint32_t mask = 0;
    wchar_t dllFullPath[MAX_PATH]{};
    GetFullPathNameW(L"HookDLL.dll", MAX_PATH, dllFullPath, nullptr);
    std::wprintf(L"Using DLL: %s\n", dllFullPath);
    const uint16_t PORT = 27015;
    IpcServer server;
    if (!server.Start(PORT))
        return 1;
    if (!InjectDll(pid, dllFullPath))
    {
        std::printf("Injection failed\n");
        return 1;
    }
    SOCKET client = server.AcceptClient();
    if (client == INVALID_SOCKET)
    {
        std::printf("accept failed (%d)\n", WSAGetLastError());
        return 1;
    }
    std::printf("DLL connected\n");

    std::string line;
    if (IpcServer::RecvLine(client, line))
        std::printf("[HookDLL]: %s", line.c_str());
    if (hasHide)
    {
        std::string hideUtf8 = WideToUtf8(hidePath);
        std::string cfg = BuildCfgHide(hideUtf8);
        IpcServer::SendLine(client, cfg);
        std::printf("Sent: %s", cfg.c_str());
    }
    else
    {
        std::string cfg = BuildCfgMonitor(func);
        IpcServer::SendLine(client, cfg);
        std::printf("Sent: %s", cfg.c_str());
    }
    std::printf("\nListening for events\n");
    while (IpcServer::RecvLine(client, line))
        std::printf("%s", line.c_str());

    std::printf("DLL disconnected.\n");
    closesocket(client);
    return 0;
}