#include "tcpProtocolServer.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

static volatile SOCKET tcp_client = INVALID_SOCKET;
static TcpProtocol_ReceiveCallback tcp_receive_cb;

void TcpProtocol_SendBuf(uint8_t* data, uint16_t len)
{
    SOCKET client = tcp_client;
    uint16_t offset = 0;
    while (client != INVALID_SOCKET && offset < len)
    {
        int sent = send(client, (const char*)data + offset, len - offset, 0);
        if (sent <= 0)
            return;
        offset = (uint16_t)(offset + sent);
    }
}

static DWORD WINAPI TcpProtocol_Thread(LPVOID parameter)
{
    WSADATA wsa;
    SOCKET listener;
    struct sockaddr_in address;
    uint16_t port = (uint16_t)(uintptr_t)parameter;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
        return 2;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (bind(listener, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR ||
        listen(listener, 1) == SOCKET_ERROR)
    {
        closesocket(listener);
        return 3;
    }
    printf("Host control TCP: 127.0.0.1:%u\n", port);
    for (;;)
    {
        uint8_t buffer[256];
        SOCKET client = accept(listener, NULL, NULL);
        if (client == INVALID_SOCKET)
            continue;
        tcp_client = client;
        for (;;)
        {
            int count = recv(client, (char*)buffer, sizeof(buffer), 0);
            int i;
            if (count <= 0)
                break;
            for (i = 0; i < count; ++i)
                if (tcp_receive_cb != 0)
                    tcp_receive_cb(buffer[i]);
        }
        tcp_client = INVALID_SOCKET;
        closesocket(client);
    }
}

uint8_t TcpProtocol_Start(uint16_t port, TcpProtocol_ReceiveCallback receive_cb)
{
    HANDLE thread;
    tcp_receive_cb = receive_cb;
    thread = CreateThread(NULL, 0, TcpProtocol_Thread, (LPVOID)(uintptr_t)port, 0, NULL);
    if (thread == NULL)
        return 0;
    CloseHandle(thread);
    return 1;
}
