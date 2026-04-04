#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

class IpcClient
{
public:
    IpcClient();
    ~IpcClient();

    bool ConnectLocal(uint16_t port);
    void Close();

    bool SendLine(const std::string& line);
    bool RecvLine(std::string& outLine);

private:
    SOCKET m_sock;
    bool m_wsaInit;
    CRITICAL_SECTION m_sendLock;
};
