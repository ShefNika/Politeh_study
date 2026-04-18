#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

class IpcServer
{
public:
    IpcServer();
    ~IpcServer();

    bool Start(uint16_t port);
    SOCKET AcceptClient();           
    void Stop();

    static bool SendLine(SOCKET s, const std::string& line);
    static bool RecvLine(SOCKET s, std::string& outLine); 

private:
    SOCKET m_listen;
    bool m_wsaInit;
};