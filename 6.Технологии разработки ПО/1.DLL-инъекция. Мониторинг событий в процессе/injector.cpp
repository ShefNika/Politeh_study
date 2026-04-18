#include <windows.h>
#include <stdio.h>


bool InjectDll(DWORD pid, const wchar_t* dllFullPath)
{
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | // desiredAccess - запись в память, 
                                                                                    //выделение памяти, создание удаленного потока
        PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, // inheritHandle - наследование handle дочерникими процессами
        pid
    );
    if (!hProcess)
        return 1;
    size_t bytesToWrite = (wcslen(dllFullPath) + 1) * sizeof(wchar_t);
    LPVOID remoteMem = VirtualAllocEx(
        hProcess,
        nullptr,
        bytesToWrite,
        MEM_COMMIT | MEM_RESERVE, // выделить и сразу сделать доступной
        PAGE_READWRITE
    );
    if (!remoteMem) {
        CloseHandle(hProcess);
        return 1;
    }
    if (!WriteProcessMemory(
        hProcess,
        remoteMem,
        dllFullPath,
        bytesToWrite,
        nullptr))
    {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32)
    {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW)
    {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    // создание удаленного потока в целевом процессе,
    // он вызовет LoadLibraryW(remoteMem);
    HANDLE hThread = CreateRemoteThread(
        hProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)pLoadLibraryW,
        remoteMem,
        0,
        nullptr
    );
    if (!hThread)
    {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    WaitForSingleObject(hThread, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return exitCode != 0;
}