#pragma once
#include <string>
#include <vector>
#include <cstdint>


inline std::string BuildCfgMonitor(const std::string& funcName)
{
    return "CFG MONITOR " + funcName + "\n";
}

inline std::string BuildCfgHide(const std::string& hidePath)
{
    return "CFG HIDE " +  hidePath + "\n";
}
