#include "hook_patch.h"
#include <cstring>
#include <cstdio>


static constexpr size_t ABS_JMP_SIZE = 12;
static constexpr size_t K32_STUB_SIZE = 6;

HookPatch::HookPatch(uint8_t* targetFunction, uint8_t* detourFunction)
    : m_targetFunction(targetFunction)
    , m_detourFunction(detourFunction)
    , m_trampoline(nullptr)
    , m_relay(nullptr)
    , m_patchSize(0)
    , m_isInstalled(false)
{}

HookPatch::~HookPatch()
{
    if (m_isInstalled)
        Uninstall();
    if (m_trampoline)
        VirtualFree(m_trampoline, 0, MEM_RELEASE);
    if (m_relay)
        VirtualFree(m_relay, 0, MEM_RELEASE);
}

bool HookPatch::DecidePatchSize()
{
    if (!m_targetFunction) return false;
    if (m_targetFunction[0] == 0xFF && m_targetFunction[1] == 0x25)
    {
        m_patchSize = K32_STUB_SIZE;
        return true;
    }
    m_patchSize = ABS_JMP_SIZE;
    return true;
}

void HookPatch::SaveOriginalBytes()
{
    m_originalBytes.resize(m_patchSize);
    std::memcpy(m_originalBytes.data(), m_targetFunction, m_patchSize);
}

static uint8_t* FindStubTarget(uint8_t* fn)
{
    if (!fn || fn[0] != 0xFF || fn[1] != 0x25)
        return nullptr;
    int32_t disp = 0;
    std::memcpy(&disp, fn + 2, sizeof(disp));
    // rip после инструкции 
    // т.е. jmp qword ptr [rip + disp32]
    uint8_t* ripNext = fn + 6;
    // адрес ячейки (в памяти), где лежит 8-байтный указатель назначения
    uint8_t** pTargetPtr = (uint8_t**)(ripNext + disp);
    uint8_t* realTarget = *pTargetPtr;
    return realTarget;
}

bool HookPatch::BuildTrampoline()
{
    if (m_patchSize == K32_STUB_SIZE &&
        m_targetFunction[0] == 0xFF && m_targetFunction[1] == 0x25)
    {
        uint8_t* realTarget = FindStubTarget(m_targetFunction);
        if (!realTarget) return false;
        const size_t size = ABS_JMP_SIZE;
        m_trampoline = (uint8_t*)VirtualAlloc(
            nullptr, size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
        );
        if (!m_trampoline) return false;
        WriteAbsJump(m_trampoline, realTarget);
        FlushInstructionCache(GetCurrentProcess(), m_trampoline, size);
        return true;
    }
    const size_t size = m_patchSize + ABS_JMP_SIZE;
    m_trampoline = (uint8_t*)VirtualAlloc(
        nullptr, size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (!m_trampoline) return false;
    std::memcpy(m_trampoline, m_originalBytes.data(), m_patchSize);
    if (!WriteAbsJump(m_trampoline + m_patchSize, m_targetFunction + m_patchSize))
        return false;
    FlushInstructionCache(GetCurrentProcess(), m_trampoline, size);
    return true;
}


bool HookPatch::WriteAbsJump(uint8_t* at, uint8_t* destination)
{
    // mov rax, imm64
    at[0] = 0x48;
    at[1] = 0xB8;
    uint64_t addr = (uint64_t)destination;
    std::memcpy(at + 2, &addr, 8);
    // jmp rax
    at[10] = 0xFF;
    at[11] = 0xE0;
    return true;
}

bool HookPatch::WriteRelJump(uint8_t* at, uint8_t* destination)
{
    // jmp reg32
    intptr_t rel = (intptr_t)destination - (intptr_t)(at + 5);
    if (rel < INT32_MIN || rel > INT32_MAX)
        return false;
    at[0] = 0xE9;
    int32_t rel32 = (int32_t)rel;
    std::memcpy(at + 1, &rel32, 4);
    if (m_patchSize > 5)
        at[5] = 0x90;
    return true;
}

uint8_t* HookPatch::AllocNear(uint8_t* nearTo, size_t size)
{
    SYSTEM_INFO si{};
    GetSystemInfo(&si);

    const size_t gran = si.dwAllocationGranularity; 
    uintptr_t start = ((uintptr_t)nearTo) & ~(gran - 1);
    const intptr_t MAX_DELTA = (intptr_t)0x7FFF0000; 

    for (intptr_t delta = 0; delta < MAX_DELTA; delta += (intptr_t)gran)
    {
        uintptr_t hi = start + (uintptr_t)delta;
        uintptr_t lo = start - (uintptr_t)delta;
        void* p1 = VirtualAlloc((void*)hi, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (p1) return (uint8_t*)p1;
        void* p2 = VirtualAlloc((void*)lo, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (p2) return (uint8_t*)p2;
    }
    return nullptr;
}



bool HookPatch::Install()
{
    if (m_isInstalled) return true;
    if (!m_targetFunction || !m_detourFunction) return false;
    if (!DecidePatchSize()) return false;
    SaveOriginalBytes();
    if (!BuildTrampoline())
        return false;
    DWORD oldProtect{};
    if (!VirtualProtect(m_targetFunction, m_patchSize, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;
    bool ok = false;
    if (m_patchSize == ABS_JMP_SIZE)
        ok = WriteAbsJump(m_targetFunction, m_detourFunction);
    else
    {
        m_relay = AllocNear(m_targetFunction, ABS_JMP_SIZE);
        if (m_relay)
        {
            WriteAbsJump(m_relay, m_detourFunction);
            ok = WriteRelJump(m_targetFunction, m_relay);
        }
    }
    VirtualProtect(m_targetFunction, m_patchSize, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), m_targetFunction, m_patchSize);
    if (!ok)
    {
        if (m_trampoline) { VirtualFree(m_trampoline, 0, MEM_RELEASE); m_trampoline = nullptr; }
        if (m_relay) { VirtualFree(m_relay, 0, MEM_RELEASE); m_relay = nullptr; }
        return false;
    }
    m_isInstalled = true;
    return true;
}


bool HookPatch::Uninstall()
{
    if (!m_isInstalled) return true;

    DWORD oldProtect{};
    if (!VirtualProtect(m_targetFunction, m_patchSize, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    std::memcpy(m_targetFunction, m_originalBytes.data(), m_patchSize);

    VirtualProtect(m_targetFunction, m_patchSize, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), m_targetFunction, m_patchSize);

    m_isInstalled = false;
    return true;
}

