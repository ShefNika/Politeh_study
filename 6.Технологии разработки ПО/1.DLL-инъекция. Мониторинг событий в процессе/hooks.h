#pragma once
#include <windows.h>
#include <string>
#include <cstdint>

struct HookConfig
{
    bool hideMode = false;
    std::string monitorFunc;  
    std::wstring hideFullPath;
    std::wstring hideName;
};

DWORD WINAPI HookInitThread(LPVOID);
void ApplyConfig(const HookConfig& cfg);

void InstallHooks();
void UninstallHooks();