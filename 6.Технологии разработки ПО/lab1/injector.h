#pragma once
#include <windows.h>

bool InjectDll(DWORD pid, const wchar_t* dllFullPath);
