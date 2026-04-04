#include "pch.h"
#include "hooks.h"
#include "client.h"
#include "hook_patch.h"

#include <cstdio>
#include <cstring>
#include <cwchar>


typedef HANDLE(WINAPI* CreateFileW_t)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* FindFirstFileW_t)(LPCWSTR, LPWIN32_FIND_DATAW);
typedef BOOL(WINAPI* FindNextFileW_t)(HANDLE, LPWIN32_FIND_DATAW);

static IpcClient  g_ipc;
static HookConfig g_cfg;

static CreateFileW_t    g_OrigCreateFileW = nullptr;
static FindFirstFileW_t g_OrigFindFirstFileW = nullptr;
static FindNextFileW_t  g_OrigFindNextFileW = nullptr;

static HookPatch* g_hookCreate = nullptr;
static HookPatch* g_hookFF = nullptr;
static HookPatch* g_hookFN = nullptr;

static HookPatch* g_hookMonitor = nullptr;
static uint8_t* g_monitorStub = nullptr;
static uint8_t* g_monitorTrampoline = nullptr;

static std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], n);
    return out;
}

static std::string WideToUtf8(const wchar_t* w)
{
    if (!w) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    std::string out(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], n, nullptr, nullptr);
    return out;
}

static std::wstring Basename(const std::wstring& path)
{
    size_t pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return path;
    return path.substr(pos + 1);
}


static bool IsHiddenName(const wchar_t* name)
{
    if (!g_cfg.hideMode) return false;
    if (g_cfg.hideName.empty() || !name) return false;
    return _wcsicmp(g_cfg.hideName.c_str(), name) == 0;
}

static bool IsHiddenPath(const wchar_t* fullPath)
{
    if (!g_cfg.hideMode) return false;
    if (g_cfg.hideFullPath.empty() || !fullPath) return false;
    return _wcsicmp(g_cfg.hideFullPath.c_str(), fullPath) == 0;
}


static void SendEvent(const char* funcName, const std::string& detail)
{
    SYSTEMTIME st{};
    GetLocalTime(&st);
    char ts[64];
    sprintf_s(ts, "%02u:%02u:%02u %02u.%02u",
        st.wHour, st.wMinute, st.wSecond, st.wDay, st.wMonth);
    std::string line = std::string("EVT ") + ts + " " + (funcName ? funcName : "(null)");
    if (!detail.empty())
        line += " " + detail;
    line += "\n";
    g_ipc.SendLine(line);
}


static void LogGenericName(const char* funcName)
{
    if (!funcName) return;
    SendEvent(funcName, "");
}


static uint8_t* BuildStub(const char* funcNamePtr, void(*logFn)(const char*), uint8_t** pTrampolinePtr) {
    uint8_t* code = (uint8_t*)VirtualAlloc(nullptr, 256, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!code) return nullptr;
    uint8_t* p = code;
    auto emit = [&](uint8_t b) { *p++ = b; };
    auto emit64 = [&](uint64_t v) { std::memcpy(p, &v, 8); p += 8; };
    // pushfq
    emit(0x9C);
    emit(0x50);                 // push rax
    emit(0x51);                 // push rcx
    emit(0x52);                 // push rdx
    emit(0x41); emit(0x50);     // push r8
    emit(0x41); emit(0x51);     // push r9
    emit(0x41); emit(0x52);     // push r10
    emit(0x41); emit(0x53);     // push r11
    // sub rsp, 0x28  
    emit(0x48); emit(0x83); emit(0xEC); emit(0x28);
    // mov rcx, imm64 (arg1 = funcNamePtr)
    emit(0x48); emit(0xB9);
    emit64((uint64_t)funcNamePtr);
    // mov rax, imm64 (logFn)
    emit(0x48); emit(0xB8);
    emit64((uint64_t)logFn);
    // call rax
    emit(0xFF); emit(0xD0);
    // add rsp, 0x28
    emit(0x48); emit(0x83); emit(0xC4); emit(0x28);
    // pop r11 r10 r9 r8 rdx rcx rax
    emit(0x41); emit(0x5B);     // pop r11
    emit(0x41); emit(0x5A);     // pop r10
    emit(0x41); emit(0x59);     // pop r9
    emit(0x41); emit(0x58);     // pop r8
    emit(0x5A);                 // pop rdx
    emit(0x59);                 // pop rcx
    emit(0x58);                 // pop rax
    // popfq
    emit(0x9D);
    // mov rax, imm64 (&g_monitorTrampoline)
    emit(0x48); emit(0xB8);
    emit64((uint64_t)pTrampolinePtr);
    // mov rax, [rax]
    emit(0x48); emit(0x8B); emit(0x00);
    // jmp rax
    emit(0xFF); emit(0xE0);
    FlushInstructionCache(GetCurrentProcess(), code, 256);
    return code;
}


HANDLE WINAPI Hooked_CreateFileW(
    LPCWSTR fileName, DWORD access, DWORD share,
    LPSECURITY_ATTRIBUTES sec, DWORD creation,
    DWORD flags, HANDLE templ)
{
    SendEvent("CreateFileW", WideToUtf8(fileName ? fileName : L"(null)"));
    if (g_cfg.hideMode && fileName)
    {
        const wchar_t* base = wcsrchr(fileName, L'\\');
        base = base ? base + 1 : fileName;
        if (IsHiddenPath(fileName) || IsHiddenName(base))
        {
            SetLastError(ERROR_FILE_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
        }
    }
    return g_OrigCreateFileW(fileName, access, share, sec, creation, flags, templ);
}

HANDLE WINAPI Hooked_FindFirstFileW(LPCWSTR pattern, LPWIN32_FIND_DATAW data)
{
    SendEvent("FindFirstFileW", WideToUtf8(pattern ? pattern : L"(null)"));
    HANDLE h = g_OrigFindFirstFileW(pattern, data);
    if (g_cfg.hideMode && h != INVALID_HANDLE_VALUE && data)
    {
        if (IsHiddenName(data->cFileName))
        {
            while (true)
            {
                BOOL ok = g_OrigFindNextFileW(h, data);
                if (!ok)
                {
                    DWORD e = GetLastError();
                    FindClose(h);
                    SetLastError(e);
                    return INVALID_HANDLE_VALUE;
                }
                if (!IsHiddenName(data->cFileName))
                    break;
            }
        }
    }
    return h;
}

BOOL WINAPI Hooked_FindNextFileW(HANDLE hFind, LPWIN32_FIND_DATAW data)
{
    BOOL ok = g_OrigFindNextFileW(hFind, data);
    if (!ok) return ok;
    if (data)
        SendEvent("FindNextFileW", WideToUtf8(data->cFileName));
    if (g_cfg.hideMode && data)
    {
        while (ok && IsHiddenName(data->cFileName))
        {
            ok = g_OrigFindNextFileW(hFind, data);
            if (ok && data)
                SendEvent("FindNextFileW", WideToUtf8(data->cFileName));
        }
    }
    return ok;
}


static void InstallOneTyped(uint8_t* target, uint8_t* detour, HookPatch** outHook, void** outOrig)
{
    HookPatch* hp = new HookPatch(target, detour);
    if (!hp->Install())
    {
        delete hp;
        *outHook = nullptr;
        *outOrig = nullptr;
        return;
    }
    *outHook = hp;
    *outOrig = hp->GetTrampoline();
}

static void FreeMonitorStub()
{
    if (g_monitorStub)
    {
        VirtualFree(g_monitorStub, 0, MEM_RELEASE);
        g_monitorStub = nullptr;
    }
}


static void InstallMonitorByName(const std::string& funcName)
{
    HMODULE hK32 = GetModuleHandleW(L"kernel32.dll");
    uint8_t* target = (uint8_t*)GetProcAddress(hK32, funcName.c_str());
    if (!target)
    {
        std::string msg = "EVT 0 ERROR Unknown function: " + funcName + "\n";
        g_ipc.SendLine(msg);
        return;
    }
    const char* funcNamePtr = g_cfg.monitorFunc.c_str();
    FreeMonitorStub();
    g_monitorTrampoline = nullptr;
    g_monitorStub = BuildStub(funcNamePtr, &LogGenericName, &g_monitorTrampoline);
    g_hookMonitor = new HookPatch(target, g_monitorStub);
    if (!g_hookMonitor->Install())
    {
        delete g_hookMonitor;
        g_hookMonitor = nullptr;
        g_ipc.SendLine("EVT 0 ERROR Monitor hook install failed\n");
        return;
    }
    g_monitorTrampoline = g_hookMonitor->GetTrampoline();
}


void InstallHooks()
{
    HMODULE hK32 = GetModuleHandleW(L"kernel32.dll");
    if (!hK32) return;
    if (g_cfg.hideMode)
    {
        uint8_t* pCreate = (uint8_t*)GetProcAddress(hK32, "CreateFileW");
        uint8_t* pFF = (uint8_t*)GetProcAddress(hK32, "FindFirstFileW");
        uint8_t* pFN = (uint8_t*)GetProcAddress(hK32, "FindNextFileW");
        if (!pCreate || !pFF || !pFN) return;
        InstallOneTyped(pCreate, (uint8_t*)Hooked_CreateFileW, &g_hookCreate, (void**)&g_OrigCreateFileW);
        InstallOneTyped(pFF, (uint8_t*)Hooked_FindFirstFileW, &g_hookFF, (void**)&g_OrigFindFirstFileW);
        InstallOneTyped(pFN, (uint8_t*)Hooked_FindNextFileW, &g_hookFN, (void**)&g_OrigFindNextFileW);
        return;
    }
    if (g_cfg.monitorFunc.empty())
    {
        g_ipc.SendLine("EVT 0 ERROR Empty monitor function\n");
        return;
    }
    InstallMonitorByName(g_cfg.monitorFunc);
}

void UninstallHooks()
{
    if (g_hookMonitor)
    {
        g_hookMonitor->Uninstall();
        delete g_hookMonitor;
        g_hookMonitor = nullptr;
    }
    FreeMonitorStub();
    g_monitorTrampoline = nullptr;
    if (g_hookFN) { g_hookFN->Uninstall(); delete g_hookFN; g_hookFN = nullptr; }
    if (g_hookFF) { g_hookFF->Uninstall(); delete g_hookFF; g_hookFF = nullptr; }
    if (g_hookCreate) { g_hookCreate->Uninstall(); delete g_hookCreate; g_hookCreate = nullptr; }
    g_OrigCreateFileW = nullptr;
    g_OrigFindFirstFileW = nullptr;
    g_OrigFindNextFileW = nullptr;
}

void ApplyConfig(const HookConfig& cfg)
{
    g_cfg = cfg;
}


static bool ParseCfgLine(const std::string& line, HookConfig& out)
{
    std::string s = line;
    if (!s.empty() && s.back() == '\n') s.pop_back();
    if (s.rfind("CFG MONITOR ", 0) == 0)
    {
        out.hideMode = false;
        out.monitorFunc = s.substr(strlen("CFG MONITOR "));
        out.hideFullPath.clear();
        out.hideName.clear();
        return !out.monitorFunc.empty();
    }

    if (s.rfind("CFG HIDE ", 0) == 0)
    {
        out.hideMode = true;
        std::string pathUtf8 = s.substr(strlen("CFG HIDE "));
        if (pathUtf8.empty()) return false;

        out.monitorFunc.clear();
        out.hideFullPath = Utf8ToWide(pathUtf8);
        out.hideName = Basename(out.hideFullPath);
        return true;
    }

    return false;
}


DWORD WINAPI HookInitThread(LPVOID)
{
    const uint16_t PORT = 27015;
    if (!g_ipc.ConnectLocal(PORT))
        return 0;
    {
        char buf[64];
        sprintf_s(buf, "HELLO %lu\n", GetCurrentProcessId());
        g_ipc.SendLine(buf);
    }
    std::string cfgLine;
    if (!g_ipc.RecvLine(cfgLine))
        return 0;

    HookConfig cfg{};
    if (!ParseCfgLine(cfgLine, cfg))
    {
        g_ipc.SendLine("EVT 0 ERROR Bad CFG line\n");
        return 0;
    }
    ApplyConfig(cfg);
    InstallHooks();
    return 0;
}