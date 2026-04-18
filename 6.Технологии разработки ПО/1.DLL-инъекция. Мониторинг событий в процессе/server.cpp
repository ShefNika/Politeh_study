#include "server.h"
#include <cstdio>

IpcServer::IpcServer() : m_listen(INVALID_SOCKET), m_wsaInit(false) {}
IpcServer::~IpcServer() { Stop(); }

bool IpcServer::Start(uint16_t port)
{
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::printf("WSAStartup failed\n");
        return false;
    }
    m_wsaInit = true;

    m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4, потоковый сокет, TCP
    if (m_listen == INVALID_SOCKET)
    {
        std::printf("socket failed (%d)\n", WSAGetLastError());
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(m_listen, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::printf("bind failed (%d)\n", WSAGetLastError());
        return false;
    }

    if (listen(m_listen, 1) == SOCKET_ERROR)
    {
        std::printf("listen failed (%d)\n", WSAGetLastError());
        return false;
    }

    std::printf("Server listening on 127.0.0.1:%u\n", port);
    return true;
}

SOCKET IpcServer::AcceptClient()
{
    return accept(m_listen, nullptr, nullptr);
}

void IpcServer::Stop()
{
    if (m_listen != INVALID_SOCKET)
    {
        closesocket(m_listen);
        m_listen = INVALID_SOCKET;
    }
    if (m_wsaInit)
    {
        WSACleanup();
        m_wsaInit = false;
    }
}

bool IpcServer::SendLine(SOCKET s, const std::string& line)
{
    int total = 0;
    int len = (int)line.size();
    while (total < len)
    {
        int sent = send(s, line.data() + total, len - total, 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

bool IpcServer::RecvLine(SOCKET s, std::string& outLine)
{
    outLine.clear();
    char c;
    while (true)
    {
        int r = recv(s, &c, 1, 0);
        if (r <= 0) return false;
        outLine.push_back(c);
        if (c == '\n') return true;
        if (outLine.size() > 64 * 1024) return false;
    }
}