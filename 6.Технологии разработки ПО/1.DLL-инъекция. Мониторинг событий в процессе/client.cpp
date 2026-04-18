#include "pch.h"
#include "client.h"
#include <cstdio>

IpcClient::IpcClient() : m_sock(INVALID_SOCKET), m_wsaInit(false)
{
    InitializeCriticalSection(&m_sendLock);
}

IpcClient::~IpcClient()
{
    Close();
    DeleteCriticalSection(&m_sendLock);
}

bool IpcClient::ConnectLocal(uint16_t port)
{
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;
    m_wsaInit = true;

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sock == INVALID_SOCKET)
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return false;

    return true;
}

void IpcClient::Close()
{
    if (m_sock != INVALID_SOCKET)
    {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }
    if (m_wsaInit)
    {
        WSACleanup();
        m_wsaInit = false;
    }
}

bool IpcClient::SendLine(const std::string& line)
{
    if (m_sock == INVALID_SOCKET) return false;

    EnterCriticalSection(&m_sendLock);

    int total = 0;
    int len = (int)line.size();
    while (total < len)
    {
        int sent = send(m_sock, line.data() + total, len - total, 0);
        if (sent <= 0)
        {
            LeaveCriticalSection(&m_sendLock);
            return false;
        }
        total += sent;
    }

    LeaveCriticalSection(&m_sendLock);
    return true;
}

bool IpcClient::RecvLine(std::string& outLine)
{
    outLine.clear();
    char c;
    while (true)
    {
        int r = recv(m_sock, &c, 1, 0);
        if (r <= 0) return false;
        outLine.push_back(c);
        if (c == '\n') return true;
        if (outLine.size() > 64 * 1024) return false;
    }
}