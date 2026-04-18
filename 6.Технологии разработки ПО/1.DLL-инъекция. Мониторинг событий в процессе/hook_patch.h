#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>


class HookPatch {
private:
    uint8_t* m_targetFunction;
    uint8_t* m_detourFunction;
    uint8_t* m_trampoline;
    uint8_t* m_relay;
    std::vector<uint8_t> m_originalBytes;
    size_t m_patchSize;
    bool m_isInstalled;

    void SaveOriginalBytes();
    bool DecidePatchSize();
    bool BuildTrampoline();
    bool WriteAbsJump(uint8_t* at, uint8_t* destination);
    bool WriteRelJump(uint8_t* at, uint8_t* destination);
    uint8_t* AllocNear(uint8_t* nearTo, size_t size);

public:
    HookPatch(uint8_t* targetFunction, uint8_t* detourFunction);
    ~HookPatch();

    bool Install();
    bool Uninstall();
    uint8_t* GetTrampoline() const { return m_trampoline; }
    bool IsInstalled() const { return m_isInstalled; }
};